# Black Shades on modern macOS

This is the icculus.org SDL port of David Rosen's *Black Shades* (uDevGame 2002),
brought forward to current macOS: SDL2 instead of SDL 1.2, no GLU, no SDL_image,
no libvorbis, no OpenAL install, 64-bit clean, and packaged as a native
`BlackShades.app`. It also carries the enhancements from
[Black Shades Enhanced](https://github.com/Bitl/Black-Shades-Enhanced),
including its Revenge Mod, which is a second game mode on the main menu here
(see *Black Shades Enhanced and Revenge mode* below).

## Building

You need the Xcode Command Line Tools (`xcode-select --install`). Nothing else:
the app build downloads the official universal `SDL2.framework` and bundles it.

```sh
make app          # -> BlackShades.app for this Mac (arm64 on Apple Silicon)
open BlackShades.app
```

`make app UNIVERSAL=1` builds an arm64 + x86_64 fat binary instead, and
`make app ARCHS="arm64"` picks the slices by hand. The build prints what it
targeted, and `lipo -archs BlackShades.app/Contents/MacOS/BlackShades` confirms
it afterwards.

To build against the SDL2 you already have (Homebrew, MacPorts) instead:

```sh
make app USE_SYSTEM_SDL2=1
```

That builds for this machine's architecture only, and copies the SDL2 dylib it
linked against into the bundle (re-pointed with `install_name_tool`), so the
.app still runs without that Homebrew prefix. SDL2 is located via
`sdl2-config`, then `pkg-config`, then a look in `~/.homebrew`,
`/opt/homebrew`, `/usr/local` and `/opt/local`.

Note that Homebrew's `sdl2` is now **sdl2-compat**, a shim that implements the
SDL2 API on top of SDL3 and `dlopen`s SDL3 at run time (it looks for
`@loader_path/libSDL3.dylib`). Its own rpath points back into the Homebrew
prefix, which stops resolving as soon as the dylib is copied into an .app, so
the build detects the shim and bundles `libSDL3.dylib` beside it. Without that
you get `Fatal error! Cannot continue! Failed loading SDL3 library.` at launch.
If you would rather not run the game through the SDL3 translation layer at all,
plain `make app` uses the real SDL2 framework.

Other targets:

```sh
make              # plain ./blackshades for this machine
make run
make clean
make distclean    # also removes the downloaded SDL2 and the .app

make app USE_SYSTEM_OPENAL=1    # use OpenAL.framework instead of the built-in mixer
SDL2_VERSION=2.32.10 make sdl2  # pin a different SDL2
```

The same Makefile still builds and runs on Linux (`libsdl2-dev` and a GL
driver are all it needs).

## Where your files live

Everything the game writes now goes to
`~/Library/Application Support/Black Shades/` — an `.app` bundle is read-only
and code-signed, so writing inside it is not an option:

| File | What it is |
| --- | --- |
| `config.txt` | resolution, mouse sensitivity, blood, music, fullscreen, antialiasing… written on first run |
| `Highscore` | high score and "beat the game" flag |
| `HighscoreRevenge` | the same for revenge mode |
| `customlevels.txt` | optional; if present it overrides the bundled `Data/customlevels.txt` |

The game's read-only data is found next to the executable (inside the bundle
that is `Contents/Resources/Data`). `BLACKSHADES_DATA=/some/dir` overrides it.

`config.txt` gained twelve lines at the end: `Antialiasing (0, 2, 4 or 8)`,
`Start fullscreen`, from Black Shades Enhanced `Fps (frames per second)
limit` (default 300) and `FOV (Field of View)` (default 90), and `View
distance` (default 1), `Population density` (default 1), `Assassins`
(default 1), `Controller face buttons` (default -1), `Pointer aiming`
(default 0, and 1 on a Wii), `Sound` (default 1), `Widescreen on a Wii`
(default -1, which asks the console) and `Overscan` (default 0; the percent
of each edge a television hides, which everything is then drawn inside). A config file from before a line existed
still loads - the missing ones take their defaults - and is rewritten with the
new lines added.

### Pointer aiming

`Pointer aiming (1 = the pointer moves the gun and the view follows it; the
Wii's own way)` swaps the mouse from turning the view to moving a crosshair.
The gun points at the crosshair - exactly at it, since a perspective
projection turns a screen position into an angle - the view holds still while
the crosshair is in the middle 55% of the screen, and past that the view is
pushed round, faster the nearer the edge. The mouse is not captured in this
mode, because its position *is* the aim.

It is off by default and nothing changes while it is off. It exists for the
Wii, where the Wiimote is an absolute pointer, and it works with a mouse
anywhere - which is how it was developed and tuned. See `README-wii.md`.

### View distance

`View distance (1 = original, no upper limit; the fog, the crowd and the
assassins follow)` is one setting for everything that has to keep up with how
far you can see. It multiplies each weather's own view distance (sunny 2000,
snow 800, rain 700, fire 600, fog and night 500 units; the fog is solid at 80%
of it), how far away people are drawn (1000 units), and how far they are drawn
in full before turning into billboards (about 316 units, 1760 through the
scope). Anything from 0.5 up works, fractions included; below 1 only the view
and the fog come closer.

### The crowd

The city is populated out to a block or so past where anyone can be seen, with
the same number of people in every block: 10, the original's 90 people over
the nine blocks around you that it kept them in. (It bunched them towards
your own block - about 20 there, 5 or 6 in the corner blocks - and left every
block past those nine empty, even where you could see it.) How far that is
depends on the weather, so at a View distance of 1 the crowd is 250 in fog, at
night and in the fire, 490 in rain and snow, and 810 under clear skies.

The streets are full when a mission starts. After that nobody appears or
vanishes where you can see them: people only spawn out beyond what you can
see, into the blocks at the edge of the populated square that are short of
people, and someone is only taken away once they are past that edge and out of
sight - as you walk, the people you leave behind are moved to the blocks
coming up ahead. Measured at the defaults under clear skies: 810 people from
the first frame, 9 to 10 in every block, and no one spawned closer than 1069
units, against the 1000 out to which people are drawn. Where the square runs
past the edge of the city there are simply fewer people.

Everyone but you, the VIP and the zombies (whose speed follows their health,
as it always did) goes about at their own pace, picked when they spawn on a
bell curve: most people at the original's walk (7.2 units a
second), fewer the further from it, out to half that (3.6) on the slow side
and a full run (15) on the fast one, each three standard deviations out.
About two thirds walk at 6 to 9.8 - the animation slowed or quickened to
match - a quarter jog, and one in 75 is close to a full run. Running for a reason (civilians fleeing
gunfire, assassins closing in, zombies giving chase) is as fast as it always
was. `walk_pace` (the peak), `slowest_pace`, `run_pace` and `cruise_jog_pace`
(where walking turns into jogging) at the top of `GameTick.cpp` are the
knobs.

The cost is frame rate and memory: everyone is simulated every frame, and each
person takes about 125 KB. At density 1 the clear-weather crowd is 810 people
(100 MB) at a View distance of 1, 6250 (780 MB) at 4 and 34810 (4.3 GB) at 10;
the whole city, from about 17, is 98010 (12 GB). If the memory for a crowd
cannot be had, the game halves the density until it can.

### Population density and assassins

`Population density (1 = original; people per block, times this)` multiplies
the crowd: 2 puts 20 people in every block instead of 10 (about what the
original had right around you), 0.5 puts 5, and 0 leaves just you and the
VIP. There is no upper limit but memory (see above), and anything below 0
counts as 1.

`Assassins (1 = original; the share of the crowd that is one, times this)`
scales what fraction of the people on the street are assassins. The original's
odds differ from mission to mission - one person in 4, 5 or 6 - and 1 keeps
each mission's own odds exactly; 2 doubles the share, 0.5 halves it, 0 means
no assassins at all, and anything that would take the share past everyone is
capped there. Custom levels' own odds are scaled the same way. Zombie missions
are all zombies, whatever it says.

That share is deliberately *not* a function of View distance. What makes a
street feel like the original's is how many of the people on it are out to
kill you, so that is what is held fixed: seeing further puts proportionally
more assassins in the world because it puts proportionally more people in it,
and the crowd around you reads the same at any setting. Measured across view
distances 0.5 to 4, the assassin share comes out 16.1-18.6% on the 1-in-6
missions, against the original port's own 15.7%.

Two consequences worth knowing. A long View distance means more assassins
converging on the VIP from further out, so it is not quite difficulty-neutral
even though it is density-neutral - at 4 it runs a little hotter than the
original. And a platform that caps the crowd for memory, like the Wii, now
gets the right odds out of however many people it can afford; before this it
got the odds divided by the size of the square, which came to somewhere
between an eighth and a fifteenth of them depending on the weather and the
View distance.

## Controls

As the original (see `Readme`), plus:

| Key | Action |
| --- | --- |
| Alt+Enter, Cmd+Enter, Cmd+F | toggle fullscreen |
| Ctrl+G | release / re-grab the mouse pointer |
| Esc | main menu |
| Cmd+Q | quit (saves the high score) |

The pointer is captured while you play and released in the menu, when the
window loses focus, and on Ctrl+G.

### A game controller

Any controller SDL knows (Xbox, PlayStation, Switch Pro, 8BitDo and so on)
works alongside the keyboard and mouse, and can be plugged in while the game
is running. It is the only way to play on the Switch, where the mapping is the
same (plus − to save and quit).

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

The look speed is `pad_look` in `GameLoop.cpp`, squared so that small pushes
aim finely and the edge of the stick still whips round; the walk keys are
whichever ones config.txt binds.

## What changed, and why

**SDL 1.2 → SDL2.** `SDL_SetVideoMode` became a real window plus a GL context;
the window is resizable, Retina-aware (the viewport uses the drawable size
while the game keeps working in points), and `vblsync` in config.txt now drives
`SDL_GL_SetSwapInterval`. Fullscreen is borderless-desktop rather than a mode
switch.

**Keyboard by scancode.** The game speaks Mac virtual key codes, which describe
key *positions*; SDL2 keycodes are layout-dependent, so the key map is built
from SDL scancodes. WASD therefore stays under the same fingers on an AZERTY
keyboard, as it did on a real Mac, and the `Azerty keyboard` line in
config.txt (which swapped to the ZQSD positions for ports that read
characters) is now ignored. Keys are released when the window loses focus, so
you no longer keep running after Cmd-Tab.

**Mouse.** Mouse-look uses `SDL_SetRelativeMouseMode` instead of warping the
pointer around the window.

**OpenGL.** `<OpenGL/gl.h>` with `GL_SILENCE_DEPRECATION`, and GLU is gone:
`gluPerspective` is four lines of `glFrustum`, `gluBuild2DMipmaps` is
`GL_GENERATE_MIPMAP`. The renderer is still fixed-function OpenGL 1.x, which
macOS still runs (on Apple Silicon it goes through Metal). Colour buffer is now
8 bits per channel with a 24-bit depth buffer instead of 5/5/5/16, and
multisampling is available from config.txt.

**Textures without SDL_image.** PNG loading is stb_image, vendored in
`Source/thirdparty/`. The old loader also freed `SDL_Surface`s with `delete`
and fed `GL_BGRA` to `gluBuild2DMipmaps` as an internal format; both are gone.

**Audio without OpenAL or libvorbis.** Ogg decoding is stb_vorbis. OpenAL is
replaced by `Source/MiniAL.cpp`, a small OpenAL-1.1 subset (about 600 lines) on
top of SDL2's audio device, implementing exactly what the game uses: 3D
inverse-distance-clamped attenuation, `AL_MIN_GAIN`/`AL_MAX_GAIN` clamping
applied *after* attenuation, pitch, looping, pan, and the source state machine.

That gain clamp is not a detail: every piece of music and every "2D" sound in
Black Shades is a mono source parked at the world origin with `AL_MIN_GAIN 1`,
so without spec-correct clamping the music fades out as you walk away from the
middle of the city. Apple's OpenAL.framework has been deprecated since macOS
10.15, and OpenAL Soft would mean a Homebrew or CMake dependency for everyone
who builds this, so the mixer is built in. `USE_SYSTEM_OPENAL=1` still switches
back to the platform's OpenAL — the headers are signature-compatible.

**64-bit and lifetime fixes.**

* `~Game`, `~Text`, `~Sprites` and `~Decals` deleted their textures by casting
  the texture *name* to a `const GLuint *`, and ran after SDL had torn down the
  GL context: a crash on exit. The context owns the textures; the destructors
  are now empty.
* The frame timer stored a 32-bit microsecond count that wrapped after ~71
  minutes and, on a 64-bit build, wrapped into a garbage frame time when it
  did. It is now a 64-bit monotonic clock (`SDL_GetPerformanceCounter`).
* `LoadSounds` passed `&(long)` where an `unsigned int *` was expected, so half
  of each buffer length was uninitialised stack.
* `Model::load` stored a file descriptor in a `short` and never checked for
  failure.
* A global named `max` collided with `std::max` (the file does
  `using namespace std`), which modern compilers reject; it is now
  `maxcomponent`.
* Clang rejects things gcc only warned about: `register` on `fast_sqrt`'s
  parameters (removed) and four brace-initialised `GLfloat` arrays that
  narrowed a `double` (the literals are `1.6f`/`.8f`/`.1f` now, so the values
  are unchanged).
* `float totalarea;` in the grenade-fragment impact path was used uninitialised
  (the two other copies of the same loop zero it), so a body hit by shrapnel
  divided by whatever was on the stack. It is zeroed now.
* The frame limiter busy-spun; it now sleeps through the wait.
* `SDL_QUIT` called `exit(0)` mid-frame. Quitting goes through the normal
  shutdown path and saves the high score.
* ~370 lines of dead `NOOGG` WAV-loading code were dropped (the WAVs have not
  shipped with the game since the Ogg conversion).

**What did not change:** the renderer's look, the physics, the timing
constants and the data files. The game logic is the original apart from the
crowd change, the bug fixes and the Black Shades Enhanced changes below, and
the port no longer reads `config.txt` from the game folder.

## Gameplay change: civilians cross the street

In the original, civilians circle their own block's sidewalk forever and only
assassins (and panicking civilians) ever step into the road, so anyone
crossing a street is an assassin. Measured over 2.5 minutes of play: civilians
made 0 crossings in 134 person-minutes; assassins about 1.8 a minute.

Now each civilian crosses to the nearest corner of a neighbouring block every
so often (the new code is the `civiliantype` block in the civilian branch of the
pathfinding in `GameTick.cpp`), checking the straight line against both
buildings first. With the default timing they cross about 1.2-1.5 times a
minute against the assassins' 1.5-1.7, so a crossing now only nudges the odds
that someone is an assassin from 20% to about 23%. Assassins still give
themselves away by closing in on the VIP.

* `civiliancrossdelay` in `Constants.h` is the knob: roughly the game time
  between one civilian's crossings (about 1.7 units per second). Lower means
  more crossing; a huge value (say `1000000`) restores the original behaviour.
* Civilians' distance from the buildings now ranges up to the assassins' fixed
  1.04, so walking on the curb edge is no longer assassin-only either.
* Two bugs in the same code were fixed along the way: the three neighbouring-
  block searches (assassins closing in, civilians fleeing, line of sight)
  computed a z range and then looped over the x range instead, so anyone off
  the city's diagonal searched the wrong rows; and people switching blocks
  never updated the per-block head count the spawner uses.

## Bug fixes in the game itself

A sanitizer playthrough of all 13 missions (AddressSanitizer + UBSan) and a
read through the code turned up these; the same playthrough is now clean.

Things you can notice:

* **Stereo was backwards half the time.** The listener orientation negated
  its x axis but the sound positions did not, so with any spec-following
  OpenAL (the built-in mixer, OpenAL Soft, Apple's) a sound on your right was
  heard on the right facing along one street axis, on the *left* facing along
  the other, and in the middle on the diagonals. It is right at every heading
  now (`GameTick.cpp`, listener orientation).
* **Too many bullet holes crashed the game.** With 120 decals alive at once
  (holes, craters, blood pools) the next one was written past the end of every
  decal array, and the draw that followed crashed. The oldest decal is now
  recycled; sprites (2000) stop taking new ones when full instead of
  overwriting memory.
* **The menu kept its old size after a resize or fullscreen toggle** (a bug in
  this port): it never set its own viewport, so it drew into the old window
  size and its buttons no longer lined up with the mouse.
* **Bullets could pass through the top corners of two building types.** The
  loop that fixes up the buildings' bounding spheres tested the wrong variable
  and stopped after 26 of 56 vertices, leaving types 1 and 3 with spheres
  that were too small (204.7 instead of 234.8, 800.8 instead of 816.2); the
  sphere is the first test for bullet and grenade hits and for culling.
* **The city now uses all four building types equally.** `Random()` is
  signed, so `Random()%4` plus a clamp made 62.5% of the blocks the lowest
  building. For the old skyline, put the line in `GameInitDispose.cpp` back to
  `citytype[i][j]=Random()%4;`.
* **Every mission started on the city's diagonal**, because the VIP's
  starting block used the x coordinate for both axes. It now starts where you
  were.
* **The penalty for failing a mission** (-100 if the VIP's killer is dead,
  -200 otherwise) looked at a `murderer` left over from earlier missions when
  you lost some other way; it is reset every mission now, so losing by dying
  or by shooting a civilian always costs the full 200.
* **Esc** repeated its "back to the menu" every frame it was held, restarting
  the menu music each time (in the menu too); it now acts once per press.
* **The near-miss bullet whoosh** tested a bullet's infinite line instead of
  its path, so a shot fired away from you could whoosh if you stood behind the
  shooter.
* **The debug read-out** (`Show fps and other info`) printed its lines on top
  of each other.

Under the hood:

* `person[-1]`: 22 places read or wrote `person[killtarget]` while
  `killtarget` was -1 ("no target"), e.g. right after an assassin's target
  died or when your melee found nobody in reach. The writes happened to land
  in unused memory; the reads fed junk into the AI and could play a whack
  sound with nobody hit.
* `customlevels.txt` is now checked as it is read: more than six guns, a gun
  or starting gun outside 0-7, "one in 0 civilians is an assassin" and a
  difficulty of 0 all used to overwrite memory or divide by zero (an assassin
  level with no guns in it crashed Intel Macs with a floating-point exception
  and, on Apple Silicon, where dividing by zero does not trap, handed the
  assassin a junk gun number that was then used as an array index).
* A bone standing exactly upright made the joint-angle maths divide 0 by 0,
  and that limb's angle NaN for the frame (a ragdoll landed that way while
  revenge mode was being tested); the angle now keeps its last value.
* Block numbers are now kept inside the 100x100 city. Standing in a block at
  the edge let the spawner pick a neighbouring block that does not exist, and
  a person, ragdoll or grenade that went past the edge looked up a building
  type and rotation outside the city's arrays.
* The model, skeleton and animation loaders checked a failed open with
  `if(files.sFile)`, which is true for -1, and trusted every count and index
  in the file. A missing or damaged data file is now an error message instead
  of a memory overwrite.
* Declarations that disagreed with their definitions (`gunmodels[10]` vs
  `[11]`, `costume[2]` vs `[10]`, `bool slomo` vs `int`), a read of
  `normals[-1]` in the ragdoll ground check, a divide by zero on the very
  first frame, the data path's `:`-to-`/` conversion also rewriting any `:`
  in the install location, `DeleteSprite` leaving the moved sprite with the
  deleted one's velocity and owner, a bullet tracer seen exactly end-on
  dividing by zero in the sprite renderer, and a few harmless oddities
  (`!x==0`, a read of an unset `olddistance`, no-op `==` statements).

Left alone on purpose: the shotgun's spread uses the signed `Random()`
directly. That looks deliberate: the rest of the code uses `abs(Random())`
whenever it wants a non-negative number, and the shotgun's -1° and 2° read as
the same kind of per-weapon aim offset every gun has (-2.5° for the assault
rifle, +4° for the sniper rifle), with the spread centred on them. So the
pattern still sits where it always has.

## Black Shades Enhanced and Revenge mode

[Black Shades Enhanced](https://github.com/Bitl/Black-Shades-Enhanced) (Bitl,
with fixes by Alexander "z33ky" Hirsch) is a Windows version of the game with
a handful of improvements and a second game, the Revenge Mod, built as a
separate program from the same source. Both are in this one app.

**Revenge mode.** The main menu has three buttons: *New Game*, *Revenge Game*
and *Quit*. In revenge mode there is no VIP: every assassin is after you, a
bullet hurts you like anyone else (in the normal game it only knocks the
bodyguard down), and you lose the mission only by dying or by killing an
innocent. It plays the same 13 missions (or your custom levels) and keeps its
own high score in `HighscoreRevenge`; the menu shows both scores, and while a
revenge game is paused its title reads *Revenge*. Three things differ
from the Revenge Mod's own build: it started you in the corner block of the
city every mission (an integer division that always came out 0), where you
now start on the sidewalk of the block you were in, as the VIP does in the
normal game; its spawner still turned the first person it spawned into a VIP,
which it no longer does; and getting shot while scoped used to shift your aim
by the scope's correction every time (see the scope, below).

**Settings.**

* `FOV (Field of View)` in config.txt, default 90, which is what the original
  used; Black Shades Enhanced opened it to 100 and that was the default here
  until now. The sniper scope zooms to a quarter of it - 22.5 degrees at the
  default, where the original zoomed to 10 - as in Black Shades Enhanced.
* `Blur`, default 0. Black Shades Enhanced turned the motion blur on by
  default; it smears the whole screen while you turn, so it is off here and
  `Blur: 1` puts it back.
* `Fps (frames per second) limit`, default 300 (was a fixed 90). The game
  counts frames in its own unit, 0.6 of a real frame, so 300 lets it run at up
  to 500 real frames a second; with `VBL sync` on, the display's refresh rate
  is the limit anyway.
* Depth-of-field blur is on by default in a new config, and never through the
  scope.
* `Blood: 0` now really turns the blood off: no sprays, drips or pools, and a
  bullet hit puffs smoke instead (a headshot, one puff). It used to be read
  and ignored.

**The sniper scope** (z33ky's fixes). Zooming in turns the view 14 degrees
down and 9 left, so the scope looks where the rifle shoots (the original
turned it 6 down, which missed), zooming out turns it back, and the view snaps
straight back when the scope comes down, say to reload. The scope does not
work in soul mode (the original meant it not to, but a later line turned it
straight back on). Where Enhanced applied the correction on every frame the
scope was cancelled - so standing in soul mode with the scope up spun the
view 14 degrees a frame, and in revenge mode every hit while scoped moved your
aim - here it is applied only when the scope actually goes up or down.

**Smaller fixes from Enhanced.**

* The shotgun's laser sight used the sniper rifle's angle; it now points at
  the middle of the shotgun's spread. (Enhanced also combined the shotgun's
  two aiming turns into one call, which turns them in the other order and
  tilts the spread sideways when you face along one street axis; that part
  was left out.)
* No laser sight in soul mode.
* Pressing one Ctrl key while holding the other, then letting go, no longer
  drops your crouch (and the scope): the modifier keys are read from SDL's
  modifier state, as Enhanced does for Ctrl (its issue #7).
* The skeleton's "knocked out of its animation" flag was called `offset`,
  hiding the `float offset` its ragdoll code writes `.2` into; renamed to
  `offsetted` as in Enhanced, a ragdoll that hits a wall is now put back 0.2
  from it as the code says, not a whole unit.

**Already done here, or not needed on a Mac.** SDL2, windowed mode and the
icon, mouse capture, the fix for Esc re-opening the menu every frame, the
compiler-warning cleanup, and the music-off hang (which this port's mixer
never had). Two Enhanced changes were deliberately not carried over: its
default window of 1920x1080 (1280x720 here; the window resizes and goes
fullscreen), and its near-miss "whoosh" test, which it changed to one that
can never be true (the bug fix above makes it test the bullet's path). The
soundtrack Enhanced ships as separate WAV files is the game's own music,
which is in `Data/Sounds`.

## Known limitations

* Fixed-function OpenGL is deprecated on macOS. It works today, on both Apple
  Silicon and Intel, but a future macOS could remove it; that would mean a
  renderer rewrite, not a port fix.
* The .app is signed ad-hoc (`codesign -s -`), which is fine for running it
  yourself. Sharing it with someone else means either notarising it or having
  them right-click → Open the first time.
* The `Rifleaim(old)` animation, `TGALoader.cpp` and `Files.cpp` (the Mac OS 9
  file dialogs) are still in the tree, unused.
* The known pathfinding quirk from the original `TODO` — people re-deciding
  where they are walking — is untouched.

## Other platforms

A Nintendo Switch homebrew target (devkitPro/libnx) lives in
`Makefile.switch`, built with `make switch`, with everything it needs behind
`#ifdef __SWITCH__`. It has never run on a console; see `README-switch.md`.

A WebAssembly target (Emscripten) lives in `Makefile.wasm`, built with
`make wasm`, with its differences behind `#ifdef __EMSCRIPTEN__`. That one has
been built and played — in a browser the game runs on WebGL through
Emscripten's fixed-function emulation, and saves to IndexedDB; see
`README-wasm.md` for the two or three places the web needed something
different.

A Wii homebrew target (devkitPPC/libogc, with SDL2 and opengx) lives in
`Makefile.wii`, built with `make wii`, behind `#ifdef __wii__`. It plays as a
Wii shooter does: the Wiimote points the gun at the screen and the nunchuk
drives. That aiming is a runtime option rather than a Wii-only code path -
`Pointer aiming` in config.txt turns it on with a mouse anywhere, and it
defaults off everywhere but the Wii - so it can be played and tuned on a
desktop. It has never run on a console; see `README-wii.md`.

All three keep their own object directory and their own flags, so building any
of them leaves the desktop build exactly where it was.

## Repository notes

This is an SVN checkout, so `svn diff` shows the whole port and `svn revert`
undoes any part of it. The original Linux `Makefile` was replaced; the old one
is in SVN history (`svn cat Makefile`), and `Makefile.zakk` is untouched.

Worth adding to `svn:ignore`: `build`, `build-arm64`, `build-x86_64`,
`blackshades`, `BlackShades.app`, `macos/Frameworks`.
