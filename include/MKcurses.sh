#!/bin/sh
#
# MKcurses.sh
#
### Generate curses.h from curses.h.in.  Insert the proper type for bool,
### also the current version stamp (get these from arguments).
version=$1
builtin_bool=$2
type_of_bool=$3

trap 'rm -f conftest.tmp; exit 0' 0 1 2 15

sed \
	-e "s/#\\(define\\)[ 	][ 	]*\\(CXX_BUILTIN_BOOL\\).*/#\\1 \\2 $builtin_bool/" \
	-e "s/#\\(define\\)[ 	][ 	]*\\(CXX_TYPE_OF_BOOL\\).*/#\\1 \\2 $type_of_bool/" \
	-e "s/@VERSION@/$version/" \
	$4/curses.h.in >conftest.tmp

if cmp -s curses.h conftest.tmp 2>/dev/null; then
	echo include/curses.h is unchanged
	rm -f conftest.tmp
else
	echo updating include/curses.h
	rm -f curses.h~
	if test -f curses.h; then mv curses.h curses.h~; fi
	mv conftest.tmp curses.h
fi
