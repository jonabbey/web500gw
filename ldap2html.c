/* 
 * ldap2html.c: display routines for HTML, based on:
 * tmplout.c:  display template library output routines for LDAP clients
 *             12 April 1994 by Mark C Smith
 *
 * $Id: ldap2html.c,v 1.2 2001/04/26 22:16:53 dekarl Exp $
 */

#include "web500gw.h"

static int searchaction();
static void print_out();

#define NONFATAL_LDAP_ERR(err)        (err == LDAP_SUCCESS || \
        err == LDAP_TIMELIMIT_EXCEEDED || err == LDAP_SIZELIMIT_EXCEEDED)

static char label_buf[4096];
static char value_buf[4096];

int
web500gw_entry2html(
    LDAP                    *ld,
    REQUEST                 *r,
    RESPONSE                *resp,
    LDAPMessage             *entry,
    struct ldap_disptmpl    *tmpl,
    char                    **defattrs,
    char                    ***defvals,
    unsigned long           opts
)
{
    int     i, err, show;
    int     freevals;
    int     oneline, form, addform, list, search, table, valsonly, vcard;
    int     in_table = 0, in_dl = 0;
    char    *dn, *f_label, **vals, filter[BUFSIZ];
    struct ldap_tmplitem    *rowp, *colp;
    struct ldap_adddeflist  *defaults;

    dn = strdup(r->r_dn);

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE,
        "web500gw_entry2html (%s, %s template, flags 0x%x = %s)\n", 
        dn, tmpl ? tmpl->dt_name : "generic", r->r_flags,
        flag2string(r->r_flags));
#endif

    /* a lot of error conditions ... */
    if (!dn || !r || !resp)
        return(LDAP_PARAM_ERROR);
    if (!(ld || (r->r_actions & ACTION_ADDFORM)))
        return(LDAP_PARAM_ERROR);

    if (tmpl == NULL) {
        if (r->r_actions & ACTION_ADDFORM || r->r_flags & FLAG_VCARD) {
            /* vCard needs vcard template, addform needs template! */
            return(LDAP_PARAM_ERROR);
        } else if (access_ok(r, resp, ACCESS_ATTRS) == NOTOK) {
            /* No generic template if not allowed to see other attributes */
            return (LDAP_OTHER);
        }
    }

    if (((r->r_flags & FLAG_LAYOUT) == FLAG_VALSONLY) ||
        !(r->r_flags & FLAG_LAYOUT || r->r_flags & FLAG_VCARD))
        /* use default browser flags */
        r->r_flags |= r->r_browser->b_flags;

    err = LDAP_SUCCESS;
    oneline  = r->r_flags & FLAG_ONELINE;
    list     = r->r_flags & FLAG_LIST;
    search   = r->r_flags & FLAG_FILTER;
    table    = r->r_flags & FLAG_TABLE;
    valsonly = r->r_flags & FLAG_VALSONLY;
    vcard    = r->r_flags & FLAG_VCARD;
    form     = r->r_actions & ACTION_FORM;
    addform  = r->r_actions & ACTION_ADDFORM;

    if (vcard)
        fputs ("BEGIN:vCard\n", fp);

    if (tmpl == NULL) {       /* use generic template */
        BerElement  *ber;
        char        *attr;

        fprintf (fp, "\n<!-- Using generic template, flags %s -->\n",
             flag2string(r->r_flags));
        ber = NULL;
        for (i = 0; i < r->r_attrnumb; i++) {
            if (table && in_table == 0) {
                if (form)
                    fprintf(fp, "\n<P>%s\n", r->r_flags & FLAG_BORDER ? 
                        MSG_TABLE_BORDER_FORM : MSG_TABLE_FORM);
                else
                    fprintf(fp, "\n<P>%s\n", r->r_flags & FLAG_BORDER ? 
                        MSG_TABLE_BORDER : MSG_TABLE);
                in_table = 1;
            } else if (list && in_dl == 0) {
                fputs("<DL>", fp);
                in_dl = 1;
            }

            attr = r->r_attrs[i];
/*
        for (attr = ldap_first_attribute(ld, entry, &ber);
                NONFATAL_LDAP_ERR(err) && attr != NULL;
                attr = ldap_next_attribute(ld, entry, ber)) 
 */
            label_buf[0] = '\0';
            value_buf[0] = '\0';
            if ((vals = ldap_get_values(ld, entry, attr)) == NULL) {
                freevals = 0;
#ifdef nodef
                if (defattrs != NULL) {
                    for (i = 0; defattrs[i] != NULL; ++i) {
                        if (strcasecmp(attr, defattrs[ i ]) == 0) {
                            break;
                        }
                    }
                    if (defattrs[i] != NULL) {
                        vals = defvals[i];
                    }
                }
#endif
            } else {
                freevals = 1;
            }

            if (islower(*attr)) {        /* cosmetic -- upcase attr. name */
                *attr = toupper(*attr);
            }

            err = web500gw_vals2html(ld, r, resp, dn, vals, attr,
                    LDAP_SYN_CASEIGNORESTR, 0, NULL);
            if (freevals) {
                ldap_value_free(vals);
            }
            print_out(r, resp);
            fflush(fp);
        }
        if (ber != NULL) {
            ber_free(ber, 1);
        }
    } else {        /* Not Generic template */
        if (! vcard)
            fprintf(fp, "\n<!-- Using template: %s, flags %s -->\n", 
                tmpl->dt_name, flag2string(r->r_flags));
        for (rowp = ldap_first_tmplrow(tmpl);
                NONFATAL_LDAP_ERR(err) && rowp != NULLTMPLITEM;
                rowp = ldap_next_tmplrow(tmpl, rowp)) {
            /* start of a line / row */ 

            if (table && in_table == 0) {
                if (form || addform)
                    fprintf(fp, "\n<P>%s\n", r->r_flags & FLAG_BORDER ? 
                        MSG_TABLE_BORDER_FORM : MSG_TABLE_FORM);
                else
                    fprintf(fp, "\n<P>%s\n", r->r_flags & FLAG_BORDER ? 
                        MSG_TABLE_BORDER : MSG_TABLE);
                in_table = 1;
            } else if (list && in_dl == 0) {
                fputs("<DL>", fp);
                in_dl = 1;
            }

            label_buf[0] = '\0';
            value_buf[0] = '\0';
            for (colp = ldap_first_tmplcol(tmpl, rowp); colp != NULLTMPLITEM;
                    colp = ldap_next_tmplcol(tmpl, rowp, colp)) {
                    /* same line / row */
                freevals = 0;

                if (r->r_flags & FLAG_ATTRSONLY) { /* only wanted attrs */
                    if (colp->ti_attrname == NULL)
                        continue;
                    i = 0;
                    show = 0;
                    while (i < r->r_attrnumb && r->r_attrs[i]) {
                        if (strcasecmp(r->r_attrs[i], 
                                       colp->ti_attrname) == 0) {
                            show = 1;
                            break;
                        }
                        i++;
                    }
                    if (! show)
                        continue;
                }

                if (*label_buf && !vcard)
                    /* add attribute label, but not in vcard */
                    sprintf(label_buf + strlen(label_buf), ", ");
                if (*value_buf)
                    sprintf(value_buf + strlen(value_buf), vcard ? "," : " ");
                vals = NULL;

                if (addform) {      /* looking for default values */
                    if (colp->ti_attrname != NULL) {
                        i = 0;
                        for (defaults = tmpl->dt_adddeflist; 
                             defaults && i < BUFSIZ;
                             defaults = defaults->ad_next) {
                            if (strcasecmp(colp->ti_attrname,
                                        defaults->ad_attrname) == 0) {
                                if (!vals)
                                    vals = (char **)calloc(BUFSIZ, sizeof(char *));
    
                                if (defaults->ad_source == LDAP_ADSRC_CONSTANTVALUE) {
                                    vals[i++] = defaults->ad_value;
                                } else if (defaults->ad_source == LDAP_ADSRC_ADDERSDN) {
                                    vals[i++] = r->r_binddn;
                                }
                            }
                        }
                        if (vals)
                            vals[i] = NULL;
                    }
                } else if (colp->ti_attrname == NULL || 
                    (vals = ldap_get_values(ld, entry, colp->ti_attrname)) == NULL) {
#ifdef nodef
                    if (!LDAP_IS_TMPLITEM_OPTION_SET(colp,
                            LDAP_DITEM_OPT_HIDEIFEMPTY) && defattrs != NULL
                            && colp->ti_attrname != NULL) {
                        for (i = 0; defattrs[i] != NULL; ++i) {
                            if (strcasecmp(colp->ti_attrname, defattrs[i])
                                    == 0) {
                                break;
                            }
                        }
                        if (defattrs[i] != NULL) {
                            vals = defvals[i];
                        }
                    }
#endif
                } else {
                    freevals = 1;
                    if (LDAP_IS_TMPLITEM_OPTION_SET(colp,
                            LDAP_DITEM_OPT_SORTVALUES) && vals[0] != NULL
                            && vals[1] != NULL) {
                        ldap_sort_values(ld, vals, ldap_sort_strcasecmp);
                    }
                }

                show = form || addform || (vals != NULL && vals[0] != NULL);

                /* don't print empty rows or so for the following cases */
                if ((!(addform || form)) && show 
                    && LDAP_GET_SYN_TYPE(colp->ti_syntaxid) == LDAP_SYN_TYPE_BOOLEAN
                    && LDAP_IS_TMPLITEM_OPTION_SET(colp, LDAP_DITEM_OPT_HIDEIFFALSE) 
                    && (vals != NULL && vals[0] != NULL 
                    &&  toupper(vals[0][0]) != 'T')) {

                    /* hide boolean attr if false */
                    show = 0;
                }
                if (form && (! vals &&
                    LDAP_IS_TMPLITEM_OPTION_SET(colp,LDAP_DITEM_OPT_READONLY)))
                    /* don't show: form and no value and read only */
                    show = 0;
                if ((form || addform) &&
                    (colp->ti_syntaxid == LDAP_SYN_SEARCHACTION ||
                     colp->ti_syntaxid == LDAP_SYN_LINKACTION))
                     /* don't show: form and actions */
                    show = 0;

                if (! (form || addform || vcard)) {
                    /* no link or search in a form or vcard */
                    if (colp->ti_syntaxid == LDAP_SYN_SEARCHACTION) {
                        /* search action */
                        if ((opts & LDAP_DISP_OPT_DOSEARCHACTIONS) != 0) {
                            if (colp->ti_attrname == NULL || 
                                (show && toupper(vals[0][0]) == 'T')) {
                                fputs(list ? "</DL>" : 
                                   (table ?  "</TABLE>" : ((*dn) ?  "<P>" : "")),
                                      fp);
                                fflush(fp);
                                err = searchaction(ld, r, resp, entry, colp);
                                in_table = in_dl = 0;
                            }
                        }
                        show = 0;
                        label_buf[0] = '\0';
                        value_buf[0] = '\0';
                    } else if (colp->ti_syntaxid == LDAP_SYN_LINKACTION) {
                        /* link action */
                        if (colp->ti_args && colp->ti_args[0]) {
                            if (colp->ti_args[0][0] == '?') {
                                /* full flags/search specified */
                                strcpy(filter, colp->ti_args[0]);
                            } else {
                                sprintf(filter, "?%s", colp->ti_args[0]);
                            }
                        } else {
                            filter[0] = '\0';
                        }
                        f_label = friendly_label(resp, colp->ti_label);
                        msg_snprintf(label_buf+strlen(label_buf),
                            sizeof(label_buf) - strlen(label_buf),
                            MSG_LINKACTION, "ss",
                            dn2url(r, dn, FLAG_LANGUAGE, 0, filter, NULL), 
                            f_label);
                        show = 0;
                    }
                }
                if (show) {
                    err = web500gw_vals2html(ld, r, resp, dn, vals, 
                            colp->ti_label, colp->ti_syntaxid, colp);
                }
                if (freevals) {
                    ldap_value_free(vals);
                }
            }
            if (show != 0 || (*label_buf != '\0' || *value_buf != '\0'))
                print_out(r, resp);
            /* end of a line */
        }
    }
    if (vcard) {
        fprintf(fp, "PRODID:%s\nEND:vCard\n", version);
    } else {
        fputs(table && in_table ? "</TABLE>" : (list && in_dl ? "</DL>" : ""),
            fp);
    }
    free(dn);
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "web500gw_entry2html ><\n", 
        0, 0, 0, 0);
#endif
    return(err);
}

void
web500gw_list2html(
    LDAP            *ld,
    REQUEST         *r,
    RESPONSE        *resp,
    struct dncompare **dnlist,
    int             count
)
{
    struct ldap_disptmpl    *tmpl, *default_tmpl;
    struct ldap_tmplitem    *rowp, *colp, *tmplitem;
    int     list, oneline, search, table, valsonly, nodn;
    int     i, j, err, first, firstval, show;
    char    *attr, *foc, *cp, **vals;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "web500gw_list2html (%d, %d, %s)\n", 
        count, r->r_attrnumb, flag2string(r->r_flags), 0);
#endif

    if (((r->r_flags & FLAG_LAYOUT) == FLAG_VALSONLY) ||
        !(r->r_flags & FLAG_LAYOUT)) {          /* use default flags */
        r->r_flags |= r->r_browser->b_flags | FLAG_VALSONLY;
        if (r->r_flags & FLAG_FILTER) {      /* with search filter */
            r->r_flags |= FLAG_ATTRSONLY;
        }
    }
    list     = r->r_flags & FLAG_LIST;
    oneline  = r->r_flags & FLAG_ONELINE;
    search   = r->r_flags & FLAG_FILTER;
    table    = r->r_flags & FLAG_TABLE;
    valsonly = r->r_flags & FLAG_VALSONLY;
    nodn     = r->r_flags & FLAG_NODN;

    if (r->r_attrnumb) {     /* with user attrs */
        if ((default_tmpl = ldap_oc2template(generic_ocl, 
                r->r_access->a_tmpllist)) == NULL) {
#ifdef WEB500GW_DEBUG
            if (web500gw_debug)
                fprintf(stderr, "\"Generic\" template entry in %s missed!\n", 
                    templatefile);
#endif
            fprintf(fp, "Server error: \"Generic\" template entry in %s missed!\n", 
                templatefile);
            return;
        }
    }

    fprintf (fp, "\n<!-- Using display flags %s -->\n",
        flag2string(r->r_flags));

    if (table) {
        fprintf (fp, "\n<P>%s\n", 
            r->r_flags & FLAG_BORDER ? MSG_TABLE_BORDER : MSG_TABLE);
    } else if (list) {
        fputs("<DL>", fp);
    }
    fflush(fp);
    foc = ""; 

    for (i = 0; i < count ; i++) {
        if (strcmp(dnlist[i]->friendly_oc, foc)) {
            /* new template/objectclass */
            first = 1;
            if (table) {
                if (! first)
                    fputs("</TD>\n</TR>\n", fp);
                if (r->r_attrnumb) {
                    fprintf(fp, MSG_OC_NAME_TR_COLS, 
                        valsonly ? r->r_attrnumb + (nodn ? 0 : 1) 
                                 : 2 * r->r_attrnumb + 1);
                    fprintf(fp, MSG_OC_NAME, dnlist[i]->friendly_oc);
                    fputs("</TD></TR>\n", fp);
                } else {
                    fputs(MSG_OC_NAME_TR, fp);
                    fprintf(fp, MSG_OC_NAME, dnlist[i]->friendly_oc);
                    fputs("</TD>\n", fp);
                    fputs(MSG_ATTR_VAL_TD, fp);
                }
            } else if (list) {
                fprintf(fp, "%s\n<DT>", (i > 0) ? "</DL>" : "");
                fprintf(fp, MSG_OC_NAME, dnlist[i]->friendly_oc);
                fputs("\n<DD><DL>\n", fp);
            } else {
                fputs("<BR><BR>\n", fp);
                fprintf(fp, MSG_OC_NAME, dnlist[i]->friendly_oc);
            }
            foc = dnlist[i]->friendly_oc;

            if (r->r_attrnumb) {
                tmpl = dnlist[i]->tmpl ? dnlist[i]->tmpl : default_tmpl;
                fprintf(fp, "<!-- Using template: %s -->\n", tmpl->dt_name);

                if (table && valsonly) {
                /* attributes && values only -> Table cell with attr name */
                    fputs("<TR>", fp);
                    if (! nodn)
                        fputs("<TH><BR></TH>", fp);
                    for (j = 0; j < r->r_attrnumb; j++) {
                        cp = r->r_attrs[j];
                        tmplitem = NULLTMPLITEM;
                        for (rowp = ldap_first_tmplrow(tmpl); 
                            rowp != NULLTMPLITEM && tmplitem == NULLTMPLITEM;
                            rowp = ldap_next_tmplrow(tmpl, rowp)) {
                            for (colp = ldap_first_tmplcol(tmpl, rowp);
                                colp != NULLTMPLITEM;
                                colp = ldap_next_tmplcol(tmpl, rowp, colp)) {
                                if (colp->ti_attrname && 
                                    strcasecmp(colp->ti_attrname, cp) == 0) {
                                    tmplitem = colp;
                                    break;
                                }
                            }
                        }
                        if (tmplitem != NULLTMPLITEM && tmplitem->ti_label) {
                            cp = friendly_label(resp, tmplitem->ti_label);
                        } else {
                            cp = "<BR>";
                        }
                        fprintf(fp, MSG_ATTR_NAME_TH_SEARCH, cp);
                    }
                    fputs("</TR>\n", fp);
                }
            }
        }
        if (table && r->r_attrnumb)
            fputs("\n<TR>", fp);
        if (! nodn) {
            /* print HREF-DN */
            if (table) {
                if (r->r_attrnumb) {
                    fputs(MSG_ATTR_VAL_TD, fp);
                } else {
                    fputs((first ? "" : (oneline ? ", " : "<BR>\n")), fp);
                }
            } else if (list) {
                fputs((oneline && r->r_attrnumb == 0 ? 
                    (!first ? ", " : "") : "\n<DT>"), fp);
            } else {
                fputs((oneline && r->r_attrnumb == 0 ? 
                    (!first ? ", " : "") : "<BR>\n"), fp);
            }
            fprintf(fp, MSG_ATTR_VAL, dnlist[i]->href);
        

            if (table && r->r_attrnumb)
                fputs("</TD>\n", fp);
        }
        first = 0;
        fflush(fp);
        if (r->r_attrnumb) {     /* show these attrs */
            j = 0;
            firstval = 1;
            fputs((list) ? "\n<DD>" : "", fp);
            while ((attr = r->r_attrs[j++])) {
                show = 0;
                label_buf[0] = '\0';
                value_buf[0] = '\0';

                /* read value(s) for all attrs to show */
                if ((vals = ldap_get_values(ld, dnlist[i]->entry, attr)) != NULL) {
                    /* if there's a value look how to display */
                    tmplitem = NULLTMPLITEM;
                    for (rowp = ldap_first_tmplrow(tmpl);
                        rowp != NULLTMPLITEM && tmplitem == NULLTMPLITEM;
                        rowp = ldap_next_tmplrow(tmpl, rowp)) {
                        for (colp = ldap_first_tmplcol(tmpl, rowp); 
                            colp != NULLTMPLITEM;
                            colp = ldap_next_tmplcol(tmpl, rowp, colp)) {
                            if (colp->ti_attrname && 
                                strcasecmp(colp->ti_attrname, attr) == 0) {
                                tmplitem = colp;
                                break;
                            }
                        }
                    }
                    if ((!firstval) && list && valsonly) {
                        if (oneline)
                            fputs(", ", fp);
                        else 
                            fputs("<BR>\n", fp);
                    }
                    firstval = 0;
                    if (tmplitem != NULLTMPLITEM) {
                        show = 1;
                        err = web500gw_vals2html(ld, r, resp, dnlist[i]->dn,
                                vals, tmplitem->ti_label, 
                                tmplitem->ti_syntaxid, r->r_flags, tmplitem);
                    } else {    /* nothing found */
#ifdef nodef
                        if (islower(*attr)) { 
                            /* cosmetic - upcase attr. name */
                            *attr = toupper(*attr);
                        }
                        err = web500gw_vals2html(ld, r, resp, dnlist[i]->dn, 
                          vals, attr, LDAP_SYN_CASEIGNORESTR, r->r_flags, NULL);
#endif
                    }
                } else {    /* No directory values but maybe link action */
                    tmplitem = NULLTMPLITEM;
                    for (rowp = ldap_first_tmplrow(tmpl);
                         rowp != NULLTMPLITEM && tmplitem == NULLTMPLITEM;
                         rowp = ldap_next_tmplrow(tmpl, rowp)) {
                        for (colp = ldap_first_tmplcol(tmpl, rowp);
                             colp != NULLTMPLITEM;
                             colp = ldap_next_tmplcol(tmpl, rowp, colp)) {
                            if (colp->ti_attrname &&
                                strcasecmp(colp->ti_attrname, attr) == 0 &&
                                colp->ti_syntaxid == LDAP_SYN_LINKACTION) {

                                show = 1;
                                msg_snprintf(value_buf+strlen(value_buf),
                                    sizeof(value_buf) - strlen(value_buf),
                                    MSG_LINKACTION, "ss",
                                    dn2url(r, dnlist[i]->dn, FLAG_LANGUAGE, 0, 
                                    colp->ti_args[0], NULL),
                                    friendly_label(resp, colp->ti_label));
                                break;
                            }
                        }
                    }
                }
                if (show && (!valsonly || strlen(value_buf) > 0)) {
                    if (table) {
                        if (!valsonly) {
                            fputs(search ? MSG_ATTR_NAME_TD_SEARCH : 
                                           MSG_ATTR_NAME_TD, fp);
                            msg_fprintf(fp, MSG_ATTR_NAME, "ss",
                                label_buf, search ? ":" : "");
                            fputs(search ? " " : "</TD>", fp);
                        }
                        fputs(MSG_ATTR_VAL_TD, fp);
                        /* fprintf(fp, "</TD ddd>%s", MSG_ATTR_VAL_TD); */
                        fprintf(fp, MSG_ATTR_VAL, value_buf);
                        fputs("</TD>\n", fp);
                    } else if (list) {
                        if (!valsonly) {
                            fputs("\n<DT>", fp);
                            msg_fprintf(fp, MSG_ATTR_NAME, "ss",
                                label_buf, "");
                            fputs("\n<DD>", fp);
                        }
                        fprintf(fp, MSG_ATTR_VAL, value_buf);
                    } else {
                        fputs(" ", fp);
                        if (!valsonly)
                            msg_fprintf(fp, MSG_ATTR_NAME, "ss",
                                label_buf, *value_buf ? ":" : "");
                        fputs(" ", fp);
                        fprintf(fp, MSG_ATTR_VAL, value_buf);
                        /* fprintf(fp, "%s%s\n", value_buf, oneline ? "" : "<BR>"); */
                    }
                } else {
                    if (table) {
                        fputs(MSG_ATTR_VAL_TD, fp);
                        fputs("<BR></TD>", fp);
                    }
                }
            }
            fputs(table ? "</TR>" : "", fp);
        }
        fflush(fp);
        free(dnlist[i]->sortstring);
        free(dnlist[i]->href);
    }
    fputs(table ? "\n</TABLE>\n" : (list ? "</DL></DL>\n" : "<BR>\n"), fp);
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "web500gw_list2html ><\n", 
        0, 0, 0, 0);
#endif
}

static void
print_out (
    REQUEST         *r,
    RESPONSE        *resp
)
{
    int     oneline, form, addform, list, search, table, valsonly, vcard;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, " print_out (0x%x)\n", 
        r->r_flags, 0, 0, 0);
#endif

    oneline  = r->r_flags & FLAG_ONELINE;
    list     = r->r_flags & FLAG_LIST;
    search   = r->r_flags & FLAG_FILTER;
    table    = r->r_flags & FLAG_TABLE;
    valsonly = r->r_flags & FLAG_VALSONLY;
    vcard    = r->r_flags & FLAG_VCARD;
    form     = r->r_actions & ACTION_FORM;
    addform  = r->r_actions & ACTION_ADDFORM;

    if (list) {
        if (!valsonly && *label_buf) {
            fputs("\n<DT>", fp);
            msg_fprintf(fp, MSG_ATTR_NAME, "ss", label_buf, "");
        }
        fputs("\n<DD>", fp);
        fputs(value_buf, fp);
    } else if (table) {
        fputs("<TR>", fp);
        if (!valsonly) {
            fputs(search ? MSG_ATTR_NAME_TD_SEARCH : MSG_ATTR_NAME_TD, fp);
            msg_fprintf(fp, MSG_ATTR_NAME, "ss", label_buf, search ? ":" : "");
        }
        fputs(search ? " " : "</TD>", fp);
        fputs(search ? "":  MSG_ATTR_VAL_TD, fp);
        fprintf(fp, MSG_ATTR_VAL, value_buf);
        fputs("</TD></TR>\n", fp);
    } else if (vcard) {
        if (value_buf && *value_buf)
            fprintf(fp, "%s:%s\n", label_buf, value_buf);
    } else if (!valsonly) {
        msg_fprintf(fp, MSG_ATTR_NAME, "ss", label_buf, *value_buf ? ":" : "");
        fprintf(fp, "%s%s\n", value_buf, "<BR>");
    }
    fflush(fp);
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, " print_out ><\n", 0, 0, 0, 0);
#endif
}


int
web500gw_vals2html (
        LDAP                    *ld,
        REQUEST                 *r,
        RESPONSE                *resp,
        char                    *dn,
        char                    **vals,
        char                    *label,
        unsigned long           syntaxid,
        struct ldap_tmplitem    *ti
)
{
    int     i, l, writeoutval = 0, syntax, line = 0, noval = 0, ignored;
    int     maxlen = 0, urllen = 0, image = 0, freevals = 0, valcount;
    int     labellen = 0;
    int     oneline, form, addform, list, search, table, valsonly, vcard;
    char    *p, *s, *t, *outval = NULL, buffer[BUFSIZ], *f_label;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL,
        " web500gw_vals2html (%s, %s, 0x%x)\n", dn, label, r->r_flags, 0);
#endif

    list     = r->r_flags & FLAG_LIST;
    search   = r->r_flags & FLAG_FILTER;
    table    = r->r_flags & FLAG_TABLE;
    valsonly = r->r_flags & FLAG_VALSONLY;
    vcard    = r->r_flags & FLAG_VCARD;
    oneline  = r->r_flags & FLAG_ONELINE || vcard;
    form     = r->r_actions & ACTION_FORM && ti &&
               (!LDAP_IS_TMPLITEM_OPTION_SET(ti, LDAP_DITEM_OPT_READONLY));
    addform  = r->r_actions & ACTION_ADDFORM;

    syntax = LDAP_GET_SYN_TYPE(syntaxid);
    if (!vals) {
        if ((form || addform) && 
            syntax != LDAP_SYN_TYPE_IMAGE && syntax != LDAP_SYN_TYPE_BUTTON)
            noval = 1;   /* no value for this attr., but add an empty field */
        else
            return LDAP_SUCCESS;
    }
    if (vcard && syntax != LDAP_SYN_TYPE_TEXT) {
        return(LDAP_SUCCESS);
    } else {
        switch(syntax) {
        case LDAP_SYN_TYPE_IMAGE:
        case LDAP_SYN_TYPE_BUTTON:
        case LDAP_SYN_TYPE_TEXT:
        case LDAP_SYN_TYPE_BOOLEAN:
            break;                /* we only bother with these types... */
        default:
            return(LDAP_SUCCESS);
        }
    }
    /* print Attribute name */
    if (vcard) {        /* don't map vCard attribut names from template file */
        f_label = label;
    } else {
        f_label = friendly_label(resp, label);
    }
    if (form || addform) {
        if (syntax != LDAP_SYN_TYPE_TEXT && syntax != LDAP_SYN_TYPE_BOOLEAN)
            return(LDAP_SUCCESS);

        msg_snprintf(label_buf+strlen(label_buf), 
            sizeof(value_buf) - strlen(value_buf), MSG_ATTR_HELP, "sss", 
            f_label, ti->ti_attrname,
            LDAP_IS_TMPLITEM_OPTION_SET(ti, LDAP_DITEM_OPT_VALUEREQUIRED) ?
            MSG_ATTR_REQUIRED : "");

        if (form) {
            for (i = 0; vals != NULL && vals[i] != NULL; ++i) {
                if ((l = strlen(vals[i])) > maxlen)
                    maxlen = l;
            }
            valcount = i;
            maxlen += 3;
            if (maxlen < 30)
                maxlen = 30; 
        } else {
            valcount = 0;
            maxlen = 30;
        }
    } else if (syntax != LDAP_SYN_TYPE_BUTTON && !(vcard && *label_buf)) {
        /* don't add additional attribute label in vcard */
        strcat(label_buf, f_label);
    }
    for (i = 0; vals != NULL && vals[i] != NULL; ++i) {
        if (i > 0 && vcard && 
            LDAP_IS_TMPLITEM_OPTION_SET(ti, LDAP_DITEM_OPT_SINGLEVALUED)) {
            /* in vcard - if 1val - show only one value */
            return(LDAP_SUCCESS);
        }
        if (syntax == LDAP_SYN_TYPE_TEXT) {
            if (vcard || syntaxid == LDAP_SYN_DN) {
                outval = vals[i];
                freevals = 0;
            } else {
                outval = html_encode(web500gw_t61toiso(vals[i]));
                freevals = 1;
            }
        }
        if (i > 0 && !ignored) {
         /* if ((!list && !table) || oneline) */
            sprintf(value_buf+strlen(value_buf), 
                oneline ? (vcard ? ";" : ", "): "<BR>\n");
                /* vcard should be comma, but Netscape doesn't recognizes it */
        }
        writeoutval = 0;        /* if non-zero, write outval after switch */
        ignored = 0;

        switch(syntaxid) {
        case LDAP_SYN_CASEIGNORESTR:
            ++writeoutval;
            break;

        case LDAP_SYN_RFC822ADDR:
            if (form || addform || vcard) {
                ++writeoutval;
            } else {
                if (r->r_browser->b_opts & B_MAILTO) {
                    sprintf(value_buf+strlen(value_buf), 
                        "<A HREF=\"mailto:%s\">%s</A>", outval, outval);
                } else {
#ifdef MAIL_CMD
                    sprintf(value_buf+strlen(value_buf), MAIL_CMD, outval);
#else
                    ++writeoutval;
#endif
                }
            }
            break;

        case LDAP_SYN_DN:
            if (form || addform)
                ++writeoutval;
            else
                sprintf(value_buf+strlen(value_buf), "<A HREF=\"%s\">%s</A>",
                    dn2url(r, outval, FLAG_LANGUAGE, 0, NULL, NULL), 
                    friendly_dn(resp, outval));
            break;

        case LDAP_SYN_MULTILINESTR:
            p = s = outval;
            line = 0;
            buffer[0] = '\0';
            while ((t = s = strchr(s, '$')) != NULL) {
                *s++ = '\0';
                while (isspace(*--t)) *t = '\0';
                while (isspace(*s)) ++s;
                if (form) {
                    if (line++ > 0)         /* not first line */
                        strcat(buffer, "\r\n");
                    strcat(buffer, p);
                } else {
                    if (line++ > 0)
                        strcat(buffer, ((list || table) && !oneline) ? "<BR>\n" :
                            (vcard ? ";" : ", "));
                    strcat(buffer, p);
                }
                p = s;
            }
            if (form || addform) {
                if (line++ > 0)
                    strcat(buffer, "\r\n");
                strcat(buffer, p);
                s = buffer;
                sprintf(value_buf+strlen(value_buf), "\
<TEXTAREA NAME=\"%s\" COLS=30 ROWS=%d>%s</TEXTAREA><BR>\n",
                    ti->ti_attrname, line + 1, s);
            } else {
                if (line++ > 0)
                    strcat(buffer, ((list || table) && !oneline) ?  "<BR>\n" : 
                        (vcard ? ";" : ", "));
                strcat(value_buf, buffer);
                strcat(value_buf, p);
            }
            break;

        case LDAP_SYN_BOOLEAN:

            /* ??? 
            if (addform && 
                (strncmp(vals[i], "edit:", 5) == 0 ||
                 strncmp(vals[i], "change:", 7) == 0)) {
                 outval = vals[i];
            } else {
            */
            if (addform || form)
                outval = vals[i];
            else
                outval = toupper(*vals[i]) == 'T' ? MSG_TRUE : MSG_FALSE;

            ++writeoutval;
            break;

        case LDAP_SYN_TIME:
            outval = format_date(outval, DATE_FORMAT);
            ++writeoutval;
            break;
        case LDAP_SYN_DATE:
            outval = format_date(outval, DATE_ONLY);
            ++writeoutval;
            break;

        case LDAP_SYN_LABELEDURL:
            if (gwswitch && strstr (outval, "(gw")) {
                ignored = 1;
#ifdef WEB500GW_DEBUG
                Web500gw_debug(WEB500GW_DEBUG_UTIL,
                    " web500gw_vals2html: Ignoring labeledURI GW-switch: %s\n",
                    outval, 0, 0, 0);
#endif
                break;
            }

            if ((p = strchr(outval, '$')) != NULL) {   
                /* old style: label $ URL */
                *p++ = '\0';
                while (isspace(*p)) {
                    ++p;
                }
                s = outval;
            } else if ((s = strchr(outval, ' ')) != NULL) {    
                /* new labelledURI: URI label */
                *s++ = '\0';
                while (isspace(*s)) ++s;
                p = outval;
            } else {
                s = form ? "" : MSG_URL;
                p = outval;
            }

            /*
             * at this point `s' points to the label & `p' to the URL
             */
            if (form || addform) {
                /* s = html_encode(s); */
                labellen = strlen(s);
                /* p = html_encode(p); */
                urllen = strlen(p);
                sprintf(value_buf+strlen(value_buf), "\
%s: <INPUT NAME=\"%s\" SIZE=\"%d\" VALUE=\"%s\"><BR> \
%s: <INPUT NAME=\"ext\" SIZE=\"%d\" VALUE=\"%s\"><BR>\n",
                    MSG_URL, ti->ti_attrname, 35, p, MSG_URL_INFO, 35, s);
            } else if (vcard) {
                strcat(value_buf, p);
            } else {
                sprintf(value_buf+strlen(value_buf), "<A HREF=\"%s\">%s</A>",
                     p, s);
            }
            break;
        case LDAP_SYN_JPEGIMAGE:
            if (form || addform || vcard)       /* for now ... */
                return(LDAP_SUCCESS);
            if (r->r_browser->b_opts & B_IMG) {
                /* IMG SRC -> buffer */
                sprintf(buffer, "%s?jpegPhoto", url_encode(dn));
                if (image) 
                    sprintf(buffer+strlen(buffer), "%d", image);
                image++;
                if (r->r_browser->b_opts & B_NEEDSUFFIX)
                    strcat(buffer, r->r_browser->b_opts & B_JPEG ? 
                                        ".jpg" : ".gif");

                msg_snprintf(value_buf+strlen(value_buf),
                    sizeof(value_buf) - strlen(value_buf), MSG_IMG, "sss",
                    buffer, "JPEG", f_label);
                break;
            }
            /* browser can't handle inline IMG - make button */
        case LDAP_SYN_JPEGBUTTON:
            if (form || addform || vcard)
                return(LDAP_SUCCESS);
            sprintf(valsonly ?
                value_buf+strlen(value_buf) : label_buf+strlen(label_buf), 
                "<A HREF=\"%s?jpegPhoto%s\">%s</A>\n",
                url_encode(dn), r->r_browser->b_opts & B_NEEDSUFFIX ?
                (r->r_browser->b_opts & B_JPEG ? ".jpg" : ".gif") : "", 
                f_label ? f_label : "Photo (JPEG)");
            break;
        case LDAP_SYN_FAXIMAGE:
            if (form || addform || vcard || g3togif == NULL)
                return(LDAP_SUCCESS);
            if (r->r_browser->b_opts & B_IMG) {
                /* IMG SRC -> buffer */
                sprintf(buffer, "%s?photo%s", url_encode(dn),
                    (r->r_browser->b_opts & B_NEEDSUFFIX) ? ".xbm" : "");
                msg_snprintf(value_buf+strlen(value_buf), 
                    sizeof(value_buf) - strlen(value_buf), "sss", MSG_IMG,
                    buffer, "G3Fax", f_label);
                break;
            }
            /* browser can't handle inline IMG - make button */
        case LDAP_SYN_FAXBUTTON:
            if (form || addform || vcard || g3togif == NULL)
                return(LDAP_SUCCESS);
            sprintf(valsonly ? 
                value_buf+strlen(value_buf) : label_buf+strlen(label_buf), 
                "<A HREF=\"%s?photo%s\">%s</A>\n", url_encode(dn),
                r->r_browser->b_opts & B_NEEDSUFFIX ? ".xbm" : "",
                f_label);
            break;
        case LDAP_SYN_AUDIOBUTTON:
            if (form || addform || vcard)
                return(LDAP_SUCCESS);
            sprintf(valsonly ? 
                value_buf+strlen(value_buf) : label_buf+strlen(label_buf), 
                "<A HREF=\"%s?audio%s\">%s</A>\n", url_encode(dn),
               r->r_browser->b_opts & B_NEEDSUFFIX ? ".au" : "",
               f_label);
            break;
        default:
            sprintf(value_buf+strlen(value_buf), " Can't display item type %ld!", syntaxid);
            break;
        }

        if (writeoutval) {
            if (form) {
                if (syntaxid == LDAP_SYN_BOOLEAN) {
                    sprintf(value_buf+strlen(value_buf),
    "<INPUT TYPE=\"radio\" NAME=\"%s\" VALUE=\"T\" %s> %s \
     <INPUT TYPE=\"radio\" NAME=\"%s\" VALUE=\"F\" %s> %s\n",
                    ti->ti_attrname, *outval == 'T' ? "CHECKED" : "", MSG_TRUE,
                    ti->ti_attrname, *outval != 'T' ? "CHECKED" : "", MSG_FALSE);
                } else { 
                    sprintf(value_buf+strlen(value_buf),
                        "<INPUT NAME=\"%s\" SIZE=\"%d\" VALUE=\"%s\">\n",
                        ti->ti_attrname, maxlen, outval);
                }
            } else if (addform && strncmp(outval, "edit:", 5) == 0) {
                if (syntaxid == LDAP_SYN_BOOLEAN) {
                    sprintf(value_buf+strlen(value_buf),
    "<INPUT TYPE=\"radio\" NAME=\"%s\" VALUE=\"T\" %s> %s \
    <INPUT TYPE=\"radio\" NAME=\"%s\" VALUE=\"F\" %s> %s\n",
                    ti->ti_attrname, *(outval + 5) == 'T' ? "CHECKED" : "", MSG_TRUE,
                    ti->ti_attrname, *(outval + 5) != 'T' ? "CHECKED" : "", MSG_FALSE);
                } else { 
                    sprintf(value_buf+strlen(value_buf),
                        "<INPUT NAME=\"%s\" SIZE=\"%d\" VALUE=\"%s\">\n",
                        ti->ti_attrname, maxlen, outval + 5);
                }
            } else if (addform && strncmp(outval, "change:", 7) == 0) {
                if (syntaxid == LDAP_SYN_BOOLEAN) {
                    sprintf(value_buf+strlen(value_buf),
    "<INPUT TYPE=\"radio\" NAME=\"%s\" VALUE=\"T\" %s> %s \
    <INPUT TYPE=\"radio\" NAME=\"%s\" VALUE=\"F\" %s> %s\n",
                    ti->ti_attrname, *(outval + 7) == 'T' ? "CHECKED" : "", MSG_TRUE,
                    ti->ti_attrname, *(outval + 7) != 'T' ? "CHECKED" : "", MSG_FALSE);
                } else { 
                    sprintf(value_buf+strlen(value_buf),
                        "<INPUT NAME=\"%s\" SIZE=\"%d\" VALUE=\"%s\">\n",
                        ti->ti_attrname, maxlen, outval + 7);
                }
            } else {
                    /* short values have line breaks in TABLE,
                       but adding NOBR may be dangerous
                if (strlen(outval) < 0) {
                    strcat(value_buf, "<NOBR>");
                    strcat(value_buf, outval);
                    strcat(value_buf, "</NOBR>");
                } else {
                */

                strcat(value_buf, outval);

                /* } */
            }
        }
    }
    if ((addform && noval) ||
        (form && (maxvalues == 0  || valcount < maxvalues) && 
        (noval || ((syntaxid != LDAP_SYN_BOOLEAN) &&
        (!LDAP_IS_TMPLITEM_OPTION_SET(ti, LDAP_DITEM_OPT_SINGLEVALUED)))))) {

        switch (syntaxid) {
            case LDAP_SYN_MULTILINESTR:
                sprintf(value_buf+strlen(value_buf), "\
<TEXTAREA NAME=\"%s\" COLS=30 ROWS=4></TEXTAREA><BR>\n",
                        ti->ti_attrname);
                break;
            case LDAP_SYN_LABELEDURL:
                sprintf(value_buf+strlen(value_buf), "<BR>\
%s: <INPUT NAME=\"%s\" SIZE=\"%d\"><BR> \
%s: <INPUT NAME=\"ext\" SIZE=\"%d\"><BR><BR>\n",
                    MSG_URL, ti->ti_attrname, 35, 
                    MSG_URL_INFO, 35);
                break;
            case LDAP_SYN_BOOLEAN:
                sprintf(value_buf+strlen(value_buf),
    "<INPUT TYPE=\"radio\" NAME=\"%s\" VALUE=\"T\"> %s \
    <INPUT TYPE=\"radio\" NAME=\"%s\" VALUE=\"F\"> %s\n",
                    ti->ti_attrname, MSG_TRUE, ti->ti_attrname, MSG_FALSE);
                break;
            default:
                sprintf(value_buf+strlen(value_buf), 
                    "%s<INPUT NAME=\"%s\" SIZE=%d >\n", 
                    writeoutval ? "<BR>" : "", ti->ti_attrname, 
                    maxlen > 0 ? maxlen : 30);
        }
    }

    if (freevals)
        free(outval);

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, " web500gw_vals2html ><\n", 0, 0, 0, 0);
#endif
    return LDAP_SUCCESS;
}

static int
searchaction(
    LDAP        *ld, 
    REQUEST     *r,
    RESPONSE    *resp,
    LDAPMessage *entry, 
    struct ldap_tmplitem *tip
)
{
    char    **vals, *value, *filtpattern, *attr, *attrs, *flags, *f, *selectname;
    char    filter[BUFSIZ], buf[BUFSIZ];
    REQUEST  new_r, *n_r;
    RESPONSE new_resp, *n_resp;
    int     i = 0, rc;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "searchaction (%s, %s)\n",
        r->r_dn, tip->ti_label, 0, 0);
#endif

    for (i = 0; tip->ti_args != NULL && tip->ti_args[i] != NULL; ++i);
    if (i < 3) {
        return(LDAP_PARAM_ERROR);
    }

    /* save current state, so that we can resume after do_search call */
    new_r    = *r;
    new_resp = *resp;
    n_r    = &new_r;
    n_resp = &new_resp;

    attr = tip->ti_args[0];
    filtpattern = tip->ti_args[1];

    flags = NULL;
    attrs = tip->ti_args[2];

    if (attrs != NULL && *attrs != '\0') {
        if ((flags = strchr(attrs, '$')) != NULL)   /* flags */
            *flags++ = '\0';
        i = 0;
        n_r->r_attrs[i++] = attrs;
        while ((attrs = strchr(attrs, ',')) != NULL && i < MAX_ATTRS) {
            /* comma separated list of attributes */
            *attrs++ = '\0';
            n_r->r_attrs[i++] = attrs;
        }
        n_r->r_attrs[i] = NULL;
        n_r->r_attrnumb = i;
    } else {
        n_r->r_attrs[0] = NULL;
        n_r->r_attrnumb = 0;
    }
    
    if (flags && *flags != '\0') {      /* flags */
        n_r->r_flags &= ~FLAG_LAYOUT;   /* remove all display flags */
        while ((f = strchr(flags, ',')) != NULL) {
            *f++ = '\0';
            n_r->r_flags |= string2flag(flags);
            flags = f;
        }
        n_r->r_flags |= string2flag(flags);
    } else {
        n_r->r_flags |= FLAG_VALSONLY;
    }
    /* always set these flags */   
    n_r->r_flags |= FLAG_ENTRYONLY|FLAG_ATTRSONLY|FLAG_SEARCHACT;

    selectname = tip->ti_args[3];

    vals = NULL;
    if (attr == NULL) {
        value = NULL;
    } else if (strcasecmp(attr, "-dn") == 0) {
        value = r->r_dn;
    } else if ((vals = ldap_get_values(ld, entry, attr)) != NULL) {
        value = vals[0];
    } else {
        value = NULL;
    }
    ldap_build_filter(buf, sizeof(buf), filtpattern, NULL, NULL, NULL,
            value, NULL);
    filter[0] = '\0';
    if ((buf[0] != 'S' && buf[0] != 'O') || buf[1] != '=') {
        /* no search level given -> search subtree */
        strcpy(filter, "S=");
    }
    strcat(filter, buf);
    n_r->r_filter = strdup(filter);

    rc = do_search(n_r, n_resp);
    return (rc);
}
