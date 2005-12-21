/*
 * web500gw:  A LDAP based WWW - X.500 gateway
 *
 * written by: Frank Richter, Frank.Richter@hrz.tu-chemnitz.de
 *             Chemnitz University of Technology, Germany
 *
 * Copyright (c) 1994-8 Chemnitz University of Technology.
 * All rights reserved.
 *
 * $Id: search.c,v 1.2 2001/04/26 22:16:53 dekarl Exp $
 */

#include "web500gw.h"

/*
 * do_search: The search routine
 */

int
do_search(
    REQUEST         *r,
    RESPONSE        *resp
)
{
    char            *filtertype, **search_attrs, *search_filter;
    char            *print_filter, *human_filter;
    int             scope, count = 0, rc, i = 0, j, in_home;
    struct timeval  timeout;
    LDAPFiltInfo    *fi;
    LDAPMessage     *e, *res;
    static char     *def_attrs[] = 
                {"objectClass", "sn", "aliasedObjectName", "labeledURI", 0};
    struct ldap_disptmpl    *tmpl;
    char    **oc;
    char    *result_dn, *doc, *foc, *url_dn;
    char    *base_dn, *base_ufn, *result_ufn, *alias_ufn, *sortstring, *cp, *bp;
    char    **sn, href[10 * BUFSIZ], *temp, **val, *server = NULL;
    int     counter = 0, base_len, isalias = 0;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "do_search (%s, %s, (",
        r->r_dn, flag2string(r->r_flags), 0, 0);
    while (r->r_attrs && r->r_attrs[i]) {
        Web500gw_debug(WEB500GW_DEBUG_TRACE, "%s ", r->r_attrs[i], 0, 0, 0);
        i++;
    }
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "), %s)\n", 
        (r->r_filter && *r->r_filter) ? r->r_filter : "No filter", 0, 0, 0);
#endif

    if (r->r_ld == (LDAP *)0 &&
        (r->r_ld = web500gw_ldap_init(r, resp, NULL, NULL, 1)) == NULL)
        return NOTOK;

    base_dn = r->r_dn;
    if (strlen(base_dn) > 0) {
        base_ufn = friendly_dn(resp, base_dn);
    } else {
        base_ufn = NULL;
    }
    base_len = base_ufn ? strlen(base_ufn) : 0;


    if (r->r_filter == NULL) {
        return NOTOK;
    }
    print_filter = r->r_filter;
#if defined(OWN_STR_TRANSLATION)
    search_filter = strdup(web500gw_isotot61(r->r_filter));
#else
    search_filter = r->r_filter;
#endif
    if (*search_filter == '\0' || *(search_filter+1) != '=') {
        scope = LDAP_SCOPE_ONELEVEL;
    } else {
        if (*search_filter == 'S') {
            scope = LDAP_SCOPE_SUBTREE;
        } else {
            scope = LDAP_SCOPE_ONELEVEL;
        }
        search_filter += 2;
        print_filter += 2;
    }

    /* make a "human readable filter" - for now, no artistic things are done */
    human_filter = print_filter;
    if ((cp = strchr(print_filter, '=')) != NULL) {
        if ((cp = strchr(cp+1, '=')) != NULL) {
            /* two '=' in it - must be a complex filter - isn't readable... */
            human_filter = MSG_COMPLEX_FILTER;
        }
    }

    if (*search_filter) {   /* not the default filter */
        r->r_flags |= FLAG_FILTER;
                            /* skip leading white spaces */
        while (isspace(*search_filter)) ++search_filter;
    }
    if (r->r_attrs == NULL || *r->r_attrs == NULL) {
        /* No attributes requested - read default attrs for sorting */
        search_attrs = def_attrs;
    } else {
        j = 0;
        while (def_attrs[j]) j++;
        search_attrs = (char **) calloc(r->r_attrnumb + j + 1, sizeof(char *));
        i = j = 0;
        while (r->r_attrs && r->r_attrs[i])  {
            search_attrs[i] = r->r_attrs[i];
            i++;
        }
        while (def_attrs[j]) 
            search_attrs[i++] = def_attrs[j++];    
        search_attrs[i] = NULL;
    }

    if ((in_home = isinhome(base_dn)))
        /* ACCESS sizelimit if searching below HOME DN */
        r->r_ld->ld_sizelimit = r->r_access->a_sizelimit;
    else
        r->r_ld->ld_sizelimit = sizelimit;

    /* A simple filter (not starting with '(') with a comma in it 
     *  -> UFN assumed */
    if (ufnsearch && search_filter[0] != '(' && 
        strchr(search_filter, ',') != NULL && strlen(search_filter) > 3) {
        if (strlen(base_dn) > 0)
            ldap_ufn_setprefix(r->r_ld, base_dn);
        timeout.tv_sec = timelimit;
        timeout.tv_usec = 0;
        ldap_ufn_timeout((void *) &timeout);
        r->r_ld->ld_deref = LDAP_DEREF_FINDING;
#ifdef WEB500GW_DEBUG
        Web500gw_debug(WEB500GW_DEBUG_TRACE, "ldap_ufn_search_s (%s)\n", 
            search_filter, 0, 0, 0);
#endif
        if ((rc = ldap_ufn_search_s(r->r_ld, search_filter, search_attrs, 0, &res))
            != LDAP_SUCCESS && rc != LDAP_SIZELIMIT_EXCEEDED) {
            do_error(r, resp, rc, 0, r->r_ld->ld_error, r->r_ld->ld_matched);
            return NOTOK;
        }
        count = ldap_count_entries(r->r_ld, res);
#ifdef WEB500GW_DEBUG
        Web500gw_debug(WEB500GW_DEBUG_TRACE, "UFN ready: %d results!\n",
            count, 0, 0, 0);
#endif
        /* Reset DN, because this UFN search was probably not below DN */
        base_dn = base_ufn = "";
        in_home = 0;

        friendlyDesc = MSG_UFN;

    } else {
        /* let's do the search */
        filtertype = (scope == LDAP_SCOPE_ONELEVEL ? "web500gw onelevel" :
            "web500gw subtree");
        r->r_ld->ld_deref = (scope == LDAP_SCOPE_ONELEVEL ? LDAP_DEREF_FINDING :
            LDAP_DEREF_ALWAYS);
        timeout.tv_sec = timelimit;
        timeout.tv_usec = 0;
    
        friendlyDesc = NULL;

        /* try all filters til we have success */
        for (fi = ldap_getfirstfilter(r->r_access->a_filtd, filtertype, search_filter);
            fi != NULL; fi = ldap_getnextfilter(r->r_access->a_filtd)) {
#ifdef WEB500GW_DEBUG
            Web500gw_debug(WEB500GW_DEBUG_FILTER, "  search %s: %s -- %s\n",
                scope == LDAP_SCOPE_ONELEVEL ? "onelevel" : "subtree",
                fi->lfi_filter, fi->lfi_desc, 0);
#endif
            if ((rc = ldap_search_st(r->r_ld, base_dn, scope, fi->lfi_filter,
                search_attrs, 0, &timeout, &res)) != LDAP_SUCCESS && 
                rc != LDAP_SIZELIMIT_EXCEEDED && rc != LDAP_INSUFFICIENT_ACCESS 
                && rc != LDAP_TIMELIMIT_EXCEEDED && rc != LDAP_TIMEOUT
                && rc != LDAP_PARTIAL_RESULTS) {
                do_error(r, resp, rc, 0, r->r_ld->ld_error, r->r_ld->ld_matched);
                return NOTOK;
            }
            if ((count = ldap_count_entries(r->r_ld, res)) > 0) {
                /* found something */
                friendlyDesc = friendly_label(resp, fi->lfi_desc);
#ifdef WEB500GW_DEBUG
                Web500gw_debug(WEB500GW_DEBUG_FILTER, 
                    " searched ... and found %d results!\n", count, 0, 0, 0);
#endif
                break;
            }
#ifdef WEB500GW_DEBUG
            Web500gw_debug(WEB500GW_DEBUG_FILTER, 
                " searched ... and found no results!\n", 0, 0, 0, 0);
#endif
        }
        r->r_ld->ld_deref = LDAP_DEREF_ALWAYS;
    }

    if (count < 1) {        /* nothing found :-( */
        if (r->r_flags & FLAG_SEARCHACT) {
            /* in a search action we silently ignore this */
            return OK;
        }

        resp->resp_status = NOT_FOUND;
        if (resp->resp_httpheader == 0) {
            if (r->r_httpversion == 1) {
                /* Expires now */
                resp->resp_expires = 0;
                http_header(r, resp);
            }
            if (r->r_method == HEAD) {
                return OK;
            }
        }
        if (resp->resp_htmlheader == 0) {
            msg_fprintf(fp, in_home && *MSG_HTML_START_NO_SEARCH_RESULTS_HOME
            ? MSG_HTML_START_NO_SEARCH_RESULTS_HOME : MSG_HTML_START_NO_SEARCH_RESULTS, 
                "ss", print_filter, base_ufn ? base_ufn : MSG_ROOT);
            resp->resp_htmlheader = 1;

            if (!(r->r_flags & FLAG_ENTRYONLY)) {
                if (in_home && MSG_HOMEBANNER) {
                    msg_fprintf(fp, MSG_HOMEBANNER, "sss", r->r_query,
                        other_lang_select, other_lang_string);
                } else {
                    msg_fprintf(fp, MSG_BANNER, "sss", r->r_query,
                        other_lang_select, other_lang_string);
                }
            }
            if (rc == LDAP_INSUFFICIENT_ACCESS) {
                fputs(MSG_NO_LIST_ACCESS, fp);
            } else if (rc == LDAP_TIMELIMIT_EXCEEDED || rc == LDAP_TIMEOUT) {
                fputs(MSG_TIMELIMIT, fp);
            } else {
                if (print_filter && *print_filter) {
                    msg_fprintf(fp, MSG_NO_SEARCH_RESULTS, "ssss",
                    (scope == LDAP_SCOPE_ONELEVEL ?  MSG_ONELEVEL : MSG_SUBTREE),
                    print_filter, human_filter, base_ufn ? base_ufn : MSG_ROOT);
                } else {
                    fputs(MSG_NOTHING_FOUND_READORSEARCH, fp);
                }
            }
            if (!(r->r_flags & FLAG_ENTRYONLY)) {
                make_upsearch(r->r_ld, r, resp, 1);
                if (in_home && MSG_HOMETRAILER) {
                    fputs(MSG_HOMETRAILER, fp);
                } else {
                    fputs(MSG_TRAILER, fp);
                }
            }
            fputs(MSG_HTML_END, fp);
            fputs("\n", fp);
        } else {        
            /* resp->resp_htmlheader != 0 -> within a document (searchaction) */
            if (rc == LDAP_INSUFFICIENT_ACCESS) {
                fputs(MSG_NO_LIST_ACCESS, fp);
            } else if (rc == LDAP_TIMELIMIT_EXCEEDED || rc == LDAP_TIMEOUT) {
                fputs(MSG_TIMELIMIT, fp);
            } else {
                fputs(MSG_NOTHING_FOUND_READORSEARCH, fp);
                fputs(MSG_HTML_END, fp);
                fputs("\n", fp);
            }
        }
        return OK;
    }
    if (showonematch == 1 && count == 1 && r->r_attrnumb == 0 &&
        !(r->r_flags & FLAG_SEARCHACT)) {
    /* header != 0 ? */
        /* only one result and not searching for special attrs (searchaction) */
        e = ldap_first_entry(r->r_ld, res);
        if (e != NULL) {
            char **oc, **aliasdn;
            oc = ldap_get_values(r->r_ld, e, objectclass_attr);
            aliasdn = ldap_get_values(r->r_ld, e, "aliasedObjectName");
            if (aliasdn && *aliasdn)
                result_dn = *aliasdn;
            else
                result_dn = ldap_get_dn(r->r_ld, e);
            if (result_dn) {
                result_ufn = friendly_dn(resp, result_dn);
                if (gwswitch) {
                    server = gw_switch(r->r_ld, resp, e);
                }
                r->r_flags &= ~FLAG_FILTER;    /* it's not a search anymore */
                url_dn = dn2url(r, result_dn, 
                          server ? r->r_flags & ~FLAG_LANGUAGE : r->r_flags, 
                          0, NULL, server);

                /* Redirect */
#ifdef WEB500GW_DEBUG
                Web500gw_debug(WEB500GW_DEBUG_TRACE, 
                    "do_search: found one entry -> REDIRECT: %s\n",
                         url_dn, 0, 0, 0);
#endif
                resp->resp_status = REDIRECT;
                if (resp->resp_httpheader == 0 && r->r_httpversion == 1) {
                    resp->resp_location = malloc(strlen(url_dn) + 11);
                    strcpy(resp->resp_location, url_dn);

                    http_header(r, resp);
                    /* free(resp->resp_location); */
                }
                if (r->r_method == HEAD) {
                    return OK;
                }
                msg_fprintf(fp, in_home && *MSG_HTML_START_SEARCH_RESULTS_HOME ?
                MSG_HTML_START_SEARCH_RESULTS_HOME : MSG_HTML_START_SEARCH_RESULTS, 
                    "ss", print_filter, result_ufn);
                msg_fprintf(fp, MSG_REDIRECT, "ss", url_dn, result_ufn);
                fputs(MSG_HTML_END, fp);
                fputs("\n", fp);
                return OK;
            }
        }
    }

    /* Found something */
    /* Prepare HTTP + HTML header */
    if (resp->resp_httpheader == 0 && r->r_httpversion == 1) {
        http_header(r, resp);
        if (r->r_method == HEAD)
            return OK;
    }
    if (resp->resp_htmlheader == 0) {
        msg_fprintf(fp, in_home && *MSG_HTML_START_SEARCH_RESULTS_HOME ?
            MSG_HTML_START_SEARCH_RESULTS_HOME : MSG_HTML_START_SEARCH_RESULTS, 
            "ss", print_filter, base_ufn ? base_ufn : MSG_ROOT);
        if (!(r->r_flags & FLAG_ENTRYONLY)) {
            if (in_home && MSG_HOMEBANNER) {
                msg_fprintf(fp, MSG_HOMEBANNER, "sss", r->r_query,
                    other_lang_select, other_lang_string);
            } else {
                msg_fprintf(fp, MSG_BANNER, "sss", r->r_query,
                    other_lang_select, other_lang_string);
            }
        }
        resp->resp_htmlheader = 1;
    }
    if ((!(r->r_flags & FLAG_ENTRYONLY)) && 
        (r->r_browser->b_upsearch & UPSEARCH_ON_TOP)) {
        make_upsearch(r->r_ld, r, resp, 1);
    }
    if (r->r_flags & FLAG_FILTER && 
        (!(r->r_flags & FLAG_ENTRYONLY))) {
        /* Not the default filter and not entryonly */

        msg_fprintf(fp, MSG_SEARCH_RESULTS, "ssssisss",
            (scope == LDAP_SCOPE_ONELEVEL ?  MSG_ONELEVEL : MSG_SUBTREE),
            print_filter, human_filter, 
            base_ufn && *base_ufn ? base_ufn : MSG_ROOT,
            count, count == 1 ? MSG_ENTRY : MSG_ENTRIES, friendlyDesc,
            rc == LDAP_PARTIAL_RESULTS ?
            resp->resp_language->l_conf->c_errmsg[LDAP_PARTIAL_RESULTS] : "");
    }
    if (ldap_result2error(r->r_ld, res, 0) == LDAP_SIZELIMIT_EXCEEDED) {
        fprintf(fp, r->r_flags & FLAG_FILTER && r->r_access->a_sizelimit == 0 ? 
            MSG_SIZELIMIT_1 : MSG_SIZELIMIT_0, count);
    }
    
    /* Now prepare the result for sorting */ 

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, 
        "do_search: preparing for sort\n", 0, 0, 0, 0);
#endif

    /* for all the results ... */
    for (e = ldap_first_entry(r->r_ld, res); e != NULL && counter < MAX_LISTSIZE; 
         e = ldap_next_entry(r->r_ld, e)) {

        result_dn = ldap_get_dn(r->r_ld, e);
        result_ufn = friendly_dn(resp, result_dn);
        if ((val = ldap_get_values(r->r_ld, e, "aliasedObjectName"))) {
            /* entry is alias */
            free(result_dn);
            result_dn = strdup(*val);
            alias_ufn = friendly_dn(resp, result_dn);
            ldap_value_free(val);
            isalias = 1;
        } else {
            isalias = 0;
        }
        if (isalias && r->r_flags & FLAG_DEREFALIAS) {
            /* if alias, look for objectClass of real entry */
            oc = deref(r->r_ld, result_dn);
        } else {
            oc = ldap_get_values(r->r_ld, e, objectclass_attr);
        }
            
        /* find template to see the plural name of the objectclass */
        tmpl = ldap_oc2template(oc, r->r_access->a_tmpllist);
        doc = pick_oc(oc);
        if (tmpl && tmpl->dt_pluralname) {
            foc = strdup(friendly_label(resp, tmpl->dt_pluralname));
        } else {
            foc = strdup(friendly_label(resp, doc));
            if (strcmp(foc, doc) == 0) {
            /* "no friendly objectclass" found -> last in the list (a hack) */
                foc = (char *)malloc(strlen(doc)+9);
                sprintf(foc, "&#032;%s", doc);
            }
        }
        if (strncasecmp(result_ufn, "{ASN}", 5) == 0) {
            /* ASNs are not something we want to display! */
            free(result_dn);
            ldap_value_free(oc);
            continue;
        }
        if (base_len > 0) {
            /* strip search base from result */
            result_ufn = strip_dn(result_ufn, base_ufn);
        }
#ifdef nodef
            cp = result_ufn + strlen(result_ufn) - 1;
            bp = base_ufn + base_len - 1;
            while (bp >= base_ufn && *cp-- && *bp-- && *cp == *bp);
            
            if ((bp+1) == base_ufn && cp != NULL) { /* Complete match on base */
                /* strip trailing whitespaces and ',' */
                while (*cp && isspace(*cp)) cp--;
                if (cp != NULL && *cp == ',')
                    *cp = '\0';
            }
        }
#endif
        
        sortstring = strdup(result_ufn);
        if (strcasecmp(doc, "country") == 0) {  /* a country */
            sn = NULL;
        } else {                                /* not a country */
            /* DNs may have components in '"', ignored  when sorting */
            if (*sortstring == '"') {
                sortstring++;
                cp = strchr(sortstring+1, '"');
            } else {
                cp = sortstring;
            }
            if ((cp = strchr(cp,',')))
                *cp = '\0';
            if (isalias) {      /* aliases don't have surnames */
                if (isoc (oc, "person")) {
                }
            }
            sn = ldap_get_values(r->r_ld, e, "sn");
/*  Delete spaces ???
            if (sn) {
                cp = *sn;
                while ((cp = strchr(cp,' '))) {
                    cp ++;
                    spaces ++;
                }
            }
            while (spaces > 0) {
                if ((cp = strrchr(sortstring,' '))) {
                    *cp = '\0';
                    spaces --;
                } else
                    break;
            }
*/
            if ((cp = strchr(sortstring,'+'))) {
                cp --;
                *cp = '\0';
            }
        }

        if (gwswitch) {
            server = gw_switch(r->r_ld, resp, e);
        }
        /* build the link (HREF) */
        if (isalias) {
            if (isnonleaf(r->r_ld, oc, result_dn)) {
                if (r->r_flags & FLAG_NOHREFDN)
                    msg_snprintf(href, sizeof(href), MSG_DN_ALIAS_TO_NONLEAF, 
                        "sss", result_ufn, alias_ufn, html_encode(result_dn));
                else
                    msg_snprintf(href, sizeof(href), MSG_HREF_ALIAS_TO_NONLEAF,
                        "ssssss",
                        dn2url(r, result_dn, server ? 0 : FLAG_LANGUAGE,
                        0, NULL, server), result_ufn,
                        alias_ufn, html_encode(result_dn),
                        string_encode(result_ufn),string_encode(result_dn));
            } else {
                if (r->r_flags & FLAG_NOHREFDN)
                    msg_snprintf(href, sizeof(href), MSG_DN_ALIAS_TO_LEAF, 
                        "sss",
                        result_ufn, alias_ufn, html_encode(result_dn));
                else
                    msg_snprintf(href, sizeof(href), MSG_HREF_ALIAS_TO_LEAF,
                        "ssssss",
                        dn2url(r, result_dn, server ? 0 : FLAG_LANGUAGE, 0, NULL,
                        server), result_ufn,
                        alias_ufn, html_encode(result_dn),
                        string_encode(result_ufn),string_encode(result_dn));
            }
        } else if (isnonleaf(r->r_ld, oc, result_dn)) {
            if (r->r_flags & FLAG_NOHREFDN)
                msg_snprintf(href, sizeof(href), MSG_DN_NON_LEAF, "ss",
                    result_ufn, html_encode(result_dn));
            else
                msg_snprintf(href, sizeof(href), MSG_HREF_NON_LEAF, "sssss",
                    dn2url(r, result_dn, server ? 0 : FLAG_LANGUAGE, 0,
                    NULL, server), result_ufn,
                    html_encode(result_dn), string_encode(result_ufn),
                    string_encode(result_dn));
        } else {
            if (r->r_flags & FLAG_NOHREFDN)
                msg_snprintf(href, sizeof(href), MSG_DN_LEAF, "ss",
                    result_ufn, html_encode(result_dn));
            else
                msg_snprintf(href, sizeof(href), MSG_HREF_LEAF, "sssss",
                    dn2url(r, result_dn, server ? 0 : FLAG_LANGUAGE, 0,
                    NULL, server), result_ufn, 
                    html_encode(result_dn), string_encode(result_ufn),
                    string_encode(result_dn));
        }
        /* build the sortstring:  foc[sn]sortstring */
        if (sn) {
            temp = (char *) malloc(strlen(foc)+strlen(*sn)+strlen(sortstring)+1);
        } else {
            temp = (char *) malloc(strlen(foc)+strlen(sortstring)+1);
        }
        strcpy(temp, foc);
        if (sn) {
            strcat(temp, *sn);
        }
        strcat(temp, sortstring);
        if (!dnlist[counter]) {
            dnlist[counter] = (struct dncompare *) malloc(sizeof(struct dncompare));
        }
        dnlist[counter]->sortstring = temp;
        dnlist[counter]->href = strdup(href);
        dnlist[counter]->friendly_oc = foc;
        dnlist[counter]->oc = doc;
        dnlist[counter]->tmpl = tmpl;
        dnlist[counter]->entry = e;
        dnlist[counter]->dn = strdup(result_dn);
        counter++;
        ldap_value_free(oc);
        free(result_dn);
        if (sn)
            ldap_value_free(sn);
    }
    if (counter < 1)        /* No result */
        return NOTOK;

    /* Now sort the list ... */
    qsort(dnlist, counter, sizeof(dnlist[counter]->sortstring), compare);
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, 
        "do_search: qsort done - %d entries\n", counter, 0, 0, 0);
#endif

    /* ... and print it out */
    web500gw_list2html(r->r_ld, r, resp, dnlist, counter);
    fflush(fp);

    if (r->r_flags & FLAG_SEARCHACT) {
        return OK;
    }
    if (! (r->r_flags & FLAG_ENTRYONLY)) {
        if (!(r->r_browser->b_upsearch & UPSEARCH_ON_TOP)) {
            make_upsearch(r->r_ld, r, resp, 1);
        }
        if (in_home && MSG_HOMETRAILER) {
            fputs(MSG_HOMETRAILER, fp);
        } else {
            fputs(MSG_TRAILER, fp);
        }
    }
    fputs(MSG_HTML_END, fp);
    fputs("\n", fp);
    return OK;
}


int
do_search_form (
    REQUEST     *r,
    RESPONSE    *resp
)
{
    char    **s, *o, *c, *search_action;
    int     cnt, in_home;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "do_search_form (%s)\n", 
        r->r_dn, 0, 0, 0);
#endif
    resp->resp_expires = -1;    /* don't need to expire this form */
    http_header(r, resp);
    if (r->r_method == HEAD) {
        return OK;
    }
    in_home = isinhome(r->r_dn);

    msg_fprintf(fp, in_home && *MSG_HTML_START_ENTRY_HOME ?
        MSG_HTML_START_ENTRY_HOME : MSG_HTML_START_ENTRY, "ss",
        MSG_EXT_SEARCH_FORM_TITLE, MSG_EXT_SEARCH_FORM_TITLE);
    if (in_home && MSG_HOMEBANNER) {
        msg_fprintf(fp, MSG_HOMEBANNER, "sss", r->r_query,
            other_lang_select, other_lang_string);
    } else if (MSG_BANNER) {
        msg_fprintf(fp, MSG_BANNER, "sss", r->r_query,
            other_lang_select, other_lang_string);
    }

    s = ldap_explode_dn(r->r_dn, 1);
    for (cnt = 0; s[cnt]; cnt++);
    c = friendly_label(resp, s[cnt-1]);
    /* c = s[cnt-1]; */
    if (cnt > 1)
        o = clean_ufn(s[cnt - 2]);
    else 
        o = "";

    search_action = dn2url(r, r->r_dn, FLAG_LANGUAGE, ACTION_DO_SEARCH, NULL, NULL);
    msg_fprintf(fp, MSG_EXT_SEARCH_FORM, "sssss", 
        search_action, url_encode(r->r_dn),
        html_encode(friendly_dn(resp, r->r_dn)), html_encode(c), html_encode(o));

    if (in_home && MSG_HOMETRAILER) {
        fputs(MSG_HOMETRAILER, fp);
    } else {
        fputs(MSG_TRAILER, fp);
    }
    fputs(MSG_HTML_END, fp);
    fputs("\n", fp);

    ldap_value_free(s);
    free(search_action);
    return OK;
}

int
process_search_form (
    REQUEST     *r,
    RESPONSE    *resp
)
{
    int             attrnumb = 0, len, port = 0;
    char            buf[BUFSIZ], *valwords[2], *cp, *lp, *ap, *np, *args,
                    *oc = NULL, *query = NULL, *filtertemplate = NULL,
                    *matchquery, *match = NULL, *searchattr = NULL,
                    *server = NULL;
    struct language *lang;

    r->r_attrs[0] = NULL;        /* default: read all attributes */
    r->r_flags = 0;

    args = r->r_method == POST ? r->r_postdata : r->r_filter;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "process_search_form (%s)\n", 
        args && *args ? args : "no args!", 0, 0, 0);
#endif

    if (!args || !*args)
        return NOTOK;

/* args:
 * query=searchval&attr=a1[,a2]&attr=a3...&flags=flag1[,f2]&flags=flag3...
 *   -- we parse the args and build arguments for do_search
 */

    lp = cp = args;
    while (lp && *lp) {
        if ((cp = strchr(cp, '&')))
            *cp++ = '\0';
        if (strncasecmp(lp, "filtertemplate=", 15) == 0) {
            /* a search filter template */
            filtertemplate = hex_qdecode(lp + 15);
        } else if (oc == NULL && strncasecmp(lp, "objectclass=", 12) == 0) {
            /* objectClass */
            oc = hex_qdecode(lp + 12);
        } else if (strncasecmp(lp, "searchattr=", 11) == 0) {
            /* attribute to search for */
            searchattr = hex_qdecode(lp + 11);
        } else if (strncasecmp(lp, "match=", 6) == 0) {
            /* search method */
            match = hex_qdecode(lp + 6);
        } else if (query == NULL && strncasecmp(lp, "query=", 6) == 0) {
            query = hex_qdecode(lp + 6);
        } else if (strncasecmp(lp, "base=", 5) == 0) {
            r->r_dn = hex_qdecode(lp + 5);
        } else if (strncasecmp(lp, "ldapserver=", 11) == 0) {
            server = hex_qdecode(lp + 11);
        } else if (strncasecmp(lp, "ldapport=", 9) == 0) {
            port = atoi(hex_qdecode(lp + 9));
        } else if (strncasecmp(lp, "args=", 5) == 0) {  
            /* is a bit recursive... */
            ap = hex_qdecode(lp + 5);
            np = cp;
            cp = lp = ap;
            continue;
        } else if (strncasecmp(lp, "endargs", 7) == 0) {
            cp = np;        /* go further */
        } else if (strncasecmp(lp, "attr=", 5) == 0) {
            ap = lp + 5;
            if (*ap)
                r->r_attrs[attrnumb++] = ap;
            while ((ap = strchr(ap, ','))) {
                *ap++ = '\0';
                if (*ap)
                    r->r_attrs[attrnumb++] = ap;
            }
        } else if (strncasecmp(lp, "flags=", 6) == 0) {
            ap = hex_qdecode(lp += 6);
#ifdef WEB500GW_DEBUG
            Web500gw_debug(WEB500GW_DEBUG_PARSE, "FLAGS: %s\n",
                ap ? ap : "", 0, 0, 0);
#endif
            while (ap && *ap) {
                if ((ap = strchr(ap, ',')))
                    *ap++ = '\0';
                if (strncasecmp(lp, "tmpl=", 5) == 0) {   /* template */
                    r->r_template = lp + 5;
                } else if (strncasecmp(lp, "lang=", 5) == 0) {
                    if ((lang = find_language(lp + 5))) {
                        resp->resp_language = lang;
                        r->r_language = resp->resp_language->l_content_lang;
                        r->r_flags |= FLAG_LANGUAGE;
                    }
                } else {
                    r->r_flags |= string2flag(lp);
                }
                lp = ap;
            }
        } else {
            /* unknown args */
        }
        lp = cp;
    }
    r->r_attrs[attrnumb] = NULL;
    r->r_attrnumb = attrnumb;

    if (allow_other_servers) {
        if (server)
            r->r_ldaphost = server;
        if (port > 0)
            r->r_ldapport = port;
    }

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_PARSE, "LDAP server + port: %s %d\n",
        r->r_ldaphost, r->r_ldapport, 0, 0);
    Web500gw_debug(WEB500GW_DEBUG_PARSE, 
      "process_search_form: query = %s, searchattr = %s, objectclass = %s\n",
      query ? query : "", searchattr ? searchattr : "", oc ? oc : "", 0);
#endif
    len = strlen(match) + strlen(query) + strlen(searchattr);
    matchquery = calloc(1, len);
    ldap_build_filter(matchquery, len, match, NULL, NULL,
        searchattr, query, NULL);
    valwords[0] = oc;
    valwords[1] = matchquery;
    ldap_build_filter(buf, sizeof(buf), filtertemplate, "S=", NULL,
        searchattr, NULL, valwords);

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_PARSE,
      "process_search_form: match = %s, filtertemplate = %s\nmatchquery = %s, searchfilter = %s\n",
      match, filtertemplate, matchquery, buf);
#endif

    r->r_filter = buf;
    do_search(r, resp);

    return OK;
}

