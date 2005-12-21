/*
 * web500gw:  A LDAP based WWW - X.500 gateway
 *
 * written by: Frank Richter, Frank.Richter@hrz.tu-chemnitz.de
 *             Chemnitz University of Technology, Germany
 *
 * Copyright (c) 1994-8 Chemnitz University of Technology.
 * All rights reserved.
 *
 * $Id: bind.c,v 1.2 2001/04/26 22:16:53 dekarl Exp $
 */

#include "web500gw.h"


int
do_bind(
    REQUEST     *r,
    RESPONSE    *resp
)
{
#ifndef MODIFY
    do_error(r, resp, LDAP_OTHER, NOT_IMPLEMENTED, MSG_NOT_SUPPORTED, NULL);
    return NOTOK;
#else   /* defined MODIFY */

    char    *ufn, *dn, *rdn, *trdn, *cp, **binddns = NULL, *s, **dns, *auth_form;
    int     in_home, rc, dncount, j, k, max;
    unsigned int    flag;
    LDAPMessage *res, *e = (LDAPMessage *)0;
    struct timeval    timeout;
    struct ldap_disptmpl    *tmpl;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "do_bind (%s, 0x%x = %s)\n", r->r_dn,
        r->r_flags, flag2string(r->r_flags), 0);
#endif

    if ((dn = strchr(r->r_dn, '?')) != NULL)
        *dn = '\0';
    dn = r->r_dn;
    ufn = friendly_dn(resp, dn);
    rdn = friendly_rdn(resp, dn, 1);
    trdn = friendly_rdn(resp, dn, 0);

    in_home = isinhome(dn);

    /* find the requested template */
    if (r->r_template == NULL ||
       (tmpl = ldap_name2template(r->r_template, r->r_access->a_tmpllist)) == NULL) {
        /* template not found */
        do_error(r, resp, LDAP_OTHER, BAD_REQUEST, "Template not found", "");
        return NOTOK;
    }

    max = BUFSIZ;
    binddns = (char **) calloc(max, sizeof(char *));
    dncount = 0;
    /* Bind as ... ? */
    if (tmpl->dt_authattrname != NULL) {
        if (strcasecmp(tmpl->dt_authattrname, "self") == 0) {
            binddns[dncount++] = dn;        /* bind as entry itself */
        } else {    /* read the auth attribute(s) */
            r->r_attrnumb = 0;
            cp = tmpl->dt_authattrname;
            while (cp && *cp) {
                s = strchr(cp, '|');
                if (s && *s)
                    *s++ = '\0';
                if (strncmp(cp, "dn:", 3) == 0) {
                    binddns[dncount++] = cp + 3;      /* given DN */
                } else if (strncmp(cp, "self", 4) == 0) {
                    binddns[dncount++] = dn;          /* DN of entry itself */
                } else {                        /* DN in attribute */
                    r->r_attrs[r->r_attrnumb++] = cp;
                }
                if (dncount + 1 > max) {
                    max += BUFSIZ;
                    binddns = (char **) realloc(binddns, max);
                }
                cp = s;
            }
        }
        r->r_attrs[r->r_attrnumb] = NULL;

        dns = NULL;

        if (r->r_attrnumb > 0) {
#ifdef WEB500GW_DEBUG
            if (web500gw_debug & WEB500GW_DEBUG_PARSE) {
                fprintf(stderr, "  reading DN in attrs: ");
                for (k = 0; k < r->r_attrnumb; k++) {
                    fprintf(stderr, "%s ", r->r_attrs[k]);
                }
                fprintf(stderr, "\n");
            }
#endif
            if (r->r_ld == (LDAP *)0 &&
               (r->r_ld = web500gw_ldap_init(r, resp, NULL, NULL, 1)) == (LDAP *)0) {
                return NOTOK;
            }

            timeout.tv_sec = timelimit;
            timeout.tv_usec = 0;
            if ((rc = ldap_search_st(r->r_ld, r->r_dn, LDAP_SCOPE_BASE,
                default_filter, r->r_attrs, 0, &timeout, &res)) != LDAP_SUCCESS
                || (e = ldap_first_entry(r->r_ld, res)) == NULL) {
                /* ignore ?? */
            } else {
                for (k = 0;  k < r->r_attrnumb; k++) {
                    dns = ldap_get_values(r->r_ld, e, r->r_attrs[k]);
                    if (dns && *dns) {
                        j = 0;
                        while (dns[j++] != NULL);       /* count read values */
                        if (dncount + j + 1 > max) {
                            max += BUFSIZ;
                            binddns = (char **) realloc(binddns, max);
                        }
                        j = 0;
                        while (dns[j]) {
                            binddns[dncount++] = dns[j++];
                        }
                    }
                }
            }
        }
    }
    binddns[dncount] = NULL;

    max = BUFSIZ;
    if (tmpl->dt_authattrname == NULL || dncount == 0) {     
        /* No auth DNs found, use baseDN itself in an input field */
        auth_form = calloc(max, sizeof(char));
        sprintf(auth_form, "<INPUT SIZE=\"%d\" NAME=\"dn\" VALUE=\"%s\">\n",
                    strlen(dn) + 3, html_encode(dn));
    } else {
        cp = auth_form = calloc(max, sizeof(char));
        j = 0;
        strcpy(auth_form, "<SELECT NAME=\"dn\">\n");
        cp += strlen(cp);
        while (j < dncount) {
            if (strlen(auth_form) + 3 * strlen(binddns[j]) + 20 > max) {
                max += BUFSIZ;
                auth_form = realloc(auth_form, max);
            }
            s = html_encode(binddns[j]);
            sprintf(cp, "<OPTION VALUE=\"%s\">%s\n", s, s);
            free(s);
            j++;
            cp += strlen(cp);
        }
        if (strlen(auth_form) + 11 > max) {
            max += BUFSIZ;
            auth_form = realloc(auth_form, max);
        }
        strcat(auth_form, "</SELECT>\n");
    }

    if (resp->resp_httpheader == 0 && r->r_httpversion == 1) {
        http_header(r, resp);
        if (r->r_method == HEAD)
            return OK;
    }
    if (resp->resp_htmlheader == 0) {
        msg_fprintf(fp, in_home && *MSG_HTML_START_MISC_HOME ?
            MSG_HTML_START_MISC_HOME : MSG_HTML_START_MISC, 
            "sss", MSG_ASK_FOR_BIND, rdn, ufn);
        resp->resp_htmlheader = 1;
    }
   
    s = NULL;

    if (r->r_actions & ACTION_ADDFORM) {
        cp = MSG_BIND_ADD_FORM;
        flag = ACTION_ADDFORM;
    } else if (r->r_actions & ACTION_DELETEFORM) {
        cp = MSG_BIND_DELETE_FORM;
        flag = ACTION_DELETE;
    } else if (r->r_actions & ACTION_MODRDNFORM) {
        cp = MSG_BIND_MODRDN_FORM;
        flag = ACTION_MODRDN;
    } else {
        cp = MSG_BIND_FORM;
        flag = ACTION_FORM;
    }
    msg_fprintf(fp, cp, "sssss", rdn, trdn, ufn,
      dn2url(r, dn, FLAG_LANGUAGE|FLAG_TMPL, flag, s, NULL), auth_form);
    fputs(MSG_HTML_END, fp);
    free(rdn);
    free(trdn);
    free(ufn);
    if (binddns)
        free(binddns);
    if (auth_form)
        free(auth_form);
    return OK;
#endif
}

/* This is very, very experimental ... */
int
do_auth_bind(
    REQUEST     *r,
    RESPONSE    *resp
)
{
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "do_auth_bind (%s, 0x%x = %s): %s\n", 
            r->r_dn, r->r_flags, flag2string(r->r_flags),
            r->r_authorization ? r->r_authorization : "no auth");
#endif

    resp->resp_status = AUTH_REQUIRED;
    resp->resp_extra = "WWW-Authenticate: Basic realm=\"BIND DN + password\"\r\n";
    http_header(r, resp);
    fprintf(fp, "Authorization didn't succeed! <A HREF=\"%s\">%s</A>\n",
        dn2url(r, r->r_dn, FLAG_LANGUAGE|FLAG_TMPL, 0, NULL, NULL), r->r_dn);
}


int
do_basic_auth(
    REQUEST     *r,
    RESPONSE    *resp
)
{
    char    *bind_dn, *bind_pw;
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "do_basic_auth (%s, 0x%x = %s): %s\n", 
            r->r_dn, r->r_flags, flag2string(r->r_flags),
            r->r_authorization ? r->r_authorization : "no auth");
#endif

    if (r->r_authorization && *r->r_authorization &&
        (strncasecmp(r->r_authorization, "Basic ", 6) == 0)) {
        bind_dn = b64_decode(r->r_authorization + 6);
        if ((bind_pw = strchr(bind_dn, ':')) != NULL) {
            *bind_pw++ = '\0';
        }
        if (strcasecmp(bind_dn, r->r_dn) != 0 && 
            (r->r_ld = web500gw_ldap_init(r, resp, bind_dn, bind_pw, 0)) 
             != (LDAP *)0) {
            return(OK);
        }
    }
    do_auth_bind(r, resp);
    return(NOTOK);
}
