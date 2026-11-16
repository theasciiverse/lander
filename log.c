/*
** log.c (Asciiverse) - a session log for bug reports. Set LANDER_LOG to
** anything and lander appends to lander.log in its state folder (see
** state.c): every key as it arrives, each landing's start and end, game
** over, resizes, and the lander's state once a tick. Without it nothing
** is written.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <curses.h>
#include "funcs.h"

static FILE *LogFp;

void
LogOpen(void)
{
	const char *path;

	if (getenv("LANDER_LOG") == NULL)
		return;
	if ((path = StatePath("lander.log")) != NULL)
		LogFp = fopen(path, "a");
	if (LogFp != NULL)
		setvbuf(LogFp, NULL, _IOLBF, BUFSIZ);
	Log("session start, terminal %dx%d", COLS, LINES);
}

void
Log(const char *fmt, ...)
{
	va_list ap;
	time_t now;
	char stamp[16];

	if (LogFp == NULL)
		return;
	now = time(NULL);
	strftime(stamp, sizeof(stamp), "%H:%M:%S", localtime(&now));
	fprintf(LogFp, "%s ", stamp);
	va_start(ap, fmt);
	vfprintf(LogFp, fmt, ap);
	va_end(ap);
	fputc('\n', LogFp);
}
