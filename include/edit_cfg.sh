#!/bin/sh
# $Id: edit_cfg.sh,v 1.7 1997/11/08 18:14:57 tom Exp $
################################################################################
# Copyright 1996,1997 by Thomas E. Dickey <dickey@clark.net>                   #
# All Rights Reserved.                                                         #
#                                                                              #
# Permission to use, copy, modify, and distribute this software and its        #
# documentation for any purpose and without fee is hereby granted, provided    #
# that the above copyright notice appear in all copies and that both that      #
# copyright notice and this permission notice appear in supporting             #
# documentation, and that the name of the above listed copyright holder(s) not #
# be used in advertising or publicity pertaining to distribution of the        #
# software without specific, written prior permission. THE ABOVE LISTED        #
# COPYRIGHT HOLDER(S) DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,    #
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT #
# SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY SPECIAL,        #
# INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM   #
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE   #
# OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR    #
# PERFORMANCE OF THIS SOFTWARE.                                                #
################################################################################
# Edit the default value of the term.h file based on the autoconf-generated
# values:
#
#	$1 = ncurses_cfg.h
#	$2 = term.h
#
TMP=edit$$
trap "rm -f $TMP" 0 1 2 5 15
for name in \
	HAVE_TCGETATTR \
	HAVE_TERMIOS_H \
	HAVE_TERMIO_H \
	NCURSES_CONST \
	BROKEN_LINKER
do
	mv $2 $2.bak
	if ( grep "[ 	]$name[ 	]" $1 2>&1 >$TMP )
	then
		sed -e 's@#define '$name'.*$@'"`cat $TMP`@" $2.bak >$2
	else
		sed -e 's/define '$name'.*$/define '$name' 0/' $2.bak >$2
	fi
	if (cmp -s $2 $2.bak)
	then
		echo '** same: '$name
		mv $2.bak $2
	else
		echo '** edit: '$name
		rm -f $2.bak
	fi
done
