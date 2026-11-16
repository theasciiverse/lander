"""drive.py SOCKET SESSION: fly Lander in a tmux pane for the catalog preview.

asciiverse/record-preview.sh runs lander in a 100x30 tmux session and
records it; this script reads the pane and types the keys, as a player
would. Lander is deterministic: each one-second tick it reads the keys
typed since the last one, then moves the lander (land.c, move.c). So the
flights below were planned ahead against the game's own arithmetic and
are typed one tick at a time, each tick's keys sent as soon as the
previous tick has shown its new altitude on the top line.

Flight 1, the 50 pad at the foot of the shaft on the right: left rocket
full for three ticks to cross the moon in time, free fall, then the
right rocket full from tick 16 for eleven ticks and 5, 1 and 0.1 after
them, which leaves the lander hanging over the shaft; vertical thrust
full from tick 23 to brake, then 15 to sink down the shaft at under
5 m/s. It touches down on tick 37.

Flight 2 starts faster, as the game makes every next landing harder: one
tick of right rocket, then nothing, straight onto the 30 pad on the
mesa at 44 m/s. Game over, the score table, no other game, and out."""
import subprocess, sys, time

SOCK, S = sys.argv[1], sys.argv[2]
POWER = "0123456789"   # power levels 0 to 9: 0, .01, .1, .5, 1, 2, 5, 10, 15, 20
ROCKET = {"L": "z", "V": "x", "R": "c"}

def tmux(*a):
    return subprocess.run(["tmux", "-L", SOCK, *a], capture_output=True, text=True).stdout

def key(*k):
    tmux("send-keys", "-t", S, *k)

def screen():
    return tmux("capture-pane", "-p", "-t", S)

def wait_for(text, timeout=30):
    end = time.time() + timeout
    while time.time() < end:
        if text in screen():
            return
        time.sleep(0.05)
    raise SystemExit(f"never saw {text!r}")

def top_line():
    for row in screen().split("\n"):
        if "alt:" in row:
            return row.strip()
    return ""

def flight1(t):
    L = 9 if t <= 3 else 0
    R = {27: 6, 28: 4, 29: 2}.get(t, 9 if 16 <= t <= 26 else 0)
    V = 0 if t < 23 else (9 if t < 28 else 8)
    return {"L": L, "V": V, "R": R}

def flight2(t):
    return {"L": 0, "V": 0, "R": 9 if t == 1 else 0}

def fly(plan):
    """Types the plan's settings tick by tick until the game asks for space,
    after a landing or under the score table after a crash."""
    now = {"L": 0, "V": 0, "R": 0}
    t, last = 1, top_line()
    while True:
        want = plan(t)
        keys = "".join(POWER[want[r]] + ROCKET[r] for r in "LVR" if want[r] != now[r])
        if keys:
            key("-l", keys)
        now = want
        while True:   # the next tick shows on the top line
            s = screen()
            if "press space" in s:
                return
            line = top_line()
            if line and line != last:
                last = line
                break
            time.sleep(0.02)
        t += 1

wait_for("press space to start game")
time.sleep(0.6)
key("Space")
time.sleep(0.2)
fly(flight1)
time.sleep(1.6)
key("Space")
time.sleep(0.2)
fly(flight2)
time.sleep(1.6)
key("Space")
wait_for("another game?")
time.sleep(0.9)
key("n")
wait_for("Best Score")
time.sleep(1.1)
key("Space")
