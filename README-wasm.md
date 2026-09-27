# Black Shades in a browser (WebAssembly)

An Emscripten build of the SDL2 port. It compiles the same sources everything
else does, with the browser's differences behind `__EMSCRIPTEN__`, and drops
`build-wasm/` on you: `index.html` and `blackshades.{js,wasm,data}`. Put that
folder on any web server and the game runs.

Unlike the Switch port, **this one has been built and played**: Emscripten
3.1.6, run in headless Chromium on a software renderer. The menu, a mission,
the HUD, the crowd, fog, the mouse and keyboard, and quitting (which saves)
all work. What has not been tried is a real GPU, a gamepad in a browser, and
sound coming out of actual speakers.

## Building

```sh
# either the SDK
git clone https://github.com/emscripten-core/emsdk && emsdk/emsdk install latest && emsdk/emsdk activate latest
source emsdk/emsdk_env.sh
# or the distribution's package
sudo apt install emscripten

make wasm                 # or: make -f Makefile.wasm
make -f Makefile.wasm serve     # http://localhost:8000/
```

The first build fetches and compiles Emscripten's SDL2 port, which takes a
couple of minutes; after that it is cached.

`make wasm` only runs `Makefile.wasm`, which has its own object directory
(`build-wasm`), its own flags and its own source list. The desktop Makefile is
untouched apart from the delegating target, so the builds cannot disturb each
other.

A page loaded from `file://` cannot fetch the `.wasm` or the `.data`, so it
has to be served over http — that is all `make -f Makefile.wasm serve` does.

**Debian and Ubuntu package Emscripten with a read-only cache**, and the build
stops with `Attempt to lock the cache but FROZEN_CACHE is set`. Point it at a
writable copy:

```sh
cp -a /usr/share/emscripten/cache /tmp/emcache
EM_FROZEN_CACHE= EM_CACHE=/tmp/emcache make wasm
```

## Playing it

Click the canvas and the game takes the mouse (pointer lock); Esc gives it
back, as it does in the menu. Keys are the ones in README-macOS.md. The page
has a Fullscreen button because a browser only goes fullscreen from a click,
so `Start fullscreen` in config.txt is ignored on the web.

Sound starts at the first click anywhere on the page — browsers refuse to make
noise before that, and the menu wants a click anyway.

`config.txt`, `Highscore` and `HighscoreRevenge` live in an IndexedDB mount
(`/blackshades`) and are written when you quit through the menu, so they
survive a reload but belong to that one browser on that one machine. Closing
the tab mid-mission loses the score, as pulling the plug would.

To change a setting, quit once so `config.txt` exists, then edit it from the
browser's console:

```js
FS.writeFile('/blackshades/config.txt',
  new TextDecoder().decode(FS.readFile('/blackshades/config.txt'))
     .replace(/View distance: .*/, 'View distance: 0.6'));
FS.syncfs(false, e => location.reload());
```

Frame rate is the thing to watch: `View distance` and `Population density` are
what to turn down, in that order. The default crowd is 810 people, and a
browser on a laptop will not enjoy that as much as a desktop does.

## What the port does

* **The frame loop.** A browser will not let a program keep the thread, so
  `Game::EventLoop`'s body became `Game::Frame()` and the browser calls that
  through `emscripten_set_main_loop_arg`. Every other platform still runs the
  same `while (!gQuit) Frame();` it always did. Quitting is handled in the
  frame callback, because the main loop never returns to `main()`, and that is
  what writes the high score.
* **OpenGL.** `-sLEGACY_GL_EMULATION` puts a fixed-function layer over WebGL 1,
  which covers glBegin/glEnd, the matrix stack, lighting, fog and clip planes.
  The three things it does not cover are patched around:
  * **Display lists** (the font's 256 quads). `Text.cpp` draws the quad where
    it would have called the list. Same geometry, no recording.
  * **`GL_COLOR_MATERIAL`.** With lighting on, the emulation works the vertex
    colour out from the material alone and throws `glColor` away - the whole
    world comes out grey. `glColor`/`glEnable`/`glDisable` go through
    stand-ins (`GLHeaders.h`, implemented in `Support.cpp`) that keep the
    material in step with the colour.
  * **`GL_GENERATE_MIPMAP` and `GL_CLAMP`,** neither of which WebGL has:
    mipmaps are built with `glGenerateMipmap` after the upload, and `GL_CLAMP`
    becomes `GL_CLAMP_TO_EDGE`, which is what drivers do with it anyway.
* **Memory.** The heap starts at 128 MB and grows to 2 GB. A person is about
  125 KB, so the default crowd is around 100 MB; wasm32 cannot go past 4 GB
  whatever the settings say.
* **Audio.** Unchanged: the built-in mixer runs on SDL's audio device, which
  in a browser is a Web Audio node.

## What is different on the web

1. **Models are unlit.** People, guns and cars carry their colours in a vertex
   array, and the emulation ignores a colour array as soon as lighting is on,
   which turned everyone grey. They are drawn unlit instead, so they keep
   their colours and lose the sun on their faces. Buildings, ground, sky and
   effects are lit as usual. This is the one visible difference from the
   desktop build, and the fix for it is the same shader-based renderer the
   Switch port wants.
2. **No fullscreen at startup**, and no way to ask for one - the page's button
   is the only route, because that is the browser's rule.
3. **The window is the canvas.** `Screenwidth`/`Screenheight` still choose the
   size, clamped to the screen; resizing works as it does elsewhere.
4. **Saving is per-browser** and happens on quit, as above.
5. **Music and sound effects** need that first click, and Emscripten's SDL2
   mixes through a `ScriptProcessorNode`, which is deprecated but works
   everywhere today.

## What has not been tried

Gamepads (the controller code is untouched and the browser exposes the
Gamepad API to SDL, so it ought to work), sound on real speakers, touch
devices, and any GPU faster than a software rasteriser. Performance numbers
from this container are meaningless - it drew about one frame a second on
SwiftShader, which says nothing about a real machine.

Not attempted either: shipping it anywhere. The source is under the uDevGame
Limited Use License and the music is separately licensed, so where the folder
goes is your call, not mine.
