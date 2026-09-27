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

#if defined(__EMSCRIPTEN__)
  /* Emscripten's own GL headers, sitting on its fixed-function emulation
     over WebGL 1 (-sLEGACY_GL_EMULATION).  The emulation covers most of
     what this renderer asks for; the two gaps it leaves are papered over at
     the bottom of this file. */
  #include <GL/gl.h>
  #include <GLES2/gl2.h>          /* glGenerateMipmap, which WebGL has */
  /* WebGL dropped GL_CLAMP along with texture borders; clamping to the edge
     is what every driver does with it in practice anyway. */
  #undef  GL_CLAMP
  #define GL_CLAMP GL_CLAMP_TO_EDGE
#elif defined(__SWITCH__)
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

/* ----------------------------------------------------------------------
   The web's two gaps.

   Emscripten's emulation has no GL_COLOR_MATERIAL: with lighting on it works
   the vertex colour out from the material uniforms alone and throws glColor
   away, so a game that colours the world with glColor - as this one does,
   with colour material on for the whole of it - comes out grey.  glColor is
   therefore routed through a stand-in that also writes the colour to the
   material, and glEnable/glDisable through one that knows what
   GL_COLOR_MATERIAL and GL_LIGHTING are doing (a cap the emulation does not
   know would otherwise reach WebGL and raise an error every frame).

   None of this exists off the web: everywhere else the calls below are the
   real ones, unwrapped.
   ---------------------------------------------------------------------- */
#if defined(__EMSCRIPTEN__) && !defined(BS_GL_SHIM_IMPLEMENTATION)

#ifdef __cplusplus
extern "C" {
#endif
void BS_Enable (GLenum cap);
void BS_Disable(GLenum cap);
void BS_Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
int  BS_LightingEnabled(void);
#ifdef __cplusplus
}
#endif

#define glEnable(cap)       BS_Enable(cap)
#define glDisable(cap)      BS_Disable(cap)
#define glColor4f(r,g,b,a)  BS_Color4f((r),(g),(b),(a))
#define glColor3f(r,g,b)    BS_Color4f((r),(g),(b),1.0f)

#endif

#endif /* BS_GLHEADERS_H */
