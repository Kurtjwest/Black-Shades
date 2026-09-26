#ifndef _DECALS_H_
#define _DECALS_H_

#include "Quaternions.h"
#include "GLHeaders.h"
#include "Files.h"
#include "Quaternions.h"
#include "Camera.h"
#include "Models.h"
#include "Fog.h"
//
// Model Structures
//

#define maxdecals 120

class Decals{
	public:
				GLuint 				bulletholetextureptr;
				GLuint 				cratertextureptr;
				GLuint 				bloodtextureptr[11];
				
				int howmanydecals;
				
				int type[maxdecals];
				
				XYZ points[8*maxdecals];
				int numpoints[maxdecals];
				float texcoordsx[8*maxdecals];
				float texcoordsy[8*maxdecals];
				float alivetime[maxdecals];
				
				void draw();
				
				int DeleteDecal(int which);
				int MakeDecal(int atype, XYZ location, float size, XYZ normal, int poly, Model *model, XYZ move, float rotation);
				
				void DoStuff();
				void LoadBulletHoleTexture(char *fileName);
				void LoadCraterTexture(char *fileName);
				void LoadBloodTexture(char *fileName, int which);
				
				/* see the note in Game.h: deleting textures here was a crash
				   on exit (and passed texture names as pointers) */
				~Decals() {
				};
};

#endif

