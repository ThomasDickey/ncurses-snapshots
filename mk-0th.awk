# $Id: mk-0th.awk,v 1.1 1996/06/01 19:51:23 tom Exp $
# Generate list of sources for a library, together with lint/lintlib rules
#
# Variables:
#	name (library name, e.g., "ncurses", "panel", "forms", "menus")
#
BEGIN	{
		print  ""
		found = 0;
	}
	{
		if ( $1 != "#" && $2 == "lib" )
		{
			if ( found == 0 )
			{
				printf "C_SRC ="
				found = 1
			}
			printf " \\\n\t%s/%s.c", $3, $1
		}
	}
END	{
		print  ""
		if ( found != 0 )
		{
			print  ""
			printf "# Producing llib-l%s is time-consuming, so there's no direct-dependency for\n", name
			print  "# it in the lintlib rule.  We'll only remove in the cleanest setup."
			print  "clean ::"
			printf "\trm -f llib-l%s.*\n", name
			print  ""
			print  "realclean ::"
			printf "\trm -f llib-l%s\n", name
			print  ""
			printf "llib-l%s : $(C_SRC)\n", name
			printf "\tcproto -a -l -DLINT $(CPPFLAGS) $(C_SRC) >$@\n"
			print  ""
			print  "lintlib :"
			printf "\t$(srcdir)/../misc/makellib %s $(CPPFLAGS)", name
			print ""
			print "lint :"
			print "\t$(LINT) $(LINT_OPTS) $(CPPFLAGS) $(C_SRC) $(LINT_LIBS)"
		}
		else
		{
			print  "lintlib :"
			print  "\t@echo no action needed"
		}
	}
