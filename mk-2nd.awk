# Generate compile-rules for the modules that we're using in libraries or
# programs.  We're listing them explicitly because we've turned off the suffix
# rules (to force compilation with the appropriate flags).  We could use
# make-recursion but that would result in makefiles that are useless for
# development.
#
# Variables:
#	model
#	MODEL (uppercase version of 'model'; toupper is not portable)
#	echo (yes iff we'll show the $(CC) lines)
#
# Fields in src/modules:
#	$1 = module name
#	$2 = progs|ncurses|panel
#	$3 = source-directory
#
# Fields in src/modules past $3 are dependencies
#
BEGIN	{
	print  ""
	}
	{
	if ( $1 != "#" && $1 != "" ) {
		print  ""
		printf "%s/%s.o :\t%s/%s.c", model, $1, $3, $1
		for (n = 4; n <= NF; n++) printf " \\\n\t\t\t%s", $n
		print  ""
		if ( $3 == "." || srcdir == "." )
			source=".."
		else
			source=$3
		if ( echo == "yes" )
			atsign=""
		else {
			atsign="@"
			printf "\t@echo 'compiling %s (%s)'\n", $1, model
		}
		printf "\t%scd %s; $(CC) $(CFLAGS_%s) -c %s/%s.c", atsign, model, MODEL, source, $1
	} else {
		printf "%s", $1
		for (n = 2; n <= NF; n++) printf " %s", $n
	}
	print  ""
	}
END	{
	print  ""
	}
