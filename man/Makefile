

install:
	-@mkdir -p $(MAN) $(MAN)/man3 $(MAN)/man5 $(MAN)/man1
	@for i in *.3x; do \
		sed 's,@TERMINFO@,$(SRCDIR),' < $$i >tmp.tmp; \
		echo installing man page $$i;\
		$(INSTALL) -m 0644 -o bin -g bin tmp.tmp $(MAN)/man3/$$i; \
	done 
	@for i in *.1m *.1; do \
		sed 's,@TERMINFO@,$(SRCDIR),' < $$i >tmp.tmp; \
		echo installing man page $$i;\
		$(INSTALL) -m 0644 -o bin -g bin tmp.tmp $(MAN)/man1/$$i; \
	done
	@for i in *.5; do \
		sed 's,@TERMINFO@,$(SRCDIR),' < $$i >tmp.tmp; \
		echo installing man page $$i;\
		$(INSTALL) -m 0644 -o bin -g bin tmp.tmp $(MAN)/man5/$$i; \
	done
	@rm -f tmp.tmp

