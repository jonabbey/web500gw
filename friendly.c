/* $OpenLDAP: pkg/ldap/libraries/libldap/friendly.c,v 1.13.8.4 2002/01/04 20:38:20 kurt Exp $ */
/*
 * Copyright 1998-2002 The OpenLDAP Foundation, All Rights Reserved.
 * COPYING RESTRICTIONS APPLY, see COPYRIGHT file
 */
/*  Portions
 *  Copyright (c) 1990 Regents of the University of Michigan.
 *  All rights reserved.
 *
 *  friendly.c
 */

#include "web500gw.h"

/* #include "ldap-int.h" */

char *
ldap_friendly_name(
	LDAP_CONST char *filename,
	/* LDAP_CONST */ char *uname,
	LDAPFriendlyMap **map )
{
	int	i, entries;
	FILE	*fp;
	char	*s;
	char	buf[BUFSIZ];

	if ( map == NULL ) {
		errno = EINVAL;
		return( uname );
	}

	if ( *map == NULL ) {
		if ( (fp = fopen( filename, "r" )) == NULL )
			return( uname );

		entries = 0;
		while ( fgets( buf, sizeof(buf), fp ) != NULL ) {
			if ( buf[0] != '#' )
				entries++;
		}
		rewind( fp );

		if ( (*map = (LDAPFriendlyMap *) LDAP_MALLOC( (entries + 1) *
		    sizeof(LDAPFriendlyMap) )) == NULL ) {
			fclose( fp );
			return( uname );
		}

		i = 0;
		while ( fgets( buf, sizeof(buf), fp ) != NULL && i < entries ) {
			if ( buf[0] == '#' )
				continue;

			if ( (s = strchr( buf, '\n' )) != NULL )
				*s = '\0';

			if ( (s = strchr( buf, '\t' )) == NULL )
				continue;
			*s++ = '\0';

			if ( *s == '"' ) {
				int	esc = 0, found = 0;

				for ( ++s; *s && !found; s++ ) {
					switch ( *s ) {
					case '\\':
						esc = 1;
						break;
					case '"':
						if ( !esc )
							found = 1;
						/* FALL */
					default:
						esc = 0;
						break;
					}
				}
			}

			(*map)[i].lf_unfriendly = strdup( buf );
			(*map)[i].lf_friendly   = strdup( s );
			i++;
		}

		fclose( fp );
		(*map)[i].lf_unfriendly = NULL;
	}

	for ( i = 0; (*map)[i].lf_unfriendly != NULL; i++ ) {
		if ( strcasecmp( uname, (*map)[i].lf_unfriendly ) == 0 )
			return( (*map)[i].lf_friendly );
	}
	return( uname );
}


void
ldap_free_friendlymap( LDAPFriendlyMap **map )
{
	LDAPFriendlyMap* pF = *map;

	if ( pF == NULL )
		return;

	while ( pF->lf_unfriendly )
	{
		free( pF->lf_unfriendly );
		free( pF->lf_friendly );
		pF++;
	}
	free( *map );
	*map = NULL;
}
