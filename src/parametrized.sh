#!/bin/sh
#
# termsort.sh -- generate indirection vectors for the various sort methods
#
# The output of this script is C source for an array specifying whether
# termcap strings should undergo parameter translation is also included.
#
echo "/*";
echo " * parametrized.h --- is a termcap capability parametrized?";
echo " *";
echo " * Note: this file is generated using parametrized.sh, do not edit by hand.";
echo " */";

echo "static bool parametrized[] = {";
awk <Caps.filtered '
$3 == "str" && !($0 ~ /#1/)     {print "0,\t/* ", $2, " */";}
$3 == "str" && $0 ~ /#1/        {print "1,\t/* ", $2, " */";}
'
echo "};";
echo "";

