# Black Shades on the Wii (homebrew)

A devkitPPC/libogc build of the SDL2 port, played the way a Wii shooter is
played: the Wiimote points the gun at the screen, the nunchuk drives.

**This runs on a console.** It has been played on real hardware through the
Homebrew Channel — missions completed, the crowd and the assassins behaving,
the pointer aiming the gun. One fault remains unsolved and is written up in
full under *What will probably need fixing first*; read item 3 before you
change anything in `GameTick.cpp`, because at the moment some of what is in
there is load-bearing for reasons nobody has got to the bottom of.

It was written without a Wii and without devkitPro's toolchain (the machine
it was written on cannot reach their servers), so every source file was
type-checked with `__wii__` defined against the real libogc, libfat and SDL2
headers, the aiming was built and played on a desktop with a mouse — the same
absolute pointer a Wiimote is — and the console builds and the play-testing
were done by hand on the other side.

## Building

```sh
sudo dkp-pacman -S wii-dev wii-sdl2 wii-opengx gamecube-tools
export DEVKITPRO=/opt/devkitpro
make wii            # or: make -f Makefile.wii
make -f Makefile.wii sdcard
```

`gamecube-tools` is the one that provides `elf2dol`, which turns the linked
`.elf` into the `.dol` the console loads; it installs into
`$DEVKITPRO/tools/bin` rather than devkitPPC's own bin, and the makefile looks
in both that and `PATH`.

`make wii` leaves `boot.dol` in the source folder. `make -f Makefile.wii
sdcard` then assembles `sdcard/apps/blackshades/` with the .dol, `meta.xml`,
`icon.png` and the game's `Data` folder — copy that `apps` folder to the root
of the SD card and the game appears in the Homebrew Channel.

`make wii` only runs `Makefile.wii`, which has its own object directory
(`build-wii`), its own flags and its own source list. The desktop Makefile is
untouched apart from the one delegating target.

Two things in that makefile are worth knowing. It is the only build that does
**not** set `SDL_MAIN_HANDLED`, because SDL2 supplies `main()` on a Wii and
that function does setup nothing else will — the IOS reload, the power and
reset callbacks, and `WPAD_SetDataFormat(..., WPAD_FMT_BTNS_ACC_IR)` with
`WPAD_SetVRes(..., 640, 480)`, without which the pointer reports nothing.
`main()` in `Main.cpp` becomes `SDL_main()` and SDL calls it. And the port's
Wii code is behind `__wii__`, which devkitPPC's gcc defines for `-mrvl`; the
makefile asks the compiler whether it does and adds it if not.

## Where things live

| | |
| --- | --- |
| `boot.dol`, `meta.xml`, `icon.png`, `Data` | `sd:/apps/blackshades/` |
| `config.txt`, `Highscore`, `HighscoreRevenge` | the same folder |

Saves are flushed to the card after every write (libfat caches otherwise), so
switching the console off at the wall after a mission does not lose the score.

## Controls

The Wiimote points, the nunchuk drives. This is the "Pointer aiming" mode
described below, which is on by default here and off everywhere else.

| Control | Action |
| --- | --- |
| Point the Wiimote | move the crosshair; the gun follows it, and the view turns once the crosshair nears the edge of the screen |
| **B** (trigger) | fire while aiming, smash while running, pick a gun up while crouched over a body, otherwise disarm |
| **Z** (nunchuk trigger) | crouch, which is also the sniper scope |
| **C** (nunchuk thumb) | gun up, gun down - a press each way |
| **A** | reload |
| Nunchuk stick, up/down | walk forward and back; all the way, run |
| Nunchuk stick, left/right | turn |
| D-pad up | psychic aim - hold it for the slow motion |
| D-pad down | dive |
| D-pad left | laser sight on and off |
| D-pad right | soul release on and off |
| **+** | the menu, as Esc |
| **HOME** | save and quit to the Homebrew Channel |
| Power / Reset | quits the same way, saving first |

Trigger fires, nunchuk trigger crouches, nunchuk thumb raises and lowers the
gun, Wiimote thumb reloads. The gun is a toggle, a press each way, not a
hold.

The one place this departs from Call of Duty is the stick: here left and
right *turn* rather than strafe, which is what you asked for. If you would
rather have the CoD arrangement - stick strafes, the pointer does all the
turning at the screen edge - it is the one `PlatformPadMouse` line in the
Wii block of `GameLoop.cpp`, swapped for `leftkey`/`rightkey`.

In the menu the pointer *is* the cursor and **B** clicks, so the menu works
without touching the nunchuk.

## How the aiming works

The original game aims with the mouse: the view turns and the gun points down
the middle of it. A light gun is the other way round — the gun moves inside a
view that mostly stays put — so the port grew a second aiming mode.

* The pointer's position on screen becomes an angle, exactly: a perspective
  projection puts the point *x* across the viewport at `atan(x·tan(fov/2))`
  from the middle, so the gun ends up under the crosshair rather than near it.
* Inside a dead zone — the middle 55% of the screen — the view does not move
  at all. That is what makes it feel like a light gun rather than a mouse.
* Past the dead zone the view is pushed round, faster the further out the
  pointer is (squared, so it drifts near the edge of the zone and swings at
  the very edge of the screen), on top of whatever the stick is doing.
* A Wiimote stops reporting as soon as it nears the edge of what the sensor
  bar can see - which is exactly when you are pushing the view round - so the
  crosshair is held where it last was rather than thrown back to the middle of
  the screen, and a nudge past the edge is clamped to the edge rather than
  discarded. The view carries on turning through a blink like that, and stops
  after a second of it (`pointer_grace` in `GameTick.cpp`) so that a Wiimote
  put down on the sofa does not leave it spinning.
* The view stops pitching `fov/2` short of straight up and straight down,
  because that is what the crosshair at the top or bottom of the screen is
  worth on top of it and the body will not bend past 89°. You can still aim
  at anything you could before — the reach is just split between the view and
  the pointer now — but the gun can always reach the crosshair, which is the
  whole point of drawing one.
* A shot's kick is **borrowed rather than kept**. The original pushed the view
  up a few degrees a shot and left it there, because you pulled the mouse back
  down; a pointer has no such gesture, so the same kick used to ratchet. With
  the pointer sitting still in the middle of the screen, two bursts of an
  assault rifle walked the view from level to 59° up and left it there, and
  from there the crosshair ran 45° off the gun and the body bent to vertical
  with the gun out of frame. Now the view jolts by exactly what the shot was
  worth and slides back over the next half-second (`recoilreturn` in
  `GameTick.cpp`), so sustained fire climbs a few degrees and settles instead
  of walking away. The kick goes to the shown view as well as the camera's own
  angle: the gun is aimed off the shown one, so a kick to the camera alone did
  nothing at all until it had eaten the 15° of slack between them and then
  dragged everything at once.

Most of what happens downstream follows for free, because of how the original
game is built: the shot direction is taken from the player's *posed skeleton*
(the vector between two hand joints), and the skeleton is posed from the body
angles, which is what the pointer now drives. So the bullet, the gun model and
the laser sight all move together without any of them being special-cased.

Two things did not follow for free, and both had to be measured and taken out,
because a crosshair that lands somewhere other than the bullet is worse than
no crosshair at all.

* **The gun is not bolted to the chest.** That skeleton vector sits several
  degrees off whichever way the body is pointed, by a different amount for
  every gun — 6.2°–6.7° for the assault rifle — so aiming the *body* at the
  crosshair left the *barrel* beside it. Now the offset is read back out of
  the pose every frame, using the same expressions the firing code uses, and
  the body is given that much more angle. The gun model, which is held by the
  same hands, swings round with it, so you can see it happen.
* **The eye and the muzzle are not in the same place.** The crosshair is drawn
  on a line out of the camera and the bullet leaves the hand, about half a
  unit down and to the left, and parallel lines never meet: that was half a
  unit of miss at every range, a head's width. The shot's starting point is
  now slid sideways onto the camera's line, keeping the distance down it that
  it already had, so the shot runs along the crosshair's own line. The muzzle
  it appears to leave moves by less than the length of the gun. The direction
  is untouched, so a shotgun still throws its pellets apart — and for a
  shotgun the crosshair marks the middle of the spread.

Measured on the desktop build, sweeping the pointer around the screen and
across two guns: the fired direction now stays within 0.1° of the crosshair,
and the shot passes within 0.01–0.05 units of the point under it at 10, 30 and
100 units of range — against 6.4° and 3.2 units at 30 before. Its angle from
the *camera*, meanwhile, still grows from nothing in the middle to 45° at the
edge. The gun tracks the crosshair, not the view.

Neither correction exists unless `Pointer aiming` is on, and neither touches
anybody but you: everyone else shoots exactly as the original had them shoot.

### Trying it without a Wii

`Pointer aiming` in `config.txt` turns the same mode on anywhere — a mouse is
an absolute pointer too. It defaults to 0 on every other platform, so nothing
changes unless you ask for it, and to 1 on the Wii. With it on, the mouse is
not captured: the pointer moves freely and the crosshair goes where it is.

## What will probably need fixing first

1. ~~**Whether opengx covers this renderer.**~~ Settled: it compiles, links
   and runs. Every `gl*` call resolved except `glListBase`, which opengx does
   not have because it keeps no list-base state at all.

   Display lists turned out to be unusable on the Wii for a second and worse
   reason, found by following a hang to `GX_GetDrawSync`: opengx resets its
   draw-sync token to zero on every buffer swap (`ogx_prepare_swap_buffers`)
   while `call_lists.c` keeps a separate `s_last_draw_sync_token` that the
   swap does not reset, so `while (GX_GetDrawSync() < s_last_draw_sync_token);`
   in `setup_draw_geometry` waits for a token the hardware has already counted
   back past. The loading screen prints text every frame, so the game wedged
   on the second one. The font is the only thing here that used lists, and it
   now takes the same immediate-mode path as the web build
   (`BS_NO_DISPLAY_LISTS` in `GLHeaders.h`). Worth reporting upstream.
2. **The matrix stacks are shallow.** opengx gives the projection stack four
   slots against a desktop driver's thirty-two, and it is strict: a push past
   the end sets `GL_STACK_OVERFLOW` and does nothing, while the matching pop
   still hands back whatever is on top. The menu's cursor used to push the
   projection matrix and then pop the *modelview* one - a pair split across a
   `glMatrixMode` - so it leaked a projection level every frame it drew. On a
   desktop that is invisible: thirty-two slots all holding much the same
   perspective matrix, and nothing looks wrong. On a Wii the stack was full
   within four frames of the menu, and from then on every HUD or sprite pass
   that pushed, drew and popped handed the *world* back a leftover
   orthographic matrix. The result was grey geometry exploding across the
   whole view, following the camera, surviving a new mission and the menu,
   and only clearing on a reboot. Fixed in `GameDraw.cpp`; worth knowing
   about if you add another overlay, because the desktop will not tell you.
3. **People exploding into triangles — unsolved, and currently suppressed by
   accident.** Shooting someone can spray that person's geometry across the
   whole view in their own colours. It survives a new mission and the main
   menu and clears only on a reboot. Part of the original fault was the
   matrix stack in item 2, which is properly fixed. The rest is not.

   What has been *eliminated*, each with a check calibrated to be worth
   something rather than guessed at:

   * Joint positions and velocities. Nothing ever sat more than forty units
     from the middle of its own body, against a measured worst case of 5.95.
   * The ten body models and the city's, hashed against their state at load
     and never seen to change.
   * The matrix stacks — peak depth three of sixteen, one of four, no drift.
   * The weighted impulse that pushes a body around, now floored
     (`HitWeight` in `GameTick.cpp`) so a joint landing exactly on a
     bullet's hit point cannot weigh infinity and turn every joint's share
     into a NaN. Measured never to bind in play.

   **The awkward part.** Those checks — `CheckSkeletons()` and
   `CheckModels()` — never once logged a line. They only read. And yet the
   explosion stops when they are compiled in and comes straight back when
   they are taken out; that was tested deliberately, in both directions, on
   hardware. Two functions that only read memory cannot fix anything, so
   what they are doing is moving the binary's layout and timing, and the
   real fault is a stray write whose landing spot moves with them. They are
   still in the tree because taking them out breaks the game, which is a bad
   reason to keep code and an honest one.

   So: **do not remove them without testing on hardware**, and expect
   anything else you add or remove in `GameTick.cpp` to be able to flip it
   either way.

   If someone picks this up, the ground that was never covered is what the
   draw actually transforms by — `rotate1`, `rotate2`, `rotate3` and
   `length` on each joint, handed straight to `glRotatef` and `glScalef`. A
   wrong angle or scale there stretches a body part across the world while
   every position stays sane and every model stays intact, which is exactly
   the state that was observed. `Normalise` and `BoneRatio` are both guarded,
   so it is not a naive divide by zero. After that, the tool to reach for is
   guard words on the end of `Model` and around the crowd allocation, since
   what is wanted is the identity of a stray write, not another observer.

   `Model::DrawableNow()` is the one thing here that earns its place on
   merit: every draw does `glDrawArrays(GL_TRIANGLES, 0, TriangleNum*3)`,
   and `TriangleNum` is a `short` sitting in front of the array it indexes,
   which holds four hundred triangles and no more. Two comparisons, one line
   in the log, and nothing drawn rather than garbage.

4. **Mipmaps.** opengx has no `GL_GENERATE_MIPMAP`, so textures will be
   unmipmapped and will shimmer at distance. `Textures.cpp` is where to build
   the levels by hand if it bothers you.
5. **Memory.** 24 MB of MEM1 and 64 MB of MEM2, and a person is about 125 KB.
   The crowd is taken out of MEM2 (`PlatformBigAlloc` in `Support.cpp`, a bump
   off the arena) and capped at 24 MB, about 190 people, whatever `View
   distance` and `Population density` say. If it still runs out, the crowd
   halves its density until it fits, all the way down to two people — you and
   the VIP — and only then gives up, with a line in the log saying so rather
   than a crash. (It used to stop thinning at ninety, and because the same
   loop did the allocating, any setting that asked for ninety or fewer never
   allocated at all: `View distance 0.5` with `Population density 0.1` was
   enough to start the game on a null crowd.)
6. **Frame rate.** A 729 MHz Broadway is a fraction of the Mac this was
   written for. Expect to want `View distance` at 0.5 and `Population
   density` below 1.
7. **The dead zone and the pan speed** (`pointer_deadzone` and `pointer_pan`
   at the top of the pointer-aiming block in `GameTick.cpp`, 0.55 and 150
   degrees a second) were tuned with a mouse on a monitor, not a Wiimote
   across a living room. They are two constants.
8. **Audio.** SDL2's ogc backend drives it. `Sound: 0` in config.txt opens no
   audio device at all, which is worth knowing as a way of taking the DSP out
   of the picture when something else misbehaves.

Not attempted: 480p/widescreen handling beyond whatever SDL reports, rumble,
the Classic Controller (SDL exposes it, but the buttons are not mapped),
GameCube controllers, or anything to do with distributing the result — the
source is under the uDevGame Limited Use License and the music is separately
licensed.
