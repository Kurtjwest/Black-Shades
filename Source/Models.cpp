#include "Models.h"

#include "Serialize.h"

//Functions
void Model::UpdateVertexArray(){
	int i;
	for(i=0;i<TriangleNum;i++){
		vArray[i*27+0]=vertex[Triangles[i].vertex[0]].x;
		vArray[i*27+1]=vertex[Triangles[i].vertex[0]].y;
		vArray[i*27+2]=vertex[Triangles[i].vertex[0]].z;
		vArray[i*27+3]=normals[i].x;
		vArray[i*27+4]=normals[i].y;
		vArray[i*27+5]=normals[i].z;
		vArray[i*27+6]=Triangles[i].r;
		vArray[i*27+7]=Triangles[i].g;
		vArray[i*27+8]=Triangles[i].b;
		
		vArray[i*27+9]=vertex[Triangles[i].vertex[1]].x;
		vArray[i*27+10]=vertex[Triangles[i].vertex[1]].y;
		vArray[i*27+11]=vertex[Triangles[i].vertex[1]].z;
		vArray[i*27+12]=normals[i].x;
		vArray[i*27+13]=normals[i].y;
		vArray[i*27+14]=normals[i].z;
		vArray[i*27+15]=Triangles[i].r;
		vArray[i*27+16]=Triangles[i].g;
		vArray[i*27+17]=Triangles[i].b;
		
		vArray[i*27+18]=vertex[Triangles[i].vertex[2]].x;
		vArray[i*27+19]=vertex[Triangles[i].vertex[2]].y;
		vArray[i*27+20]=vertex[Triangles[i].vertex[2]].z;
		vArray[i*27+21]=normals[i].x;
		vArray[i*27+22]=normals[i].y;
		vArray[i*27+23]=normals[i].z;
		vArray[i*27+24]=Triangles[i].r;
		vArray[i*27+25]=Triangles[i].g;
		vArray[i*27+26]=Triangles[i].b;
	}
	
	XYZ average;
	int howmany;
	average=0;
	howmany=0;
	boundingboxmin=20000;
	boundingboxmax=-20000;
	for(int i=0;i<vertexNum;i++){
		howmany++;
		average=average+vertex[i];
		if(vertex[i].x<boundingboxmin.x)boundingboxmin.x=vertex[i].x;
		if(vertex[i].y<boundingboxmin.y)boundingboxmin.y=vertex[i].y;
		if(vertex[i].z<boundingboxmin.z)boundingboxmin.z=vertex[i].z;
		if(vertex[i].x>boundingboxmax.x)boundingboxmax.x=vertex[i].x;
		if(vertex[i].y>boundingboxmax.y)boundingboxmax.y=vertex[i].y;
		if(vertex[i].z>boundingboxmax.z)boundingboxmax.z=vertex[i].z;
	}
	average=average/howmany;
	boundingspherecenter=average;
	boundingsphereradius=0;
	for(int i=0;i<vertexNum;i++){
		if(findDistancefast(average,vertex[i])>boundingsphereradius)boundingsphereradius=findDistancefast(average,vertex[i]);
	}
	boundingsphereradius=fast_sqrt(boundingsphereradius);
}

bool Model::load(Str255 Name)
{
	int					tfile;   /* a file descriptor, not a Mac file ref */
	long				err;
	Files file;

	tfile=file.OpenFile(Name);
	if (tfile == -1) {
		PlatformLogf("   model %s: COULD NOT OPEN\n",(const char *)Name);
		return 0;
	}
	SetFPos(tfile,fsFromStart,0);

		// read model settings
	
	err=ReadShort(tfile,1,&vertexNum);
	err=ReadShort(tfile,1,&TriangleNum);

	PlatformLogf("   model %s: %d vertices, %d triangles (limits %d/%d)\n",
	             (const char *)Name,vertexNum,TriangleNum,
	             max_model_vertex,max_textured_triangle);

	/* the counts come from the file and size fixed arrays: a damaged model
	   must not be read past their ends */
	if(vertexNum<0||vertexNum>max_model_vertex||TriangleNum<0||TriangleNum>max_textured_triangle){
		fprintf(stderr,"ERROR: %s is damaged (%d vertices, %d triangles)\n",(char*)Name,vertexNum,TriangleNum);
		vertexNum=TriangleNum=0;
		FSClose(tfile);
		return 0;
	}
	
		// read the model data
	
	err=ReadXYZ(tfile,vertexNum,vertex);
	err=ReadTexturedTriangle(tfile,TriangleNum,Triangles);

	FSClose(tfile);

	for(int i=0;i<TriangleNum;i++){
		for(int j=0;j<3;j++){
			if(Triangles[i].vertex[j]<0||Triangles[i].vertex[j]>=vertexNum){
				fprintf(stderr,"ERROR: %s is damaged (triangle %d uses vertex %d of %d)\n",(char*)Name,i,Triangles[i].vertex[j],vertexNum);
				vertexNum=TriangleNum=0;
				return 0;
			}
		}
	}
		
	UpdateVertexArray();
	
	XYZ average;
	int howmany;
	average=0;
	howmany=0;
	for(int i=0;i<vertexNum;i++){
		howmany++;
		average=average+vertex[i];
	}
	average=average/howmany;
	boundingspherecenter=average;
	boundingsphereradius=0;
	for(int i=0;i<vertexNum;i++){
		if(findDistancefast(average,vertex[i])>boundingsphereradius)boundingsphereradius=findDistancefast(average,vertex[i]);
	}
	boundingsphereradius=fast_sqrt(boundingsphereradius);
	
	return 1;
}

void Model::Scale(float xscale,float yscale,float zscale)
{
	int i;
	for(i=0; i<vertexNum; i++){
		vertex[i].x*=xscale;
		vertex[i].y*=yscale;
		vertex[i].z*=zscale;
	}
	UpdateVertexArray();
}

void Model::MultColor(float howmuch)
{
	int i;
	for(i=0; i<TriangleNum; i++){
		Triangles[i].r*=howmuch;
		Triangles[i].g*=howmuch;
		Triangles[i].b*=howmuch;
	}
	UpdateVertexArray();
}

void Model::ScaleNormals(float xscale,float yscale,float zscale)
{
	int i;
	for(i=0; i<vertexNum; i++){
		normals[i].x*=xscale;
		normals[i].y*=yscale;
		normals[i].z*=zscale;
	}
	UpdateVertexArray();
}

void Model::Translate(float xtrans,float ytrans,float ztrans)
{
	int i;
	for(i=0; i<vertexNum; i++){
		vertex[i].x+=xtrans;
		vertex[i].y+=ytrans;
		vertex[i].z+=ztrans;
	}
	UpdateVertexArray();
}

void Model::Rotate(float xang,float yang,float zang)
{
	int i;
	for(i=0; i<vertexNum; i++){
		vertex[i]=DoRotation(vertex[i],xang,yang,zang);
	}
	UpdateVertexArray();
}


void Model::CalculateNormals()
{
	int i;
	for(i=0;i<TriangleNum;i++){
		CrossProduct(vertex[Triangles[i].vertex[1]]-vertex[Triangles[i].vertex[0]],vertex[Triangles[i].vertex[2]]-vertex[Triangles[i].vertex[0]],&normals[i]);
		Normalise(&normals[i]);
	}
	UpdateVertexArray();
}

extern int nocolors;

/* ----------------------------------------------------------------------
   Whether this model is still safe to hand to GL.

   Every draw below does glDrawArrays(GL_TRIANGLES, 0, TriangleNum*3), and
   the array it reads holds max_textured_triangle triangles and no more.
   TriangleNum is a short sitting in front of that array, so anything that
   writes past the end of one model lands on the next one's counts - and a
   count past four hundred makes the next draw walk straight off the end of
   vArray and into whatever follows, drawn as triangles in that model's own
   colours.  That is a spray of geometry that does not go away, because the
   count stays broken for the life of the process.

   So: check it before drawing, say so once, and draw nothing rather than
   garbage.  A line in the log means something scribbled on a model and the
   checksum sweep in GameTick.cpp will say which.
   ---------------------------------------------------------------------- */
bool Model::DrawableNow(const char *where)
{
	if(TriangleNum>=0&&TriangleNum<=max_textured_triangle&&
	   vertexNum >=0&&vertexNum <=max_model_vertex)return true;

	static int said=0;

	if(said<8){

		said++;

		char note[200];

		snprintf(note,sizeof(note),
		         "model: refusing to draw from %s - TriangleNum %d, vertexNum %d"
		         " (limits %d/%d)\n",
		         where,(int)TriangleNum,(int)vertexNum,
		         max_textured_triangle,max_model_vertex);

		PlatformLogf("%s",note);

		fputs(note,stderr);

		fflush(stderr);

	}

	return false;
}

void Model::draw()
{
	if(!DrawableNow("draw()"))return;

	if(!nocolors){
#ifdef __EMSCRIPTEN__
	/* A colour array is the one thing the web's GL emulation will not take
	   alongside lighting - it builds the vertex colour from the material and
	   ignores the array - so the model is drawn unlit and keeps the colours
	   it is made of.  It loses the sun on its faces; a person you cannot
	   tell the colour of is worse. */
	const bool relight = BS_LightingEnabled() != 0;
	if(relight) glDisable(GL_LIGHTING);
#endif
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, 9*sizeof(GLfloat),&vArray[0]);
	glNormalPointer(GL_FLOAT, 9*sizeof(GLfloat),&vArray[3]);
	glColorPointer(3,GL_FLOAT, 9*sizeof(GLfloat),&vArray[6]);
	glDrawArrays(GL_TRIANGLES, 0, TriangleNum*3);
#ifdef __EMSCRIPTEN__
	if(relight) glEnable(GL_LIGHTING);
#endif
	}
	if(nocolors){
		glColor4f(0,0,0,1);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glVertexPointer(3, GL_FLOAT, 9*sizeof(GLfloat),&vArray[0]);
		glNormalPointer(GL_FLOAT, 9*sizeof(GLfloat),&vArray[3]);
		glDrawArrays(GL_TRIANGLES, 0, TriangleNum*3);
	}
}


void Model::draw(float r, float g, float b)
{
	if(!DrawableNow("draw(rgb)"))return;

	if(!nocolors)glColor4f(r,g,b,1);
	if(nocolors==1)glColor4f(0,0,0,1);
	if(nocolors==2)glColor4f(1,0,0,1);
	if(nocolors==3)glColor4f(0,0,1,1);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, 9*sizeof(GLfloat),&vArray[0]);
	glNormalPointer(GL_FLOAT, 9*sizeof(GLfloat),&vArray[3]);
	glDrawArrays(GL_TRIANGLES, 0, TriangleNum*3);
}

void Model::draw(float r, float g, float b, float o)
{
	if(!DrawableNow("draw(rgba)"))return;

	if(!nocolors)glColor4f(r,g,b,o);
	if(nocolors==1)glColor4f(0,0,0,1);
	if(nocolors==2)glColor4f(1,0,0,1);
	if(nocolors==3)glColor4f(1,1,1,1);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, 9*sizeof(GLfloat),&vArray[0]);
	glNormalPointer(GL_FLOAT, 9*sizeof(GLfloat),&vArray[3]);
	glDrawArrays(GL_TRIANGLES, 0, TriangleNum*3);
}

void Model::draw(float r, float g, float b, float x, float y, float z)
{
	if(!DrawableNow("draw(rgbxyz)"))return;

	if(!nocolors)glColor4f(r,g,b,1);
	if(nocolors==1)glColor4f(0,0,0,1);
	if(nocolors==2)glColor4f(1,0,0,1);
	if(nocolors==3)glColor4f(1,1,1,1);
	glNormal3f(x,y,z);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, 9*sizeof(GLfloat),&vArray[0]);
	glDrawArrays(GL_TRIANGLES, 0, TriangleNum*3);
}


int Model::LineCheck(XYZ p1,XYZ p2, XYZ *p)
{
  	int j;
	float distance;
	float olddistance=9999999.0;
	int intersecting=0;
	int firstintersecting=-1;
	XYZ point;
	if(sphere_line_intersection(p1.x,p1.y,p1.z,
								p2.x,p2.y,p2.z,
								boundingspherecenter.x,boundingspherecenter.y,boundingspherecenter.z,
								boundingsphereradius))
	for (j=0;j<TriangleNum;j++){
		intersecting=LineFacetd(p1,p2,vertex[Triangles[j].vertex[0]],vertex[Triangles[j].vertex[1]],vertex[Triangles[j].vertex[2]],normals[j],&point);
		if (intersecting == 0) continue;
		distance=(point.x-p1.x)*(point.x-p1.x)+(point.y-p1.y)*(point.y-p1.y)+(point.z-p1.z)*(point.z-p1.z);
		if((distance<olddistance||firstintersecting==-1)&&intersecting){olddistance=distance; firstintersecting=j; *p=point;}
	}
	return firstintersecting;
}

int Model::LineCheck2(XYZ p1,XYZ p2, XYZ *p, XYZ move, float rotate)
{
  	int j;
	float distance;
	float olddistance=9999999.0;
	int intersecting=0;
	int firstintersecting=-1;
	XYZ point;
	p1=p1-move;
	p2=p2-move;
	if(rotate)p1=DoRotation(p1,0,-rotate,0);
	if(rotate)p2=DoRotation(p2,0,-rotate,0);
	if(sphere_line_intersection(p1.x,p1.y,p1.z,
								p2.x,p2.y,p2.z,
								boundingspherecenter.x,boundingspherecenter.y,boundingspherecenter.z,
								boundingsphereradius))
	for (j=0;j<TriangleNum;j++){
		intersecting=LineFacetd(p1,p2,vertex[Triangles[j].vertex[0]],vertex[Triangles[j].vertex[1]],vertex[Triangles[j].vertex[2]],normals[j],&point);
		if (intersecting == 0) continue;
		distance=(point.x-p1.x)*(point.x-p1.x)+(point.y-p1.y)*(point.y-p1.y)+(point.z-p1.z)*(point.z-p1.z);
		if((distance<olddistance||firstintersecting==-1)&&intersecting){olddistance=distance; firstintersecting=j; *p=point;}
	}
	
	if(rotate)*p=DoRotation(*p,0,rotate,0);
	*p=*p+move;
	return firstintersecting;
}

int Model::LineCheck2(XYZ *p1,XYZ *p2, XYZ *p, XYZ *move, float *rotate)
{
  	int j;
	float distance;
	float olddistance=9999999.0;
	int intersecting=0;
	int firstintersecting=-1;
	XYZ point;
	*p1=*p1-*move;
	*p2=*p2-*move;
	if(*rotate)*p1=DoRotation(*p1,0,-*rotate,0);
	if(*rotate)*p2=DoRotation(*p2,0,-*rotate,0);
	if(sphere_line_intersection(p1->x,p1->y,p1->z,
								p2->x,p2->y,p2->z,
								boundingspherecenter.x,boundingspherecenter.y,boundingspherecenter.z,
								boundingsphereradius))
	for (j=0;j<TriangleNum;j++){
		intersecting=LineFacetd(p1,p2,&vertex[Triangles[j].vertex[0]],&vertex[Triangles[j].vertex[1]],&vertex[Triangles[j].vertex[2]],&normals[j],&point);
		if (intersecting == 0) continue;
		distance=(point.x-p1->x)*(point.x-p1->x)+(point.y-p1->y)*(point.y-p1->y)+(point.z-p1->z)*(point.z-p1->z);
		if((distance<olddistance||firstintersecting==-1)&&intersecting){olddistance=distance; firstintersecting=j; *p=point;}
	}
	
	if(*rotate)*p=DoRotation(*p,0,*rotate,0);
	*p=*p+*move;
	return firstintersecting;
}

int Model::LineCheck3(XYZ p1,XYZ p2, XYZ *p, XYZ move, float rotate, float *d)
{
  	int j;
	float distance;
	float olddistance=9999999.0;
	int intersecting=0;
	int firstintersecting=-1;
	XYZ point;
	p1=p1-move;
	p2=p2-move;
	p1=DoRotation(p1,0,-rotate,0);
	p2=DoRotation(p2,0,-rotate,0);
	if(sphere_line_intersection(p1.x,p1.y,p1.z,
								p2.x,p2.y,p2.z,
								boundingspherecenter.x,boundingspherecenter.y,boundingspherecenter.z,
								boundingsphereradius))
	for (j=0;j<TriangleNum;j++){
		intersecting=LineFacetd(p1,p2,vertex[Triangles[j].vertex[0]],vertex[Triangles[j].vertex[1]],vertex[Triangles[j].vertex[2]],normals[j],&point);
		if (intersecting == 0) continue;
		distance=(point.x-p1.x)*(point.x-p1.x)+(point.y-p1.y)*(point.y-p1.y)+(point.z-p1.z)*(point.z-p1.z);
		if((distance<olddistance||firstintersecting==-1)&&intersecting){olddistance=distance; firstintersecting=j; *p=point;}
	}
	*d=intersecting;
	*p=DoRotation(*p,0,rotate,0);
	*p=*p+move;
	return firstintersecting;
}
