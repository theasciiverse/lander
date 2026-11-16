# Changes from upstream

This is the Asciiverse packaging of Lander, Stacey Campbell's curses lunar
lander, written at HCR Corporation in Toronto in 1989 and posted to
comp.sources.games (volume 6, issue 70, 1 May 1989; a patch followed as
issue 78 on 21 June 1989). Upstream is
https://github.com/staceycampbell/lander, the `upstream` remote; this build
is its `master` at `e30c175` (2022-08-20). Upstream names no version, so
releases here are tagged `v1.0+20220820-av.N`. The `asciiverse` branch
carries the changes below, one commit each.

## The game

Unchanged: the same lunarscape, pads and pad scores, physics, one-second
tick, keys, texts, explosion, scoring and bonuses.

## Today's compilers

- **Signal handlers take the signal number.** GCC 15 defaults to C23,
  where `zap()` and `dumpcore()`, declared with an empty parameter list,
  take no arguments, and passing them to `signal()` is an error.

## Ours

- **High score file in the player's state folder.** It was a
  world-writable `/usr/local/games/lib/lander.hs` fixed at build time. It
  is now `lander.hs` in `$XDG_STATE_HOME/lander` (else
  `~/.local/state/lander`, `%LOCALAPPDATA%\lander` on Windows), the folders
  made as needed. A missing file reads as an empty table instead of three
  seconds of "Unable to display HS file." after a first game, and is made
  quietly with the first score. The name is `LOGNAME`, else `USER` or
  `USERNAME`, else "player", cut to the table's 19 characters (`strcpy()`
  could write past them); the file is opened in binary mode, as it holds
  raw records.
- **Centred, following a resize, and a message when the terminal is too
  small.** The 80x24 display is a curses pad drawn in the middle of the
  terminal and moved to the new middle after a resize (a pad keeps its
  contents when the terminal shrinks, so nothing is redrawn). Below 80x24
  the game waits with a message saying so. The score table and messages
  are small pads over the display, where they were. Keypad mode is on, so
  a cursor key counts as one unknown key (one flash) instead of three, and
  power levels are read as `0` to `9` instead of with `isdigit()`, which
  would see curses key codes above 255.
- **Builds without nroff.** The binary no longer depends on the formatted
  man page; `make man` makes it, and `lander.6` ships as it is. The link
  uses `$(CC)` and `$(LDFLAGS)`.
- **Windows.** Builds with MinGW against PDCurses: waits use curses'
  `napms()` instead of `sleep()` and `usleep()` (the same one-second tick),
  the explosion seeds with `time(NULL)`, `rand()` stands in for the
  `drand48()` family, and SIGQUIT and the `fcntl()` reset of stdin are
  left out there.
- **Session log behind `LANDER_LOG`.** Set it to anything and `lander.log`
  in the state folder gets every key as it arrives, the thrust and the
  lander's altitude, position, speeds and score each tick, each game's
  start, each landing or crash, running out of fuel, game over and
  resizes. Without it nothing is written.
- **How to play it, at the top of README.md.** A short note on installing
  and starting it with av; the original text follows unchanged.

## Packaging (new files)

- `asciiverse/build.sh`, `asciiverse/package.sh`: one compiler call, and
  one archive per target with the binary (`lander`), `lander.6`,
  README.md, LICENSE.md and PATCHES.md.
- `.github/workflows/release.yml`: static Linux in Alpine (x86_64,
  aarch64), macOS (Apple silicon and Intel) and Windows x86_64 (MinGW,
  PDCurses built wide), plus SHA256SUMS, on a `*-av.*` tag.
- `asciiverse/record-preview.sh`, `asciiverse/drive.py`,
  `asciiverse/lander.cast`: the catalog preview (100x30, 60 s) and the
  player that flew it: a landing on the 50 pad at the foot of the shaft,
  then a second, faster descent that hits the mesa pad too hard.
- `asciiverse/lander.toml`, `asciiverse/lander.md`: the catalog manifest
  and store page, as drafts for hub.
- `PATCHES.md`: this file.
