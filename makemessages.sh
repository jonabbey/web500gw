#!/bin/sh
#
# $Id: makemessages.sh,v 1.2 2001/04/26 22:16:53 dekarl Exp $
#

[ -n "$1" ] || set web500gw.c util.c web_util.c dir_util.c ldap2html.c init.c config.c

cat "$@" |
  tr -s ' \11\12,;<=>()' '\12' |
  sed -n 's/.*\(MSG_[A-Za-z0-9_]*\).*/\1/p' |
  sort -u > msg-tmp || exit 1

awk '	BEGIN { OFS = "" }
	{print "#define ", $0, " resp->resp_language->l_conf->c_msg[", NR-1, "]"}
	END	{ print "#define MSG_count ", NR; }
' < msg-tmp > msg-tmp.h || exit 1
{
	[ -f messages.h ] && cmp -s msg-tmp.h messages.h
} ||
 mv msg-tmp.h messages.h

(
	cat <<EOH
#include "messages.h"
char *msg[MSG_count + 1] = {
EOH
	sed 's/MSG_\(.*\)/	"\1",/' < msg-tmp
	cat <<EOH
	(char *)0
};
EOH
) > msg-tmp.c || exit 1
{
	[ -f messages.c ] && cmp -s msg-tmp.c messages.c
} ||
 mv msg-tmp.c messages.c
