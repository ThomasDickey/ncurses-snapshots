# Generate compile-rules for the modules that we are using in libraries or
# programs.  We are listing them explicitly because we have turned off the
# suffix rules (to force compilation with the appropriate flags).  We could use
# make-recursion but that would result in makefiles that are useless for
# development.
#
# Variables:
#	model
#	MODEL (uppercase version of "model"; toupper is not portable)
#	echo (yes iff we will show the $(CC) lines)
#
# Fields in src/modules:
#	$1 = module name
#	$2 = progs|lib|c++
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
		if ( $2 == "c++" )
			suffix=".cc"
		else
			suffix=".c"
		printf "../%s/%s.o :\t%s/%s%s", model, $1, $3, $1, suffix
		for (n = 4; n <= NF; n++) printf " \\\n\t\t\t%s", $n
		print  ""
		if ( echo == "yes" )
			atsign=""
		else {
			atsign="@"
			printf "\t@echo 'compiling %s (%s)'\n", $1, model
		}
		if ( $3 == "." || srcdir == "." )
			printf "\t%scd ../%s; $(CC) $(CFLAGS_%s) -c ../%s/%s%s", atsign, model, MODEL, name, $1, suffix
		else
			printf "\t%scd ../%s; $(CC) $(CFLAGS_%s) -c %s/%s%s", atsign, model, MODEL, $3, $1, suffix
	} else {
		printf "%s", $1
		for (n = 2; n <= NF; n++) printf " %s", $n
	}
	print  ""
	}
END	{
	print  ""
	}
