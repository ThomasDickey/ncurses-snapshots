#!/bin/sh
# $Id: listused.sh,v 1.3 2003/03/29 19:32:58 tom Exp $
# A very simple script to list entrypoints that are used by either a test
# program, or within the libraries.  This relies on the output format of 'nm',
# and assumes that the libraries are configured using
#	--disable-macros
#	--enable-widec
# Static libraries are used, to provide some filtering based on internal usage
# of the different symbols.

NM_OPTS=

if test ! -d ../objects ; then
	echo "? need objects to run this script"
	exit 1
elif test ! -d ../lib ; then
	echo "? need libraries to run this script"
	exit 1
fi

PROGS=
for name in `(echo "test:";sort modules; echo "progs:";sort ../progs/modules) |sed -e 's/[ 	].*//' -e '/^[#@]/d'`
do
	case $name in
	*:)
		PROGS="$PROGS $name"
		;;
	*)
		NAME=../objects/${name}.o
		if test -f $NAME
		then
			PROGS="$PROGS $NAME"
		fi
		;;
	esac
done

# For each library -
for lib in ../lib/*.a
do
	LIB=`basename $lib .a`
	case $LIB in
	*_*|*+*)
		continue
		;;
	esac

	tmp=`echo $LIB|sed -e 's/w$//'`
	echo
	echo "${tmp}:"
	echo $tmp |sed -e 's/./-/g'
	# Construct a list of public externals provided by the library.
	WANT=`nm $NM_OPTS $lib |\
		sed	-e 's/^[^ ]*//' \
			-e 's/^ *//' \
			-e '/^[ a-z] /d' \
			-e '/:$/d' \
			-e '/^$/d' \
			-e '/^U /d' \
			-e 's/^[A-Z] //' \
			-e '/^_/d' |\
		sort -u`
	# List programs which use that external.
	for name in $WANT
	do
		HAVE=
		tags=
		last=
		for prog in $PROGS
		do
			case $prog in
			*:)
				tags=$prog
				;;
			*)
				TEST=`nm $NM_OPTS $prog |\
					sed	-e 's/^[^ ]*//' \
						-e 's/^ *//' \
						-e '/^[ a-z] /d' \
						-e '/:$/d' \
						-e '/^$/d' \
						-e 's/^[A-Z] //' \
						-e '/^_/d' \
						-e 's/^'${name}'$/_/' \
						-e '/^[^_]/d'`
				if test -n "$TEST"
				then
					have=`basename $prog .o`
					if test -n "$HAVE"
					then
						if test "$last" = "$tags"
						then
							HAVE="$HAVE $have"
						else
							HAVE="$HAVE $tags $have"
						fi
					else
						HAVE="$tags $have"
					fi
					last="$tags"
				fi
				;;
			esac
		done
		# if we did not find a program using it directly, see if it
		# is used within a library.
		if test -z "$HAVE"
		then
			for tmp in ../lib/*.a
			do 
				case $tmp in
				*_*|*+*)
					continue
					;;
				esac
				TEST=`nm $NM_OPTS $tmp |\
					sed	-e 's/^[^ ]*//' \
						-e 's/^ *//' \
						-e '/^[ a-z] /d' \
						-e '/:$/d' \
						-e '/^$/d' \
						-e '/^[A-TV-Z] /d' \
						-e 's/^[A-Z] //' \
						-e '/^_/d' \
						-e 's/^'${name}'$/_/' \
						-e '/^[^_]/d'`
				if test -n "$TEST"
				then
					tmp=`basename $tmp .a |sed -e 's/w$//'`
					HAVE=`echo $tmp | sed -e 's/lib/lib: /'`
					break
				fi
			done
		fi
		test -z "$HAVE" && HAVE="-"
		lenn=`expr 39 - length $name`
		lenn=`expr $lenn / 8`
		tabs=
		while test $lenn != 0
		do
			tabs="${tabs}	"
			lenn=`expr $lenn - 1`
		done
		echo "${name}${tabs}${HAVE}"
	done
done
