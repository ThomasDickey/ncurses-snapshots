
/***************************************************************************
*                            COPYRIGHT NOTICE                              *
****************************************************************************
*                ncurses is copyright (C) 1992-1995                        *
*                          Zeyd M. Ben-Halim                               *
*                          zmbenhal@netcom.com                             *
*                          Eric S. Raymond                                 *
*                          esr@snark.thyrsus.com                           *
*                                                                          *
*        Permission is hereby granted to reproduce and distribute ncurses  *
*        by any means and for any fee, whether alone or as part of a       *
*        larger distribution, in source or in binary form, PROVIDED        *
*        this notice is included with any such distribution, and is not    *
*        removed from any of its header files. Mention of ncurses in any   *
*        applications linked with it is highly appreciated.                *
*                                                                          *
*        ncurses comes AS IS with no warranty, implied or expressed.       *
*                                                                          *
***************************************************************************/


/*
 *	parse_entry.c -- compile one terminfo or termcap entry
 *
 *	Get an exact in-core representation of an entry.  Don't
 *	try to resolve use or tc capabilities, that is someone
 *	else's job.  Depends on the lexical analyzer to get tokens
 *	from the input stream.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tic.h"
#include "term.h"
#include "term_entry.h"
#include "parametrized.h"

void set_termcap_defaults(TERMTYPE *);

/*
 *	int
 *	parse_entry(entry, literal)
 *
 *	Compile one entry.  Doesn't try to resolve use or tc capabilities.
 *
 *	found-forward-use = FALSE
 *	re-initialise internal arrays
 *	get_token();
 *	if the token was not a name in column 1, complain and die
 *	save names in entry's string table
 *	while (get_token() is not EOF and not NAMES)
 *	        check for existance and type-correctness
 *	        enter cap into structure
 *	        if STRING
 *	            save string in entry's string table
 *	push back token
 */

int parse_entry(struct entry *entryp, int literal)
{
    int			token_type;
    struct name_table_entry	*entry_ptr;
    char			*ptr, *bp, buf[NAMESIZE];

    token_type = get_token();

    if (token_type == EOF)
	return(EOF);
    if (token_type != NAMES)
	err_abort("Entry does not start with terminal names in column one");
	
    init_entry(&entryp->tterm);

    entryp->cstart = comment_start;
    entryp->cend = comment_end;
    DEBUG(2, ("Comment range is %ld to %ld", entryp->cstart, entryp->cend));

    /* junk the 2-character termcap name, if present */
    ptr = curr_token.tk_name;
    if (ptr[2] == '|')
    {
	ptr = curr_token.tk_name + 3;
	curr_token.tk_name[2] = '\0';
    }

    entryp->tterm.str_table = entryp->tterm.term_names = save_str(ptr);

    (void) strcpy(buf, ptr);
    if ((bp = strchr(buf, '|')) != (char *)NULL)
	*bp = '\0';
    set_type(buf);

    DEBUG(1, ("Starting '%s'", buf));

    entryp->nuses = 0;

    for (token_type = get_token();
	 token_type != EOF  &&  token_type != NAMES;
	 token_type = get_token())
    {
	if (strcmp(curr_token.tk_name, "use") == 0
	    || strcmp(curr_token.tk_name, "tc") == 0) {
	    entryp->uses[entryp->nuses++] = save_str(curr_token.tk_valstring);
	} else {
	    entry_ptr = find_entry(curr_token.tk_name,
	    				syntax ? cap_hash_table : info_hash_table);

	    /*
	     * Our kluge to handle aliasing.  The reason it's done
	     * this ugly way, with a linear search, is so the hashing
	     * machinery doesn't have to be made really complicated
	     * (also we get better warnings this way).  No point in
	     * making this case fast, aliased caps aren't common now
	     * and will get rarer.
	     */
	    if (syntax && entry_ptr == NOTFOUND)
	    {
		struct alias	*ap;

		for (ap = alias_table; ap->from; ap++)
		    if (strcmp(ap->from, curr_token.tk_name) == 0)
		    {
			entry_ptr = find_entry(ap->to, cap_hash_table);
			if (entry_ptr)
			    warning("%s aliased to %s", ap->from, ap->to);
			break;
		    }
	    }

	    if (entry_ptr == NOTFOUND) {
		warning("Unknown Capability - '%s'",
			curr_token.tk_name);
		continue;
	    }


	    if (token_type != CANCEL &&  entry_ptr->nte_type != token_type)
	    {
		entry_ptr = NOTFOUND;

		/*
		 * Nasty special cases here handle situations in which type
		 * information can resolve name clashes.  Normal lookup
		 * finds the last instance in the capability table of a
		 * given name, regardless of type.  find_type_entry looks
		 * for a first matching instance with given type.  So as 
		 * long as all ambiguous names occur in pairs of distinct
		 * type, this will do the job.
		 */

		/* tell max_attributes from arrow_key_map */
		if (token_type == NUMBER && !strcmp("ma", curr_token.tk_name))
			entry_ptr = find_type_entry("ma", NUMBER,
	    				syntax ? cap_table : info_table);

		/* map terminfo's string MT to MT */
		if (token_type == STRING && !strcmp("MT", curr_token.tk_name))
			entry_ptr = find_type_entry("MT", STRING,
	    				syntax ? cap_table : info_table);

		if (entry_ptr == NOTFOUND)
			warning("Wrong type used for capability '%s'",
				curr_token.tk_name);
	    }
	    switch (token_type) {
	    case CANCEL:
		switch (entry_ptr->nte_type) {
		case BOOLEAN:
		    entryp->tterm.Booleans[entry_ptr->nte_index] = CANCELLED_BOOLEAN;
		    break;

		case NUMBER:
		    entryp->tterm.Numbers[entry_ptr->nte_index] = CANCELLED_NUMERIC;
		    break;

		case STRING:
		    entryp->tterm.Strings[entry_ptr->nte_index] = CANCELLED_STRING;
		    break;
		}
		break;
		
	    case BOOLEAN:
		entryp->tterm.Booleans[entry_ptr->nte_index] = TRUE;
		break;
		    
	    case NUMBER:
		entryp->tterm.Numbers[entry_ptr->nte_index] =
		    curr_token.tk_valnumber;
		break;

	    case STRING:
		ptr = curr_token.tk_valstring;
		if (syntax==SYN_TERMCAP)
		    ptr = captoinfo(curr_token.tk_name,
				    ptr,
				    parametrized[entry_ptr->nte_index]);
		entryp->tterm.Strings[entry_ptr->nte_index] = save_str(ptr);
		break;

	    default:
		warning("Unknown token type");
		panic_mode((syntax==SYN_TERMCAP) ? ':' : ',');
		continue;
	    }
	} /* end else cur_token.name != "use" */

    } /* endwhile (not EOF and not NAMES) */

    push_token(token_type);

    /*
     * If this is a termcap entry, try to deduce as much as possible
     * from obsolete termcap capabilities.
     */
    if (syntax == SYN_TERMCAP && !literal && !entryp->nuses)
	set_termcap_defaults(&entryp->tterm);

    wrap_entry(entryp);

    return(OK);
}

/*
 * This routine fills in string caps that either had defaults under
 * termcap or can be manufactured from obsolete termcap capabilities.
 * It was lifted from Ross Ridge's mytinfo package.
 */

static char *C_CR = "\r";
static char *C_LF = "\n";
static char *C_BS = "\b";
static char *C_HT = "\t";

#define NOTSET(s)	((s) == (char *)NULL)

/*
 * This bit of legerdemain turns all the terminfo variable names into
 * references to locations in the arrays Booleans, Numbers, and Strings ---
 * precisely what's needed.
 *
 * Note: This code is the functional inverse of the fragment in capdefaults.c.
 */
#undef CUR
#define CUR tp->

void set_termcap_defaults(TERMTYPE *tp)
{
    char buf[MAX_LINE * 2 + 2];

    if (NOTSET(init_3string) && termcap_init2)
	init_3string = save_str(termcap_init2);

    if (NOTSET(reset_1string) && termcap_reset)
	reset_1string = save_str(termcap_reset);

    if (NOTSET(carriage_return)) {
	if (carriage_return_delay > 0) {
	    sprintf(buf, "%s$<%d>", C_CR, carriage_return_delay);
	    carriage_return = save_str(buf);
	} else
	    carriage_return = save_str(C_CR);
    }
    if (NOTSET(cursor_left)) {
	if (backspace_delay > 0) {
	    sprintf(buf, "%s$<%d>", C_BS, backspace_delay);
	    cursor_left = save_str(buf);
	} else if (backspaces_with_bs == 1)
	    cursor_left = save_str(C_BS);
	else if (!NOTSET(backspace_if_not_bs))
	    cursor_left = backspace_if_not_bs;
    }
    /* vi doesn't use "do", but it does seems to use nl (or '\n') instead */
    if (NOTSET(cursor_down)) {
	if (!NOTSET(linefeed_if_not_lf)) 
	    cursor_down = linefeed_if_not_lf;
	else if (linefeed_is_newline != 1) {
	    if (new_line_delay > 0) {
		sprintf(buf, "%s$<%d>", C_LF, new_line_delay);
		cursor_down = save_str(buf);
	    } else
		cursor_down = save_str(C_LF);
	}
    }
    if (NOTSET(scroll_forward) && crt_without_scrolling != 1) {
	if (!NOTSET(linefeed_if_not_lf)) 
	    cursor_down = linefeed_if_not_lf;
	else if (linefeed_is_newline != 1) {
	    if (new_line_delay > 0) {
		sprintf(buf, "%s$<%d>", C_LF, new_line_delay);
		scroll_forward = save_str(buf);
	    } else
		scroll_forward = save_str(C_LF);
	}
    }
    if (NOTSET(newline)) {
	if (linefeed_is_newline == 1) {
	    if (new_line_delay > 0) {
		sprintf(buf, "%s$<%d>", C_LF, new_line_delay);
		newline = save_str(buf);
	    } else
		newline = save_str(C_LF);
	} else if (!NOTSET(carriage_return) && !NOTSET(scroll_forward)) {
	    strncpy(buf, carriage_return, MAX_LINE-2);
	    buf[MAX_LINE-1] = '\0';
	    strncat(buf, scroll_forward, MAX_LINE-strlen(buf)-1);
	    buf[MAX_LINE] = '\0';
	    newline = save_str(buf);
	} else if (!NOTSET(carriage_return) && !NOTSET(cursor_down)) {
	    strncpy(buf, carriage_return, MAX_LINE-2);
	    buf[MAX_LINE-1] = '\0';
	    strncat(buf, cursor_down, MAX_LINE-strlen(buf)-1);
	    buf[MAX_LINE] = '\0';
	    newline = save_str(buf);
	}
    }

    /*
     * We wait until now to decide if we've got a working cr because even
     * one that doesn't work can be used for newline. Unfortunately the
     * space allocated for it is wasted.
     */
    if (return_does_clr_eol == 1 || no_correctly_working_cr == 1)
	carriage_return = NULL;

    /*
     * supposedly most termcap entries have ta now and '\t' is no longer a
     * default, but it doesn't seem to be true...
     */
    if (NOTSET(tab)) {
	if (horizontal_tab_delay > 0) {
	    sprintf(buf, "%s$<%d>", C_HT, horizontal_tab_delay);
	    tab = save_str(buf);
	} else
	    tab = save_str(C_HT);
    }

    if (init_tabs == -1 && has_hardware_tabs == 1)
	init_tabs = 8;

    if (!hard_copy)
    {
	if (NOTSET(key_backspace))
	    key_backspace = save_str(C_BS);
	if (NOTSET(key_left))
	    key_left = save_str(C_BS);
	if (NOTSET(key_down))
	    key_down = save_str(C_LF);
    }
}
