/*
 * web500gw:  A LDAP based WWW - X.500 gateway
 *
 * written by: Frank Richter, Frank.Richter@hrz.tu-chemnitz.de
 *             Chemnitz University of Technology, Germany
 *
 * Copyright (c) 1994-8 Chemnitz University of Technology.
 * All rights reserved.
 *
 * $Id: read.c,v 1.2 2001/04/26 22:16:53 dekarl Exp $
 */

#include "web500gw.h"

/* 
 * do_read:  Read one entry, all or specified attributes, find template
 *           an print it with web500gw_entry2html
 */

int
do_read(
    REQUEST     *r,
    RESPONSE    *resp
)
{
    int         rc, i = 0, nonleaf = 0, in_home, vcard, monitor = 0;
    unsigned int    read_flags;
    char        **val, **ocl;
    char        *a, *rdn, *ufn, *newdn;
    static char *root_ocl[] = { "root", 0 };    /* pseudo root ocl */
    LDAPMessage *res, *e = (LDAPMessage *)0;
    struct timeval    timeout;
    struct ldap_disptmpl    *tmpl, *tmp;
   
    if ((a = strchr(r->r_dn, '?')) != NULL)
        *a = '\0';

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "do_read (%s, %s (", 
        r->r_dn, flag2string(r->r_flags), 0, 0);
    while (r->r_attrs && r->r_attrs[i]) {
        Web500gw_debug(WEB500GW_DEBUG_TRACE, "%s ", r->r_attrs[i], 0, 0, 0);
        i++;
    }
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "))\n", 0, 0, 0, 0);
#endif

    if (r->r_ld == (LDAP *)0 && 
        (r->r_ld = web500gw_ldap_init(r, resp, NULL, NULL, 1)) == (LDAP *)0) {
        return NOTOK;
    }
        
    vcard = r->r_flags & FLAG_VCARD;

    if (monitor_dn && strcmp(r->r_dn, monitor_dn) == 0) {
        monitor = 1;
        do_monitor(r, resp);
        r->r_flags = FLAG_ENTRYONLY | r->r_browser->b_flags;
    }

    if (*r->r_dn) {  /* not the root */
        /* read the entry itself */
        if (r->r_attrnumb) {      /* always read objectClass */
            r->r_attrs[r->r_attrnumb] = objectclass_attr;
            r->r_attrs[r->r_attrnumb + 1] = NULL;
        }
        timeout.tv_sec = timelimit;
        timeout.tv_usec = 0;
        if ((rc = ldap_search_st(r->r_ld, r->r_dn, LDAP_SCOPE_BASE, 
            default_filter, r->r_attrs, 0, &timeout, &res)) != LDAP_SUCCESS) {
            
            if (monitor)        /* ignore error */
                goto end;

            if (rc == LDAP_NO_SUCH_OBJECT && strlen(home_dn) > 0) {
                /* try to add home_dn -> could use short local DN's */

                newdn = malloc(strlen(r->r_dn) + strlen(home_dn) + 2);
                strcpy(newdn, r->r_dn);
                strcat(newdn, ",");
                strcat(newdn, home_dn);
#ifdef WEB500GW_DEBUG
                Web500gw_debug(WEB500GW_DEBUG_PARSE, 
                    "Adding HOME DN: %s -> %s\n", r->r_dn, newdn, 0, 0);
#endif
                rc = ldap_search_st(r->r_ld, newdn, LDAP_SCOPE_BASE,
                    default_filter, r->r_attrs, 0, &timeout, &res);
            }
        }
        if (rc != LDAP_SUCCESS) {
	    do_error(r, resp, rc, 0, get_ldap_error_str(r->r_ld), get_ldap_matched_str(r->r_ld));
            return NOTOK;
        }
        if ((e = ldap_first_entry(r->r_ld, res)) == NULL) {
            fprintf(stderr, "e = ldap_first_entry(r->r_ld, res)) == NULL\n");
            do_error(r, resp, get_ldap_result_code(r->r_ld), 0, get_ldap_error_str(r->r_ld), NULL);
            return NOTOK;
        }
        ocl = NULL;
        if (vcard) {
            ocl = vcard_ocl;
            nonleaf = 0;
        } else if (r->r_attrnumb && r->r_access &&
                   (r->r_access->a_rights & ACCESS_ATTRS)) {
            ocl = generic_ocl;
            nonleaf = isnonleaf(r->r_ld, ldap_get_values(r->r_ld, e, objectclass_attr),
                                    r->r_dn);
        } else {
            ocl = ldap_get_values(r->r_ld, e, objectclass_attr);
            nonleaf = isnonleaf(r->r_ld, ocl, r->r_dn);
        }
        r->r_dn = ldap_get_dn(r->r_ld, e);
        rdn = friendly_rdn(resp, r->r_dn, 1);
        ufn = friendly_dn(resp, r->r_dn);
    } else {
        rdn = MSG_ROOT;
        ufn = MSG_ROOT;
        ocl = root_ocl;
        nonleaf = 1;
    }

    if (resp->resp_httpheader == 0) {      /* No HTTP + HTML header yet! */
        if (r->r_httpversion == 1) {
            /* Check if_modified_since */
            if (*r->r_dn && lastmodified && !(r->r_flags & FLAG_NOCACHE) &&
                (val = ldap_get_values(r->r_ld, e, "lastModifiedTime")) != NULL) {
                resp->resp_last_modified = format_date(*val, DATE_FORMAT);
                if (r->r_if_modified_since && *(r->r_if_modified_since) &&
                    cmp_dates(*val, r->r_if_modified_since) != 1) {
                    resp->resp_status = NOT_MODIFIED;
#ifdef WEB500GW_DEBUG
                    Web500gw_debug(WEB500GW_DEBUG_PARSE, 
                        "Not modified since %s (%s)\n",
                        r->r_if_modified_since, *val, 0, 0);
#endif
                }
            }
            if (vcard) {
                resp->resp_content_type = CT_VCARD;
            }
            http_header(r, resp);
        }
        if (r->r_method == HEAD || resp->resp_status == NOT_MODIFIED) {
            return OK;
        }
    }
    in_home = isinhome(r->r_dn);

    if (resp->resp_htmlheader == 0 && ! vcard) {
        msg_fprintf(fp, (in_home && *MSG_HTML_START_ENTRY_HOME) ?
            MSG_HTML_START_ENTRY_HOME : MSG_HTML_START_ENTRY, "ss", rdn, ufn);
        resp->resp_htmlheader = 1;
    }
    read_flags = r->r_flags;  /* save flags - they may change in searchaction */
    if (!((read_flags & FLAG_ENTRYONLY) || vcard)) {
        if (in_home && MSG_HOMEBANNER) {
            msg_fprintf(fp, MSG_HOMEBANNER, "sss", r->r_query, 
                other_lang_select, other_lang_string);
        } else if (MSG_BANNER) {
            msg_fprintf(fp, MSG_BANNER, "sss", r->r_query, 
                other_lang_select, other_lang_string);
        }

        if (r->r_browser->b_upsearch & UPSEARCH_ON_TOP) {
            make_upsearch(r->r_ld, r, resp, nonleaf);
        }
    }
    if (!vcard)
        msg_fprintf(fp, MSG_ENTRY_HEADER, "ss", rdn, ufn);
    /* if not root free(ufn); */
    fflush(fp);
    /* not the root */
    if (r->r_template) {
        tmpl = ldap_name2template(r->r_template, r->r_access->a_tmpllist);
#ifdef WEB500GW_DEBUG
        if (tmpl == NULL) { /* user template not found */
            Web500gw_debug(WEB500GW_DEBUG_TRACE, 
                "do_read: user template %s not found, using generic\n", 
                 r->r_template, 0, 0, 0);
        }
#endif

    } else {
        tmpl = ldap_oc2template(ocl, r->r_access->a_tmpllist);
        if (read_flags &  FLAG_ALT) {  /* alternate attributes */
            for (tmp = tmpl; tmp != (struct ldap_disptmpl *)NULL; 
                tmp = tmp->dt_next) {
                if (LDAP_IS_DISPTMPL_OPTION_SET(tmp, LDAP_DTMPL_OPT_ALTVIEW)) {
                    tmpl = tmp;
                    break;
                }
            }
        }
    }

    if ((rc = web500gw_entry2html(r->r_ld, r, resp, e, tmpl, NULL, NULL,
              vcard ? 0 : LDAP_DISP_OPT_DOSEARCHACTIONS)) != LDAP_SUCCESS) {
            /* do_error(rc, 0); */
            return NOTOK;
    }
    if (vcard)
        return OK;
    if (read_flags & FLAG_ENTRYONLY) {    /* the entry only */
        fputs(MSG_HTML_END, fp);
        fputs("\n", fp);
        return OK;
    }

    if (!(r->r_browser->b_upsearch & UPSEARCH_ON_TOP)) {
        make_upsearch(r->r_ld, r, resp, nonleaf);
    }

end:
    if (in_home && MSG_HOMETRAILER) {
        fputs(MSG_HOMETRAILER, fp);
    } else {
        fputs(MSG_TRAILER, fp);
    }
    fputs(MSG_HTML_END, fp);
    fputs("\n", fp);
    return OK;
}


int
do_special (
    REQUEST     *r,
    RESPONSE    *resp
)
{
    int             rc, type, numb = 0, data_len = 0;
    struct berval   **val;
    char            cmd[128];
    char            *read_attrs[3], **date_val;
    char            *a, *s, *data;
    FILE            *op, *tp;
    LDAPMessage     *res, *e;
    struct timeval  timeout;
    struct stat     st;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "do_special (%s, %s)\n", r->r_dn, 
        (r->r_attrs && *r->r_attrs) ? *r->r_attrs : "No attributes!", 0, 0);
#endif

    if (!(r->r_attrs || *r->r_attrs))
        return NOTOK;
    if (r->r_ld == (LDAP *)0)
        r->r_ld = web500gw_ldap_init(r, resp, NULL, NULL, 1);

#if (defined(STR_TRANSLATION) && defined(LDAPVERSION) && LDAPVERSION > 31)
    /* avoid "T.61 conversion" for binary attributes */
    ldap_set_string_translators(r->r_ld, NULL, NULL);
#endif

    if (strncasecmp(*r->r_attrs, "jpegPhoto", 9) == 0) {
        if (!(r->r_browser->b_opts & B_JPEG) && jpegtogif == NULL) {
            return NOTOK;
        }
        type = ATTR_JPEG;
        if (r->r_attrs[0][9]) {
            numb = atoi(r->r_attrs[0] + 9);
            r->r_attrs[0][9] = '\0';
        }
    } else if (strncasecmp(*r->r_attrs, "photo", 5) == 0) {
        if (g3togif == NULL) {
            return NOTOK;
        }
        type = ATTR_G3FAX;
        if (r->r_attrs[0][5]) {
            numb = atoi(r->r_attrs[0] + 5);
            r->r_attrs[0][5] = '\0';
        }
    } else if (strncasecmp(*r->r_attrs, "audio", 5) == 0) {
        type = ATTR_AUDIO;
        if (r->r_attrs[0][5]) {
            numb = atoi(r->r_attrs[0] + 5);
            r->r_attrs[0][5] = '\0';
        }
    } else {
#ifdef WEB500GW_DEBUG
        Web500gw_debug(WEB500GW_DEBUG_TRACE, "do_special: Can't handle %s!\n",
            *r->r_attrs, 0, 0, 0);
#endif
        return NOTOK;
    }

    if (r->r_browser->b_opts & B_NEEDSUFFIX && 
        (strcasecmp(r->r_dn, ".gif") || strcasecmp(r->r_dn, ".xbm") ||
         strcasecmp(r->r_dn, ".jpg"))) {
        r->r_dn[strlen(r->r_dn)-4] = '\0';
    }
    if ((a = strchr(r->r_dn, '?')) != NULL) {
        *a++ = '\0';
    }
    read_attrs[0] = r->r_attrs[0];
    read_attrs[1] = "lastModifiedTime";
    read_attrs[2] = NULL;

    timeout.tv_sec = timelimit;
    timeout.tv_usec = 0;
    if ((rc = ldap_search_st(r->r_ld, r->r_dn, LDAP_SCOPE_BASE, default_filter,
        read_attrs, 0, &timeout, &res)) != LDAP_SUCCESS) {
        do_error(r, resp, rc, 0, get_ldap_result_code(r->r_ld), get_ldap_matched_str(r->r_ld));
        return NOTOK;
    }

    if ((e = ldap_first_entry(r->r_ld, res)) == NULL) {
        do_error(r, resp, get_ldap_result_code(r->r_ld), 0, NULL, NULL);
        return NOTOK;
    }

    /* Check r->r_if_modified_since */
    if ((date_val = ldap_get_values(r->r_ld, e, "lastModifiedTime")) != NULL) {
        resp->resp_last_modified = format_date(*date_val, DATE_FORMAT);
        if (!(r->r_flags & FLAG_NOCACHE) &&
            r->r_if_modified_since && *(r->r_if_modified_since) &&
            cmp_dates(*date_val, r->r_if_modified_since) != 1) {
            resp->resp_status = NOT_MODIFIED;
#ifdef WEB500GW_DEBUG
            Web500gw_debug(WEB500GW_DEBUG_PARSE, "Not modified since %s\n", 
                r->r_if_modified_since, 0, 0, 0);
#endif
            http_header(r, resp);
            return OK;
        }
    }

    if ((val = ldap_get_values_len(r->r_ld, e, *r->r_attrs)) == NULL)
        return NOTOK;
    if (val[numb] == NULL)
        return NOTOK;
    
    switch (type) {
    case ATTR_G3FAX:        /* g3fax photo -> gif */
        if (g3togif && *g3togif == '/') {

	  // XXX we apparently should be using mkstemp here, but that
	  // would require quite a bit of work, since we're using the
	  // generated temporary name with an external command.  we
	  // could handle it by calling mkstemp(), saving the result,
	  // forking, duping the file descriptor and setting stdout to
	  // it, execing the command, terminating the exec, and having
	  // the parent process then use the same file descriptor 

            tp = fopen(s = tempnam("/tmp", "web500"), "w+");
            sprintf(cmd, "%s > %s", g3togif, s);
            resp->resp_content_type = CT_IMG_GIF;
        }
        break;
    case ATTR_JPEG:         /* jpeg */
        if (r->r_browser->b_opts & B_JPEG) {    
            *cmd = '\0';
            resp->resp_content_type = CT_IMG_JPEG;
        } else if (jpegtogif && *jpegtogif == '/') {    /* jpeg -> gif */
            tp = fopen(s = tempnam("/tmp", "web500"), "w+");
            sprintf(cmd, "%s > %s", jpegtogif, s);
            resp->resp_content_type = CT_IMG_GIF;
        }
        break;
    case ATTR_AUDIO:
        *cmd = '\0';
        resp->resp_content_type = CT_SND_BASIC;
    }

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "%lu bytes %s : %s\n", 
        val[numb]->bv_len, resp->resp_content_type, cmd, 0);
#endif

    if (*cmd == '\0') {             /* just send raw data */
        resp->resp_content_length = (int)val[numb]->bv_len;
        data = val[numb]->bv_val;
        data_len = (int)val[numb]->bv_len;
    } else {                        /* convert data via cmd */
        if ((op = popen(cmd, "w")) == NULL)  {
            perror("Image conversion");
            return NOTOK;
        }
        fwrite(val[numb]->bv_val, val[numb]->bv_len, 1, op);
        pclose(op);
        if (stat(s, &st) != 0) {
            perror("Stat tempfile");
            return NOTOK;
        }
        resp->resp_content_length = (int)st.st_size;
#ifdef WEB500GW_DEBUG
        Web500gw_debug(WEB500GW_DEBUG_TRACE, "Data converted - size: %d\n", 
            (int)st.st_size, 0, 0, 0);
#endif
        if (r->r_method == GET) {
            data = malloc((int)st.st_size);
            data_len = fread(data, 1, (int)st.st_size, tp);
        }
        fclose(tp);
        if (unlink(s) == -1) {
#ifdef WEB500GW_DEBUG
            if (web500gw_debug) perror("Couldn't unlink temp image file!");
#endif
        }
    }

    if (r->r_httpversion == 1 && resp->resp_httpheader == 0) {
        http_header(r, resp);
        if (r->r_method == HEAD)
            return OK;
    }

    if (fwrite(data, data_len, 1, fp) > 0) {
        return OK;
    } else {
        return NOTOK;
    }
}


