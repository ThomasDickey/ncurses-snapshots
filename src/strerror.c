#include <stdio.h>
extern int errno;
extern char *sys_errlist[];
extern int sys_nerr;

char *strerror(int err)
{
	if (err >= sys_nerr)
		return NULL;
	return sys_errlist[err];
}
