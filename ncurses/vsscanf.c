/*
 * This function is needed to support vwscanw
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if !HAVE_VSSCANF
#include <stdarg.h>

int vsscanf(const char *str, const char *format, va_list ap)
{
	return -1;	/* not implemented */
}
#else
void _nc_vsscanf() { }	/* nonempty for strict ANSI compilers */
#endif
