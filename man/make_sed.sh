#!/bin/sh
# $Id: make_sed.sh,v 1.4 1997/12/06 22:24:45 tom Exp $
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
# Construct a sed-script to perform renaming within man-pages.  Originally
# written in much simpler form, this one accounts for the common cases of
# section-names in man-pages.

if test $# != 1 ; then
	echo '? expected a single filename'
	exit 1
fi

COL=col$$
INPUT=input$$
UPPER=upper$$
SCRIPT=script$$
RESULT=result$$
rm -f $UPPER $SCRIPT $RESULT
trap "rm -f $COL.* $INPUT $UPPER $SCRIPT $RESULT" 0 1 2 5 15
fgrep -v \# $1 | \
sed	-e 's/[	]\+/	/g' >$INPUT

for F in 1 2 3 4
do
sed	-e 's/\./	/g' $INPUT | \
cut	-f $F > $COL.$F
done
for F in 2 4
do
	tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ <$COL.$F >$UPPER
	mv $UPPER $COL.$F 
done
paste $COL.* | \
sed	-e 's/^/s\/\\</' \
	-e 's/$/\//' >$UPPER

# Do the TH lines
sed	-e 's/\//\/TH /' \
	-e 's/	/ /' \
	-e 's/	/ ""\/TH /' \
	-e 's/	/ /' \
	-e 's/\/$/ ""\//' \
	$UPPER >>$RESULT

# Do the embedded references
sed	-e 's/</<fB/' \
	-e 's/	/\\\\fR(/' \
	-e 's/	/)\/fB/' \
	-e 's/	/\\\\fR(/' \
	-e 's/\/$/)\//' \
	$UPPER >>$RESULT

# Finally, send the result to standard output
cat $RESULT
