# Makefile for creating ncurses distribution.
# This only needs to be used by developers
#
SHELL = /bin/sh

# This is the master version number.  If you change it here and do a configure,
# this value will be patched in everywhere a version is needed.
VERSION = 1.9.4

dist: ANNOUNCE INTRO
	(cd ..;  tar cvf ncurses-$(VERSION).tar `sed <ncurses-$(VERSION)/MANIFEST 's/^./ncurses-$(VERSION)/'`;  gzip ncurses-$(VERSION).tar)

stamp:
	(cd include;\
	 for N in MKterm.h.awk termcap.h curses.h unctrl.h; do \
	 rm -f $$N; sed 's,@VERSION@,$(VERSION),' <$$N.in >$$N; done)


# Don't mess with announce.html.in unless you have lynx available!
ANNOUNCE: announce.html.in
	sed 's,@VERSION@,$(VERSION),' <announce.html.in >announce.html
	lynx -dump announce.html > ANNOUNCE

INTRO: misc/ncurses-intro.html
	lynx -dump $^ > misc/ncurses-intro.doc

# Prepare distribution for version control
vcprepare:
	find . -type d -exec mkdir {}/RCS \;

# Write-lock almost all files not under version control.
EXCEPTIONS = "announce.html\\|ANNOUNCE\\|src/curses.h\\|misc/ncurses-intro.doc"
writelock:
	for x in `grep -v $(EXCEPTIONS) MANIFEST`; do if [ ! -f `dirname $$x`/RCS/`basename $$x`,v ]; then chmod a-w $${x}; fi; done

