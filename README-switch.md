# Black Shades on the Nintendo Switch (homebrew)

A devkitPro/libnx build target for the SDL2 port. It builds a `blackshades.nro`
for hbmenu with the game's `Data` folder packed inside it.

**This has never run on a console.** It was written and checked without a
Switch, without the devkitPro toolchain (the container it was written in cannot
reach devkitPro's servers), and so the build itself has not been compiled once.
Every Switch code path *has* been type-checked against stub headers, and the
Mac and Linux builds were rebuilt and played afterwards to confirm nothing
moved for them. Treat this as a well-formed starting point that needs a first
compile and a first boot, not as a finished port. The section at the bottom
lists what is most likely to need fixing.

## Building

```sh
sudo dkp-pacman -S switch-dev switch-sdl2 switch-mesa switch-libdrm_nouveau
export DEVKITPRO=/opt/devkitpro
make switch            # or: make -f Makefile.switch
```

That leaves `blackshades.nro` in the source folder. Copy it to
`/switch/blackshades/blackshades.nro` on the SD card and start it from hbmenu.

`make switch` only runs `Makefile.switch`, which has its own object directory
(`build-switch`), its own flags and its own source list. The desktop Makefile
is untouched apart from that one delegating target, so the two builds cannot
disturb each other.

Build with `make -f Makefile.switch NXLINK=1` and run
`nxlink -s blackshades.nro` to get the game's output over the network. That is
the fastest way to see what the first boot says. It prints the settings it loaded
from the SD card as it starts (`Settings: view distance 1, density 1,
assassins 1, face buttons ...`), which is the quickest way to tell whether an
edit to config.txt actually took.

For an icon, put a 256x256 JPEG at `macos/switch-icon.jpg`; otherwise the .nro
gets devkitPro's default.

## Where things live

| | |
| --- | --- |
| `Data` | inside the .nro as romfs, at `romfs:/Data` (the build stages it under `build-switch/romfs` first, because elf2nro packs the contents of the folder it is given) |
| `config.txt`, `Highscore`, `HighscoreRevenge` | `sdmc:/switch/blackshades/` |

Saves are committed to the SD card after every write, so pulling the card or
crashing right after a mission does not lose the score.

## Controls

| Control | Action |
| --- | --- |
| Left stick | walk; all the way over, run. In the menu it moves the pointer |
| Right stick | look around |
| Right trigger (ZR, RT, R2) | click: fire while aiming, smash while running, pick up a gun while crouched over a body, otherwise disarm |
| Left trigger (ZL, LT, L2) | crouch, which is also the sniper scope |
| Bottom button (A) | dive, and click in the menu |
| Left button (X) | reload |
| Top button (Y) | aim and un-aim (you shoot while aiming and pick guns up while not) |
| Right button (B) | laser sight on and off |
| Left shoulder (L) | psychic aim - hold it for the slow motion |
| Right shoulder (R) | soul release on and off |
| D-pad up | the debug weapon key, which does nothing unless debug is on |
| Start (+) | the menu, as Esc |
| Back (−) | save and quit to hbmenu |
| Touch screen | put the pointer where you touched, and click |

Buttons are named by **position**, with SDL's Xbox-style letters beside them:
the bottom button is A on an Xbox pad and B on a Nintendo one. Which of the
two SDL reports depends on the driver - macOS hands a Nintendo pad over by its
labels, while the Switch's own driver reports positions - so the game works it
out per controller and says which at startup:

```
Controller: Pro Controller - face buttons swapped: the bottom button is SDL's B
```

If it guesses wrong, `Controller face buttons` in config.txt overrides it: 0
takes SDL's letters as positions, 1 swaps A/B and X/Y, and -1 (the default)
leaves it to the game. Each machine has its own config.txt, so a Mac and a
Switch can disagree.

This is the same mapping the Mac and Linux builds use with a controller. The
game is built around a mouse, so the right stick feeds mouse motion and the
menu gets an invented pointer.

## What the port does

* **OpenGL.** devkitPro's Mesa has no `libGL` to link against, so every `gl*`
  call goes through a pointer loaded at startup by glad (generated for the 2.1
  compatibility profile, vendored in `Source/thirdparty/glad/`). The game asks
  SDL for a compatibility-profile context and falls back to whatever the driver
  offers, then checks that the fixed-function entry points it lives on are
  really there.
* **Screen.** The window is the screen — 1280x720 handheld, 1920x1080 docked —
  and docking is a resize, which the port already handles.
* **Memory.** hbmenu leaves homebrew a few hundred MB and each person in the
  crowd is about 125 KB, so the crowd is capped at 112 MB of people (about 900)
  no matter what View distance and Population density say in `config.txt`.
* **Audio.** Unchanged: the built-in mixer runs on SDL's audio device, which on
  the Switch is libnx's.

## What will probably need fixing first

1. **Whether fixed-function OpenGL exists at all.** This is the one that
   decides whether the port is half a day of fixes or a renderer rewrite. If
   the driver has no compatibility profile, the game exits at startup saying
   which calls are missing and what `GL_VERSION` it got — read that over
   nxlink. The fix in that case is not a Switch fix: it is the fixed-function
   shim over a modern backend that the renderer needs anyway.
2. **Memory.** Started from hbmenu normally, homebrew gets a few hundred MB.
   If it runs out, launch hbmenu over a game title (hold R while starting a
   game) for the full allocation, or lower `Population density` in
   `config.txt`. The crowd allocation already halves the density until it
   fits rather than failing.
3. **Frame rate.** The Tegra X1 is much slower than a Mac, and the default
   crowd is 810 people in clear weather. Expect to drop `View distance` to
   0.5–1 and `Population density` below 1.
4. **Button positions.** If SDL's Switch mapping differs from the assumption
   above, the buttons will be rotated; the mapping is one table at the top of
   the controller section in `GameLoop.cpp`.
5. **Aiming feel.** `pad_look` in `GameLoop.cpp` is the look speed, about 185
   degrees a second at the edge of the stick, squared so that small pushes aim
   finely. Measured with a synthetic controller on a desktop, so it will want
   tuning by hand.
6. **The applet lifecycle.** Suspending and resuming, and hbmenu's own exit
   path, have not been exercised. The game quits through its normal shutdown
   (which saves the high score) on SDL_QUIT and on the − button.

Not attempted: gyro aiming, rumble, per-mode (docked/handheld) settings, and
anything to do with distributing the result — the source is under the uDevGame
Limited Use License, and the music is separately licensed.
