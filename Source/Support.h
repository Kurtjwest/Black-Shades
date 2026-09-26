#ifndef SUPPORT_H
#define SUPPORT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>

#include <SDL.h>

#include "Audio.h"

#define STUB_FUNCTION fprintf(stderr,"STUB: %s at " __FILE__ ", line %d\n",__FUNCTION__,__LINE__)

#define fsFromStart SEEK_SET

typedef unsigned char * Str255;
typedef int OSErr;
typedef short int SInt16;

typedef bool Boolean;
#ifndef TRUE
#define TRUE true
#endif
#ifndef FALSE
#define FALSE false
#endif

typedef struct UnsignedWide
{
	unsigned int lo;
	unsigned int hi;
} UnsignedWide;

typedef struct Point
{
	int h;
	int v;
} Point;

#define SetFPos(fildes, whence, offset) lseek(fildes, offset, whence)
#define FSClose(fildes) close(fildes)

int Random();
void Microseconds(UnsignedWide *microTickCount);

/* ----------------------------------------------------------------------
   Nintendo Switch (devkitPro/libnx).  The console has no keyboard, mouse or
   window manager, so the controller stands in for all three (see the pad
   code in GameLoop.cpp), OpenGL is loaded by hand, and anything written to
   the SD card has to be committed before it is really there.
   ---------------------------------------------------------------------- */
/* A game controller can stand in for the mouse anywhere (see the pad code in
   GameLoop.cpp); on a Switch it is the only pointer there is. */
void PlatformPadMouse(int dx, int dy);       /* stick motion, as mouse motion */
void PlatformPadButton(bool down);           /* a trigger, as the mouse button */
void PlatformPadPointerMove(int dx, int dy); /* nudge the menu pointer */
void PlatformPadPointerTo(int x, int y);     /* put it somewhere (touch) */

#ifdef __SWITCH__
bool PlatformLoadGL(void);             /* glad, once the GL context exists */
void PlatformCommitSave(void);         /* flush the SD card after a save */
#else
#define PlatformCommitSave() ((void)0)
#endif
void GetMouse(Point *p);
void GetMouseRel(Point *p);
void GetKeys(unsigned long *keys);
int Button(void);

void LoadOGG_CFH(const char *filename, ALenum *format, void **wave,
	unsigned int *size, ALsizei *freq);
void FreeOGG(ALenum format, void *wave, unsigned int size,
	ALsizei freq);

FILE *cfh_fopen(const char *filename, const char *mode);

/* ----------------------------------------------------------------------
   Platform layer (SDL2).  Window and GL context live here so that any
   part of the game can ask for a swap, a fullscreen toggle or the mouse.
   ---------------------------------------------------------------------- */

extern SDL_Window   *g_window;
extern SDL_GLContext g_glcontext;

/* Monotonic clock, nanoseconds since the game started. */
double PlatformTimeNanos(void);

/* Where the game's read-only data and the player's own files live.
   Call PlatformInitPaths() once, before touching any file. */
void        PlatformInitPaths(void);
const char *PlatformDataRoot(void);     /* directory that holds "Data/" */
const char *PlatformPrefRoot(void);     /* per-user writable directory  */

/* Resolve a game path - ":Data:Models:Head.solid" or "Data/customlevels.txt" -
   into a real path under the data root, fixing up the ':' separators and
   correcting the letter case if the filesystem is case sensitive. */
const char *PlatformDataPath(const char *name, char *out, size_t outlen);

/* Path for a file the game writes (config.txt, Highscore): always inside the
   per-user preferences directory, never inside the .app bundle. */
const char *PlatformPrefPath(const char *name, char *out, size_t outlen);

/* Existing copy of a user file to read: the per-user one if present,
   otherwise the one shipped next to the data (returns NULL if neither). */
const char *PlatformUserFileForRead(const char *prefName, const char *dataName,
                                    char *out, size_t outlen);

bool PlatformFileExists(const char *path);

/* Mouse / window helpers. */
void PlatformSetRelativeMouse(bool enable);
bool PlatformRelativeMouse(void);
void PlatformToggleFullscreen(void);
bool PlatformIsFullscreen(void);
void PlatformUpdateViewportSize(int *windowW, int *windowH);
/* glViewport() over the whole window, in pixels (Retina-aware). */
void PlatformSetViewport(void);
void PlatformSwapBuffers(void);

#endif
