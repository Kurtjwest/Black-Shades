/*
 * Audio.h - pick an OpenAL implementation.
 *
 * Default: MiniAL, the built-in OpenAL-subset mixer (MiniAL.cpp), so the game
 * has no audio dependency beyond SDL2.
 *
 * Build with USE_SYSTEM_OPENAL=1 to use the platform's OpenAL instead
 * (OpenAL.framework on macOS, libopenal elsewhere).
 */
#ifndef BS_AUDIO_H
#define BS_AUDIO_H

#ifdef USE_SYSTEM_OPENAL
  #ifdef __APPLE__
    #define OPENAL_DEPRECATED /* keep the deprecation attributes quiet */
    #include <OpenAL/al.h>
    #include <OpenAL/alc.h>
  #else
    #include <AL/al.h>
    #include <AL/alc.h>
  #endif
#else
  #include "MiniAL.h"
#endif

/* Replaces alutInit()/alutExit(): opens the output device and starts mixing. */
void Audio_Init(void);
void Audio_Shutdown(void);

#endif /* BS_AUDIO_H */
