#!/bin/sh
# simple BSD-like install replacement
#
# Copyright (c) 1994 The Regents of the University of Michigan
#
# $Id: install.sh,v 1.2 2001/04/26 22:16:53 dekarl Exp $
#

MODE=0755
USAGE="usage: $0 [-c] [-m mode] file dir"

while [ $# != 0 ]; do
    case "$1" in
    -c)
	;;
    -m)
	MODE=$2
	shift
	;;
    -*)
	echo "$USAGE"
	exit 1
	;;
    *)
	break
	;;
    esac
    shift
done

if [ $# != 2 ]; then
    echo "$USAGE"
    exit 1
fi

FILE=$1
DIR=$2

# added by fri to save older versions
DF="$DIR/`basename $FILE`"

[ -f "$DF~~" ] && cp "$DF~~" "$DF~~~"
[ -f "$DF~" ] && cp "$DF~" "$DF~~"
[ -f "$DF"   ] && cp "$DF" "$DF~"

cp $FILE $DIR
if [ -d $DIR ]; then
    chmod $MODE $DIR/`basename $FILE`
else
#
# DIR is really the destination file
#
    chmod $MODE $DIR
fi
