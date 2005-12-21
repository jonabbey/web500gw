/*
 * web500gw:  A LDAP based WWW - X.500 gateway
 *
 * written by: Frank Richter, Frank.Richter@hrz.tu-chemnitz.de
 *             Chemnitz University of Technology, Germany
 *
 * Copyright (c) 1994-8 Chemnitz University of Technology.
 * All rights reserved.
 *
 * $Id: modrdn.c,v 1.2 2001/04/26 22:16:53 dekarl Exp $
 */

#include "web500gw.h"


/*
 * do_modrdn: Performs an LDAP MODIFY RDN request:
 */
int
do_modrdn(
    REQUEST     *r,
    RESPONSE    *resp
)
{

#ifndef MODIFY
    do_error(r, resp, LDAP_OTHER, NOT_IMPLEMENTED, MSG_NOT_SUPPORTED, NULL);
    return NOTOK;
#else   /* defined MODIFY */

    char    *bind_as, *pw, *nrdn, *args; 
    char    *olddn, *oldufn, *oldrdn, *newdn, *newrdn, *newufn, *next, **s, *cp;
    int     rc, i, in_home;
    int     deleteoldrdn = 0;

/* query: 
 *   query: DN
 *   args:  dn=BindDN&userPassword=PW&newrdn=NewRDN&deleteoldrdn=[01]
 */

    args = r->r_method == POST ? r->r_postdata : r->r_filter;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "do_modrdn (%s, %s)\n", 
        r->r_dn, args ? args : "", 0, 0);
#endif

    olddn = r->r_dn;
    bind_as = pw = nrdn = NULL;
    next = args;
    while ((cp = next) && *cp) {     /* search args */
        next = strchr(cp, '&');
        if (next)
            *next++ = '\0';
        if (strncmp(cp, "dn=", 3) == 0) {
            bind_as = strdup(cp + 3);
        } else if (strncmp(cp, "userPassword=", 13) == 0) {
            pw = strdup(cp + 13);
        } else if (strncmp(cp, "newrdn=", 7) == 0) {
            nrdn = strdup(cp + 7);
        } else if (strncmp(cp, "deleteoldrdn=", 13) == 0) {
            deleteoldrdn = atoi(cp + 13);
        }
    }  /* end of args parsing */

    if (! (bind_as && *bind_as)) {
        do_error(r, resp, LDAP_OTHER, BAD_REQUEST, MSG_NO_BIND_DN, NULL);
        return NOTOK;
    }
    hex_qdecode(bind_as);
    r->r_binddn = bind_as;
    if (! (pw && *pw)) {
        do_error(r, resp, LDAP_OTHER, BAD_REQUEST, MSG_MISSING_PASSWORD, NULL);
        return NOTOK;
    }
    hex_qdecode(pw);

    if (! (nrdn && *nrdn)) {
        do_error(r, resp, LDAP_OTHER, BAD_REQUEST, MSG_MODRDN_NO_NEWRDN, NULL);
        return NOTOK;
    }
    hex_qdecode(nrdn);

    in_home = isinhome(olddn);
    oldufn = friendly_dn(resp, olddn);
    oldrdn = friendly_rdn(resp, olddn, 1);

    s = ldap_explode_dn(olddn, 0);
    newdn = calloc(strlen(nrdn) + strlen(olddn) + 3, sizeof(char));
    strcpy(newdn, nrdn);
    for (i = 1; s[i] != NULL; i++) {
        strcat(newdn, ", ");
        strcat(newdn, s[i]);
    }

    newufn = friendly_dn(resp, newdn);
    newrdn = friendly_rdn(resp, newdn, 1);

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "BINDING as %s ...\n", bind_as, 0, 0, 0);
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
            MSG_HTML_START_MISC_HOME : MSG_HTML_START_MISC, "sss",
            MSG_MODRDN_RESULTS, oldrdn, oldufn);
        resp->resp_htmlheader = 1;
    }
    msg_fprintf(fp, MSG_MODRDN_RESULTS_FOR, "ssss", oldrdn, oldufn, newrdn, newufn);

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "ldap_modrdn2_s (%s, %s, %s)\n", 
        olddn, nrdn, deleteoldrdn ? "delete old rdn" : "don't delete old rdn", 0);
#endif

    if ((rc = ldap_modrdn2_s(r->r_ld, olddn, nrdn, deleteoldrdn) != LDAP_SUCCESS)) {
        /* Error! */
#ifdef WEB500GW_DEBUG
            if (web500gw_debug)
                ldap_perror(r->r_ld, "ldap_modrdn2_s");
#endif
        msg_fprintf(fp, MSG_MODRDN_ERROR, "ssssiss",oldrdn, oldufn, newrdn, newufn,
            rc, web500gw_err2string(rc, resp), 
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
    msg_fprintf(fp, MSG_MODRDN_OK, "sssss", oldrdn, oldufn, newrdn, newufn,
        dn2url(r, newdn, FLAG_LANGUAGE, 0, NULL, NULL));
    fputs(MSG_HTML_END, fp);
    fputs("\n", fp);

    free(oldufn); free(oldrdn);
    free(newufn); free(newrdn); free(newdn);
    ldap_value_free(s);

    return OK;

#endif
}

