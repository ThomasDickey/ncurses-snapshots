
/* This work is copyrighted. See COPYRIGHT.OLD & COPYRIGHT.NEW for   *
*  details. If they are missing then this copy is in violation of    *
*  the copyright conditions.                                        */

/*
**
**	object.h - Format of compiled terminfo files
**
**		Header (12 bytes), containing information given below
**		Names Section, containing the names of the terminal
**		Boolean Section, containing the values of all of the
**				boolean capabilities
**				A null byte may be inserted here to make
**				sure that the Number Section begins on an
**				even word boundary.
**		Number Section, containing the values of all of the numeric
**				capabilities, each as a short integer
**		String Section, containing short integer offsets into the
**				String Table, one per string capability
**		String Table, containing the actual characters of the string
**				capabilities.
**
**	NOTE that all short integers in the file are stored using VAX/PDP-style
**	byte-swapping, i.e., least-significant byte first.  The code in
**	read_entry() automatically fixes this up on machines which don't use
**	this system (I hope).
**
*/


#define MAGIC	0432

struct header
{
	short	magic;		/* Magic Number (0432)			*/
	short	name_size;	/* Size of names section		*/
	short	bool_count;	/* Number of booleans			*/
	short	num_count;	/* Number of numbers			*/
	short	str_count;	/* Number of strings			*/
	short	str_size;	/* Size of string table			*/
};
