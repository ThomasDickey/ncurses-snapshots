# Generated automatically from Makefile.in by configure.
#  Simple makefile for NCursesPanel demo
#
#  NOTE:  This demo requires the panels library. 

SHELL		= /bin/sh

srcdir		= .
prefix		= /usr
exec_prefix	= ${prefix}
libdir		= $(exec_prefix)/lib
includedir	= $(prefix)/include/ncurses

CC		= c++
CFLAGS		= -g -O

CPPFLAGS	= -I. -I$(srcdir) -I$(srcdir)/../include -I$(srcdir)/../src -I$(includedir) -DHAVE_CONFIG_H

CCFLAGS		= $(CPPFLAGS) $(CFLAGS)

LINK		= $(CC)
LDFLAGS		= -L../src -L$(libdir)  -lbsd 

.cc.o:
	@echo compiling $@
	@$(CC) -c $(CCFLAGS) -o $*.o $(srcdir)/$*.cc

all: demo

DEPS_DEMO = cursesw.o cursesp.o demo.o
demo: $(DEPS_DEMO)
	$(LINK) -o $@ $(DEPS_DEMO) $(LDFLAGS) -lpanel -lncurses

install:

clean:
	-rm -f *.o demo

distclean: clean
	-rm -f Makefile

mostlyclean: clean

realclean: distclean
