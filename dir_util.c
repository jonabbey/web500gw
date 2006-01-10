/*
 * dir_util.c:      some utility routines for directory
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
 * $Id: dir_util.c,v 1.2 2001/04/26 22:16:53 dekarl Exp $
 */

#include "web500gw.h"
#include <errno.h>

/* web500gw_ldap_init: ldap_open + ldap_bind */
LDAP *
web500gw_ldap_init(
    REQUEST     *r,
    RESPONSE    *resp,
    char        *bind_as,
    char        *pw,
    int         printerror
)
{
    LDAP    *ld;
    int     rc, basic_auth = 0;
    int option_param;
    char    *who, *password;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, " web500gw_ldap_init (%s:%d, %s, %s)\n",
        r->r_ldaphost, r->r_ldapport, 
        bind_as ? bind_as : (r->r_authorization ? "basic auth" : 
            (r->r_access->a_bind_dn ? r->r_access->a_bind_dn : "anon bind")),
        printerror ? "printerror" : "no printerror");
#endif

    if ((ld = ldap_open(r->r_ldaphost, r->r_ldapport)) == NULL) {
#ifdef WEB500GW_DEBUG
        if (web500gw_debug) perror("ldap_open");
#endif
        do_error(r, resp, errno, SERVER_ERROR, strerror(errno), NULL);
        /* do_error(r, resp, LDAP_SERVER_DOWN, SERVER_ERROR, NULL,
         * NULL); */
        return (LDAP *)0;
    }

    if (r->r_flags & FLAG_DEREFALIAS)
      {
	option_param = LDAP_DEREF_ALWAYS;
      }
    else
      {
	option_param = LDAP_DEREF_FINDING;
      }

    ldap_set_option(ld, LDAP_OPT_DEREF, (void *) &option_param);

    if (bind_as) {
        who = bind_as;
        password = pw;
    } else {
        if (strcasecmp(r->r_ldaphost, ldaphost) != 0) {
            /* non default ldap server - anon bind */
            who = NULL;
            password = NULL;
        } else if (r->r_authorization && *r->r_authorization &&
            (strncasecmp(r->r_authorization, "Basic ", 6) == 0)) {
            /* basic authentification */
            basic_auth = 1;
            who = b64_decode(r->r_authorization + 6);
            if (password = strchr(who, ':')) {
                *password++ = '\0';
            }
        } else {
            who = r->r_access->a_bind_dn;
            password = r->r_access->a_bind_pw;
        }
    }

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, " web500gw_ldap_init: ldap_simple_bind_s (%s)\n", 
        who ? who : "anon", 0, 0, 0);
#endif

    if ((rc = ldap_simple_bind_s(ld, who, password)) != LDAP_SUCCESS) {
#ifdef WEB500GW_DEBUG
        if (web500gw_debug) ldap_perror(ld, "ldap_simple_bind_s");
#endif
        if (basic_auth) {
            do_auth_bind(r, resp);
        } else if (printerror) {
	  do_error(r, resp, rc, 0, get_ldap_error_str(ld), NULL);
        }
        return (LDAP *)0;
    }
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, " web500gw_ldap_init: ldap_simple_bind_s >OK<\n", 
        0, 0, 0, 0);
#endif

    r->r_ld = ld;
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, " web500gw_ldap_init >OK<\n", 0,0,0,0);
#endif
    return(ld);
}


/* Picks an objectclass from a list of oc's */
char * 
pick_oc(char **oclist)
{
    int    i;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, " pick_oc (", 0, 0, 0, 0);
    if (oclist) {
        for (i = 0; oclist[i] != NULL; i++) {
            Web500gw_debug(WEB500GW_DEBUG_UTIL, "%s, ", oclist[i], 0, 0, 0);
        }
    }
    Web500gw_debug(WEB500GW_DEBUG_UTIL, ")\n", 0, 0, 0, 0);
#endif
    if (oclist == NULL) {
#ifdef WEB500GW_DEBUG
        Web500gw_debug(WEB500GW_DEBUG_UTIL, " pick_oc >unknown<\n", 0, 0, 0, 0);
#endif
        return("unknown");
    }
    for (i = 0; oclist[i] != NULL; i++) {
        if (strcasecmp(oclist[i], "top") != 0 &&
            strcasecmp(oclist[i], "quipuObject") != 0 &&
            strcasecmp(oclist[i], "quipuNonLeafObject") != 0) {
#ifdef WEB500GW_DEBUG
            Web500gw_debug(WEB500GW_DEBUG_UTIL, " pick_oc >%s<\n", oclist[i], 0, 0, 0);
#endif
            return(oclist[i]);
        }
    }
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, " pick_oc >unknown<\n", 0, 0, 0, 0);
#endif
    return("unknown");
}

/* tries to determine if an object is a leaf or a nonleaf entry */
int
isnonleaf(
    LDAP    *ld,
    char    **oclist,
    char    *dn
)
{
    int    i, quipuobject = 0;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, " isnonleaf (%s, ", dn, 0, 0, 0);
    if (oclist) {
        for (i = 0; oclist[i] != NULL; i++) {
            Web500gw_debug(WEB500GW_DEBUG_UTIL, "%s, ", oclist[i], 0, 0, 0);
        }
    }
    Web500gw_debug(WEB500GW_DEBUG_UTIL, ")\n", 0, 0, 0, 0);
#endif

    if (oclist == NULL)
        return (0);

    for (i = 0; oclist[i] != NULL; i++) {
        if (strcasecmp(oclist[i], "quipuObject") == 0)
            quipuobject = 1;
        if (strcasecmp(oclist[i], "quipuNonLeafObject") == 0 ||
            strcasecmp(oclist[i], "externalNonLeafObject") == 0)
            return (1);
    }

    /*
     * not a quipu thang - no easy way to tell leaves from nonleaves
     * except by trying to search or list, which would be expensive.
     * for now, just guess assume all non-quipu objects are non-leaf
     */
    return (quipuobject ? 0 : 1);
#ifdef notdef
    if (!quipuobject) {
        int     rc, numentries;
        struct timeval  timeout;
        LDAPMessage *res = NULL;
        static char *attrs[] = { "objectClass", 0 };

        timeout.tv_sec = timelimit;
        timeout.tv_usec = 0;
        ld->ld_sizelimit = 1;
        if ((rc = ldap_search_st(ld, dn, LDAP_SCOPE_ONELEVEL,
            default_filter, attrs, 0, &timeout, &res))
                == LDAP_SUCCESS || rc == LDAP_SIZELIMIT_EXCEEDED) {
            ld->ld_sizelimit = LDAP_NO_LIMIT;

            numentries = ldap_count_entries(ld, res);
            if (res != NULL)
                ldap_msgfree(res);
            return(numentries == 1 ? 1 : 0);
        }
    }
    return (0);
#endif
}


/* looks if an objectclass is in a list of oc's */
int
isoc(
    char    **ocl,
    char    *oc
)
{
    int    i;
    if (ocl == NULL || oc == NULL)
        return(0);
    for (i = 0; ocl[i] != NULL; i++) {
        if (strcasecmp(ocl[i], oc) == 0)
            return (1);
    }
    return (0);
}


/* tries to determine if a "seach below" should be onelevel or subtree */
int 
make_scope(
    LDAP    *ld,
    char    *dn
)
{
    int             scope, i = 0;
    char            **oc;
    LDAPMessage     *res;
    struct timeval  timeout;
    static char     *attrs[] = { "objectclass", 0 };

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, " make_scope (%s)\n", dn, 0, 0, 0);
#endif

    if (dn == NULL || *dn == '\0')
        return (LDAP_SCOPE_ONELEVEL);

    timeout.tv_sec = timelimit;
    timeout.tv_usec = 0;
    if (ldap_search_st(ld, dn, LDAP_SCOPE_BASE, default_filter,
        attrs, 0, &timeout, &res) != LDAP_SUCCESS) {
        return NOTOK;
    }
    oc = ldap_get_values(ld, ldap_first_entry(ld, res), objectclass_attr);

    scope = LDAP_SCOPE_ONELEVEL;
    while (search_subtree_oc[i]) {
        if (isoc(oc, search_subtree_oc[i])) {
            scope = LDAP_SCOPE_SUBTREE;
            break;
        }
        i++;
    }
    ldap_value_free(oc);
    ldap_msgfree(res);

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, " make_scope >%s<\n", 
        scope == LDAP_SCOPE_SUBTREE ? "S" : "O", 0, 0, 0);
#endif
    return (scope);
}


/*
 * friendly_dn - returns a "user friendly DN" in ISO 8859-1
 * input:   dn - pointer to DN
 * returns: string to UFN (allocated mem)
 */

char *
friendly_dn (
    RESPONSE    *resp,
    char        *dn
)
{
    char    *fufn, *ufn, *cp, *f_label;
    int     len;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, " friendly_dn (%s)\n", dn ? dn : "", 
        0, 0, 0);
#endif

    if (!dn)
        return NULL;

    if (strlen(dn) == 0)
        return MSG_ROOT;

    ufn = (char *)web500gw_t61toiso(clean_ufn(ldap_dn2ufn(dn)));
    if ((cp = strrchr(ufn,','))) {
                /* now make friendly name for the toplevel */
        *cp++ = '\0';
        len = strlen(ufn) + 2;      /* + strlen(", ") */
        while (isspace(*cp)) cp++;
        f_label = friendly_label(resp, cp);
        len += strlen(f_label);
        fufn = (char *) calloc(1, len + 1);
        strcpy(fufn, ufn);
        strcat(fufn, ", ");
        strcat(fufn, f_label);
    } else {
        fufn = strdup(friendly_label(resp, ufn));
    }
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, " friendly_dn >%s<\n", fufn, 0, 0, 0);
#endif
    return (fufn);
}

/*
 * friendly_rdn - returns a "user friendly RDN" (== first component of DN)
 * input:   dn - pointer to DN, notypes - 0 = with attr, 1 = only values
 * returns: string to RDN (allocated mem)
 */

char *
friendly_rdn (
    RESPONSE    *resp,
    char        *dn,
    int         notypes
)
{
    char    **s, *rdn;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, " friendly_rdn (%s)\n", dn, 0, 0, 0);
#endif

    if (!dn || strlen(dn) == 0)
        return MSG_ROOT;
    
    s = ldap_explode_dn(dn, notypes);
    if (notypes) {
        if (s[1] == NULL) {   /* toplevel */
            rdn = strdup(friendly_label(resp, s[0]));
        } else {
            rdn = clean_ufn(s[0]);
        }
    } else {
        rdn = strdup(s[0]);
    }
    ldap_value_free(s);

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, " friendly_rdn >%s<\n", rdn, 0, 0, 0);
#endif
    return (rdn);
}


/*
 * clean_ufn - some cosmetics in UFN (removes quotes/unknown attributes)
 * input:   ufn - pointer to UFN
 * returns: string to cleaned UFN (allocated mem)
 */

char *
clean_ufn (char *ufn)
{
    char *cp, *new, *next;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, " clean_ufn (%s)\n", ufn, 0, 0, 0);
#endif

    /* We make sure to eliminate any unknown attributes in the UFN! */
    if (ufn == NULL)
        return NULL;
    cp = ufn;
    next = new = (char *) calloc(1, strlen(ufn) + 1);
    while (*ufn != '\0') {
        if (*ufn == '"') {        /* remove quotes */
            ufn++;
            continue;
        }
        if (*ufn == '=') {      /* remove unknown atttribute */
            ufn++;
            while (!isspace(*next) && *next != ',' && next > new) next--;
            next++;     /* dont remove the space or ',' */
            continue;
        }
        *next++ = *ufn++;
    }
    *next = '\0';
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, " clean_ufn >%s<\n", new, 0, 0, 0);
#endif
    return(new);
}

char **
deref (
    LDAP  *ld,
    char *dn
)
{
    LDAPMessage *e, *res;
    struct timeval  timeout;
    static char   *retattrs[] = { "objectClass" , 0 };
    char        **oc = NULL;
    int         i;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "deref (%s)\n", dn, 0, 0, 0);
#endif

    timeout.tv_sec = timelimit;
    timeout.tv_usec = 0;
    if (ldap_search_st(ld, dn, LDAP_SCOPE_BASE, default_filter, retattrs, 
            0, &timeout, &res) == LDAP_SUCCESS) {
        e = ldap_first_entry(ld, res);
        oc = ldap_get_values(ld, e, "objectClass");
#ifdef WEB500GW_DEBUG
        Web500gw_debug(WEB500GW_DEBUG_UTIL, "deref search ok:\n", 0, 0, 0, 0);
        for (i = 0; (oc)[i] != NULL; i++) {
            Web500gw_debug(WEB500GW_DEBUG_UTIL, "%s, ", (oc)[i], 0, 0, 0);
        }
        Web500gw_debug(WEB500GW_DEBUG_UTIL, "\n", 0, 0, 0, 0);
#endif
        ldap_msgfree(res);
    }
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "deref ><\n", 0, 0, 0, 0);
#endif
    return oc;
}


/* determines if a given DN is below defined HomeDN */
int
isinhome (char * dn)
{
    char    **h, **d;
    int     hcnt, dcnt, ret = 0;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  isinhome (%s)\n", dn ? dn : "", 0, 0, 0);
#endif

    if (*home_dn && dn && *dn) {
        h = ldap_explode_dn(home_dn, 0);
        d = ldap_explode_dn(dn, 0);
        for (hcnt = 0; h[hcnt]; hcnt++);
        for (dcnt = 0; d[dcnt]; dcnt++);
        for (--hcnt, --dcnt; hcnt >= 0 && dcnt >= 0; hcnt--, dcnt--) {
            if (strcasecmp(h[hcnt], d[dcnt]) != 0)
                break;
        }
        if (hcnt < 0)
            /* all "higher" name components of dn are equal to home_dn
             * -> dn is below HOME DN */
            ret = 1;    
        ldap_value_free(h);
        ldap_value_free(d);
    }

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  isinhome >%d<\n", ret, 0, 0, 0);
#endif
    return(ret);
}

/* strips a base DN from a DN */
char *
strip_dn (char * dn, char *base_dn)
{
    char    **b, **d, *ret;
    int     bcnt, dcnt, i;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  strip_dn (%s, %s)\n", dn ? dn : "",
        base_dn ? base_dn : "", 0, 0);
#endif

    if (*base_dn && dn && *dn) {
        b = ldap_explode_dn(base_dn, 0);
        d = ldap_explode_dn(dn, 0);
        for (bcnt = 0; b[bcnt]; bcnt++);
        for (dcnt = 0; d[dcnt]; dcnt++);
        --bcnt; --dcnt;
        for (i = dcnt; bcnt >= 0 && i >= 0; bcnt--, i--) {
            if (strcasecmp(b[bcnt], d[i]) != 0)
                break;
        }
        if (i == dcnt) {
            ret = strdup(dn);   /* nothing to strip */
        } else {
            dcnt = i;
            ret = calloc(strlen(dn), sizeof(char));
            for (i = 0; i < dcnt; i++) {
                strcat(ret, d[i]);
                strcat(ret, ", ");
            }
            strcat(ret, d[i]);
        }
        ldap_value_free(b);
        ldap_value_free(d);
    } else {
        ret = strdup(dn);
    }
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  strip_dn >%s<\n", ret, 0, 0, 0);
#endif
    return(ret);
}

/*------------------------------------------------------------------------------

                                                                  strip_ufn_dn()

 Strips a base 'friendly' DN from a 'friendly' DN.  This differs from
 strip_dn, above, in that it operates strictly as a textual operation,
 removing the base_ufn_dn string from the end of ufn_dn and returning
 it, if indeed base_ufn_dn is a suffix for ufn_dn.  The above
 function, strip_dn(), depends on the ldap_explode_dn() function
 working properly with ufn DN's, which is no longer the case in recent
 versions of OpenLDAP.

 The returned string is allocated in this function, and would
 ordinarily have to be freed.  web500gw actually forks a separate,
 short-lived process for each search, though, so that's not really
 important in practice.

 This function may return NULL if memory cannot be allocated for a
 copy of the result, or if ufn_dn is NULL.

------------------------------------------------------------------------------*/
char *
strip_ufn_dn (char * ufn_dn, char *base_ufn_dn)
{
  char *p1, *p2;
  char *result;
  int i = 0;
  int ufn_dn_length, base_ufn_dn_length;

  /* -- */

  if (ufn_dn == NULL)
    {
      return NULL;
    }

  if (base_ufn_dn == NULL)
    {
      return strdup(ufn_dn);
    }

  ufn_dn_length = strlen(ufn_dn);
  base_ufn_dn_length = strlen(base_ufn_dn);

  p1 = strstr(ufn_dn, base_ufn_dn);

  if (p1 == NULL || p1 == ufn_dn ||
      ufn_dn_length == 0 || base_ufn_dn_length == 0)
    {
      return strdup(ufn_dn);
    }

  // advance p1 and p2 to the last character of ufn_dn and
  // base_ufn_dn, respectively.

  p1 = ufn_dn + ufn_dn_length - 1;
  p2 = base_ufn_dn + base_ufn_dn_length - 1;

  // we know base_ufn_dn can't be longer than ufn_dn, or else we would
  // have returned early, above

  i = 0;

  while (p1 > ufn_dn && *p1 == *p2)
    {
      p1--;
      p2--;
      i++;
    }

  /* i now tells us how many characters from the end of ufn_dn we need
     to cut off in order to remove the matching suffix.  we also need
     to trim any commas and whitespace from the end of our truncated
     ufn_dn. */

  while (p1 > ufn_dn && isspace(*p1) || *p1 == ',')
    {
      p1--;
      i++;
    }

  result = malloc(ufn_dn_length - i + 1);

  if (result)
    {
      memcpy(result, ufn_dn, ufn_dn_length - i);
      result[ufn_dn_length - i] = '\0';

      return result;
    }

  return NULL;
}

#ifdef UP_SMALL
/* Make UFN components to hypertext links */

char *
href_dn (
    RESPONSE    *resp,
    char        *dn,
    int         action
)
{
    char **s, **t, 
    fhdn[2048],    /* full hypertexted DN, return value */
    thdn[1024],    /* temp */
    hrdn[1024];    /* temp, hypertexted RDN */
    int  cnt, i, j;
    
    fhdn[0] = '\0';
    thdn[0] = '\0';
    hrdn[0] = '\0';
    s = ldap_explode_dn(dn, 1);
    t = ldap_explode_dn(dn, 0);
    for (cnt=0; t[cnt]; cnt++);
    for (i=0; s[i]; i++) {
        strcpy(hrdn, t[i]);
        for (j=i+1; j < cnt ; j++) {
            strcat(hrdn, ", ");
            strcat(hrdn,t[j]);
        }
        
        sprintf(thdn, "<A HREF=\"/%s%s\">%s</A>%s ",
                action || i > 0 ? "M" : "R", url_encode(hrdn),
                friendly_label(resp, s[i]),
                i == cnt - 1 ? "" : ",");
        strcat(fhdn, thdn);
        hrdn[0] = '\0';
        thdn[0] = '\0';
    }
    ldap_value_free(s);
    ldap_value_free(t);
    return fhdn;
}
#endif /* UP_SMALL */
