/*
 * $Id: demo_forms.c,v 1.10 2005/03/06 00:27:27 tom Exp $
 *
 * Demonstrate a variety of functions from the form library.
 * Thomas Dickey - 2003/4/26
 */
/*
TYPE_ALNUM			-
TYPE_ENUM			-
TYPE_REGEXP			-
dup_field			-
field_arg			-
field_back			-
field_count			-
field_fore			-
field_init			-
field_just			-
field_opts_on			-
field_pad			-
field_status			-
field_term			-
field_type			-
form_init			-
form_opts			-
form_opts_off			-
form_opts_on			-
form_page			-
form_request_by_name		-
form_term			-
form_userptr			-
free_fieldtype			-
link_field			-
link_fieldtype			-
move_field			-
new_page			-
pos_form_cursor			-
set_current_field		-
set_field_fore			-
set_field_init			-
set_field_pad			-
set_field_status		-
set_field_term			-
set_fieldtype_arg		-
set_fieldtype_choice		-
set_form_fields			-
set_form_init			-
set_form_opts			-
set_form_page			-
set_form_term			-
set_form_userptr		-
set_max_field			-
*/

#include <test.priv.h>

#if USE_LIBFORM

#include <edit_field.h>

static int d_option = 0;
static int m_value = 0;
static int o_value = 0;

static FIELD *
make_label(int frow, int fcol, NCURSES_CONST char *label)
{
    FIELD *f = new_field(1, strlen(label), frow, fcol, 0, 0);

    if (f) {
	set_field_buffer(f, 0, label);
	set_field_opts(f, field_opts(f) & ~O_ACTIVE);
    }
    return (f);
}

/*
 * Define each field with an extra one, for reflecting "actual" text.
 */
static FIELD *
make_field(int frow, int fcol, int rows, int cols)
{
    FIELD *f = new_field(rows, cols, frow, fcol, o_value, 1);

    if (f) {
	set_field_back(f, A_UNDERLINE);
	set_field_userptr(f, (void *) 0);
	if (d_option) {
	    field_opts_off(f, O_STATIC);
	    set_max_field(f, m_value);
	}
    }
    return (f);
}

static void
display_form(FORM * f)
{
    WINDOW *w;
    int rows, cols;

    scale_form(f, &rows, &cols);

    /*
     * Put the form at the upper-left corner of the display, with just a box
     * around it.
     */
    if ((w = newwin(rows + 2, cols + 4, 0, 0)) != (WINDOW *) 0) {
	set_form_win(f, w);
	set_form_sub(f, derwin(w, rows, cols, 1, 2));
	box(w, 0, 0);
	keypad(w, TRUE);
    }

    if (post_form(f) != E_OK)
	wrefresh(w);
}

static void
erase_form(FORM * f)
{
    WINDOW *w = form_win(f);
    WINDOW *s = form_sub(f);

    unpost_form(f);
    werase(w);
    wrefresh(w);
    delwin(s);
    delwin(w);
}

static int
my_form_driver(FORM * form, int c)
{
    switch (c) {
    case EDIT_FIELD('q'):
	if (form_driver(form, REQ_VALIDATION) == E_OK)
	    return (TRUE);
	break;
    case EDIT_FIELD('h'):
	help_edit_field();
	break;
    default:
	beep();
	break;
    }
    return (FALSE);
}

static void
show_current_field(WINDOW *win, FORM * form)
{
    FIELD *field;
    FIELDTYPE *type;
    char *buffer;
    int nbuf;
    int field_rows, field_cols, field_max;

    if (has_colors()) {
	wbkgd(win, COLOR_PAIR(1));
    }
    werase(win);
    wprintw(win, "Cursor: %d,%d", form->currow, form->curcol);
    if (data_ahead(form))
	waddstr(win, " ahead");
    if (data_behind(form))
	waddstr(win, " behind");
    waddch(win, '\n');
    if ((field = current_field(form)) != 0) {
	wprintw(win, "Field %d:", field_index(field));
	if ((type = field_type(field)) != 0) {
	    if (type == TYPE_ALNUM)
		waddstr(win, "ALNUM");
	    else if (type == TYPE_ALPHA)
		waddstr(win, "ALPHA");
	    else if (type == TYPE_ENUM)
		waddstr(win, "ENUM");
	    else if (type == TYPE_INTEGER)
		waddstr(win, "INTEGER");
	    else if (type == TYPE_IPV4)
		waddstr(win, "IPV4");
	    else if (type == TYPE_NUMERIC)
		waddstr(win, "NUMERIC");
	    else if (type == TYPE_REGEXP)
		waddstr(win, "REGEXP");
	    else
		waddstr(win, "other");
	}
	if (dynamic_field_info(field, &field_rows, &field_cols, &field_max)
	    != ERR) {
	    wprintw(win, " size %dx%d (max %d)",
		    field_rows, field_cols, field_max);
	}
	waddstr(win, "\n");
	for (nbuf = 0; nbuf <= 2; ++nbuf) {
	    if ((buffer = field_buffer(field, nbuf)) != 0) {
		wprintw(win, "buffer %d:", nbuf);
		wattrset(win, A_REVERSE);
		waddstr(win, buffer);
		wattroff(win, A_REVERSE);
		waddstr(win, "\n");
	    }
	}
    }
    wrefresh(win);
}

static void
demo_forms(void)
{
    WINDOW *w;
    FORM *form;
    FIELD *f[100];
    int finished = 0, c;
    unsigned n = 0;
    int pg;
    WINDOW *also;

    help_edit_field();

    mvaddstr(4, 57, "Forms Entry Test");

    refresh();

    /* describe the form */
    for (pg = 0; pg < 4; ++pg) {
	char label[80];
	sprintf(label, "Sample Form Page %d", pg + 1);
	f[n++] = make_label(0, 15, label);
	set_new_page(f[n - 1], TRUE);

	switch (pg) {
	default:
	    f[n++] = make_label(2, 0, "Last Name");
	    f[n++] = make_field(3, 0, 1, 18);
	    set_field_type(f[n - 1], TYPE_ALPHA, 1);

	    f[n++] = make_label(2, 20, "First Name");
	    f[n++] = make_field(3, 20, 1, 12);
	    set_field_type(f[n - 1], TYPE_ALPHA, 1);

	    f[n++] = make_label(2, 34, "Middle Name");
	    f[n++] = make_field(3, 34, 1, 12);
	    set_field_type(f[n - 1], TYPE_ALPHA, 1);
	    break;
	case 2:
	    f[n++] = make_label(2, 0, "Host Name");
	    f[n++] = make_field(3, 0, 1, 18);
	    set_field_type(f[n - 1], TYPE_ALPHA, 1);

	    f[n++] = make_label(2, 20, "IP Address");
	    f[n++] = make_field(3, 20, 1, 12);
	    set_field_type(f[n - 1], TYPE_IPV4, 1);

	    break;

	case 3:
	    f[n++] = make_label(2, 0, "Four digits");
	    f[n++] = make_field(3, 0, 1, 18);
	    set_field_type(f[n - 1], TYPE_INTEGER, 4, 0, 0);

	    f[n++] = make_label(2, 20, "Numeric");
	    f[n++] = make_field(3, 20, 1, 12);
	    set_field_type(f[n - 1], TYPE_NUMERIC, 3, -10000.0, 100000000.0);

	    break;
	}

	f[n++] = make_label(5, 0, "Comments");
	f[n++] = make_field(6, 0, 4, 46);
    }

    f[n++] = (FIELD *) 0;

    form = new_form(f);

    display_form(form);

    w = form_win(form);
    also = newwin(getmaxy(stdscr) - getmaxy(w), COLS, getmaxy(w), 0);
    show_current_field(also, form);

    while (!finished) {
	switch (edit_field(form, &c)) {
	case E_OK:
	    break;
	case E_UNKNOWN_COMMAND:
	    finished = my_form_driver(form, c);
	    break;
	default:
	    beep();
	    break;
	}
	show_current_field(also, form);
    }

    erase_form(form);

    free_form(form);
    for (c = 0; f[c] != 0; c++)
	free_field(f[c]);
    noraw();
    nl();
}

static void
usage(void)
{
    static const char *tbl[] =
    {
	"Usage: demo_forms [options]"
	,""
	," -d        make fields dynamic"
	," -m value  set maximum size of dynamic fields"
	," -o value  specify number of offscreen rows in new_field()"
    };
    unsigned int j;
    for (j = 0; j < SIZEOF(tbl); ++j)
	fprintf(stderr, "%s\n", tbl[j]);
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    int ch;

    setlocale(LC_ALL, "");

    while ((ch = getopt(argc, argv, "dm:o:")) != EOF) {
	switch (ch) {
	case 'd':
	    d_option = TRUE;
	    break;
	case 'm':
	    m_value = atoi(optarg);
	    break;
	case 'o':
	    o_value = atoi(optarg);
	    break;
	default:
	    usage();

	}
    }

    initscr();
    cbreak();
    noecho();
    raw();
    nonl();			/* lets us read ^M's */
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);

    if (has_colors()) {
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLUE);
	bkgd(COLOR_PAIR(1));
	refresh();
    }

    demo_forms();

    endwin();
    ExitProgram(EXIT_SUCCESS);
}
#else
int
main(void)
{
    printf("This program requires the curses form library\n");
    ExitProgram(EXIT_FAILURE);
}
#endif
