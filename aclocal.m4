dnl Macros used in NCURSES auto-configuration script.
dnl
dnl ---------------------------------------------------------------------------
dnl Test if 'bool' is a builtin type in the configured C++ compiler.  Some
dnl older compilers (e.g., gcc 2.5.8) don't support 'bool' directly; gcc
dnl 2.6.3 does, in anticipation of the ANSI C++ standard.
dnl
dnl Treat the configuration-variable specially here, since we're directly
dnl substituting its value (i.e., 1/0).
AC_DEFUN([NC_BOOL_DECL],
[
AC_MSG_CHECKING([for builtin c++ bool type])
AC_CACHE_VAL(nc_cv_builtin_bool,[
	AC_TRY_COMPILE([],[bool x = false],
		[nc_cv_builtin_bool=1],
		[nc_cv_builtin_bool=0])
	])
if test $nc_cv_builtin_bool = 1
then	AC_MSG_RESULT(yes)
else	AC_MSG_RESULT(no)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl Test for the size of 'bool' in the configured C++ compiler.
AC_DEFUN([NC_BOOL_SIZE],
[
AC_MSG_CHECKING([for size of c++ bool])
AC_CACHE_VAL(nc_cv_sizeof_bool,[
	rm -f nc_test.out
	AC_TRY_RUN([
#include <stdio.h>
#include <builtin.h>
main()
{
	FILE *fp = fopen("nc_test.out", "w");
	if (fp != 0) {
		bool x = false;
		if (sizeof(x) == sizeof(int))       fputs("int",  fp);
		else if (sizeof(x) == sizeof(char)) fputs("char", fp);
		else if (sizeof(x) == sizeof(long)) fputs("long", fp);
		fclose(fp);
	}
	exit(0);
}
		],
		[nc_cv_sizeof_bool=`cat nc_test.out`],
		[nc_cv_sizeof_bool=unsigned],
		[nc_cv_sizeof_bool=unknown])
	])
	rm -f nc_test.out
AC_MSG_RESULT($nc_cv_sizeof_bool)
])dnl
dnl ---------------------------------------------------------------------------
dnl If we're trying to use g++, test if libg++ is installed (a rather common
dnl problem :-).  If we have the compiler but no library, we'll be able to
dnl configure, but won't be able to build the c++ demo program.
AC_DEFUN([NC_CXX_LIBRARY],
[
nc_cxx_library=unknown
if test $ac_cv_prog_gxx = yes; then
	AC_MSG_CHECKING([for libg++])
	nc_save="$LIBS"
	LIBS="$LIBS -lg++ -lm"
	AC_TRY_LINK([
#include <builtin.h>
	],
	[float foo=abs(1.0)],
	[nc_cxx_library=yes],
	[nc_cxx_library=no])
	LIBS="$nc_save"
	AC_MSG_RESULT($nc_cxx_library)
fi
])dnl
dnl ---------------------------------------------------------------------------
AC_DEFUN([NC_DIRS_TO_MAKE],
[
DIRS_TO_MAKE="lib"
for nc_item in $nc_list_models
do
	NC_OBJ_SUBDIR($nc_item,nc_subdir)
	DIRS_TO_MAKE="$DIRS_TO_MAKE $nc_subdir"
done
for nc_dir in $DIRS_TO_MAKE
do
	test ! -d $nc_dir && mkdir $nc_dir
done
AC_SUBST(DIRS_TO_MAKE)
])dnl
dnl ---------------------------------------------------------------------------
dnl
AC_DEFUN([NC_ERRNO],
[
AC_MSG_CHECKING([for errno external decl])
AC_CACHE_VAL(nc_cv_extern_errno,[
	AC_TRY_COMPILE([
#include <errno.h>],
		[int x = errno],
		[nc_cv_extern_errno=yes],
		[nc_cv_extern_errno=no])
	])
AC_MSG_RESULT($nc_cv_extern_errno)
test $nc_cv_extern_errno = yes && AC_DEFINE(HAVE_EXTERN_ERRNO)
])dnl
dnl ---------------------------------------------------------------------------
dnl Construct the list of include-options according to whether we're building
dnl in the source directory or using '--srcdir=DIR' option.
AC_DEFUN([NC_INCLUDE_DIRS],
[
INCLUDES="-I. -I../include"
if test "$srcdir" != "."; then
	INCLUDES="$INCLUDES -I\$(srcdir)/../include"
fi
INCLUDES="$INCLUDES -I\$(includedir)"
AC_SUBST(INCLUDES)
])dnl
dnl ---------------------------------------------------------------------------
dnl Append definitions and rules for the given models to the subdirectory
dnl Makefiles, and the recursion rule for the top-level Makefile.  If the
dnl subdirectory is a library-source directory, modify the LIBRARIES list in
dnl the corresponding makefile to list the models that we'll generate.
dnl
dnl For shared libraries, make a list of symbolic links to construct when
dnl generating each library.  The convention used for Linux is the simplest
dnl one:
dnl	lib<name>.so	->
dnl	lib<name>.so.<major>	->
dnl	lib<name>.so.<maj>.<minor>
AC_DEFUN([NC_LIB_RULES],
[
AC_REQUIRE([NC_SYSTYPE])
AC_REQUIRE([NC_VERSION])
for nc_dir in $SRC_SUBDIRS
do
	if test -f $srcdir/$nc_dir/modules; then

		nc_libs_to_make=
		for nc_item in $NC_LIST_MODELS
		do
			NC_LIB_SUFFIX($nc_item,nc_suffix)
			nc_libs_to_make="$nc_libs_to_make ../lib/lib${nc_dir}${nc_suffix}"
		done

		sed -e "s@\@LIBS_TO_MAKE\@@$nc_libs_to_make@" \
			$nc_dir/Makefile >$nc_dir/Makefile.out
		mv $nc_dir/Makefile.out $nc_dir/Makefile

		for nc_item in $NC_LIST_MODELS
		do
			echo 'Appending rules for '$nc_item' model ('$nc_dir')'
			NC_UPPERCASE($nc_item,NC_ITEM)
			NC_LIB_SUFFIX($nc_item,nc_suffix)
			NC_OBJ_SUBDIR($nc_item,nc_subdir)
			$AWK -f $srcdir/mk-1st.awk \
				name=$nc_dir \
				MODEL=$NC_ITEM \
				model=$nc_subdir \
				suffix=$nc_suffix \
				DoLinks=$nc_cv_do_symlinks \
				rmSoLocs=$nc_cv_rm_so_locs \
				$srcdir/$nc_dir/modules >>$nc_dir/Makefile
			$AWK -f $srcdir/mk-2nd.awk \
				name=$nc_dir \
				MODEL=$NC_ITEM \
				model=$nc_subdir \
				srcdir=$srcdir \
				echo=$WITH_ECHO \
				$srcdir/$nc_dir/modules >>$nc_dir/Makefile
		done
	fi
	echo '	cd '$nc_dir'; $(MAKE) $(NC_MFLAGS) [$]@' >>Makefile
done

for nc_dir in include $SRC_SUBDIRS
do
	if test -f $srcdir/$nc_dir/modules; then
cat >> Makefile <<NC_EOF

install.libs ::
	cd $nc_dir; \$(MAKE) \$(NC_MFLAGS) \[$]@
NC_EOF
fi
done

cat >> Makefile <<NC_EOF

distclean ::
	rm -f config.cache config.log config.status Makefile include/config.h
	rm -rf \$(DIRS_TO_MAKE)
NC_EOF

])dnl
dnl ---------------------------------------------------------------------------
dnl Compute the library-suffix from the given model name
AC_DEFUN([NC_LIB_SUFFIX],
[
	AC_REQUIRE([NC_SYSTYPE])
	AC_REQUIRE([NC_VERSION])
	case $1 in
	normal)  $2='.a'   ;;
	debug)   $2='_g.a' ;;
	profile) $2='_p.a' ;;
	shared)  $2='.so'  ;;
	esac
])dnl
dnl ---------------------------------------------------------------------------
dnl Compute the string to append to -library from the given model name
AC_DEFUN([NC_LIB_TYPE],
[
	case $1 in
	normal)  $2=''   ;;
	debug)   $2='_g' ;;
	profile) $2='_p' ;;
	shared)  $2=''   ;;
	esac
])dnl
dnl ---------------------------------------------------------------------------
dnl Some systems have a non-ANSI linker that doesn't pull in modules that have
dnl only data (i.e., no functions), for example NeXT.  On those systems we'll
dnl have to provide wrappers for global tables to ensure they're linked
dnl properly.
AC_DEFUN([NC_LINK_DATAONLY],
[
AC_MSG_CHECKING([if data-only library module links])
AC_CACHE_VAL(nc_cv_link_dataonly,[
	rm -f conftest.a
	changequote(,)dnl
	cat >conftest.$ac_ext <<EOF
#line __oline__ "configure"
int	testdata[3] = { 123, 456, 789 };
EOF
	changequote([,])dnl
	if eval $ac_compile; then
		mv conftest.o data.o
		ar cr conftest.a data.o
	fi
	rm -f conftest.$ac_ext data.o
	changequote(,)dnl
	cat >conftest.$ac_ext <<EOF
#line __oline__ "configure"
int	testfunc()
{
#if defined(NeXT)
	exit(1);	/* I'm told this linker is broken */
#else
	extern int testdata[3];
	return testdata[0] == 123
	   &&  testdata[1] == 456
	   &&  testdata[2] == 789;
#endif
}
EOF
	changequote([,])dnl
	if eval $ac_compile; then
		mv conftest.o func.o
		ar cr conftest.a func.o
	fi
	rm -f conftest.$ac_ext func.o
	eval $ac_cv_prog_RANLIB conftest.a 2>&1 >/dev/null
	nc_saveLIBS="$LIBS"
	LIBS="conftest.a $LIBS"
	AC_TRY_RUN([
	int main()
	{
		extern int testfunc();
		exit (!testfunc());
	}
	],
	[nc_cv_link_dataonly=yes],
	[nc_cv_link_dataonly=no],
	[nc_cv_link_dataonly=unknown])
	LIBS="$nc_saveLIBS"
	])
AC_MSG_RESULT($nc_cv_link_dataonly)
test $nc_cv_link_dataonly = no && AC_DEFINE(BROKEN_LINKER)
])dnl
dnl ---------------------------------------------------------------------------
dnl Some 'make' programs support $(MAKEFLAGS), some $(MFLAGS), to pass 'make'
dnl options to lower-levels.  It's very useful for "make -n" -- if we have it.
dnl (GNU 'make' does both :-)
AC_DEFUN([NC_MAKEFLAGS],
[
AC_MSG_CHECKING([for makeflags variable])
AC_CACHE_VAL(nc_cv_makeflags,[
	nc_cv_makeflags=''
	for nc_option in '$(MFLAGS)' '-$(MAKEFLAGS)'
	do
		cat >ncurses.tmp <<NC_EOF
all :
	echo '.$nc_option'
NC_EOF
		set nc_result=`${MAKE-make} -f ncurses.tmp 2>/dev/null`
		if test "$nc_result" != "."
		then
			nc_cv_makeflags=$nc_option
			break
		fi
	done
	rm -f ncurses.tmp
	])
AC_MSG_RESULT($nc_cv_makeflags)
AC_SUBST(nc_cv_makeflags)
])dnl
dnl ---------------------------------------------------------------------------
dnl Compute the object-directory name from the given model name
AC_DEFUN([NC_OBJ_SUBDIR],
[
	case $1 in
	normal)  $2='objects' ;;
	debug)   $2='obj_g' ;;
	profile) $2='obj_p' ;;
	shared)  $2='obj_s' ;;
	esac
])dnl
dnl ---------------------------------------------------------------------------
dnl Attempt to determine the appropriate CC/LD options for creating a shared
dnl library.
dnl
dnl Note: $(LOCAL_LDFLAGS) is used to link executables that will run within the 
dnl build-tree, i.e., by making use of the libraries that are compiled in ../lib
dnl We avoid compiling-in a ../lib path for the shared library since that can
dnl lead to unexpected results at runtime.
dnl
dnl The variable 'nc_cv_do_symlinks' is used to control whether we configure
dnl to install symbolic links to the rel/abi versions of shared libraries.
dnl
dnl Some loaders leave 'so_locations' lying around.  It's nice to clean up.
AC_DEFUN([NC_SHARED_OPTS],
[
	AC_REQUIRE([NC_SYSTYPE])
	AC_REQUIRE([NC_VERSION])
 	LOCAL_LDFLAGS=

	nc_cv_do_symlinks=no
	nc_cv_rm_so_locs=no

	case $nc_cv_systype in
	HP_UX)
		# (untested: gcc 2.5.8 doesn't do PIC, and I don't have c89)
		if test "${CC}" = "gcc"; then
			CC_SHARED_OPTS='-fPIC'
		else
			CC_SHARED_OPTS='+Z'
		fi
		MK_SHARED_LIB='ld -b -o $[@]'
		;;
	IRIX)
		# tested with IRIX 5.2 and 'cc'.
		if test "${CC}" = "gcc"; then
			CC_SHARED_OPTS='-fPIC'
		else
			CC_SHARED_OPTS='-KPIC'
		fi
		MK_SHARED_LIB='ld -shared -rdata_shared -soname `basename $[@]` -o $[@]'
		nc_cv_rm_so_locs=yes
		;;
	Linux)
		# tested with Linux 1.2.8 and gcc 2.7.0 (ELF)
		CC_SHARED_OPTS='-fPIC'
 		MK_SHARED_LIB="gcc -o \$[@].\$(REL_VERSION) -shared -Wl,-soname,\`basename \$[@].\$(ABI_VERSION)\`,-stats"
		if test $DFT_LWR_MODEL = "shared" ; then
 			LOCAL_LDFLAGS='-Wl,-rpath,../lib'
		fi
		nc_cv_do_symlinks=yes
		;;
	NetBSD)
		CC_SHARED_OPTS='-fpic -DPIC'
		MK_SHARED_LIB='ld -Bshareable -o $[@]'
		;;
	OSF1)
		# tested with OSF/1 V3.2 and 'cc'
		# tested with OSF/1 V3.2 and gcc 2.6.3 (but the c++ demo didn't
		# link with shared libs).
		CC_SHARED_OPTS=''
 		MK_SHARED_LIB="ld -o \$[@].\$(REL_VERSION) -shared -soname \`basename \$[@].\$(ABI_VERSION)\`"
		if test $DFT_LWR_MODEL = "shared" ; then
 			LOCAL_LDFLAGS='-Wl,-rpath,../lib'
		fi
		nc_cv_do_symlinks=yes
		nc_cv_rm_so_locs=yes
		;;
	SunOS)
		# tested with SunOS 4.1.1 and gcc 2.7.0
		# tested with SunOS 5.3 (solaris 2.3) and gcc 2.7.0
		if test $ac_cv_prog_gcc = yes; then
			CC_SHARED_OPTS='-fpic'
		else
			CC_SHARED_OPTS='-pic'
		fi
		case `uname -r` in
		4.*)
			MK_SHARED_LIB="ld -assert pure-text -o \$[@].\$(REL_VERSION)"
			;;
		5.*)
			MK_SHARED_LIB="ld -d y -G -o \$[@].\$(REL_VERSION)"
			;;
		esac
		nc_cv_do_symlinks=yes
		;;
	UNIX_SV)
		# tested with UnixWare 1.1.2
		CC_SHARED_OPTS='-KPIC'
		MK_SHARED_LIB='ld -d y -G -o $[@]'
		;;
	*)
		CC_SHARED_OPTS='unknown'
		MK_SHARED_LIB='echo unknown'
		;;
	esac
	AC_SUBST(CC_SHARED_OPTS)
	AC_SUBST(MK_SHARED_LIB)
	AC_SUBST(LOCAL_LDFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl Check for datatype 'speed_t', which is normally declared via either
dnl sys/types.h or termios.h
AC_DEFUN([NC_SPEED_TYPE],
[
AC_MSG_CHECKING([for speed_t])
AC_CACHE_VAL(nc_cv_type_speed_t,[
	AC_TRY_COMPILE([
#include <sys/types.h>
#if HAVE_TERMIOS_H
#include <termios.h>
#endif],
	[speed_t x = 0],
	[nc_cv_type_speed_t=yes],
	[nc_cv_type_speed_t=no])
	])
AC_MSG_RESULT($nc_cv_type_speed_t)
test $nc_cv_type_speed_t != yes && AC_DEFINE(speed_t,unsigned)
])dnl
dnl ---------------------------------------------------------------------------
dnl For each parameter, test if the source-directory exists, and if it contains
dnl a 'modules' file.  If so, add to the list $nc_cv_src_modules which we'll
dnl use in NC_LIB_RULES.
dnl
dnl This uses the configured value to make the lists SRC_SUBDIRS and
dnl SUB_MAKEFILES which are used in the makefile-generation scheme.
AC_DEFUN([NC_SRC_MODULES],
[
AC_MSG_CHECKING(for src modules)
AC_CACHE_VAL(nc_cv_src_modules,[
nc_cv_src_modules=
for nc_dir in $1
do
	if test -f $srcdir/$nc_dir/modules; then
		if test -z "$nc_cv_src_modules"; then
			nc_cv_src_modules=$nc_dir
		else
			nc_cv_src_modules="$nc_cv_src_modules $nc_dir"
		fi
	fi
done
])
AC_MSG_RESULT($nc_cv_src_modules)

SRC_SUBDIRS="include man"
for nc_dir in $nc_cv_src_modules
do
	SRC_SUBDIRS="$SRC_SUBDIRS $nc_dir"
done
SRC_SUBDIRS="$SRC_SUBDIRS misc test"
test $nc_cxx_library != no && SRC_SUBDIRS="$SRC_SUBDIRS c++"

SUB_MAKEFILES=
for nc_dir in $SRC_SUBDIRS
do
	SUB_MAKEFILES="$SUB_MAKEFILES $nc_dir/Makefile"
done
])dnl
dnl ---------------------------------------------------------------------------
dnl	Check for declarion of sys_errlist in one of stdio.h and errno.h.  
dnl	Declaration of sys_errlist on BSD4.4 interferes with our declaration.
dnl	Reported by Keith Bostic.
AC_DEFUN([NC_SYS_ERRLIST],
[
AC_MSG_CHECKING([declaration of sys_errlist])
AC_CACHE_VAL(nc_cv_dcl_sys_errlist,[
	AC_TRY_COMPILE([
#include <stdio.h>
#include <sys/types.h>
#include <errno.h> ],
	[ char *c = (char *) *sys_errlist; ],
	[nc_cv_dcl_sys_errlist=yes],
	[nc_cv_dcl_sys_errlist=no])
	])
AC_MSG_RESULT($nc_cv_dcl_sys_errlist)
test $nc_cv_dcl_sys_errlist = yes && AC_DEFINE(HAVE_EXTERN_SYS_ERRLIST)
])dnl
dnl ---------------------------------------------------------------------------
dnl Derive the system-type (our main clue to the method of building shared
dnl libraries).
AC_DEFUN([NC_SYSTYPE],
[
AC_CACHE_VAL(nc_cv_systype,[
changequote(,)dnl
nc_cv_systype="`(uname -s || hostname || echo unknown) 2>/dev/null |sed -e s'/[:\/.-]/_/'g  | sed 1q`"
changequote([,])dnl
if test -z "$nc_cv_systype"; then nc_cv_systype=unknown;fi
])
AC_MSG_RESULT(System type is $nc_cv_systype)
])dnl
dnl ---------------------------------------------------------------------------
dnl On some systems ioctl(fd, TIOCGWINSZ, &size) will always return {0,0} until
dnl ioctl(fd, TIOCSWINSZ, &size) is called to explicitly set the size of the
dnl screen.
dnl
dnl Attempt to determine if we're on such a system by running a test-program.
dnl This won't work, of course, if the configure script is run in batch mode,
dnl since we've got to have access to the terminal.
AC_DEFUN([NC_TIOCGWINSZ],
[
AC_MSG_CHECKING([for working TIOCGWINSZ])
AC_CACHE_VAL(nc_cv_use_tiocgwinsz,[
	AC_TRY_RUN([
#if HAVE_TERMIOS_H
#include <termios.h>
#endif
#if !defined(sun) || !defined(HAVE_TERMIOS_H)
#include <sys/ioctl.h>
#endif
int main()
{
	static	struct winsize size;
	int fd;
	for (fd = 0; fd <= 2; fd++) {	/* try in/out/err in case redirected */
		if (ioctl(0, TIOCGWINSZ, &size) == 0
		 && size.ws_row > 0
		 && size.ws_col > 0)
			exit(0);
	}
	exit(1);
}
		],
		[nc_cv_use_tiocgwinsz=yes],
		[nc_cv_use_tiocgwinsz=no],
		[nc_cv_use_tiocgwinsz=unknown])
	])
AC_MSG_RESULT($nc_cv_use_tiocgwinsz)
test $nc_cv_use_tiocgwinsz != yes && AC_DEFINE(BROKEN_TIOCGETWINSZ)
])dnl
dnl ---------------------------------------------------------------------------
dnl
AC_DEFUN([NC_TYPE_SIGACTION],
[
AC_MSG_CHECKING([for type sigaction_t])
AC_CACHE_VAL(nc_cv_type_sigaction,[
	AC_TRY_COMPILE([
#include <signal.h>],
		[sigaction_t x],
		[nc_cv_type_sigaction=yes],
		[nc_cv_type_sigaction=no])
	])
AC_MSG_RESULT($nc_cv_type_sigaction)
test $nc_cv_type_sigaction = yes && AC_DEFINE(HAVE_TYPE_SIGACTION)
])
dnl ---------------------------------------------------------------------------
dnl Make an uppercase version of a given name
AC_DEFUN([NC_UPPERCASE],
[
changequote(,)dnl
$2=`echo $1 |tr '[a-z]' '[A-Z]'`
changequote([,])dnl
])dnl
dnl ---------------------------------------------------------------------------
dnl Get the version-number for use in shared-library naming, etc.
AC_DEFUN([NC_VERSION],
[
AC_CACHE_VAL(nc_cv_rel_version,[
changequote(,)dnl
nc_cv_rel_version=`egrep 'VERSION[ 	]*=' $srcdir/dist.mk | sed -e 's/^[^0-9]*//'`
changequote([,])dnl
])
AC_CACHE_VAL(nc_cv_abi_version,[
changequote(,)dnl
nc_cv_abi_version=`egrep 'SHARED_ABI[ 	]*=' $srcdir/dist.mk | sed -e 's/^[^0-9]*//'`
changequote([,])dnl
])
AC_MSG_RESULT(Configuring NCURSES $nc_cv_rel_version ABI $nc_cv_abi_version (`date`))
dnl We need these values in the generated makefiles
AC_SUBST(nc_cv_rel_version)
AC_SUBST(nc_cv_abi_version)
AC_SUBST(nc_cv_builtin_bool)
AC_SUBST(nc_cv_sizeof_bool)
])dnl
