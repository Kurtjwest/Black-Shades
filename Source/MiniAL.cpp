/*
 * MiniAL.cpp - a small, self-contained OpenAL 1.1 subset on top of SDL2 audio.
 *
 * Implements exactly what Black Shades uses:
 *   - mono/stereo 16-bit buffers
 *   - per-source gain, min/max gain, pitch, looping, position
 *   - listener position + orientation
 *   - AL_INVERSE_DISTANCE_CLAMPED attenuation (the OpenAL default)
 *   - play / pause / stop / rewind and AL_SOURCE_STATE queries
 *
 * Gain follows the spec order the game depends on:
 *     gain = clamp(source_gain * distance_attenuation, min_gain, max_gain)
 * so a source with AL_MIN_GAIN 1 (all of the music, and the "2D" effects) is
 * heard at full volume no matter where the listener is, which is exactly how
 * the game mixes itself.
 *
 * Multi-channel buffers are not attenuated or panned (same rule OpenAL Soft
 * uses for non-mono sources); they are played straight through.
 *
 * Mixing happens in SDL's audio callback; every public entry point takes the
 * audio lock, so the game can poke sources from the main thread as it likes.
 */

#include <SDL.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "MiniAL.h"
#include "Audio.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {

const int   kOutFreq     = 44100;
const int   kOutChannels = 2;
const int   kOutSamples  = 1024;   /* ~23ms of latency */
const float kGainRampPerSample = 1.0f / 256.0f;  /* click-free gain changes */

struct MABuffer {
    Sint16 *data;      /* interleaved 16-bit samples, owned */
    int     frames;    /* frames (not samples) */
    int     channels;
    int     freq;
    MABuffer() : data(NULL), frames(0), channels(1), freq(kOutFreq) {}
};

struct MASource {
    bool   used;
    int    state;          /* AL_INITIAL / AL_PLAYING / AL_PAUSED / AL_STOPPED */
    ALuint buffer;
    double pos;            /* playback cursor, in source frames */
    float  pitch;
    float  gain, minGain, maxGain;
    float  refDistance, rolloff, maxDistance;
    float  position[3];
    bool   relative;
    bool   looping;
    /* smoothed output gains, to avoid zipper noise on abrupt changes */
    float  curL, curR;
    bool   primed;

    MASource()
        : used(false), state(AL_INITIAL), buffer(0), pos(0.0), pitch(1.0f),
          gain(1.0f), minGain(0.0f), maxGain(1.0f),
          refDistance(1.0f), rolloff(1.0f), maxDistance(3.4e38f),
          relative(false), looping(false), curL(0.0f), curR(0.0f), primed(false)
    {
        position[0] = position[1] = position[2] = 0.0f;
    }
};

struct MAListener {
    float position[3];
    float at[3];
    float up[3];
    float gain;
    MAListener() : gain(1.0f) {
        position[0] = position[1] = position[2] = 0.0f;
        at[0] = 0.0f; at[1] = 0.0f; at[2] = -1.0f;
        up[0] = 0.0f; up[1] = 1.0f; up[2] = 0.0f;
    }
};

std::vector<MABuffer>  g_buffers;    /* index 0 unused: AL name 0 means "none" */
std::vector<MASource>  g_sources;
MAListener             g_listener;
SDL_AudioDeviceID      g_device   = 0;
ALenum                 g_error    = AL_NO_ERROR;
ALenum                 g_distanceModel = AL_INVERSE_DISTANCE_CLAMPED;
std::vector<float>     g_mixbuf;

inline void Lock()   { if (g_device) SDL_LockAudioDevice(g_device); }
inline void Unlock() { if (g_device) SDL_UnlockAudioDevice(g_device); }

inline float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

MASource *GetSource(ALuint name) {
    if (name == 0 || name >= g_sources.size() || !g_sources[name].used) {
        g_error = AL_INVALID_NAME;
        return NULL;
    }
    return &g_sources[name];
}

MABuffer *GetBuffer(ALuint name) {
    if (name == 0 || name >= g_buffers.size()) return NULL;
    return &g_buffers[name];
}

/* Compute the left/right gains for a source, per the OpenAL 1.1 rules. */
void ComputeGains(const MASource &src, const MABuffer *buf, float *outL, float *outR)
{
    float gain = src.gain;
    float pan  = 0.0f;   /* -1 hard left .. +1 hard right */

    const bool spatialize = (buf && buf->channels == 1);

    if (spatialize) {
        float dx = src.position[0];
        float dy = src.position[1];
        float dz = src.position[2];
        if (!src.relative) {
            dx -= g_listener.position[0];
            dy -= g_listener.position[1];
            dz -= g_listener.position[2];
        }
        float dist = sqrtf(dx * dx + dy * dy + dz * dz);

        if (g_distanceModel != AL_NONE) {
            float ref = src.refDistance > 0.0f ? src.refDistance : 1.0f;
            float d   = dist;
            if (g_distanceModel == AL_INVERSE_DISTANCE_CLAMPED) {
                if (d < ref) d = ref;
                if (d > src.maxDistance) d = src.maxDistance;
            } else if (d < 1e-6f) {
                d = 1e-6f;
            }
            float denom = ref + src.rolloff * (d - ref);
            gain *= (denom > 1e-6f) ? (ref / denom) : 1.0f;
        }

        if (dist > 1e-6f) {
            /* listener basis: right = at x up (OpenAL is right-handed) */
            const float *at = g_listener.at;
            const float *up = g_listener.up;
            float rx = at[1] * up[2] - at[2] * up[1];
            float ry = at[2] * up[0] - at[0] * up[2];
            float rz = at[0] * up[1] - at[1] * up[0];
            float rl = sqrtf(rx * rx + ry * ry + rz * rz);
            if (rl > 1e-6f) {
                rx /= rl; ry /= rl; rz /= rl;
                pan = clampf((dx * rx + dy * ry + dz * rz) / dist, -1.0f, 1.0f);
            }
        }
    }

    /* The clamp the game leans on: applied after distance attenuation. */
    gain = clampf(gain, src.minGain, src.maxGain);
    gain *= g_listener.gain;

    if (spatialize) {
        /* constant-power pan: a centred source sits at -3dB in both ears,
           which is what OpenAL Soft's stereo pan-pot does too */
        float angle = (pan + 1.0f) * (float)(M_PI / 4.0);
        *outL = gain * cosf(angle);
        *outR = gain * sinf(angle);
    } else {
        /* multi-channel buffer: straight through, no panning */
        *outL = gain;
        *outR = gain;
    }
}

void MixSource(MASource &src, float *out, int frames)
{
    MABuffer *buf = GetBuffer(src.buffer);
    if (!buf || !buf->data || buf->frames <= 0) {
        src.state = AL_STOPPED;
        return;
    }

    float targetL, targetR;
    ComputeGains(src, buf, &targetL, &targetR);
    if (!src.primed) {                 /* first block: start at target */
        src.curL = targetL;
        src.curR = targetR;
        src.primed = true;
    }

    const double step = (double)src.pitch * (double)buf->freq / (double)kOutFreq;
    const int    ch   = buf->channels;
    const int    n    = buf->frames;

    for (int i = 0; i < frames; ++i) {
        if (src.pos >= (double)n) {
            if (src.looping) {
                src.pos = fmod(src.pos, (double)n);
            } else {
                src.state = AL_STOPPED;
                src.pos   = 0.0;
                src.primed = false;
                return;
            }
        }

        int    i0 = (int)src.pos;
        double fr = src.pos - (double)i0;
        int    i1 = i0 + 1;
        if (i1 >= n) i1 = src.looping ? 0 : i0;

        float l, r;
        if (ch == 1) {
            float s0 = buf->data[i0] * (1.0f / 32768.0f);
            float s1 = buf->data[i1] * (1.0f / 32768.0f);
            float s  = s0 + (s1 - s0) * (float)fr;
            l = r = s;
        } else {
            float l0 = buf->data[i0 * 2 + 0] * (1.0f / 32768.0f);
            float l1 = buf->data[i1 * 2 + 0] * (1.0f / 32768.0f);
            float r0 = buf->data[i0 * 2 + 1] * (1.0f / 32768.0f);
            float r1 = buf->data[i1 * 2 + 1] * (1.0f / 32768.0f);
            l = l0 + (l1 - l0) * (float)fr;
            r = r0 + (r1 - r0) * (float)fr;
        }

        /* ramp towards the target gain */
        if (src.curL != targetL) {
            float d = targetL - src.curL;
            float m = d > 0 ? kGainRampPerSample : -kGainRampPerSample;
            src.curL = (fabsf(d) <= kGainRampPerSample) ? targetL : src.curL + m;
        }
        if (src.curR != targetR) {
            float d = targetR - src.curR;
            float m = d > 0 ? kGainRampPerSample : -kGainRampPerSample;
            src.curR = (fabsf(d) <= kGainRampPerSample) ? targetR : src.curR + m;
        }

        out[i * 2 + 0] += l * src.curL;
        out[i * 2 + 1] += r * src.curR;

        src.pos += step;
    }
}

void SDLCALL AudioCallback(void * /*userdata*/, Uint8 *stream, int len)
{
    const int frames = len / (kOutChannels * (int)sizeof(float));
    float *out = (float *)stream;
    memset(stream, 0, (size_t)len);

    for (size_t i = 1; i < g_sources.size(); ++i) {
        MASource &src = g_sources[i];
        if (!src.used || src.state != AL_PLAYING) continue;
        MixSource(src, out, frames);
    }

    /* keep the sum in range; the game happily plays a dozen things at once */
    for (int i = 0; i < frames * kOutChannels; ++i) {
        float v = out[i];
        if (v > 1.0f) v = 1.0f;
        else if (v < -1.0f) v = -1.0f;
        out[i] = v;
    }
}

} /* anonymous namespace */

/* ------------------------------------------------------------------ */
/* Device setup                                                        */
/* ------------------------------------------------------------------ */

void Audio_Init(void)
{
    if (g_device) return;

    if (!SDL_WasInit(SDL_INIT_AUDIO) && SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "MiniAL: could not init SDL audio: %s\n", SDL_GetError());
        return;
    }

    SDL_AudioSpec want;
    SDL_zero(want);
    want.freq     = kOutFreq;
    want.format   = AUDIO_F32SYS;
    want.channels = kOutChannels;
    want.samples  = kOutSamples;
    want.callback = AudioCallback;

    SDL_AudioSpec have;
    /* allowed_changes = 0: SDL converts for us, so the callback always sees
       44100Hz float stereo no matter what the hardware wants. */
    g_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (!g_device) {
        fprintf(stderr, "MiniAL: could not open audio device: %s\n", SDL_GetError());
        return;
    }

    g_buffers.resize(1);   /* name 0 is reserved */
    g_sources.resize(1);
    SDL_PauseAudioDevice(g_device, 0);
}

void Audio_Shutdown(void)
{
    if (g_device) {
        SDL_PauseAudioDevice(g_device, 1);
        SDL_CloseAudioDevice(g_device);
        g_device = 0;
    }
    for (size_t i = 0; i < g_buffers.size(); ++i) {
        free(g_buffers[i].data);
        g_buffers[i].data = NULL;
    }
    g_buffers.clear();
    g_sources.clear();
}

/* ------------------------------------------------------------------ */
/* Objects                                                             */
/* ------------------------------------------------------------------ */

void alGenSources(ALsizei n, ALuint *sources)
{
    Lock();
    if (g_sources.empty()) g_sources.resize(1);
    for (ALsizei i = 0; i < n; ++i) {
        ALuint name = 0;
        for (size_t s = 1; s < g_sources.size(); ++s) {
            if (!g_sources[s].used) { name = (ALuint)s; break; }
        }
        if (!name) {
            g_sources.push_back(MASource());
            name = (ALuint)(g_sources.size() - 1);
        }
        g_sources[name] = MASource();
        g_sources[name].used = true;
        sources[i] = name;
    }
    Unlock();
}

void alDeleteSources(ALsizei n, const ALuint *sources)
{
    Lock();
    for (ALsizei i = 0; i < n; ++i) {
        ALuint name = sources[i];
        if (name != 0 && name < g_sources.size()) {
            g_sources[name] = MASource();   /* used = false */
        }
    }
    Unlock();
}

ALboolean alIsSource(ALuint source)
{
    return (source != 0 && source < g_sources.size() && g_sources[source].used) ? AL_TRUE : AL_FALSE;
}

void alGenBuffers(ALsizei n, ALuint *buffers)
{
    Lock();
    if (g_buffers.empty()) g_buffers.resize(1);
    for (ALsizei i = 0; i < n; ++i) {
        g_buffers.push_back(MABuffer());
        buffers[i] = (ALuint)(g_buffers.size() - 1);
    }
    Unlock();
}

void alDeleteBuffers(ALsizei n, const ALuint *buffers)
{
    Lock();
    for (ALsizei i = 0; i < n; ++i) {
        MABuffer *buf = GetBuffer(buffers[i]);
        if (buf) {
            free(buf->data);
            buf->data   = NULL;
            buf->frames = 0;
        }
    }
    Unlock();
}

void alBufferData(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei freq)
{
    Lock();
    MABuffer *buf = GetBuffer(buffer);
    if (!buf) { g_error = AL_INVALID_NAME; Unlock(); return; }

    free(buf->data);
    buf->data   = NULL;
    buf->frames = 0;
    buf->freq   = freq > 0 ? freq : kOutFreq;

    int channels = 1, bits = 16;
    switch (format) {
        case AL_FORMAT_MONO8:    channels = 1; bits = 8;  break;
        case AL_FORMAT_MONO16:   channels = 1; bits = 16; break;
        case AL_FORMAT_STEREO8:  channels = 2; bits = 8;  break;
        case AL_FORMAT_STEREO16: channels = 2; bits = 16; break;
        default: g_error = AL_INVALID_ENUM; Unlock(); return;
    }
    buf->channels = channels;

    if (size <= 0 || !data) { Unlock(); return; }

    const int bytesPerFrame = channels * (bits / 8);
    const int frames        = size / bytesPerFrame;
    Sint16 *dst = (Sint16 *)malloc((size_t)frames * channels * sizeof(Sint16));
    if (!dst) { g_error = AL_OUT_OF_MEMORY; Unlock(); return; }

    if (bits == 16) {
        memcpy(dst, data, (size_t)frames * channels * sizeof(Sint16));
    } else {
        const Uint8 *src = (const Uint8 *)data;
        for (int i = 0; i < frames * channels; ++i)
            dst[i] = (Sint16)((int)src[i] - 128) << 8;
    }

    buf->data   = dst;
    buf->frames = frames;
    Unlock();
}

/* ------------------------------------------------------------------ */
/* Source properties                                                   */
/* ------------------------------------------------------------------ */

void alSourcei(ALuint source, ALenum param, ALint value)
{
    Lock();
    MASource *src = GetSource(source);
    if (src) {
        switch (param) {
            case AL_BUFFER:
                src->buffer = (ALuint)value;
                src->pos    = 0.0;
                src->primed = false;
                break;
            case AL_LOOPING:         src->looping  = (value != 0); break;
            case AL_SOURCE_RELATIVE: src->relative = (value != 0); break;
            default: break;
        }
    }
    Unlock();
}

void alSourcef(ALuint source, ALenum param, ALfloat value)
{
    Lock();
    MASource *src = GetSource(source);
    if (src) {
        switch (param) {
            case AL_PITCH:              src->pitch       = value > 0.0f ? value : 0.0001f; break;
            case AL_GAIN:               src->gain        = value < 0.0f ? 0.0f : value; break;
            case AL_MIN_GAIN:           src->minGain     = clampf(value, 0.0f, 1.0f); break;
            case AL_MAX_GAIN:           src->maxGain     = clampf(value, 0.0f, 1.0f); break;
            case AL_REFERENCE_DISTANCE: src->refDistance = value; break;
            case AL_ROLLOFF_FACTOR:     src->rolloff     = value; break;
            case AL_MAX_DISTANCE:       src->maxDistance = value; break;
            default: break;
        }
    }
    Unlock();
}

void alSource3f(ALuint source, ALenum param, ALfloat v1, ALfloat v2, ALfloat v3)
{
    const ALfloat v[3] = { v1, v2, v3 };
    alSourcefv(source, param, v);
}

void alSourcefv(ALuint source, ALenum param, const ALfloat *values)
{
    if (!values) return;
    Lock();
    MASource *src = GetSource(source);
    if (src) {
        switch (param) {
            case AL_POSITION:
                src->position[0] = values[0];
                src->position[1] = values[1];
                src->position[2] = values[2];
                break;
            case AL_VELOCITY:
            case AL_DIRECTION:
                break;   /* unused by the game (no doppler, no cones) */
            default:
                Unlock();
                alSourcef(source, param, values[0]);
                return;
        }
    }
    Unlock();
}

void alGetSourcei(ALuint source, ALenum param, ALint *value)
{
    if (!value) return;
    Lock();
    MASource *src = GetSource(source);
    *value = 0;
    if (src) {
        switch (param) {
            case AL_SOURCE_STATE: *value = src->state; break;
            case AL_BUFFER:       *value = (ALint)src->buffer; break;
            case AL_LOOPING:      *value = src->looping ? AL_TRUE : AL_FALSE; break;
            default: break;
        }
    }
    Unlock();
}

void alGetSourceiv(ALuint source, ALenum param, ALint *values)
{
    alGetSourcei(source, param, values);
}

void alGetSourcef(ALuint source, ALenum param, ALfloat *value)
{
    if (!value) return;
    Lock();
    MASource *src = GetSource(source);
    *value = 0.0f;
    if (src) {
        switch (param) {
            case AL_PITCH:    *value = src->pitch; break;
            case AL_GAIN:     *value = src->gain; break;
            case AL_MIN_GAIN: *value = src->minGain; break;
            case AL_MAX_GAIN: *value = src->maxGain; break;
            default: break;
        }
    }
    Unlock();
}

/* ------------------------------------------------------------------ */
/* Transport                                                           */
/* ------------------------------------------------------------------ */

void alSourcePlay(ALuint source)
{
    Lock();
    MASource *src = GetSource(source);
    if (src) {
        /* spec: Play on a PLAYING source rewinds it; on PAUSED it resumes */
        if (src->state != AL_PAUSED) {
            src->pos    = 0.0;
            src->primed = false;
        }
        src->state = AL_PLAYING;
    }
    Unlock();
}

void alSourceStop(ALuint source)
{
    Lock();
    MASource *src = GetSource(source);
    if (src && src->state != AL_INITIAL) {
        src->state  = AL_STOPPED;
        src->pos    = 0.0;
        src->primed = false;
    }
    Unlock();
}

void alSourcePause(ALuint source)
{
    Lock();
    MASource *src = GetSource(source);
    if (src && src->state == AL_PLAYING) src->state = AL_PAUSED;
    Unlock();
}

void alSourceRewind(ALuint source)
{
    Lock();
    MASource *src = GetSource(source);
    if (src) {
        src->state  = AL_INITIAL;
        src->pos    = 0.0;
        src->primed = false;
    }
    Unlock();
}

/* ------------------------------------------------------------------ */
/* Listener / global                                                   */
/* ------------------------------------------------------------------ */

void alListenerf(ALenum param, ALfloat value)
{
    Lock();
    if (param == AL_GAIN) g_listener.gain = value < 0.0f ? 0.0f : value;
    Unlock();
}

void alListener3f(ALenum param, ALfloat v1, ALfloat v2, ALfloat v3)
{
    const ALfloat v[3] = { v1, v2, v3 };
    alListenerfv(param, v);
}

void alListenerfv(ALenum param, const ALfloat *values)
{
    if (!values) return;
    Lock();
    switch (param) {
        case AL_POSITION:
            g_listener.position[0] = values[0];
            g_listener.position[1] = values[1];
            g_listener.position[2] = values[2];
            break;
        case AL_ORIENTATION:
            g_listener.at[0] = values[0];
            g_listener.at[1] = values[1];
            g_listener.at[2] = values[2];
            g_listener.up[0] = values[3];
            g_listener.up[1] = values[4];
            g_listener.up[2] = values[5];
            break;
        case AL_GAIN:
            g_listener.gain = values[0] < 0.0f ? 0.0f : values[0];
            break;
        default: break;
    }
    Unlock();
}

void alDistanceModel(ALenum model)
{
    Lock();
    g_distanceModel = model;
    Unlock();
}

ALenum alGetError(void)
{
    ALenum e = g_error;
    g_error = AL_NO_ERROR;
    return e;
}
