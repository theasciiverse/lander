#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include "consts.h"
#include "funcs.h"

#define ALT_ADD(screen, y, x, ch) \
	wmove(screen, y, x); \
	waddch(screen, ch);
#define STAND_ADD(screen, y, x, ch) \
	wmove(screen, y, x); \
	wstandout(screen); \
	waddch(screen, ch); \
	wstandend(screen);
#define STAND_ALT_ADD(screen, y, x, ch) \
	wstandout(screen); \
	ALT_ADD(screen, y, x, ch); \
	wstandend(screen);

const char *Template[] = {
	"                                                                            ",
	".                                                                          /",
	"<                                                                        /v+",
	"<                                                                       /+++",
	"<                                                                     /v++++",
	"<                            /====.                                   >+++++",
	"<                            `v30v<                                  /++++++",
	"<                             >++++.                                 >++++++",
	"<                             >++++<                                /+++++++",
	"<                             >++++<                               /++++++++",
	"+.                            >+++++.                              `++++++++",
	"+<                           /++++++<                               `+++++++",
	"+<                          /+++++++<                                >++++++",
	"+<                          >+++++++<                          /v.   >++++++",
	"+<                          >+++++++<                         /+++.  >++++++",
	"+<                          >+++++++'                         >+++<  >++++++",
	"++====.                /====+++++++<                          >+++<  >++++++",
	"++v30v<            /vvv+v15v+++++++<          /v.           /v++++<  >++++++",
	"+++++++v.       /vv++++++++++++++++<        /v+++====.      >++++++==+++++++",
	"+++++++++=======+++++++++++++++++++<      /v+++++v20v+======+++++++50+++++++",
	"+++++++++vv10vvv++++++++++++++++++++======++++++++++++vv15vv++++++++++++++++",
	"++++++++++++++++++++++++++++++++++++vv25vv++++++++++++++++++++++++++++++++++"
};

const int PadScore[MAX_PADS] = {30, 30, 15, 20, 50, 10, 15, 25};

int LastLegalY, LastLegalX;

typedef chtype chtype_port_t;
static chtype_port_t LineMap[128] = { 0 };

static int Old_Y, Old_X;

#define L0 "Lunar Lander"
#define L1 "The classic arcade game comes to Unix."
#define L2 "Controls:"
#define L3 "'0'-'9' - power level"
#define L4 "'z' - set left retro rocket to power level"
#define L5 "'x' - set vertical thrust to power level"
#define L6 "'c' - set right retro to power level"
#define L7 "--press space to start game--"
#define L8 "Stacey Campbell at HCR, 1989"
#define CENTRE(win, line, str) mvwaddstr(win, line, (SCREEN_COLS - sizeof(str)) / 2, str)

/*
** The display is a pad of SCREEN_ROWS x SCREEN_COLS, drawn centred in the
** terminal (Asciiverse). A pad, unlike a window, keeps what is on it when
** the terminal shrinks, so after a resize it is simply shown again in the
** new middle. Score tables and messages are smaller pads over it.
*/

static WINDOW *Pad, *Overlay;
static int OrgY, OrgX, OverY, OverX;

void
ScRefresh(void)
{
	int rows, cols;

	pnoutrefresh(Pad, 0, 0, OrgY, OrgX, OrgY + SCREEN_ROWS - 1, OrgX + SCREEN_COLS - 1);
	if (Overlay != NULL)
	{
		getmaxyx(Overlay, rows, cols);
		pnoutrefresh(Overlay, 0, 0, OrgY + OverY, OrgX + OverX, OrgY + OverY + rows - 1, OrgX + OverX + cols - 1);
	}
	doupdate();
}

void
ScOverlay(WINDOW *win, int y, int x)
{
	Overlay = win;
	OverY = y;
	OverX = x;
}

/*
** Recentre() - after a resize, show the display in the middle of the
** terminal; while it is smaller than the display, say so and wait.
*/

static void
Recentre(void)
{
#ifdef PDCURSES
	resize_term(0, 0);
#endif
	while (LINES < SCREEN_ROWS || COLS < SCREEN_COLS)
	{
		werase(stdscr);
		mvwprintw(stdscr, 0, 0, "Lander needs a terminal of %dx%d,", SCREEN_COLS, SCREEN_ROWS);
		mvwprintw(stdscr, 1, 0, "this one is %dx%d.", COLS, LINES);
		mvwaddstr(stdscr, 2, 0, "Make it bigger to go on.");
		wrefresh(stdscr);
		if (wgetch(stdscr) == KEY_RESIZE)
		{
#ifdef PDCURSES
			resize_term(0, 0);
#endif
		}
	}
	OrgY = (LINES - SCREEN_ROWS) / 2;
	OrgX = (COLS - SCREEN_COLS) / 2;
	werase(stdscr);
	wnoutrefresh(stdscr);
	touchwin(Pad);
	if (Overlay != NULL)
		touchwin(Overlay);
	ScRefresh();
}

/*
** GetKey() - wgetch() for the display: shows it first, as wgetch() does
** for a window, and recentres it on a resize.
*/

int
GetKey(WINDOW *win)
{
	int ch;

	for (;;)
	{
		ScRefresh();
		if ((ch = wgetch(win)) != KEY_RESIZE)
			return ch;
		Recentre();
	}
}

static void
Introduction(WINDOW *screen)
{
	werase(screen);
	CENTRE(screen, 2, L0);
	CENTRE(screen, 5, L1);
	CENTRE(screen, 9, L2);
	CENTRE(screen, 11, L3);
	CENTRE(screen, 12, L4);
	CENTRE(screen, 13, L5);
	CENTRE(screen, 14, L6);
	CENTRE(screen, 18, L8);
	CENTRE(screen, 20, L7);
	while (GetKey(screen) != ' ');
}

static void
zap(int sig)
{
	endwin();
#ifndef _WIN32
	fcntl(fileno(stdin), F_SETFL, 0);
#endif
	exit(1);
}

#ifndef _WIN32
static void
dumpcore(int sig)
{
	fcntl(fileno(stdin), F_SETFL, 0);
	endwin();
	signal(SIGQUIT, SIG_DFL);
	kill(getpid(), SIGQUIT);
	pause();
}
#endif

void
InitialiseScreen(WINDOW **init_scr)
{
	register WINDOW *screen;

	signal(SIGINT, zap);
#ifndef _WIN32
	signal(SIGQUIT, dumpcore);
#endif
	initscr();
	noecho();
	cbreak();
	keypad(stdscr, TRUE);
	*init_scr = Pad = newpad(SCREEN_ROWS, SCREEN_COLS);
	screen = *init_scr;
	keypad(screen, TRUE);
	Recentre();
#if 0
	LineMap['\''] = '+';
	LineMap['.'] = '+';
	LineMap['|'] = '|';
	LineMap['-'] = '-';
	LineMap['/'] = '+';
	LineMap['`'] = '+';
	LineMap['+'] = '+';
	LineMap['>'] = '+';
	LineMap['<'] = '+';
	LineMap['^'] = '+';
	LineMap['v'] = '+';
	LineMap[PAD] = '=';
#else
	LineMap['\''] = ACS_LRCORNER;
	LineMap['.'] = ACS_URCORNER;
	LineMap['|'] = ACS_VLINE;
	LineMap['-'] = ACS_HLINE;
	LineMap['/'] = ACS_ULCORNER;
	LineMap['`'] = ACS_LLCORNER;
	LineMap['+'] = ACS_PLUS;
	LineMap['>'] = ACS_LTEE;
	LineMap['<'] = ACS_RTEE;
	LineMap['^'] = ACS_BTEE;
	LineMap['v'] = ACS_TTEE;
	LineMap[PAD] = ACS_HLINE;
#endif
	Introduction(screen);
}

void
DrawScreen(WINDOW *screen)
{
	register int i, j, scr_i;
	chtype_port_t map;
	const char *line;
	char ch;

	werase(screen);
	for (i = 0; i < SCR_Y; ++i)
	{
		line = Template[i];
		scr_i = i + 1;
		for (j = 0; j < SCR_X; ++j)
		{
			ch = *(line + j);
			map = LineMap[(unsigned char)ch];
			if (map)
				if (ch == PAD)
				{
					STAND_ALT_ADD(screen, scr_i, j, map);
				}
				else
				{
					ALT_ADD(screen, scr_i, j, map);
				}
			else if (ch == PAD)
			{
				STAND_ADD(screen, scr_i, j, ch);
			}
			else
				mvwaddch(screen, scr_i, j, ch);
		}
	}
	ScRefresh();
	LastLegalY = SCR_ADJ(0);
	LastLegalX = 0;
	Old_Y = Old_X = -1;
}

int
MoveLander(WINDOW *screen, double land_y, double land_x)
{
	int y, x, touchup = 0, screen_y, new_legal;
	double y_real, x_real;
	char ch;

	y_real = (ALTITUDE_INIT - land_y) / ALTITUDE_INIT * (double)SCR_Y;
	x_real = land_x / LANDSCAPE_WIDTH * (double)SCR_X;
	y = y_real + 0.5;
	x = x_real + 0.5;
	new_legal = LEGAL_YX(y, x);
	if (y != Old_Y || x != Old_X)
	{
		if (LEGAL_YX(Old_Y, Old_X))
		{
			mvwaddch(screen, SCR_ADJ(Old_Y), Old_X, '.');
			touchup = 1;
		}
		if (new_legal)
		{
			screen_y = SCR_ADJ(y);
			ALT_ADD(screen, screen_y, x, ACS_TTEE);
			wmove(screen, screen_y, x);
			LastLegalY = y;
			LastLegalX = x;
			touchup = 1;
		}
	}
	if (touchup)
		ScRefresh();
	Old_Y = y;
	Old_X = x;
	if (new_legal)
	{
		ch = *(Template[y] + x);
		if (ch == PAD)
			return LANDED;
		if (ch != ' ')
			return CRASH;
	}
	else
		return CRASH;
	return FLYING;
}
