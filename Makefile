#
# Master makefile for the ncurses package
#
all:
	more INSTALL

PKG = ncurses-1.9.1

$(PKG).tar:
	(cd ..; tar -cvf $(PKG)/$(PKG).tar `sed <$(PKG)/MANIFEST 's/^./$(PKG)/'`)

$(PKG).tar.gz: $(PKG).tar
	gzip $(PKG).tar

dist: $(PKG).tar.gz

vcprepare:
	find . -type f -exec chmod -w {} \;
	find . -type d -exec mkdir {}/RCS \;
#	for x in . src man test; do cd $$x; ci -u -t- </dev/null *; done

clean:
	rm -f $(PKG).tar*
	cd src; make clean
	cd test; make clean
