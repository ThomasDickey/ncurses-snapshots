#!/bin/sh
# Edit the default value of the term.h file based on the autoconf-generated
# values:
#
#	$1 = config.h
#	$2 = term.h
#
for name in \
	HAVE_TCGETATTR \
	HAVE_TERMIOS_H \
	BROKEN_LINKER
do
	mv $2 $2.bak
	if ( grep "[ 	]$name[ 	]" $1 2>&1 >/dev/null )
	then
		sed -e 's/define '$name'.*$/define '$name' 1/' $2.bak >$2
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
