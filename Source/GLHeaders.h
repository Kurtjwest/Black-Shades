/*
 * GLHeaders.h - one place to get an OpenGL 1.x/2.1 compatibility-profile
 * header on every platform we care about.
 *
 * macOS still ships OpenGL (running on top of Metal on Apple Silicon), but the
 * headers are in OpenGL.framework and every entry point is flagged deprecated,
 * so we silence that warning here rather than in 20 source files.
 *
 * GLU is deliberately NOT used any more: gluPerspective and gluBuild2DMipmaps
 * were the only two calls, and both are replaced with plain GL in this port.
 */
#ifndef BS_GLHEADERS_H
#define BS_GLHEADERS_H

#if defined(__SWITCH__)
  /* devkitPro's Mesa ships no libGL to link against, so every entry point is
     fetched through SDL once the context exists (PlatformLoadGL() in
     Support.cpp).  glad's header, generated for the 2.1 compatibility
     profile, stands in for <GL/gl.h> and turns each gl* call into a call
     through the pointer it loaded. */
  #include <glad/gl.h>
#elif defined(__APPLE__)
  #ifndef GL_SILENCE_DEPRECATION
    #define GL_SILENCE_DEPRECATION 1
  #endif
  #include <OpenGL/gl.h>
#else
  #include <GL/gl.h>
#endif

/* GL_GENERATE_MIPMAP is core in GL 1.4 (and in the 2.1 profile macOS gives
   us), but some old headers only have the SGIS suffixed name. */
#if !defined(GL_GENERATE_MIPMAP) && defined(GL_GENERATE_MIPMAP_SGIS)
  #define GL_GENERATE_MIPMAP GL_GENERATE_MIPMAP_SGIS
#endif
#ifndef GL_GENERATE_MIPMAP
  #define GL_GENERATE_MIPMAP 0x8191
#endif
#ifndef GL_CLAMP_TO_EDGE
  #define GL_CLAMP_TO_EDGE 0x812F
#endif
#ifndef GL_BGR
  #define GL_BGR 0x80E0
#endif
#ifndef GL_BGRA
  #define GL_BGRA 0x80E1
#endif

#endif /* BS_GLHEADERS_H */
