/*
 * web500gw:  A LDAP based WWW - X.500 gateway
 *
 * written by: Frank Richter, Frank.Richter@hrz.tu-chemnitz.de
 *             Chemnitz University of Technology, Germany
 *
 * Copyright (c) 1994-8 Chemnitz University of Technology.
 * All rights reserved.
 *
 * $Id: modify.c,v 1.3 2001/04/26 22:16:53 dekarl Exp $
 */

#include "web500gw.h"

int
do_form(
    REQUEST     *r,
    RESPONSE    *resp
)
{
#ifndef MODIFY
    do_error(r, resp, LDAP_OTHER, NOT_IMPLEMENTED, MSG_NOT_SUPPORTED, NULL);
    return NOTOK;
#else   /* defined MODIFY */

    int             rc, i = 0, in_home;
    char            *dn, *bind_as, *pw, *args, *attr, *rdn, *ufn, *cp;
    char            **ocl, **search_attrs;
    LDAPMessage     *res, *e;
    struct timeval  timeout;
    struct ldap_disptmpl    *tmpl;

/* OLD: query = DN?userPassword=PASSWD
 * NEW: query = DN, args = dn=Bind_DN&userPassword=PASSWD[&attrs=...]
 */
    dn = r->r_dn;
    args = r->r_method == POST ? r->r_postdata : r->r_filter;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "do_form (%s, %s)\n",    
        r->r_dn, args ? args : "", 0, 0);
#endif

    if (args == NULL) {     /* OLD style */
        bind_as = dn;
        if ((pw = strchr(dn, '?')) == NULL) {
            do_error(r, resp, LDAP_OTHER, BAD_REQUEST, NULL, NULL);
            return NOTOK;
        }
        *pw++ = '\0';
        if (strncmp(pw, "userPassword=", 13) == 0) {
            pw += 13;
        } else {
            do_error(r, resp, LDAP_INVALID_CREDENTIALS, 0, MSG_UNKNOWN_ARGUMENT, NULL);
            return NOTOK;
        }
    } else {                /* NEW style */
        bind_as = pw = attr = NULL;
        cp = args;
        while (cp && *cp) {
            if (strncmp(cp, "dn=", 3) == 0) {
                bind_as = cp + 3;
            } else if (strncmp(cp, "userPassword=", 13) == 0) {
                pw = cp + 13;
            } else if (strncmp(cp, "attrs=", 6) == 0) {
                attr = cp + 6;
            } else {
                do_error(r, resp, LDAP_OTHER, BAD_REQUEST, MSG_UNKNOWN_ARGUMENT, NULL);
                return NOTOK;
            }
            cp = strchr(cp, '&');
            if (cp && *cp)
                *cp++ = '\0';
        }
        if (bind_as) {
            hex_qdecode(bind_as);
        } else {
            bind_as = dn;
        }
        if (! pw) {
            do_error(r, resp, LDAP_OTHER, BAD_REQUEST, MSG_MISSING_PASSWORD, NULL);
            return NOTOK;
        }
        hex_qdecode(pw);

        if (attr && *attr) {    /* additional r->r_attrs from HTML form */
            hex_qdecode(attr);
            r->r_flags |= FLAG_ATTRSONLY;
            r->r_attrs[r->r_attrnumb++] = attr;
            while ((attr = strchr(attr, ',')) != NULL) {
                *attr++ = '\0';
                r->r_attrs[r->r_attrnumb++] = attr;
            }
            r->r_attrs[r->r_attrnumb] = NULL;
        }
    }

    if ((!bind_as) || strlen(bind_as) == 0) { /* No DN to bind as */
        do_error (r, resp, LDAP_INAPPROPRIATE_AUTH, BAD_REQUEST, MSG_UNKNOWN_ARGUMENT, NULL);
        return NOTOK;
    }
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

    /* what attributes wanted to modify */
    search_attrs = NULL;
    if (r->r_attrnumb > 0) {
        /* user wants to modify only these attributes ... */
        search_attrs = (char **) calloc(r->r_attrnumb + 2, sizeof(char *));
        i = 0;
        while (r->r_attrs[i]) {
            search_attrs[i] = r->r_attrs[i];
            i++;
        }
        search_attrs[i++] = objectclass_attr;
        search_attrs[i] = NULL;

#ifdef WEB500GW_DEBUG
        Web500gw_debug(WEB500GW_DEBUG_TRACE, "  looking for attrs: ",
             0, 0, 0, 0);
        i = 0;
        while (search_attrs && search_attrs[i]) {
            Web500gw_debug(WEB500GW_DEBUG_TRACE, "%s, ", 
                search_attrs[i], 0, 0, 0);
            i++;
        }
        Web500gw_debug(WEB500GW_DEBUG_TRACE, "\n", 0, 0, 0, 0);
#endif
    } 

    timeout.tv_sec = timelimit;
    timeout.tv_usec = 0;
    if ((rc = ldap_search_st(r->r_ld, dn, LDAP_SCOPE_BASE, default_filter,
        search_attrs, 0, &timeout, &res)) != LDAP_SUCCESS) {
        do_error(r, resp, rc, 0, get_ldap_error_str(r->r_ld), get_ldap_matched_str(r->r_ld));
        return NOTOK;
    }
    if ((e = ldap_first_entry(r->r_ld, res)) == NULL) {
        do_error(r, resp, get_ldap_result_code(r->r_ld), 0, NULL, NULL);
        return NOTOK;
    }
    dn = ldap_get_dn(r->r_ld, e);
    /* dn = r->r_dn; */
    ufn = friendly_dn(resp, dn);
    rdn = friendly_rdn(resp, dn, 1);
    in_home = isinhome(dn);

    if (r->r_httpversion == 1 && resp->resp_httpheader == 0)
        http_header(r, resp);
    if (resp->resp_htmlheader == 0) {
        msg_fprintf(fp, in_home && *MSG_HTML_START_MISC_HOME ?
            MSG_HTML_START_MISC_HOME : MSG_HTML_START_MISC, "sss",
            MSG_MODIFY, rdn, ufn);
        resp->resp_htmlheader = 1;
    }
    msg_fprintf(fp, MSG_EXPLAIN_MODIFY, "ss", rdn, ufn);
    fprintf(fp, "<FORM METHOD=POST ACTION=\"%s\">\n", 
        dn2url(r, dn, FLAG_LANGUAGE|FLAG_TMPL, ACTION_MODIFY, NULL, NULL));

    if (r->r_browser->b_opts & B_HIDDEN) {
        fprintf(fp, "<INPUT TYPE=\"hidden\" NAME=\"dn\" VALUE=\"%s\">\n\
<INPUT TYPE=\"hidden\" NAME=\"pw\" VALUE=\"%s\">\n",
		html_encode(bind_as, strlen(bind_as)), html_encode(pw, strlen(pw)));

    } else {
        fprintf(fp, "<INPUT TYPE=\"radio\" NAME=\"dn\" VALUE=\"%s\" CHECKED>\n\
<INPUT TYPE=\"radio\" NAME=\"pw\" VALUE=\"%s\" CHECKED>\n",
		html_encode(bind_as, strlen(bind_as)), html_encode(pw, strlen(pw)));
    }


    if (r->r_attrnumb == 1 && strcasecmp(r->r_attrs[0], "userPassword") == 0) {
        /* Special case - Change password */
        fputs(MSG_CHANGE_PASSWORD, fp);
    } else {
        fputs(MSG_HELP_MODIFY, fp);
        /* print the form with attributes */
        if (!(r->r_flags & FLAG_LAYOUT)) { 
            r->r_flags |= r->r_browser->b_flags;
        }
        r->r_actions |= ACTION_FORM;
        if (r->r_template) {
            tmpl = ldap_name2template(r->r_template, r->r_access->a_tmpllist);
        } else {
            ocl = ldap_get_values(r->r_ld, e, objectclass_attr);
            tmpl = ldap_oc2template(ocl, r->r_access->a_tmpllist);
        }
        if (tmpl == NULL) {            /* template not found */
            do_error(r, resp, LDAP_OTHER, BAD_REQUEST, MSG_TEMPLATE_MISSING, NULL);
            return NOTOK;
        }

        if ((rc =  web500gw_entry2html(r->r_ld, r, resp, e, tmpl, NULL,
                 NULL, 0)) != LDAP_SUCCESS) {
                do_error(r, resp, rc, 0, NULL, NULL);
                return NOTOK;
        }
    }
    fprintf(fp, "\n%s</FORM>\n%s\n", MSG_MODIFY_ACTION, MSG_HTML_END);
    free(rdn);
    free(ufn);
    return OK;
#endif
}


/*
 * do_modify: Performs an LDAP MODIFY request:
 */

int
do_modify(
    REQUEST     *r,
    RESPONSE    *resp
)
{

#ifndef MODIFY
    do_error(r, resp, LDAP_OTHER, NOT_IMPLEMENTED, MSG_NOT_SUPPORTED, NULL);
    return NOTOK;
#else   /* defined MODIFY */

    char    *dn, *bind_as, *args, *change_pw = NULL; 
    char    *ufn, *rdn, *pw, *next, *val, *nval, *cp, *attr, *error;
    char    **vals;
    int     rc, attrvals = 0, maxattrvals, i, j, valcount, found, differs;
    int     changes = 0, in_home;
    LDAPMessage     *res, *e;
    BerElement      *ber;
    struct timeval  timeout;
    struct ldap_disptmpl    *tmpl;
    struct ldap_tmplitem    *rowp, *colp;
    LDAPMod  **user_attrs, **mod_attrs, *user_mods;

/* query: 
 *   query: DN
 *   args:  dn=DN&oldPassword=OLDPW&att1=VAL1&att2=VAL2&...
 */

    dn = r->r_dn;
    args = r->r_method == POST ? r->r_postdata : r->r_filter;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "do_modify (%s, %s)\n", 
        r->r_dn, args ? args : "", 0, 0);
#endif

    /* find the requested template */
    if (r->r_template == NULL || 
       (tmpl = ldap_name2template(r->r_template, r->r_access->a_tmpllist)) == NULL) {
        /* template not found */
        do_error(r, resp, LDAP_OTHER, BAD_REQUEST, MSG_TEMPLATE_MISSING, NULL);
        return NOTOK;
    }

    maxattrvals = BUFSIZ;
    user_attrs = (LDAPMod **)calloc(maxattrvals, sizeof(LDAPMod *));

    bind_as = pw = NULL;
    next = args;
    while ((cp = next) && *cp) {     /* search args */
        next = strchr(cp, '&');
        if (next)
            *next++ = '\0';
        if (strncmp(cp, "dn=", 3) == 0) {
            bind_as = cp + 3;
        } else if (strncmp(cp, "pw=", 3) == 0) {
            pw = cp + 3;
        } else {
            /* attribute=value */
            val = strchr(cp, '=');
            if (!val)
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
                    if (maxvalues > 0 && valcount >= maxvalues) {
                        /* more than max values - ignore */
                        break;
                    }
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
                user_mods = (LDAPMod *)calloc(1, sizeof(LDAPMod));
                user_mods->mod_type = attr;
                user_mods->mod_values = (char **)calloc(BUFSIZ, sizeof(char *));
                user_mods->mod_values[0] = val;
                user_mods->mod_values[1] = NULL;
                user_attrs[attrvals++] = user_mods;
                if (attrvals >= maxattrvals) {
                    maxattrvals += BUFSIZ;
                    user_attrs = (LDAPMod **)realloc(user_attrs, maxattrvals * sizeof(LDAPMod *));
                }
            }
        }
    }  /* end of args parsing */

    user_attrs[attrvals] = NULL;

    /* see if user attrs are ok with template, 
     * e.g. don't allow changes for read only values */
    for (rowp = ldap_first_tmplrow(tmpl); rowp != NULLTMPLITEM;
         rowp = ldap_next_tmplrow(tmpl, rowp)) {
         for (colp = ldap_first_tmplcol(tmpl, rowp); colp != NULLTMPLITEM;
              colp = ldap_next_tmplcol(tmpl, rowp, colp)) {
            if (! colp->ti_attrname)
                continue;
            found = 0;
            for (i = 0; user_attrs[i] != NULL; i++) {
                if (user_attrs[i] == MOD_ATTR_USED)
                    continue;       /* already processed */
                if (strcasecmp(user_attrs[i]->mod_type, colp->ti_attrname) != 0)
                    continue;     /* another attribute */
                found = 1;       /* found attribute */
                if (LDAP_IS_TMPLITEM_OPTION_SET(colp, LDAP_DITEM_OPT_READONLY)) {
                    /* readonly value - set attribute to NULL - don't change */
                    user_attrs[i]->mod_type = NULL;
                    continue;
                }
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
                if ((user_attrs[i]->mod_values[0] == NULL ||
                    *user_attrs[i]->mod_values[0] == '\0') &&
                    LDAP_IS_TMPLITEM_OPTION_SET(colp, LDAP_DITEM_OPT_VALUEREQUIRED)) {
                    /* no attribute value given - but it's required! */
#ifdef WEB500GW_DEBUG
                Web500gw_debug(WEB500GW_DEBUG_TRACE,
                    "Required attribute %s missing!", colp->ti_attrname,
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
        }
    }
    for (i = 0; user_attrs[i]; i++) {         /* check if all user attrs */
        if (user_attrs[i] == MOD_ATTR_USED)  /* are in template */
            continue;       /* already processed */
        found = 0;
        if (strcasecmp(user_attrs[i]->mod_type, "userPassword") == 0) {
            /* special case - change password */
             if (!user_attrs[i]->mod_values[0] ||
                 strlen(user_attrs[i]->mod_values[0]) < 1) {
                 /* Don't allow null passwords */
                 do_error(r, resp, LDAP_OTHER, BAD_REQUEST,
                    MSG_NULL_PASSWORD, NULL);
                 return NOTOK;
             }
             for (j = 0; user_attrs[j]; j++) {
                /* look for "verify_userPassword" and see if values are equal */
                if (strcasecmp(user_attrs[j]->mod_type, "verify_userPassword") 
                    == 0) {
                    if (strcmp(user_attrs[i]->mod_values[0], 
                               user_attrs[j]->mod_values[0]) != 0) {
                        do_error(r, resp, LDAP_OTHER, BAD_REQUEST,
                            MSG_PASSWORD_VERIFY, NULL);
                        return NOTOK;
                    }
                    user_attrs[j] = MOD_ATTR_USED;
                    change_pw = user_attrs[i]->mod_values[0];
                    break;
                }
            }
            continue;
        }
        for (rowp = ldap_first_tmplrow(tmpl); rowp != NULLTMPLITEM && !found;
             rowp = ldap_next_tmplrow(tmpl, rowp)) {
            for (colp = ldap_first_tmplcol(tmpl, rowp); colp != NULLTMPLITEM;
                colp = ldap_next_tmplcol(tmpl, rowp, colp)) {
                if (! colp->ti_attrname)
                    continue;
                if (strcasecmp(user_attrs[i]->mod_type, colp->ti_attrname) == 0) { 
                    found = 1;
                    break;
                }
            }
        }
        if (!found)
            user_attrs[i] = MOD_ATTR_USED;
    }               /* end of template processing */

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "USER MOD %s:\n", dn, 0, 0, 0);
    for (i = 0; user_attrs[i]; i++) {
        if (user_attrs[i] == MOD_ATTR_USED || 
            user_attrs[i]->mod_values[0] == NULL)
            continue;
        Web500gw_debug(WEB500GW_DEBUG_TRACE, "  %s:",
            user_attrs[i]->mod_type, 0, 0, 0);
        for (j = 0; user_attrs[i]->mod_values[j]; j++) {
            Web500gw_debug(WEB500GW_DEBUG_TRACE, "    .%s.\n",
            user_attrs[i]->mod_values[j], 0, 0, 0);
        }
    }
#endif

    mod_attrs = (LDAPMod **)calloc(BUFSIZ, sizeof(LDAPMod *));
    maxattrvals = BUFSIZ;
    attrvals = 0;

    /* we need to bind as the user */
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

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "BINDING as %s ... ", bind_as, 0, 0, 0);
#endif
    if ((r->r_ld = web500gw_ldap_init(r, resp, bind_as, pw, 1)) == (LDAP *)0)
        return NOTOK;
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "BOUND\n", 0, 0, 0, 0);
#endif

    /* and read the existing entry */
    timeout.tv_sec = timelimit;
    timeout.tv_usec = 0;
    if ((rc = ldap_search_st(r->r_ld, dn, LDAP_SCOPE_BASE, default_filter,
        NULL, 0, &timeout, &res)) != LDAP_SUCCESS) {
        /* better error description here ??? */
        do_error(r, resp, rc, 0, get_ldap_error_str(r->r_ld), get_ldap_matched_str(r->r_ld));
        return NOTOK;
    }
    if ((e = ldap_first_entry(r->r_ld, res)) == NULL) {
        do_error(r, resp, get_ldap_result_code(r->r_ld), 0, get_ldap_error_str(r->r_ld), NULL);
        return NOTOK;
    }

    ber = NULL;
    for (attr = ldap_first_attribute(r->r_ld, e, &ber); attr != NULL;
         attr = ldap_next_attribute(r->r_ld, e, ber)) {
        found = 0;
        /* ignore if attr not in user request */
        for (i = 0; user_attrs[i] != NULL; i++) {
            if (user_attrs[i] == MOD_ATTR_USED)
                continue;       /* already processed */
            if (user_attrs[i]->mod_type &&
                strcasecmp(user_attrs[i]->mod_type, attr) == 0) {
                found = 1;
                break;
            }
        }
        if (!found)
            continue;
        if (user_attrs[i]->mod_values[0] == NULL) {
            /* no new values: DELETE all values */
            user_attrs[i]->mod_op = LDAP_MOD_DELETE;
            mod_attrs[attrvals++] = user_attrs[i];
            user_attrs[i] = MOD_ATTR_USED;     /* mark as processed */

        } else if ((vals = ldap_get_values(r->r_ld, e, attr)) != NULL) {
            /* compare new with old values */
            differs = 0;
            /* ldap_sort_values(r->r_ld, vals, ldap_sort_strcasecmp); */
            /* ldap_sort_values(r->r_ld, user_attrs[i]->mod_values,  */
            /*         ldap_sort_strcasecmp); */
            for (j = 0; user_attrs[i]->mod_values[j] != NULL; j++) {
                if (!vals[j] ||
                    strcmp(vals[j], user_attrs[i]->mod_values[j]) != 0) {
                    differs = 1;
                    break;
                }
            }
            if (vals[j] || differs) {   /* values differ: REPLACE */
                user_attrs[i]->mod_op = LDAP_MOD_REPLACE;
                mod_attrs[attrvals++] = user_attrs[i];
            }
            user_attrs[i] = MOD_ATTR_USED;     /* mark as processed */
        }
    }
    for (i = 0; user_attrs[i] != NULL; i++) {
        if (user_attrs[i] != MOD_ATTR_USED &&
            user_attrs[i]->mod_values[0] != NULL) {
        /* no old values: ADD new values */
            user_attrs[i]->mod_op = LDAP_MOD_ADD;
            mod_attrs[attrvals++] = user_attrs[i];
        }
    }

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "MODIFY %s:\n", dn, 0, 0, 0);
#endif

    for (i = 0; mod_attrs[i]; i++) {
#ifdef WEB500GW_DEBUG
        Web500gw_debug(WEB500GW_DEBUG_TRACE, " %s %s:",
            mod_attrs[i]->mod_op == LDAP_MOD_REPLACE ? "REPLACE" :
            mod_attrs[i]->mod_op == LDAP_MOD_DELETE  ? "DELETE " :
            mod_attrs[i]->mod_op == LDAP_MOD_ADD     ? "ADD    " : "UNKNOWN",
            mod_attrs[i]->mod_type, 0, 0);
        for (j = 0; mod_attrs[i]->mod_values[j]; j++) {
            Web500gw_debug(WEB500GW_DEBUG_TRACE, "    %s\n",
            mod_attrs[i]->mod_values[j], 0, 0, 0);
        }
        Web500gw_debug(WEB500GW_DEBUG_TRACE, "\n", 0, 0, 0, 0);
#endif
    }
    changes = i;

    if (r->r_httpversion == 1 && resp->resp_httpheader == 0) {
        http_header(r, resp);
        if (r->r_method == HEAD)
            return NOTOK;
    }
    ufn = friendly_dn(resp, dn);
    rdn = friendly_rdn(resp, dn, 1);
    in_home = isinhome(dn);

    if (resp->resp_htmlheader == 0) {
        msg_fprintf(fp, in_home && *MSG_HTML_START_MISC_HOME ?
            MSG_HTML_START_MISC_HOME : MSG_HTML_START_MISC, "sss",
            MSG_MODIFY_RESULTS, rdn, ufn);
        resp->resp_htmlheader = 1;
    }
    msg_fprintf(fp, MSG_MODIFY_RESULTS_FOR, "ss", rdn, ufn);
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "ldap_modify_s (%s)\n", dn, 0, 0, 0);
#endif

    if ((rc = ldap_modify_s(r->r_ld, dn, mod_attrs)) != LDAP_SUCCESS) {
#ifdef WEB500GW_DEBUG
        if (web500gw_debug) 
            ldap_perror(r->r_ld, "ldap_modify_s");
#endif
        msg_fprintf(fp, MSG_MODIFY_ERROR, "iss", rc,
            web500gw_err2string(rc, resp), 
            get_ldap_error_str(r->r_ld) ? get_ldap_error_str(r->r_ld) : "");
        fputs(MSG_HTML_END, fp);
        fputs("\n", fp);

        resp->resp_error = rc;
        resp->resp_status = BAD_REQUEST;
        return NOTOK;
    }
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "Ok\n", 0, 0, 0, 0);
#endif

    for (i = 0; mod_attrs[i]; i++) {
        msg_fprintf(fp, MSG_MODIFY_REPORT, "sss",
        friendly_label(resp, mod_attrs[i]->mod_type),
        change_pw ? "" : 
        mod_attrs[i]->mod_op == LDAP_MOD_DELETE ? "" :
		    html_encode(mod_attrs[i]->mod_values[0], strlen(mod_attrs[i]->mod_values[0])),
        mod_attrs[i]->mod_op == LDAP_MOD_REPLACE ? MSG_REPLACED :
        mod_attrs[i]->mod_op == LDAP_MOD_DELETE  ? MSG_DELETED  :
        mod_attrs[i]->mod_op == LDAP_MOD_ADD     ? MSG_ADDED    : "UNKNOWN");
    }
    if (changes) {
        msg_fprintf(fp, MSG_MODIFY_SUMMARY, "is",
            changes, changes == 1 ? MSG_CHANGE : MSG_CHANGES);
    } else {
        fputs(MSG_NO_CHANGES, fp);
    }
    if (change_pw && strcasecmp(bind_as, dn) == 0)
        pw = change_pw;
    msg_fprintf(fp, MSG_AFTER_MODIFY, "ssss",
        dn2url(r, dn, FLAG_LANGUAGE|FLAG_NOCACHE, 0, NULL, NULL),
        dn2url(r, dn, FLAG_LANGUAGE|FLAG_TMPL, ACTION_FORM, 
        change_pw ? "userPassword" : NULL, NULL),  
	html_encode(bind_as, strlen(bind_as)), html_encode(pw, strlen(pw)));
    fputs(MSG_HTML_END, fp);
    fputs("\n", fp);
    free(ufn);
    free(rdn);
    return OK;

#endif
}

