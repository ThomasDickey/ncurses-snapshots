

#ifndef _TERMCAP_H
#define _TERMCAP_H	1

#ifdef __cplusplus
extern "C" 
{
#endif /* __cplusplus */

#include <sys/types.h>

extern char PC;
extern char *UP;
extern char *BC;
extern speed_t ospeed;

extern int tgetent(char *, const char *);
extern int tgetflag(const char *);
extern int tgetnum(const char *);
extern char *tgetstr(const char *, char **);

extern int tputs(const char *, int, int (*)(void));

extern char *tgoto(const char *, int, int);
extern char *tparam(const char *, char *, int, ...);

#ifdef __cplusplus
}
#endif

#endif /* _TERMCAP_H */
