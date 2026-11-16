/*
** state.c (Asciiverse) - the player's state folder, where lander keeps its
** high score file: $XDG_STATE_HOME/lander, else ~/.local/state/lander, or
** %LOCALAPPDATA%\lander on Windows. The folders are made as needed.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#define MKDIR(p) mkdir(p, 0755)
#endif
#include <curses.h>
#include "funcs.h"

/*
** StatePath() - the full name of file in the state folder, made if need be,
** or NULL when there is no home folder to put it in.
*/

const char *
StatePath(const char *file)
{
	static char path[1024];
	const char *base;
	char *p;

#ifdef _WIN32
	if ((base = getenv("LOCALAPPDATA")) != NULL && *base)
		snprintf(path, sizeof(path), "%s\\lander", base);
	else
		return NULL;
#else
	if ((base = getenv("XDG_STATE_HOME")) != NULL && *base == '/')
		snprintf(path, sizeof(path), "%s/lander", base);
	else if ((base = getenv("HOME")) != NULL && *base)
		snprintf(path, sizeof(path), "%s/.local/state/lander", base);
	else
		return NULL;
#endif
	for (p = path + 1; *p != '\0'; p++)
		if (*p == '/' || *p == '\\')
		{
			char c = *p;

			*p = '\0';
			MKDIR(path);
			*p = c;
		}
	MKDIR(path);
#ifdef _WIN32
	strncat(path, "\\", sizeof(path) - strlen(path) - 1);
#else
	strncat(path, "/", sizeof(path) - strlen(path) - 1);
#endif
	strncat(path, file, sizeof(path) - strlen(path) - 1);
	return path;
}
