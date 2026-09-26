/* stb_image implementation, compiled once.  PNG is all the game ships, but
   TGA and BMP are cheap to keep around for anyone dropping in their own art. */
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_TGA
#define STBI_ONLY_BMP
#include "stb_image.h"
