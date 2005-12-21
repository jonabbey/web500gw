/*
 * init.c:      some initialization code
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
 * $Id: init.c,v 1.2 2001/04/26 22:16:53 dekarl Exp $
 */

#include "web500gw.h"

/* Initialize the global variables */
char    *config_file = WEB500GW_CONFIG;
int     port = WEB500PORT;
char    *ldaphost = LDAPHOST;
int     ldapport = LDAP_PORT;
char    *home_dn = DEFAULT_HOMEDN;
char    *web500dn = WEB500DN;
char    *web500pw = WEB500PW;
char    *monitor_dn = NULL;
int     dosyslog = 0;
int     gwswitch = 0;
int     derefaliases = 0;
int     showonematch = 1;
int     ufnsearch = 1;
int     rootishome = 0;
char    *search_subtree = SEARCH_SUBTREE;
int     maxvalues = 0;
char    *etcdir = WEB500GWDIR;
char    *helpfile = HELPFILE;
char    *attrfile = ATTRFILE;
char    *filterfile = FILTERFILE;
char    *templatefile = TEMPLATEFILE;
char    *friendlyfile = FRIENDLYFILE;
char    *messagefile = MESSAGEFILE;
char    *g3togif = NULL;
char    *jpegtogif = NULL;
char    *log_format = LOGFORMAT;
char    *robots = ROBOTS;
// XARL char    *friendlyDesc = NULL;
int     home_dn_OK = 0;       /* checked ? */
int     timelimit = TIMELIMIT;
int     sizelimit = SIZELIMIT;
int     testmode = 0;
int     cgimode = 0;
int     allow_other_servers = 0;
char    *testurl = NULL;
int     lastmodified = 1;
int     expires = -1;

char **search_subtree_oc;
char *objectclass_attr = "objectClass";
char *default_filter    = "objectClass=*";
char *generic_ocl[] = { "web500gw_generic", 0 };
char *vcard_ocl[]   = { "web500gw_vcard", 0 };

/* Initialize some arrays */

/* Accepted configuration parameters in config file */

struct config_params config_params [] = {
    "access",       C_ACCESS, (char **)&accesses,
    "attrfile",     C_STRING, (char **)&attrfile,
    "browser",      C_BROWSER,(char **)&browser_opts,
    "derefaliases", C_BOOL,   (char **)&derefaliases,
    "etcdir",       C_STRING, (char **)&etcdir,
    "expires",      C_INT,    (char **)&expires,
    "filterfile",   C_STRING, (char **)&filterfile,
    "friendlyfile", C_STRING, (char **)&friendlyfile,
    "g3togif",      C_STRING, (char **)&g3togif,
    "gwswitch",     C_BOOL,   (char **)&gwswitch,
    "helpfile",     C_STRING, (char **)&helpfile,
    "homedn",       C_STRING, (char **)&home_dn,
    "jpegtogif",    C_STRING, (char **)&jpegtogif,
    "language",     C_LANG,   (char **)&languages,
    "lastmodified", C_BOOL,   (char **)&lastmodified,
    "ldapserver",   C_STRING, (char **)&ldaphost,
    "ldapport",     C_INT,    (char **)&ldapport,
    "logformat",    C_STRING, (char **)&log_format,
    "maxvalues",    C_INT,    (char **)&maxvalues,
    "messagefile",  C_STRING, (char **)&messagefile,
    "monitordn",    C_STRING, (char **)&monitor_dn,
    "otherservers", C_BOOL,   (char **)&allow_other_servers,
    "port",         C_INT,    (char **)&port,
    "subsearch",    C_STRING, (char **)&search_subtree,
    "robots",       C_STRING, (char **)&robots,
    "rootishome",   C_BOOL,   (char **)&rootishome,
    "showonematch", C_BOOL,   (char **)&showonematch,
    "sizelimit",    C_INT,    (char **)&sizelimit,
    "syslog",       C_BOOL,   (char **)&dosyslog,
    "templatefile", C_STRING, (char **)&templatefile,
    "timelimit",    C_INT,    (char **)&timelimit,
    "ufnsearch",    C_BOOL,   (char **)&ufnsearch,
    "web500dn",     C_STRING, (char **)&web500dn,
    "web500pw",     C_STRING, (char **)&web500pw,
    NULL
};


/* Flags */
struct config_options disp_flags[]    = {
    { "list",         FLAG_LIST         },
    { "oneline",      FLAG_ONELINE      },
    { "table",        FLAG_TABLE        },
    { "btable",       FLAG_TABLE_BORDER },
    { "valsonly",     FLAG_VALSONLY     },
    { "entryonly",    FLAG_ENTRYONLY    },
    { "img",          FLAG_IMG          },
    { "nocache",      FLAG_NOCACHE      },
    { "nohrefdn",     FLAG_NOHREFDN     },
    { "nodn",         FLAG_NODN         },
    { "deref",        FLAG_DEREFALIAS   },
    { NULL,           0                 }
};
struct config_options disp_actions[] = {
    { "bind",         ACTION_BIND         },
    { "nbind",        ACTION_NBIND        },
    { "form",         ACTION_FORM         },
    { "addform",      ACTION_ADDFORM      },
    { "deleteform",   ACTION_DELETEFORM   },
    { "modrdnform",   ACTION_MODRDNFORM   },
    { "modify",       ACTION_MODIFY       },
    { "add",          ACTION_ADD          },
    { "delete",       ACTION_DELETE       },
    { "modrdn",       ACTION_MODRDN       },
    { "searchform",   ACTION_SEARCH_FORM  },
    { "search",       ACTION_DO_SEARCH    },
    { NULL,           0                 }
};

/* ACCESS rights */
struct config_options access_rights[]    = {
    { "full",       ACCESS_FULL},
    { "read",       ACCESS_READ},
    { "readall",    (ACCESS_READ | ACCESS_ATTRS)},
    { "none",       ACCESS_NOTHING},
    { NULL,         0}
};

/* browser options */
struct config_options browser_options[] = {
    { "html32", (B_FORMS|B_HIDDEN|B_MAILTO|B_JPEG_IMG|B_TABLE) },
    { "forms",  (B_FORMS|B_HIDDEN) },
    { "mailto", B_MAILTO           },
    { "img",    B_IMG              },
    { "jpg",    B_JPEG_IMG         },
    { NULL,     0                  }
};

struct config_options upsearch_options[] = {
    { "top",    UPSEARCH_ON_TOP     },
    { "bottom", UPSEARCH_ON_BOTTOM  },
    { "small",  UPSEARCH_SMALL      },
    { "list",   UPSEARCH_LIST       },
    { "menu",   UPSEARCH_MENU       },
    { "none",   UPSEARCH_NONE       },
    { NULL,     0}
};

