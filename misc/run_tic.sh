#!/bin/sh
# $Id: run_tic.sh,v 1.1 1996/05/26 00:54:55 tom Exp $
# This script is used to install terminfo.src using tic.  We use a script
# because the path checking is too awkward to do in a makefile.
#
# Parameters:
#	$1 = nominal directory in which to find 'tic', i.e., $(bindir).
#	$2 = source-directory, i.e., $(srcdir)
#	$3 = destination-directory path, i.e., $(ticdir)
#
echo '** Building terminfo database, please wait...'

prefix=/usr/local
if test $# != 0 ; then
	bindir=$1
	shift
	PREFIX=`echo $bindir | sed -e 's/\/bin$//'`
	test -n "$PREFIX" && test "x$PREFIX" != "x$bindir" && prefix=$PREFIX
else
	bindir=$prefix/bin
fi

if test $# != 0 ; then
	srcdir=$1
	shift
else
	srcdir=.
fi

if test $# != 0 ; then
	ticdir=$1
else
	ticdir=$prefix/share/terminfo
fi

# Allow tic to run either from the install-path, or from the build-directory
PATH=$bindir:$srcdir/../progs:$PATH ; export PATH
TERMINFO=$ticdir ; export TERMINFO
umask 022

TICDIR=`echo $ticdir | sed -e 's/\/share\//\/lib\//'`

# Remove the old terminfo stuff; we don't care if it existed before
( rm -fr $ticdir/[0-9A-Za-z] 2>/dev/null )

# If we're not installing into /usr/share/, we'll have to adjust the location
# of the tabset files in terminfo.src (which are in a parallel directory).
TABSET=`echo $ticdir | sed -e 's/\/terminfo$/\/tabset/'`
SRC=$srcdir/terminfo.src
if test "x$TABSET" != "x/usr/share/tabset" ; then
	echo '** adjusting tabset paths'
	TMP=/usr/tmp/$$
	sed -e s:/usr/share/tabset:$TABSET:g $SRC >$TMP
	trap "rm -f $TMP" 0 1 2 5 15
	SRC=$TMP
fi

if ( tic $SRC )
then
	echo '** built new '$ticdir
else
	echo '? tic could not build '$ticdir
	exit 1
fi

# Make a symbolic link to provide compatibility with applications that expect
# to find terminfo under /usr/lib.  That is, we'll _try_ to do that.  Not
# all systems support symbolic links, and those that do provide a variety
# of options for 'test'.
if test "$TICDIR" != "$ticdir" ; then
	if ( cd $TICDIR 2>/dev/null )
	then
		cd $TICDIR
		TICDIR=`pwd`
		if test $TICDIR != $ticdir ; then
			# Well, we tried.  Some systems lie to us, so the
			# installer will have to double-check.
			echo "Verify if $TICDIR and $ticdir are the same."
			echo "The new terminfo is in $ticdir; the other should be a link to it."
		fi
	else
		( rm -f $TICDIR 2>/dev/null )
		if ( ln -s $ticdir $TICDIR )
		then
			echo '** linked '$TICDIR' for compatibility'
		fi
	fi
fi
