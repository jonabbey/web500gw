/* $OpenLDAP: pkg/ldap/include/ldap.h,v 1.65.2.40 2002/02/11 19:28:01 kurt Exp $ */
/*
 * Copyright 1998-2002 The OpenLDAP Foundation, Redwood City, California, USA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted only as authorized by the OpenLDAP
 * Public License.  A copy of this license is available at
 * http://www.OpenLDAP.org/license.html or in file LICENSE in the
 * top-level directory of the distribution.
 */
/* Portions
 * Copyright (c) 1990 Regents of the University of Michigan.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of Michigan at Ann Arbor. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 */

#ifndef _LDAP_COMPAT_H
#define _LDAP_COMPAT_H

#define LDAP_DEPRECATED 1

#include <ldap.h>
#include <string.h>

LDAP_BEGIN_DECL

#define LDAP_MALLOC(v) (malloc(v))
#define LDAP_VFREE(v)  (free(v))
#define LDAP_FREE(v)  (free(v))
#define LBER_FREE(v)  (free(v))
#define LDAP_STRDUP(v) (strdup(v))
#define LDAP_REALLOC(p,s) (realloc((p),(s)))
#define LDAP_CALLOC(s,t) (calloc((s),(t)))

#define LDAP_DIGIT(c)		( (c) >= '0' && (c) <= '9' )
#define LDAP_SPACE(c)		((c) == ' ' || (c) == '\t' || (c) == '\n')
#define AC_MEMCPY( d, s, n ) memcpy((d), (s), (n))

/*
 * structures for ldap getfilter routines
 */

typedef struct ldap_filt_info {
	char			*lfi_filter;
	char			*lfi_desc;
	int			lfi_scope;
	int			lfi_isexact;
	struct ldap_filt_info	*lfi_next;
} LDAPFiltInfo;

typedef struct ldap_filt_list {
    char			*lfl_tag;
    char			*lfl_pattern;
    char			*lfl_delims;
    LDAPFiltInfo		*lfl_ilist;
    struct ldap_filt_list	*lfl_next;
} LDAPFiltList;


#define LDAP_FILT_MAXSIZ	1024

typedef struct ldap_filt_desc {
	LDAPFiltList		*lfd_filtlist;
	LDAPFiltInfo		*lfd_curfip;
	LDAPFiltInfo		lfd_retfi;
	char			lfd_filter[ LDAP_FILT_MAXSIZ ];
	char			*lfd_curval;
	char			*lfd_curvalcopy;
	char			**lfd_curvalwords;
	char			*lfd_filtprefix;
	char			*lfd_filtsuffix;
} LDAPFiltDesc;

/*
 * structure for ldap friendly mapping routines
 */

typedef struct ldap_friendly {
	char	*lf_unfriendly;
	char	*lf_friendly;
} LDAPFriendlyMap;


/*
 * in friendly.c
 *	(deprecated)
 */
LDAP_F( char * )
ldap_friendly_name LDAP_P(( /* deprecated */
	LDAP_CONST char *filename,
	/* LDAP_CONST */ char *uname,
	LDAPFriendlyMap **map ));

LDAP_F( void )
ldap_free_friendlymap LDAP_P(( /* deprecated */
	LDAPFriendlyMap **map ));

/*
 * in getfilter.c
 *	(deprecated)
 */
LDAP_F( LDAPFiltDesc *)
ldap_init_getfilter LDAP_P(( /* deprecated */
	LDAP_CONST char *fname ));

LDAP_F( LDAPFiltDesc *)
ldap_init_getfilter_buf LDAP_P(( /* deprecated */
	/* LDAP_CONST */ char *buf,
	ber_len_t buflen ));

LDAP_F( LDAPFiltInfo *)
ldap_getfirstfilter LDAP_P(( /* deprecated */
	LDAPFiltDesc *lfdp,
	/* LDAP_CONST */ char *tagpat,
	/* LDAP_CONST */ char *value ));

LDAP_F( LDAPFiltInfo *)
ldap_getnextfilter LDAP_P(( /* deprecated */
	LDAPFiltDesc *lfdp ));

LDAP_F( void )
ldap_setfilteraffixes LDAP_P(( /* deprecated */
	LDAPFiltDesc *lfdp,
	LDAP_CONST char *prefix,
	LDAP_CONST char *suffix ));

LDAP_F( void )
ldap_build_filter LDAP_P(( /* deprecated */
	char *buf,
	ber_len_t buflen,
	LDAP_CONST char *pattern,
	LDAP_CONST char *prefix,
	LDAP_CONST char *suffix,
	LDAP_CONST char *attr,
	LDAP_CONST char *value,
	char **valwords ));

extern char *(ldap_pvt_strtok)( char *str, const char *delim, char **pos );

LDAP_END_DECL

#endif
