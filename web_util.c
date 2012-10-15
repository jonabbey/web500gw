/*
 * web_util.c:      some utility routines for HTTP/HTML
 *
 * written by: Frank Richter, Frank.Richter@hrz.tu-chemnitz.de
 *             Chemnitz University of Technology, Germany, 1994-8
 *
 * Copyright (c) 1994-8 Chemnitz University of Technology.
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
 * $Id: web_util.c,v 1.2 2001/04/26 22:16:53 dekarl Exp $
 */

#include "web500gw.h"

/* 
 * http_header: prints the HTTP/1.0 header
 */

int
http_header (
    REQUEST     *r,
    RESPONSE    *resp
)
{
    char    *status_msg, expiredate[256];
    time_t  expiretime;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, " http_header (%d, %s, %d, %s)\n",
           resp->resp_status, resp->resp_content_type, 
           resp->resp_content_length, 
           resp->resp_last_modified ?  resp->resp_last_modified : "");
#endif

    if (r->r_httpversion != 1 || resp->resp_httpheader != 0) {
        return (NOTOK);
    }

    switch (resp->resp_status) {
        case DOCUMENT_FOLLOWS:  status_msg = M_DOCUMENT_FOLLOWS;  break;
        case REDIRECT:          status_msg = M_REDIRECT;      break;
        case NOT_MODIFIED:      status_msg = M_NOT_MODIFIED;  break;
        case BAD_REQUEST:       status_msg = M_BAD_REQUEST;   break;
        case AUTH_REQUIRED:     status_msg = M_AUTH_REQUIRED; break;
        case FORBIDDEN:         status_msg = M_FORBIDDEN;     break;
        case NOT_FOUND:         status_msg = M_NOT_FOUND;     break;
        case SERVER_ERROR:      status_msg = M_SERVER_ERROR;  break;
        case NOT_IMPLEMENTED:   status_msg = M_NOT_IMPLEMENTED;   break;
        default:                status_msg = M_UNKNOWN_ERROR;
    }
    fprintf(fp, "\
HTTP/1.0 %d %s\r\n\
Date: %s\r\n\
Server: %s\r\n\
MIME-Version: 1.0\r\n\
Content-Type: %s\r\n\
Content-Language: %s\r\n", 
    resp->resp_status, status_msg, resp->resp_date, version, 
    resp->resp_content_type, 
    resp->resp_language->l_content_lang);

    if (resp->resp_content_length > 0) {
        fprintf(fp, "Content-Length: %d\r\n", resp->resp_content_length);
    }

    if (r->r_flags & FLAG_NOCACHE || resp->resp_expires == 0) {
        /* expire now! */
        fprintf(fp, "Expires: %s\r\n", resp->resp_date);
    } else if (resp->resp_expires > 0) {
        /* expires in the future */
        expiretime = r->r_tm + resp->resp_expires;
        strftime(expiredate, 256, DATE_FORMAT, 
            gmtime(&expiretime));
        fprintf(fp, "Expires: %s\r\n", expiredate);
    }

    if (lastmodified && (!(r->r_flags & FLAG_NOCACHE)) && 
        resp->resp_last_modified && *resp->resp_last_modified) {
        fprintf(fp, "Last-Modified: %s\r\n", resp->resp_last_modified);
    }
    if (resp->resp_location && *resp->resp_location) {
        fprintf(fp, "Location: %s\r\n", resp->resp_location);
    }
    if (resp->resp_extra && resp->resp_extra) {
        fputs(resp->resp_extra, fp);
    }
    fputs("\r\n", fp);
    fflush(fp);
    resp->resp_httpheader = 1;
    return(OK);
}

/* 
 * dn2url:  prepare DN's as URL - add flags + language (if user req.), encode 
 */

char *
dn2url(
    REQUEST     *r,
    char        *dn, 
    unsigned int flags, 
    unsigned int actions, 
    char        *addstring, 
    char        *server
)
{
    char    *new, *out, *cp, *np, *fp, *flagstring = NULL, *s;
    int     flaglen = 0, addlen = 0, serverlen = 0, flagseen = 0;

    if (! dn)
        return(NULL);
    if ((! *dn) && rootishome && (!cgimode))
        dn = "root";        /* pseudo root DN */

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, " dn2url (%s, 0x%x, 0x%x, %s,", 
        dn, flags, actions, addstring ? addstring : "");
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "%s)\n", server ? server : "", 0, 0, 0);
#endif

    if (server != NULL) {
        serverlen = strlen(server);
    }
    if (flags || actions) {
        flagstring = calloc(BUFSIZ, sizeof(char));
        if (flags) {
            s = flag2string(flags);
            strncat(flagstring, s, BUFSIZ - strlen(flagstring) - 1); 
        }
        if (flags & FLAG_LANGUAGE && r->r_language) {
            if (*flagstring)
                strcat(flagstring, ",");
            strncat(flagstring, "lang=", BUFSIZ - strlen(flagstring) - 1);
            strncat(flagstring, r->r_language, BUFSIZ - strlen(flagstring) - 1);
        }
        if (flags & FLAG_TMPL && r->r_template) {
            if (*flagstring)
	      strncat(flagstring, ",", BUFSIZ - strlen(flagstring) - 1);
            strncat(flagstring, "tmpl=", BUFSIZ - strlen(flagstring) - 1);
            strncat(flagstring, r->r_template, BUFSIZ - strlen(flagstring) - 1);
        }
        if (actions) {
            s = actions2string(actions);
            if (*flagstring)
	      strncat(flagstring, ",", BUFSIZ - strlen(flagstring) - 1);
            strncat(flagstring, s, BUFSIZ - strlen(flagstring) - 1);
        }
        flaglen = strlen(flagstring);
    }
    if (addstring)
        addlen = strlen(addstring);

    if (flaglen + addlen > 0) {        /* flags ... to add */
        new = malloc(strlen(dn) + flaglen + addlen + 4);
        strcpy(new, dn);
        if (addlen == 0) {            /* flags only */
            strcat(new, "?$");       /* add separator */
            strcat(new, flagstring);
            free(flagstring);
        } else {     /* given strings (attributes, flags and/or search) */
            strcat(new, "?");
            np = new + strlen(new);
            cp = addstring;
            if (*addstring == '?') {
                cp++;
            }
            /* add addstring to dn till ? (start of search) */
            while (*cp) {
                if (*cp == '$') {           /* start of flags */
                    flagseen = 1;
                } else if (*cp == '?') {    /* start of search specification */
                    break;
                }
                *np++ = *cp++;
            }
            /* add additional flags */
            if (flaglen > 0) {
                if (!flagseen) {
                    *np++ = '$';
                    flagseen = 1;
                } else {
                    *np++ = ',';
                }
                fp = flagstring;
                while (*fp) *np++ = *fp++;
                free(flagstring);
            }
            /* now copy the rest - i.e. search (if some ...) */
            while (*cp) *np++ = *cp++;
            *np = '\0';
        }
        out = url_encode(new);
        /* free(new); */
    } else {                    /* just encode DN */
        out = url_encode(dn);
    }
    if (serverlen > 0) {        /* other server requested */
        new = malloc(serverlen + strlen(out) + 1);
        if (strncmp(server, "ldap://", 7) == 0) {
            if (allow_other_servers)
                sprintf(new, "/%s/%s", server + 7, out);
        } else {
            strcpy(new, server);
            strcat(new, out);
        }
        free(out);
        out = new;
    } else if (allow_other_servers && 
               (strcasecmp(r->r_ldaphost, ldaphost) != 0 ||
                r->r_ldapport != ldapport)) {
               /* non default ldap server */
        new = malloc(strlen(out) + strlen(r->r_ldaphost) + 16);
        sprintf(new, "/%s:%d/%s", r->r_ldaphost, r->r_ldapport, out);
        free(out);
        out = new;
    }
    if (serverlen <= 0 && cgimode && (cp = getenv("SCRIPT_NAME")) != NULL) {
        /* in CGI mode and no other http server: copy script URL at first */
        new = malloc(strlen(out) + strlen(cp) + 2);
        sprintf(new, "%s%s%s", cp, *out == '/' ? "" : "/", out);
        free(out);
        out = new;
    }

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, " dn2url >%s<\n", out, 0, 0, 0);
#endif
    return out;
}


/*
 *  Utilities for dealing with HTML/HTTP
 */

#define from_hex(c)     ((c>='0')&&(c<='9') ? c-'0' : (c>='A')&&(c<='F') ?\
                        c-'A'+10 : (c>='a')&&(c<='f') ? c-'a'+10 : 0)

static int acceptable[256];
static int acceptable_inited = 0;

void init_acceptable()
{
    unsigned int i;
    char * good =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_$";
    for(i=0; i<256; i++) acceptable[i] = 0;
    for(;*good; good++) acceptable[(unsigned int)*good] = 1;
    acceptable_inited = 1;
}

static char hex[17] = "0123456789abcdef";

/* url_encode:  substitute unsafe chars to %## (hex) */

char *
url_encode (char *in)
{
    char *out, *buffer;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  url_encode (%s)\n", in, 0, 0, 0);
#endif

    out = buffer = malloc(3 * strlen(in) + 1);   /* worst case: all encoded */
    if (acceptable_inited == 0)
        init_acceptable();
    while (*in) {
        if (acceptable[(unsigned char)*in] == 1) {
           *out++ = *in++;
        } else {
           *out++ = '%';
           *out++ = hex[(unsigned char)*in >> 4];
           *out++ = hex[(unsigned char)*in & 15];
           in++;
        }
    }
    *out = '\0';
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  url_encode >%s<\n", buffer, 0, 0, 0);
#endif

    return (buffer);
}


/* url_decode:  decode %## (hex) encoding */

void
url_decode (char *in)
{
    char b, c;
    int q = 0;
    char *out, *cp; 

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  url_decode (%s)\n", in, 0, 0, 0);
#endif

    out = cp = in;
    while (*in) {
        if (*in == '?')     /* start search */
            q = 1;
        if (*in == '%') {   /* Hex escape */
            in++;
            c = *in++;
            b = from_hex(c);
            c = *in++;
            if (!c) break;
                *out++ = (b<<4) + from_hex(c);
        } else if (q && *in == '+') {
        /* '+' is legal in path, in search it's a ' ' */
            *out++ = ' ';
            in++;
        } else {
            *out++ = *in++;
        }
    }
    *out = '\0';
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  url_decode >%s<\n", cp, 0, 0, 0);
#endif
}

/* decode in search (for do_modify) */

char *
hex_qdecode (char *in)
{
    char b, c;
    char *ret = in, *out = in;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  hex_qdecode (%s)\n", 
        in ? in : "", 0, 0, 0);
#endif
    if (!in)
        return NULL;

    while (*in) {
        if (*in == '%') {     /* Hex escape */
            in++;
            c = *in++;
            b = from_hex(c);
            c = *in++;
            if (!c) break;
            *out++ = (b<<4) + from_hex(c);
        } else if (*in == '+') { /* we are in search, so: '+' becomes SPACE */
            *out++ = ' ';
            in++;
        } else {
            *out++ = *in++;
        }
    }
    *out = '\0';
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  hex_qdecode >%s<\n", ret, 0, 0, 0);
#endif

    return (ret);
}

/* html_encode:  substitute ", &, <, > to &###; (decimal) 
 *               allocates mem */

char *
html_encode (unsigned char *in, int len)
{
    char *out, *buffer;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  html_encode (%s)\n", in, 0, 0, 0);
#endif

    int i;
    int length = 6 * len + 1;   /* worst case: all encoded */

    out = buffer = malloc(length);

    for (i = 0; i < len; i++)
      {
        if (*in > 0x80 || *in == '"' || *in == '\'' || *in == '&' ||
            *in == '<' || *in == '>') {
            *out++ = '&';
            *out++ = '#';
            *out++ = (*in / 100) + '0';
            *out++ = ((*in % 100) / 10) + '0';
            *out++ = ((*in % 100) % 10) + '0';
            *out++ = ';';
            in++;
	    i++;
        } else {
           *out++ = *in++;
        }
      }

    *out = '\0';
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  html_encode >%s<\n", buffer, 0, 0, 0);
#endif

    return (buffer);
}

/* string_encode:  substitute ' to \' for use in String Literals
 *                 (window.status='string'...)
 *                 allocates mem */

char *
string_encode (unsigned char *in)
{
    char *out, *buffer;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  string_encode (%s)\n", in, 0, 0, 0);
#endif

    if (! (strchr((char *)in, '\'') || strchr((char *)in, '"')))     
        /* No ' nor " in string */
        return((char *)in);

    out = buffer = malloc(6 * strlen((char *)in) + 1);
        /* worst case: all encoded */

    while (*in) {
        if (*in  == '"') {
            *out++ = '&'; *out++ = 'q'; *out++ = 'u'; *out++ = 'o';
            *out++ = 't'; *out++ = ';';
            in++;
        } else {
            if (*in  == '\'')
                *out++ = '\\';
            *out++ = *in++;
        }
    }
    *out = '\0';
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  string_encode >%s<\n", buffer, 0, 0, 0);
#endif

    return (buffer);
}

static const unsigned char pr2six[256] =
{
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63, 52, 53, 54,
    55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64, 64, 0, 1, 2, 3,
    4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
    22, 23, 24, 25, 64, 64, 64, 64, 64, 64, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
    50, 51, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

char *
b64_decode(char *in)
{
    int nbytesdecoded;
    unsigned char *cpin, *cpout;
    char *buffer;
    int nprbytes;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  b64_decode (%s)\n", in, 0, 0, 0);
#endif

    /* Strip leading whitespace. */
    while (isspace(*in)) in++;

    /* Figure out how many characters are in the input buffer.
     * Allocate this many from the per-transaction pool for the result.
     */
    cpin = (unsigned char *) in;
    while (pr2six[*(cpin++)] <= 63);
    nprbytes = (cpin - (unsigned char *) in) - 1;
    nbytesdecoded = ((nprbytes + 3) / 4) * 3;

    buffer = calloc(nbytesdecoded + 1, sizeof(char));
    cpout = (unsigned char *) buffer;

    cpin = (unsigned char *) in;

    while (nprbytes > 0) {
        *(cpout++) =
            (unsigned char) (pr2six[*cpin] << 2 | pr2six[cpin[1]] >> 4);
        *(cpout++) =
            (unsigned char) (pr2six[cpin[1]] << 4 | pr2six[cpin[2]] >> 2);
        *(cpout++) =
            (unsigned char) (pr2six[cpin[2]] << 6 | pr2six[cpin[3]]);
        cpin += 4;
        nprbytes -= 4;
    }

    if (nprbytes & 03) {
        if (pr2six[cpin[-2]] > 63) {
            nbytesdecoded -= 2;
	    } else {
            nbytesdecoded -= 1;
        }
    }
    buffer[nbytesdecoded] = '\0';

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  b64_decode >%s<\n",
            buffer, 0, 0, 0);
#endif
    return buffer;
}
