# Generate list of objects for a given model library
# Variables:
#	name (library name, e.g., "ncurses", "panel", "forms", "menus")
#	model (directory into which we compile, e.g., "obj")
#	suffix (e.g., "_g.a", for debug libraries)
#	MODEL (e.g., "DEBUG", uppercase; toupper is not portable)
#
function symlink(src,dst) {
		if ( src != dst ) {
			printf "rm -f %s; ", dst
			printf "$(LN_S) %s %s; ", src, dst
		}
	}
function sharedlinks(directory) {
		if ( end_name != lib_name ) {
			printf "\tcd %s && (", directory
			abi_name = sprintf("%s.%s", lib_name, ABI_VERSION);
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
		if ( SYSTYPE == "Linux" || SYSTYPE == "SunOS" ) {
			return sprintf("%s.%s", lib_name, REL_VERSION);
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
				end_name = installed_name();
				printf "../lib/%s : $(%s_OBJS)\n", lib_name, MODEL
				print  "\t@-rm -f $@"
				printf "\t$(MK_SHARED_LIB) $(%s_OBJS)\n", MODEL
				sharedlinks("../lib")
				print  ""
				printf "install.libs :: $(libdir) ../lib/%s\n", end_name
				printf "\t@echo installing ../lib/%s as $(libdir)/%s \n", lib_name, end_name
				printf "\t$(INSTALL_DATA) ../lib/%s $(libdir)/%s \n", lib_name, end_name
				sharedlinks("$(libdir)")
				if ( SYSTYPE == "IRIX" ) {
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
				printf "install.libs :: $(libdir) ../lib/%s\n", lib_name
				printf "\t@echo installing ../lib/%s as $(libdir)/%s \n", lib_name, lib_name
				printf "\t$(INSTALL_DATA) ../lib/%s $(libdir)/%s \n", lib_name, lib_name
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
