#!/bin/sh
#
# termsort.sh -- generate indirection vectors for the various sort methods
#
# The output of this script is C source for nine arrays that list three sort
# orders for each of the three different classes of terminfo capabilities.
#
echo "/*";
echo " * termsort.c --- sort order arrays for use by infocmp.";
echo " *";
echo " * Note: this file is generated using termsort.sh, do not edit by hand.";
echo " */";

echo "int bool_terminfo_sort[] = {";
awk <Caps '
BEGIN           {i = 0;}
/^#/            {next;}
$3 == "bool"    {printf("%s\t%d\n", $2, i++);}
' | sort | awk '{print "\t", $2, ",\t/* ", $1, " */";}';
echo "};";
echo "";

echo "int num_terminfo_sort[] = {";
awk <Caps '
BEGIN           {i = 0;}
/^#/            {next;}
$3 == "num"     {printf("%s\t%d\n", $2, i++);}
' | sort | awk '{print "\t", $2, ",\t/* ", $1, " */";}';
echo "};";
echo "";

echo "int str_terminfo_sort[] = {";
awk <Caps '
BEGIN           {i = 0;}
/^#/            {next;}
$3 == "str"     {printf("%s\t%d\n", $2, i++);}
' | sort | awk '{print "\t", $2, ",\t/* ", $1, " */";}';
echo "};";
echo "";

echo "int bool_variable_sort[] = {";
awk <Caps '
BEGIN           {i = 0;}
/^#/            {next;}
$3 == "bool"    {printf("%s\t%d\n", $1, i++);}
' | sort | awk '{print "\t", $2, ",\t/* ", $1, " */";}';
echo "};";
echo "";

echo "int num_variable_sort[] = {";
awk <Caps '
BEGIN           {i = 0;}
/^#/            {next;}
$3 == "num"     {printf("%s\t%d\n", $1, i++);}
' | sort | awk '{print "\t", $2, ",\t/* ", $1, " */";}';
echo "};";
echo "";

echo "int str_variable_sort[] = {";
awk <Caps '
BEGIN           {i = 0;}
/^#/            {next;}
$3 == "str"     {printf("%s\t%d\n", $1, i++);}
' | sort | awk '{print "\t", $2, ",\t/* ", $1, " */";}';
echo "};";
echo "";

echo "int bool_termcap_sort[] = {";
awk <Caps '
BEGIN           {i = 0;}
/^#/            {next;}
$3 == "bool"    {printf("%s\t%d\n", $4, i++);}
' | sort | awk '{print "\t", $2, ",\t/* ", $1, " */";}';
echo "};";
echo "";

echo "int num_termcap_sort[] = {";
awk <Caps '
BEGIN           {i = 0;}
/^#/            {next;}
$3 == "num"     {printf("%s\t%d\n", $4, i++);}
' | sort | awk '{print "\t", $2, ",\t/* ", $1, " */";}';
echo "};";
echo "";

echo "int str_termcap_sort[] = {";
awk <Caps '
BEGIN           {i = 0;}
/^#/            {next;}
$3 == "str"     {printf("%s\t%d\n", $4, i++);}
' | sort | awk '{print "\t", $2, ",\t/* ", $1, " */";}';
echo "};";
echo "";

echo "int bool_from_termcap[] = {";
awk <Caps '
$3 == "bool" && $5 == "-"       {print "0,\t/* ", $2, " */";}
$3 == "bool" && $5 == "Y"       {print "1,\t/* ", $2, " */";}
'
echo "};";
echo "";

echo "int num_from_termcap[] = {";
awk <Caps '
$3 == "num" && $5 == "-"        {print "0,\t/* ", $2, " */";}
$3 == "num" && $5 == "Y"        {print "1,\t/* ", $2, " */";}
'
echo "};";
echo "";

echo "int str_from_termcap[] = {";
awk <Caps '
$3 == "str" && $5 == "-"        {print "0,\t/* ", $2, " */";}
$3 == "str" && $5 == "Y"        {print "1,\t/* ", $2, " */";}
'
echo "};";
echo "";

