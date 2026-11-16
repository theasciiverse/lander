# Lander Makefile

LIBS := -lcurses -lm

OBJS := land.o screen.o move.o score.o state.o
SRC := land.c screen.c move.c score.c state.c
INC := consts.h funcs.h
BIN := /usr/local/bin
OPT := -O2
CFLAGS := -Wall $(OPT)

lander: $(OBJS)
	$(CC) $(LDFLAGS) -o lander $(OBJS) $(LIBS)

# the formatted man page; needs nroff (groff-base), so not part of the build
man: lander.man

$(OBJS): consts.h funcs.h

lander.man: lander.6
	nroff -man lander.6 > lander.man

clean:
	rm -f $(OBJS) lander lander.man

install: lander
	sudo rm -f $(BIN)/lander
	sudo cp lander $(BIN)
	sudo chown stacey $(BIN)/lander
