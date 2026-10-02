#include "Game.h"	

extern int thirdperson;

extern double multiplier;

extern int nocolors;

extern int visions;

extern unsigned int gSourceID[100];

extern unsigned int gSampleSet[100];

extern Camera camera;

extern int pointeraim;      /* the crosshair below only exists when this is on */
extern float aimrotation;
extern float aimrotation2;

extern Sprites sprites;

extern float camerashake;

extern Fog fog;

extern float fogcolorr;

extern float fogcolorg;

extern float fogcolorb;

extern float sinefluct;

extern float sinefluctprog;

extern int environment;

extern Decals decals;

/*********************> DrawGLScene() <*****/

/* One of the main menu's smoky buttons: the two counter-rotating quads the
   original drew, twice over, for its New Game and Quit buttons.  y is the
   button's bottom edge in the menu's 640x480 space; a lit button (the mouse
   is over it) uses the flare texture instead of the smoke. */
static void DrawMenuButtonSmoke(float y, bool lit, GLuint smoketexture, GLuint flaretexture)
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glDepthMask(0);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0,640,0,480,-100,100);
	glMatrixMode(GL_MODELVIEW);
	for(int pass=0;pass<2;pass++){
		glPushMatrix();
			glLoadIdentity();
			glTranslatef(120-sinefluct*40,y+sinefluct*9,0);
			if(pass==0)glScalef(440+sinefluct*80,70-sinefluct*18,1);
			else glScalef(460+sinefluct*80,50-sinefluct*18,1);
			glTranslatef(.5,.5,0);
			glScalef(.5,.5,1);
			glRotatef((pass==0?1:-1)*sinefluctprog*50,0,0,1);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, lit ? flaretexture : smoketexture);
			glEnable(GL_BLEND);
			glColor4f(1,0,0,pass==0 ? .5+sinefluct/6 : .5-sinefluct/6);
			glBegin(GL_TRIANGLES);
				glTexCoord2f(1.0f, 1.0f); glVertex3f( 1, 1, 0.0f);
				glTexCoord2f(0.0f, 1.0f); glVertex3f(-1, 1, 0.0f);
				glTexCoord2f(1.0f, 0.0f); glVertex3f( 1,-1, 0.0f);
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-1,-1, 0.0f);
				glTexCoord2f(1.0f, 0.0f); glVertex3f( 1, -1, 0.0f);
				glTexCoord2f(0.0f, 1.0f); glVertex3f(-1, 1, 0.0f);
			glEnd();
		glPopMatrix();
	}
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glDepthMask(1);
}

int Game::DrawGLScene(void)									

{	

	//Main menu

	if(mainmenu==1){

		/* The game view gets its viewport from ReSizeGLScene() every frame; the
		   menu never set one, so after a resize or a fullscreen toggle it went
		   on drawing into the old window size, with its buttons no longer
		   where the mouse thought they were. */
		PlatformSetViewport();

		//Setup fast sine fluctuation

		sinefluct=sin(sinefluctprog);

		sinefluctprog+=multiplier*1.5;

		

		glLoadIdentity();

		glClearColor(0,0,0,1);

		glDisable(GL_CLIP_PLANE0);

		glDisable(GL_FOG);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		

		//"Black Shades"

		glDisable(GL_TEXTURE_2D);

		glDisable(GL_DEPTH_TEST);							// Disables Depth Testing

		glDisable(GL_CULL_FACE);

		glDisable(GL_LIGHTING);

		glBlendFunc(GL_SRC_ALPHA,GL_ONE);

		glDepthMask(0);

		glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix

		glPushMatrix();										// Store The Projection Matrix

		glLoadIdentity();									// Reset The Projection Matrix

		glOrtho(0,640,0,480,-100,100);						// Set Up An Ortho Screen

		glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix

		//Draw smoke

		glPushMatrix();										// Store The Modelview Matrix

			glLoadIdentity();								// Reset The Modelview Matrix

			glTranslatef(60+sinefluct*40,335-sinefluct*9,0);

			glScalef(500-sinefluct*80,70+sinefluct*18,1);

			glTranslatef(.5,.5,0);

			glScalef(.5,.5,1);

			glRotatef(sinefluctprog*50,0,0,1);

			glEnable(GL_TEXTURE_2D);

			glBindTexture(GL_TEXTURE_2D,  sprites.smoketextureptr);

			glEnable(GL_BLEND);

			glColor4f(1,1,1,.4+sinefluct/8);

			glBegin(GL_TRIANGLES);

				glTexCoord2f(1.0f, 1.0f); glVertex3f( 1, 1, 0.0f);

				glTexCoord2f(0.0f, 1.0f); glVertex3f(-1, 1, 0.0f);

				glTexCoord2f(1.0f, 0.0f); glVertex3f( 1,-1, 0.0f);

				glTexCoord2f(0.0f, 0.0f); glVertex3f(-1,-1, 0.0f);

				glTexCoord2f(1.0f, 0.0f); glVertex3f( 1, -1, 0.0f);

				glTexCoord2f(0.0f, 1.0f); glVertex3f(-1, 1, 0.0f);

			glEnd();

		glPopMatrix();

		glPushMatrix();										// Store The Modelview Matrix

			glLoadIdentity();								// Reset The Modelview Matrix

			glTranslatef(60+sinefluct*40,335-sinefluct*9,0);

			glScalef(530-sinefluct*80,50+sinefluct*18,1);

			glTranslatef(.5,.5,0);

			glScalef(.5,.5,1);

			glRotatef(-sinefluctprog*50,0,0,1);

			glEnable(GL_TEXTURE_2D);

			glBindTexture(GL_TEXTURE_2D,  sprites.smoketextureptr);

			glEnable(GL_BLEND);

			glColor4f(1,1,1,.4-sinefluct/8);

			glBegin(GL_TRIANGLES);

				glTexCoord2f(1.0f, 1.0f); glVertex3f( 1, 1, 0.0f);

				glTexCoord2f(0.0f, 1.0f); glVertex3f(-1, 1, 0.0f);

				glTexCoord2f(1.0f, 0.0f); glVertex3f( 1,-1, 0.0f);

				glTexCoord2f(0.0f, 0.0f); glVertex3f(-1,-1, 0.0f);

				glTexCoord2f(1.0f, 0.0f); glVertex3f( 1, -1, 0.0f);

				glTexCoord2f(0.0f, 1.0f); glVertex3f(-1, 1, 0.0f);

			glEnd();

		glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix

		glPopMatrix();										// Restore The Old Projection Matrix

		glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix

		glPopMatrix();										// Restore The Old Projection Matrix

		glDepthMask(1);

		

		//Text

		

		glEnable(GL_TEXTURE_2D);

		glColor4f(0,0,0,1);

		static char string[256]="";

		/* a revenge game shows its mode on the pause menu, as the Revenge
		   Mod's own build titled its menu (no capital M in any of these: at
		   this size the font's M blurs into an N) */
		if(gameinprogress&&revenge){

			sprintf (string, "Revenge");

			text.glPrint(125,175,string,1,2,640,480);

		}

		else{

			sprintf (string, "Black Shades");

			text.glPrint(100,175,string,1,2,640,480);

		}

		

		//Buttons, top to bottom: New Game / Resume Game, Revenge Game, Quit / End Game

		const int menubuttons[3]={1,3,2};

		for(int b=0;b<3;b++){

			const int id=menubuttons[b];

			const float y=MenuButtonY(id);

			if(y<0)continue;

			DrawMenuButtonSmoke(y,mouseoverbutton==id,sprites.smoketextureptr,flaretextureptr);

			if(id==1)sprintf (string, gameinprogress ? "Resume Game" : "New Game");

			if(id==2)sprintf (string, gameinprogress ? "End Game" : "Quit");

			if(id==3)sprintf (string, "Revenge Game");

			glEnable(GL_TEXTURE_2D);

			glColor4f(0,0,0,1);

			//centred on the button: glyphs are 10 units apart, at 1.5x

			text.glPrint((int)((340-strlen(string)*15/2.0f)/1.5f),(int)((y+20)/1.5f),string,1,1.5,640,480);

		}

		

		//High scores, one per mode

		glColor4f(.5+sinefluct/5,0,0,1);

		if(!beatgame)sprintf (string, "High Score:  %d", highscore);

		if(beatgame)sprintf (string, "High Score:  %d *COMPLETED* Please vote for Black Shades at iDevGames.com!", highscore);

		text.glPrint(0,18,string,1,.8,640,480);

		if(!revengebeatgame)sprintf (string, "Revenge High Score:  %d", revengehighscore);

		if(revengebeatgame)sprintf (string, "Revenge High Score:  %d *COMPLETED*", revengehighscore);

		text.glPrint(0,0,string,1,.8,640,480);

		

		//Mandatory udg text

		glColor4f(.3-sinefluct/20,.3-sinefluct/20,.3-sinefluct/20,1);

		sprintf (string, "uDevGame 2002 Entry - Visit iDevGames.com for more games!");

		text.glPrint(500,750,string,1,.6,640,480);

		

		
		//Mouse (draw)

		glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix

		glPushMatrix();										// Store The Projection Matrix

		glLoadIdentity();									// Reset The Projection Matrix

		glOrtho(0,screenwidth,0,screenheight,-100,100);						// Set Up An Ortho Screen

		glMatrixMode(GL_MODELVIEW);			

		glPushMatrix();										// Store The Modelview Matrix

		glDisable(GL_TEXTURE_2D);


		Point mouseloc;

		GetMouse(&mouseloc);

		mouseloc.v=screenheight-mouseloc.v;
		

		glColor4f(.1,0,0,1);

		

		float size=5;

		

		glBegin(GL_TRIANGLES);

			glVertex3f(mouseloc.h,mouseloc.v,0);

			glVertex3f(mouseloc.h+2*size,mouseloc.v-2*size,0);

			glVertex3f(mouseloc.h+.5*size,mouseloc.v-2*size,0);

		glEnd();

		

		glColor4f(1,0,0,1);

		

		glBegin(GL_QUADS);

			glVertex3f(olddrawmouse.h,olddrawmouse.v,0);

			glVertex3f(mouseloc.h,mouseloc.v,0);

			glVertex3f(mouseloc.h+2*size,mouseloc.v-2*size,0);

			glVertex3f(olddrawmouse.h+2*size,olddrawmouse.v-2*size,0);

			

			glVertex3f(olddrawmouse.h,olddrawmouse.v,0);

			glVertex3f(mouseloc.h,mouseloc.v,0);

			glVertex3f(mouseloc.h+.5*size,mouseloc.v-2*size,0);

			glVertex3f(olddrawmouse.h+.5*size,olddrawmouse.v-2*size,0);

			

			glVertex3f(olddrawmouse.h+2*size,olddrawmouse.v-2*size,0);

			glVertex3f(mouseloc.h+2*size,mouseloc.v-2*size,0);

			glVertex3f(mouseloc.h+.5*size,mouseloc.v-2*size,0);

			glVertex3f(olddrawmouse.h+.5*size,olddrawmouse.v-2*size,0);

		glEnd();

		/* Both stacks, and in the right mode for each.  This pushed the
		   projection and then popped the modelview, so every frame the
		   menu drew leaked a projection level and underflowed a
		   modelview one.  A desktop has 32 projection slots and hides
		   it; opengx has four, so on a Wii the stack was full within
		   four frames and every later pop handed the world back a
		   leftover HUD matrix. */
		glPopMatrix();										// Restore The Old Modelview Matrix

		glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix

		glPopMatrix();										// Restore The Old Projection Matrix

		glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix

		
		olddrawmouse=mouseloc;
		

		//Flash

		if(flashamount>0){

			if(flashamount>1)flashamount=1;

			flashamount-=multiplier;

			if(flashamount<0)flashamount=0;

			glDisable(GL_DEPTH_TEST);							// Disables Depth Testing

			glDisable(GL_CULL_FACE);

			glDisable(GL_LIGHTING);

			glDepthMask(0);

			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix

			glPushMatrix();										// Store The Projection Matrix

			glLoadIdentity();									// Reset The Projection Matrix

			glOrtho(0,screenwidth,0,screenheight,-100,100);						// Set Up An Ortho Screen

			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix

			glPushMatrix();										// Store The Modelview Matrix

			glLoadIdentity();								// Reset The Modelview Matrix

			glScalef(screenwidth,screenheight,1);

			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

			glEnable(GL_BLEND);

			glColor4f(flashr,flashg,flashb,flashamount);

			glBegin(GL_QUADS);

				glVertex3f(0,		0, 	 0.0f);

				glVertex3f(256,	0, 	 0.0f);

				glVertex3f(256,	256, 0.0f);

				glVertex3f(0, 	256, 0.0f);

			glEnd();

			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix

			glPopMatrix();										// Restore The Old Projection Matrix

			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix

			glPopMatrix();										// Restore The Old Projection Matrix

			glEnable(GL_DEPTH_TEST);							// Enables Depth Testing

			glEnable(GL_CULL_FACE);

			glDisable(GL_BLEND);

			glDepthMask(1);

		}	

	}

	//If in-game

	if(mainmenu!=1){

		//If flashing to fix menu bug, go back to menu after a frame

		if(mainmenu==2)mainmenu=1;

		glLoadIdentity();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);

		glDisable(GL_CLIP_PLANE0);

		//Visions

		sinefluct=sin(sinefluctprog);

		sinefluctprog+=multiplier*3;

		if(visions==0){

			//environment light

			/* each environment's view distance (the fog ends at 80% of it), times
			   config.txt's View distance */

			if(environment==sunny_environment){

				viewdistance=2000*viewscale;	

				fogcolorr=.5;

				fogcolorg=.5;

				fogcolorb=.8;

				fog.SetFog(fogcolorr,fogcolorg,fogcolorb,0,viewdistance*.8,.1);

			}

			if(environment==foggy_environment){

				viewdistance=500*viewscale;	

				fogcolorr=.5;

				fogcolorg=.5;

				fogcolorb=.5;

				fog.SetFog(fogcolorr,fogcolorg,fogcolorb,0,viewdistance*.8,.2);

			}

			if(environment==night_environment){

				viewdistance=500*viewscale;	

				fogcolorr=.15;

				fogcolorg=.15;

				fogcolorb=.3;

				fog.SetFog(fogcolorr,fogcolorg,fogcolorb,0,viewdistance*.8,.2);

			}

			if(environment==snowy_environment){

				viewdistance=800*viewscale;	

				fogcolorr=.5;

				fogcolorg=.5;

				fogcolorb=.5;

				fog.SetFog(fogcolorr,fogcolorg,fogcolorb,0,viewdistance*.8,.2);

			}

			if(environment==rainy_environment){

				viewdistance=700*viewscale;	

				fogcolorr=.3;

				fogcolorg=.3;

				fogcolorb=.3;

				fog.SetFog(fogcolorr,fogcolorg,fogcolorb,0,viewdistance*.8,.2);

			}

			if(environment==firey_environment){

				viewdistance=600*viewscale;	

				fogcolorr=.3;

				fogcolorg=.1;

				fogcolorb=0;

				fog.SetFog(fogcolorr,fogcolorg,fogcolorb,0,viewdistance*.8,.2);

			}

			glClearColor(fogcolorr,fogcolorg,fogcolorb,1);

			

			if(environment==sunny_environment){

				GLfloat LightAmbient[]=	{ fogcolorr/4, fogcolorg/4, fogcolorb/4, 1.0f};

				GLfloat LightDiffuse[]=	{ fogcolorr*1.6f, fogcolorg*1.6f, fogcolorr*1.6f, 1.0f };

				glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);		

				glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);	

			}

			if(environment!=sunny_environment){

				GLfloat LightAmbient[]=	{ fogcolorr*.8f, fogcolorg*.8f, fogcolorb*.8f, 1.0f};

				GLfloat LightDiffuse[]=	{ fogcolorr*.8f, fogcolorg*.8f, fogcolorr*.8f, 1.0f };

				glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);		

				glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);	

			}			

				

			glEnable(GL_LIGHT0);	

			

			//Change fov if zooming with scope

			/* FOV from config.txt; the scope zooms to a quarter of it (Black Shades
			   Enhanced - the original's fixed 90 and 10) */
			if(!zoom)ReSizeGLScene(fov,.1);

			if(zoom)ReSizeGLScene(fov/4,.6);

			nocolors=0;

		}

		

		if(visions==1){

			//light

			GLfloat LightAmbient[]=		{ 0, 0, 0, 1.0f};

			GLfloat LightDiffuse[]=		{ .1f+sinefluct/5, 0, 0, 1.0f };

			

			glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);		

			glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);		

			glEnable(GL_LIGHT0);	

			

			fogcolorr=(sinefluct/4+.5);

			fogcolorg=0;

			fogcolorb=0;

			fog.SetFog(fogcolorr,fogcolorg,fogcolorb,0,viewdistance*.8*.5*(sinefluct/4+.3),sinefluct/3+.7);

			glClearColor(fogcolorr,fogcolorg,fogcolorb,1);

			ReSizeGLScene(120-sinefluct*20,.3);

			glRotatef(sinefluct*10,0,0,.1);

			nocolors=1;

			//Pitch higher if moving for effect

			if(person[0].currentanimation==idleanim)alSourcef(gSourceID[visionsound], AL_PITCH, 1);

			if(person[0].currentanimation!=idleanim)alSourcef(gSourceID[visionsound], AL_PITCH, 2);

		}

		

		//Camera

		//no depth-of-field blur through the scope (Black Shades Enhanced)
		float bluramount=zoom?0:.1*blurness;

		blur=1-blur;

		//Set rotation/position

		if(thirdperson)glTranslatef(camera.targetoffset.x,camera.targetoffset.y,camera.targetoffset.z);

		if(thirdperson!=2&&(person[0].skeleton.free!=1||thirdperson!=0)){

			glRotatef(camera.visrotation2+-bluramount/2+(float)blur*bluramount,1,0,0);

			glRotatef(camera.visrotation+-bluramount/2+(float)blur*bluramount,0,1,0);

		}

		if(thirdperson==0&&person[0].skeleton.free==1){

			glRotatef(person[0].skeleton.joints[(person[0].skeleton.jointlabels[head])].rotate3,0,1,0);

			glRotatef(180-(person[0].skeleton.joints[(person[0].skeleton.jointlabels[head])].rotate2+90),0,0,1);

			glRotatef(person[0].skeleton.joints[(person[0].skeleton.jointlabels[head])].rotate1+90,0,1,0);

		}

		if(thirdperson==2){

			glRotatef(oldrot2+-bluramount/2+(float)blur*bluramount,1,0,0);

			glRotatef(oldrot+-bluramount/2+(float)blur*bluramount,0,1,0);

		}

		//Shake camera if grenade explosion

		if(camerashake>0){

			if(!(person[0].aiming<1||person[0].whichgun==grenade||thirdperson)){

				camerashake=0;

			}

			glTranslatef((float)(Random()%100)/100*camerashake,(float)(Random()%100)/100*camerashake,(float)(Random()%100)/100*camerashake);

		}

		camera.Apply();

		

		glPushMatrix();

			glClipPlane(GL_CLIP_PLANE0, eqn);

			glDisable(GL_CLIP_PLANE0);

		glPopMatrix();

		

		frustum.GetFrustum();



		GLfloat LightPosition[]=	{ -.5, 1, -.8, 0.0f };

		glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);

		

		glDisable(GL_TEXTURE_2D);

		glEnable(GL_FOG);

		glEnable(GL_COLOR_MATERIAL);

		glEnable(GL_CULL_FACE);

		glDepthMask(1);

				

		//Draw street

		glPushMatrix();

			glDepthMask(0);

			glDisable(GL_DEPTH_TEST);

			glEnable(GL_LIGHTING);

			glTranslatef(camera.position.x,0,camera.position.z);

			glScalef(viewdistance*5/100,1,viewdistance*5/100);

			if(visions==0)street.draw(.22,.22,.22);

			if(visions==1)street.draw(0,0,0);

			glEnable(GL_DEPTH_TEST);

			glDepthMask(1);

		glPopMatrix();

		

		if(visions==0)glEnable(GL_LIGHTING);

		if(visions==1)glDisable(GL_LIGHTING);

		

		//Draw blocks

		glEnable(GL_BLEND);

		XYZ move2;

		XYZ move;

		int beginx,endx;

		int beginz,endz;

		int beginoccludex,endoccludex;

		int beginoccludez,endoccludez;

		int distsquared;

		

		//Only nearby blocks

		beginx=(camera.position.x-viewdistance+block_spacing/2)/block_spacing-2;

		if(beginx<0)beginx=0;

		beginz=(camera.position.z-viewdistance+block_spacing/2)/block_spacing-2;

		if(beginz<0)beginz=0;

		

		endx=(camera.position.x+viewdistance+block_spacing/2)/block_spacing+2;

		if(endx>num_blocks-1)endx=num_blocks-1;

		endz=(camera.position.z+viewdistance+block_spacing/2)/block_spacing+2;

		if(endz>num_blocks-1)endz=num_blocks-1;

		

		bool draw;

		int numfail;

		int whichtri;

		XYZ collpoint;

		

		for(int i=beginx;i<=endx;i++){

			for(int j=beginz;j<=endz;j++){

				drawn[i][j]=1;

			}

		}

		

		if(beginx<endx&&beginz<endz)

		for(int i=beginx;i<=endx;i++){

			for(int j=beginz;j<=endz;j++){

				draw=1;

				//Only draw if visible

				distsquared=((i)*block_spacing-camera.position.x)*((i)*block_spacing-camera.position.x)+((j)*block_spacing-camera.position.z)*((j)*block_spacing-camera.position.z);

				if(distsquared>(viewdistance*viewdistance+block_spacing*block_spacing	))draw=0;

				if(draw&&citytype[i][j]!=3&&!frustum.CubeInFrustum((i)*block_spacing,0,(j)*block_spacing,block_spacing))draw=0;

				if(draw&&citytype[i][j]!=3&&!frustum.SphereInFrustum(blocks[citytype[i][j]].boundingspherecenter.x+(i)*block_spacing,blocks[citytype[i][j]].boundingspherecenter.y,blocks[citytype[i][j]].boundingspherecenter.z+(j)*block_spacing,blocks[citytype[i][j]].boundingsphereradius))draw=0;

				if(draw){

					glPushMatrix();

						glTranslatef(i*block_spacing,0,j*block_spacing);

						glRotatef(cityrotation[i][j]*90,0,1,0);

						blocks[citytype[i][j]].draw();

					glPopMatrix();

				}

				if(!draw){

					move.y=0;

					move.x=i*block_spacing;

					move.z=j*block_spacing;

					if(findDistancefast(move,camera.position)<300000)drawn[i][j]=0;

				}

			}

		}

		

		//Decals

		decals.draw();

		

		//Occluding blocks

		beginx=(camera.position.x+block_spacing/2)/block_spacing-2;

		if(beginx<0)beginx=0;

		beginz=(camera.position.z+block_spacing/2)/block_spacing-2;

		if(beginz<0)beginz=0;

		

		endx=(camera.position.x+block_spacing/2)/block_spacing+2;

		if(endx>num_blocks-1)endx=num_blocks-1;

		endz=(camera.position.z+block_spacing/2)/block_spacing+2;

		if(endz>num_blocks-1)endz=num_blocks-1;

		

		float M[16];

		XYZ drawpoint;

		float size=20;

		

		//Draw people

		if(visions==1)fog.SetFog(fogcolorr,fogcolorg,fogcolorb,0,viewdistance*.8*.5*(-sinefluct/4+.3),-sinefluct/3+.7);

		glColor4f(1,1,1,1);

		glEnable(GL_COLOR_MATERIAL);

		glEnable(GL_BLEND);

		/* People are drawn out to 1000 units, times the View distance.  Only
		   blocks whose middle is within ~316 units of the camera can hide
		   anyone (the findDistancefast(...)<100000 test below), and those are
		   the camera's own block and its neighbours - so only those are
		   looped over, rather than every visible block, which would grow
		   with the square of the view distance for every person drawn. */
		const float peopledrawscale=viewscale>1?viewscale:1;

		const float peopledrawdistance=1000000*peopledrawscale*peopledrawscale;

		/* ...and in full out to ~316 units (~1760 through the scope), times the
		   View distance too; past that they are billboards */
		const float billboarddistance=(100000+zoom*3000000)*peopledrawscale*peopledrawscale;

		const int cameraoccludex=CityBlock(camera.position.x), cameraoccludez=CityBlock(camera.position.z);

		const int occludebeginx=cameraoccludex-1>beginx?cameraoccludex-1:beginx;

		const int occludeendx=cameraoccludex+1<endx?cameraoccludex+1:endx;

		const int occludebeginz=cameraoccludez-1>beginz?cameraoccludez-1:beginz;

		const int occludeendz=cameraoccludez+1<endz?cameraoccludez+1:endz;

		for(int i=0;i<numpeople;i++){

		

			draw=1;

			if(person[i].skeleton.free<1){

				if(person[i].whichblockx>=0&&person[i].whichblockx<num_blocks&&person[i].whichblocky>=0&&person[i].whichblocky<num_blocks){

					if(!drawn[person[i].whichblockx][person[i].whichblocky])draw=0;

				}else draw=0;

				if(draw)

				if(!frustum.CubeInFrustum(person[i].playercoords.x,person[i].playercoords.y,person[i].playercoords.z,5))draw=0;

				if(draw)

				if(findDistancefast(person[i].playercoords,camera.position)>peopledrawdistance)draw=0;

				if(draw)

				for(int j=occludebeginx;j<=occludeendx;j++){

					for(int k=occludebeginz;k<=occludeendz;k++){

						if(draw){

							move.y=0;

							move.x=j*block_spacing;

							move.z=k*block_spacing;

							if(findDistancefast(move,camera.position)<100000){

								whichtri=blockocclude.LineCheck2(camera.position,person[i].playercoords,&collpoint,move,0);

								if(whichtri!=-1)draw=0;

							}

						}

					}

				}

				if(draw){

					move.y=0;

					move.x=person[i].whichblockx*block_spacing;

					move.z=person[i].whichblocky*block_spacing;

					whichtri=blockocclude.LineCheck2(camera.position,person[i].playercoords,&collpoint,move,0);

					if(whichtri!=-1)draw=0;

				}

				if(i==0)draw=1;

			}

			if(person[i].skeleton.free==1){

				if(draw)

				if(!person[i].skeleton.broken&&!frustum.CubeInFrustum(person[i].averageloc.x,person[i].averageloc.y,person[i].averageloc.z,5))draw=0;

				if(draw)

				if(findDistancefast(person[i].averageloc,camera.position)>peopledrawdistance)draw=0;

				if(draw)

				if(person[i].skeleton.joints[0].position.y<-2)draw=0;

				for(int j=occludebeginx;j<=occludeendx;j++){

					for(int k=occludebeginz;k<=occludeendz;k++){

						if(draw){

							move.y=0;

							move.x=j*block_spacing;

							move.z=k*block_spacing;

							if(findDistancefast(move,camera.position)<100000){

								whichtri=blockocclude.LineCheck2(camera.position,person[i].averageloc,&collpoint,move,0);

								if(whichtri!=-1)draw=0;

							}

						}

					}

				}

				if(draw){

					move.y=0;

					move.x=person[i].whichblockx*block_spacing;

					move.z=person[i].whichblocky*block_spacing;

					whichtri=blockocclude.LineCheck2(camera.position,person[i].averageloc,&collpoint,move,0);

					if(whichtri!=-1)draw=0;

				}

				if(i==0)draw=1;

			}

			if(draw&&person[i].existing==1){

				if((findDistancefast(person[i].playercoords,camera.position)<billboarddistance&&person[i].skeleton.free<1)||(findDistancefast(person[i].averageloc,camera.position)<billboarddistance&&person[i].skeleton.free>=1)){

					glPushMatrix();

						if(person[i].skeleton.free==0){

							glTranslatef(person[i].playercoords.x,person[i].playercoords.y,person[i].playercoords.z);

							glRotatef(person[i].playerrotation,0,1,0);					

							if(i!=0||visions==0)person[i].DoAnimations(i);

							if(i==0&&visions==1)person[i].DoAnimationslite(i);

						}

						if(visions==1)nocolors=1;

						if(visions==1&&person[i].type==eviltype)nocolors=2;

						if(visions==1&&person[i].type==viptype)nocolors=3;

						if(!(visions==1&&i==0)&&!(zoom==1&&i==0))person[i].DrawSkeleton(i);

					glPopMatrix();

				}

				else{

					glPushMatrix();

						if(person[i].skeleton.free<1)person[i].DoAnimationslite(i);

						glColor4f(1,1,1,1);

						glEnable(GL_BLEND);

						glDisable(GL_CULL_FACE);

						glEnable(GL_TEXTURE_2D);

						glDisable(GL_LIGHTING);

						glDepthMask(0);

						glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

						if(person[i].skeleton.free<1){

							glBindTexture(GL_TEXTURE_2D, personspritetextureptr);

							glTranslatef(person[i].playercoords.x,person[i].playercoords.y+size/2*.3,person[i].playercoords.z);

						}

						if(person[i].skeleton.free==1){

							glBindTexture(GL_TEXTURE_2D, deadpersonspritetextureptr);

							glTranslatef(person[i].averageloc.x,person[i].averageloc.y+size/2*.3,person[i].averageloc.z);

						}

						glGetFloatv(GL_MODELVIEW_MATRIX,M);

						drawpoint.x=M[12];

						drawpoint.y=M[13];

						drawpoint.z=M[14];

						glLoadIdentity();

						glTranslatef(drawpoint.x, drawpoint.y, drawpoint.z);

						glBegin(GL_TRIANGLES);

							glTexCoord2f(1.0f, 1.0f); glVertex3f( .3f*size, .3f*size, 0.0f);

							glTexCoord2f(0.0f, 1.0f); glVertex3f(-.3f*size, .3f*size, 0.0f);

							glTexCoord2f(1.0f, 0.0f); glVertex3f( .3f*size,-.3f*size, 0.0f);

							glTexCoord2f(0.0f, 0.0f); glVertex3f(-.3f*size,-.3f*size, 0.0f);

							glTexCoord2f(1.0f, 0.0f); glVertex3f( .3f*size, -.3f*size, 0.0f);

							glTexCoord2f(0.0f, 1.0f); glVertex3f(-.3f*size, .3f*size, 0.0f);

						glEnd();

					glPopMatrix();

					glDepthMask(1);

					glDisable(GL_TEXTURE_2D);

					glEnable(GL_CULL_FACE);

					if(visions!=1)glEnable(GL_LIGHTING);

				}

			}

			if(person[i].skeleton.free<1&&!draw)person[i].DoAnimationslite(i);

						

			if(!person[i].existing)

				if(!draw||findDistancefast(person[i].playercoords,camera.position)>10000){person[i].existing=1;}

		}

		glDisable(GL_COLOR_MATERIAL);

		glDisable(GL_BLEND);

		

		//Attacker psychicness 

		for(int i=0;i<numpeople;i++){

			if(person[i].killtarget>-1&&person[i].killtargetvisible&&person[i].skeleton.free==0&&person[person[i].killtarget].skeleton.free==0){

				sprites.MakeSprite(bulletinstant, (shotdelayamount/difficulty-person[i].shotdelay)/shotdelayamount/difficulty/2, 1, person[i].shotdelay/shotdelayamount/difficulty, person[i].shotdelay/shotdelayamount/difficulty, DoRotation(person[i].skeleton.joints[person[i].skeleton.jointlabels[lefthand]].position,0,person[i].playerrotation,0)+person[i].playercoords, person[person[i].killtarget].skeleton.joints[person[person[i].killtarget].skeleton.jointlabels[abdomen]].position+person[person[i].killtarget].playercoords, person[i].shotdelay*2);

			}

			if(person[i].killtarget>-1&&person[i].killtargetvisible&&person[i].skeleton.free==0&&person[person[i].killtarget].skeleton.free!=0){

				sprites.MakeSprite(bulletinstant, (shotdelayamount/difficulty-person[i].shotdelay)/shotdelayamount/difficulty/2, 1, person[i].shotdelay/shotdelayamount/difficulty, person[i].shotdelay/shotdelayamount/difficulty, DoRotation(person[i].skeleton.joints[person[i].skeleton.jointlabels[lefthand]].position,0,person[i].playerrotation,0)+person[i].playercoords, person[person[i].killtarget].skeleton.joints[person[person[i].killtarget].skeleton.jointlabels[abdomen]].position, person[i].shotdelay*2);

			}

		}

		

		//Sprites

		glEnable(GL_CLIP_PLANE0);

		sprites.draw();

		

		glDisable(GL_CLIP_PLANE0);



		glDisable(GL_FOG);

		//Zoom

		glAlphaFunc(GL_GREATER, 0.01);

		

		if(zoom){

			glDisable(GL_DEPTH_TEST);							// Disables Depth Testing

			glDisable(GL_CULL_FACE);

			glDisable(GL_LIGHTING);

			glDepthMask(0);

			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix

			glPushMatrix();										// Store The Projection Matrix

			glLoadIdentity();									// Reset The Projection Matrix

			glOrtho(0,screenwidth,0,screenheight,-100,100);						// Set Up An Ortho Screen

			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix

			glPushMatrix();										// Store The Modelview Matrix

			glLoadIdentity();								// Reset The Modelview Matrix

			glScalef(screenwidth,screenheight,1);

			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

			glEnable(GL_BLEND);

			glEnable(GL_TEXTURE_2D);

			glColor4f(.5,.5,.5,1);

			glBindTexture(GL_TEXTURE_2D, scopetextureptr);

			glBegin(GL_QUADS);

				glTexCoord2f(0,0);

				glVertex3f(0,		0, 	 0.0f);

				glTexCoord2f(1,0);

				glVertex3f(1,	0, 	 0.0f);

				glTexCoord2f(1,1);

				glVertex3f(1,	1, 0.0f);

				glTexCoord2f(0,1);

				glVertex3f(0, 	1, 0.0f);

			glEnd();

			glDisable(GL_TEXTURE_2D);

			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix

			glPopMatrix();										// Restore The Old Projection Matrix

			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix

			glPopMatrix();										// Restore The Old Projection Matrix

			glEnable(GL_DEPTH_TEST);							// Enables Depth Testing

			glEnable(GL_CULL_FACE);

			glDisable(GL_BLEND);

			glDepthMask(1);

		}




		//Flash

		if(flashamount>0){

			if(flashamount>1)flashamount=1;

			flashamount-=multiplier;

			if(flashamount<0)flashamount=0;

			glDisable(GL_DEPTH_TEST);							// Disables Depth Testing

			glDisable(GL_CULL_FACE);

			glDisable(GL_LIGHTING);

			glDepthMask(0);

			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix

			glPushMatrix();										// Store The Projection Matrix

			glLoadIdentity();									// Reset The Projection Matrix

			glOrtho(0,screenwidth,0,screenheight,-100,100);						// Set Up An Ortho Screen

			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix

			glPushMatrix();										// Store The Modelview Matrix

			glLoadIdentity();								// Reset The Modelview Matrix

			glScalef(screenwidth,screenheight,1);

			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

			glEnable(GL_BLEND);

			glColor4f(flashr,flashg,flashb,flashamount);

			glBegin(GL_QUADS);

				glVertex3f(0,		0, 	 0.0f);

				glVertex3f(256,	0, 	 0.0f);

				glVertex3f(256,	256, 0.0f);

				glVertex3f(0, 	256, 0.0f);

			glEnd();

			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix

			glPopMatrix();										// Restore The Old Projection Matrix

			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix

			glPopMatrix();										// Restore The Old Projection Matrix

			glEnable(GL_DEPTH_TEST);							// Enables Depth Testing

			glEnable(GL_CULL_FACE);

			glDisable(GL_BLEND);

			glDepthMask(1);

		}	

		if(person[0].skeleton.free>0&&thirdperson!=2){

			glDisable(GL_DEPTH_TEST);							// Disables Depth Testing

			glDisable(GL_CULL_FACE);

			glDisable(GL_LIGHTING);

			glDepthMask(0);

			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix

			glPushMatrix();										// Store The Projection Matrix

			glLoadIdentity();									// Reset The Projection Matrix

			glOrtho(0,screenwidth,0,screenheight,-100,100);						// Set Up An Ortho Screen

			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix

			glPushMatrix();										// Store The Modelview Matrix

			glLoadIdentity();								// Reset The Modelview Matrix

			glScalef(screenwidth,screenheight,1);

			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

			glEnable(GL_BLEND);

			glColor4f(0,0,0,1-person[0].longdead);

			glBegin(GL_QUADS);

				glVertex3f(0,		0, 	 0.0f);

				glVertex3f(256,	0, 	 0.0f);

				glVertex3f(256,	256, 0.0f);

				glVertex3f(0, 	256, 0.0f);

			glEnd();

			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix

			glPopMatrix();										// Restore The Old Projection Matrix

			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix

			glPopMatrix();										// Restore The Old Projection Matrix

			glEnable(GL_DEPTH_TEST);							// Enables Depth Testing

			glEnable(GL_CULL_FACE);

			glDisable(GL_BLEND);

			glDepthMask(1);

		}	

		

		//Text

		glEnable(GL_TEXTURE_2D);

		glColor4f(1,1,1,1);

		static char string[256]="";

		if(!debug){

		if(person[0].whichgun==nogun)sprintf (string, "UNARMED");

		if(person[0].whichgun==knife)sprintf (string, "KNIFE");

		if(person[0].whichgun==assaultrifle)sprintf (string, "ASSAULT RIFLE");

		if(person[0].whichgun==shotgun)sprintf (string, "SHOTGUN");

		if(person[0].whichgun==sniperrifle)sprintf (string, "SNIPER RIFLE");

		if(person[0].whichgun==grenade)sprintf (string, "HAND GRENADE");

		if(person[0].whichgun==handgun1)sprintf (string, "MAGNUM");

		if(person[0].whichgun==handgun2)sprintf (string, "HANDGUN");

		text.glPrint(10,90,string,1,.8,640,480);

		if(person[0].whichgun!=nogun&&person[0].whichgun!=knife&&person[0].whichgun!=grenade){

			glColor4f(0,1,0,1);

			sprintf (string, "Magazines");

			text.glPrint(10,30,string,0,.8,640,480);

			glColor4f(1,1,1,1);

			sprintf (string, "0%d",(int)person[0].reloads[person[0].whichgun]);

			text.glPrint(10,10,string,1,1,640,480);

			if(person[0].ammo>=0){

				glColor4f(0,1,0,1);

				sprintf (string, "Ammo");

				text.glPrint(10,65,string,0,.8,640,480);

				glColor4f(1,1,1,1);

				if(person[0].ammo>=10)sprintf (string, "%d",(int)person[0].ammo);

				if(person[0].ammo<10)sprintf (string, "0%d",(int)person[0].ammo);

				text.glPrint(10,40,string,1,1,640,480);

			}

		}

		if(person[0].whichgun==grenade){

			sprintf (string, "Grenades Left:  %d",(int)person[0].reloads[person[0].whichgun]+1);

			text.glPrint(10,20,string,0,.8,640,480);

		}

		glColor4f(1,1,0,1);

		sprintf (string, "Score");

		text.glPrint(725,575,string,0,.8,640,480);

		glColor4f(1,1,1,1);

		sprintf (string, "%d", score);

		text.glPrint(580,440,string,1,1,640,480);

		

		glColor4f(1,1,0,1);

		sprintf (string, "Time Remaining");

		text.glPrint(50,575,string,0,.8,640,480);

		glColor4f(1,1,1,1);

		if((int)timeremaining%60>=10)sprintf (string, "%d:%d", (int)(timeremaining/60),(int)timeremaining%60);

		if((int)timeremaining%60<10)sprintf (string, "%d:0%d", (int)(timeremaining/60),(int)timeremaining%60);

		text.glPrint(72,440,string,1,1,640,480);   //was 72.5, which the int parameter truncated to 72 anyway

		}

		

		if(debug){

		/* glyphs are 16 units tall, so lines 10 apart overprinted each other */
		sprintf (string, "The framespersecond is %d out of maximum %d.",(int)framespersecond+1,maxfps);

		text.glPrint(10,36,string,0,.8,screenwidth,screenheight);

		if(enemystate==0)sprintf (string, "Enemies are in random assassination mode.");

		if(enemystate==1)sprintf (string, "Enemies are in passive mode.");

		if(enemystate==2)sprintf (string, "Enemies are in DIE!!!! mode.");

		text.glPrint(10,20,string,0,.8,screenwidth,screenheight);

		sprintf (string, "You have pointlessly shot or beaten %d unarmed civilians.",civkills);

		text.glPrint(10,60,string,0,.8,screenwidth,screenheight);

		sprintf (string, "You have incapacitated %d assassins.",goodkills);

		text.glPrint(10,76,string,0,.8,screenwidth,screenheight);

		if(!revenge){

		sprintf (string, "You have allowed %d successful murders.",badkills);

		text.glPrint(10,92,string,0,.8,screenwidth,screenheight);

		}

		}

		/*
		sprintf (string, "");

		text.glPrint(10,80,string,0,.8,screenwidth,screenheight);
		*/	

	}

		/* The crosshair, which only exists when a pointer is doing the
		   aiming: the gun is pointed at this spot, so something has to mark
		   it.  Not while scoped, which has a sight of its own, and not in
		   the menu, which has its own cursor.  It is drawn whether or not
		   the gun happens to be raised - the pointer is still what you pick
		   things up and disarm people with. */
		if(pointeraim&&!zoom&&!mainmenu&&person[0].health>0){

			float px,py;

			/* drawn wherever the pointer last was, so it holds still
			   through a blink instead of disappearing */
			PointerPosition(&px,&py);

			{



				const float x=(px+1)*screenwidth /2;
				const float y=screenheight-(py+1)*screenheight/2;
				const float r=10,gap=4;

				glDisable(GL_DEPTH_TEST);
				glDisable(GL_CULL_FACE);
				glDisable(GL_LIGHTING);
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_ALPHA_TEST);
				glDisable(GL_FOG);
				glDepthMask(0);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
				glMatrixMode(GL_PROJECTION);
				glPushMatrix();
				glLoadIdentity();
				glOrtho(0,screenwidth,0,screenheight,-100,100);
				glMatrixMode(GL_MODELVIEW);
				glPushMatrix();
				glLoadIdentity();

				/* four ticks around a gap, so whatever is under it stays
				   visible - the thing you are about to shoot */
				glColor4f(1,1,1,.85);
				glLineWidth(2);
				glBegin(GL_LINES);
					glVertex3f(x-r,y,0);   glVertex3f(x-gap,y,0);
					glVertex3f(x+gap,y,0); glVertex3f(x+r,y,0);
					glVertex3f(x,y-r,0);   glVertex3f(x,y-gap,0);
					glVertex3f(x,y+gap,0); glVertex3f(x,y+r,0);
				glEnd();
				glColor4f(1,1,1,1);
				glLineWidth(1);

				glMatrixMode(GL_PROJECTION);
				glPopMatrix();
				glMatrixMode(GL_MODELVIEW);
				glPopMatrix();
				glDepthMask(1);
				glEnable(GL_DEPTH_TEST);
				glEnable(GL_CULL_FACE);
				glEnable(GL_TEXTURE_2D);
				glEnable(GL_ALPHA_TEST);

			}

		}

	return 1;
}

