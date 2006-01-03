/*
 * web500gw:  A LDAP based WWW - X.500 gateway
 *
 * written by: Frank Richter, Frank.Richter@hrz.tu-chemnitz.de
 *             Chemnitz University of Technology, Germany
 *
 * Copyright (c) 1994-8 Chemnitz University of Technology.
 * All rights reserved.
 *
 * filters.c
 *
 * written by: Jonathan Abbey
 *             Applied Research Laboratories, The University of Texas at Austin
 *
 * Copyright (c) 2005 The University of Texas at Austin
 *
 * $Id$
 */

#include "web500gw.h"

static int filter_index = 0;

/*------------------------------------------------------------------------------

                                                   construct_first_ldap_filter()

This function is used to take a search attribute (a string, like
"Jonathan Abbey", "D. Scott", or "835-3199") and return a proper
search expression in LDAP RFC 2254 format, which can be used to query
the LDAP server.  It is intended to serve as a partial substitute for
the LDAP ldap_getfirstfilter() function that was removed in recent
versions of the LDAP C client API specification.

construct_first_ldap_filter() should be used for the first search
attempt.  If a search on the resulting search expression does not
return useful data, subsequent search expressions can be retrieved by
calling construct_next_ldap_filter() in sequence, until such time as
construct_next_ldap_filter() returns NULL.

If at any time construct_first_ldap_filter() is called again, the
search pattern sequencing will be reset to the beginning.

-------------------------------------------------------------------------------*/

char *construct_first_ldap_filter(char *search_value)
{
  filter_index = 0;
  return construct_next_ldap_filter(search_value);
}

/*------------------------------------------------------------------------------

                                                    construct_next_ldap_filter()

This function is used to take a search attribute (a string, like
"Jonathan Abbey", "D. Scott", or "835-3199") and return a proper
search expression in LDAP RFC 2254 format, which can be used to query
the LDAP server.  It is intended to serve as a partial substitute for
the LDAP ldap_getnextfilter() function that was removed in recent
versions of the LDAP C client API specification.

construct_next_ldap_filter() should be used for constructing
additional search attempt in a sequence.  construct_next_ldap_filter()
may be called repeatedly to generate successive formulations of search
expressions for web500gw.  If the end of the sequence is reached,
construct_next_ldap_filter() will return NULL.

If at any time construct_first_ldap_filter() is called again, the
search pattern sequencing will be reset to the beginning.

-------------------------------------------------------------------------------*/

char *construct_next_ldap_filter(char *search_value)
{
  return NULL;
}
