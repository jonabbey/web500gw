/*
 * web500gw:  A LDAP based WWW - X.500 gateway
 *
 * written by: Frank Richter, Frank.Richter@hrz.tu-chemnitz.de
 *             Chemnitz University of Technology, Germany
 *
 * Copyright (c) 1994-8 Chemnitz University of Technology.
 * All rights reserved.
 *
 * $Id: delete.c,v 1.2 2001/04/26 22:16:53 dekarl Exp $
 */

#include "web500gw.h"


/*
 * do_delete: Performs an LDAP DELETE request:
 */
int
do_delete(
    REQUEST     *r,
    RESPONSE    *resp
)
{

#ifndef MODIFY
    do_error(r, resp, LDAP_OTHER, NOT_IMPLEMENTED, MSG_NOT_SUPPORTED, NULL);
    return NOTOK;
#else   /* defined MODIFY */

    char    *bind_as, *args; 
    char    *ufn, *rdn = NULL, *dn, *pw, *next, *cp;
    int     rc, in_home;

/* query: 
 *   query: DN
 *   args:  dn=DN&userPassword=PW
 */

    args = r->r_method == POST ? r->r_postdata : r->r_filter;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "do_delete (%s, %s)\n", 
        r->r_dn, args ? args : "", 0, 0);
#endif

    dn = r->r_dn;
    bind_as = pw = NULL;
    next = args;
    while ((cp = next) && *cp) {     /* search args */
        next = strchr(cp, '&');
        if (next)
            *next++ = '\0';
        if (strncmp(cp, "dn=", 3) == 0) {
            bind_as = strdup(cp + 3);
            if ((cp = strchr(bind_as, '&')) != NULL)
                *cp = '\0';
        } else if (strncmp(cp, "userPassword=", 13) == 0) {
            pw = strdup(cp + 13);
            if ((cp = strchr(pw, '&')) != NULL)
                *cp = '\0';
        }
    }  /* end of args parsing */

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

    ufn = friendly_dn(resp, dn);
    rdn = friendly_rdn(resp, dn, 1);
    in_home = isinhome(dn);

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "BINDING as %s ... ", bind_as, 0, 0, 0);
#endif
    if ((r->r_ld = web500gw_ldap_init(r, resp, bind_as, pw)) == (LDAP *)0)
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
            MSG_HTML_START_MISC_HOME : MSG_HTML_START_MISC, "sss",
            MSG_DELETE_RESULTS, rdn, ufn);
        resp->resp_htmlheader = 1;
    }
    msg_fprintf(fp, MSG_DELETE_RESULTS_FOR, "ss", rdn, ufn);

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "ldap_delete_s (%s)\n", dn, 0, 0, 0);
#endif

    if ((rc = ldap_delete_s(r->r_ld, dn) != LDAP_SUCCESS)) {
        /* oh shit */
#ifdef WEB500GW_DEBUG
            if (web500gw_debug)
                ldap_perror(r->r_ld, "ldap_delete_s");
#endif
        msg_fprintf(fp, MSG_DELETE_ERROR, "ssiss", rdn, ufn, rc,
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
    msg_fprintf(fp, MSG_DELETE_OK, "ss", rdn, ufn);
    fputs(MSG_HTML_END, fp);
    fputs("\n", fp);

    free(ufn); free(rdn);
    return OK;

#endif
}

