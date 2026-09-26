#ifndef _TEXT_H_
#define _TEXT_H_


/**> HEADER FILES <**/
#include "Quaternions.h"
#include "GLHeaders.h"
#include "Files.h"
#include "Quaternions.h"

class Text{
	public:
		GLuint FontTexture;
		GLuint base;
		
		void LoadFontTexture(char *fileName);
		void BuildFont();
		void glPrint(GLint x, GLint y, char *string, int set, float size, float width, float height);
		
		/* see the note in Game.h: deleting textures here was a crash on exit */
		~Text(){
		}
};

#endif

