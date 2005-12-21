/*
 * navigation.c:      prints navigation / search menu
 *
 * written by: Frank Richter, Frank.Richter@hrz.tu-chemnitz.de
 *             Chemnitz University of Technology, Germany, 1994-7
 *
 * Copyright (c) 1994-7 Chemnitz University of Technology.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the Chemnitz University of Technology. The name of the
 * University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 *
 * $Id: navigation.c,v 1.2 2001/04/26 22:16:53 dekarl Exp $
 */

#include "web500gw.h"

/* Make "Move upwards" navigation and search form if nonleaf */

int
make_upsearch (
    LDAP        *ld,
    REQUEST     *r,
    RESPONSE    *resp,
    int         nonleaf
)
{
    char **s, **t, *format, string[BUFSIZ], hrdn[BUFSIZ], *moveup, *url_moveup;
    int  cnt, i, j, size = BUFSIZ;
    static int go_up_already = 0;
    int     scope, cgimode_save = 0;
    char    *scope_msg, *scope_name, *search_base, *url_dn, *ext_search;


#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "make_upsearch (%s, %d)\n", 
        r->r_dn ? r->r_dn : "", nonleaf, 0, 0);
#endif

    if (go_up_already++)
        return OK;
    if (r->r_browser->b_upsearch == UPSEARCH_NONE)
        return OK;
    if (!r->r_dn || ! *r->r_dn)        /* the root */
        return OK;

    url_moveup = dn2url(r, cgimode ? "" : "/", 0, 0, NULL, NULL);

    string[0] = '\0';
    moveup = calloc(size, sizeof(char));

    s = ldap_explode_dn(r->r_dn, 1);
    t = ldap_explode_dn(r->r_dn, 0);

    if (!(r->r_browser->b_upsearch & UPSEARCH_ON_TOP))
        fputs(MSG_HR, fp);

    if (r->r_browser->b_upsearch & UPSEARCH_MENU && cgimode) {
        /* make values for SELECT without /cgi-bin/... URL */
        cgimode_save = cgimode;
        cgimode = 0;
    }

    /* Start of navigation menu: ROOT */
    if (r->r_browser->b_upsearch & UPSEARCH_MENU) {
        sprintf(string, " <SELECT NAME=\"\"><OPTION VALUE=\"%s\">%s\n", 
            dn2url(r, "", FLAG_LANGUAGE, 0, NULL, NULL), MSG_ROOT);
    } else if (r->r_browser->b_upsearch & UPSEARCH_SMALL) {
        sprintf(string, " <A HREF=\"/%s\">%s</A>\n", 
            dn2url(r, "", FLAG_LANGUAGE, 0, NULL, NULL), MSG_ROOT);
    } else {
        sprintf(string, " <LI><A HREF=\"/%s\">%s</A>\n", 
            dn2url(r, "", FLAG_LANGUAGE, 0, NULL, NULL), MSG_ROOT);
    }
    while (strlen(string) + strlen(moveup) >= size) {
        size += BUFSIZ;
        moveup = realloc(moveup, size);
    }
    strcat(moveup, string);

    string[0] = '\0';
    for (cnt = 0; t[cnt]; cnt++);
    for (i = cnt - 1; i > 0 && s[i]; i--) {
        strcpy(hrdn, t[i]);
        for (j = i + 1; j < cnt ; j++) {
            strcat(hrdn, ", ");
            strcat(hrdn,t[j]);
        }
        if (r->r_browser->b_upsearch & UPSEARCH_MENU) {
            sprintf(string, "<OPTION VALUE=\"%s\" %s>%s\n",
                dn2url(r, hrdn, FLAG_LANGUAGE, 0, NULL, NULL), 
                i == 1 ? "SELECTED" : "", 
                i == cnt - 1 ?   friendly_label(resp, s[i]) : s[i]);
        } else if (r->r_browser->b_upsearch & UPSEARCH_SMALL) {
            sprintf(string, ", <A HREF=\"/%s\">%s</A>", 
                dn2url(r, hrdn, FLAG_LANGUAGE, 0, NULL, NULL), i == cnt - 1 ? 
                friendly_label(resp, s[i]) : s[i]);
        } else {
            sprintf(string, "<LI><A HREF=\"/%s\">%s</A>\n", 
                dn2url(r, hrdn, FLAG_LANGUAGE, 0, NULL, NULL),
                i == cnt - 1 ? friendly_label(resp, s[i]) : s[i]);
        }
        while (strlen(string) + strlen(moveup) >= size) {
            size += BUFSIZ;
            moveup = realloc(moveup, size);
        }
        strcat(moveup, string);

        hrdn[0] = '\0';
    }

    if (r->r_browser->b_upsearch & UPSEARCH_MENU) {
        strcat(moveup, "</SELECT>\n");
        if (cgimode_save)
            cgimode = cgimode_save;
    } else {
        
    }
    if (nonleaf) {
        search_base = r->r_dn;
    } else  {
        /* strip first component */
        hrdn[0] = '\0';
        for (j = 1; j < cnt ; j++) {
            if (j > 1)
                strcat(hrdn, ", ");
            strcat(hrdn,t[j]);
        }
        search_base = hrdn;
/*
        if ((search_base = strchr(r->r_dn, ',')) != NULL) {
            search_base++;
            while (isspace(*search_base)) ++search_base;
        }
*/
    }

    scope = make_scope(ld, search_base);  /* onelevel or subtree search ? */
    scope_msg = (scope == LDAP_SCOPE_ONELEVEL ? MSG_ONELEVEL : MSG_SUBTREE);

    /* That's a bit tricky to get the right URL for search */
    if (r->r_language) {
        scope_name = scope == LDAP_SCOPE_ONELEVEL ? "O" : "S";
    } else {
        scope_name = scope == LDAP_SCOPE_ONELEVEL ? "?O" : "?S";
    }

    url_dn = dn2url(r, search_base, FLAG_LANGUAGE, 0, NULL, NULL);
    ext_search = dn2url(r, search_base, FLAG_LANGUAGE, ACTION_SEARCH_FORM,
                        NULL, NULL);

    if (r->r_browser->b_upsearch & UPSEARCH_MENU) {
        format = MSG_NAV_MENU;
    } else if (r->r_browser->b_upsearch & UPSEARCH_SMALL) {
        format = MSG_NAV_SMALL;
    } else {
        format = MSG_NAV_LIST;
    }
    msg_fprintf(fp, format, "ssssssss", url_moveup, moveup, url_dn, 
        scope_msg, friendly_rdn(resp, search_base, 1),
        friendly_dn(resp, search_base), scope_name, ext_search);

    free(url_moveup);
    free(moveup);
    free(url_dn);
    free(ext_search);
    ldap_value_free(s);
    ldap_value_free(t);
    fflush(fp);

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "make_upsearch ><\n", 0, 0, 0, 0);
#endif
    return OK;
}
