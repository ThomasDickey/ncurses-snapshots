/*
 * This function is needed to support vwscanw
 */
#include "system.h"

#if !HAVE_VSSCANF
#include <stdarg.h>

int vsscanf(const char *str, const char *format, va_list ap)
{
	return -1;	/* not implemented */
}
#endif
