/*
 * config.h: define default values
 *  could be overridden by configuration file and command line flags
 *
 * $Id: config.h,v 1.2 2001/04/26 22:16:53 dekarl Exp $
 */

/* The port web500gw is running, may be overridden with the '-p' flag */
#define WEB500PORT	8888

/* Standard configuration file, found below WEB500GWDIR */

#ifndef WEB500GW_CONFIG
#define WEB500GW_CONFIG "web500gw.conf"
#endif

/*
 *******************************************************************************
 * You probably want to tailor the following:
 *******************************************************************************
 */

/* This is the host that is running the ldap server. */
#ifndef LDAPHOST
#define LDAPHOST    "localhost"
#endif

/* This is the DN web500gw will (simple) bind to the directory as.
 * empty value "" means don't bind */
#define WEB500DN ""
/* ... the password for the entry above */
#define WEB500PW    ""

/* The 'HOME' entry or starting point to use in  
 * http://<host.running.web500gw>:8888/M  -- may be set with '-b' switch
 */
#define DEFAULT_HOMEDN  ""
/* define DEFAULT_HOMEDN "o=Technische Universitaet Chemnitz, c=DE" */

/* Default string to send, when "/robots.txt" is requested */
#define ROBOTS  "User-agent: *\nDisallow: /\n"

/* A special link to bind on an email address in entry display if
 * browser doesn't understand mailto.
 * i.e. link to a cgi-bin script on a http server:
 * #define  MAIL_CMD "<A
 * HREF=\"http://my-server/cgi-bin/program?%1$s\">%1$s</A>"
 * - %1$s is substituted by the email address
 */

#define  MAIL_CMD    \
        "<A HREF=\"http://www.tu-chemnitz.de/cgi-bin/mailform?%1$s\">%1$s</A>"

/* Default log entry per request */
#define LOGFORMAT "%h \"%r\" %s %e \"%a\" %b %l"

/* Search subtree below nodes of this objectclasses */
#define SEARCH_SUBTREE "organization, organizationalUnit"

/*
 *************************************************************************
 * The rest of this stuff probably does not need to be changed
 *************************************************************************
 */

/* The name of the general helpfile located in WEB500GWDIR
 * - displayed on /H - may be overridden with the '-h' flag */
#ifndef HELPFILE
#define HELPFILE	"web500gw.help"
#endif

/* The name of the helpfile on attributes located in WEB500GWDIR
 * - is displayed when user needs help on the meaning of attributes in
 *   the "modify form" */
#ifndef ATTRFILE
#define ATTRFILE    "web500gw.attr"
#endif

/* The ldap filter and friendly files, should now be installed with the ldap
 * package */
#ifdef FILTERFILE
#undef FILTERFILE
#endif
#define FILTERFILE	"ldapfilter.conf"

#ifdef FRIENDLYFILE
#undef FRIENDLYFILE
#endif
#define FRIENDLYFILE	"ldapfriendly"

#ifdef TEMPLATEFILE
#undef TEMPLATEFILE
#endif
#define TEMPLATEFILE    "ldaptemplates.conf"

/* The file containg all messages */
#ifndef MESSAGEFILE
#define MESSAGEFILE "web500gw.messages"
#endif

#define SIZELIMIT   LDAP_NO_LIMIT  /* no sizelimit */
#define TIMELIMIT	240	/* wait for LDAP results, seconds */
#define MAX_TRIES   4   /* */
#define MAX_TIME    MAX_TRIES * TIMELIMIT /* max time a request is handled */
