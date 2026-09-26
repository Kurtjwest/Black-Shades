/*
 * Texture loading.  The 2008 SDL_image based loader that used to live here has
 * been replaced with stb_image, so PNG support needs no external library.
 */
#ifndef TEXTURES_H
#define TEXTURES_H

#include "GLHeaders.h"

GLuint loadTexture(const char* filename, GLenum minFilter = GL_LINEAR, GLenum magFilter = GL_LINEAR, bool mipmaps = true);

#endif
