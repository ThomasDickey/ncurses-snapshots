# Makefile for creating ncurses distribution.
# This only needs to be used by developers
#
SHELL = /bin/sh

# Below are the ncurses release version and the ncurses ABI version which is
# used to build the shared library.  If two ncurses libraries have the same ABI
# version, their application binary interface must be identical or one must be
# a superset of the other.  The ABI version number is independent of the
# ncurses release version.  For the ncurses release version 1.9.4, the ABI
# version is 1.9.  We can have the ABI version 1.9a, 1.9b, 1.9foobar, ..... 
# when the ncurses release version changes.  If a new ncurses has an
# incompatible application binary interface than previous one, the ABI version
# should be changed.
VERSION = 1.9.5
SHARED_ABI = 1.9

dist: ANNOUNCE INTRO
	(cd ..;  tar cvf ncurses-$(VERSION).tar `sed <ncurses-$(VERSION)/MANIFEST 's/^./ncurses-$(VERSION)/'`;  gzip ncurses-$(VERSION).tar)

stamp:
	(cd include; \
	 for N in MKterm.h.awk termcap.h curses.h unctrl.h; do \
	 rm -f $$N; sed 's,@VERSION@,$(VERSION),' <$$N.in >$$N; done)

# Don't mess with announce.html.in unless you have lynx available!
ANNOUNCE: announce.html
	lynx -dump $^ >ANNOUNCE
announce.html: announce.html.in
	sed 's,@VERSION@,$(VERSION),' <$^ >announce.html

INTRO: misc/ncurses-intro.html
	lynx -dump $^ > misc/ncurses-intro.doc

# Prepare distribution for version control
vcprepare:
	find . -type d -exec mkdir {}/RCS \;

# Write-lock almost all files not under version control.
EXCEPTIONS = "announce.html\\|ANNOUNCE\\|src/curses.h\\|misc/ncurses-intro.doc"
writelock:
	for x in `grep -v $(EXCEPTIONS) MANIFEST`; do if [ ! -f `dirname $$x`/RCS/`basename $$x`,v ]; then chmod a-w $${x}; fi; done

# Makefile ends here
