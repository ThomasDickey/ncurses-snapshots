#!/bin/sh
# $Id: tar-copy.sh,v 1.1 1997/10/18 17:56:33 tom Exp $
################################################################################
# Copyright 1997 by Thomas E. Dickey <dickey@clark.net>                        #
# All Rights Reserved.                                                         #
#                                                                              #
# Permission to use, copy, modify, and distribute this software and its        #
# documentation for any purpose and without fee is hereby granted, provided    #
# that the above copyright notice appear in all copies and that both that      #
# copyright notice and this permission notice appear in supporting             #
# documentation, and that the name of the above listed copyright holder(s) not #
# be used in advertising or publicity pertaining to distribution of the        #
# software without specific, written prior permission.                         #
#                                                                              #
# THE ABOVE LISTED COPYRIGHT HOLDER(S) DISCLAIM ALL WARRANTIES WITH REGARD TO  #
# THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND       #
# FITNESS, IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE    #
# FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER #
# RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF         #
# CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN          #
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.                     #
################################################################################
# Copy a collection of files using 'tar', so that their dates and links are
# preserved
#
# Parameters:
#	$1 = files to copy
#	$2 = source directory
#	$3 = destination directory
#
#DOIT=echo
DOIT=eval

if test $# != 3 ; then
	echo "Usage: $0 files source target"
	exit 1
elif test ! -d "$2" ; then
	echo "Source directory not found: $2"
	exit 1
elif test ! -d "$3" ; then
	echo "Target directory not found: $3"
	exit 1
fi

WD=`pwd`

TMP=$WD/copy$$
trap "rm -f $TMP" 0 1 2 5 15

cd $2
if ( tar cf $TMP $1 )
then
	cd $3
	$DOIT "rm -rf `tar tf $TMP`" 2>/dev/null
	$DOIT "tar xf $TMP"
else
	echo "Cannot create tar of $1 files"
	exit 1
fi
