#!/bin/sh
# Edit the default value of the term.h file based on the autoconf-generated
# values:
#
#	$1 = config.h
#	$2 = term.h
#
# FIXME: for now, this only has to fix the value of HAVE_TERMIOS_H
if ( grep -w HAVE_TERMIOS_H $1 2>&1 >/dev/null )
then
	echo >/dev/null
else
	echo '** edit'
	mv $2 $2.bak
	sed -e 's/define HAVE_TERMIOS_H.*$/define HAVE_TERMIOS_H 0/' $2.bak >$2
	rm -f $2.bak
fi
