/*
 * This function is needed to support vwscanw
 */

#if !HAVE_VSSCANF
#include <stdarg.h>

int vsscanf(const char *str, const char *format, va_list ap)
{
	return -1;	/* not implemented */
}
#endif
