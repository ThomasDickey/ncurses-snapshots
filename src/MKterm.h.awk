
BEGIN		{
		    print "/*"
		    print "**	term.h -- Definition of struct term"
		    print "*/"
		    print ""
		    print "#ifndef _TERMINFO_H"
		    print "#define _TERMINFO_H"
		    print ""
		    print "#ifndef NOTERMIOS"
		    print "#ifndef TERMIOS"
		    print "#define TERMIOS 1"
		    print "#endif"
		    print "#include <termios.h>"
		    print "#define TTY struct termios"
		    print ""
		    print "#else"
		    print ""
		    print "#undef TERMIOS"
		    print "#include <sgtty.h>"
		    print "#include <sys/ioctl.h>"
		    print "#define TTY struct sgttyb"
		    print "#endif"
		    print ""
		    print "extern char ttytype[];"
		    print "#define NAMESIZE 256"
		    print "" 
		    print "#define CUR cur_term->type."
		    print ""
		}


$3 == "bool"	{
		    printf "#define %-30s CUR Booleans[%d]\n", $1, BoolCount++
		}

$3 == "num"	{
		    printf "#define %-30s CUR Numbers[%d]\n", $1, NumberCount++
		}

$3 == "str"	{
		    printf "#define %-30s CUR Strings[%d]\n", $1, StringCount++
		}

$2 == "%%-STOP-HERE-%%"	{
			print  ""
			printf "#define BOOLWRITE %d\n", BoolCount
			printf "#define NUMWRITE  %d\n", NumberCount
			printf "#define STRWRITE  %d\n", StringCount
			print  ""
		}


END		{
			print  ""
			print  ""
			printf "#define BOOLCOUNT %d\n", BoolCount
			printf "#define NUMCOUNT  %d\n", NumberCount
			printf "#define STRCOUNT  %d\n", StringCount
			print  ""
			print "typedef struct termtype {	/* in-core form of terminfo data */"
			print "    char	 *term_names;		/* str_table offset of term names */"
			print "    char	 *str_table;		/* pointer to string table */"
			print "    char	 Booleans[BOOLCOUNT];	/* array of values */"
			print "    short Numbers[NUMCOUNT];	/* array of values */"
			print "    char	 *Strings[STRCOUNT];	/* array of string offsets */"
			print "} TERMTYPE;"
			print ""
			print "typedef struct term {		/* describe an actual terminal */"
			print "    TERMTYPE	type;		/* terminal type description */"
			print "    short 	Filedes;	/* file description being written to */"
			print "    TTY 	Ottyb,			/* original state of the terminal */"
			print "	     	Nttyb;			/* current state of the terminal */"
			print "} TERMINAL;"
			print  ""
			print  "extern TERMINAL	*cur_term;"
			print  ""
			print "extern char *boolnames[], *boolcodes[], *boolfnames[],"
			print "            *numnames[], *numcodes[], *numfnames[],"
			print "            *strnames[], *strcodes[], *strfnames[];"
			print ""
			print "extern set_curterm(TERMINAL *);"
			print "extern del_curterm(TERMINAL *);"
			print ""
            		print "#endif"
		}
