# Makefile for creating ncurses distribution.
# This only needs to be used by developers
#
SHELL = /bin/sh
VERSION = 1.9.2d

dist: ANNOUNCE INTRO
	(cd ..;  tar cvf ncurses-$(VERSION).tar `sed <ncurses-$(VERSION)/MANIFEST 's/^./ncurses-$(VERSION)/'`;  gzip ncurses-$(VERSION).tar)

# Don't mess with announce.html unless you have lynx available!
ANNOUNCE: announce.html.in
	sed 's,@VERSION@,$(VERSION),' <announce.html.in >announce.html
	lynx -dump announce.html > ANNOUNCE

INTRO: misc/ncurses-intro.html
	lynx -dump $^ > misc/ncurses-intro.doc

# Prepare distribution for version control
vcprepare:
	find `cat MANIFEST` -type f -exec chmod -w {} \;
	find . -type d -exec mkdir {}/RCS \;
