#!/bin/sh
# Builds Lander into out/. The compiler and curses library come from the
# environment, so one script serves every target (see
# .github/workflows/release.yml):
#
#   asciiverse/build.sh                                        # Linux
#   LIBS=-lncurses LDFLAGS="-static -s" asciiverse/build.sh    # Alpine, static
#   LIBS=-lncurses asciiverse/build.sh                         # macOS
#   CC=x86_64-w64-mingw32-gcc CFLAGS="-O2 -DPDC_WIDE -DPDC_FORCE_UTF8 -I$PDC" \
#     LIBS="$PDC/wincon/pdcurses.a" LDFLAGS="-static -s" EXE=.exe asciiverse/build.sh
#
# PDCurses for Windows is built wide (make WIDE=Y UTF8=Y), so the line
# drawing comes out as Unicode box characters in any console code page.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
CFLAGS="${CFLAGS:--O2}"
LDFLAGS="${LDFLAGS:-}"
LIBS="${LIBS:--lncurses}"
EXE="${EXE:-}"
mkdir -p out
$CC $CFLAGS -Wall -I. land.c screen.c move.c score.c state.c log.c $LDFLAGS $LIBS -lm -o "out/lander$EXE"
echo "built lander$EXE"
