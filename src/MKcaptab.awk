
#  This work is copyrighted. See COPYRIGHT.OLD & COPYRIGHT.NEW for
#  details. If they are missing then this copy is in violation of 
#  the copyright conditions.           

cat <<'EOF'
/*
 *	comp_captab.c -- The names of the capabilities in a form ready for
 *		         the making of a hash table for the compiler.
 *
 */


#include "tic.h"
#include "terminfo.h"
#include "hashsize.h"

struct name_table_entry	info_table[] =
{
EOF

awk <Caps '
BEGIN		{
		    tablesize = 0;
		}


$3 == "bool"	{
		    printf "\t{ 0,%15s,\tBOOLEAN,\t%3d },\n", $2, BoolCount++
		}


$3 == "num"	{
		    printf "\t{ 0,%15s,\tNUMBER,\t\t%3d },\n", $2, NumCount++
		}


$3 == "str"	{
		    printf "\t{ 0,%15s,\tSTRING,\t\t%3d },\n", $2, StrCount++
		}
'

cat <<'EOF'
};

struct name_table_entry	cap_table[] =
{
EOF

awk <Caps '
BEGIN		{
		    tablesize = 0;
		    BoolCount = NumCount = StrCount = 0;
		}


$3 == "bool"	{
		    printf "\t{ 0,%15s,\tBOOLEAN,\t%3d },\n", $4, BoolCount++
		}


$3 == "num"	{
		    printf "\t{ 0,%15s,\tNUMBER,\t\t%3d },\n", $4, NumCount++
		}


$3 == "str"	{
		    printf "\t{ 0,%15s,\tSTRING,\t\t%3d },\n", $4, StrCount++
		}
END	{
	    print  "};"
	    print  ""
	    print  "struct name_table_entry *info_hash_table[HASHTABSIZE];"
	    print  "struct name_table_entry *cap_hash_table[HASHTABSIZE];"
	    print  ""
	    printf "#if (BOOLCOUNT!=%d)||(NUMCOUNT!=%d)||(STRCOUNT!=%d)\n",\
						BoolCount, NumCount, StrCount
	    print  "#error	--> terminfo.h and comp_captab.c disagree about the <--"
	    print  "#error	--> numbers of booleans, numbers and/or strings <--"
	    print  "#endif"
	}
'
