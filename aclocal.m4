dnl Macros used in NCURSES auto-configuration script.
dnl
dnl ---------------------------------------------------------------------------
dnl Test if 'bool' is a builtin type in the configured C++ compiler.  Some
dnl older compilers (e.g., gcc 2.5.8) don't support 'bool' directly; gcc
dnl 2.6.3 does, in anticipation of the ANSI C++ standard.
define([NC_BOOL_DECL],
[
AC_MSG_CHECKING([for builtin c++ bool type])
AC_CACHE_VAL(nc_cv_builtin_bool,[
	AC_TRY_COMPILE([],[bool x = false],
		[nc_cv_builtin_bool=yes],
		[nc_cv_builtin_bool=no])
	])
AC_MSG_RESULT($nc_cv_builtin_bool)
test $nc_cv_builtin_bool = yes && AC_DEFINE(CXX_BUILTIN_BOOL)
])
dnl ---------------------------------------------------------------------------
dnl Test for the size of 'bool' in the configured C++ compiler.
define([NC_BOOL_SIZE],
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
test $nc_cv_sizeof_bool != unknown && AC_DEFINE_UNQUOTED(CXX_TYPE_OF_BOOL,$nc_cv_sizeof_bool)
])
dnl ---------------------------------------------------------------------------
dnl
define([NC_ERRNO],
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
])
dnl ---------------------------------------------------------------------------
dnl Construct the list of include-options according to whether we're building
dnl in the source directory or using '--srcdir=DIR' option.  The applications
dnl (in 'test' and 'c++') have a different include-path from the library units
dnl because the latter are compiled within subdirectories of 'src'.
define([NC_INCLUDE_DIRS],
[
APP_INCLUDES="-I. -I../src -I../include"
LIB_INCLUDES="-I.. -I../../include"
if test "$srcdir" != "."; then
	APP_INCLUDES="$APP_INCLUDES -I\$(srcdir)/../src -I\$(srcdir)/../include"
	LIB_INCLUDES="$LIB_INCLUDES -I\$(srcdir) -I\$(srcdir)/../include"
fi
APP_INCLUDES="$APP_INCLUDES -I\$(includedir)"
AC_SUBST(APP_INCLUDES)
AC_SUBST(LIB_INCLUDES)
])dnl
dnl ---------------------------------------------------------------------------
dnl Append definitions and rules for the given models to the src/Makefile
define([NC_LIB_RULES],
[
for nc_item in $NC_LIST_MODELS
do
	echo 'Appending rules for '$nc_item' library'
	NC_UPPERCASE($nc_item,NC_ITEM)
	NC_LIB_SUFFIX($nc_item,nc_suffix)
	$AWK -f $srcdir/mk-1st.awk -v MODEL=$NC_ITEM model=$nc_item suffix=$nc_suffix $srcdir/src/modules >>src/Makefile
	$AWK -f $srcdir/mk-2nd.awk -v MODEL=$NC_ITEM model=$nc_item srcdir=$srcdir echo=$WITH_ECHO $srcdir/src/modules >>src/Makefile
	test ! -d src/$nc_item && mkdir src/$nc_item
done
])dnl
dnl ---------------------------------------------------------------------------
dnl Compute the library-suffix from the given model name
define([NC_LIB_SUFFIX],
[
	case $1 in
	normal)  $2='.a'   ;;
	debug)   $2='_g.a' ;;
	profile) $2='_p.a' ;;
	shared)  $2='.so'  ;;
	esac
])dnl
dnl ---------------------------------------------------------------------------
dnl Compute the string to append to -library from the given model name
define([NC_LIB_TYPE],
[
	case $1 in
	normal)  $2=''   ;;
	debug)   $2='_g' ;;
	profile) $2='_p' ;;
	shared)  $2=''   ;;
	esac
])dnl
dnl ---------------------------------------------------------------------------
dnl Construct the list of library names to generate.
define([NC_LIBS_TO_MAKE],
[
LIBS_TO_MAKE=""
for nc_item in $nc_list_models
do
	NC_LIB_SUFFIX($nc_item,nc_suffix)
	LIBS_TO_MAKE="$LIBS_TO_MAKE libncurses$nc_suffix libpanel$nc_suffix"
done
AC_SUBST(LIBS_TO_MAKE)dnl
])dnl
dnl ---------------------------------------------------------------------------
dnl Some 'make' programs support $(MAKEFLAGS), some $(MFLAGS), to pass 'make'
dnl options to lower-levels.  It's very useful for "make -n" -- if we have it.
dnl (GNU 'make' does both :-)
define([NC_MAKEFLAGS],
[
AC_MSG_CHECKING([for makeflags variable])
AC_CACHE_VAL(nc_cv_makeflags,[
	nc_cv_makeflags=''
	for nc_option in '$(MFLAGS)' '-$(MAKEFLAGS)'
	do
		cat >ncurses.tmp <<EOF
all :
	echo '.$nc_option'
EOF
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
]
)
dnl ---------------------------------------------------------------------------
dnl Attempt to determine the appropriate CC/LD options for creating a shared
dnl library.
define([NC_SHARED_OPTS],
[
	nc_systype="`(uname -s || hostname) 2>/dev/null | sed 1q`"
	if test -z "$nc_systype"; then nc_systype=unknown;fi
	case $nc_systype in
	Linux)
		CC_SHARED_OPTS='-fPIC'
		MK_SHARED_LIB='gcc -o $[@] -shared -Wl,-soname,$[@],-stats'
		;;
	SunOS)
		CC_SHARED_OPTS='-pic'
		MK_SHARED_LIB='ld --assert-pure-text -o $[@]'
		;;
	*)
		CC_SHARED_OPTS='unknown'
		MK_SHARED_LIB='echo unknown'
		;;
	esac
	AC_SUBST(CC_SHARED_OPTS)
	AC_SUBST(MK_SHARED_LIB)
])dnl
dnl ---------------------------------------------------------------------------
dnl	Check for declarion of sys_errlist in one of stdio.h and errno.h.  
dnl	Declaration of sys_errlist on BSD4.4 interferes with our declaration.
dnl	Reported by Keith Bostic.
define([NC_SYS_ERRLIST],
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
dnl On some systems ioctl(fd, TIOCGWINSZ, &size) will always return {0,0} until
dnl ioctl(fd, TIOCSWINSZ, &size) is called to explicitly set the size of the
dnl screen.
dnl
dnl Attempt to determine if we're on such a system by running a test-program.
dnl This won't work, of course, if the configure script is run in batch mode,
dnl since we've got to have access to the terminal.
define([NC_TIOCGWINSZ],
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
])
dnl ---------------------------------------------------------------------------
dnl
define([NC_TYPE_SIGACTION],
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
define([NC_UPPERCASE],
[
changequote(,)dnl
$2=`echo $1 |tr '[a-z]' '[A-Z]'`
changequote([,])dnl
])dnl
