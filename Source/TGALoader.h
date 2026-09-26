#ifdef OS9 
#pragma once
#endif

#ifndef	_TGA_LOADER_H_
#define	_TGA_LOADER_H_


/**> HEADER FILES <**/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "GLHeaders.h"

/**> DATA STRUCTURES <**/
typedef struct TGAImageRec
{
	GLubyte	*data;		// Image Data (Up To 32 Bits)
	GLuint	bpp;		// Image Color Depth In Bits Per Pixel.
	GLuint	sizeX;
	GLuint	sizeY;
}	TGAImageRec;



/**> FUNCTION PROTOTYPES <**/
TGAImageRec*	LoadTGA( char *filename );


#endif
