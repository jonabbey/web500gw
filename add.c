/*
 * web500gw:  A LDAP based WWW - X.500 gateway
 *
 * written by: Frank Richter, Frank.Richter@hrz.tu-chemnitz.de
 *             Chemnitz University of Technology, Germany
 *
 * Copyright (c) 1994-8 Chemnitz University of Technology.
 * All rights reserved.
 *
 * $Id: add.c,v 1.2 2001/04/26 22:16:53 dekarl Exp $
 */

#include "web500gw.h"

int
do_addform(
    REQUEST     *r,
    RESPONSE    *resp
)
{
#ifndef MODIFY
    do_error(r, resp, LDAP_OTHER, NOT_IMPLEMENTED, MSG_NOT_SUPPORTED, NULL);
    return NOTOK;
#else   /* defined MODIFY */

    int             rc, in_home;
    char            *bind_as, *pw, *args, *tmpl, *basedn, *rdn, *ufn, *cp;
    LDAPMessage     *res;
    struct timeval  timeout;
    struct ldap_disptmpl    *templ;


/* 
 * query = default Base-DN, 
 * args = dn=BindDN&userPassword=PASSWD&tmpl=template&basedn=DN
 */
    args = r->r_method == POST ? r->r_postdata : r->r_filter;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "do_addform (%s, %s)\n",    
        r->r_dn, args ? args : "", 0, 0);
#endif

    if (args == NULL) {
        do_error(r, resp, LDAP_OTHER, BAD_REQUEST, NULL, NULL);
        return NOTOK;
    }
    bind_as = pw = tmpl = basedn = NULL;
    cp = args;
    while (cp && *cp) {
        if (strncmp(cp, "dn=", 3) == 0) {
            bind_as = cp + 3;
        } else if (strncmp(cp, "userPassword=", 13) == 0) {
            pw = cp + 13;
        } else if (strncmp(cp, "tmpl=", 5) == 0) {
            tmpl = cp + 5;
        } else if (strncmp(cp, "basedn=", 7) == 0) {
            basedn = cp + 7;
        } /* else ignore */
        cp = strchr(cp, '&');
        if (cp && *cp)
            *cp++ = '\0';
    }
    if (!bind_as) {
        do_error(r, resp, LDAP_OTHER, BAD_REQUEST, MSG_NO_BIND_DN, NULL);
        return NOTOK;
    }
    hex_qdecode(bind_as);

    if (! pw) {
        do_error(r, resp, LDAP_OTHER, BAD_REQUEST, MSG_MISSING_PASSWORD, NULL);
        return NOTOK;
    }
    hex_qdecode(pw);
    if (basedn && *basedn) {
        hex_qdecode(basedn);
    } else {
        basedn = r->r_dn;
    }
    hex_qdecode(tmpl);
    if (tmpl && *tmpl)      /* overrides the template in URL */
        r->r_template = tmpl;
        
    if (! (r->r_template && *r->r_template)) {
        /* need a objectclass template */
        do_error(r, resp, LDAP_OTHER, BAD_REQUEST, MSG_TEMPLATE_MISSING, NULL);
        return NOTOK;
    }

/*  No bind necessary at this stage but checking it anyway to block
 *  non-admins */
    if ((!bind_as) || strlen(bind_as) == 0) { /* No DN to bind as */
        do_error (r, resp, LDAP_INAPPROPRIATE_AUTH, BAD_REQUEST, 
                 MSG_UNKNOWN_ARGUMENT, NULL);
        return NOTOK;
    }
    r->r_binddn = bind_as;
    if ((!pw) || strlen(pw) == 0) { /* we need a password for simple auth */
        do_error (r, resp, LDAP_INAPPROPRIATE_AUTH, 0, MSG_NULL_PASSWORD, NULL);
        return NOTOK;
    }

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "BINDING as %s ... ", bind_as, 0, 0, 0);
#endif
    if ((r->r_ld = web500gw_ldap_init(r, resp, bind_as, pw, 1)) == (LDAP *)0)
        return NOTOK;
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "BOUND\n", 0, 0, 0, 0);
#endif

    /* let's see if basedn exists ... */
    timeout.tv_sec = timelimit;
    timeout.tv_usec = 0;
    if ((rc = ldap_search_st(r->r_ld, basedn, LDAP_SCOPE_BASE, default_filter,
        NULL, 0, &timeout, &res)) != LDAP_SUCCESS) {
        /* better error description here ??? */
        do_error(r, resp, rc, 0, r->r_ld->ld_error, r->r_ld->ld_matched);
        return NOTOK;
    }
    /* check if nonleaf here ??? */

    ufn = friendly_dn(resp, basedn);
    rdn = friendly_rdn(resp, basedn, 1);
    in_home = isinhome(basedn);
 
    /* find the requested template */
    templ = ldap_name2template(r->r_template, r->r_access->a_tmpllist);
    if (templ == NULL) {            /* template not found */
        do_error(r, resp, LDAP_OTHER, BAD_REQUEST, MSG_TEMPLATE_MISSING, NULL);
        return NOTOK;
    }
    /* .. and see if it's allowed to add such an entry */
    if (!LDAP_IS_DISPTMPL_OPTION_SET(templ, LDAP_DTMPL_OPT_ADDABLE)) {
        do_error(r, resp, LDAP_OTHER, BAD_REQUEST, MSG_ADD_OC_NOT_ADDABLE, NULL);
        return NOTOK;
    }

    if (r->r_httpversion == 1 && resp->resp_httpheader == 0) {
        /* Expires now - don't allow to cache */
        /* resp->resp_expires = 0; */
        http_header(r, resp);
    }
    if (resp->resp_htmlheader == 0) {
        msg_fprintf(fp, in_home && *MSG_HTML_START_MISC_HOME ?
            MSG_HTML_START_MISC_HOME : MSG_HTML_START_MISC, 
            "sss", MSG_ADD, rdn, ufn);
        resp->resp_htmlheader = 1;
    }
    msg_fprintf(fp, MSG_EXPLAIN_ADD, "ss", rdn, ufn);
    fprintf(fp, "<FORM METHOD=POST ACTION=\"%s\">\n", 
        dn2url(r, basedn, FLAG_LANGUAGE|FLAG_TMPL, ACTION_ADD, NULL, NULL));

    if (r->r_browser->b_opts & B_HIDDEN) {
        fprintf(fp, "<INPUT TYPE=\"hidden\" NAME=\"adder_dn\" VALUE=\"%s\">\n\
<INPUT TYPE=\"hidden\" NAME=\"adder_pw\" VALUE=\"%s\">\n",
            html_encode(bind_as), html_encode(pw));

    } else {
        fprintf(fp, "<INPUT TYPE=\"radio\" NAME=\"adder_dn\" VALUE=\"%s\" CHECKED>\n\
<INPUT TYPE=\"radio\" NAME=\"adder_pw\" VALUE=\"%s\" CHECKED>\n",
            html_encode(bind_as), html_encode(pw));
    }

    free(rdn);
    free(ufn);

    fputs(MSG_HELP_MODIFY, fp);
    /* print the empty form with attributes */
    if (!(r->r_flags & FLAG_LAYOUT)) { 
        r->r_flags |= r->r_browser->b_flags;
    }
    r->r_actions |= ACTION_ADDFORM;

    if ((rc =  web500gw_entry2html(NULL, r, resp, NULL, templ, NULL,
             NULL, 0)) != LDAP_SUCCESS) {
            do_error(r, resp, rc, 0, NULL, NULL);
            return NOTOK;
    }
    fprintf(fp, "\n%s</FORM>\n%s\n", MSG_ADD_ACTION, MSG_HTML_END);
    return OK;
#endif
}


/*
 * do_add: Performs an LDAP ADD request:
 */
int
do_add(
    REQUEST     *r,
    RESPONSE    *resp
)
{

#ifndef MODIFY
    do_error(r, resp, LDAP_OTHER, NOT_IMPLEMENTED, MSG_NOT_SUPPORTED, NULL);
    return NOTOK;
#else   /* defined MODIFY */

    char    *bind_as, *basedn, *args, *nval, *error; 
    char    *ufn, *rdn = NULL, *dn, *pw, *rdnattr, *next, *val, *cp, *attr;
    int     rc, attrvals = 0, maxattrvals, i, j, valcount, found, in_home;
    struct ldap_disptmpl    *tmpl;
    struct ldap_tmplitem    *rowp, *colp;
    struct ldap_adddeflist  *defaults;
    LDAPMod  **user_attrs, **ignored_attrs, **add_attrs, *user_adds;

/* query: 
 *   query: BaseDN
 *   args:  adder_dn=DN&adder_pw=PW&att1=VAL1&att2=VAL2&...
 */

    args = r->r_method == POST ? r->r_postdata : r->r_filter;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "do_add (%s, %s)\n", 
        r->r_dn, args ? args : "", 0, 0);
#endif

    /* find the requested template */
    tmpl = ldap_name2template(r->r_template, r->r_access->a_tmpllist);
    if (tmpl == NULL) {            /* template not found */
        do_error(r, resp, LDAP_OTHER, BAD_REQUEST, MSG_TEMPLATE_MISSING, NULL);
        return NOTOK;
    }
    /* .. and see if it's allowed to add such an entry */
    if (!LDAP_IS_DISPTMPL_OPTION_SET(tmpl, LDAP_DTMPL_OPT_ADDABLE)) {
        do_error(r, resp, LDAP_OTHER, BAD_REQUEST, MSG_ADD_OC_NOT_ADDABLE, NULL);
        return NOTOK;
    }
    /* read what attribute should be used as RDN */
    rdnattr = tmpl->dt_defrdnattrname;
    if (!rdnattr || ! *rdnattr) {
        do_error(r, resp, LDAP_OTHER, BAD_REQUEST, MSG_ADD_NO_RDN_DEFINED, NULL);
        return NOTOK;
    }

    maxattrvals = BUFSIZ;
    user_attrs = (LDAPMod **)calloc(maxattrvals, sizeof(LDAPMod *));

    bind_as = pw = basedn = NULL;
    next = args;
    while ((cp = next) && *cp) {     /* search args */
        next = strchr(cp, '&');
        if (next)
            *next++ = '\0';
        if (strncmp(cp, "adder_dn=", 9) == 0) {
            bind_as = strdup(cp + 9);
            if ((cp = strchr(bind_as, '&')) != NULL)
                *cp = '\0';
        } else if (strncmp(cp, "adder_pw=", 9) == 0) {
            pw = strdup(cp + 9);
            if ((cp = strchr(pw, '&')) != NULL)
                *cp = '\0';
/*
        } else if (strncmp(cp, "basedn=", 7) == 0) {
            basedn = cp + 7;
 */
        } else {
            /* attribute=value */
            val = strchr(cp, '=');
            if (!(val && *(val+1)))
                continue;
            *val++ = '\0';
            attr = hex_qdecode(cp);
            if (*val && next && strncasecmp(next, "ext=", 4) == 0 &&
                *(next + 4) != '\0' && *(next + 4) != '&') {
                /* another value for this attribute, e.g. info for URI */
                *(next - 1) = ' ';
                nval = next + 4;
                while (*nval && *nval != '&')
                    *next++ = *nval++;
                *next = '\0';
                next = nval;
                *next++ = '\0';
            }
            val = hex_qdecode(val);
#if defined(OWN_STR_TRANSLATION)
            val = strdup(web500gw_isotot61(val));
#endif
            if (!val)
                continue;
            if (!*val)
                val = NULL;         /* empty value */
            for (i = 0; i < attrvals; i++) {
                if (strcasecmp(attr, user_attrs[i]->mod_type) == 0) {
                    /* already another value for this attribute */
                    for (valcount = 0; valcount < BUFSIZ-1 && 
                        user_attrs[i]->mod_values[valcount]; valcount++);
                    if (valcount >= BUFSIZ-1) {
                        i = attrvals;       /* take a new LDAPMod */
                    } else {                /* add value */
                        user_attrs[i]->mod_values[valcount] = val;
                        user_attrs[i]->mod_values[valcount+1] = NULL;
                    }
                    break;
                }
            }
            if (i >= attrvals) {   /* new attribute */
                user_adds = (LDAPMod *)calloc(1, sizeof(LDAPMod));
                user_adds->mod_type = attr;
                user_adds->mod_values = (char **)calloc(BUFSIZ, sizeof(char *));
                user_adds->mod_values[0] = val;
                user_adds->mod_values[1] = NULL;
                user_attrs[attrvals++] = user_adds;
                if (attrvals >= maxattrvals) {
                    maxattrvals += BUFSIZ;
                    user_attrs = (LDAPMod **)realloc(user_attrs, maxattrvals * sizeof(LDAPMod *));
                }
            }
        }
    }  /* end of args parsing */

    user_attrs[attrvals] = NULL;

    if (!bind_as) {
        do_error(r, resp, LDAP_OTHER, BAD_REQUEST, MSG_NO_BIND_DN, NULL);
        return NOTOK;
    }
    hex_qdecode(bind_as);
    r->r_binddn = bind_as;
    if (! pw) {
        do_error(r, resp, LDAP_OTHER, BAD_REQUEST, MSG_MISSING_PASSWORD, NULL);
        return NOTOK;
    }
    hex_qdecode(pw);

    add_attrs     = (LDAPMod **)calloc(BUFSIZ, sizeof(LDAPMod *));
    ignored_attrs = (LDAPMod **)calloc(BUFSIZ, sizeof(LDAPMod *));
    maxattrvals = BUFSIZ;
    attrvals = 0;

    /* read constant default values */
    for (defaults = tmpl->dt_adddeflist; defaults;
         defaults = defaults->ad_next) {
        if (defaults->ad_source == LDAP_ADSRC_ADDERSDN) {
            defaults->ad_value = r->r_binddn;
        } else if ( !defaults->ad_attrname || !defaults->ad_value ||
            strncmp(defaults->ad_value, "edit:", 5) == 0) {
            /* constant value starting with "edit:" we allow to change */
            continue;
        } else if (strncmp(defaults->ad_value, "change:", 7) == 0) {
            /* constant value starting with "change:" must be changed */
            continue;
        }
        for (i = 0; i < attrvals; i++) {
            if (strcasecmp(defaults->ad_attrname, add_attrs[i]->mod_type) 
                    == 0) {
                /* already another value for this attribute */
                for (valcount = 0; valcount < BUFSIZ-1 && 
                    add_attrs[i]->mod_values[valcount]; valcount++);
                if (valcount >= BUFSIZ-1) {
                    i = attrvals;       /* take a new LDAPMod */
                } else {                /* add value */
                    add_attrs[i]->mod_values[valcount] = defaults->ad_value;
                    add_attrs[i]->mod_values[valcount+1] = NULL;
                }
                break;
            }
        }
        if (i >= attrvals) {   /* new attribute */
            user_adds = (LDAPMod *)calloc(1, sizeof(LDAPMod));
            user_adds->mod_type = defaults->ad_attrname;
            user_adds->mod_values = (char **)calloc(BUFSIZ, sizeof(char *));
            user_adds->mod_values[0] = defaults->ad_value;
            user_adds->mod_values[1] = NULL;
            add_attrs[attrvals] = user_adds;
            attrvals++;
            if (attrvals >= maxattrvals) {
                maxattrvals += BUFSIZ;
                user_attrs = (LDAPMod **)realloc(user_attrs, maxattrvals * sizeof(LDAPMod *));
            }
        }
        
    }

    /* now see if users attributes are ok with template */
    for (rowp = ldap_first_tmplrow(tmpl); rowp != NULLTMPLITEM;
         rowp = ldap_next_tmplrow(tmpl, rowp)) {
         for (colp = ldap_first_tmplcol(tmpl, rowp); colp != NULLTMPLITEM;
              colp = ldap_next_tmplcol(tmpl, rowp, colp)) {
            if (! colp->ti_attrname)
                continue;
            found = 0;
            for (i = 0; user_attrs[i] != NULL; i++) {
                if (strcasecmp(user_attrs[i]->mod_type, colp->ti_attrname) != 0)
                    continue;     /* another attribute */
                found = 1;       /* found attribute */

                /* an unchanged default value starting with "change:" ? */
                for (defaults = tmpl->dt_adddeflist; defaults;
                    defaults = defaults->ad_next) {
                    if (strcasecmp(user_attrs[i]->mod_type, defaults->ad_attrname) != 0)
                        continue;
                    if (defaults->ad_source == LDAP_ADSRC_CONSTANTVALUE
                        && strncmp(defaults->ad_value, "change:", 7) == 0
                        && strcmp(defaults->ad_value + 7, user_attrs[i]->mod_values[0]) == 0) {
                    /* unchanged constant value starting with "change:" */
                        found = 0;      /* ignoring */
                        break;
                    }

                }
                if (! found)
                    continue;

                /* is the RDN attribute ? */
                if (!rdn && strcasecmp(colp->ti_attrname, rdnattr) == 0)
                    rdn = user_attrs[i]->mod_values[0];

                if (colp->ti_syntaxid == LDAP_SYN_MULTILINESTR) {
                    for (j = 0; user_attrs[i]->mod_values[j]; j++) {
                        cp = user_attrs[i]->mod_values[j];
                        while (*cp++) {
                            if (*cp == '\r') *cp = '$';
                            if (*cp == '\n') *cp = ' ';
                        }
                    }
                }

                if (LDAP_IS_TMPLITEM_OPTION_SET(colp, LDAP_DITEM_OPT_SINGLEVALUED)) {
                    if (user_attrs[i]->mod_values[1] != NULL) {
                        /* ignoring more values for 1val */
                        user_attrs[i]->mod_values[1] = NULL;
                    }
                }

                add_attrs[attrvals++] = user_attrs[i]; /* we are adding this */
                if (attrvals >= maxattrvals) {
                    maxattrvals += BUFSIZ;
                    add_attrs = (LDAPMod **)realloc(add_attrs, maxattrvals * sizeof(LDAPMod *));
                }
            }
            if (!found && 
                LDAP_IS_TMPLITEM_OPTION_SET(colp, LDAP_DITEM_OPT_VALUEREQUIRED)) {
                /* no attribute value given - but it's required! */
#ifdef WEB500GW_DEBUG
                Web500gw_debug(WEB500GW_DEBUG_TRACE, 
                    "required attribute %s missing!", colp->ti_attrname, 
                    0, 0, 0);
#endif
                error = calloc(strlen(MSG_ADD_MISSING_REQ_VALUE) + 
                            strlen(colp->ti_attrname) + 10, sizeof(char));
                sprintf(error, MSG_ADD_MISSING_REQ_VALUE,
                        colp->ti_attrname);

                do_error(r, resp, LDAP_OTHER, BAD_REQUEST, error, NULL);
                free(error);
                return NOTOK;
            }
        }
    }               /* end of template processing */

    if (!rdn || ! *rdn) {
        do_error(r, resp, LDAP_OTHER, BAD_REQUEST, MSG_ADD_NO_RDN, NULL);
        return NOTOK;
    }

    if (basedn && *basedn) {
        hex_qdecode(basedn);
    } else {
        basedn = r->r_dn;
    }
    dn = calloc(strlen(rdnattr) + strlen(rdn) + strlen(basedn) + 4, sizeof(char));
    sprintf(dn, "%s=%s, %s", rdnattr, rdn, basedn);

    ufn = friendly_dn(resp, dn);
    rdn = friendly_rdn(resp, dn, 1);
    in_home = isinhome(dn);

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "ADD %s:\n", dn, 0, 0, 0);
    for (i = 0; add_attrs[i]; i++) {
        Web500gw_debug(WEB500GW_DEBUG_TRACE, "  %s:", 
            add_attrs[i]->mod_type, 0, 0, 0);
        for (j = 0; add_attrs[i]->mod_values[j]; j++) {
            Web500gw_debug(WEB500GW_DEBUG_TRACE, "    %s\n",
            add_attrs[i]->mod_values[j], 0, 0, 0);
        }
    }
#endif


#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "BINDING as %s ... ", bind_as, 0, 0, 0);
#endif
    if ((r->r_ld = web500gw_ldap_init(r, resp, bind_as, pw, 1)) == (LDAP *)0)
        return NOTOK;
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "BOUND\n", 0, 0, 0, 0);
#endif
 
    if (r->r_httpversion == 1 && resp->resp_httpheader == 0) {
        http_header(r, resp);
        if (r->r_method == HEAD)
            return OK;
    }
    if (resp->resp_htmlheader == 0) {
        msg_fprintf(fp, in_home && *MSG_HTML_START_MISC_HOME ?
            MSG_HTML_START_MISC_HOME : MSG_HTML_START_MISC, 
            "sss", MSG_ADD_RESULTS, rdn, ufn);
        resp->resp_htmlheader = 1;
    }
    msg_fprintf(fp, MSG_ADD_RESULTS_FOR, "ss", rdn, ufn);

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "ldap_add_s (%s)\n", dn, 0, 0, 0);
#endif

    if ((rc = ldap_add_s(r->r_ld, dn, add_attrs) != LDAP_SUCCESS)) {
        /* oh shit */
#ifdef WEB500GW_DEBUG
            if (web500gw_debug)
                ldap_perror(r->r_ld, "ldap_add_s");
#endif
        msg_fprintf(fp, MSG_ADD_ERROR, "ssiss", rdn, ufn, rc,
            web500gw_err2string(rc, resp), 
            web500gw_err2string(r->r_ld->ld_errno, resp));
        fputs(MSG_HTML_END, fp);
        fputs("\n", fp);
        
        resp->resp_error = rc;
        resp->resp_status = BAD_REQUEST;
        return NOTOK;
    }
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "Ok\n", 0, 0, 0, 0);
#endif

    msg_fprintf(fp, MSG_ADD_OK, "sss", rdn, ufn, 
        dn2url(r, dn, FLAG_LANGUAGE|FLAG_NOCACHE, 0, NULL, NULL));
    fputs(MSG_HTML_END, fp);
    fputs("\n", fp);

    free(ufn); free(rdn); free(dn);
    for (i = 0; user_attrs[i]; i++) {
        if (add_attrs[i]->mod_values)
            free(add_attrs[i]->mod_values);
    }
    free(user_attrs); free(add_attrs); free(ignored_attrs);

    return OK;

#endif
}

