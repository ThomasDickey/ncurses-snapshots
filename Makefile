#
# Master makefile for the ncurses package
#
all:
	more INSTALL

PKG = ncurses-1.8.8

$(PKG).tar:
	(cd ..; tar -cvf $(PKG)/$(PKG).tar `sed <$(PKG)/MANIFEST 's/^./$(PKG)/'`)

$(PKG).tar.gz: $(PKG).tar
	gzip $(PKG).tar

dist: $(PKG).tar.gz

clean:
	rm -f $(PKG).tar*
	cd src; make clean
	cd test; make clean
