# Makefile for creating ncurses distributions.
#
# This only needs to be used directly as a makefile by developers, but
# configure mines the current version number out of here.  To move
# to a new version number, just edit this file and run configure.
#
SHELL = /bin/sh

# Below are the ncurses release version and the ncurses ABI version
# (the latter is used to build the shared library).  If two ncurses libraries
# have the same ABI version, their application binary interface must be
# identical or one must be a superset of the other.  The ABI version number
# is independent of the ncurses release version. For ncurses release
# version 1.9.5, the ABI version is 1.9.  We can have the ABI version
# 1.9a, 1.9b, 1.9foobar, ... when the ncurses release version changes
# If a new ncurses has an incompatible application binary interface than
# previous one, the ABI version should be changed.
VERSION = 1.9.9
SHARED_ABI = 3.0

dist: announce.html misc/ncurses-intro.doc misc/hackguide.doc
	(cd ..;  tar cvf ncurses-$(VERSION).tar `sed <ncurses-$(VERSION)/MANIFEST 's/^./ncurses-$(VERSION)/'`;  gzip ncurses-$(VERSION).tar)

distclean:
	rm ANNOUNCE announce.html misc/ncurses-intro.doc misc/hackguide.doc

# Don't mess with announce.html.in unless you have lynx available!
announce.html: announce.html.in
	sed 's,@VERSION@,$(VERSION),' <announce.html.in >announce.html
	lynx -dump announce.html >ANNOUNCE

misc/ncurses-intro.doc: misc/ncurses-intro.html
	lynx -dump misc/ncurses-intro.html > misc/ncurses-intro.doc
misc/hackguide.doc: misc/hackguide.html
	lynx -dump misc/hackguide.html > misc/hackguide.doc

# Prepare distribution for version control
vcprepare:
	find . -type d -exec mkdir {}/RCS \;

# Write-lock almost all files not under version control.
EXCEPTIONS = "announce.html\\|ANNOUNCE\\|misc/.*\\.doc\\|man/terminfo.5\\|lib_gen.c"
writelock:
	for x in `grep -v $(EXCEPTIONS) MANIFEST`; do if [ ! -f `dirname $$x`/RCS/`basename $$x`,v ]; then chmod a-w $${x}; fi; done

TAGS:
	etags */*.[ch]

# Makefile ends here
