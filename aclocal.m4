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
test $nc_cv_sizeof_bool != unknown && AC_DEFINE_UNQUOTED(CXX_TYPEOF_BOOL,$nc_cv_sizeof_bool)
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
		set nc_result=`${MAKE-make} -f ncurses.tmp`
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
	if (ioctl(0, TIOCGWINSZ, &size) < 0
	 || size.ws_row == 0
	 || size.ws_col == 0)
		exit(1);
	exit(0);
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
