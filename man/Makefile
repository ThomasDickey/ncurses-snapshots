# Generated automatically from Makefile.in by configure.
# Makefile for ncurses manual pages.

SHELL		= /bin/sh

srcdir		= .
prefix		= /usr
exec_prefix	= ${prefix}
datadir		= $(exec_prefix)/lib/terminfo
mandir		= $(prefix)/man

OWNER		= -o bin -g bin
INSTALL		= /usr/local/bin/install -c $(OWNER)
INSTALL_PROGRAM	= ${INSTALL}
INSTALL_DATA	= ${INSTALL} -m 644

all:

install:
	$(srcdir)/../mkinstalldirs $(mandir) $(mandir)/man3 $(mandir)/man5 $(mandir)/man1
	@cd $(srcdir); for i in *.3x; do \
		sed 's,@TERMINFO@,$(datadir),' < $$i >/tmp/man.tmp; \
		echo installing man page $$i;\
		$(INSTALL_DATA) /tmp/man.tmp $(mandir)/man3/$$i; \
	done 
	@cd $(srcdir); for i in *.1m; do \
		sed 's,@TERMINFO@,$(datadir),' < $$i >/tmp/man.tmp; \
		echo installing man page $$i;\
		$(INSTALL_DATA) /tmp/man.tmp $(mandir)/man1/$$i; \
	done
	@cd $(srcdir); for i in *.5; do \
		sed 's,@TERMINFO@,$(datadir),' < $$i >/tmp/man.tmp; \
		echo installing man page $$i;\
		$(INSTALL_DATA) /tmp/man.tmp $(mandir)/man5/$$i; \
	done
	rm -f /tmp/man.tmp

clean mostlyclean:

distclean realclean: clean
	rm -f Makefile
