#ifndef NOSETPROCTITLE
/*
 * Copyright (c) 1990,1991 Regents of the University of Michigan.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of Michigan at Ann Arbor. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 *
 * $Id: setproctitle.c,v 1.3 2001/05/01 09:35:13 dekarl Exp $
 */

#include "web500gw.h"

char	**Argv;		/* pointer to original (main's) argv */
int	Argc;		/* original argc */

/*
 * takes the remote host and own port
 * this clobbers the original argv...
 */

void setproctitle(char *host, int port)
{
	static char *endargv = (char *)0;
	char	*s, *buf;
	int		i;

	/* make ps print "-progname ..." */
	if ( endargv == (char *)0 ) {
		/* set pointer to end of original argv */
		endargv = Argv[Argc-1] + strlen(Argv[Argc-1]);
	}
    s = (char *)strrchr(Argv[0], '/');       /* basename */

    buf = (char *)calloc(strlen(s ? s : Argv[0]) + strlen(host) + 10, sizeof(char));
    sprintf(buf, "-%s: %s %d", s ? s + 1 : Argv[0], host, port);
	i = strlen(buf);

	s = Argv[0];
	if (i > endargv - s - 1) {
		i = endargv - s - 1;
		buf[i] = '\0';
	}
	strcpy(s, buf);
    free(buf);
	s += i;
	while (s < endargv) *s++ = ' ';
}
#endif
