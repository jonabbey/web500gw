/*
 * $Id: config.c,v 1.2 2001/04/26 22:16:53 dekarl Exp $
 */

#include "web500gw.h"

/* read in configuration file */
int
read_configuration(
    char    *configfile
)
{
    FILE    *cf;
    char    line[1024], *start, *value, *buf;
    int     bufsize, lineno = 0, end, i, found;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_CONFIG,
        "read_configuration(%s)\n", configfile, 0, 0, 0);
#endif

    if (! (cf = fopen(configfile, "r"))) {
        fprintf(stderr, "Couldn't open configuration file %s\n", configfile);
        return(1);
    }
    buf = line;
    bufsize = sizeof(line);

    while (fgets(buf, bufsize, cf)) {
        lineno++;
        if (line[end = strlen(line) - 1] != '\n') {
#ifdef WEB500GW_DEBUG
            Web500gw_debug(WEB500GW_DEBUG_CONFIG,
                "read_configuration: %s - line %d: missing newline or line too long",
                configfile, lineno, 0, 0);
#endif
            continue;
        }
        start = line;
        /* skip leading whitespaces */
        while (isspace(*start)) ++start;

        if (*start == '#')     /* comment */
            continue;
        /* strip trailing whitespaces */
        while (end > 0 && isspace(line[end - 1])) end--;
        line[end] = '\0';

        if (*start == '\0')    /* skip blank lines */
            continue;

        if (line[end - 1] == '\\') {   /* continuation */
            line[end - 1] = '\0';
            buf = &line[end - 1];
            bufsize = sizeof(line) - end;
            continue;
        } else {
               buf = line;
               bufsize = sizeof(line);
        }

        found = 0;
        for (i = 0; config_params[i].c_name; i++) {
            if (strncmp(start, config_params[i].c_name,
                    strlen(config_params[i].c_name)) == 0) {
                if ((value = strchr(start, ':')) == NULL) {
                    break;
                }
                value++;
                found = 1;
                while (isspace(*value)) ++value;

#ifdef WEB500GW_DEBUG
            Web500gw_debug(WEB500GW_DEBUG_CONFIG, "  line %d: %s = >%s<\n",
                lineno, config_params[i].c_name, value, 0);
#endif
                if (config_params[i].c_ptr != NULL) {
                    switch (config_params[i].c_type) {
                    case C_STRING:
                        *config_params[i].c_ptr = strdup(value);
                        break;
                    case C_INT:
                        *config_params[i].c_ptr = (char *)atoi(value);
                        /* *(int *)config_params[i].c_ptr = atoi(value);
                         * */
                        break;
                    case C_BOOL:
                        if (strncasecmp(value, "on", 2) == 0 ||
                            strncasecmp(value, "true", 4) == 0 ||
                            strncasecmp(value, "yes", 3) == 0 ||
                            strncasecmp(value, "1", 1) == 0) {
                                *config_params[i].c_ptr = (char *)1;
                        } else {
                            *config_params[i].c_ptr = (char *)0;
                        }
                        break;
                    case C_ACCESS:
                        if (read_access(value))
                            found = 0;
                        break;
                    case C_LANG:
                        if (read_lang(value))
                            found = 0;
                        break;
                    case C_BROWSER:
                        if (read_browser(value))
                            found = 0;
                        break;
                    default:
                        found = 0;
                   }
                }
                break;
            }
        }
        if (! found) {
#ifdef WEB500GW_DEBUG
            Web500gw_debug(WEB500GW_DEBUG_CONFIG,
                "Unknown option in line %d: %s\n", lineno, line, 0, 0);
#endif
        }
    }
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_CONFIG,
        "read_configuration ><\n", 0, 0, 0, 0);
#endif
    return(0);
}


read_access (
    char *line
)
{
    struct access   *access, *a;
    char            *start, *next, *end, *error = NULL;
    int             recno = 0;
    char            sep = ':';

#ifdef WEB500GW_DEBUG
    Web500gw_debug((WEB500GW_DEBUG_CONFIG), 
        "   read_access(%s)\n", line, 0, 0, 0);
#endif

    access = (struct access *) calloc(1, sizeof(struct access));

    start = line;
   /* ACCESS description */
    recno++;
    while (isspace(*start)) ++start;
    if (! (end = next = strchr(start, sep)))
        goto error;
    *end-- = '\0';
    while (end > start && isspace(*end)) *end-- = '\0'; 
    if (start) {
        access->a_desc = strdup(start);
    } else {
        error = "ACCESS must contain a description";
        goto error;
    }
    start = next + 1;

    /* ACCESS client pattern */
    recno++;
    while (isspace(*start)) ++start;
    if (! (end = next = strchr(start, sep)))
        goto error;
    *end-- = '\0';
    while (end > start && isspace(*end)) *end-- = '\0';
    if (*start) {
        access->a_domainpattern = strdup(start);
    } else {
        error = "ACCESS must contain a pattern";
        goto error;
    }
    start = next + 1;

    /* ACCESS rights */
    recno++;
    while (isspace(*start)) ++start;
    if (! (end = next = strchr(start, sep)))
        goto error;
    *end-- = '\0';
    while (end > start && isspace(*end)) *end-- = '\0'; 
    if (*start)
        access->a_rights = string2access_right(start);
    start = next + 1;

    /* searchlimit */
    recno++;
    while (isspace(*start)) ++start;
    if (! (end = next = strchr(start, sep)))
        goto error;
    *end-- = '\0';
    while (end > start && isspace(*end)) *end-- = '\0'; 
    if (*start)
        access->a_sizelimit = atoi(start);
    start = next + 1;

    /* ACCESS language */
    recno++;
    while (isspace(*start)) ++start;
    if (! (end = next = strchr(start, sep)))
        goto error;
    *end-- = '\0';
    while (end > start && isspace(*end)) *end-- = '\0'; 
    if (*start)
        access->a_language = strdup(start);
    start = next + 1;

    /* ACCESS home DN*/
    recno++;
    while (isspace(*start)) ++start;
    if (! (end = next = strchr(start, sep)))
        goto error;
    *end-- = '\0';
    while (end > start && isspace(*end)) *end-- = '\0'; 
    if (*start)
        access->a_start_dn = strdup(start);
    start = next + 1;

    /* ACCESS bind DN*/
    recno++;
    while (isspace(*start)) ++start;
    if (! (end = next = strchr(start, sep)))
        goto error;
    *end-- = '\0';
    while (end > start && isspace(*end)) *end-- = '\0'; 
    if (*start)
        access->a_bind_dn = strdup(start);
    start = next + 1;

    /* ACCESS bind password */
    recno++;
    while (isspace(*start)) ++start;
    if (! (end = next = strchr(start, sep)))
        goto error;
    *end-- = '\0';
    while (end > start && isspace(*end)) *end-- = '\0'; 
    if (*start)
        access->a_bind_pw = strdup(start);
    start = next + 1;

    /* suffix for ACCESS specific files */
    recno++;
    while (isspace(*start)) ++start;
    end = start + strlen(start) - 1;
    while (end > start && isspace(*end)) end--; 
    if (*start)
        access->a_suffix = strdup(start);

    if (accesses == (struct access *)0) {     /* first ACCESS */
        accesses = access;
    } else {                /* add to end of list */
        a = accesses;
        while (a->a_next) a = a->a_next;
        a->a_next = access;
    }
    return(0);

error:
#ifdef WEB500GW_DEBUG
    Web500gw_debug((WEB500GW_DEBUG_CONFIG), 
        "   read_access error: line %s record %d: %s\n", 
        line, recno, error ? error : "missing record", 0);
#endif
    free(access);
    return(1);
}

read_lang (
    char *line
)
{
    struct language  *lang, *l;
    char        *start, *next, *end, *error = NULL;
    int             recno = 0;
    char sep = ':';

#ifdef WEB500GW_DEBUG
    Web500gw_debug((WEB500GW_DEBUG_CONFIG), 
        "   read_lang(%s)\n", line, 0, 0, 0);
#endif

    lang = (struct language *) calloc(1, sizeof(struct language));

    start = line;
   /* Language Content-Language */
    recno++;
    while (isspace(*start)) ++start;
    if (! (end = next = strchr(start, sep)))
        goto error;
    *end-- = '\0';
    while (end > start && isspace(*end)) *end-- = '\0'; 
    if (start) {
        lang->l_content_lang = strdup(start);
    } else {
        error = "Language definition must contain a Content-Language";
        goto error;
    }
    start = next + 1;

    /* Language pattern */
    recno++;
    while (isspace(*start)) ++start;
    if (! (end = next = strchr(start, sep)))
        goto error;
    *end-- = '\0';
    while (end > start && isspace(*end)) *end-- = '\0'; 
    if (start) {
        lang->l_pattern = strdup(start);
    } else {
        error = "Language definition must contain a pattern";
        goto error;
    }
    start = next + 1;

    /* Language suffix*/
    recno++;
    while (isspace(*start)) ++start;
    end = start + strlen(start) - 1;
    while (end > start && isspace(*end)) *end-- = '\0'; 
    if (start)
        lang->l_suffix = strdup(start);

    if (languages == (struct language *)0) {     /* first language */
        languages = lang;
    } else {                /* add to end of list */
        l = languages;
        while (l->l_next) l = l->l_next;
        l->l_next = lang;
    }
    return(0);

error:
#ifdef WEB500GW_DEBUG
    Web500gw_debug((WEB500GW_DEBUG_CONFIG), 
        "   read_lang error: line %si record %d: %s\n", 
        line, recno, error ? error : "", 0);
#endif
    free(lang);
    return(1);
}

read_browser (
    char *line
)
{
    struct browser_opts  *browser, *b;
    char        *start, *next, *end, *error = NULL, *cp;
    int         recno = 0;
    char sep = ':';

#ifdef WEB500GW_DEBUG
    Web500gw_debug((WEB500GW_DEBUG_CONFIG), 
        "   read_browser(%s)\n", line, 0, 0, 0);
#endif

    browser = (struct browser_opts *) calloc(1, sizeof(struct browser_opts));

    start = line;
   /* Browser description */
    recno++;
    while (isspace(*start)) ++start;
    if (! (end = next = strchr(start, sep)))
        goto error;
    *end-- = '\0';
    while (end > start && isspace(*end)) *end-- = '\0'; 
    if (start) {
        browser->b_desc = strdup(start);
    }
    start = next + 1;

    /* Browser pattern */
    recno++;
    while (isspace(*start)) ++start;
    if (! (end = next = strchr(start, sep)))
        goto error;
    *end-- = '\0';
    while (end > start && isspace(*end)) *end-- = '\0';
    if (*start) {
        browser->b_pattern = strdup(start);
    } else {
        error = "Browser must contain a pattern";
        goto error;
    }
    start = next + 1;

    /* Browser options */
    recno++;
    while (isspace(*start)) ++start;
    if (! (end = next = strchr(start, sep)))
        goto error;
    *end-- = '\0';
    while (end > start && isspace(*end)) *end-- = '\0'; 
    while (start && *start) {
        if ((cp = strchr(start, ',')))
            *cp++ = '\0';
        browser->b_opts |= string2opts(start, browser_options);
        start = cp;
    }
    start = next + 1;

    /* Browser default display flags */
    recno++;
    while (isspace(*start)) ++start;
    if (! (end = next = strchr(start, sep)))
        goto error;
    *end-- = '\0';
    while (end > start && isspace(*end)) *end-- = '\0'; 
    while (start && *start) {
        if ((cp = strchr(start, ',')))
            *cp++ = '\0';
        browser->b_flags |= string2flag(start);
        start = cp;
    }
    start = next + 1;

    /* Browser search and navigate location */
    recno++;
    while (isspace(*start)) ++start;
    end = start + strlen(start) - 1;
    while (end > start && isspace(*end)) *end-- = '\0'; 
    while (start && *start) {
        if ((cp = strchr(start, ',')))
            *cp++ = '\0';
        browser->b_upsearch |= string2opts(start, upsearch_options);
        start = cp;
    }

    if (browser_opts == (struct browser_opts *)0) {     /* first ACCESS */
        browser_opts = browser;
    } else {                /* add to end of list */
        b = browser_opts;
        while (b->b_next) b = b->b_next;
        b->b_next = browser;
    }
    return(0);

error:
#ifdef WEB500GW_DEBUG
    Web500gw_debug((WEB500GW_DEBUG_CONFIG), 
        "   read_browser error: line %s record %d: %s\n", 
        line, recno, error ? error : "missing record", 0);
#endif
    free(browser);
    return(1);
}



/* message file handling,
 * from Hallvard B Furuseth <h.b.furuseth@usit.uio.no>
 */


struct ldaperror {
    int     e_code;
    char    *e_reason;
};

static struct ldaperror ldap_errlist[] = {
	{ LDAP_SUCCESS,                      "LDAP_SUCCESS" },
	{ LDAP_OPERATIONS_ERROR,             "LDAP_OPERATIONS_ERROR" },
	{ LDAP_PROTOCOL_ERROR,               "LDAP_PROTOCOL_ERROR" },
	{ LDAP_TIMELIMIT_EXCEEDED,           "LDAP_TIMELIMIT_EXCEEDED" },
	{ LDAP_SIZELIMIT_EXCEEDED,           "LDAP_SIZELIMIT_EXCEEDED" },
	{ LDAP_COMPARE_FALSE,                "LDAP_COMPARE_FALSE" },
	{ LDAP_COMPARE_TRUE,                 "LDAP_COMPARE_TRUE" },
	{ LDAP_STRONG_AUTH_NOT_SUPPORTED,    "LDAP_STRONG_AUTH_NOT_SUPPORTED" },
	{ LDAP_STRONG_AUTH_REQUIRED,         "LDAP_STRONG_AUTH_REQUIRED" },
    { LDAP_PARTIAL_RESULTS,              "LDAP_PARTIAL_RESULTS" },
	{ LDAP_NO_SUCH_ATTRIBUTE,            "LDAP_NO_SUCH_ATTRIBUTE" },
	{ LDAP_UNDEFINED_TYPE,               "LDAP_UNDEFINED_TYPE" },
	{ LDAP_INAPPROPRIATE_MATCHING,       "LDAP_INAPPROPRIATE_MATCHING" },
	{ LDAP_CONSTRAINT_VIOLATION,         "LDAP_CONSTRAINT_VIOLATION" },
	{ LDAP_TYPE_OR_VALUE_EXISTS,         "LDAP_TYPE_OR_VALUE_EXISTS" },
	{ LDAP_INVALID_SYNTAX,               "LDAP_INVALID_SYNTAX" },
	{ LDAP_NO_SUCH_OBJECT,               "LDAP_NO_SUCH_OBJECT" },
	{ LDAP_ALIAS_PROBLEM,                "LDAP_ALIAS_PROBLEM" },
	{ LDAP_INVALID_DN_SYNTAX,            "LDAP_INVALID_DN_SYNTAX" },
	{ LDAP_IS_LEAF,                      "LDAP_IS_LEAF" },
	{ LDAP_ALIAS_DEREF_PROBLEM,          "LDAP_ALIAS_DEREF_PROBLEM" },
	{ LDAP_INAPPROPRIATE_AUTH,           "LDAP_INAPPROPRIATE_AUTH" },
	{ LDAP_INVALID_CREDENTIALS,          "LDAP_INVALID_CREDENTIALS" },
	{ LDAP_INSUFFICIENT_ACCESS,          "LDAP_INSUFFICIENT_ACCESS" },
	{ LDAP_BUSY,                         "LDAP_BUSY" },
	{ LDAP_UNAVAILABLE,                  "LDAP_UNAVAILABLE" },
	{ LDAP_UNWILLING_TO_PERFORM,         "LDAP_UNWILLING_TO_PERFORM" },
	{ LDAP_LOOP_DETECT,                  "LDAP_LOOP_DETECT" },
	{ LDAP_NAMING_VIOLATION,             "LDAP_NAMING_VIOLATION" },
	{ LDAP_OBJECT_CLASS_VIOLATION,       "LDAP_OBJECT_CLASS_VIOLATION" },
	{ LDAP_NOT_ALLOWED_ON_NONLEAF,       "LDAP_NOT_ALLOWED_ON_NONLEAF" },
	{ LDAP_NOT_ALLOWED_ON_RDN,           "LDAP_NOT_ALLOWED_ON_RDN" },
	{ LDAP_ALREADY_EXISTS,               "LDAP_ALREADY_EXISTS" },
	{ LDAP_NO_OBJECT_CLASS_MODS,         "LDAP_NO_OBJECT_CLASS_MODS" },
	{ LDAP_RESULTS_TOO_LARGE,            "LDAP_RESULTS_TOO_LARGE" },
	{ LDAP_OTHER,                        "LDAP_OTHER" },
	{ LDAP_SERVER_DOWN,                  "LDAP_SERVER_DOWN" },
	{ LDAP_LOCAL_ERROR,                  "LDAP_LOCAL_ERROR" },
	{ LDAP_ENCODING_ERROR,               "LDAP_ENCODING_ERROR" },
	{ LDAP_DECODING_ERROR,               "LDAP_DECODING_ERROR" },
	{ LDAP_TIMEOUT,                      "LDAP_TIMEOUT" },
	{ LDAP_AUTH_UNKNOWN,                 "LDAP_AUTH_UNKNOWN" },
	{ LDAP_FILTER_ERROR,                 "LDAP_FILTER_ERROR" },
	{ LDAP_USER_CANCELLED,               "LDAP_USER_CANCELLED" },
	{ LDAP_PARAM_ERROR,                  "LDAP_PARAM_ERROR" },
	{ LDAP_NO_MEMORY,                    "LDAP_NO_MEMORY" },
	{ -1, 0 }
};

char *
web500gw_err2string(
    int         err,
    RESPONSE    *resp
)
{
    int i;

    for (i = 0; ldap_errlist[i].e_code != -1; i++) {
        if (err == ldap_errlist[i].e_code) {
            if (resp->resp_language->l_conf->c_errmsg[i])
                /* language error desription */
                return(resp->resp_language->l_conf->c_errmsg[i]);
            else
                /* generic error description */
                return(ldap_errlist[i].e_reason);
        }
    }

    return(MSG_UNKNOWN_ERROR);
}
/* Reads in a message file */

void read_messages (
    char    *messagefile,
    struct language    *lang)
{
    FILE *f;
    struct stat statbuf;
    char *buf, *cp, *cp2, **filemsgs, **filemsg, *message;
    int	i,len;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  read_messages (%s, %s)\n",
        messagefile, lang->l_content_lang, 0, 0);
#endif

    f = fopen(messagefile, "r");
    if (!f) {
#ifdef WEB500GW_DEBUG
        Web500gw_debug(WEB500GW_DEBUG_CONFIG, 
            "Failed to open message file %s\n", messagefile, 0, 0, 0);
#endif
	    exit(1);
    }

    if (fstat(fileno(f), &statbuf) < 0) {
#ifdef WEB500GW_DEBUG
        Web500gw_debug(WEB500GW_DEBUG_CONFIG,
	       "Failed to stat message file %s\n", messagefile, 0, 0, 0);
#endif
	   exit(1);
    }

    buf = (char *) calloc((len = statbuf.st_size) + 4, 1);
    strcpy (buf, "X\n");
    fread(buf+2, 1, len, f);
    len += 2;
    while (len && ((unsigned char *)buf)[len-1] > ' ') len--;
    buf[len] = buf[len+1] = '\0';
    fclose(f);

    for (i = 2, cp = buf;  (cp = strchr(cp, ':')) != NULL;  cp++, i++);
    i *= 2;
    filemsg = filemsgs = (char **)calloc (i, sizeof(char *));

    for (cp = buf;  (cp = strchr(cp, '\n')) != NULL; ) {
        if (*++cp == ':' || *cp == '#') {
	       for (cp2 = cp;  *(unsigned char *)--cp2 <= ' '; );
	       *++cp2 = '\0';
	       if (*cp == ':') {
		      *filemsg++ = ++cp;      /* name */
		      while (*(unsigned char *)cp > ' ')
                  cp++;
              *cp++ = '\0';
		      while ((*(unsigned char *)cp <= ' ') && *(unsigned char *)cp != '\n')
                 cp++;
              if (*(unsigned char *)cp != '\n') {
		          *filemsg++ = cp;
              } else {
#ifdef WEB500GW_DEBUG
                Web500gw_debug(WEB500GW_DEBUG_CONFIG, 
                    "read_messages: message for \"%s\"  empty!\n", 
                    *(filemsg-1), 0, 0, 0);
#endif
                  *filemsg++ = "";
              }
	       }
	   }
    }
    *filemsg = NULL;
    for (i = 0;  (message = msg[i]);  i++) {
	   for (filemsg = filemsgs;  *filemsg;  filemsg += 2) {
	       if (strcmp (*filemsg, message) == 0) {
/*              fprintf(stderr, "%s: %s\n", msg[i], filemsg[1]); */
		       lang->l_conf->c_msg[i] = filemsg[1];
		       goto found;
	       }
        }
#ifdef WEB500GW_DEBUG
        Web500gw_debug(WEB500GW_DEBUG_CONFIG,
            "Warning: Message \":%s\" not found in message file %s\n",
            message, messagefile, 0, 0);
#endif

       lang->l_conf->c_msg[i] = "";
found: ;
    }

    /* LDAP error codes */
    lang->l_conf->c_errmsg = malloc (sizeof(ldap_errlist));
    for (i = 0; (message = ldap_errlist[i].e_reason); i++) {
        for (filemsg = filemsgs;  *filemsg;  filemsg += 2) {
            if (strcmp (*filemsg, message) == 0) {
                lang->l_conf->c_errmsg[i] = filemsg[1];
                goto found1;
            }
        }
#ifdef WEB500GW_DEBUG
        Web500gw_debug(WEB500GW_DEBUG_CONFIG,
            "Warning: Message \":%s\" not found in message file %s\n",
            message, messagefile, 0, 0);
#endif

        message = "";
found1: ;
    }
}

/* prints out our fine configuration */
int
print_configuration()
{
    struct language     *l;
    struct access       *a;
    struct browser_opts *b;
    char                *yes = "yes", *no = "no";

fprintf(stderr, "Settings:\n\
    own port: %d\tldapd host: %s\tport: %d\n\
      etcdir: %s\tconfig file: %s\n\
derefaliases: %3s            syslog: %3s\n\
showonematch: %3s         ufnsearch: %3s\n\
    gwswitch: %3s        rootishome: %3s\n\
   maxvalues: %3d         timelimit: %d s\n\
lastmodified: %3s           expires: %d\n\
   subsearch: %s\n\
      robots: %s\n\n",
            port, ldaphost, ldapport, etcdir, config_file,
            derefaliases ? yes : no, dosyslog ? yes : no,
            showonematch ? yes : no, ufnsearch ? yes : no,
            gwswitch ? yes : no, rootishome ? yes : no,
            maxvalues, timelimit, lastmodified ? yes : no,
            expires, search_subtree, robots);

    for (l = languages; l && l->l_pattern; l = l->l_next) {
        fprintf(stderr, "\
----- LANGUAGE: %s%s\n\
      helpfile: %s\n\
attrs helpfile: %s\n\
  friendlyfile: %s\n\
   messagefile: %s\n", 
            l->l_content_lang, l == default_language ? " - DEFAULT" : "",
            l->l_conf->c_helpfile, l->l_conf->c_attrfile, 
            l->l_conf->c_friendlyfile, l->l_conf->c_messagefile);
    }
    fprintf(stderr, "\n");
    for (a = accesses; a && a->a_domainpattern; a = a->a_next) {
        fprintf(stderr, "\
--- ACCESS: %s\n\
   pattern: %s\n\
    rights: 0x%x %s\n\
 sizelimit: %d\n\
  language: %s\n\
  start_dn: %s\n\
   bind_dn: %s\n",
        a->a_desc, a->a_domainpattern, a->a_rights,
        access_right2string(a->a_rights), a->a_sizelimit, a->a_language,
        a->a_start_dn, a->a_bind_dn);
    }
    fprintf(stderr, "\n");
    for (b = browser_opts; b && b->b_pattern; b = b->b_next) {
        fprintf(stderr, "\
------ Browser: %s\n\
       pattern: %s\n\
       options: 0x%x %s\n\
         flags: 0x%x %s\n\
      upsearch: 0x%x %s\n",
            b->b_desc, b->b_pattern, 
            b->b_opts, opts2string(b->b_opts, browser_options),
            b->b_flags, flag2string(b->b_flags),
            b->b_upsearch, opts2string(b->b_upsearch, upsearch_options));
    }
}


/* initialize unconfigured values with default values */

int
initialize(
    char    *language
)
{
    char                *hf, *cp, *next, *search_subtree_clean;
    int                 err, i;
    struct conf_files   *cf;
    struct language     *l;
    struct access       *a;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_CONFIG,
            "initialize(%s)\n", language ? language : "", 0, 0, 0);
#endif

    /* Handle all the files (language dependant) */

    if (languages == (struct language *)NULL) {
        fprintf(stderr, "Warning: No language definitions!\n");
    }
    for (l = languages; l && l->l_pattern; l = l->l_next) {

#ifdef WEB500GW_DEBUG
        Web500gw_debug(WEB500GW_DEBUG_CONFIG, "\n ------ LANGUAGE: %s\n",
            l->l_content_lang, 0, 0, 0);
#endif
        if ((language && strcmp(language, l->l_content_lang) == 0)
            || (!default_language && l->l_next == NULL)) {
            default_language = l;
        }

        /* Get a structure */
        cf = (struct conf_files *)calloc(1, sizeof(struct conf_files));
        l->l_conf = cf;

        /* helpfile */
        hf = abs_file_name(helpfile, etcdir, l->l_suffix);
        if (access(hf, R_OK) != 0) {
            fprintf(stderr, "Unable to access helpfile %s:\n\t", hf);
            perror("");
            exit(1);
        }
        cf->c_helpfile = strdup(hf);

#ifdef MODIFY
        /* helpfile for attributes */
        hf = abs_file_name(attrfile, etcdir, l->l_suffix);
        if (access(hf, R_OK) != 0) {
            fprintf(stderr, "Unable to access attrfile %s:\n\t", hf);
            perror("");
            exit(1);
        }
        cf->c_attrfile = strdup(hf);
#endif
        /* friendly file (country names) */
        if (friendlyfile) {
            hf = abs_file_name(friendlyfile, etcdir, l->l_suffix);
            if (access(hf, R_OK) != 0) {
                fprintf(stderr, "Unable to access friendlyfile %s:\n\t", hf);
                perror("");
                exit(1);
            }
            cf->c_friendlyfile = strdup(hf);
        }

        /* message file for output */
        cf->c_messagefile = strdup(abs_file_name(messagefile, etcdir, l->l_suffix));
        read_messages (cf->c_messagefile, l);

#ifdef WEB500GW_DEBUG
        Web500gw_debug(WEB500GW_DEBUG_CONFIG, "\
        helpfile: %s\n\
  attrs helpfile: %s\n\
    friendlyfile: %s\n\
     messagefile: %s\n",
        cf->c_helpfile, cf->c_attrfile, cf->c_friendlyfile, cf->c_messagefile);
#endif
    }

    if (default_language == (struct language *)NULL) {
        fprintf(stderr, 
            "Couldn't find a default language - no language definitions in configfile?\n");
        return(1);
    }

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_CONFIG, "\nDefault language: %s\n",
        default_language->l_content_lang, 0, 0, 0);
#endif

    /* ACCESS */
    if (accesses == (struct access *)NULL) {
        fprintf(stderr, "Warning: No ACCESS definitions!\n");
    }
    for (a = accesses; a && a->a_domainpattern; a = a->a_next) {

#ifdef WEB500GW_DEBUG
        Web500gw_debug(WEB500GW_DEBUG_CONFIG, "\n\
--- ACCESS: %s\n\
   pattern: %s\n\
    rights: 0x%x %s\n",
            a->a_desc, a->a_domainpattern, a->a_rights, 
            access_right2string(a->a_rights));
#endif

        if (a->a_language && *(a->a_language) &&
            (l = find_language(a->a_language))) {
            a->a_lang = l;
        } else {
            a->a_language = default_language->l_content_lang;
            a->a_lang = default_language;
        }

        /* default values if empty after reading the config file */

        if (a->a_sizelimit == 0 && sizelimit != 0)
            a->a_sizelimit = sizelimit;
        if (!(a->a_start_dn && *a->a_start_dn))  {
            a->a_start_dn = home_dn;
        } else if (*a->a_start_dn == '/' && *(a->a_start_dn + 1) == '\0') {
            /* / == Root */
            a->a_start_dn = "";
        }
        if (!(a->a_bind_dn && *a->a_bind_dn)) 
            a->a_bind_dn = web500dn;
        if (!(a->a_bind_pw && *a->a_bind_pw)) 
            a->a_bind_pw = web500pw;

#ifdef WEB500GW_DEBUG
        Web500gw_debug(WEB500GW_DEBUG_CONFIG, "\
  language: %s\n\
 sizelimit: %d\n",
            a->a_language, a->a_sizelimit, 0, 0);
        Web500gw_debug(WEB500GW_DEBUG_CONFIG, "\
  start_dn: %s\n\
   bind_dn: %s\n", 
            a->a_start_dn, a->a_bind_dn);
#endif
    }

    /* Browsers */
    if (browser_opts == (struct browser_opts *)NULL) {
        fprintf(stderr, "Warning: No Browser definitions!\n");
    }
    if (search_subtree != NULL) {   /* parse search_subtree objectclass list */
        cp = search_subtree;
        i = 1;
        while ((cp = strchr(cp, ',')) != NULL) {
            i++;     /* just count */
            cp++;
        }
        search_subtree_oc = (char **)calloc(i + 1, sizeof(char *));
        search_subtree_clean = (char *)calloc(strlen(search_subtree) + 1, sizeof(char));
        i = 0; 
        next = cp = search_subtree;
        while ((next = strchr(cp, ',')) != NULL) {
            *next++ = '\0';
            search_subtree_oc[i++] = cp;
            strcat(search_subtree_clean, cp);
            strcat(search_subtree_clean, ", ");
            cp = next;
            while (cp && isspace(*cp)) cp++;
        }
        search_subtree_oc[i++] = cp;
        search_subtree_oc[i] = '\0';
        strcat(search_subtree_clean, cp);
        search_subtree = search_subtree_clean;
    }

    return(0);
}

