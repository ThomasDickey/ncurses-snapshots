#!/bin/sh
#
# MKhashsize.sh --- generate size include for hash functions
#
echo "/*"
echo " * hashsize.h -- hash and token table constants"
echo " */"

TABSIZE=`grep '},$' comp_captab.c | wc -l`

echo ""
echo "#define CAPTABSIZE	${TABSIZE}"
echo "#define HASHTABSIZE	(${TABSIZE} * 2)"
