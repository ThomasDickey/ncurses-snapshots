#
#  Simple makefile for NCursesPanel demo
#
#  NOTE:  This demo requires the panels library. 
#
CFLAGS = -I/usr/include/ncurses
LDFLAGS = -L/usr/lib
CC = g++

.cc.o:
	$(CC) $(CFLAGS) -c $?

demo: cursesw.o cursesp.o demo.o
	$(CC) -o demo cursesw.o cursesp.o demo.o $(LDFLAGS) -lpanel -lncurses

clean:
	rm *.o demo
