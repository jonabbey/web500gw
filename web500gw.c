static char copyright[] = 
  "Copyright (c) 1994-1998 Chemnitz University of Technology, Germany";

/*
 * web500gw.c, Version 2.1: 
 *
 *               A WWW - LDAP (X.500) gateway
 *
 * written by: Frank Richter, Frank.Richter@hrz.tu-chemnitz.de
 *             Chemnitz University of Technology, Germany, 1994-1998
 *
 * Copyright (c) 1994-1998 Chemnitz University of Technology.
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
 * $Id: web500gw.c,v 1.2 2001/04/26 22:16:53 dekarl Exp $
 */

/* For changes see file CHANGES */

#include "web500gw.h"

static set_socket();
static SIG_FN wait4child();
static SIG_FN sig_timeout();
static do_queries();
static do_help();
static got_request = 0;

static void
usage(
    char    *name
)
{
    fprintf(stderr, "\nUsage: %s\n\
   [-I] [-p port] [-P ldapport] [-x ldaphost] [-l] [-a] [-s default_language]\n\
   [-e etcdir] [-c configfile] [-t urlpath] [-d debuglevel] [-D LDAP debug] [-v]\n",
    name);
    fprintf(stderr, "Debug levels:\n\
\t 1   short: client's requests + results\n\
\t 2   trace: major function calls\n\
\t 4   long: HTTP requests from client\n\
\t 8   parsing/conversion of requests/results\n\
\t16   search/filter handling\n\
\t32   handling of config files\n\
\t64   utility function calls\n");
    exit(NOTOK);
}


/* 
 * main: process command line flags, initialize files/port, listen and
 * fork for each request
 */

int
main (
    int    argc,
    char    **argv
)
{
    int            s, ns, rc, i, pid;
    int            derefaliases_flag = 0, dosyslog_flag = 0, print_version = 0;
    int            port_flag = 0, ldapport_flag = 0;
    char           *myname, *remote;
    char           *language_flag = NULL, *ldaphost_flag = NULL;
    char           *etcdir_flag = NULL;
    fd_set         readfds;
    struct hostent        *hp;
    struct sockaddr_in    from;
    socklen_t            fromlen;
    extern char    *optarg;
    extern char    **Argv;
    extern int     Argc;
    int            debug = 0;

    time(&statistics.stat_start_time);

    /* for setproctitle */
    Argv = argv;
    Argc = argc;

    if (getenv("GATEWAY_INTERFACE") != NULL) {
        cgimode = 1;
        if (getenv("WEB500GW_DEBUG") != NULL)
            web500gw_debug = atoi(getenv("WEB500GW_DEBUG"));
    } else {

        while ((i = getopt(argc, argv, "vP:ac:d:D:e:hlp:s:x:It:")) != EOF) {
            switch(i) {
            case 'a':   /* search aliases */
                derefaliases_flag = 1;
                break;
            case 'c':   /* configuration file */
                config_file = strdup(optarg);
                break;
            case 'd':   /* web500gw debugging level */
#ifdef WEB500GW_DEBUG
                web500gw_debug = atoi(optarg);
                debug += web500gw_debug;
#else
                fprintf(stderr, "Warning: Web500gw debugging code was not compiled in (requires WEB500GW_DEBUG)\n");
#endif
                break;
            case 'D':   /* LDAP debugging level */
#ifdef LDAP_DEBUG
                ldap_debug = atoi(optarg);
                debug += ldap_debug;
#else
                fprintf(stderr, "Warning: LDAP debugging code was not compiled in (requires LDAP_DEBUG)\n");
#endif
                break;
            case 'e':
                etcdir_flag = strdup (optarg);
                break;
    
            case 'l':   /* log to LOG_LOCAL3 */
                dosyslog_flag = 1;
                break;
    
            case 'p':   /* port to listen on */
                port_flag = atoi(optarg);
                break;
    
            case 'P':   /* port to connect to ldap server */
                ldapport_flag = atoi(optarg);
                break;
    
    	    case 's':   /* default language */
                language_flag = strdup (optarg);
                break;
	    	
            case 'x':   /* ldap server hostname */
                ldaphost_flag = strdup(optarg);
                break;
    
            case 'I':   /* run from inetd */
                inetd = 1;
                break;
    
            case 't':   /* test mode - generate HTML output only */
                testmode = 1;
                testurl  = strdup(optarg);
                break;
    
            case 'v':   /* print version information */
            case 'h':   /* print help information */
                print_version = 1;
                break;
    
            default:
                usage(argv[0]);
            }
        }
    }

    if (read_configuration(abs_file_name(config_file, 
        etcdir_flag ? etcdir_flag : etcdir, "")) != 0) {
        usage(argv[0]);
    }
    /* command line flags override config file values */
    if (derefaliases_flag)  derefaliases = derefaliases_flag;
    if (etcdir_flag)        etcdir = etcdir_flag;
    if (dosyslog_flag)      dosyslog = dosyslog_flag;
    if (port_flag)          port = port_flag;
    if (ldapport_flag)      ldapport = ldapport_flag;
    if (ldaphost_flag)      ldaphost = ldaphost_flag;
    
    if (initialize(language_flag) != 0) {
        fprintf(stderr, "Aborting - error in initialization!\n");
    }

    if (print_version) {
        fprintf(stderr, "\nweb500gw - A WWW/HTTP - X.500/LDAP Gateway\n\
\t%s\n\t%s\n", copyright, Version);
        print_configuration();
        usage(argv[0]);
    }

#ifdef USE_SYSCONF
    dtblsize = sysconf(_SC_OPEN_MAX);
#else
    dtblsize = getdtablesize();
#endif

    /* detach if stderr is redirected or no debugging */
    if (inetd == 0 && testmode == 0 && cgimode == 0)
        (void) detach(debug);

    if ((myname = strrchr(argv[0], '/')) == NULL)
        myname = strdup(argv[0]);
    else
        myname = strdup(myname + 1);

    if (dosyslog)
#if defined (__NetBSD__)
        openlog(myname, OPENLOG_OPTIONS, LOG_LOCAL3);
#else
#ifdef LOG_LOCAL3
        openlog(myname, OPENLOG_OPTIONS, LOG_LOCAL3);
#else
        openlog(myname, OPENLOG_OPTIONS);
#endif    
#endif

    /* set up the socket to listen on */
    if (inetd == 0 && testmode == 0 && cgimode == 0) {
        s = set_socket(port);
        /* arrange to reap children */
        (void) signal(SIGCHLD, wait4child);
    }

    if (inetd) {
        hp = NULL;
        fromlen = sizeof(from);
        if (getpeername(0, (struct sockaddr *) &from, &fromlen) == 0) {
            hp = gethostbyaddr((char *) &(from.sin_addr.s_addr),
                sizeof(from.sin_addr.s_addr), AF_INET);
#ifndef NOSETPROCTITLE
            setproctitle( hp == NULL ? inet_ntoa(from.sin_addr) :
                hp->h_name, port);
#endif
            do_queries(0, (hp == NULL ? inet_ntoa(from.sin_addr) : hp->h_name));

#ifdef WEB500GW_DEBUG
        /* if getpeername fails, try "localhost" in debug mode only */
        Web500gw_debug(WEB500GW_DEBUG_CONFIG, "      home_dn ok: %s\n", 
            home_dn, 0, 0, 0);
        } else {
            if (web500gw_debug) {
                do_queries(0, "localhost");
            }
#endif
        }
        close(0);
        exit(0);
    }
    if (cgimode) {
        if ((remote = getenv("REMOTE_HOST")) != NULL ||
            (remote = getenv("REMOTE_ADDR")) != NULL) {
#ifndef NOSETPROCTITLE
            setproctitle(remote, port);
#endif
            do_queries(0, remote);
        }
        close(0);
        exit(0);
    }
    if (testmode) {
        do_queries(0, "localhost");
        exit(0);
    }

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_ANY, "web500gw: initialization complete, waiting for connection on port %d\n", 
        port, 0, 0, 0);
#endif
    if (dosyslog) {
        syslog(LOG_INFO, "web500gw: initialization complete, waiting for connection on port %d", port);
    }

    for (;;) {
        FD_ZERO(&readfds);
        FD_SET(s, &readfds);
        if ((rc = select(dtblsize, &readfds, 0, 0, 0)) == -1) {
#ifdef WEB500GW_DEBUG
            if (web500gw_debug) perror("select parent");
#endif
            continue;
        } else if (rc == 0) {
            continue;
        }
        if (! FD_ISSET(s, &readfds))
            continue;

        fromlen = sizeof(from);
        if ((ns = accept(s, (struct sockaddr *) &from, &fromlen)) == -1) {
#ifdef WEB500GW_DEBUG
            if (web500gw_debug) perror("accept");
#endif
            continue;
        }

        hp = gethostbyaddr((char *) &(from.sin_addr.s_addr),
            sizeof(from.sin_addr.s_addr), AF_INET);
#ifdef WEB500GW_DEBUG
        Web500gw_debug(WEB500GW_DEBUG_CONNS, "\n--- Connection from %s (%s)\n",
                (hp == NULL) ? "unknown" : hp->h_name,
                inet_ntoa(from.sin_addr), 0, 0);
#endif

	// since we fork to handle each request, we don't have to
	// worry about memory leaks in our processing, yay!

        switch(pid = fork()) {
        case 0:        /* child */
            close(s);
#ifndef NOSETPROCTITLE
            setproctitle(hp == NULL ? inet_ntoa(from.sin_addr) :
                          hp->h_name, port);
#endif
            /* (void) SIGNAL( SIGPIPE, (void *) log_and_exit ); */
            signal(SIGALRM, sig_timeout);
            alarm(4 * timelimit);
            do_queries(ns, (hp == NULL ? inet_ntoa(from.sin_addr) : hp->h_name));
            break;

        case -1:    /* failed */
            perror("fork");
            break;

        default:    /* parent */
            close(ns);
            statistics.stat_active_conns++;
            statistics.stat_total_conns++;
#ifdef WEB500GW_DEBUG
            Web500gw_debug(WEB500GW_DEBUG_TRACE, "forked child %d\n",
                pid, 0, 0, 0);
#endif
            break;
        }
    }
    /* NOT REACHED */
}


static int
set_socket(int port)
{
    int                 s, one;
    struct sockaddr_in  addr;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "set_socket (%d)\n", port, 0, 0, 0);
#endif

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    /* set option so clients can't keep us from coming back up */
    one = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &one,
        sizeof(one)) < 0) {
        perror("setsockopt");
        exit(1);
    }

    /* bind to a name */
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(s, (struct sockaddr *) &addr, sizeof(addr))) {
        perror("bind");
        exit(1);
    }

    /* listen for connections */
    if (listen(s, 5) == -1) {
        perror("listen");
        exit(1);
    }
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_ANY, "web500gw listening on port %d\n", 
        port, 0, 0, 0);
#endif
    return (s);
}

static SIG_FN
wait4child()
{
    WAITSTATUSTYPE     status;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, 
        "wait4child parent: catching child status\n\n", 0, 0, 0, 0);
#endif

#ifdef USE_WAITPID
        while (waitpid ((pid_t) -1, 0, WAIT_FLAGS) > 0)
#else /* USE_WAITPID */
        while (wait3(&status, WNOHANG | WUNTRACED, 0) > 0)
#endif /* USE_WAITPID */
            ;       /* NULL */
    statistics.stat_active_conns--;
    (void) signal(SIGCHLD, wait4child);
}

/*
 * Alarm handler: Maximum time is over
 */
static SIG_FN
sig_timeout()
{
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, 
        "sig_timeout: alarm - got timeout\n\n", 0, 0, 0, 0);
#endif
    statistics.stat_active_conns--;
/*
    if (got_request) {
        do_error(r, resp, LDAP_TIMEOUT, SERVER_TIMEOUT, MSG_TIMELIMIT, NULL);
    } else {
        resp->resp_status = CLIENT_TIMEOUT;
    }    
    if (dosyslog || web500gw_debug & WEB500GW_DEBUG_CONNS) {
        log_request(r, resp);
    }
 */
    exit(0);
}


/* 
 *  Process the client request
 */
static int
do_queries(
    int             s,
    char            *host
)
{
    REQUEST  request;
    REQUEST  *r = &request;
    RESPONSE response;
    RESPONSE *resp = &response;

    char        buf[4096];
    char        *query, *tail, *cp, *query_string, *path_info;
    char        *attrs, *flags = NULL, *f, *tmpl;
    int         len, i, rc;
    struct timeval  timeout;
    fd_set      readfds;
    struct language     *lang;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "do_queries (%d, %s %s)\n",
        s, host, cgimode ? "cgimode" : testmode ? "testmode" : "", 0);
#endif

    bzero(&request, sizeof(REQUEST));
    bzero(&response, sizeof(RESPONSE));

    timeout.tv_sec = timelimit;
    timeout.tv_usec = 0;

    if (testmode || cgimode) {
        fp = stdout;
    } else {
#if defined(unixware7)
        if ((fp = fdopen(s, "r+")) == NULL) {
#else
        if ((fp = fdopen(s, "a+")) == NULL) {
#endif
            perror("fdopen");
            exit(1);
        }

        FD_ZERO(&readfds);
        FD_SET(fileno(fp), &readfds);
        if ((rc = select(dtblsize, &readfds, 0, 0, &timeout)) <= 0) {
            perror("select child");
            exit(1);
        }
    }

    /* Access control */
    r->r_client_host = strdup(host);
    r->r_access = find_access(r->r_client_host);

    /* init request and response structures */
    r->r_tm = time(0);          /* current time - for HTTP-Date: */
    r->r_dn = r->r_if_modified_since =  r->r_referer = r->r_attrs[0] = 
      // XARL    r->r_template = r->r_language = r->r_filter = r->r_postdata = 
      // XARL    r->r_useragent = r->r_authorization = NULL;

    r->r_language = r->r_filter = r->r_postdata = 
    r->r_useragent = r->r_authorization = NULL;
    r->r_browser = browser_opts;
    r->r_httpversion = r->r_content_length = r->r_attrnumb =  r->r_flags = 0;
    r->r_method = UNKNOWN;
    r->r_ld = (LDAP *)0;
    r->r_ldaphost = ldaphost;
    r->r_ldapport = ldapport;
    if (derefaliases)
        r->r_flags |= FLAG_DEREFALIAS;
    
    strftime(resp->resp_date, 256, DATE_FORMAT, gmtime(&(r->r_tm)));
    resp->resp_httpheader = resp->resp_htmlheader = 0;
    resp->resp_status = DOCUMENT_FOLLOWS;  /* default status is OK :*/
    resp->resp_error = LDAP_SUCCESS;
    resp->resp_content_type = CT_HTML;
    resp->resp_content_length = 0;
    resp->resp_last_modified = resp->resp_location = resp->resp_extra = NULL;
    resp->resp_language = default_language;
    resp->resp_expires = expires;


    if (testmode) {
        r->r_request = testurl;
        query = strdup(testurl);
        r->r_method = GET;
    } else if (cgimode) {
        /* read the request from environment */
        if ((cp = getenv("SERVER_PROTOCOL")) == NULL)
            return NOTOK;
        if (strncasecmp(cp, "HTTP/1", 6) == 0)
            r->r_httpversion = 1;
        if ((query_string = getenv("QUERY_STRING")) == NULL)
            return NOTOK;
        if ((path_info = getenv("PATH_INFO")) == NULL)
            return NOTOK;
        /* PATH_INFO is decoded by the Web server, QUERY_STRING isn't */
        path_info = url_encode(path_info);
        query = calloc(strlen(path_info) + strlen(query_string) + 1, sizeof(char));
        strcpy(query, path_info);
        if (strlen(query_string)) {
            strcat(query, "?");
            strcat(query, query_string);
        }

        if ((cp = getenv("REQUEST_METHOD")) == NULL)
            return NOTOK;

        r->r_request = calloc(strlen(cp) + strlen(query) + 
                        strlen(getenv("SERVER_PROTOCOL")) + 6, sizeof(char));
        sprintf(r->r_request, "%s %s %s", cp, query,
                getenv("SERVER_PROTOCOL"));

        if (strcasecmp(cp, "GET") == 0) {
            r->r_method = GET;
        } else if (strcasecmp(cp, "POST") == 0) {
            r->r_method = POST;
        } else if (strcasecmp(cp, "HEAD") == 0) {
            r->r_method = HEAD;
        } else {
            do_error(r, resp, LDAP_OTHER, NOT_IMPLEMENTED, NULL, NULL);
            goto end;
        }
    } else {
        /* read the request (GET, POST, HEAD) from the socket */
        if (fgets(buf, sizeof(buf), fp) == NULL)
            exit(1);

        len = strlen(buf);
        /* strip of \r \n */
        if (buf[len - 1] == '\n') buf[--len] = '\0';
        if (buf[len - 1] == '\r') buf[--len] = '\0';

        r->r_request = strdup(buf);
        query = strdup(buf);            /* for parsing */

#ifdef WEB500GW_DEBUG
        Web500gw_debug(WEB500GW_DEBUG_PACKETS, "\n\"%s\"\n", query, 0, 0, 0);
#endif

        /* Determine HTTP method */
        if (tolower(*query) == 'g' && tolower(*(query + 1)) == 'e' && 
            tolower(*(query + 2)) == 't') {    /* HTTP GET request */
            r->r_method = GET;
            query += 3;
        } else if (tolower(*query) == 'h' && tolower(*(query + 1)) == 'e' && 
                   tolower(*(query + 2)) == 'a' && tolower(*(query + 3)) == 'd') {
            /* HTTP HEAD request */
            r->r_method = HEAD;
            query += 4;
        } else if (tolower(*query) == 'p' && tolower(*(query + 1)) == 'o' &&
                   tolower(*(query + 2)) == 's' && tolower(*(query + 3)) == 't') {
            /* HTTP POST request */
            r->r_method = POST;
            query += 4;
        } else {    /* unknown request */
            do_error(r, resp, LDAP_OTHER, NOT_IMPLEMENTED, NULL, NULL);
            goto end;
        }
        /* strip off leading white space */
        while (isspace(*query)) ++query;
        if ((tail = strstr(query, " HTTP")) != NULL || 
            (tail = strstr(query, " http")) != NULL) { /* a HTTP/1.x request */
            r->r_httpversion = 1;
            *tail = '\0';
        }
    }

    r->r_query = strdup(query);       /* save clients query */

    if (cgimode)
        url_decode(query);
    /* strip off leading white space and '/' */
    while (isspace(*query) || *query == '/') ++query;
    if ((cp = strchr(query, '#'))) { /* inline anchor handled by browser */
        *cp = '\0';
    }
    len = strlen(query);
    if (*query) {
        if (*query == '?' && *(query + 1) == '=') {
            /* from "Move upwards" FORM:    ?=urlencoded_DN */
            query += 2;                     /* skip ?= */
            while(isspace(*query)) ++query;
            url_decode(query);
        }
        if (*query == '/') ++query;
        if ((cp = strchr(query, '/')) != NULL) {
            /* ldap server given! */
            *cp++ = '\0';
            if (*query && allow_other_servers) {
                r->r_ldaphost = query;
                if ((tail = strchr(query, ':')) != NULL) {  /* port */
                    *tail++ = '\0';
                    if ((i = atoi(tail)) > 0)
                        r->r_ldapport = i;
                }
            }
            query = cp;

#ifdef WEB500GW_DEBUG
        Web500gw_debug(WEB500GW_DEBUG_PARSE, "LDAP host: %s:%d\n",
            r->r_ldaphost, r->r_ldapport, 0, 0);
#endif
        }
    }

    if (!cgimode)
        url_decode(query);
    /* strip off leading white space and '/' */
    while (isspace(*query) || *query == '/') ++query;
    if ((cp = strchr(query, '#'))) { /* inline anchor handled by browser */
        *cp = '\0';
    }

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_CONNS, "--> %s.\n", query, 0, 0, 0);
#endif

    /* Access control: deny if no rights */
    if (r->r_access == (struct access *)NULL || 
        r->r_access->a_rights == ACCESS_NOTHING) {
        /* No access control found or no access at all -> deny access */
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, 
        "do_queries: Access denied for %s: %s\n", r->r_client_host,
        r->r_access ? "access nothing" : "No ACCESS found!", 0, 0);
#endif
#if __hpux | __osf__
        fflush(fp);
#else
        rewind(fp);
#endif
        resp->resp_status = FORBIDDEN;
        resp->resp_error = LDAP_OTHER;
        do_error(r, resp, LDAP_INSUFFICIENT_ACCESS, resp->resp_status, 
            MSG_ACCESS_NOTHING, NULL);

        goto end;
    }

    resp->resp_language =  r->r_access->a_lang;  /* default language for client */

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE,
        "do_queries: ACCESS for %s: %s (%s - %s)\n", r->r_client_host, 
            r->r_access->a_desc, access_right2string(r->r_access->a_rights), 
            r->r_access->a_language);
#endif

    if (r->r_httpversion == 1) {
        if (cgimode) {
            if ((cp = getenv("HTTP_USER_AGENT")) != NULL) {
                r->r_browser = find_browser(cp);
                r->r_useragent = strdup(buf+12);
            }
            if ((cp = getenv("CONTENT_LENGTH")) != NULL) {
                r->r_content_length = atoi(cp);
            }
            if ((cp = getenv("HTTP_ACCEPT_LANGUAGE")) != NULL) {
                if ((lang = find_language(cp)))
                    resp->resp_language = lang;
            }
            if ((cp = getenv("HTTP_REFERER")) != NULL) {
                r->r_referer = cp;
            }
        } else {
        /* process the following lines from client until empty line */
        /* for now most of them are ignored */
            while (fgets(buf, sizeof(buf), fp) != NULL && strlen(buf) > 2) {
                len = strlen(buf);
                /* strip of \r \n */
                if (buf[len - 1] == '\n') buf[--len] = '\0';
                if (buf[len - 1] == '\r') buf[--len] = '\0';

#ifdef WEB500GW_DEBUG
                Web500gw_debug(WEB500GW_DEBUG_PACKETS, "%s\n", buf, 0, 0, 0);
#endif
                if (strncasecmp(buf, "User-Agent: ", 12) == 0) {
                    /* the web browser name */
                    r->r_browser = find_browser(buf + 12);
                    r->r_useragent = strdup(buf+12);
#ifdef WEB500GW_DEBUG
                    Web500gw_debug(WEB500GW_DEBUG_PARSE, " -> Browser: %s (Options 0x%x)\n",
                        r->r_browser ? r->r_browser->b_desc : "no browser", 
                        r->r_browser ? r->r_browser->b_opts : 0, 0, 0);
#endif
                } else if (strncasecmp(buf, "Content-length:", 15) == 0) {
                    /* Content-length of POST data */
                    r->r_content_length = atoi(buf + 15);
                } else if (strncasecmp(buf, "If-Modified-Since:", 18) == 0) {
                    /* send only if newer than given date */
                    if ((cp = strchr(buf, ';'))) {  /* strip next parameter */
                        *cp = '\0';
                    }
                    cp = buf + 18;
                    while (isspace(*cp)) ++cp;
                    r->r_if_modified_since = strdup(cp);
#ifdef WEB500GW_DEBUG
                    Web500gw_debug(WEB500GW_DEBUG_PARSE, " -> If-Modified-Since: >%s<\n",
                        r->r_if_modified_since, 0, 0, 0);
#endif
                } else if (strncasecmp(buf, "Accept-Language:", 16) == 0) {
                    /* Language negotiation, for now: take the first mentioned */
                    if ((cp = strchr(buf, ','))) {
                        *cp = '\0';
                    }
                    if ((cp = strchr(buf, ';'))) {  /* strip next parameter */
                        *cp = '\0';
                    }
                    cp = buf + 16;
                    while (isspace(*cp)) ++cp;
                    if ((lang = find_language(cp))) {
                        resp->resp_language = lang;
                    }
#ifdef WEB500GW_DEBUG
                    Web500gw_debug(WEB500GW_DEBUG_PARSE, " -> Accept-Language: %s -> %s\n",
                        cp, resp->resp_language->l_content_lang, 0, 0);
#endif
                } else if (strncasecmp(buf, "Referer:", 8) == 0) {
                    /* The Referer */
                    cp = buf + 8;
                    while (isspace(*cp)) ++cp;
                    r->r_referer = strdup(cp);
                } else if (strncasecmp(buf, "Authorization:", 14) == 0) {
                    /* Authorization */
                    cp = buf + 14;
                    while (isspace(*cp)) ++cp;
                    r->r_authorization = strdup(cp);
                }   /* else {ignore} */
            }
        }
    }

    if (r->r_method == POST) {
    /* HTTP POST: read r->r_content_length chars from input */
#ifdef WEB500GW_DEBUG
        Web500gw_debug(WEB500GW_DEBUG_PARSE, "Content-length = %i\n",
            r->r_content_length, 0, 0, 0);
#endif
        if (r->r_content_length == 0) {  /* error! */
            do_error(r, resp, LDAP_OTHER, BAD_REQUEST, 
                "POST but not Content-Length!", NULL);
            goto end;
        }
        if ((r->r_postdata = malloc (r->r_content_length + 2)) == NULL) {
#ifdef WEB500GW_DEBUG
            if (web500gw_debug) perror("Getting POST data!");
#endif
            resp->resp_status = BAD_REQUEST;
            goto end;
        }
        cp = r->r_postdata;
        while (r->r_content_length-- > 0) {
            *cp++ = getc(cgimode ? stdin : fp);
        }
        *cp = '\0';
    } 
#if __hpux | __osf__
    fflush(fp);
#else
    rewind(fp);
#endif
    got_request = 1;

/* END of client header (+ POST data) processing */

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_PACKETS, "\n", 0, 0, 0, 0);
#endif


/* Now process the request */

    /* ROBOT handling */
    if (strcasecmp(query, "robots.txt") == 0) {
#ifdef WEB500GW_DEBUG
            Web500gw_debug(WEB500GW_DEBUG_PARSE, "ROBOTS.TXT URL: %s\n",
                            query, 0, 0, 0);
#endif
        resp->resp_content_type = CT_TEXT;
        if (r->r_httpversion == 1) {
            http_header(r, resp);
        }
        fprintf(fp, "%s\r\n", robots);
        goto end;
    }

#ifdef SUPPORT_OLD_URLS
    if (*query == 'R' || *query == 'L' || *query == 'S' ||
        *query == 'I' || *query == 'G' || *query == 'A' || *query == 'F' ||
        (*query == 'M' && *(query + 1) != '\0' && *(query + 1) != '?')) {
        
        /* OLD go500gw like URL's */

#ifdef WEB500GW_DEBUG
        Web500gw_debug(WEB500GW_DEBUG_PARSE, "OLD style URL: %s\n",
                        query, 0, 0, 0);
#endif
        r->r_dn = strdup(query + 1);
        find_other_languages(r, resp);

        if (*query == 'Y') {  /* modify request, need user bind  */
            do_modify(r, resp);
            goto end;
        }

        if (*query == 'F') {    /* Make form for modify, need user bind  */
            do_form(r, resp);
            goto end;
        }

        switch (*query) {
        case 'R':    /* Read an entry */
            do_read(r, resp);
            break;
        case 'L':    /* read aLl attributes of an entry */
            do_read(r, resp);
            break;
        case 'S':    /* search */
            /* query string: base-DN?[OS]=filter
             *     search onelevel <--||--> search subtree
             */
            if ((r->r_filter = strchr(query, '?')) == NULL) {
                do_error(r, resp, LDAP_OTHER, BAD_REQUEST, MSG_MISSING_FILTER, NULL);
                goto end;
            }
            *(r->r_filter)++ = '\0';
            r->r_dn = strdup(query + 1);
            do_search(r, resp);
            break;
        case 'M':    /* X.500 Menu */
            do_read(r, resp);
            break;
        case 'I':    /* jpegPhoto of entry as jpeg or gif image */
            r->r_attrs[0] = "jpegPhoto";
            r->r_attrs[1] = NULL;
            do_special(r, resp);
            break;
        case 'G':    /* G3fax photo of entry as xbm */
            r->r_attrs[0] = "photo";
            r->r_attrs[1] = NULL;
            do_special(r, resp);
            break;
        case 'A':    /* Audio of entry */
            r->r_attrs[0] = "audio";
            r->r_attrs[1] = NULL;
            do_special(r, resp);
            break;
        }
        goto end;
    }
#endif /* SUPPORT_OLD_URLS */

    /* NEW URL:  DN[?[attrs][$flags][?{OS}=filter]] */

    r->r_attrs[0] = NULL;        /* default: read all attributes */
    r->r_attrnumb = 0;
    r->r_flags = 0;
    r->r_actions = 0;
    r->r_filter = NULL;          /* No search */

    if (*query == '?' && *(query + 1) == '=') { 
        /* from "Move upwards" FORM:  /?=urlencoded_DN */
        query += 2;           /* skip ?= */
        while(isspace(*query)) ++(query);
        url_decode(query);
    } 
    if ((*query == '\0' || *query == '?') && rootishome) {
        /* no DN, but maybe flags */
        r->r_dn = malloc(strlen(query) + strlen(r->r_access->a_start_dn));
        strcpy(r->r_dn, r->r_access->a_start_dn);
        if (*(query) == '?') {
            strcat(r->r_dn, query);
        }
#ifdef WEB500GW_DEBUG
        Web500gw_debug(WEB500GW_DEBUG_PARSE, "ROOT -> HOME: %s -> %s\n",
                        query, r->r_dn, 0, 0);
#endif
    } else if (*query == 'M' && (*(query + 1) == '\0' || *(query + 1) == '?')) {
        /* "Home page", maybe with flags  */
        r->r_dn = malloc(strlen(query) + strlen(r->r_access->a_start_dn));
        strcpy(r->r_dn, r->r_access->a_start_dn);
        if (*(query + 1) == '?') {
            strcat(r->r_dn, query + 1);
        }
#ifdef WEB500GW_DEBUG
            Web500gw_debug(WEB500GW_DEBUG_PARSE, "HOME URL: %s -> %s\n",
                            query, r->r_dn, 0, 0);
#endif
        /* free(query); */
    } else if (strncmp(query, "root", 4) == 0) {
        r->r_dn = query + 4;
    } else {
        r->r_dn = query;
    }

    i = 0;
    if ((attrs = strchr(r->r_dn, '?')) != NULL) {
        /* user specified attributes/flags/filter */                
        *attrs++ = '\0';
        if (*attrs != '\0') {    /* real data ? */
            if ((flags = strchr(attrs, '$')) != NULL) { /* flags */
                *flags++ = '\0';
            }
            if ((r->r_filter = strchr(flags ? flags : attrs, '?')) != NULL) {  
                /* filter */
                *(r->r_filter)++ = '\0';
                if (r->r_filter[strlen(r->r_filter) - 1] == '?') 
                    r->r_filter[strlen(r->r_filter) - 1] = '\0';
            }
            if (*attrs != '\0') {
                r->r_attrs[i++] = attrs;
                while ((attrs = strchr(attrs, ',')) != NULL && i < MAX_ATTRS) {
                    /* comma separated list of attributes */
                    *attrs++ = '\0';
                    r->r_attrs[i++] = attrs;
                }
                r->r_attrs[i] = NULL;
                r->r_attrnumb = i;
            }
#ifdef WEB500GW_DEBUG
            Web500gw_debug(WEB500GW_DEBUG_PARSE, "FLAGS: %s ", 
                flags ? flags : "", 0, 0, 0);
#endif
            while (flags && *flags != '\0') {   /* flags */
                if ((f = strchr(flags, ',')) != NULL)
                    *f++ = '\0';
                if ((tmpl = strchr(flags, '=')) != NULL) {
                /* flag=value -> template or language */
                    *tmpl++ = '\0';

		    // XARL                    if (strcasecmp(flags, "tmpl") == 0) {
		    // XARL                        r->r_template = tmpl;
		    // XARL                        r->r_flags |= FLAG_TMPL;
		    // XARL#ifdef WEB500GW_DEBUG
		    // XARL                        Web500gw_debug(WEB500GW_DEBUG_PARSE, "TEMPLATE: %s\n",
		    // XARL                            r->r_template, 0, 0, 0);
		    // XARL#endif
		    // XARL                    }  if (strcasecmp(flags, "lang") == 0) {

		    if (strcasecmp(flags, "lang") == 0) {
                        if ((lang = find_language(tmpl))) {
                            resp->resp_language = lang;
                            r->r_language = resp->resp_language->l_content_lang;
                            r->r_flags |= FLAG_LANGUAGE;
                        }

#ifdef WEB500GW_DEBUG
                        Web500gw_debug(WEB500GW_DEBUG_PARSE, "LANGUAGE: %s\n",
                            r->r_language, 0, 0, 0);
#endif
                    }   /* else {ignore}    */
                } else {
                    r->r_flags   |= string2flag(flags);
                    r->r_actions |= string2action(flags);
                }
                flags = f;
            }
            if (r->r_attrnumb > 0) {  /* we read only this attrs */
                r->r_flags |= FLAG_ATTRSONLY;
#ifdef WEB500GW_DEBUG
                Web500gw_debug(WEB500GW_DEBUG_PARSE, "+ attrsonly", 0, 0, 0, 0);
#endif
            }
#ifdef WEB500GW_DEBUG
            Web500gw_debug(WEB500GW_DEBUG_PARSE, "flags = 0x%x, actions = 0x%x\n", 
                r->r_flags, r->r_actions, 0, 0);
#endif
        }
    }

    // XARL    if (r->r_template) {
    // XARL        if (access_ok(r, resp, ACCESS_ATTRS) == NOTOK)
    // XARL            goto end;
    // XARL    }
    if (r->r_dn && *(r->r_dn) == 'H') {   /* HELP! */
        r->r_dn++;
        do_help(r, resp);
        goto end;
    }

    find_other_languages(r, resp);

    if (r->r_actions & ACTION_BIND) {       /* simple, no LDAP action */
        if (access_ok(r, resp, ACCESS_WRITE) == OK) {
            do_bind(r, resp);
        }
        goto end;
    }
    if (r->r_actions & ACTION_NBIND) {       /* simple, no LDAP action */
        /* This is very, very experimental */
        if (do_basic_auth(r, resp) == NOTOK) {
            goto end;
        }
    }

    if (r->r_actions & ACTION_SEARCH_FORM) {   /* Make a search form, no LDAP */
        do_search_form(r, resp);
        goto end;
    }

    if (r->r_actions & ACTION_MODIFY) { /* Modify request, need user bind */
        if (access_ok(r, resp, ACCESS_WRITE) == OK) {
            do_modify(r, resp);
        }
        goto end;
    }
    if (r->r_actions & ACTION_ADD) { /* Add request, need user bind */
        if (access_ok(r, resp, ACCESS_WRITE) == OK) {
            do_add(r, resp);
        }
        goto end;
    }
    if (r->r_actions & ACTION_DELETE) { /* Delete request, need user bind */
        if (access_ok(r, resp, ACCESS_WRITE) == OK) {
            do_delete(r, resp);
        }
        goto end;
    }
    if (r->r_actions & ACTION_MODRDN) { /* ModRDN request, need user bind */
        if (access_ok(r, resp, ACCESS_WRITE) == OK) {
            do_modrdn(r, resp);
        }
        goto end;
    }

    if (r->r_attrnumb == 1) {
        /* a single attribute */
        if (strncasecmp(r->r_attrs[0], "vcard", 5) == 0
            && !(r->r_filter)) {
            /* vcard, special handling, text/x-vcard */
            r->r_attrnumb = 0;
            r->r_attrs[0] = NULL;
            r->r_flags |= FLAG_VCARD;
            r->r_flags &= ~FLAG_ATTRSONLY;
        } else if (!(r->r_filter || flags)) {
        /* if image/audio attr -> send raw data */
            if (strncasecmp(r->r_attrs[0], "jpegPhoto", 9) == 0 ||
                strcasecmp(r->r_attrs[0], "photo") == 0 ||
                strcasecmp(r->r_attrs[0], "audio") == 0) {
                do_special(r, resp);
                goto end;
            }
        }
    }
    if (r->r_actions & ACTION_DO_SEARCH) {
        process_search_form(r, resp);
        goto end;
    }
    if (r->r_actions & ACTION_FORM) {   /* Make form for modify */
        if (access_ok(r, resp, ACCESS_WRITE) == OK) {
            do_form(r, resp);
        }
    } else if (r->r_actions & ACTION_ADDFORM) {   /* Make form for add */
        if (access_ok(r, resp, ACCESS_WRITE) == OK) {
            do_addform(r, resp);
        }
	// XARL    } else if (r->r_filter && r->r_flags !=  FLAG_VCARD && !(r->r_template)) {
    } else if (r->r_filter && r->r_flags !=  FLAG_VCARD) {
        do_search(r, resp);
    } else {
        do_read(r, resp);
    }

end:
    fflush(fp);
    if (dosyslog || web500gw_debug & WEB500GW_DEBUG_CONNS) {
        log_request(r, resp);
    }
    if (r->r_ld != (LDAP *)0)
        ldap_unbind(r->r_ld);
    exit(0);
    /* NOT REACHED */
}



static int
do_help(
    REQUEST  *r,
    RESPONSE *resp
)
{
    FILE    *op;
    char    line[BUFSIZ], *hf;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "do_help (%s)\n", r->r_dn, 0, 0, 0);
#endif

    if (r->r_httpversion == 1 && resp->resp_httpheader == 0) {
        http_header(r, resp);
        if (r->r_method == HEAD)
            return OK;
    }

    switch (*r->r_dn) {
    case 'A':
            hf = resp->resp_language->l_conf->c_attrfile;
            break;
    default:                /* general help file */
            hf = resp->resp_language->l_conf->c_helpfile;
    }
    if ((op = fopen(hf, "r")) == NULL) {
        fprintf(fp, "%s (%s)\n", MSG_NO_HELPFILE, hf);

#ifdef WEB500GW_DEBUG
        Web500gw_debug(WEB500GW_DEBUG_UTIL, "  %s (%s)\n", 
            MSG_NO_HELPFILE, hf, 0, 0);
#endif
        return NOTOK;
    }

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  Using helpfile: %s\n", hf, 0, 0, 0);
#endif

    while (fgets(line, sizeof(line), op)) {
        fputs(line, fp);
    }
    fclose(op);
    return OK;
}


int
do_error
(
    REQUEST     *r,
    RESPONSE    *resp,
    int         ldap_code,
    int         http_status,
    char        *extra_text,
    char        *matched
)
{
  return do_error_dispatch(r,resp,ldap_code,http_status, extra_text, matched, 0, 0);
}


/* This function processes an error that arises during the processing
   of an LDAP request. 

   extra_text and matched, if non-null, will be freed using the
   ldap_freemem() call.

*/

int
do_ldap_error
(
    REQUEST     *r,
    RESPONSE    *resp,
    int         ldap_code,
    int         http_status,
    char        *extra_text,
    char        *matched
)
{
  return do_error_dispatch(r,resp,ldap_code,http_status, extra_text, matched, 1, 1);
}

/* This function processes an error that arises during the processing
   of a request. 

   extra_text is a descriptive message string 
   matched is a pointer to the DN that the error was encountered on.
   free_extra_text: if true, extra_text will be freed with ldap_freemem().
   free_matched: if true, matched will be freed with ldap_freemem().

*/

int
do_error_dispatch(
    REQUEST     *r,
    RESPONSE    *resp,
    int         ldap_code,
    int         http_status,
    char        *extra_text,
    char        *matched,
    int         free_extra_text,
    int         free_matched
)
{
    char    *s, *matchmsg = NULL, *mdn, *expl;
    int     len;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "do_error (%d, %d, %.20s)\n", 
        ldap_code, http_status, extra_text ? extra_text : "", 0);
#endif
    
    if (http_status == 0) {
        switch (ldap_code) {
            case LDAP_STRONG_AUTH_REQUIRED:
                http_status = AUTH_REQUIRED;
                break;
            case LDAP_INAPPROPRIATE_AUTH:
            case LDAP_INVALID_CREDENTIALS:
                http_status = FORBIDDEN;
                break;
            case LDAP_NO_SUCH_OBJECT:
                http_status = NOT_FOUND;
                break;
            default:    
                http_status = SERVER_ERROR;
        }
    }
    switch (http_status) {
        case BAD_REQUEST:       s = MSG_BAD_REQUEST;    break;  
        case AUTH_REQUIRED:     s = MSG_AUTH_REQUIRED;  break;  
        case FORBIDDEN:         s = MSG_FORBIDDEN;      break;
        case NOT_FOUND:         s = MSG_NOT_FOUND;      break;
        case SERVER_ERROR:      s = MSG_SERVER_ERROR;   break;  
        case NOT_IMPLEMENTED:   s = MSG_NOT_IMPLEMENTED;    break;  
        default:                s = MSG_UNKNOWN_ERROR;
    }
    /* set global error values */
    resp->resp_status = http_status;
    resp->resp_error = ldap_code;
    if (r->r_httpversion == 1 && resp->resp_httpheader == 0) { 
        http_header(r, resp);
    }

    if (r->r_method == HEAD) {
      goto exit;
    }
    if (resp->resp_htmlheader == 0) {
        msg_fprintf(fp, MSG_HTML_START_ERROR, "s", MSG_ERROR);
        msg_fprintf(fp, MSG_ERRORBANNER, "s", r->r_query ? r->r_query : "");
    }
    if (matched && *matched) {
        mdn = dn2url(r, matched, FLAG_LANGUAGE, 0, NULL, NULL);
        len = strlen(matched) + strlen(mdn) + strlen(MSG_ERR_MATCHED) + 1;
        matchmsg = calloc(len, sizeof(char)); 
        msg_snprintf(matchmsg, len, MSG_ERR_MATCHED, "ss", mdn, matched);
    }
    if (ldap_code == LDAP_OTHER && (extra_text && *extra_text)) {   
        /* Unknown error - do we have an explanation */
        expl = extra_text;
        extra_text = NULL;
    } else {
        expl = web500gw_err2string(ldap_code, resp);
    }

    msg_fprintf(fp, MSG_ERROR_OCCURRED, "sisss",
        s, ldap_code, expl, matchmsg ? matchmsg : "",
        (extra_text && *extra_text) ? extra_text : " ");

    if (resp->resp_htmlheader == 0) {
        msg_fprintf(fp, MSG_ERRORTRAILER, "ss",
            r->r_query ? r->r_query : "", MSG_ADMIN);
        fputs(MSG_HTML_END, fp);
        fputs("\n", fp);
        resp->resp_htmlheader = 1;
    }
    if (matchmsg)
        free(matchmsg);

 exit:
    if (free_extra_text && extra_text)
      {
	ldap_freemem(extra_text);
      }

    if (free_matched && matched)
      {
	ldap_freemem(matched);
      }

    return OK;
}


int
do_monitor (
    REQUEST     *r,
    RESPONSE    *resp
)
{
    char    start_date[256];

    http_header(r, resp);
    if (r->r_method == HEAD) {
        return OK;
    }
    strftime(start_date, 256, DATE_FORMAT, gmtime(&statistics.stat_start_time));

    if (resp->resp_htmlheader == 0) {
        msg_fprintf(fp, MSG_HTML_START_MISC, "sss", MSG_MONITOR_TITLE, 
            monitor_dn, monitor_dn);
        resp->resp_htmlheader = 1;
    }

    msg_fprintf(fp, MSG_MONITOR, "ssssssssissii",
        version, Compiled, LDAP_Version, copyright, 
        MSG_ADMIN, url_encode(home_dn), friendly_dn(resp, home_dn), 
        ldaphost, ldapport, start_date, resp->resp_date, 
        statistics.stat_total_conns + 1, statistics.stat_active_conns + 1);
    return OK;
}

char *
gw_switch (
    LDAP        *ld, 
    RESPONSE    *resp,
    LDAPMessage *e
) 
{

/* GATEWAY SWITCHING:
 * read labeledURI, search for "(gw-language)"
 * if exists: check, if full URL or http://host:port only
 * construct server value
 */
    char **urls, *url = NULL, magic[8], *server = NULL;
    int  i;

    if ((urls = ldap_get_values(ld, e, "labeledURI")) == NULL) {
        return (NULL);
    }
    if (resp->resp_language->l_content_lang) {
        sprintf(magic, "(gw-%2.2s)", resp->resp_language->l_content_lang);
    }
    for (i = 0; urls != NULL && urls[i]; ++i) {
        if (strstr (urls[i], magic)) {
            /* found - with our language */
            if (strchr(urls[i], ' ') != NULL) {
                /* We must have a space, don't accept if there is none */
                url = urls[i];
                break;
            }
        }
        if (strstr (urls[i], "(gw)")) {
            /* found - but not with our language, look further */
            url = urls[i];
        }
    }
    if (url == NULL) {      /* nothing found */
        return (NULL);
    }

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "gw_switch: FOUND %s\n", url, 0, 0, 0);
#endif

    if ((server = strchr(url, ' ')) == NULL) {
        return (NULL);
    }
    if (strncmp(url, "http://", 7) == 0) {
        if (*(server - 1) != 'M') {
            if (*(server - 1) != '/') {
                *server++ = '/';
            }
            *server++ = 'M';    /* for now */
        }
    }
    *server = '\0';
    server = malloc(strlen(url) + 1);
    strcpy(server, url);

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE, "gw_switch >%s<\n", 
        server ?  server : "" , 0, 0, 0);
#endif

    return(server);
}

