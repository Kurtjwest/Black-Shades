/*
 * MiniAL.h - the small slice of the OpenAL 1.1 API that Black Shades uses,
 * implemented on top of SDL2's audio device (see MiniAL.cpp).
 *
 * Why: the game needs OpenAL's AL_MIN_GAIN / AL_MAX_GAIN semantics (the gain
 * clamp that is applied *after* distance attenuation) - that is how every
 * non-positional sound in the game, including all of the music, is kept at
 * full volume while the listener walks around the city.  Apple's OpenAL
 * framework has been deprecated since macOS 10.15 and its gain-clamp behaviour
 * is not something we want to depend on, and pulling in OpenAL Soft would mean
 * a Homebrew/CMake dependency for everyone who builds this.  So we implement
 * the ~20 entry points the game calls, with spec behaviour, in one file.
 *
 * The declarations below are deliberately signature-compatible with <AL/al.h>,
 * so building with USE_SYSTEM_OPENAL=1 swaps a real OpenAL back in unchanged.
 */
#ifndef BS_MINIAL_H
#define BS_MINIAL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef char            ALboolean;
typedef char            ALchar;
typedef signed char     ALbyte;
typedef unsigned char   ALubyte;
typedef short           ALshort;
typedef unsigned short  ALushort;
typedef int             ALint;
typedef unsigned int    ALuint;
typedef int             ALsizei;
typedef int             ALenum;
typedef float           ALfloat;
typedef double          ALdouble;
typedef void            ALvoid;

#define AL_NONE                    0
#define AL_FALSE                   0
#define AL_TRUE                    1

#define AL_SOURCE_RELATIVE         0x202
#define AL_PITCH                   0x1003
#define AL_POSITION                0x1004
#define AL_DIRECTION               0x1005
#define AL_VELOCITY                0x1006
#define AL_LOOPING                 0x1007
#define AL_BUFFER                  0x1009
#define AL_GAIN                    0x100A
#define AL_MIN_GAIN                0x100D
#define AL_MAX_GAIN                0x100E
#define AL_ORIENTATION             0x100F
#define AL_SOURCE_STATE            0x1010
#define AL_INITIAL                 0x1011
#define AL_PLAYING                 0x1012
#define AL_PAUSED                  0x1013
#define AL_STOPPED                 0x1014
#define AL_BUFFERS_QUEUED          0x1015
#define AL_BUFFERS_PROCESSED       0x1016
#define AL_REFERENCE_DISTANCE      0x1020
#define AL_ROLLOFF_FACTOR          0x1021
#define AL_MAX_DISTANCE            0x1023
#define AL_SEC_OFFSET              0x1024

#define AL_FORMAT_MONO8            0x1100
#define AL_FORMAT_MONO16           0x1101
#define AL_FORMAT_STEREO8          0x1102
#define AL_FORMAT_STEREO16         0x1103

#define AL_NO_ERROR                0
#define AL_INVALID_NAME            0xA001
#define AL_INVALID_ENUM            0xA002
#define AL_INVALID_VALUE           0xA003
#define AL_INVALID_OPERATION       0xA004
#define AL_OUT_OF_MEMORY           0xA005

#define AL_INVERSE_DISTANCE        0xD001
#define AL_INVERSE_DISTANCE_CLAMPED 0xD002

void      alGenSources(ALsizei n, ALuint *sources);
void      alDeleteSources(ALsizei n, const ALuint *sources);
ALboolean alIsSource(ALuint source);
void      alGenBuffers(ALsizei n, ALuint *buffers);
void      alDeleteBuffers(ALsizei n, const ALuint *buffers);
void      alBufferData(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei freq);

void      alSourcei(ALuint source, ALenum param, ALint value);
void      alSourcef(ALuint source, ALenum param, ALfloat value);
void      alSource3f(ALuint source, ALenum param, ALfloat v1, ALfloat v2, ALfloat v3);
void      alSourcefv(ALuint source, ALenum param, const ALfloat *values);
void      alGetSourcei(ALuint source, ALenum param, ALint *value);
void      alGetSourceiv(ALuint source, ALenum param, ALint *values);
void      alGetSourcef(ALuint source, ALenum param, ALfloat *value);

void      alSourcePlay(ALuint source);
void      alSourceStop(ALuint source);
void      alSourcePause(ALuint source);
void      alSourceRewind(ALuint source);

void      alListenerf(ALenum param, ALfloat value);
void      alListener3f(ALenum param, ALfloat v1, ALfloat v2, ALfloat v3);
void      alListenerfv(ALenum param, const ALfloat *values);

void      alDistanceModel(ALenum model);
ALenum    alGetError(void);

#ifdef __cplusplus
}
#endif

#endif /* BS_MINIAL_H */
