# Generate list of objects for a given model library
# Variables:
#	name (library name, e.g., "ncurses", "panel", "forms", "menus")
#	model (directory into which we compile, e.g., "obj")
#	suffix (e.g., "_g.a", for debug libraries)
#	MODEL (e.g., "DEBUG", uppercase; toupper is not portable)
#	DoLinks ("yes" or "no", flag to add symbolic links)
#	rmSoLocs ("yes" or "no", flag to add extra clean target)
#	overwrite ("yes" or "no", flag to add link to libcurses.a
#
# Notes:
#	CLIXs nawk does not like underscores in command-line variable names.
#	Mixed-case is ok.
#	HP/UX requires shared libraries to have executable permissions.
#
function symlink(src,dst) {
		if ( src != dst ) {
			printf "rm -f %s; ", dst
			printf "$(LN_S) %s %s; ", src, dst
		}
	}
function sharedlinks(directory) {
		if ( end_name != lib_name ) {
			printf "\t(cd %s ;", directory
			abi_name = sprintf("%s.$(ABI_VERSION)", lib_name);
			symlink(end_name, abi_name);
			symlink(abi_name, lib_name);
			printf ")\n"
		}
	}
function removelinks() {
		if ( end_name != lib_name ) {
			printf "\trm -f ../lib/%s ../lib/%s\n", abi_name, end_name
		}
	}
function installed_name() {
		if ( DO_LINKS == "yes" ) {
			return sprintf("%s.$(REL_VERSION)", lib_name);
		} else {
			return lib_name;
		}
	}
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
			lib_name = sprintf("lib%s%s", name, suffix)
			if ( MODEL == "SHARED" )
			{
				if ( DoLinks == "yes" ) {
					end_name = sprintf("%s.$(REL_VERSION)", lib_name);
				} else {
					end_name = lib_name;
				}
				printf "../lib/%s : $(%s_OBJS)\n", lib_name, MODEL
				print  "\t@-rm -f $@"
				printf "\t$(MK_SHARED_LIB) $(%s_OBJS)\n", MODEL
				sharedlinks("../lib")
				print  ""
				print  "install.libs \\"
				printf "install.%s :: $(libdir) ../lib/%s\n", name, end_name
				printf "\t@echo installing ../lib/%s as $(libdir)/%s \n", lib_name, end_name
				printf "\t$(INSTALL) ../lib/%s $(libdir)/%s \n", lib_name, end_name
				sharedlinks("$(libdir)")
				if ( rmSoLocs == "yes" ) {
					print  ""
					print  "clean ::"
					printf "\t@-rm -f so_locations\n"
				}
			}
			else
			{
				end_name = lib_name;
				printf "../lib/%s : $(%s_OBJS)\n", lib_name, MODEL
				printf "\tar rv $@ $?\n"
				printf "\t$(RANLIB) $@\n"
				print  ""
				print  "install.libs \\"
				printf "install.%s :: $(libdir) ../lib/%s\n", name, lib_name
				printf "\t@echo installing ../lib/%s as $(libdir)/%s \n", lib_name, lib_name
				printf "\t$(INSTALL_DATA) ../lib/%s $(libdir)/%s \n", lib_name, lib_name
				if ( overwrite == "yes" && lib_name == "libncurses.a" )
				{
					printf "\t@echo linking libcurses.a to libncurses.a \n"
					printf "\trm -f $(libdir)/libcurses.a \n"
					printf "\t$(LN_S) $(libdir)/libncurses.a $(libdir)/libcurses.a \n"
				}
				printf "\t$(RANLIB) $(libdir)/%s\n", lib_name
			}
			print ""
			print "clean ::"
			printf "\trm -f ../lib/%s\n", lib_name
			printf "\trm -f $(%s_OBJS)\n", MODEL
			removelinks();
		}
		else if ( found == 2 )
		{
			print ""
			print "clean ::"
			printf "\trm -f $(%s_OBJS)\n", MODEL
		}
	}
