
#  This work is copyrighted. See COPYRIGHT.OLD & COPYRIGHT.NEW for
#  details. If they are missing then this copy is in violation of 
#  the copyright conditions.           


BEGIN	{
	    print  "/*"
	    print  " *	comp_captab.c -- The names of the capabilities in a form ready for"
	    print  " *		         the making of a hash table for the compiler."
	    print  " *"
	    print  " */"
	    print  ""
	    print  ""
	    print  "#include \"tic.h\""
	    print  "#include \"terminfo.h\""
	    print  "#include \"hashsize.h\""
	    print  ""
	    print  ""
	    tablesize = 0;
	    print  "struct name_table_entry	cap_table[] ="
	    print  "{"
	}


$3 == "bool"	{
    		    if ($2 != $4)
		    {
			tablesize++;
			printf "\t{ 0,%15s,\tBOOLEAN,\t%3d },\n", $4, BoolCount
		    }
		    printf "\t{ 0,%15s,\tBOOLEAN,\t%3d },\n", $2, BoolCount++
		    tablesize++;
		}


$3 == "num"	{
    		    if ($2 != $4)
		    {
			tablesize++;
			printf "\t{ 0,%15s,\tNUMBER,\t\t%3d },\n", $4, NumCount
		    }
		    printf "\t{ 0,%15s,\tNUMBER,\t\t%3d },\n", $2, NumCount++
		    tablesize++;
		}


$3 == "str"	{
    		    if ($2 != $4)
		    {
			tablesize++;
			printf "\t{ 0,%15s,\tSTRING,\t\t%3d },\n", $4, StrCount
		    }
		    printf "\t{ 0,%15s,\tSTRING,\t\t%3d },\n", $2, StrCount++
		    tablesize++;
		}


END	{
	    print  "};"
	    print  ""
	    print  "struct name_table_entry *cap_hash_table[HASHTABSIZE];"
	    print  ""
	    printf "#if (BOOLCOUNT!=%d)||(NUMCOUNT!=%d)||(STRCOUNT!=%d)\n",\
						BoolCount, NumCount, StrCount
	    print  "#error	--> terminfo.h and comp_captab.c disagree about the <--"
	    print  "#error	--> numbers of booleans, numbers and/or strings <--"
	    print  "#endif"
	}
