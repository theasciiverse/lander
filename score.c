#include <stdio.h>
#include <curses.h>
#include <string.h>
#include <stdlib.h>
#include "consts.h"
#include "funcs.h"

#define HEADER "Scores"
#define NAME_LEN 20
#define HS_ENTRIES 10
#define SC_WIN_LEN 16
#define SPC_LINE (SC_WIN_LEN - 2)

typedef struct score_pad_t
{
	int y;
	int start_x;
	int end_x;
} score_pad_t;
typedef struct score_t
{
	int score;
	char name[NAME_LEN];
} score_t;

extern double Fuel;
extern const int PadScore[];
extern int BSLandings, Landings;
extern const char *Template[];
extern int LastLegalY, LastLegalX;

score_pad_t ScorePad[MAX_PADS];
int Score;
int BestScore = 0;

#define HS_FILE "lander.hs"
static int TotalPads;

static int
ScCmp(const void *arg1, const void *arg2)
{
	const score_t *sc_rec1 = arg1;
	const score_t *sc_rec2 = arg2;
	
	if (sc_rec1->score < sc_rec2->score)
		return 1;
	if (sc_rec1->score > sc_rec2->score)
		return -1;
	return 0;
}

static void
ScDisplayErr(WINDOW *back_win, char *str)
{
	int old_y, old_x, win_len;
	WINDOW *err_win;

	getyx(back_win, old_y, old_x);
	wmove(back_win, 0, 0);
	ScRefresh();
	win_len = strlen(str) + 2;
	err_win = newpad(3, win_len);
	ScOverlay(err_win, SCREEN_ROWS / 2 - 2, (SCREEN_COLS - win_len - 2) / 2);
	box(err_win, 0, 0);
	mvwaddstr(err_win, 1, 1, str);
	ScRefresh();
	napms(3000);
	ScOverlay(NULL, 0, 0);
	delwin(err_win);
	wmove(back_win, old_y, old_x);
	touchwin(back_win);
	ScRefresh();
}

static void
PressSpace(WINDOW *back_win, WINDOW *cur_win, int y, int x, int sy, int sx)
{
	mvwaddstr(cur_win, sy, sx, "--press space--");
	while (GetKey(back_win) != ' ');
	ScOverlay(NULL, 0, 0);
	delwin(cur_win);
	wmove(back_win, y, x);
	touchwin(back_win);
	ScRefresh();
}

void
ScReadDisplay(WINDOW *back_win)
{
	int old_y, old_x, items, i;
	WINDOW *score_win;
	FILE *fp;
	const char *hs_file;
	score_t score_pad[HS_ENTRIES];

	hs_file = StatePath(HS_FILE);
	fp = hs_file ? fopen(hs_file, "rb") : NULL;
	getyx(back_win, old_y, old_x);
	wmove(back_win, 0, 0);
	ScRefresh();
	score_win = newpad(SC_WIN_LEN, 25);
	ScOverlay(score_win, 2, 5);
	werase(score_win);
	box(score_win, 0, 0);
	items = 0;
	if (fp != NULL)
	{
		items = fread((char *)score_pad, sizeof(score_t), HS_ENTRIES, fp);
		fclose(fp);
	}
	if (items == 0)
	{
		mvwaddstr(score_win, 5, 1, "HS file empty.");
		PressSpace(back_win, score_win, old_y, old_x, SPC_LINE, 1);
		return;
	}
	mvwaddstr(score_win, 1, (25 - sizeof(HEADER)) / 2, HEADER);
	ScRefresh();
	for (i = 0; i < items; ++i)
	{
		wmove(score_win, 3 + i, 1);
		wprintw(score_win, "%4d %s", score_pad[i].score, score_pad[i].name);
	}
	PressSpace(back_win, score_win, old_y, old_x, SPC_LINE, 1);
}

void
ScWrite(WINDOW *back_win)
{
	int items;
	FILE *fp;
	const char *hs_file, *user;
	score_t score_pad[HS_ENTRIES + 1];

	if (Score == 0)
		return;
	if ((hs_file = StatePath(HS_FILE)) == NULL)
	{
		ScDisplayErr(back_win, "No home folder for the HS file.");
		return;
	}
	items = 0;
	if ((fp = fopen(hs_file, "rb")) != NULL)
	{
		items = fread((char *)score_pad, sizeof(score_t), HS_ENTRIES, fp);
		fclose(fp);
	}
	if ((user = getenv("LOGNAME")) == NULL && (user = getenv("USER")) == NULL && (user = getenv("USERNAME")) == NULL)
		user = "player";
	snprintf(score_pad[items].name, NAME_LEN, "%s", user);
	score_pad[items].score = Score;
	++items;
	qsort((char *)score_pad, items, sizeof(score_t), ScCmp);
	if (items > HS_ENTRIES)
		items = HS_ENTRIES;
	if ((fp = fopen(hs_file, "wb")) == NULL)
	{
		ScDisplayErr(back_win, "Unable to write HS file.");
		return;
	}
	if (fwrite((char *)score_pad, sizeof(score_t), items, fp) == 0)
		ScDisplayErr(back_win, "No HS entries written.");
	fclose(fp);
}

void
UpdateScore(WINDOW *screen)
{
	int pad_i = 0, found = 0, fuel_bonus, diff_bonus;
	char scr_buf[128];

	while (pad_i < SCR_X && !found)
	{
		found = LastLegalX >= ScorePad[pad_i].start_x && LastLegalX <= ScorePad[pad_i].end_x && ScorePad[pad_i].y == LastLegalY;
		++pad_i;
	}
	--pad_i;
	fuel_bonus = Fuel / 100.0 + 0.5;
	sprintf(scr_buf, "Fuel bonus: %d", fuel_bonus);
	mvwaddstr(screen, 3, 25, scr_buf);
	ScRefresh();
	diff_bonus = (Landings + 1) * 10;
	sprintf(scr_buf, "Consecutive Landings Bonus: %d", diff_bonus);
	mvwaddstr(screen, 4, 9, scr_buf);
	ScRefresh();
	Score += PadScore[pad_i] + fuel_bonus + diff_bonus;
}

void
InitScore(void)
{
	int i, j, pad_count = 0;
	const char *line;

	for (i = 0; i < SCR_Y; ++i)
	{
		j = 0;
		line = Template[i];
		while (j < SCR_X)
		{
			if (line[j] == PAD)
			{
				ScorePad[pad_count].y = i;
				ScorePad[pad_count].start_x = j;
				while (j < SCR_X && line[j] == PAD)
					++j;
				if (j >= SCR_X)
					--j;
				ScorePad[pad_count].end_x = j;
				++pad_count;
			}
			++j;
		}
	}
	TotalPads = pad_count;
}
