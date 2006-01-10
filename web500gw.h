/*
 * web500gw.h, Version 2.1: 
 * written by: Frank Richter, Frank.Richter@hrz.tu-chemnitz.de
 *             Chemnitz University of Technology, Germany, 1994-8
 *
 * Copyright (c) 1994-7 Chemnitz University of Technology.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the Chemnitz University of Technology. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 *
 * ----------------------------------------------------------------------
 * This source is based on go500gw by Tim Howes, University of Michigan:
 * Copyright (c) 1990 Regents of the University of Michigan.
 * All rights reserved.
 * ----------------------------------------------------------------------
 *
 * $Id: web500gw.h,v 1.3 2001/04/29 21:19:17 dekarl Exp $
 */

#include "ldap_compat.h"
#include "lber.h"
#include "ldap.h"
#include "disptmpl.h"
/*#include "ldap_compat.h" */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/param.h>
/* XARL -- fix the time inclusion for linux more gracefully */
/*#ifdef aix */
#include <time.h>
/*#else
  #include <sys/time.h>
  #endif*/
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
/* #ifdef __hpux */
#include <syslog.h>
/* #else
#include <sys/syslog.h>
#endif */
#include <sys/wait.h>
#include <signal.h>
#ifdef aix
#include <sys/select.h>
#include <sys/access.h>
#endif /* aix */

#include "portable.h"
#include <regex.h>

#include "config.h"

/*
#if (defined(LDAPVERSION) && LDAPVERSION == IC)
#include "isode/ldap/config.h"
#else
#include "ldapconfig.h"
#endif
*/

#if defined (USE_SYSCONF) || defined (BSD)
#include <unistd.h>
#endif /* USE_SYSCONF or BSD*/

#include "messages.h"
extern char *msg[];


/*
#ifndef FD_SET
#define NFDBITS         32
#define FD_SETSIZE      32
#define FD_SET(n, p)    ((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define FD_CLR(n, p)    ((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)      bzero((char *)(p), sizeof(*(p)))
#endif

#ifdef __hpux
#define getdtablesize()	_NFILE
#endif
*/

/* debugging stuff, LDAP like ... */
#ifdef WEB500GW_DEBUG
int     web500gw_debug;

#define WEB500GW_DEBUG_CONNS    0x01    /* client's requests + results */
#define WEB500GW_DEBUG_TRACE    0x02    /* major function calls etc. */
#define WEB500GW_DEBUG_PACKETS  0x04    /* input from client */
#define WEB500GW_DEBUG_PARSE    0x08    /* parsing/conv of requests/results */
#define WEB500GW_DEBUG_FILTER   0x10    /* search/filter handling */
#define WEB500GW_DEBUG_CONFIG   0x20    /* config files etc. */
#define WEB500GW_DEBUG_UTIL     0x40    /* util function calls */
#define WEB500GW_DEBUG_ANY      0xff

#define Web500gw_debug(level, fmt, arg1, arg2, arg3, arg4)   \
    { \
        if (web500gw_debug & level) \
            fprintf(stderr, fmt, arg1, arg2, arg3, arg4); \
    }
#endif

int     inetd;
int     dtblsize;

char    *config_file;
char    *ldaphost;
int     port;
int     ldapport;
char    *home_dn;
char    *web500dn;
char    *web500pw;
char    *monitor_dn;
int		derefaliases;
int     dosyslog;
int     gwswitch;
int     showonematch;
int     ufnsearch;
int     rootishome;
char    *search_subtree;
char    **search_subtree_oc;
int     maxvalues;
char    *etcdir;
char    *helpfile;
char    *attrfile;
char    *filterfile;
char    *templatefile;
char    *friendlyfile;
char    *messagefile;
char    *friendlyDesc;
char    *local_suffix;
char    *g3togif;
char    *jpegtogif;
char    *log_format;
char    *robots;
LDAPFiltDesc	*filtd;
int     home_dn_OK;		/* checked ? */
int     timelimit;
int     sizelimit;
int     testmode;
int     cgimode;
int     allow_other_servers;
char    *testurl;
int     lastmodified;
int     expires;
char    *other_lang_string;
char    *other_lang_select;
char    *objectclass_attr;
char    *default_filter;
extern char    *generic_ocl[];
extern char    *vcard_ocl[];

extern char *gw_switch();
extern int  do_error();
extern int  do_search();
extern int  do_search_form();
extern int  process_search_form();
extern int  do_read();
extern int  do_special();
extern int  do_bind();
extern int  do_form();
extern int  do_modify();
extern make_upsearch();

/* ldap2html.c */
extern int  web500gw_entry2html();
extern void web500gw_list2html();
extern char *web500gw_err2string();

/* web_util.c */
extern int  http_header();
extern char *dn2url();
extern char *url_encode();
extern void url_decode();
extern char *hex_qdecode();
extern char *html_encode();
extern char *string_encode();
extern char *b64_decode();

/* util.c */
extern int              msg_fprintf(FILE *, char *, char *, ...);
extern int              msg_snprintf(char *, int, char *, char *, ...);
extern time_t           gtime();
extern int              compare();
extern char             *format_date();
extern unsigned char    *web500gw_t61toiso();
extern unsigned char    *web500gw_isotot61();
extern char             *abs_file_name();
extern char             *flag2string();
extern char             *actions2string();
extern char             *opts2string();
extern struct language  *find_language();
extern struct browser_opts  *find_browser();
extern                  find_other_languages();
extern struct access    *find_access();
extern char             *friendly_label();
extern                  log_request();
extern char             *get_ldap_error_str(LDAP *);
extern char             *get_ldap_matched_str(LDAP *);
extern int              get_ldap_result_code(LDAP *);

/* dir_util.c */
extern LDAP *web500gw_ldap_init();
extern char	*friendly_dn();
extern char	*friendly_rdn();
extern int  make_scope();
extern int  isoc();
extern int  isnonleaf();
extern char *pick_oc();
extern char *friendly_dn();
extern char *clean_ufn();
extern char **deref();
extern char *href_dn();
extern char *access_right2string();
extern int  isinhome();
extern char *strip_dn();
extern char *strip_ufn_dn(char *, char *);

extern char	version[];
extern char	Version[];
extern char	LDAP_Version[];
extern char	Compiled[];

#ifndef NOTOK
#define NOTOK   (-1)
#endif
#ifndef OK
#define OK      (0)
#endif

/* own error routine for language support */
#define ldap_err2string web500gw_err2string

/* HTTP methods we are implementing */
#define	UNKNOWN	0
#define GET     1
#define HEAD	2
#define POST    3

/* HTTP response status */
#define DOCUMENT_FOLLOWS    200
#define REDIRECT            302
#define NOT_MODIFIED        304
#define BAD_REQUEST         400
#define AUTH_REQUIRED       401
#define FORBIDDEN           403
#define NOT_FOUND           404
#define CLIENT_TIMEOUT      408
#define SERVER_ERROR        500
#define NOT_IMPLEMENTED     501
#define SERVER_TIMEOUT      504

#define M_DOCUMENT_FOLLOWS    "Ok"
#define M_REDIRECT            "Found"
#define M_NOT_MODIFIED        "Not modified"
#define M_BAD_REQUEST         "Bad request"
#define M_AUTH_REQUIRED       "Authorization required"
#define M_FORBIDDEN           "Forbidden"
#define M_NOT_FOUND           "Not found"
#define M_SERVER_ERROR        "Server error"
#define M_NOT_IMPLEMENTED     "Not implemented"
#define M_UNKNOWN_ERROR       "Unknown error"


FILE    *fp;    /* Our I/O stream */

#define DATE_FORMAT "%a, %d %h %Y %T GMT" /* for strftime: HTTP-Date: format */
#define DATE_ONLY   "%a, %d %h %Y GMT"    /* for strftime: Date only */

#define CT_TEXT         "text/plain"
#define CT_VCARD        "text/x-vcard"
#define CT_HTML         "text/html"
#define CT_IMG_JPEG     "image/jpeg"
#define CT_IMG_GIF      "image/gif"
#define CT_SND_BASIC    "audio/basic"

#define MAX_ATTRS 64

typedef struct request {
    struct access *r_access;
    char        *r_client_host;
    char        *r_request;      /* complete req: GET /DN?... HTTP/1.0 */
    int         r_method;        /* req. methos: GET, HEAD, POST */
    char        *r_query;        /* user query: /DN?... */
    int         r_httpversion;   /* 1 for HTTP/1.0 */
    int         r_content_length;
    char        *r_if_modified_since;
    char        *r_useragent;       /* User-Agent string */
    struct browser_opts *r_browser; /* requesting browser class */
    char        *r_referer;         /* Referer header */
    char        *r_authorization;   /* Authorization header */
    time_t       r_tm;           /* time when req. arrived */
    char        *r_ldaphost;     /* user requested ldap server */
    int          r_ldapport;     /* user requested ldap port */
    LDAP        *r_ld;
    char        *r_dn;           /* user specified DN (decoded)*/
    char        *r_binddn;       /* bind as DN */
    char        *r_attrs[MAX_ATTRS];    /* user req. attributes */
    int          r_attrnumb;     /* number of user req. attributes */
    unsigned long r_flags;       /* user req. flags */
    unsigned long r_actions;     /* action */
    char        *r_template;     /* user req. template flag ($tmpl=xxx) */
    char        *r_language;     /* user req. language flag ($lang=xx) */
    char        *r_filter;       /* user req. filter */
    char        *r_postdata;     /* contains data when method == POST */
} REQUEST;

typedef struct response {
    char            resp_date[256];          /* HTTP Date: */
    int             resp_expires;            /* for HTTP Expires: */
    char            *resp_content_type;      /* HTTP Content-type: */
    int             resp_content_length;
    char            *resp_last_modified;
    char            *resp_location;
    char            *resp_extra;
    struct language *resp_language;
    int             resp_httpheader;    /* already sent HTTP header? */
    int             resp_htmlheader;    /* already sent start of HTML */
    int             resp_status;        /* Returned HTTP response status */
    int             resp_error;   /* LDAP error code if HTTP error (>= 400) */
} RESPONSE;

/* for sorting a list of entries */

#define MAX_LISTSIZE BUFSIZ
struct dncompare {          /* used to sort a list of entries */
    char    *sortstring;    /* the sort string */
    char    *href;          /* the string to print */
    char    *friendly_oc;   /* the friendly string for the objectclass */
    char    *oc;            /* objectclass */
    struct ldap_disptmpl    *tmpl;  /* display template */
    char    *dn;            /* the DN */
    LDAPMessage *entry;     /* the entry */
} *dnlist[MAX_LISTSIZE];

/*
#ifndef aix
extern int fprintf();
#endif
*/

/* Non-text attributes handled by do_special() */
#define     ATTR_JPEG   1
#define     ATTR_G3FAX  2
#define     ATTR_AUDIO  3

/* FLAGS for ldap2html: How to format output 
 *   and for special actions (bind, search)  */

#define FLAG_LIST        0x00000001
#define FLAG_ONELINE     0x00000002
#define FLAG_TABLE       0x00000004
#define FLAG_BORDER      0x00000008
#define FLAG_TABLE_BORDER (FLAG_TABLE|FLAG_BORDER)
#define FLAG_VALSONLY    0x00000010
#define FLAG_ENTRYONLY   0x00000020   /* No banner/headline/trailer ... */
#define FLAG_IMG         0x00000100   /* use icons as attr.names etc. */
#define FLAG_LAYOUT      0x0000ffff   /* mask for layout flags */

#define FLAG_ATTRSONLY   0x00010000   /* implicit, if user gives attrs */
#define FLAG_FILTER      0x00020000   /* implicit, if user gives filter */
#define FLAG_ALT         0x00040000   /* implicit, for old '/L' URL's */
#define FLAG_LANGUAGE    0x00100000   /* request a special language */
#define FLAG_TMPL        0x00200000   /* request a special template */
#define FLAG_VCARD       0x00400000   /* implicit vcard attr: entry as vCard*/
#define FLAG_NOCACHE     0x00800000   /* send always no-cache page  */
#define FLAG_NOHREFDN    0x01000000   /* don't print DN as HREF */
#define FLAG_NODN        0x02000000   /* don't print DN of entry/search res. */
#define FLAG_DEREFALIAS  0x04000000   /* dereference alias entries */
#define FLAG_SEARCHACT   0x08000000   /* in a search action */

/* 
 * Useful combinations:
 * Very compact: all values in one line 
 *  (FLAG_ONELINE|FLAG_VALSONLY)
 * Compact: all attribut names and values in one line 
 *  (FLAG_ONELINE)
 * Attribute name and values, each in a line for itself 
 * Not compact:  Attribut name and values as <DL>, all attr. values in one line
 *  (FLAG_LIST|FLAG_ONELINE|FLAG_ATTRSONLY)
 * Not compact at all: Attribut name and values as <DL>, 
 *                     each value in a line for itself
 *  (FLAG_LIST|FLAG_ATTRSONLY)
 */

#define ACTION_BIND        0x00000001   /* Bind: Ask for DN + password */
#define ACTION_FORM        0x00000002   /* Modify form */
#define ACTION_ADDFORM     0x00000004   /* Add form */
#define ACTION_DELETEFORM  0x00000008   /* Delete form */
#define ACTION_MODRDNFORM  0x00000010   /* modify RDN form */
#define ACTION_MODIFY      0x00000100   /* Do modify */
#define ACTION_ADD         0x00000200   /* Do add */
#define ACTION_DELETE      0x00000400   /* Do delete */
#define ACTION_MODRDN      0x00000800   /* Do modify RDN */
#define ACTION_SEARCH_FORM 0x00001000   /* print a search form */
#define ACTION_DO_SEARCH   0x00002000   /* do extended search */
#define ACTION_NBIND       0x00004000   /* Bind via HTTP basic auth - experimental */


/* 
 * We want to try our best to serve different WWW-Browsers.
 * So browsers with table support may show entries with tables etc.
 */
struct browser_opts {
    char        *b_desc;     /* a string used in the logfile */
    char        *b_pattern;  /* pattern matches on browser name */
    unsigned    b_opts;      /* what features has the browser */
    unsigned    b_flags;     /* default display flags */
    unsigned    b_upsearch;  /* location of search and navigate tool */
    struct browser_opts *b_next;

};

struct browser_opts *browser_opts;

/* b_upsearch: */
#define UPSEARCH_NONE       0x0000  /* no navigation menu at all */
#define UPSEARCH_SMALL      0x0001  /* in one line, each RDN is a link */
#define UPSEARCH_MENU       0x0002  /* SELECT FORM */
#define UPSEARCH_LIST       0x0004  /* a <UL> list */
#define UPSEARCH_ON_TOP     0x0100  /* on top before entry display starts */
#define UPSEARCH_ON_BOTTOM  0x0200  /* on bottom */


/* b_opts: Features */
#define B_FORMS       0x0001    /* form capable */
#define B_HIDDEN      0x0004    /* and HIDDEN type works */
#define B_MAILTO      0x0008    /* understands MAILTO URL's */
#define B_IMG         0x0010    /* inline images */
#define B_JPEG        0x0020    /* inline JPEG */
#define B_JPEG_IMG    B_IMG|B_JPEG
#define B_TABLE       0x0040    /* implements TABLE */
/* ... and bugs */
#define B_NEEDSUFFIX  0x0100    /* need a suffix to recognize GIF/JPEG... */


/* Some statistics */

struct statistics {
    time_t  stat_start_time;        /* start of the programm */
    int     stat_active_conns;      /* currently active connections */
    int     stat_total_conns;       /* total connections */
} statistics;

/* configuration files */

struct conf_files {
    char    *c_helpfile;            /* name of helpfile */
    char    *c_attrfile;            /* name of helpfile for attributes */
    char    *c_friendlyfile;        /* name of friendlyfile */
    LDAPFriendlyMap *c_fm;
    char    *c_messagefile;         /* name of messagefile */
    char    *c_msg[MSG_count + 1];
    char    **c_errmsg;
};

/* different language support */

struct language {
    char                *l_content_lang;/* Header: Content-Language */
    char                *l_pattern;     /* Matching pattern */
    char                *l_suffix;      /* suffix for lang. specific files */
    struct conf_files   *l_conf;        /* the language specific files */
    struct language     *l_next;
};
struct language *languages;
struct language *default_language;

/* Access control */
struct access {
    char                *a_desc;          /* a short description */
    char                *a_domainpattern; /* a regexp for domain */
    unsigned int        a_rights;         /* access rights */
    int                 a_sizelimit;      /* max. search results */
    char                *a_language;      /* default language */
    char                *a_start_dn;      /* Start DN (for URL /M) */
    char                *a_bind_dn;       /* bind as */
    char                *a_bind_pw;       /* ... with pw */
    char                *a_suffix;        /* suffix for ACCESS specific files */
    char                *a_tmplfile;      /* templatefile to use */
    struct ldap_disptmpl *a_tmpllist;     /* display templates */
    char                *a_filterfile;    /* name of search filter file */
    LDAPFiltDesc        *a_filtd;
    struct language     *a_lang;          /* the language structure */
    struct access          *a_next;
};
struct access *accesses;

/* Access rights */
#define ACCESS_FULL        0xff        /* full access */
#define ACCESS_READ        0x01        /* read access */
#define ACCESS_ATTRS       0x02        /* allow attrs not in template */
#define ACCESS_WRITE       0x04        /* allow modify */
#define ACCESS_NOTHING     0x00        /* no access at all */


/* configuration parameters */
struct config_params {
    char    *c_name;            /* Name of config parameter */
    int     c_type;             /* Type */
#define C_STRING    1
#define C_INT       2
#define C_BOOL      3
#define C_ACCESS    4
#define C_LANG      5
#define C_BROWSER   6
    char    **c_ptr;            /* Pointer to variable */
};

extern struct config_params config_params[];

struct config_options {
    char           *o_name;         /* name */
    unsigned int    o_value;        /* value */
};

extern struct config_options browser_options[];
extern struct config_options upsearch_options[];
extern struct config_options disp_flags[];
extern struct config_options disp_actions[];
extern struct config_options access_rights[];

#define MOD_ATTR_USED       (LDAPMod *)-1
#define MOD_VALUE_USED      (char *)-1
