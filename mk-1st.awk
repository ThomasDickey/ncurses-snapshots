# Generate list of objects for a given model library
# Variables:
#	model
#	suffix
#	MODEL (uppercase version of 'model'; toupper is not portable)
BEGIN	{
	print  ""
	printf "%s_OBJS =", MODEL
	}
	{
	if ( $2 == "ncurses" )
		printf " \\\n\t%s/%s.o", model, $1
	}
END	{
	print  ""
	print  ""
	if ( model == "shared" ) {
		printf "libncurses%s : $(%s_OBJS)\n", suffix, MODEL
		print  "\t@-rm -f $@"
		printf "\t$(MK_SHARED_LIB) $(%s_OBJS)\n", MODEL
		print  ""
		printf "libpanel%s : %s/panel.o\n", suffix, model
		print  "\t@-rm -f $@"
		printf "\t$(MK_SHARED_LIB) $(%s_OBJS)\n", MODEL
	} else	{
		printf "libncurses%s : $(%s_OBJS)\n", suffix, MODEL
		printf "\tar rv $@ $?\n"
		printf "\t$(RANLIB) $@\n"
		print  ""
		printf "libpanel%s : %s/panel.o\n", suffix, model
		printf "\tar rv $@ $?\n"
		printf "\t$(RANLIB) $@\n"
		}
	}
