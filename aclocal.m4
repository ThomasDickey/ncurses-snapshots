dnl Macros used in NCURSES auto-configuration script.
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
