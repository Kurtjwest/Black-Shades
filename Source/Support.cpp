/*
 * Support.cpp - platform support for the SDL2 port.
 *
 * Mouse/keyboard/timing helpers that stand in for the original Mac Toolbox
 * calls, path handling (the game asks for files with Mac OS 9 style
 * ":Data:Models:Head.solid" names), and Ogg Vorbis decoding.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <strings.h>

#include <SDL.h>

#include "Support.h"
#include "Files.h"
#include "GLHeaders.h"

#ifdef __SWITCH__
#include <switch.h>
#endif

SDL_Window   *g_window    = NULL;
SDL_GLContext g_glcontext = NULL;

int Random()
{
#if RAND_MAX >= 65535
	return (rand() % 65535) - 32767;
#else
#error please fix this for your platform
#endif
}

/* ------------------------------------------------------------------ */
/* Time                                                                */
/* ------------------------------------------------------------------ */

/*
 * The original game used Microseconds(), a 32-bit microsecond counter, which
 * wraps every 71 minutes - and wraps *wrongly* once "unsigned long" is 64 bits.
 * Everything now goes through this monotonic, 64-bit clock instead.
 */
double PlatformTimeNanos(void)
{
	static Uint64 freq  = 0;
	static Uint64 start = 0;
	if (freq == 0) {
		freq  = SDL_GetPerformanceFrequency();
		start = SDL_GetPerformanceCounter();
	}
	const Uint64 now = SDL_GetPerformanceCounter();
	return (double)(now - start) * (1000000000.0 / (double)freq);
}

void Microseconds(UnsignedWide *microTickCount)
{
	const double us = PlatformTimeNanos() / 1000.0;
	const unsigned long long v = (unsigned long long)us;
	microTickCount->lo = (unsigned int)(v & 0xFFFFFFFFull);
	microTickCount->hi = (unsigned int)(v >> 32);
}

/* ------------------------------------------------------------------ */
/* Mouse                                                               */
/* ------------------------------------------------------------------ */

static bool g_relativeMouse = false;

/* A controller can push the mouse around on any platform: look motion adds
   to what the mouse reports, the menu pointer can be nudged or put somewhere,
   and a trigger is the button.  On a Switch there is no real mouse behind any
   of it, so the pointer is a made-up one kept here. */
static int g_padDX = 0, g_padDY = 0;
static bool g_padButton = false;
#ifdef __SWITCH__
static int g_cursorX = 640, g_cursorY = 360;
#endif

void PlatformPadMouse(int dx, int dy)
{
	g_padDX += dx;
	g_padDY += dy;
}

void PlatformPadButton(bool down)
{
	g_padButton = down;
}

void PlatformPadPointerTo(int x, int y)
{
#ifdef __SWITCH__
	g_cursorX = x;
	g_cursorY = y;
#else
	if (g_window && !g_relativeMouse) {
		SDL_WarpMouseInWindow(g_window, x, y);
		SDL_PumpEvents();
	}
#endif
}

void PlatformPadPointerMove(int dx, int dy)
{
	if (!dx && !dy) return;
	int w = 1280, h = 720;
	if (g_window) SDL_GetWindowSize(g_window, &w, &h);
	int x = 0, y = 0;
#ifdef __SWITCH__
	x = g_cursorX;
	y = g_cursorY;
#else
	SDL_GetMouseState(&x, &y);
#endif
	x += dx;
	y += dy;
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x > w - 1) x = w - 1;
	if (y > h - 1) y = h - 1;
	PlatformPadPointerTo(x, y);
}

#ifdef __SWITCH__
/* Mesa on the Switch is reached through EGL, with no libGL to link against:
   glad turns every gl* call in the game into a call through a pointer, and
   these are the pointers.  A driver without the fixed-function pipeline
   leaves the ones the renderer lives on null, which is worth saying out
   loud rather than crashing on the first frame. */
bool PlatformLoadGL(void)
{
	if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
		fprintf(stderr, "Could not load OpenGL through SDL\n");
		return false;
	}
	const struct { const char *name; const void *fn; } needed[] = {
		{ "glBegin",         (const void *)glad_glBegin },
		{ "glEnd",           (const void *)glad_glEnd },
		{ "glVertex3f",      (const void *)glad_glVertex3f },
		{ "glMatrixMode",    (const void *)glad_glMatrixMode },
		{ "glLightfv",       (const void *)glad_glLightfv },
		{ "glFogi",          (const void *)glad_glFogi },
		{ "glAlphaFunc",     (const void *)glad_glAlphaFunc },
		{ "glVertexPointer", (const void *)glad_glVertexPointer },
	};
	int missing = 0;
	for (unsigned i = 0; i < sizeof(needed) / sizeof(needed[0]); i++) {
		if (!needed[i].fn) {
			fprintf(stderr, "OpenGL is missing %s\n", needed[i].name);
			missing++;
		}
	}
	const GLubyte *version = glGetString ? glGetString(GL_VERSION) : NULL;
	if (missing) {
		fprintf(stderr,
		        "This driver has no fixed-function pipeline, which is what the\n"
		        "renderer is written against (GL_VERSION: %s).\n",
		        version ? (const char *)version : "?");
		return false;
	}
	fprintf(stderr, "OpenGL %s\n", version ? (const char *)version : "?");
	return true;
}

/* Writes to the SD card sit in a cache until the filesystem is committed. */
void PlatformCommitSave(void)
{
	fsdevCommitDevice("sdmc");
}

void GetMouse(Point *p)
{
	p->h = g_cursorX;
	p->v = g_cursorY;
}
#else
void GetMouse(Point *p)
{
	int x = 0, y = 0;
	SDL_GetMouseState(&x, &y);
	p->h = x;
	p->v = y;
}
#endif

void GetMouseRel(Point *p)
{
	int x = 0, y = 0;
#ifndef __SWITCH__
	SDL_GetRelativeMouseState(&x, &y);
#endif
	p->h = x + g_padDX;   //the mouse and the controller both turn you
	p->v = y + g_padDY;
	g_padDX = 0;
	g_padDY = 0;
}

int Button(void)
{
	if (g_padButton) return 1;
#ifndef __SWITCH__
	if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) return 1;
#endif
	return 0;
}

void InitMouse()
{
}

void MoveMouse(int xcoord, int ycoord, Point *mouseloc)
{
#ifdef __SWITCH__
	PlatformPadPointerTo(xcoord, ycoord);
#endif
	/* With relative mouse mode the pointer is already locked, so warping it
	   would only fight with the driver; it is only useful for the menu. */
	if (!g_relativeMouse && g_window) {
		SDL_WarpMouseInWindow(g_window, xcoord, ycoord);
		SDL_PumpEvents();
	}
	GetMouse(mouseloc);
}

void DisposeMouse()
{
}

void PlatformSetRelativeMouse(bool enable)
{
	if (enable == g_relativeMouse) return;
#ifdef __SWITCH__
	/* nothing to capture: the pad is the pointer, and it is always ours */
	g_relativeMouse = enable;
	return;
#endif
	if (SDL_SetRelativeMouseMode(enable ? SDL_TRUE : SDL_FALSE) == 0) {
		g_relativeMouse = enable;
		if (enable) {
			int dx, dy;
			SDL_GetRelativeMouseState(&dx, &dy);   /* drop accumulated motion */
		}
	}
}

bool PlatformRelativeMouse(void)
{
	return g_relativeMouse;
}

bool PlatformIsFullscreen(void)
{
	if (!g_window) return false;
	return (SDL_GetWindowFlags(g_window) & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;
}

void PlatformToggleFullscreen(void)
{
	if (!g_window) return;
#ifdef __SWITCH__
	return;   //the window is the screen, docked or handheld
#endif
	const bool full = PlatformIsFullscreen();
	SDL_SetWindowFullscreen(g_window, full ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void PlatformUpdateViewportSize(int *windowW, int *windowH)
{
	if (!g_window) return;
	int w = 0, h = 0;
	SDL_GetWindowSize(g_window, &w, &h);
	if (w > 0 && h > 0) {
		if (windowW) *windowW = w;
		if (windowH) *windowH = h;
	}
}

void PlatformSetViewport(void)
{
	int w = 0, h = 0;
	if (g_window) SDL_GL_GetDrawableSize(g_window, &w, &h);
	if (w > 0 && h > 0) glViewport(0, 0, w, h);
}

void PlatformSwapBuffers(void)
{
	if (g_window) SDL_GL_SwapWindow(g_window);
}

/* ------------------------------------------------------------------ */
/* Paths                                                               */
/* ------------------------------------------------------------------ */

#ifndef O_BINARY
#define O_BINARY 0
#endif

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

static char g_dataRoot[MAX_PATH] = "";   /* directory containing "Data/" */
static char g_prefRoot[MAX_PATH] = "";   /* per-user writable directory   */

bool PlatformFileExists(const char *path)
{
	struct stat st;
	return path && *path && stat(path, &st) == 0;
}

static bool DirHasData(const char *dir)
{
	char probe[MAX_PATH];
	snprintf(probe, sizeof(probe), "%sData", dir);
	struct stat st;
	return stat(probe, &st) == 0 && S_ISDIR(st.st_mode);
}

void PlatformInitPaths(void)
{
	if (g_dataRoot[0]) return;
#ifdef __SWITCH__
	/* Data rides along inside the .nro as romfs; saves go next to the other
	   homebrew on the SD card, where you can copy them off. */
	snprintf(g_dataRoot, sizeof(g_dataRoot), "romfs:/");
	snprintf(g_prefRoot, sizeof(g_prefRoot), "sdmc:/switch/blackshades/");
	mkdir("sdmc:/switch", 0777);
	mkdir("sdmc:/switch/blackshades", 0777);
	return;
#endif

	/* 1. explicit override, handy for testing */
	const char *env = getenv("BLACKSHADES_DATA");
	if (env && *env) {
		snprintf(g_dataRoot, sizeof(g_dataRoot), "%s%s", env,
		         env[strlen(env) - 1] == '/' ? "" : "/");
	}

	/* 2. next to the executable - inside an .app bundle that is
	      BlackShades.app/Contents/Resources/, where Data/ is installed */
	if (!g_dataRoot[0]) {
		char *base = SDL_GetBasePath();
		if (base) {
			if (DirHasData(base))
				snprintf(g_dataRoot, sizeof(g_dataRoot), "%s", base);
			SDL_free(base);
		}
	}

	/* 3. the current directory, for running straight out of the source tree */
	if (!g_dataRoot[0] && DirHasData("./"))
		snprintf(g_dataRoot, sizeof(g_dataRoot), "./");

	/* 4. one level up, for ./objs/blackshades style builds */
	if (!g_dataRoot[0] && DirHasData("../"))
		snprintf(g_dataRoot, sizeof(g_dataRoot), "../");

	if (!g_dataRoot[0])
		snprintf(g_dataRoot, sizeof(g_dataRoot), "./");

	char *pref = SDL_GetPrefPath("", "Black Shades");
	if (pref) {
		snprintf(g_prefRoot, sizeof(g_prefRoot), "%s", pref);
		SDL_free(pref);
	} else {
		snprintf(g_prefRoot, sizeof(g_prefRoot), "%s", g_dataRoot);
	}
}

const char *PlatformDataRoot(void) { PlatformInitPaths(); return g_dataRoot; }
const char *PlatformPrefRoot(void) { PlatformInitPaths(); return g_prefRoot; }

/*
 * Walk a path component by component, repairing the letter case of each one
 * from the directory listing.  The game's own filenames disagree with the data
 * files in a dozen places ("Disguisekill.ogg" vs "DisguiseKill.ogg"), which
 * nobody noticed on a case-insensitive Mac volume - this keeps it working on
 * case-sensitive ones too.
 */
static int find_filename(char *filename)
{
	char *ptr;
	char *cur;
	char *next;
	DIR *dir;
	struct dirent *dirent;

	if (access(filename, R_OK) == 0) {
		return 1;
	}

	/* "romfs:/Data/..." on the Switch (and "sdmc:/..."): the device prefix is
	   not a directory anyone can look inside, so the walk starts after it and
	   the first component is looked up in the device's root rather than in the
	   current directory. */
	char *base = filename;
	char rootdir[64] = ".";
	char *colon = strchr(filename, ':');
	if (colon && colon[1] == '/' && memchr(filename, '/', (size_t)(colon - filename)) == NULL) {
		const size_t rootlen = (size_t)(colon + 2 - filename);
		if (rootlen < sizeof(rootdir)) {
			memcpy(rootdir, filename, rootlen);
			rootdir[rootlen] = 0;
			base = colon + 2;
		}
	}

	ptr = base;

	while (*ptr) {
		if (ptr == base || *ptr == '/') {
			if (*ptr == '/') {
				cur = ptr+1;
			} else {
				cur = ptr;
			}

			if (*cur == 0) {
				/* hit the end */
				break;
			}

			next = strchr(cur, '/');

			if (ptr != base) {
				*ptr = 0;
			}

			if (next) {
				*next = 0;
			}

			if (ptr == base && *ptr == '/') {
				dir = opendir("/");
			} else if (ptr == base) {
				dir = opendir(rootdir);
			} else {
				dir = opendir(filename);
			}

			if (dir == NULL) {
				if (ptr != base) {
					*ptr = '/';
				}

				if (next) {
					*next = '/';
				}

				return 0;
			}

			while ((dirent = readdir(dir)) != NULL) {
				if (strcasecmp(cur, dirent->d_name) == 0) {
					strcpy(cur, dirent->d_name);
					break;
				}
			}

			closedir(dir);

			if (ptr != base) {
				*ptr = '/';
			}

			if (next) {
				*next = '/';
				ptr = next;
			} else {
				ptr++;
			}
		} else {
			ptr++;
		}
	}

	if (access(filename, R_OK) == 0) {
		return 1;
	}

	return 0;
}

const char *PlatformDataPath(const char *name, char *out, size_t outlen)
{
	PlatformInitPaths();

	const char *start = name;
	if (start[0] == ':') start++;            /* ":Data:..." -> "Data:..." */

	/* absolute paths are passed through untouched */
	if (start[0] == '/') {
		snprintf(out, outlen, "%s", start);
		return out;
	}

	snprintf(out, outlen, "%s%s", g_dataRoot, start);

	/* Only the game's own ":Data:Models:..." part is a Mac OS 9 path.  The
	   install location is a real POSIX path and may contain ':' itself (a
	   '/' typed in a Finder name is stored as ':'), so leave that alone. */
	size_t skip = strlen(g_dataRoot);
	if (skip > strlen(out)) skip = strlen(out);
	for (char *c = out + skip; *c; ++c) {
		if (*c == ':') *c = '/';
	}

	if (!find_filename(out)) {
		/* leave the (unrepaired) path in place: the caller reports the error */
	}
	return out;
}

const char *PlatformPrefPath(const char *name, char *out, size_t outlen)
{
	PlatformInitPaths();
	snprintf(out, outlen, "%s%s", g_prefRoot, name);
	return out;
}

const char *PlatformUserFileForRead(const char *prefName, const char *dataName,
                                    char *out, size_t outlen)
{
	PlatformPrefPath(prefName, out, outlen);
	if (PlatformFileExists(out)) return out;

	if (dataName) {
		PlatformDataPath(dataName, out, outlen);
		if (PlatformFileExists(out)) return out;
	}
	return NULL;
}

/*
Convenient Filename Hacks
*/

FILE *cfh_fopen(const char *filename, const char *mode)
{
	char filename1[MAX_PATH];

	PlatformDataPath(filename, filename1, sizeof(filename1));

	return fopen(filename1, mode);
}

int Files::OpenFile(Str255 Name)
{
	char filename1[MAX_PATH];

	PlatformDataPath((const char *)Name, filename1, sizeof(filename1));

	sFile = open(filename1, O_RDONLY | O_BINARY);
	if (sFile == -1) {
		fprintf(stderr, "ERROR: could not open %s\n", filename1);
	}
	return sFile;
}

void Files::EndLoad()
{
	if (sFile != -1) {
		FSClose( sFile );
	}

	sFile = -1;
}

/* ------------------------------------------------------------------ */
/* Ogg Vorbis                                                          */
/* ------------------------------------------------------------------ */

#include "thirdparty/stb_vorbis.h"

/*
Read the requested OGG file into memory, and extract the information required
by OpenAL.  (stb_vorbis replaces libvorbisfile so the game has no codec
dependency to install.)
*/
void LoadOGG_CFH(const char *filename, ALenum *format, void **wave,
	unsigned int *size, ALsizei *freq)
{
	char filename1[MAX_PATH];

	PlatformDataPath(filename, filename1, sizeof(filename1));

	int channels = 0;
	int rate = 0;
	short *decoded = NULL;
	const int samples = stb_vorbis_decode_filename(filename1, &channels, &rate, &decoded);

	if (samples < 0 || decoded == NULL) {
		fprintf(stderr, "ERROR: unable to decode %s\n", filename1);
		*format = AL_FORMAT_MONO16;
		*wave = NULL;
		*size = 0;
		*freq = 22050;
		return;
	}

	if (channels == 1) {
		*format = AL_FORMAT_MONO16;
	} else if (channels == 2) {
		*format = AL_FORMAT_STEREO16;
	} else {
		fprintf(stderr, "ERROR: ogg %s has %d channels\n", filename1, channels);
		free(decoded);
		*format = AL_FORMAT_MONO16;
		*wave = NULL;
		*size = 0;
		*freq = 22050;
		return;
	}

	*wave = decoded;
	*size = (unsigned int)samples * (unsigned int)channels * sizeof(short);
	*freq = rate;
}

/*
Free the OGG buffer
*/
void FreeOGG(ALenum format, void *wave, unsigned int size,
	ALsizei freq)
{
	(void)format; (void)size; (void)freq;
	free(wave);
}
