#ifndef _GAME_H_
#define _GAME_H_

#include "GLHeaders.h"
#include <stdlib.h>			
#include <stdio.h>			
#include <string.h>
#include <ctype.h>
#ifdef OS9 
#include <Sound.h>
#include <Resources.h>		
#endif
#include <cstdarg>
#ifdef OS9 
#include <glm.h>
#include <TextUtils.h>
#endif
#include <SDL.h>
#include "Audio.h"
#include "Timer.h"	
#ifdef OS9 
#include "AGL_DSp.h"	
#endif
#include "MacInput.h"
#include "Quaternions.h"
#include "Camera.h"
#include "Skeleton.h"
#include "Files.h"
#include "Models.h"
#include "Text.h"
#include "Fog.h"
#include "Frustum.h"
#include "Sprites.h"
#include "Person.h"
#include "Decals.h"

#define num_blocks 100
#define block_spacing 360

/* The crowd.  The original kept 90 people in the 3x3 blocks around you,
   bunched towards your own block, and spawned and recycled them anywhere in
   those nine blocks - in plain view.  Now people live in every block out to
   a block or so past where anyone can be seen (see the spawner in
   GameTick.cpp), block_people of them to a block - the original's 90 over
   nine blocks - times config.txt's Population density.  The streets are
   full when a mission starts, and after that people only spawn, and are
   taken away, out of sight at the edge of that square. */
#define block_people 10
#define max_spawn_radius (num_blocks-2)        /* 98: from anywhere in the city, the square covers all of it */

/* The city block a world x or z coordinate falls in.  People, ragdolls and
   grenades can end up outside the 100x100 city (a body blown clear of it, a
   grenade thrown off the edge), and the block number indexes citytype[],
   cityrotation[] and the block models, so it is clamped to the city (and a
   NaN coordinate counts as block 0). */
static inline int CityBlock(float coord)
{
	const float block = (coord + block_spacing / 2) / block_spacing;
	if (!(block >= 0)) return 0;
	if (block > num_blocks - 1) return num_blocks - 1;
	return (int)block;
}


class Game			 							
{
	public:
		//Eventloop
  		Boolean	gQuit;
		float gamespeed;
		double multiplier2,multiplier3,multiplier4,multiplier5,end,start,timetaken,framespersecond;
		timer theTimer;	
		float sps;
		int maxfps;
#ifdef OS9 
		AGLContext gOpenGLContext;
		CGrafPtr	theScreen;
#endif
		//Graphics
		int screenwidth,screenheight;
		float viewdistance;
		
		//GL functions
		GLvoid ReSizeGLScene(float fov, float near);
		int DrawGLScene(void);
		int InitGL(void);
		void LoadingScreen(float percent);
		
		//Game Functions
		void	HandleKeyDown( char theChar );
#ifdef OS9 
		void	DoEvent( EventRecord *event );
#endif
		void	EventLoop( void );
		void	Frame( void );      /* one pass of it, so a browser can drive it itself */
		void 	Tick();
		void 	Splat(int k);
		void 	InitGame();
		void 	Dispose();
		
		//Mouse
		Point mouseloc;
		Point oldmouseloc;

		float mouserotation,mouserotation2;
		float oldmouserotation,oldmouserotation2;
		float mousesensitivity;
		float usermousesensitivity;
		
		//keyboard
		
		bool tabkeydown;
		
		//Project Specific
		int cityrotation[num_blocks][num_blocks];
		int citytype[num_blocks][num_blocks];
		int citypeoplenum[num_blocks][num_blocks];
		bool drawn[num_blocks][num_blocks];
		int onblockx,onblocky;
		bool cubetest;
		bool disttest;
		bool oldbutton;
		
		bool initialized;
		
		float flashamount;
		float flashr,flashg,flashb;
		
		int enemystate;
		
		int cycle;
		
		bool whacked;
		
		float losedelay;
		
		XYZ bodycoords;
		
		FRUSTUM frustum;
		Model blocks[4];
		Model blockwalls[4];
		Model blockcollide[4];
		Model blocksimplecollide[4];
		Model blockroofs[4];
		Model blockocclude;
		Model sidewalkcollide;
		Model street;
		Model Bigstreet;
		Model path;
		Model blocksimple;
		XYZ boundingpoints[8];
		Files files;
		Text text;
		int goodkills;
		int badkills;
		int civkills;
		int machinegunsoundloop;
		
		bool lasersight;
		bool debug;
		bool vblsync;
		
		bool blur;
		bool blurness;
		
		bool paused;
		
		int mainmenu;
		
		bool reloadtoggle;
		
		bool aimtoggle;

		Point olddrawmouse;
	
		XYZ vipgoal;
		
		XYZ aimer[2];
		
		double eqn[4];
		
		float oldrot,oldrot2;
		
		XYZ lastshot[2];
		bool zoom;
		bool oldzoom;
		
		int numpeople;
		float spawndelay;
		
		bool customlevels;
		
		bool musictoggle;
		
		float psychicpower;
		
		int type;
	
		bool slomokeydown;
		
		int mouseoverbutton;
		int oldmouseoverbutton;
		
		Person *person;          /* maxpeople of them, allocated once config.txt is read (InitGL) */
		
		GLuint 				personspritetextureptr;
		GLuint 				deadpersonspritetextureptr;
		GLuint 				scopetextureptr;
		GLuint 				flaretextureptr;
		
		bool killedinnocent;
		bool gameinprogress;
		bool beatgame;
		bool mainmenuness;
		int murderer;
		float timeremaining;
		int whichsong;
		int oldscore;
		int highscore;
		int score;
		int mission;
		int nummissions;
		int numpossibleguns;
		int possiblegun[6];
		int evilprobability;
		float difficulty;
		bool azertykeyboard;
		bool oldvisionkey;
		
		/* Window/quit plumbing for the SDL2 port. */
		bool mousegrab;          /* pointer locked for mouse-look (ctrl-G) */
		int  antialiasing;       /* MSAA samples requested in config.txt   */
		bool startfullscreen;
		void SaveHighscore();
		void RequestQuit();
		void UpdateMouseGrab();

		/* From Black Shades Enhanced (github.com/Bitl/Black-Shades-Enhanced). */
		float fov;               /* vertical field of view; the scope zooms to a quarter of it */
		int fpslimit;            /* config.txt's frame limit, in the game's own frame units */
		void WriteConfig(const char *path);

		/* config.txt's "View distance": multiplies every environment's view and
		   fog distance (1 = the original), and the populated area with them. */
		float viewscale;
		int maxpeople;           /* the biggest crowd any mission's weather needs (person[] is this long) */
		float populationdensity; /* config.txt's "Population density": people per block, times this */
		float assassinmultiplier;/* config.txt's "Assassins": each mission's own number times this */
		int swapfacebuttons;     /* config.txt's face buttons: -1 work it out, 0 as SDL reports, 1 swapped */
		float blockpeople;       /* people per block: block_people times the density */
		bool prepopulate;        /* a new mission: fill the streets at the next tick */
		float PeopleVisibleDistance(int env);   /* how far away anyone can be seen in that weather */
		bool PickSpawnSpot(bool anywhere, int radius, float hidden, int *x, int *z, int *vertex);
		void MovePersonToBlock(int who, int blockx, int blockz);

		/* Revenge mode, from the same project's Revenge Mod: no VIP, and the
		   assassins come for you instead.  Chosen from the main menu, and it
		   keeps its own high score (and "beat the game" flag) in
		   HighscoreRevenge beside the normal Highscore. */
		bool revenge;
		int revengehighscore;
		bool revengebeatgame;
		int &ModeHighscore() { return revenge ? revengehighscore : highscore; }
		bool &ModeBeatgame() { return revenge ? revengebeatgame : beatgame; }
		/* Who the assassins are after: the VIP (person 1), or you in revenge mode. */
		int AssassinTarget() const { return revenge ? 0 : 1; }
		void StartNewGame(bool revengemode);
		void UnZoom();          /* leave the sniper scope, undoing its aim correction */

		/* The main menu's buttons - 1 New Game / Resume Game, 2 Quit / End
		   Game, 3 Revenge Game - as the bottom edge of each 70-unit-high
		   button in the menu's 640x480 space (y up), or -1 when it is not
		   shown.  The drawing and the mouse code both go by this. */
		float MenuButtonY(int button) const {
			if (gameinprogress) return button == 1 ? 235 : button == 2 ? 112 : -1;
			return button == 1 ? 245 : button == 3 ? 160 : button == 2 ? 75 : -1;
		}

		/* This used to delete its textures by casting the texture *names* to
		   pointers, and ran after SDL had already torn the GL context down -
		   a crash on exit waiting to happen.  The context owns the textures,
		   so letting it go is all the cleanup that is needed. */
		~Game() {
		}

};

#endif
