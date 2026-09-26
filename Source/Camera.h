#ifndef _CAMERA_H_
#define _CAMERA_H_


/**> HEADER FILES <**/
#include "GLHeaders.h"

#include "Quaternions.h"

class Camera
{
	public:
		XYZ position;
		XYZ oldposition;
		XYZ targetoffset;
		
	 	float rotation, rotation2;
	 	float oldrotation, oldrotation2;
	 	float oldoldrotation, oldoldrotation2;
	 	float visrotation,  visrotation2;
		void Apply();
};

#endif

