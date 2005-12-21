/*
 * util.c:      some utility routines
 *
 * written by: Frank Richter, Frank.Richter@hrz.tu-chemnitz.de
 *             Chemnitz University of Technology, Germany, 1994
 *
 * Copyright (c) 1994 Chemnitz University of Technology.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the Chemnitz University of Technology. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 *
 * $Id: util.c,v 1.3 2001/04/26 22:16:53 dekarl Exp $
 */

#include <unistd.h>
#include <gnuregex.h>
#include "web500gw.h"

/* msg_?printf: formats (and prints out) a string contained in message file */

static char buffer[10 * BUFSIZ];
static do_msg_snprintf (char *, int, char *, char *, va_list ap);

msg_fprintf (FILE *fp, char *fmt, char *format, ...) 
{
    va_list ap;
    va_start(ap, format);
    buffer[0] = '\0';
    do_msg_snprintf(buffer, 10 * BUFSIZ, fmt, format, ap);
    va_end(ap);
    return(fputs(buffer, fp));
}

msg_snprintf (char *buf, int buflen, char *fmt, char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    do_msg_snprintf(buf, buflen, fmt, format, ap);
    va_end(ap);
}

/* do_msg_printf: formats a string contained in message file
 * buf:    buffer space for resulting string
 * buflen: buffer len, not more than this count of bytes are written
 * fmt:    string from the messagefile, containing %1, %2 placeholders
 * format: string describing the following arguments,
 *         e.g. "si" means next args are string, then integer
 * ... variable argument list
 */
static 
do_msg_snprintf (char *buf, int buflen, char *fmt, char *format, va_list ap)
{
    int a, i, *ivars, len;
    char c, *f, *ff, *fo, *fout, *p, *s, **svars, digits[20];

    if (buflen <= 0)
        return(0);

    buf[0] = '\0';
    len = buflen - 1;

    /* get all arguments from stack */
    s = format;
    svars = (char **)calloc(strlen(s), sizeof(char *));
    ivars = (int *)calloc(strlen(s), sizeof(int));
    a = 0;
    while (s && *s) {
        switch (*s) {
            case 's': p = va_arg(ap, char *); 
                    svars[a] = p;
                    break;
            case 'i': i = va_arg(ap, int); 
                    ivars[a] = i;
                    break;
            case 'c': c = va_arg(ap, char); 
                    ivars[a] = (int)c;
                    break;
        }
        s++; a++;
    }
    va_end(ap);

    /* parse fmt for place holders %<number> and substitute them */
    fout = strdup(fmt);         
    fo = fout;
    f = fmt;
    while (f && *f) {
        if (*f == '%') {            /* start of place holder? */
            f++;
            if (*f == '%') {        /* %% means print a % */
                *fo++ = *f++;
            } else {
                ff = f;
                while (*ff >= '0' && *ff <= '9') ff++;
                if ((i = atoi((f))) > 0 && i <= a) {
                    *fo = '\0';
                    strncat(buf, fout, len);
                    len = buflen - strlen(buf) - 1;
                    if (format[i-1] == 's') {
                        strncat(buf, svars[i-1], len);
                    } else {
                        sprintf(digits, "%d", ivars[i-1]);
                        strncat(buf, digits, len);
                    }
                    fo = fout;
                    len = buflen - strlen(buf) - 1;
                }
                f = ff;
           }
        } else {
            *fo++ = *f++;
        }
    }
    *fo = '\0';
    strncat(buf, fout, len);
    free(fout);
    free(svars);
    free(ivars);
    return(strlen(buf));
}



/* compare needed to sort the list of search results in print_result () */
int 
compare(
    struct dncompare **a,
    struct dncompare **b
)
{
    if (*((*a)->sortstring) == '&' && *((*b)->sortstring) != '&')
        return 1;
    else if (*((*a)->sortstring) != '&' && *((*b)->sortstring) == '&')
        return -1;
    return strcasecmp((*a)->sortstring,(*b)->sortstring);
}


/* Converting routines between T.61 and ISO 8859-1 charsets */

#if !(defined(OWN_STR_TRANSLATION))
/* Build in from LDAP 3.2 */

unsigned char *
web500gw_t61toiso(char *in)
{
    return (unsigned char *)in;
}

unsigned char *
web500gw_isotot61(char *in)
{
    return (unsigned char *)in;
}

#else	/* defined(OWN_STR_TRANSLATION) */

/* 0xa0 - 0xbf */
static unsigned char trans1[32] = {
    '?', 0xa1, 0xa2, 0xa3, '$', 0xa5, '#', 0xa7, 
    0xa4, '?', '?', 0xab, '?', '?', '?', '?', 
    0xb0, 0xb1, 0xb2, 0xb3, 0xd7, 0xb5, 0xb6, 0xb7,
    0xf7, '?', '?', 0xbb, 0xbc, 0xbd, 0xbe, 0xbf
};

/* escape char 0xc0 - 0xcf */
static unsigned char trans2[16][17] = {
    {	'A','C','E','I','N','O','U','Y', 
	   'a','c','e','i','n','o','u','y', ' '
    },
/* Grave upper case */
	{  0xc0, '?', 0xc8, 0xcc, '?', 0xd2, 0xd9, '?',  
/* Grave lower case */
	   0xe0, '?', 0xe8, 0xec, '?', 0xf2, 0xf9, '?', '`'
    },
/* Acute upper case */
	{  0xc1, '?', 0xc9, 0xcd, '?', 0xd3, 0xda, 0xdd,
/* Acute lower case */
	   0xe1, '?', 0xe9, 0xed, '?', 0xf3, 0xfa, 0xfd, '\''
    },
/* Circumflex upper case */
	{  0xc2, '?', 0xca, 0xce, '?', 0xd4, 0xdb, '?', 
/* Circumflex lower case */
	   0xe2, '?', 0xea, 0xee, '?', 0xf4, 0xfb, '?', '?'
    },
/* Tilde upper case */
	{  0xc3, '?', 0xcb, 0xcf, 0xd1, 0xd5, 0xdc, '?', 
/* Tilde lower case */
	   0xe3, '?', 0xeb, 0xef, 0xf1, 0xf5, 0xfc, '?', '?'
    },
/* Macron upper case */
	{  '?', '?', '?', '?', '?', '?', '?', '?',   
/* Macron lower case */
	   '?', '?', '?', '?', '?', '?', '?', '?',  '?'
    },
/* ? upper case */
	{  '?', '?', '?', '?', '?', '?', '?', '?', 
/* ? lower case */
	   '?', '?', '?', '?', '?', '?', '?', '?',  '?'
    },
/* Dot upper case */
	{  0xc5, '?', '?', '?', '?', '?', '?', '?',
/* Dot lower case */
	   0xe5, '?', '?', '?', '?', '?', '?', '?', '?'
    },
/* Diaresis upper case */
    {   0xc4, '?', 0xcb, 0xcf, '?', 0xd6, 0xdc, 0x82,
/* Diaresis lower case */
        0xe4, '?', 0xeb, 0xef, '?', 0xf6, 0xfc, 0xff, '"'
    },
/* Umlaut upper case */
    {   0xc4, '?', 0xcb, 0xcf, '?', 0xd6, 0xdc, 0x82,
/* Umlaut lower case */
        0xe4, '?', 0xeb, 0xef, '?', 0xf6, 0xfc, 0xff, '"'
    },
/* Ring upper case */
	{  0xc5, '?', '?', '?', '?', '?', '?', '?', 
/* Ring lower case */
	   0xe5, '?', '?', '?', '?', '?', '?', '?', '?'
    },
/* Cedilla upper case */
	{  '?', 0xc7, '?', '?', '?', '?', '?', '?',
/* Cedilla lower case */
	   '?', 0xe7, '?', '?', '?', '?', '?', '?',  '?'
    },
/* Underline upper case */
	{  '?', '?', '?', '?', '?', '?', '?', '?',
/* Underline lower case */
	   '?', '?', '?', '?', '?', '?', '?', '?',  '_'
    },
/* Umlaut upper case */
    {   0xc4, '?', 0xcb, 0xcf, '?', 0xd6, 0xdc, 0x82,
/* Umlaut lower case */
        0xe4, '?', 0xeb, 0xef, '?', 0xf6, 0xfc, 0xff, '"'
    },
/* Cedilla? upper case */
	{  '?', 0xc7, '?', '?', '?', '?', '?', '?',
/* Cedilla? lower case */
	   '?', 0xe7, '?', '?', '?', '?', '?', '?',  '?'
    },
/* ? upper case */
	{  '?', '?', '?', '?', '?', '?', '?', '?',
/* ? lower case */
	   '?', '?', '?', '?', '?', '?', '?', '?', '?'
    }
};

/* 0xe0 - 0xff */
static unsigned char trans3[32] = {
	'?', 0xc6, 0xd0, 0xaa, 'H', '?', '?', 'L', 
	'L', 0xd8, '?', 0xba, 0xde, 'T', 'N', 'n',
	'K', 0xe6, 'd', 0xf0, 'h', 'i', '?', 'l',
	'l', 0xf8, '?', 0xdf, 0xfe, 't', 'N', '?'
};

static unsigned char buff[4096];

unsigned char *
web500gw_t61toiso(char *in)
{
    unsigned char *str, *out;
    int n;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  web500gw_t61toiso (%s)\n", 
        in, 0, 0, 0);
#endif

    str = (unsigned char *) in;
    /* buff = (unsigned char *) malloc (strlen(in) * 8); */
    out = buff;
    while (*str) {
        if ((*str > 0x1f) && (*str < 0x80)) {
            *out++ = *str;
        } else if ((*str > 0x9f) && (*str < 0xc0)) { 
            *out++ = trans1[((int)(*str)) - 0xa0];
        } else if (*str > 0xdf) {
            *out++ = trans3[((int)(*str)) - 0xe0];
        } else if ((*str > 0xbf) && (*str < 0xd0)) {
            n = (int) (*str - (unsigned char) 0xc0);
	        str++;
            if (*str == '\0')
                break;      /* Error */
            switch (*str) {
            case 'A':
                *out++ = trans2[n][0];
                break;
            case 'C':
                *out++ = trans2[n][1];
                break;
            case 'E':
                *out++ = trans2[n][2];
                break;
            case 'I':
                *out++ = trans2[n][3];
                break;
            case 'N':
                *out++ = trans2[n][4];
                break;
            case 'O':
                *out++ = trans2[n][5];
                break;
            case 'U':
                *out++ = trans2[n][6];
                break;
            case 'Y':
                *out++ = trans2[n][7];
                break;
            case 'a':
                *out++ = trans2[n][8];
                break;
            case 'c':
                *out++ = trans2[n][9];
                break;
            case 'e':
                *out++ = trans2[n][10];
                break;
            case 'i':
                *out++ = trans2[n][11];
                break;
            case 'n':
                *out++ = trans2[n][12];
                break;
            case 'o':
                *out++ = trans2[n][13];
                break;
            case 'u':
                *out++ = trans2[n][14];
                break;
            case 'y':
                *out++ = trans2[n][15];
                break;
            case ' ':
                *out++ = trans2[n][16];
                break;
            default:
                *out++ = '?';
            }
        }
        str++;
    }
    *out++ = '\0';
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  web500gw_t61toiso >%s<\n", buff, 0, 0, 0);
#endif

    return (buff);
}

/* 0xa0 - 0xbf */
static unsigned char trans4[32] = {
    '?', 0xa1, 0xa2, 0xa3, 0xa8, 0xa5, '?', 0xa7,
    '?', '?', 0xe3, 0xab, '?', '?', '?', '?',
    0xb0, 0xb1, 0xb2, 0xb3, '?', 0xb5, 0xb6, 0xb7,
    '?', '?', 0xeb, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf
};

/* 0xc0 - 0xff */
static unsigned char *trans5[64] = {
	"\301A", "\302A", "\303A", "\304A", "\310A", "\312A", "\341", "\313C",
    "\301E", "\302E", "\303E", "\310E", "\301I", "\302I", "\303I", "\310I",
    "\322",  "\304N", "\301O", "\302O", "\303O", "\304O", "\310O", "\264",
    "\331",  "\301U", "\302U", "\303U", "\310U", "\302Y", "\354", "\373",
    "\301a", "\302a", "\303a", "\304a", "\310a", "\312a", "\351", "\313c",
    "\301e", "\302e", "\303e", "\310e", "\301i", "\302i", "\303i", "\310i",
    "\353",  "\304n", "\301o", "\302o", "\303o", "\304o", "\310o", "\270",
    "\371", "\301u", "\302u", "\303u", "\310u", "\302y", "\374", "\310y"
};

unsigned char *
web500gw_isotot61(char *in)
{
    unsigned char *str, *out;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  web500gw_isotot61 (%s)\n", 
        in, 0, 0, 0);
#endif

    str = (unsigned char *) in;
    /* buff = (unsigned char *) malloc (strlen(in) * 8); */
    out = buff;
    while (*str != '\0') {
        if ((*str > 0x1f) && (*str < 0x80)) {
            *out++ = *str;
        } else if ((*str > 0x9f) && (*str < 0xc0)) {
            *out++ = trans4[((int)(*str)) - 0xa0];
        } else if (*str > 0xbf) {
            strcpy(out, trans5[((int)(*str)) - 0xc0]);
            out += strlen(trans5[((int)(*str)) - 0xc0]);
        } else {
            *out++ = '?';
        }
        str++;
    }
    *out++ = '\0';
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  web500gw_isotot61 >%s<\n", buff, 0, 0, 0);
#endif

    return (buff);
}

#endif	/* OWN_STR_TRANSLATION */

/* gtime(): the inverse of localtime().
    This routine was supplied by Mike Accetta at CMU many years ago.
 */

static int    dmsize[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static char *dmnames[12] = {
    "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};

#define    dysize(y)    \
    (((y) % 4) ? 365 : (((y) % 100) ? 366 : (((y) % 400) ? 365 : 366)))

#define    YEAR(y)        ((y) >= 100 ? (y) : (y) + 1900)

time_t
gtime (struct tm *tm)
{
    int    i, sec, mins, hour, mday, mon, year;
    long   result;

    if ((sec = tm -> tm_sec) < 0 || sec > 59
        || (mins = tm -> tm_min) < 0 || mins > 59
        || (hour = tm -> tm_hour) < 0 || hour > 24
        || (mday = tm -> tm_mday) < 1 || mday > 31
        || (mon = tm -> tm_mon + 1) < 1 || mon > 12)
        return ((long) -1);
    if (hour == 24) {
        hour = 0;
        mday++;
    }
    year = YEAR (tm -> tm_year);
    /* note that 2-digit-year LDAP timestamp will have "00" for "2000" */
    /* above year transformation will return "1900" for "00", 2-digit "2000" */
    /* the following assumes no real dates < 1970 */
    if (year < 1970) year += 100; /* turn "1900" into "2000" */
    result = 0L;
    for (i = 1970; i < year; i++)
        result += dysize (i);
    if (dysize (year) == 366 && mon >= 3)
        result++;
    while (--mon)
        result += dmsize[mon - 1];
    result += mday - 1;
    result = 24 * result + hour;
    result = 60 * result + mins;
    result = 60 * result + sec;
    return result;
}


/* returns a printable date string from LDAP's date format:
 *    s       should point to: YYMMDDHHmmSSZ
 *    format  format string for strftime(3)
 */
char *
format_date (
    char    *s,
    char    *format)
{
    char        mydate[256];
    struct tm   tm, *ntm;
    time_t      t;
    int		ds_off; /* date string offset */

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  format_date (%s, \"%s\")\n",
        s ? s : "??", format, 0, 0);
#endif
    if (!s)
        return(NULL);

    /* s should point to: YYMMDDHHmmSSZ (13 chars) */
    /* ... well 2 digits for year :-( */
    /* or: YYYYMMDDHHmmSSZ (15 chars; 4-digit years) */
    if (strlen(s) == 13) {
        tm.tm_year = 10*(s[0] - '0') + (s[1] - '0'); ds_off = 2;}
    else {
        tm.tm_year = 1000*(s[0] - '0') + 100*(s[1] - '0') +
                       10*(s[2] - '0') +     (s[3] - '0'); ds_off = 4;}
    tm.tm_mon  = 10*(s[ds_off] - '0')    + (s[ds_off+1] - '0') - 1;
    tm.tm_mday = 10*(s[ds_off+2] - '0')  + (s[ds_off+3] - '0');
    tm.tm_hour = 10*(s[ds_off+4] - '0')  + (s[ds_off+5] - '0');
    tm.tm_min  = 10*(s[ds_off+6] - '0')  + (s[ds_off+7] - '0');
    tm.tm_sec  = 10*(s[ds_off+8] - '0')  + (s[ds_off+9] - '0');
    tm.tm_isdst = 0;

#if ! (defined(__hpux) || defined(_AIX) || defined(sunos5) || defined(linux) || defined(unixware7))
    tm.tm_gmtoff = 0;
#endif
    t = gtime(&tm);
    ntm = gmtime(&t);
    strftime(mydate, 256, format, ntm);

#ifdef WEB500GW_DEBUG
        Web500gw_debug(WEB500GW_DEBUG_UTIL, "  format_date >%s<\n",
            mydate, 0, 0, 0);
#endif

    return (strdup(mydate));
}

/* finds the the month number from the month name */
int
find_month (char *month) {
    int i;

    for (i = 0; i < 12; i++) {
        if (strcmp(month, dmnames[i]) == 0)
            return (i + 1);
    }
    return -1;
}

/* compares 2 dates:  
 *   ldap_date  should point to: YYMMDDHHmmSSZ
 *   http_date   is a HTTP date (3 different formats ...)
 * returns 1,0,-1 if first date is newer, equal, older to second
 */
int
cmp_dates (
    char    *ldap_date,
    char    *http_date)
{
    char month_name[4];
    int year = 0, month = 0, day = 0, hour = 0, min = 0, sec = 0;
    int i = 0;
    int ds_off; /* date stamp offset */

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  cmp_dates (%s, %s)\n",
        ldap_date ? ldap_date : "??", http_date ? http_date : "??", 0, 0);
#endif

    if (! ldap_date)
        return -1;
    if (! http_date)
        return 1;
    
    /* strip off leading white space */
    while (isspace(*http_date)) http_date++; 

    /* Every format starts with a weekday + space */
    if (!(http_date = strchr(http_date, ' '))) {
        return 1;
    } else {
        while (isspace(*http_date)) http_date++;
    }

    if (isalpha(*http_date)) {      /* ctime date: Mmm dd hh:mm:ss yyyy */
        sscanf(http_date, "%s %d %d:%d:%d %d",
            month_name, &day, &hour, &min, &sec, &year);
    } else if (http_date[2] == '-') {
                        /* RFC 850: dd-Mmm-yy hh:mm:ss */
        sscanf(http_date, "%d-%3s-%d %d:%d:%d",
            &day, month_name, &year, &hour, &min, &sec);
	/* convert 2-digit year into 4-digit year based on Unix beg date */
      if (year < 70)   year += 100;
      if (year < 1900) year += 1900;
    } else {         /* normal HTTP date (RFC 822/1123): dd Mmm yyyy hh:mm:ss */
        sscanf(http_date, "%d %s %d %d:%d:%d", 
            &day, month_name, &year, &hour, &min, &sec);
    }
    if ((month = find_month(month_name)) == -1) {
        return 1;
    }
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, 
        "  cmp_dates: scanned http_date: %d.%d.%d ",
        day, month, year, 0);
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "%d:%d:%d\n", hour, min, sec, 0);
#endif
    /* ldap_date should point to: YYMMDDHHmmSSZ (13 chars) */
    /* ... well 2 digits for year :-( */
    /* or: YYYYMMDDHHmmSSZ (15 chars; 4-digit years) */
    if (strlen(ldap_date) == 13) { 
      ds_off=2;
      if ((i = ((10*(ldap_date[0] - '0') +
	 (ldap_date[1] - '0') + 1900) - year))) return i > 0;}
    else {
      ds_off=4;
      if ((i = ((1000*(ldap_date[0] - '0') +
                  100*(ldap_date[1] - '0') +
                   10*(ldap_date[2] - '0') +
                      (ldap_date[3] - '0')) - year ))) return i > 0;}

    if ((i = ((10*(ldap_date[ds_off] - '0') + (ldap_date[ds_off+1] - '0')) - month)))
        return i > 0;
    if ((i = ((10*(ldap_date[ds_off+2] - '0') + (ldap_date[ds_off+3] - '0')) - day)))
        return i > 0;
    if ((i = ((10*(ldap_date[ds_off+4] - '0') + (ldap_date[ds_off+5] - '0')) - hour)))
        return i > 0;
    if ((i = ((10*(ldap_date[ds_off+6] - '0') + (ldap_date[ds_off+7] - '0')) - min)))
        return i > 0;
    if ((i = ((10*(ldap_date[ds_off+8] - '0') + (ldap_date[ds_off+9] - '0')) - sec)))
        return i > 0;

    /* gone so far - dates are identical */
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  cmp_dates >%d< (identical)\n",
        i, 0, 0, 0);
#endif
    return i;
}

	
char *
abs_file_name (
    char    *name, 
    char    *dir,
    char    *suffix)
{
    int suffix_len;
    static char *filename = 0;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  abs_file_name (%s, %s, %s)\n",
        name ? name : "", dir ? dir : "", suffix ? suffix : "", 0);
#endif

    if ((!suffix || !*suffix) && 
       (*name == '/' || !*name || (*name == '.' && name[1] == '/') || !dir)) {
#ifdef WEB500GW_DEBUG
        Web500gw_debug(WEB500GW_DEBUG_UTIL, "  abs_file_name >%s<\n", 
            name, 0, 0, 0);
#endif
        return name;
    }

    if (filename)
        free (filename);
    suffix_len = (suffix ? strlen (suffix) : 0);
    filename = malloc (strlen (name) + strlen (dir) + 3 + suffix_len);
    if (suffix_len) {
        sprintf (filename, "%s/%s%s", dir, name, suffix);
        if (access (filename, 0) == 0)
            goto found;
    }
    sprintf (filename, "%s/%s", dir, name);

found:
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  abs_file_name >%s<\n", filename, 0, 0, 0);
#endif

    return filename;
}


/* converts flags to string - for dn2url, debugging etc. */

char *
flag2string (
    unsigned int    flag
)
{
    char    flagstring[BUFSIZ];
    int     i;

    flagstring[0] = '\0';
    for (i = 0; disp_flags[i].o_name; i++) {
        if (flag & disp_flags[i].o_value) {
            if (disp_flags[i].o_value != FLAG_TABLE_BORDER ||
                flag & FLAG_BORDER) {
                if (flagstring[0] != '\0')
                    strcat(flagstring, ",");
                strcat(flagstring, disp_flags[i].o_name);
            }
        }
    }
    return(strdup(flagstring));
}

/* converts actions to string - for dn2url, debugging etc. */

char *
actions2string (
    unsigned int    flag
)
{
    char    flagstring[BUFSIZ];
    int     i;

    flagstring[0] = '\0';
    for (i = 0; disp_actions[i].o_name; i++) {
        if (flag & disp_actions[i].o_value) {
            if (flagstring[0] != '\0')
                strcat(flagstring, ",");
            strcat(flagstring, disp_actions[i].o_name);
        }
    }
    return(strdup(flagstring));
}

/* converts a "flag string" (in URL) to flag */

unsigned int
string2flag (
    char    *string
)
{
    unsigned int    flag = 0;
    int     i;

    for (i = 0; disp_flags[i].o_name; i++) {
        if (strcasecmp(string, disp_flags[i].o_name) == 0) {
            flag = disp_flags[i].o_value;
            break;
        }
    }
    return(flag);
}
/* converts a "flag string" (in URL) to action */

unsigned int
string2action (
    char    *string
)
{
    unsigned int    action = 0;
    int     i;

    for (i = 0; disp_actions[i].o_name; i++) {
        if (strcasecmp(string, disp_actions[i].o_name) == 0) {
            action = disp_actions[i].o_value;
            break;
        }
    }
    return(action);
}

/* find_access: finds the matching access entry */

struct access *
find_access (char *host)
{
    struct access  *a;
    char           *h;
    int             i = 0;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  find_access (%s)\n",
        host ? host : "", 0, 0, 0);
#endif

    /* ignore case - tolower all characters */
    h = malloc(strlen(host) + 1);
    while (host[i] != '\0') {
        h[i] = tolower(host[i]);
        i++;
    }
    h[i] = '\0';

    for (a = accesses; a && a->a_domainpattern; a = a->a_next) {
        if (re_comp(a->a_domainpattern) == NULL &&
            re_exec(h) == 1) {
            return(a);
        }
    }
    return (struct access *)NULL;
}


/* find_language: finds the matching language entry */

struct language *
find_language (char *lang_name)
{
    struct language *l;

    for (l = languages; l && l->l_pattern; l = l->l_next) {
        if (re_comp(l->l_pattern) == NULL &&
            re_exec(lang_name) == 1) {
            return(l);
        }
    }
    return (struct language *)NULL;
}


/* find_other_languages: finds other languages than the one used in
 *                       response, build strings to use in message file */

find_other_languages (
    REQUEST         *r,
    RESPONSE        *resp
)
{
    struct language *l;
    int     len = BUFSIZ;
    char    *url, *label, *save_r_language, *s, *f;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  find_other_languages (%s)\n",
        resp ? resp->resp_language->l_content_lang : "", 0, 0, 0);
#endif

    other_lang_string = calloc(1, len);
    other_lang_select   = calloc(1, len);

    strcpy(other_lang_select, "<SELECT NAME=\"\">\n");

    /* save user requested language */
    save_r_language = r->r_language;
    r->r_language = NULL;

    for (l = languages; l && l->l_pattern; l = l->l_next) {
        if (resp->resp_language == l) {
            /* that's our used language - ignore */
            continue;
        }
        s = calloc(1, strlen(l->l_content_lang) + 8);
        strcpy(s, "lang_");
        strcat(s, l->l_content_lang);
        label = friendly_label(resp, s);

        f = calloc(1, strlen(l->l_content_lang) + 9);
        strcpy(f, "$lang=");
        strncat(f, l->l_content_lang, strlen(l->l_content_lang)); 
        /* strcpy(f + strlen(f), l->l_content_lang); */

        url = dn2url(r, r->r_dn, r->r_flags & FLAG_LAYOUT, r->r_actions, f, NULL);

        while (strlen(url) + strlen(other_lang_string) + 20 >= len) {
            len += BUFSIZ;
            other_lang_string = realloc(other_lang_string, len);
        }
        sprintf(other_lang_string + strlen(other_lang_string), 
            " [ <A HREF=\"%s\">%s</A> ] ",
            url, label ? label : l->l_content_lang);

        while (strlen(url) + strlen(other_lang_select) + 20 >= len) {
            len += BUFSIZ;
            other_lang_select = realloc(other_lang_select, len);
        }
        sprintf(other_lang_select + strlen(other_lang_select), 
            "<OPTION VALUE=\"%s\"> %s\n", url, label);

        free(s);
        free(f);
    }
    if (strlen(other_lang_select) + 12  >= len) {
        len += BUFSIZ;
        other_lang_select = realloc(other_lang_select, len);
    }
    strcat(other_lang_select, "</SELECT>\n");

    /* reset user requested language */
    r->r_language = save_r_language;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  find_other_languages >length: %d, %d<\n",
        strlen(other_lang_string), strlen(other_lang_select), 0, 0);
#endif
}

/* find_browser: finds the matching browser entry */

struct browser_opts *
find_browser (char *browser_name)
{
    struct browser_opts *b;
    for (b = browser_opts; b && b->b_pattern; b = b->b_next) {
        if (re_comp(b->b_pattern) == NULL &&
            re_exec(browser_name) == 1) {
            return(b);
        }
    }
    return (struct browser_opts *)NULL;
}


/* converts a "access_right string" (in config file) to right flag */

unsigned int
string2access_right (
    char    *string
)
{
    unsigned int    access_right = ACCESS_NOTHING;
    int     i;

    for (i = 0; access_rights[i].o_name; i++) {
        if (strcasecmp(string, access_rights[i].o_name) == 0) {
            access_right = access_rights[i].o_value;
            break;
        }
    }
    return(access_right);
}

/* converts access rights to string - for debugging etc. */

char *
access_right2string (
    unsigned int    access_right
)
{
    char    access_string[128];
    int     i;

    access_string[0] = '\0';
    for (i = 0; access_rights[i].o_name; i++) {
        if (access_right == access_rights[i].o_value) {
            if (access_string[0] != '\0')
                strcat(access_string, ",");
            strcat(access_string, access_rights[i].o_name);
        }
    }
    return(strdup(access_string));
}

/* converts a "browser options string" (in config file) to right flag */

unsigned int
string2opts (
    char                    *string,
    struct config_options   *c

)
{
    unsigned int    option = 0;
    int     i;

    if (! (c && string))
        return (0);

    for (i = 0; c[i].o_name; i++) {
        if (strcasecmp(string, c[i].o_name) == 0) {
            option = c[i].o_value;
            break;
        }
    }
    return(option);
}

/* converts browser options to string - for debugging etc. */

char *
opts2string (
    unsigned int            option,
    struct config_options   *c
)
{
    char    string[128];
    int     i;

    if (! c)
        return("");

    string[0] = '\0';
    for (i = 0; c[i].o_name; i++) {
        if (c[i].o_value && ((option | c[i].o_value) == option)) {
            if (string[0] != '\0')
                strcat(string, ",");
            strcat(string, c[i].o_name);
        }
    }
    return(strdup(string));
}


char *
friendly_label (
    RESPONSE    *resp,
    char        *label
)
{
    char    *f_label;
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  friendly_label (%s, %s)\n",
        label, resp ? resp->resp_language->l_conf->c_friendlyfile : "", 0, 0);
#endif
    if (! (resp && label))
        return (label);
    
    f_label =
        ldap_friendly_name(resp->resp_language->l_conf->c_friendlyfile, 
        label, &resp->resp_language->l_conf->c_fm);

    if (f_label == NULL)
        f_label = label;
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  friendly_label >%s<\n",
        f_label, 0, 0, 0);
#endif
    return(f_label);
}


log_request (
    REQUEST     *r,
    RESPONSE    *resp
)
{
    char    *cp, *lp, *np, *logstring, string[BUFSIZ];
    int     len = 0, size = BUFSIZ, *ip, i;
    time_t  t;

    if (log_format == NULL)
        log_format = LOGFORMAT;
    lp = log_format;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_UTIL, "  log_request \"%s\"\n",
        log_format, 0, 0, 0);
#endif
    logstring = calloc(1, size);
    while ((cp = strchr(lp, '%')) != NULL) {
        ip = (int *)0;
        *cp++ = '\0';
        switch (*cp) {
        case 'h':
            np = r->r_client_host;
            break;
        case 'r':
            np = r->r_request;
            break;
        case 's':
            ip = &resp->resp_status;
            break;
        case 'e':
            ip = &resp->resp_error;
            break;
        case 'a':
            np = r->r_access->a_desc;
            break;
        case 'b':
            np = r->r_browser->b_desc;
            break;
        case 'B':
            np = r->r_useragent;
            break;
        case 'l':
            np = resp->resp_language->l_content_lang;
            break;
        case 't':
            np = resp->resp_date;
            break;
        case 'T':
            time(&t);
            i = (int)t - r->r_tm;
            ip = &i;
            break;
        case 'f':
            np = r->r_referer;
            break;
        case 'x':
            np = r->r_ldaphost;
            break;
        }
        cp++;
        if (ip) {       /* integer value */
            sprintf(string, "%d", *ip);
            np = string;
        }
        len += (lp ? strlen(lp) : 0) + (np ? strlen(np) : 0);
        if (len >= size) {
            size += BUFSIZ;
            logstring = realloc(logstring, size);
        }
        if (lp)
            strcat(logstring, lp);
        if (np)
            strcat(logstring, np);
        lp = cp;
    }
    if (lp && *lp) {
        len += sizeof(lp);
        if (len >= size) {
            size += BUFSIZ;
            logstring = realloc(logstring, size);
       }
       strcat(logstring, lp);
    }
#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_CONNS, "syslog: %s\n", logstring, 0, 0, 0);
#endif

    if (dosyslog) {
        syslog(LOG_INFO, "%s", logstring);
    }
    free(logstring);
}

int
access_ok (
    REQUEST     *r,
    RESPONSE    *resp,
    int         right
)
{
    if (r->r_access && r->r_access->a_rights & right)
        return OK;

#ifdef WEB500GW_DEBUG
    Web500gw_debug(WEB500GW_DEBUG_TRACE,
        " access_ok: Access denied for %s: needs 0x%x <-> has 0x%x\n", 
        r->r_client_host, right, 
        r->r_access ? ACCESS_NOTHING : r->r_access->a_rights, 0);
#endif
    resp->resp_status = FORBIDDEN;
    resp->resp_error = LDAP_OTHER;
    do_error(r, resp, LDAP_INSUFFICIENT_ACCESS, resp->resp_status, 
        right == ACCESS_WRITE ? MSG_ACCESS_READONLY : MSG_ACCESS_COMMON, NULL);
    return NOTOK;
}

#ifdef sunos4
extern int sys_nerr;
extern char *sys_errlist[];

char *
strerror(int err_number)
{
    if (err_number >= 0 && err_number < sys_nerr)
        return sys_errlist[err_number];
    else
        return NULL;
}
#endif

