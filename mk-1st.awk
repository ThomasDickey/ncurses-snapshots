# Generate list of objects for a given model library
# Variables:
#	name (library name, e.g., "ncurses", "panel", "forms", "menus")
#	model (directory into which we compile, e.g., "obj")
#	suffix (e.g., "_g.a", for debug libraries)
#	MODEL (e.g., "DEBUG", uppercase; toupper is not portable)
BEGIN	{
		print  ""
		found = 0;
	}
	{
		if ( $1 != "#" && ( $2 == "lib" || $2 == "progs" ))
		{
			if ( found == 0 )
			{
				printf "%s_OBJS =", MODEL
				if ( $2 == "lib" )
					found = 1
				else
					found = 2
			}
			printf " \\\n\t../%s/%s.o", model, $1
		}
	}
END	{
		print  ""
		if ( found == 1 )
		{
			print  ""
			if ( MODEL == "SHARED" )
			{
				printf "../lib/lib%s%s : $(%s_OBJS)\n", name, suffix, MODEL
				print  "\t@-rm -f $@"
				printf "\t$(MK_SHARED_LIB) $(%s_OBJS)\n", MODEL
			}
			else
			{
				printf "../lib/lib%s%s : $(%s_OBJS)\n", name, suffix, MODEL
				printf "\tar rv $@ $?\n"
				printf "\t$(RANLIB) $@\n"
			}
			print ""
			print "clean ::"
			printf "\trm -f ../lib/lib%s%s\n", name, suffix
			printf "\trm -f $(%s_OBJS)\n", MODEL
		}
		else if ( found == 2 )
		{
			print ""
			print "clean ::"
			printf "\trm -f $(%s_OBJS)\n", MODEL
		}
	}
