#include "Game.h"	



extern double multiplier;

extern int thirdperson;

extern int visions;

extern Sprites sprites;



extern unsigned int gSourceID[100];

extern unsigned int gSampleSet[100];

extern Camera camera;

extern int pointeraim;
extern float aimrotation;
extern float aimrotation2;
extern float rad2deg;

extern float camerashake;

extern Fog fog;

extern int environment;

extern float precipitationhorz;

extern float precipitationvert;

extern float snowdelay;

extern float precipitationdensity;

extern float soundscalefactor;

extern int slomo;



extern int forwardskey;

extern int backwardskey;

extern int leftkey;

extern int rightkey;

extern int aimkey;

extern int psychicaimkey;

extern int psychickey;



extern Decals decals;

extern bool blood;   //config.txt's Blood setting

/********************> Tick() <*****/

#define maxfallvel 40



void Game::Splat(int k){

	if(k!=0||visions==0){

		person[k].health=0;

		person[k].DoAnimations(k);

		person[k].skeleton.offsetted=0;

		person[k].skeleton.free=1;

		person[k].longdead=1;



		person[k].bleeding=1;

		person[k].bleeddelay=1;

		person[k].bjoint1=&person[k].skeleton.joints[person[k].skeleton.jointlabels[head]];

		person[k].bjoint2=&person[k].skeleton.joints[person[k].skeleton.jointlabels[neck]];

		

		for(int j=0;j<person[k].skeleton.num_joints;j++){

			person[k].skeleton.joints[j].position+=person[k].skeleton.joints[j].offset;

			person[k].skeleton.joints[j].position=DoRotation(person[k].skeleton.joints[j].position,0,person[k].playerrotation,0);

			person[k].skeleton.joints[j].position+=person[k].playercoords;

			person[k].skeleton.joints[j].realoldposition=person[k].skeleton.joints[j].position;

			person[k].skeleton.joints[j].velocity=0;

			person[k].skeleton.joints[j].velocity.y+=person[k].velocity.y;

		}

		

		float gLoc[3];

		gLoc[0]=person[k].skeleton.joints[person[k].skeleton.jointlabels[head]].position.x/soundscalefactor;

		gLoc[1]=person[k].skeleton.joints[person[k].skeleton.jointlabels[head]].position.y/soundscalefactor;

		gLoc[2]=person[k].skeleton.joints[person[k].skeleton.jointlabels[head]].position.z/soundscalefactor;

		alSourcefv(gSourceID[headwhacksound], AL_POSITION, gLoc);

		alSourcePlay(gSourceID[headwhacksound]);

	}	

}



/* Everyone's own cruising speed, picked when they spawn, on a bell curve:
   most people at the original's walk (7.2 units a second, at a speedmult of
   1), fewer the further from it, out to half that walk on the slow side and
   a full run (15, the jog) on the fast one - each three standard deviations
   from the middle, where it is cut off.  About two thirds walk at 6 to 9.8,
   a quarter jog, and one in 75 is close to a full run.  Up to
   cruise_jog_pace they walk, the animation slowed or quickened to match;
   past it they jog (see the People loop in Tick). */
#define walk_pace 7.2f
#define slowest_pace (walk_pace*.5f)
#define run_pace 15.f
#define cruise_jog_pace 9.f
static float RandomPace()
{
	float z;
	do{   //a standard normal, from two evenly spread numbers (Box-Muller)
		const float u1=(Random()+32768)/65535.f, u2=(Random()+32767)/65535.f;   //(0,1] and [0,1)
		z=sqrt(-2*log(u1))*cos(6.2831853f*u2);
	}while(z<-3||z>3);
	return z<0?walk_pace+z*(walk_pace-slowest_pace)/3:walk_pace+z*(run_pace-walk_pace)/3;
}

/* How many blocks of a square radius blocks around this one are in the
   city (1 to num_blocks-1 are the ones people live in), on one axis. */
static int BlocksInCity(int centre, int radius)
{
	const int lo=centre-radius>1?centre-radius:1;
	const int hi=centre+radius<num_blocks-1?centre+radius:num_blocks-1;
	return hi>=lo?hi-lo+1:0;
}

/* How far away anyone can be seen in the given weather, with a little to
   spare for the camera: people are drawn out to 1000 units times the View
   distance (never less than 1000), and the fog hides everything past 80%
   of the weather's own view distance (see Draw()). */
float Game::PeopleVisibleDistance(int env)
{
	float base=2000;   //sunny
	if(env==foggy_environment||env==night_environment)base=500;
	if(env==snowy_environment)base=800;
	if(env==rainy_environment)base=700;
	if(env==firey_environment)base=600;
	const float fogged=base*viewscale*.8f;
	const float drawn=1000*(viewscale>1?viewscale:1);
	return (fogged<drawn?fogged:drawn)+40;
}

/* Someone has walked, or been thrown, into another block: citypeoplenum,
   which the spawner works from, follows them.  It does not count you. */
void Game::MovePersonToBlock(int who, int blockx, int blockz)
{
	if(person[who].whichblockx==blockx&&person[who].whichblocky==blockz)return;
	if(who>0){
		citypeoplenum[person[who].whichblockx][person[who].whichblocky]--;
		citypeoplenum[blockx][blockz]++;
	}
	person[who].whichblockx=blockx;
	person[who].whichblocky=blockz;
}

/* Where someone spawns: a block of the populated square (radius blocks
   around yours) that is short of its blockpeople, and a sidewalk corner of
   it.  While a mission is being populated anywhere will do, as long as it
   is not on top of you; after that only a corner out past hidden, in the
   outer blocks of the square, so that nobody appears in view.  Returns
   false, and leaves the outputs alone, if it found nowhere this time. */
bool Game::PickSpawnSpot(bool anywhere, int radius, float hidden, int *x, int *z, int *vertex)
{
	const int px=CityBlock(person[0].playercoords.x), pz=CityBlock(person[0].playercoords.z);
	for(int tries=0;tries<64;tries++){
		const int dx=abs(Random())%(2*radius+1)-radius, dz=abs(Random())%(2*radius+1)-radius;
		if(!anywhere&&abs(dx)<radius-1&&abs(dz)<radius-1)continue;   //only the outer two rings
		const int bx=px+dx, bz=pz+dz;
		if(bx<1||bx>num_blocks-1||bz<1||bz>num_blocks-1)continue;
		if(citypeoplenum[bx][bz]+(Random()+32767)/65535.f>=blockpeople)continue;   //has its share
		int v=1;
		while(v==1&&path.vertexNum>1)v=abs(Random())%path.vertexNum;   //the original never used corner 1
		XYZ spot=path.vertex[v];
		spot.x+=bx*block_spacing;
		spot.z+=bz*block_spacing;
		const float distance=findDistancefast(spot,person[0].playercoords);
		if(anywhere?distance<900:distance<hidden*hidden)continue;   //on top of you / in sight
		*x=bx; *z=bz; *vertex=v;
		return true;
	}
	return false;
}

/* "New Game" and "Revenge Game" on the main menu: start mission 1 of either. */
void Game::StartNewGame(bool revengemode)
{
	revenge=revengemode;

	flashamount=1;
	flashr=1;flashg=1;flashb=1;
	mainmenu=0;
	alSourcePlay(gSourceID[soulinsound]);
	mission=0;
	InitGame();
	gameinprogress=1;

	/* music and rain for the mission just set up (this used to go by
	   whatever the previous game had left behind) */
	if(environment==rainy_environment)alSourcePlay(gSourceID[rainsound]);
	if(environment!=rainy_environment)alSourcePause(gSourceID[rainsound]);
	alSourceStop(gSourceID[whichsong]);
	alSourcef(gSourceID[whichsong], AL_MIN_GAIN, 0);
	alSourcef(gSourceID[whichsong], AL_MAX_GAIN, 0);
	if(person[0].whichgun==knife)whichsong=knifesong;
	if(person[0].whichgun!=knife)whichsong=shootsong;
	if(type==zombie_type)whichsong=zombiesong;
	alSourcef(gSourceID[whichsong], AL_PITCH, 1);
	alSourcePlay(gSourceID[whichsong]);
	alSourcef(gSourceID[whichsong], AL_MIN_GAIN, 1);
	alSourcef(gSourceID[whichsong], AL_MAX_GAIN, 1);
}

/* Leave the sniper scope.  Zooming in turns the view 14 degrees down and 9
   left, so the scope looks where the rifle points (z33ky's aim correction in
   Black Shades Enhanced); zooming out turns it back.  Every way out of the
   scope goes through here, so the correction is always undone exactly once -
   nothing is corrected when the scope was not up. */
void Game::UnZoom()
{
	if(!zoom)return;
	zoom=0;
	camera.rotation2+=14;
	camera.rotation+=9;
}

/* ----------------------------------------------------------------------
   Light-gun aiming.

   The pointer says where on the screen the gun is meant to be pointed.  A
   perspective projection makes that an angle: a point at x across the
   viewport is tan(x * tan(hfov/2)) away from the middle, so the offsets
   below put the gun exactly under the crosshair rather than approximately.

   Inside a dead zone in the middle of the screen the camera does not move at
   all, which is what makes it feel like a light gun instead of a mouse; past
   it the view is pushed round, faster the further out the pointer is, so you
   can follow someone off the edge of the screen without the stick.

   The camera keeps its own angles throughout.  All this writes is
   aimrotation/aimrotation2, which Tick() hands to the body, and the body
   poses the arms, which is what the gun model and the bullet are taken from.
   ---------------------------------------------------------------------- */

/* how far out the pointer travels before it starts pushing the view, as a
   fraction of half the screen, and how fast it pushes at the very edge */
static const float pointer_deadzone = .55;
static const float pointer_pan      = 150;   /* per multiplier, so 250 degrees a second */

/* where the pointer is, as -1..1 from the middle of the screen */
/* Where the pointer was the last time anyone could see it.  A Wiimote stops
   reporting as soon as it nears the edge of what the sensor bar can see -
   which is exactly when you are pushing the view round - so the aim is held
   there rather than thrown back to the middle of the screen. */
static float pointer_lastx = 0, pointer_lasty = 0;

/* and how long it has been gone, in seconds of game time */
static float pointer_lost = 0;

/* A blink at the edge should not stop you turning, but a Wiimote put down on
   the sofa should not leave the view spinning either. */
static const float pointer_grace = 1;

/* ----------------------------------------------------------------------
   Making the crosshair tell the truth.

   Nothing in this game shoots along the body's forward axis.  The shot
   direction is the vector between two joints of the posed skeleton - the
   two hands for a rifle, the gun hand and the head for a pistol - and
   that vector sits several degrees off whichever way the body is pointed,
   by a different amount for every gun.  Pointing the *body* at the
   crosshair therefore leaves the *barrel* beside it: measured on the
   desktop build, 6.2-6.7 degrees for the assault rifle, which is a clean
   miss at any range worth shooting at.

   The original never had to care, because its crosshair was the middle of
   the screen and the offset only made the gun sit a little off centre.  A
   crosshair the player has put on somebody is a promise.

   So the offset is measured out of the pose every frame, using the same
   expressions the firing code uses, and handed back to the body as extra
   angle.  The bullet, the gun model and the laser sight are all read off
   that one pose, so all three land on the crosshair together.
   ---------------------------------------------------------------------- */

/* how far the barrel sits from the angles the body was given, in degrees */
static float gunoffset_yaw = 0, gunoffset_pitch = 0;

/* and the most any pose is allowed to ask for, so that a gun this does not
   know about, or a pose caught mid-animation, cannot throw the body round */
static const float gunoffset_max = 25;

/* the line the crosshair is drawn on, out of the camera */
static XYZ pointer_dir;

/* Where the gun points, in the body's own frame - before playerrotation is
   applied, which is exactly how the firing code builds it, per-gun fudge
   angles and all.  false for anything that does not fire in a line. */
static bool GunBarrel(Person &p, XYZ *out)
{
	XYZ v;

	if(p.skeleton.free)return false;

	if(p.whichgun==assaultrifle||p.whichgun==sniperrifle||p.whichgun==shotgun){

		v=p.skeleton.joints[p.skeleton.jointlabels[lefthand]].position
		 -p.skeleton.joints[p.skeleton.jointlabels[righthand]].position;

		if(p.whichgun==assaultrifle)v=DoRotation(v,0,-2.5,0);
		if(p.whichgun==sniperrifle )v=DoRotation(v,0, 4  ,0);

		/* the shotgun throws its pellets 2 to 4 degrees up and a degree
		   either side; the middle of that spread is where the crosshair
		   belongs */
		if(p.whichgun==shotgun     )v=DoRotation(v,3, 0  ,0);

	}

	else if(p.whichgun==handgun1||p.whichgun==handgun2){

		v=p.skeleton.joints[p.skeleton.jointlabels[righthand]].position
		 -(p.skeleton.joints[p.skeleton.jointlabels[head]].position*(thirdperson?.35:.65)
		  +p.skeleton.joints[p.skeleton.jointlabels[neck]].position*(thirdperson?.65:.35));

		v=DoRotation(v,0,-.9,0);

	}

	else return false;

	if(v.x==0&&v.y==0&&v.z==0)return false;

	Normalise(&v);

	*out=v;

	return true;
}

/* Measure that offset off the pose the last frame left behind.  Only off a
   pose that is actually holding the gun up: one halfway down, or a
   ragdoll, or a body too hurt to aim, says nothing about where a shot
   would go, so the last honest measurement is kept instead. */
static void GunOffset(Person &p)
{
	XYZ v;

	if(p.aimamount<.99)return;
	if(p.health<100)return;
	if(!GunBarrel(p,&v))return;

	static const double degrees=180.0/3.14159265358979;

	/* Yaw: playerrotation is applied to this vector at the moment of
	   firing, so the vector's own angle inside the body frame *is* how
	   far the shot lands from wherever the body was pointed. */
	float offyaw=atan2(v.x,v.z)*degrees;

	/* Pitch: a yaw does not touch y, so the barrel's pitch is asin(y) -
	   negative, because a positive rotation2 looks down - and the offset
	   is how far that is from the pitch the body was given. */
	float offpitch=p.playerrotation2+asin(v.y)*degrees;

	if(offyaw  > gunoffset_max)offyaw  = gunoffset_max;
	if(offyaw  <-gunoffset_max)offyaw  =-gunoffset_max;
	if(offpitch> gunoffset_max)offpitch= gunoffset_max;
	if(offpitch<-gunoffset_max)offpitch=-gunoffset_max;

	/* eased in rather than snapped, so that picking up a different gun, or
	   coming out of an animation, swings the body over instead of
	   teleporting it.  A frame or two, no more. */
	float rate=multiplier*30;

	if(rate>1)rate=1;

	gunoffset_yaw  +=(offyaw  -gunoffset_yaw  )*rate;
	gunoffset_pitch+=(offpitch-gunoffset_pitch)*rate;
}

/* Pointing the gun the right way still leaves the shot beside the
   crosshair, because the two start in different places: the crosshair is
   drawn on a line out of the camera and the bullet leaves the hand, a foot
   or so down and to the left of it.  Parallel lines never meet, so that is
   half a unit of miss at every range - a head's width - which is exactly
   the sort of lie a crosshair should not tell.

   So the shot's starting point is slid sideways onto the camera's line,
   keeping the distance down it that it already had.  The shot then runs
   along the crosshair's own line, and the muzzle it appears to leave has
   moved by less than the length of the gun.  The direction is left alone,
   so a shotgun still throws its pellets apart.

   Only for you, only while a pointer is doing the aiming, and not in third
   person, where the camera is nowhere near the gun. */
static void PointerShotStart(XYZ *start)
{
	if(thirdperson)return;

	/* never aimed yet, so there is no line to put it on */
	if(pointer_dir.x==0&&pointer_dir.y==0&&pointer_dir.z==0)return;

	XYZ off=*start-camera.position;

	*start=camera.position+pointer_dir*(off.x*pointer_dir.x+off.y*pointer_dir.y+off.z*pointer_dir.z);
}

/* ----------------------------------------------------------------------
   Recoil, and the ceiling.

   Two things about a shot's kick only go wrong once a pointer is doing the
   aiming, and they compound.

   The original pushed camera.rotation2 up a few degrees a shot and left it
   there, because you pulled the mouse back down - that is what a mouse is
   for.  A pointer has no such gesture: it says where you are pointing, not
   which way to move, so nothing ever pulls the view back and the kick just
   ratchets.  Measured with the pointer sitting still in the middle of the
   screen, well inside the dead zone: two bursts of an assault rifle walked
   the view from level to 59 degrees up and left it there.

   And the ratchet runs into a ceiling.  The crosshair at the top of the
   screen asks the body for fov/2 of pitch on top of whatever the view has,
   the body stops at 89, and past that the gun stays at its limit while the
   crosshair carries on - measured at 45 degrees apart, stuck, with the body
   bent to vertical and the gun out of frame entirely.

   So: the kick is borrowed rather than kept, and the view stops short of
   the ceiling by exactly what the crosshair is worth.  The total reach is
   unchanged - it is just split between the view and the pointer now.
   ---------------------------------------------------------------------- */

/* degrees of kick the view still owes back, and how fast it pays: a
   proportion of what is left every second, so a burst settles in about half
   a second once it stops */
static float recoilborrowed = 0;
static const float recoilreturn = 8;

float Game::PointerPitchLimit()
{
	/* The frustum is tan(fov/2) high, so the pixel at the very top is
	   atan(tan(fov/2)) - exactly fov/2 - off the middle.  That is what the
	   crosshair can ask for, so that is what the view has to leave spare. */
	float limit=89-fov/2;

	if(limit<0)limit=0;

	return limit;
}

void Game::CameraRecoil(float degrees)
{
	const float before=camera.rotation2;

	camera.rotation2-=degrees;

	if(!pointeraim)return;   /* a mouse pulls its own recoil back down */

	/* The gun is aimed off the *shown* view, so a kick to the camera alone
	   does nothing at all until it has eaten the 15 degrees of slack
	   between the two and then drags everything at once.  Kick both, and
	   the view jolts by what the shot was worth. */
	camera.visrotation2-=degrees;

	const float limit=zoom?89:PointerPitchLimit();

	if(camera.rotation2   <-limit)camera.rotation2   =-limit;
	if(camera.rotation2   > limit)camera.rotation2   = limit;
	if(camera.visrotation2<-limit)camera.visrotation2=-limit;
	if(camera.visrotation2> limit)camera.visrotation2= limit;

	/* only what the view actually gave is owed back */
	recoilborrowed+=before-camera.rotation2;
}

void Game::PointerRecoilSettle()
{
	if(recoilborrowed<=0){recoilborrowed=0; return;}

	float back=recoilborrowed*recoilreturn*multiplier*.6f;   /* multiplier counts in .6s */

	if(back>recoilborrowed)back=recoilborrowed;

	camera.rotation2   +=back;
	camera.visrotation2+=back;

	recoilborrowed-=back;
}


/* ----------------------------------------------------------------------
   How much of a hit a joint takes, by how close it is to where the bullet
   landed.  Every site that pushes a body around shares this shape: weigh
   each joint by 200 over its squared distance from the hit, add the
   weights up, and give each joint its share of the total.

   The share is what keeps it bounded - the weights sum to one however
   large any of them is - but only while they are finite.  Divided by a
   raw squared distance, a joint sitting exactly on the hit point weighs
   infinity, the total weighs infinity, and every joint's share comes out
   inf/inf, which is NaN.  One NaN is the whole skeleton: a person drawn
   from joints that are all NaN sprays across the screen in their own
   colours, and because people are recycled rather than reloaded between
   missions it goes with them.

   A hundredth of a unit squared is a tenth of a unit apart, far below any
   separation a real skeleton has, so the floor never changes a hit that
   was already well formed.  The `!(d>floor)` form catches a NaN distance
   too.
   ---------------------------------------------------------------------- */
static float HitWeight(XYZ joint, XYZ hit)
{
	float d=findDistancefast(joint,hit);

	if(!(d>.01f))d=.01f;

	return 200/d;
}


#ifdef __wii__
/* Wii only.  These are the two read-only sweeps that the people-
   exploding-into-triangles fault turns out to depend on: it stays away
   while they are compiled in and comes back when they are not, tested
   both ways on hardware (see item 3 of README-wii.md).  That is a
   property of that console's build and nothing else, so no other
   platform carries the cost of them. */
/* ----------------------------------------------------------------------
   DO NOT DELETE THIS FUNCTION OR CheckModels() WITHOUT TESTING ON HARDWARE.

   Both of them only read.  Neither has ever logged a line.  And yet on a
   Wii the people-exploding-into-triangles fault stays away while they are
   compiled in and comes straight back when they are taken out - tested in
   both directions on a console.  Two read-only functions cannot fix
   anything, so what they are actually doing is moving the binary's layout
   and timing, and the real fault is a stray write that lands somewhere
   harmless while they are here.

   That is a bad reason to keep code and an honest one.  See item 3 of
   README-wii.md for everything that has been ruled out and where to look
   next.  Until somebody finds the write, this stays.
   ----------------------------------------------------------------------

   The net under the skeletons.

   A person is drawn as one small model per joint, so a single joint in the
   wrong place stretches triangles from wherever it is to wherever the rest
   of the body is - which is what a screenful of someone's shirt colour
   actually is.  And because people are recycled between missions rather
   than reloaded, a body in that state travels into the next mission.

   The first version of this only caught positions past ten million, which
   was useless: geometry does not need astronomical coordinates to cover the
   screen, only wrong ones.  Measured over a couple of minutes of play, a
   joint never sits further than 5.95 units from the middle of its own body,
   ragdoll or not.  Forty is seven times that - far too loose to fire on
   anything real, far too tight to let a visible explosion through.

   The middle is taken as the median of the joints on each axis rather than
   the mean, because one joint that has gone wandering drags a mean with it
   and would make the whole body look broken.

   Anything caught goes in the log, with enough to say what that person was
   doing, and is then pulled back onto the body so the session survives.  It
   is a net, not a fix: a line in the log means a bug upstream of it.
   ---------------------------------------------------------------------- */

/* how far a joint may sit from the middle of its own body */
static const float joint_reach = 40;

static float MedianOf(float *v, int n)
{
	/* n is a skeleton's worth, twenty or so: an insertion sort is the right
	   tool and leaves nothing allocated */
	for(int i=1;i<n;i++){
		const float key=v[i];
		int k=i-1;
		while(k>=0&&v[k]>key){v[k+1]=v[k];k--;}
		v[k+1]=key;
	}
	return v[n/2];
}

/* The models themselves, which nothing in the game should ever write to
   after loading.  If one changes, everything drawn from it is wrong from
   that moment on and stays wrong for the life of the process - across a new
   mission and across the menu, which is exactly the shape of the fault this
   is looking for.  One model every eighth frame keeps it free. */
void Game::CheckModels()
{
	extern Model skeletonmodels[10];

	struct Watch { const char *name; Model *m; };

	Watch watch[]={
		{"head",&skeletonmodels[0]},{"chest",&skeletonmodels[1]},{"abdomen",&skeletonmodels[2]},
		{"upperarm",&skeletonmodels[3]},{"lowerarm",&skeletonmodels[4]},{"hand",&skeletonmodels[5]},
		{"upperleg",&skeletonmodels[6]},{"lowerleg",&skeletonmodels[7]},{"foot",&skeletonmodels[8]},
		{"shades",&skeletonmodels[9]},
		{"block0",&blocks[0]},{"block1",&blocks[1]},{"block2",&blocks[2]},{"block3",&blocks[3]},
		{"street",&street},{"sidewalk",&sidewalkcollide},{"blocksimple",&blocksimple},
	};

	const int nwatch=(int)(sizeof(watch)/sizeof(watch[0]));

	static unsigned long baseline[32];
	static bool ready=false;
	static int which=0;
	static int frame=0;
	static int said=0;

	if(++frame%8)return;

	for(int pass=0;pass<(ready?1:nwatch);pass++){

		if(which>=nwatch)which=0;

		Model *m=watch[which].m;

		/* the counts and the geometry they describe; vArray is built from
		   these, so there is no need to walk it too */
		unsigned long h=2166136261u;

		h^=(unsigned long)(unsigned short)m->vertexNum;   h*=16777619u;
		h^=(unsigned long)(unsigned short)m->TriangleNum; h*=16777619u;

		int nv=m->vertexNum, nt=m->TriangleNum;

		if(nv<0||nv>max_model_vertex)nv=0;
		if(nt<0||nt>max_textured_triangle)nt=0;

		for(int i=0;i<nv;i++){
			const unsigned char *b=(const unsigned char *)&m->vertex[i];
			for(unsigned k=0;k<sizeof(XYZ);k++){h^=b[k]; h*=16777619u;}
		}
		for(int i=0;i<nt;i++){
			const unsigned char *b=(const unsigned char *)&m->Triangles[i];
			for(unsigned k=0;k<sizeof(TexturedTriangle);k++){h^=b[k]; h*=16777619u;}
		}

		if(!ready)baseline[which]=h;
		else if(h!=baseline[which]){

			baseline[which]=h;   /* only complain once per change */

			if(said<8){

				said++;

				char note[220];

				snprintf(note,sizeof(note),
				         "model: %s changed after loading - now %d vertices,"
				         " %d triangles (limits %d/%d)\n",
				         watch[which].name,(int)m->vertexNum,(int)m->TriangleNum,
				         max_model_vertex,max_textured_triangle);

				PlatformLogf("%s",note);

				fputs(note,stderr);

				fflush(stderr);

			}

		}

		which++;

	}

	ready=true;
}

void Game::CheckSkeletons()
{
	static int sweep=0;
	static int said=0;

	if(numpeople<1||!person)return;

	/* sixteen people a frame: the whole crowd two or three times a second on
	   a Wii, and far too little work to notice anywhere else */
	for(int n=0;n<16&&n<numpeople;n++){

		if(sweep>=numpeople)sweep=0;

		const int who=sweep++;

		Person &p=person[who];

		int joints=p.skeleton.num_joints;

		if(joints<1||joints>max_joints)continue;

		/* the middle of the body, robust to a joint that has left it */
		float xs[max_joints],ys[max_joints],zs[max_joints];

		int good=0;

		for(int j=0;j<joints;j++){

			const XYZ &q=p.skeleton.joints[j].position;

			if(!(fabs(q.x)<1e7)||!(fabs(q.y)<1e7)||!(fabs(q.z)<1e7))continue;   /* also drops a NaN */

			xs[good]=q.x; ys[good]=q.y; zs[good]=q.z; good++;

		}

		XYZ middle;

		middle=0;

		if(good>0){

			middle.x=MedianOf(xs,good);
			middle.y=MedianOf(ys,good);
			middle.z=MedianOf(zs,good);

		}
		else if(p.skeleton.free){

			/* every joint is gone: the body itself is the only reference left */
			middle=p.playercoords;

			if(!(fabs(middle.x)<1e7)||!(fabs(middle.y)<1e7)||!(fabs(middle.z)<1e7))middle=person[0].playercoords;

		}

		for(int j=0;j<joints;j++){

			Joint &J=p.skeleton.joints[j];

			/* written as !(x < limit) so that a NaN fails every one of them */
			const bool badpos=!(fabs(J.position.x)<1e7)||!(fabs(J.position.y)<1e7)||!(fabs(J.position.z)<1e7);
			const bool badvel=!(fabs(J.velocity.x)<1e7)||!(fabs(J.velocity.y)<1e7)||!(fabs(J.velocity.z)<1e7);
			const bool toofar=!badpos&&!(findDistance(J.position,middle)<joint_reach);

			if(!badpos&&!badvel&&!toofar)continue;

			if(said<12){

				said++;

				char note[420];

				snprintf(note,sizeof(note),
				         "skeleton: person %d joint %d %s - pos %g %g %g, middle %g %g %g,"
				         " %g out, vel %g %g %g, ragdoll %d, health %g, type %d, gun %d,"
				         " anim %d, crowd %d\n",
				         who,j,badpos?"went non-finite":(badvel?"has a wild velocity":"left its body"),
				         (double)J.position.x,(double)J.position.y,(double)J.position.z,
				         (double)middle.x,(double)middle.y,(double)middle.z,
				         (double)(badpos?-1:findDistance(J.position,middle)),
				         (double)J.velocity.x,(double)J.velocity.y,(double)J.velocity.z,
				         (int)p.skeleton.free,(double)p.health,p.type,p.whichgun,
				         p.currentanimation,numpeople);

				/* blackshades.log is the only channel on a Wii, stderr the one
				   everywhere else, so say it in both */
				PlatformLogf("%s",note);

				fputs(note,stderr);

				fflush(stderr);

			}

			if(badvel)J.velocity=0;

			if(badpos||toofar)J.position=middle;

		}

	}

}

#endif   /* __wii__ - the two sweeps above */

bool Game::PointerPosition(float *outx, float *outy)
{
	bool seen = false;

	if(screenwidth>0&&screenheight>0&&PlatformPointerValid()){

		Point p;

		GetMouse(&p);

		float x = (float)p.h/(screenwidth /2.0f)-1;
		float y = (float)p.v/(screenheight/2.0f)-1;   /* positive downwards, as the pitch is */

		/* Nudged past the edge, it is still pointing at the edge - clamped,
		   not thrown away.  Wildly out is a reading to ignore. */
		if(x>=-2&&x<=2&&y>=-2&&y<=2){

			pointer_lastx = x<-1 ? -1 : (x>1 ? 1 : x);
			pointer_lasty = y<-1 ? -1 : (y>1 ? 1 : y);

			seen = true;

		}

	}

	*outx = pointer_lastx;
	*outy = pointer_lasty;

	return seen;   /* the position is always the last real one, seen or not */
}

void Game::PointerPan()
{
	float px,py;

	const bool seen = PointerPosition(&px,&py);

	if(seen) pointer_lost = 0;
	else     pointer_lost += multiplier*.6f;   /* multiplier counts in 0.6s */

	/* Lost for a moment - which happens every time the pointer reaches the
	   edge - the view carries on turning where it was going.  Lost for
	   longer than that and it stops, rather than spinning by itself. */
	if(!seen&&pointer_lost>pointer_grace)return;

	/* scoped, the crosshair is the scope's own and the pointer steers */
	const float deadzone = zoom ? 0 : pointer_deadzone;

	const float overx = fabs(px)>deadzone ? (fabs(px)-deadzone)/(1-deadzone) : 0;
	const float overy = fabs(py)>deadzone ? (fabs(py)-deadzone)/(1-deadzone) : 0;

	/* squared, so a pointer just past the dead zone drifts and one at the
	   edge swings - the same shape the stick's look speed uses */
	if(overx>0)camera.rotation +=(px>0?1:-1)*overx*overx*pointer_pan*multiplier;
	if(overy>0)camera.rotation2+=(py>0?1:-1)*overy*overy*pointer_pan*multiplier;
}

void Game::PointerAim()
{
	float px,py;

	/* Seen or not, this is where the pointer last was: an aim that jumped
	   back to the middle every time the Wiimote blinked would be unusable. */
	PointerPosition(&px,&py);

	/* Scoped, the rifle is the crosshair; anywhere else the pointer is. */
	if(zoom){

		aimrotation =camera.visrotation;
		aimrotation2=camera.visrotation2;

		return;

	}

	const float halfheight=tan(fov*(3.14159265358979/360.0));
	const float halfwidth =halfheight*((double)screenwidth/(double)screenheight);

	/* off the view the player is actually shown, so the gun lines up with the
	   crosshair even while the view is lagging behind the camera */
	static const double degrees=180.0/3.14159265358979;   /* not the game's rad2deg, which is 56.55 */

	/* where the crosshair is */
	const float wantrotation =camera.visrotation +atan(px*halfwidth )*degrees;
	const float wantrotation2=camera.visrotation2+atan(py*halfheight)*degrees;

	/* and the body pointed that much further over, so that the gun rather
	   than the chest ends up on it */
	GunOffset(person[0]);

	aimrotation =wantrotation +gunoffset_yaw;
	aimrotation2=wantrotation2+gunoffset_pitch;

	/* and the crosshair's own line through the world, which the shot and
	   the laser sight are both put onto */
	pointer_dir=0;
	pointer_dir.z=-1;
	pointer_dir=DoRotation(pointer_dir,-wantrotation2,0,0);
	pointer_dir=DoRotation(pointer_dir,0,-wantrotation,0);

	if(aimrotation2> 89)aimrotation2= 89;
	if(aimrotation2<-89)aimrotation2=-89;


}

void 	Game::Tick(){

	if(mainmenu){

		

		unsigned char	theKeyMap[16];

		GetKeys( ( unsigned long * )theKeyMap );

		

		if(IsKeyDown(theKeyMap, MAC_SPACE_KEY)){

			mainmenu=0;

		}

		

		GetMouse(&mouseloc);

		float mousex=mouseloc.h;

		float mousey=mouseloc.v;

		mousex=(float)mouseloc.h*640/screenwidth;

		mousey=480-(float)mouseloc.v*480/screenheight;
		

		oldmouseoverbutton=mouseoverbutton;

		mouseoverbutton=0;

		for(int button=1;button<=3;button++){

			const float y=MenuButtonY(button);

			if(y>=0&&mousex>120&&mousex<560&&mousey>y&&mousey<y+70)mouseoverbutton=button;

		}

		if((Button()&&mouseoverbutton==1&&!gameinprogress&&!oldbutton)||!mainmenuness){

			StartNewGame(false);

		}

		if(Button()&&mouseoverbutton==3&&!gameinprogress&&!oldbutton){

			StartNewGame(true);

		}

		if((Button()&&mouseoverbutton==1&&gameinprogress&&!oldbutton)||!mainmenuness){

			flashamount=1;

			flashr=1;flashg=1;flashb=1;

			mainmenu=0;

			MoveMouse(oldmouseloc.h,oldmouseloc.v,&mouseloc);

			//if(!visions){

				if(environment==rainy_environment)alSourcePlay(gSourceID[rainsound]);

				if(environment!=rainy_environment)alSourcePause(gSourceID[rainsound]);

				alSourceStop(gSourceID[whichsong]);

				alSourcef(gSourceID[whichsong], AL_MIN_GAIN, 0);

				alSourcef(gSourceID[whichsong], AL_MAX_GAIN, 0);

				if(person[0].whichgun==knife)whichsong=knifesong;

				if(person[0].whichgun!=knife)whichsong=shootsong;

				if(type==zombie_type)whichsong=zombiesong;

				alSourcePlay(gSourceID[whichsong]);

				alSourcef(gSourceID[whichsong], AL_MIN_GAIN, 1);

				alSourcef(gSourceID[whichsong], AL_MAX_GAIN, 1);

			//}

			alSourcePlay(gSourceID[soulinsound]);

			if(visions)alSourcePlay(gSourceID[visionsound]);

		}

		if(Button()&&mouseoverbutton==2&&!gameinprogress&&!oldbutton){

			flashamount=1;

			flashr=1;flashg=0;flashb=0;

			alSourcePlay(gSourceID[losesound]);

			gQuit = true;

			SaveHighscore();

		}

		if(Button()&&mouseoverbutton==2&&gameinprogress&&!oldbutton){

			flashamount=1;

			flashr=1;flashg=1;flashb=1;

			alSourcePlay(gSourceID[losesound]);

			gameinprogress=0;

			SaveHighscore();

		}

		

		if(Button())oldbutton=1;

		if(!Button())oldbutton=0;

	}

	if(!mainmenu){

		

		

		XYZ facing;

		XYZ flatfacing;

		float speed=10;

		

		/* revenge mode has no VIP to lose: only your own death, or an innocent's */
		const bool viplost=!revenge&&person[1].health<=0;

		if(viplost||person[0].health<=0||killedinnocent){

			losedelay-=multiplier/6;

		}

		

		if(!viplost&&person[0].health>0&&!killedinnocent)timeremaining-=multiplier*25/40;

		if(timeremaining<=0){

			flashamount=1;

			flashr=0;flashg=1;flashb=0;

			mission++;

			alSourcePlay(gSourceID[souloutsound]);

			score+=100+(mission*50);

			if(mission>=nummissions){

				ModeBeatgame()=1;

				mainmenu=1;

				gameinprogress=0;

				alSourcePause(gSourceID[rainsound]);

		

				alSourceStop(gSourceID[visionsound]);

				alSourceStop(gSourceID[whichsong]);

				alSourcef(gSourceID[whichsong], AL_MIN_GAIN, 0);

				alSourcef(gSourceID[whichsong], AL_MAX_GAIN, 0);

				whichsong=mainmenusong;

				alSourceStop(gSourceID[whichsong]);

				alSourcef(gSourceID[whichsong], AL_PITCH, 1);

				alSourcePlay(gSourceID[whichsong]);

				alSourcef(gSourceID[whichsong], AL_MIN_GAIN, 1);

				alSourcef(gSourceID[whichsong], AL_MAX_GAIN, 1);

				SaveHighscore();

			}

			if(!mainmenu){

				InitGame();

				if(environment==rainy_environment)alSourcePlay(gSourceID[rainsound]);

				if(environment!=rainy_environment)alSourcePause(gSourceID[rainsound]);

				alSourceStop(gSourceID[visionsound]);

				alSourceStop(gSourceID[whichsong]);

				if(person[0].whichgun==knife)whichsong=knifesong;

				if(person[0].whichgun!=knife)whichsong=shootsong;

				if(type==zombie_type)whichsong=zombiesong;

				alSourcef(gSourceID[whichsong], AL_PITCH, 1);

				alSourcePlay(gSourceID[whichsong]);

				alSourcef(gSourceID[whichsong], AL_MIN_GAIN, 1);

				alSourcef(gSourceID[whichsong], AL_MAX_GAIN, 1);

			}

		}

		if(losedelay<=0){

			flashamount=1;

			flashr=0;flashg=0;flashb=0;

			/* -100 rather than -200 if the VIP's killer is dead.  murderer is reset
			   each mission, so losing some other way (dying, shooting a civilian)
			   no longer scores against whoever had that index before. */
			if(murderer>=0&&person[murderer].health<=0)score=oldscore-100;

			else score=oldscore-200;

			InitGame();

			alSourceStop(gSourceID[whichsong]);

			if(person[0].whichgun==knife)whichsong=knifesong;

			if(person[0].whichgun!=knife)whichsong=shootsong;

			if(type==zombie_type)whichsong=zombiesong;

			alSourcef(gSourceID[whichsong], AL_PITCH, 1);

			alSourcePlay(gSourceID[whichsong]);

			alSourcef(gSourceID[whichsong], AL_MIN_GAIN, 1);

			alSourcef(gSourceID[whichsong], AL_MAX_GAIN, 1);

			alSourcePlay(gSourceID[soulinsound]);

		}

		

		unsigned char	theKeyMap[16];

		GetKeys( ( unsigned long * )theKeyMap );

		

		//Sprites

		sprites.DoStuff();

		

		//Decals

		decals.DoStuff();

		

		//Facing

		facing=0;

		facing.z=-1;

		

		facing=DoRotation(facing,-camera.rotation2,0,0);

		facing=DoRotation(facing,0,0-camera.rotation,0);

		

		flatfacing=facing;

		flatfacing.y=0;

		Normalise(&flatfacing);

		

		if(IsKeyDown(theKeyMap, psychickey)&&!oldvisionkey){

			oldvisionkey=1;

			visions++;

			if(visions==2)visions=0;

			if(visions==0){

				float rad2deg=56.54866776;

				flashamount=1;

				flashr=1;flashg=1;flashb=1;

				

				if(person[0].playercoords.x!=bodycoords.x||person[0].playercoords.z!=bodycoords.z){

					XYZ towards;

					towards=person[0].playercoords-bodycoords;

					Normalise(&towards);

					camera.rotation=-asin(0-towards.x);

					camera.rotation*=360/6.28;

					if(towards.z>0)camera.rotation=180-camera.rotation;

					camera.visrotation=camera.rotation;

					camera.oldrotation=camera.rotation;

					camera.oldoldrotation=camera.rotation;

				}

				

				person[0].playercoords=bodycoords; 

				person[0].oldplayercoords=bodycoords;

				person[0].velocity=0;

				alSourcePlay(gSourceID[soulinsound]);

				alSourceStop(gSourceID[visionsound]);				

				alSourcef(gSourceID[knifesong], AL_PITCH, (ALfloat)(1));

				alSourcef(gSourceID[shootsong], AL_PITCH, (ALfloat)(1));

				alSourcef(gSourceID[zombiesong], AL_PITCH, (ALfloat)(1));

			}

			if(visions==1){

				alSourceStop(gSourceID[visionsound]);

				flashamount=1;

				flashr=1;flashg=0;flashb=0;

				bodycoords=person[0].oldplayercoords;

				alSourcePlay(gSourceID[souloutsound]);

				alSourcef(gSourceID[knifesong], AL_PITCH, (ALfloat)(.5));

				alSourcef(gSourceID[shootsong], AL_PITCH, (ALfloat)(.5));

				alSourcef(gSourceID[zombiesong], AL_PITCH, (ALfloat)(.5));

				alSourcePlay(gSourceID[visionsound]);

			}

		}

		if(!IsKeyDown(theKeyMap, psychickey)){

			oldvisionkey=0;

		}

		

		if(IsKeyDown(theKeyMap, MAC_TAB_KEY)&&!tabkeydown&&debug){

			thirdperson++;

			if(thirdperson>2)thirdperson=0;

			tabkeydown=1;

		}

		

		if(!IsKeyDown(theKeyMap, MAC_TAB_KEY))

			tabkeydown=0;

		

		if(IsKeyDown(theKeyMap, aimkey)&&!aimtoggle){

			person[0].aiming=1-person[0].aiming;

			aimtoggle=1;

		}

		

		if(!IsKeyDown(theKeyMap, aimkey))

			aimtoggle=0;

		

		if(IsKeyDown(theKeyMap, MAC_R_KEY)&&!reloadtoggle){

			if(person[0].reloads[person[0].whichgun]>0&&person[0].reloading<=0)person[0].ammo=-1;

			reloadtoggle=1;

		}

		

		if(!IsKeyDown(theKeyMap, MAC_R_KEY))

			reloadtoggle=0;

		

		if(IsKeyDown(theKeyMap, psychicaimkey)&&!slomokeydown&&slomo==0){

			alSourcePlay(gSourceID[souloutsound]);

			slomo=2;
			fprintf(stderr,"SLOMO on, psychicpower %g\n",psychicpower);

			flashamount=.5;

			flashr=1;flashg=0;flashb=0;

			slomokeydown=1;

			score-=20;

			alSourcef(gSourceID[knifesong], AL_PITCH, (ALfloat)(.5));

			alSourcef(gSourceID[shootsong], AL_PITCH, (ALfloat)(.5));

			alSourcef(gSourceID[zombiesong], AL_PITCH, (ALfloat)(.5));

		}

		

		if(!IsKeyDown(theKeyMap, psychicaimkey))

			slomokeydown=0;

			

		//Mouse look

		

		if((person[0].aimamount<=0&&person[0].targetanimation!=crouchanim)){

			camera.rotation=camera.visrotation;

			camera.rotation2=camera.visrotation2;

			mousesensitivity=usermousesensitivity;

		}

		if(person[0].aimamount>=1&&zoom==0){

			mousesensitivity=usermousesensitivity*.8;

		}

		if(slomo==2){

			mousesensitivity*=.6;

		}

#if 0	// DDOI
		GetMouse(&mouseloc);

		if (mouseloc.h>600){MoveMouse(mouseloc.h-500,mouseloc.v,&mouseloc);}

		if (mouseloc.h<100){MoveMouse(mouseloc.h+500,mouseloc.v,&mouseloc);}

		GetMouse(&mouseloc);

		if (mouseloc.v>400){MoveMouse(mouseloc.h,mouseloc.v-300,&mouseloc);}

		if (mouseloc.v<100){MoveMouse(mouseloc.h,mouseloc.v+300,&mouseloc);}

		GetMouse(&mouseloc);

#else
		/* Aiming with a pointer, the mouse's own motion is not a turn - the
		   pointer's position is the aim, and PointerPan below does the
		   turning.  What is left in here is the stick, which still turns. */
		if(pointeraim)GetPadRel(&mouseloc); else GetMouseRel(&mouseloc);
#endif

		

#if 0	// DDOI
		oldmouserotation=(oldmouseloc.h/1.3888)*mousesensitivity;

		oldmouserotation2=(oldmouseloc.v/1.3888)*mousesensitivity;
#endif

		mouserotation=(mouseloc.h/1.3888)*mousesensitivity;

		mouserotation2=(mouseloc.v/1.3888)*mousesensitivity;

		

#if 0	// DDOI
		if(abs(oldmouseloc.h-mouseloc.h)<400)camera.rotation+=mouserotation-oldmouserotation;

		if(abs(oldmouseloc.v-mouseloc.v)<200)camera.rotation2+=mouserotation2-oldmouserotation2;

		if(mouseloc.h-oldmouseloc.h>400)camera.rotation+=mouserotation-oldmouserotation-(500/1.3888*mousesensitivity);

		if(mouseloc.h-oldmouseloc.h<-400)camera.rotation+=mouserotation-oldmouserotation+(500/1.3888*mousesensitivity);

		if(mouseloc.v-oldmouseloc.v>200)camera.rotation2+=mouserotation2-oldmouserotation2-(300/1.3888*mousesensitivity);

		if(mouseloc.v-oldmouseloc.v<-200)camera.rotation2+=mouserotation2-oldmouserotation2+(300/1.3888*mousesensitivity);
#else
		/* Pointing at the screen does not turn the view by itself: the
		   pointer moves the gun, and pushes the view only once it nears the
		   edge.  A stick still turns, through the relative path below, so
		   the two work together. */
		if(pointeraim)PointerPan();

		if(abs(mouseloc.h)<400)camera.rotation+=mouserotation;
		if(abs(mouseloc.v)<200)camera.rotation2+=mouserotation2;
		if(mouseloc.h>400)camera.rotation+=mouserotation-(500/1.3888*mousesensitivity);
		if(mouseloc.h<-400)camera.rotation+=mouserotation+(500/1.3888*mousesensitivity);

		if(mouseloc.v>200)camera.rotation2+=mouserotation2-(300/1.3888*mousesensitivity);

		if(mouseloc.v<-200)camera.rotation2+=mouserotation2+(300/1.3888*mousesensitivity);
#endif

		

		/* hand back a little of whatever the last shots borrowed */
		if(pointeraim)PointerRecoilSettle();

		if(camera.rotation2>89){camera.rotation2=89;}

		if(camera.rotation2<-89){camera.rotation2=-89;}

		

		//Smooth

		camera.rotation=(camera.oldoldrotation+((camera.rotation-camera.oldoldrotation)*.7+(camera.oldrotation-camera.oldoldrotation)*.3));

		camera.rotation2=(camera.oldoldrotation2+((camera.rotation2-camera.oldoldrotation2)*.7+(camera.oldrotation2-camera.oldoldrotation2)*.3));

		

		if(camera.visrotation<camera.rotation-7)camera.visrotation=camera.rotation-7;

		if(camera.visrotation>camera.rotation+7)camera.visrotation=camera.rotation+7;

		if(camera.visrotation2<camera.rotation2-15)camera.visrotation2=camera.rotation2-15;

		if(camera.visrotation2>camera.rotation2+15)camera.visrotation2=camera.rotation2+15;

		/* The ceiling (see the recoil block above): with a pointer aiming,
		   the view stops fov/2 short of where the body gives out, so that
		   the crosshair at the edge of the screen is always an angle the gun
		   can actually reach.  Scoped, the crosshair is the scope's own and
		   the gun is the view, so the old limit stands. */
		if(pointeraim&&!zoom){

			const float pitchlimit=PointerPitchLimit();

			if(camera.rotation2   > pitchlimit)camera.rotation2   = pitchlimit;
			if(camera.rotation2   <-pitchlimit)camera.rotation2   =-pitchlimit;
			if(camera.visrotation2> pitchlimit)camera.visrotation2= pitchlimit;
			if(camera.visrotation2<-pitchlimit)camera.visrotation2=-pitchlimit;

		}

		

		/* oldzoom too: the frame the scope comes down (say, to reload), the view
		   snaps straight back instead of drifting (z33ky, Black Shades Enhanced) */
		if(oldzoom||zoom||person[0].aimamount<=0||person[0].whichgun==nogun||visions||person[0].whichgun==grenade||person[0].whichgun==knife){

			camera.visrotation=camera.rotation;

			camera.visrotation2=camera.rotation2;

		}

		oldzoom=zoom;

		

		/* the view is settled, so the gun can be pointed at the crosshair
		   drawn over it */
		if(pointeraim)PointerAim();

		camera.oldoldrotation=camera.oldrotation;

		camera.oldoldrotation2=camera.oldrotation2;

		camera.oldrotation=camera.rotation;

		camera.oldrotation2=camera.rotation2;

		

		//Check collision with buildings

		

		int beginx,endx;

		int beginz,endz;

		int distsquared;

		XYZ collpoint;

		XYZ move;

		float howmuchintersect;

		int whichtri;

		float olddist;

		XYZ nothing;

		XYZ underpoint;

		XYZ overpoint;

		int pointnum;

		float depth;

		XYZ normalrotated;

		XYZ pointrotated;

		bool inblock;

		

		

		/* the body follows the gun, not the eye, when they are not the same
		   thing: the arms are posed off this, and the shot is taken off the
		   arms */
		person[0].playerrotation=180-(pointeraim?aimrotation:camera.rotation);

		

		//Check people collisions

		for(int k=0;k<numpeople;k++){
			// SBF - backing up the old coordinates makes
			// the most sense here.
			person[k].oldplayercoords=person[k].playercoords;
			
			person[k].DoStuff(k);

			if(person[k].skeleton.free<1){

			if(1==1){

			person[k].onground=0;

			overpoint=person[k].playercoords;

			overpoint.y+=3000;

			underpoint=person[k].playercoords;

			underpoint.y-=3000;

			

			beginx=(person[k].playercoords.x+block_spacing/2)/block_spacing;

			if(beginx<0)beginx=0;

			beginz=(person[k].playercoords.z+block_spacing/2)/block_spacing;

			if(beginz<0)beginz=0;

			

			endx=(person[k].playercoords.x+block_spacing/2)/block_spacing+1;

			if(endx>num_blocks-1)endx=num_blocks-1;

			endz=(person[k].playercoords.z+block_spacing/2)/block_spacing+1;

			if(endz>num_blocks-1)endz=num_blocks-1;

			


			

			if(beginx<=endx&&beginz<=endz)

			for(int i=beginx;i<=endx;i++)

				for(int j=beginz;j<=endz;j++){

					inblock=0;

					//Ground collision

					move=0;

					move.x=i*block_spacing;

					move.z=j*block_spacing;

					whichtri=sidewalkcollide.LineCheck2(overpoint,underpoint,&collpoint,move,cityrotation[i][j]*90);

					if(whichtri!=-1&&person[k].playercoords.y<=collpoint.y&&person[k].velocity.y<=0){

						person[k].playercoords.y=collpoint.y;

						person[k].onground=1;

						if(person[k].velocity.y<-maxfallvel)Splat(k);

						else person[k].velocity.y=0;

					}

					if(whichtri!=-1){

						inblock=1;

						if(k==0){onblockx=i;onblocky=j;}

					}

					//Wall collision

					if(k==0){

						if(inblock){

							for(int l=0;l<8;l++){

								move=0;

								move.x=i*block_spacing;

								move.z=j*block_spacing;

								whichtri=blockwalls[citytype[i][j]].LineCheck3(person[k].oldplayercoords+boundingpoints[l],person[k].playercoords+boundingpoints[l],&collpoint,move,cityrotation[i][j]*90,&depth);

								if(whichtri!=-1){

									normalrotated=DoRotation(blockwalls[citytype[i][j]].normals[whichtri],0,cityrotation[i][j]*90,0);

									person[k].playercoords+=normalrotated*(-(dotproduct(normalrotated,person[k].playercoords-person[k].oldplayercoords))-depth);

								}

							}

							for(int l=0;l<8;l++){

								pointnum=k+1;

								if(pointnum>3)pointnum=0;

								move=0;

								move.x=i*block_spacing;

								move.z=j*block_spacing;

								whichtri=blockwalls[citytype[i][j]].LineCheck3(person[k].playercoords+boundingpoints[l],person[k].playercoords+boundingpoints[pointnum],&collpoint,move,cityrotation[i][j]*90,&depth);

								if(whichtri!=-1){

									normalrotated=DoRotation(blockwalls[citytype[i][j]].normals[whichtri],0,cityrotation[i][j]*90,0);

									person[k].playercoords+=normalrotated;

								}

							}

						}

						//Roof collision

						if(inblock&&person[k].playercoords.y>30){

							if(!person[k].onground){

								move=0;

								move.x=i*block_spacing;

								move.z=j*block_spacing;

								whichtri=blockroofs[citytype[i][j]].LineCheck2(overpoint,underpoint,&collpoint,move,cityrotation[i][j]*90);

								if(whichtri!=-1&&person[k].playercoords.y<=collpoint.y&&person[k].velocity.y<=0){

									person[k].playercoords.y=collpoint.y;

									person[k].onground=1;

									if(person[k].velocity.y<-maxfallvel)Splat(k);

									else person[k].velocity.y=0;

								}

								if(whichtri!=-1)inblock=1;

							}

						}

					}

				}

			if(person[k].playercoords.y<=0){

				person[k].onground=1;

				person[k].playercoords.y=0;

				if(person[k].velocity.y<-maxfallvel)Splat(k);

				else person[k].velocity.y=0;

				if(k==0)onblockx=-1;onblocky=-1;

			}

			// SBF - this is definately in the wrong spot!
			//person[k].oldplayercoords=person[k].playercoords;

			}

			}

		}

		

		if(IsKeyDown( theKeyMap, MAC_SPACE_KEY )){

			if(visions==0&&person[0].targetanimation==joganim&&person[0].currentanimation==joganim&&person[0].backwardsanim==0&&person[0].playerrotation==person[0].playerlowrotation){

				person[0].targetanimation=diveanim;

				person[0].targetframe=0;

				person[0].target=0;

				person[0].aimamount=0;

			}

		}

		

		//Camera

		camera.oldposition=camera.position;

		camera.targetoffset=0;

		camera.targetoffset.z=-5;

		

		

		//Spawn people

#ifdef __wii__
		CheckSkeletons();

		CheckModels();
#endif

		spawndelay-=multiplier;

		/* The crowd lives in the square of blocks out to peopleradius around
		   yours - a block or so past where anyone can be seen - blockpeople to
		   a block.  A new mission fills it at once; after that people only
		   spawn, and are moved on, out of sight at its edge. */
		const float peoplevisible=PeopleVisibleDistance(environment);

		int peopleradius=(int)ceil((peoplevisible+block_spacing/2)/block_spacing);

		if(peopleradius<1)peopleradius=1;

		if(peopleradius>max_spawn_radius)peopleradius=max_spawn_radius;

		const int playerblockx=CityBlock(person[0].playercoords.x), playerblockz=CityBlock(person[0].playercoords.z);

		const int crowdblocks=BlocksInCity(playerblockx,peopleradius)*BlocksInCity(playerblockz,peopleradius);

		int crowd=(int)(blockpeople*crowdblocks+.5f);

		if(crowd>maxpeople)crowd=maxpeople;

		if(crowd<2)crowd=2;   //always room for you and the VIP

		/* The assassins.  The original rolled every person it put on the
		   street at the mission's own odds, 1 in evilprobability, so that
		   fraction of the crowd was an assassin - and because the crowd it
		   rolled over was the couple of blocks around you, that fraction was
		   also what you met.  What makes a street feel like the original's is
		   that share, not the head count, so the share is what this keeps:
		   the mission's odds, times config.txt's Assassins multiplier, and
		   nothing at all to do with how far you can see.

		   A longer View distance therefore holds proportionally more of them,
		   because it holds proportionally more people - the street outside
		   your window is the same street either way, and the extra ones are
		   the ones the original would have had out there too if it had
		   simulated that far.  It also means a platform that caps the crowd,
		   like the Wii, still gets the right odds out of the people it can
		   afford.

		   This did the opposite for a while: a fixed dozen or so assassins
		   spread over whatever square the weather and the View distance
		   asked for, which measured as little as 1.1% of the crowd against
		   the original's 16%. */
		float assassinchance=assassinmultiplier/(float)evilprobability;

		if(assassinchance>1)assassinchance=1;   //a multiplier past the odds: everyone

		/* The original spawned (or recycled) someone every .1 of a second, at
		   most one a frame.  A bigger crowd does it proportionally more often,
		   which can take several a frame; no more than a frame's worth is ever
		   owed. */
		const float spawnstep=.1*90/crowd;

		if(spawndelay<-multiplier)spawndelay=-multiplier;

		const bool filling=prepopulate;   //a new mission: fill the whole square now

		prepopulate=false;

		for(int spawnpass=0;spawnpass<(filling?2*maxpeople:256)&&(filling||spawndelay<0);spawnpass++){

		if(filling)spawndelay=-1;

		const float spawndelaybefore=spawndelay;

		int blockspawnx=0,blockspawny=0,spawnvertex=0;

		int cyclenum=0;

		if(PickSpawnSpot(filling,peopleradius,peoplevisible,&blockspawnx,&blockspawny,&spawnvertex)){

		if(spawndelay<0&&numpeople<crowd){

			if(type==randomshoot_type){

				/* Anyone can be an assassin, with assassinchance (see above).
				   Zombie missions are all zombies. */

				if((Random()+32767)/65535.f<assassinchance)person[numpeople].type=eviltype;

				else person[numpeople].type=civiliantype;

			}

			if(type==zombie_type){

				person[numpeople].type=zombietype;

			}

		

			//no assassin (or zombie) starts in the block of the one it is after

			for(int retry=0;retry<16&&person[numpeople].type!=civiliantype&&blockspawnx==person[AssassinTarget()].whichblockx&&blockspawny==person[AssassinTarget()].whichblocky;retry++)PickSpawnSpot(filling,peopleradius,peoplevisible,&blockspawnx,&blockspawny,&spawnvertex);

					

			person[numpeople].playerrotation=0;

			

			person[numpeople].whichcostume=casualcostumes+abs(Random())%numcasual;

			

			person[numpeople].whichblockx=blockspawnx;

			person[numpeople].whichblocky=blockspawny;

			

			person[numpeople].pathnum=-1;

			person[numpeople].oldpathnum=-1;

			person[numpeople].oldoldpathnum=-1;

			person[numpeople].oldoldoldpathnum=-1;

			person[numpeople].pathnum=spawnvertex;   //the corner PickSpawnSpot chose

			person[numpeople].pathtarget.x=path.vertex[person[numpeople].pathnum].x;

			person[numpeople].pathtarget.z=path.vertex[person[numpeople].pathnum].z;			

			person[numpeople].pathsize=.98+float(abs(Random()%29))/400;   //.98-1.05: overlaps the assassins' 1.04

			person[numpeople].crossdelay=civiliancrossdelay*float(abs(Random()%100))/100;

			person[numpeople].pathtarget*=person[numpeople].pathsize;

			person[numpeople].pathtarget.x+=person[numpeople].whichblockx*block_spacing;

			person[numpeople].pathtarget.z+=person[numpeople].whichblocky*block_spacing;

			

			

			person[numpeople].playercoords=person[numpeople].pathtarget;

			person[numpeople].oldplayercoords=person[numpeople].playercoords;

			person[numpeople].skeleton.free=0;

			

			person[numpeople].targetanimation=walkanim;

			if(person[numpeople].type==zombietype)person[numpeople].targetanimation=zombiewalkanim;

			person[numpeople].speed=1;

			person[numpeople].existing=0;

			person[numpeople].speedmult=.8+float(abs(Random()%20))/50;

			person[numpeople].pace=person[numpeople].type==zombietype?0:RandomPace();   //zombies keep their own speeds, which their health drives

			person[numpeople].cruising=0;

			person[numpeople].health=100;	

			person[numpeople].maxhealth=100;	

			person[numpeople].playerrotation2=0;

			

			person[numpeople].lastdistancevictim=200000;

			

			if(person[numpeople].skeleton.broken)person[numpeople].skeleton.Load((char *)":Data:Skeleton:Basic Figure");

			

			if(numpeople==1&&!revenge)person[numpeople].type=viptype;   //revenge mode has no VIP: the Revenge Mod made its first spawn one

				

			person[numpeople].killtarget=-1;

			

			if(person[numpeople].type==eviltype){person[numpeople].existing=1; person[numpeople].pathsize=1.04; person[numpeople].whichgun=nogun; person[numpeople].aiming=1; person[numpeople].killtarget=-1; person[numpeople].speedmult=1+.3*difficulty;}

			if(person[numpeople].type==zombietype){person[numpeople].existing=1; person[numpeople].pathsize=1.04; person[numpeople].whichgun=nogun; person[numpeople].aiming=0; person[numpeople].killtarget=-1; person[numpeople].speedmult=0.7+.2*difficulty;}

			else {person[numpeople].whichgun=nogun; person[numpeople].aiming=0; person[numpeople].killtarget=-1;}

			

			if(person[numpeople].type==viptype){person[numpeople].existing=1;}

			

			

			if(enemystate==2)person[numpeople].killtarget=AssassinTarget();

			

			if(filling&&findDistancefast(person[numpeople].playercoords,person[0].playercoords)>900)person[numpeople].existing=1;   //there already when the mission starts

			

			numpeople++;

			citypeoplenum[blockspawnx][blockspawny]++;

			spawndelay=spawndelay+spawnstep<0?spawndelay+spawnstep:spawnstep;

		}

		else if(spawndelay<0&&numpeople>=crowd){   //one or the other a pass

			if(cycle>=numpeople)cycle=0;

			cyclenum=0;

			//move someone who is out past the square, and out of sight, to where its edge is short of people
			while(cyclenum<numpeople&&(person[cycle].type==playertype||person[cycle].type==viptype||(abs(person[cycle].whichblockx-playerblockx)<=peopleradius&&abs(person[cycle].whichblocky-playerblockz)<=peopleradius)||findDistancefast(person[cycle].skeleton.free<1?person[cycle].playercoords:person[cycle].averageloc,person[0].playercoords)<peoplevisible*peoplevisible)){

				cycle++;

				cyclenum++;

				if(cycle>=numpeople)cycle=0;

			}

			if(cycle<numpeople&&cyclenum<numpeople){

				if(type==randomshoot_type){

					/* Anyone can be an assassin, with assassinchance (see above).
				   Zombie missions are all zombies. */

				if((Random()+32767)/65535.f<assassinchance)person[cycle].type=eviltype;

					else person[cycle].type=civiliantype;

				}

				if(type==zombie_type){

					person[cycle].type=zombietype;

				}

				//no assassin (or zombie) starts in the block of the one it is after

				for(int retry=0;retry<16&&person[cycle].type!=civiliantype&&blockspawnx==person[AssassinTarget()].whichblockx&&blockspawny==person[AssassinTarget()].whichblocky;retry++)PickSpawnSpot(filling,peopleradius,peoplevisible,&blockspawnx,&blockspawny,&spawnvertex);

				person[cycle].playerrotation=0;

				

				person[cycle].whichcostume=casualcostumes+abs(Random())%numcasual;

				

				citypeoplenum[person[cycle].whichblockx][person[cycle].whichblocky]--;

				person[cycle].whichblockx=blockspawnx;

				person[cycle].whichblocky=blockspawny;

				

				person[cycle].pathnum=-1;

				person[cycle].oldpathnum=-1;

				person[cycle].oldoldpathnum=-1;

				person[cycle].oldoldoldpathnum=-1;

				person[cycle].pathnum=spawnvertex;   //the corner PickSpawnSpot chose

				person[cycle].pathtarget.x=path.vertex[person[cycle].pathnum].x;

				person[cycle].pathtarget.z=path.vertex[person[cycle].pathnum].z;			

				person[cycle].pathsize=.98+float(abs(Random()%29))/400;   //.98-1.05: overlaps the assassins' 1.04

				person[cycle].crossdelay=civiliancrossdelay*float(abs(Random()%100))/100;

				person[cycle].pathtarget*=person[cycle].pathsize;

				person[cycle].pathtarget.x+=person[cycle].whichblockx*block_spacing;

				person[cycle].pathtarget.z+=person[cycle].whichblocky*block_spacing;

				

				person[cycle].playercoords=person[cycle].pathtarget;

				person[cycle].oldplayercoords=person[cycle].playercoords;

				person[cycle].skeleton.free=0;

				

				person[cycle].targetanimation=walkanim;

				if(person[cycle].type==zombietype)person[cycle].targetanimation=zombiewalkanim;

				person[cycle].speed=1;

				person[cycle].existing=0;

				person[cycle].speedmult=.8+float(abs(Random()%20))/50;

				person[cycle].pace=person[cycle].type==zombietype?0:RandomPace();   //zombies keep their own speeds, which their health drives

				person[cycle].cruising=0;

				person[cycle].health=100;

				person[cycle].maxhealth=100;

				person[cycle].playerrotation2=0;

						

				if(person[cycle].skeleton.broken)person[cycle].skeleton.Load((char *)":Data:Skeleton:Basic Figure");

							

				if(enemystate==1)person[cycle].killtarget=-1;

				

				if(person[cycle].type==eviltype){person[cycle].existing=1;  person[cycle].pathsize=1.04; person[cycle].whichgun=nogun; person[cycle].aiming=1; person[cycle].killtarget=-1; person[cycle].speedmult=1+.3*difficulty;}

				if(person[cycle].type==zombietype){person[cycle].existing=1; person[cycle].pathsize=1.04; person[cycle].whichgun=nogun; person[cycle].aiming=0; person[cycle].killtarget=-1; person[cycle].speedmult=.7+.2*difficulty;}

				else {person[cycle].whichgun=nogun; person[cycle].aiming=0; person[cycle].killtarget=-1;}

			

				person[cycle].lastdistancevictim=200000;

			

				if(enemystate==2)person[cycle].killtarget=AssassinTarget();

			

				citypeoplenum[blockspawnx][blockspawny]++;

				cycle++;

				

				spawndelay=spawndelay+spawnstep<0?spawndelay+spawnstep:spawnstep;

			}

		}}

		if(filling){ if(numpeople>=crowd)break; }

		else if(spawndelay==spawndelaybefore)break;   //no one to spawn or move

		}

		if(filling)spawndelay=spawnstep;

		

		XYZ towards;

		XYZ finaltarget;

		XYZ blah;

		int closesttarget = 0;

		float leastdistance = 0.0;

		

		

		XYZ bleedloc;

		XYZ vel;

		

		float tooclose;

		float toofar;

		

		/* The AI below only ever has people walk when they are going about
		   their business, so anyone jogging at their own pace is handed back
		   to it as walking, and turned back into a jog after it unless it
		   asked for something else (see RandomPace). */
		for(int i=1;i<numpeople;i++)if(person[i].cruising){

			if(person[i].targetanimation==joganim)person[i].targetanimation=walkanim;

		}

		//People

		for(int i=0;i<numpeople;i++){

			if(person[i].health<0&&person[i].longdead<0&&person[i].firstlongdead==0){

				if(person[i].type!=zombietype){

					overpoint=person[i].skeleton.joints[person[i].skeleton.jointlabels[abdomen]].position;

					overpoint.y+=3000;

					underpoint=person[i].skeleton.joints[person[i].skeleton.jointlabels[abdomen]].position;

					underpoint.y-=3000;

					move=0;

					move.x=person[i].whichblockx*block_spacing;

					move.z=person[i].whichblocky*block_spacing;

					XYZ temp;

					whichtri=sidewalkcollide.LineCheck2(overpoint,underpoint,&temp,move,cityrotation[person[i].whichblockx][person[i].whichblocky]*90);

					XYZ normish;

					normish=0;

					normish.y=1;

					if(whichtri>=0&&blood){

						decals.MakeDecal(bloodpool,temp,12,normish, whichtri, &sidewalkcollide, move, cityrotation[person[i].whichblockx][person[i].whichblocky]*90);

					}

					if(whichtri==-1&&blood){

						temp=person[i].skeleton.joints[person[i].skeleton.jointlabels[abdomen]].position;

						temp.y=-.5;

						move=0;

						decals.MakeDecal(bloodpool,temp,12,normish, 0, &sidewalkcollide, move, 0);

					}

					person[i].firstlongdead=1;

				}

			}

			if(person[i].health<100&&person[i].type!=zombietype){

			 		person[i].health-=multiplier*120;

			}

			if(person[i].health<100&&person[i].type==zombietype){

			 		person[i].health+=multiplier*10;

			 		if(person[i].health>person[i].maxhealth)person[i].health=person[i].maxhealth;

			}

			if(person[i].health<100&&person[i].type==zombietype&&person[i].skeleton.free==1){

			 		person[i].health+=multiplier*10;

			 		if(person[i].health>person[i].maxhealth)person[i].health=person[i].maxhealth;

			}

			if(person[i].health<40&&person[i].type==zombietype){

			 		person[i].speedmult-=multiplier/20;

			 		if(person[i].speedmult<.6){

						person[i].speedmult=.6;

						person[i].killtarget=-1;

					}

			}

			if(person[i].health>=40&&person[i].type==zombietype){

			 		person[i].speedmult+=multiplier/40;

			 		if(person[i].speedmult>.7+difficulty*.2){

						person[i].speedmult=.7+difficulty*.2;

						person[i].killtarget=AssassinTarget();

					}

			}

			

			if(person[i].maxhealth<100&&person[i].type==zombietype){

			 		person[i].maxhealth+=multiplier*2;

			 		if(person[i].maxhealth>100)person[i].maxhealth=100;

			}

			if(person[i].bleeding>0){

				person[i].bleeding-=multiplier;

				person[i].bleeddelay-=multiplier*10;

				if(person[i].bleeddelay<=0){

					person[i].bleeddelay=1;

					if(person[i].skeleton.free==0){

						bleedloc=DoRotation((person[i].bjoint1->position+person[i].bjoint2->position)/2,0,person[i].playerrotation,0)+person[i].playercoords;

					}

					if(person[i].skeleton.free>0){

						bleedloc=(person[i].bjoint1->position+person[i].bjoint2->position)/2;

					}

					vel=0;

					if(blood)sprites.MakeSprite(bloodspritedown, .6, 1, .2, .2,bleedloc, vel, 3*person[i].bleeding);

				}

			}

			if(person[i].skeleton.free==0){

				//Gun

				if(person[i].type==playertype||person[i].type==eviltype){

					if(i==0){

						if(person[i].whichgun==shotgun)person[i].recoil-=multiplier*4;

					 	if(person[i].whichgun==sniperrifle)person[i].recoil-=multiplier*2;

					 	if(person[i].whichgun==handgun1)person[i].recoil-=multiplier*5;

					 	if(person[i].whichgun==handgun2)person[i].recoil-=multiplier*5;

					 	if(person[i].whichgun==assaultrifle)person[i].recoil-=multiplier*10;

				 	}

				 	if(i!=0){

						if(person[i].whichgun==shotgun)person[i].recoil-=multiplier*1;

					 	if(person[i].whichgun==sniperrifle)person[i].recoil-=multiplier*1;

					 	if(person[i].whichgun==handgun1)person[i].recoil-=multiplier*2;

					 	if(person[i].whichgun==handgun2)person[i].recoil-=multiplier*2;

					 	if(person[i].whichgun==assaultrifle)person[i].recoil-=multiplier*10;

				 	}

				 	if(person[i].recoil<0)person[i].recoil=0;

					if(i==0){

						/* The sniper scope is up while you crouch and aim the rifle, and
						   not in soul mode.  From z33ky's fixes in Black Shades Enhanced:
						   going in turns the view 14 down and 9 left (the original's 6
						   down missed), so the scope looks where the rifle shoots, and
						   coming out turns it back (UnZoom).  Only real transitions
						   correct the aim, so it cannot creep. */
						const bool scoped=person[i].currentanimation==crouchanim&&person[i].targetanimation==crouchanim&&person[i].aiming>=1&&person[i].whichgun==sniperrifle&&visions==0;

						if(zoom&&!scoped)UnZoom();

						if(!zoom&&scoped){

							zoom=1;

							camera.rotation2-=14;

							camera.rotation-=9;

						}

						if(zoom)mousesensitivity=.05*usermousesensitivity;

						if(!zoom)mousesensitivity=1*usermousesensitivity;

					}

				}

				//Zombie eat

				if(i>0&&person[i].killtarget>-1&&person[person[i].killtarget].eaten==i&&person[i].targetanimation==zombieeatanim){

			 		person[person[i].killtarget].skeleton.joints[person[person[i].killtarget].skeleton.jointlabels[head]].locked=1;

			 		person[person[i].killtarget].skeleton.joints[person[person[i].killtarget].skeleton.jointlabels[rightshoulder]].locked=1;

				 	for(int k=0;k<2;k++){

			 			person[person[i].killtarget].skeleton.joints[person[person[i].killtarget].skeleton.jointlabels[head]].position=DoRotation(person[i].skeleton.joints[person[i].skeleton.jointlabels[righthand]].position,0,person[i].playerrotation,0)+person[i].playercoords;

				 		person[person[i].killtarget].skeleton.joints[person[person[i].killtarget].skeleton.jointlabels[head]].velocity=0;

				 		person[person[i].killtarget].skeleton.joints[person[person[i].killtarget].skeleton.jointlabels[rightshoulder]].position=DoRotation(person[i].skeleton.joints[person[i].skeleton.jointlabels[lefthand]].position,0,person[i].playerrotation,0)+person[i].playercoords;

				 		person[person[i].killtarget].skeleton.joints[person[person[i].killtarget].skeleton.jointlabels[rightshoulder]].velocity=0;

				 		person[person[i].killtarget].skeleton.DoConstraints();

						person[person[i].killtarget].skeleton.DoConstraints(&blocksimplecollide[citytype[person[i].whichblockx][person[i].whichblocky]],&move,cityrotation[person[i].whichblockx][person[i].whichblocky]*90);

					}

					person[person[i].killtarget].skeleton.joints[person[person[i].killtarget].skeleton.jointlabels[head]].locked=0;

			 		person[person[i].killtarget].skeleton.joints[person[person[i].killtarget].skeleton.jointlabels[rightshoulder]].locked=0;

				 		

			 		person[person[i].killtarget].longdead=1;

			 	}

				

				if(i>0&&enemystate!=1&&person[i].type==zombietype&&person[i].speedmult>.7){

					if(findDistancefast(person[i].playercoords,person[AssassinTarget()].playercoords)<20000)person[i].killtarget=AssassinTarget();

					else person[i].killtarget=-1;

				}

				if(i>0&&enemystate!=1&&person[i].type==zombietype&&person[i].speedmult<.7){

					person[i].killtarget=-1;

				}

				

				bool realcheck = false;

				//Pathfinding
			 	if(i>0&&person[i].targetanimation!=getupfrontanim&&person[i].targetanimation!=thrownanim&&person[i].targetanimation!=getupbackanim&&person[i].currentanimation!=getupfrontanim&&person[i].currentanimation!=getupbackanim){

			 		person[i].pathcheckdelay-=multiplier;

			 		person[i].crossdelay-=multiplier;

					// Realcheck tells us
					// a) we've got close to the end of our path or
					// b) we're moving away from our target
			 		realcheck=(abs(person[i].playercoords.x-person[i].pathtarget.x)<1&&abs(person[i].playercoords.z-person[i].pathtarget.z)<1)
					  ||findDistancefast(person[i].playercoords,person[i].pathtarget)>findDistancefast(person[i].oldplayercoords,person[i].pathtarget);
			 		
					if(person[i].targetanimation==idleanim&&person[i].killtargetvisible==0){

						person[i].targetanimation=walkanim;

						if(person[i].type==zombietype)person[i].targetanimation=zombiewalkanim;

						realcheck=1;

					}

					if((realcheck||((person[i].killtarget>-1&&person[i].type!=civiliantype)&&person[i].pathcheckdelay<=0)||person[i].killtargetvisible)){

				 		person[i].pathcheckdelay=1.2;

				 		if((person[i].killtarget==-1||person[i].type==civiliantype)&&!person[i].running){

					 		person[i].killtargetvisible=0;

					 		leastdistance=2000000;

					 		for(int j=0;j<path.vertexNum;j++){

					 			person[i].pathtarget.x=path.vertex[j].x;

								person[i].pathtarget.z=path.vertex[j].z;		

								person[i].pathtarget.y=path.vertex[j].y;		

								person[i].pathtarget*=person[i].pathsize;

								person[i].pathtarget.x+=person[i].whichblockx*block_spacing;

								person[i].pathtarget.z+=person[i].whichblocky*block_spacing;

								if(findDistancefast(person[i].playercoords,person[i].pathtarget)<leastdistance&&j!=1&&j!=person[i].oldpathnum&&j!=person[i].oldoldpathnum&&j!=person[i].oldoldoldpathnum){

									leastdistance=findDistancefast(person[i].playercoords,person[i].pathtarget);

									closesttarget=j;

								}	

					 		}

					 		if(closesttarget>=0&&closesttarget<path.vertexNum){

						 		person[i].oldoldoldpathnum=person[i].oldoldpathnum;

						 		person[i].oldoldpathnum=person[i].oldpathnum;

						 		person[i].oldpathnum=person[i].pathnum;

						 		person[i].pathnum=closesttarget;

								person[i].pathtarget.x=path.vertex[person[i].pathnum].x;

								person[i].pathtarget.z=path.vertex[person[i].pathnum].z;		

								person[i].pathtarget*=person[i].pathsize;

								person[i].pathtarget.x+=person[i].whichblockx*block_spacing;

								person[i].pathtarget.z+=person[i].whichblocky*block_spacing;

							}

							/* Now and then a civilian crosses the street instead of
							   carrying on round the same block (civiliancrossdelay in
							   Constants.h).  When their timer is up they head for the
							   nearest corner of one of the four neighbouring blocks that
							   they can reach in a straight line without walking through
							   either building; if there is none from where they stand,
							   they try again at the next waypoint. */

							if(person[i].type==civiliantype){

								if(person[i].crossdelay<=0){

									const int crossdirx[4]={1,-1,0,0};

									const int crossdirz[4]={0,0,1,-1};

									int fromx=person[i].whichblockx;

									int fromz=person[i].whichblocky;

									int crossx=-1;

									int crossz=-1;

									int crosspath=-1;

									float crossdistance=2000000;

									XYZ crosstarget;

									XYZ candidate;

									XYZ frommove;

									XYZ tomove;

									XYZ crosshit;

									frommove=0;

									frommove.x=fromx*block_spacing;

									frommove.z=fromz*block_spacing;

									frommove.y=-3;   //as the visibility test does

									for(int dir=0;dir<4;dir++){

										int tox=fromx+crossdirx[dir];

										int toz=fromz+crossdirz[dir];

										if(tox<1||toz<1||tox>num_blocks-1||toz>num_blocks-1)continue;

										tomove=0;

										tomove.x=tox*block_spacing;

										tomove.z=toz*block_spacing;

										tomove.y=-3;

										for(int v=0;v<path.vertexNum;v++){

											if(v==1)continue;   //vertex 1 is the middle of the block

											candidate=path.vertex[v]*person[i].pathsize;

											candidate.x+=tox*block_spacing;

											candidate.z+=toz*block_spacing;

											float candidatedistance=findDistancefast(person[i].playercoords,candidate);

											if(candidatedistance<crossdistance

												&&blocksimple.LineCheck2(person[i].playercoords,candidate,&crosshit,frommove,cityrotation[fromx][fromz]*90)==-1

												&&blocksimple.LineCheck2(person[i].playercoords,candidate,&crosshit,tomove,cityrotation[tox][toz]*90)==-1){

												crossdistance=candidatedistance;

												crossx=tox;

												crossz=toz;

												crosspath=v;

												crosstarget=candidate;

											}

										}

									}

									if(crosspath!=-1){

										MovePersonToBlock(i,crossx,crossz);

										person[i].oldoldoldpathnum=-1;

										person[i].oldoldpathnum=-1;

										person[i].oldpathnum=-1;

										person[i].pathnum=crosspath;

										person[i].pathtarget=crosstarget;

										person[i].crossdelay=civiliancrossdelay*(.5+float(abs(Random()%100))/100);

									}

								}

							}

						}

						if(person[i].running&&realcheck){

							person[i].killtargetvisible=0;

							person[i].targetanimation=joganim;

							//Dead target?

							if(person[i].killtarget>-1&&person[person[i].killtarget].health<=0){

							 	person[i].running=0;

							 }

							 person[i].killtarget=AssassinTarget();

							 

							 //If pathfind

							if(realcheck){

					 		leastdistance=2000000;

					 		person[i].lastdistancevictim=0;

					 		closesttarget=-1;

					 		//Check best path

					 		closesttarget=person[i].pathnum;

					 		//Check other blocks?

					 		if ((closesttarget==person[i].pathnum)){

					 			beginx=person[i].whichblockx-2;

								if(beginx<0)beginx=0;

								beginz=person[i].whichblocky-2;

								if(beginz<0)beginz=0;

								

								endx=person[i].whichblockx+2;

								if(endx>num_blocks-1)endx=num_blocks-1;

								endz=person[i].whichblocky+2;

								if(endz>num_blocks-1)endz=num_blocks-1;

								

								leastdistance=2000000;

					 			for(int l=beginx;l<=endx;l++){

									for(int m=beginz;m<=endz;m++){   //was beginx..endx: searched the wrong rows

								 		for(int j=0;j<path.vertexNum;j++){

								 			person[i].pathtarget.x=path.vertex[j].x;

											person[i].pathtarget.y=path.vertex[j].y;

											person[i].pathtarget.z=path.vertex[j].z;		

											person[i].pathtarget*=person[i].pathsize;

											person[i].pathtarget.x+=l*block_spacing;

											person[i].pathtarget.z+=m*block_spacing;

											if(findDistancefast(person[i].playercoords,person[i].pathtarget)<leastdistance&&findDistancefast(person[i].pathtarget,person[person[i].killtarget].playercoords)>findDistancefast(person[i].playercoords,person[person[i].killtarget].playercoords)&&j!=1&&blocksimple.LineCheck2(person[i].playercoords,person[i].pathtarget,&blah,move,cityrotation[person[i].whichblockx][person[i].whichblocky])==-1&&blocksimple.LineCheck2(person[i].playercoords,person[i].pathtarget,&blah,move,cityrotation[l][m])==-1){

												person[i].lastdistancevictim=findDistancefast(person[i].pathtarget,person[person[i].killtarget].playercoords);

												leastdistance=findDistancefast(person[i].playercoords,person[i].pathtarget);

												closesttarget=j;

												finaltarget=person[i].pathtarget;

												MovePersonToBlock(i,l,m);

					 						}

								 		}

								 		}

								 	}

								 }

					  		if(closesttarget!=-1){

								person[i].pathnum=closesttarget;

								person[i].pathtarget=finaltarget;

							}

							}

						}

						//Assassin

						if((person[i].killtarget>-1&&person[i].type!=civiliantype)&&!person[i].running){

							//Dead target?

							if(person[person[i].killtarget].health<=0&&person[i].type==eviltype){

							 	person[i].playerrotation2=0;

							 	person[i].whichgun=nogun;

							 	person[i].targetanimation=walkanim;

							 	person[i].lastdistancevictim=200000;

							 	person[i].pathnum=-1;

							 	enemystate=1;

							 	person[i].killtarget=-1;
								person[i].killtargetvisible=0;

							 	realcheck=1;

							 }

							 if(person[i].type==zombietype&&person[i].killtarget>-1&&person[person[i].killtarget].health<=0){

								if(person[person[i].killtarget].eaten!=i){

									person[i].playerrotation2=0;

							 		person[i].targetanimation=zombiewalkanim;

							 		person[i].lastdistancevictim=200000;

							 		person[i].pathnum=-1;

							 		realcheck=1;

							 		person[i].killtarget=-1;
									person[i].killtargetvisible=0;

							 	}

							 	if(person[i].killtarget>-1&&person[person[i].killtarget].eaten==i&&person[i].targetanimation!=zombieeatanim){

							 		person[i].targetanimation=zombieeatanim;

									person[i].targetframe=0;

									person[i].target=0;

							 	}

							 	enemystate=1;

							 }

							 if(person[i].killtarget>-1&&person[person[i].killtarget].health>0){

							 if(person[person[i].killtarget].skeleton.free){

							 	person[person[i].killtarget].playercoords=person[person[i].killtarget].averageloc;

							 }

							 

							 //If pathfind

							if(realcheck){

					 		leastdistance=2000000;

					 		person[i].lastdistancevictim=2000000;

					 		closesttarget=-1;

					 		//Check best path

					 		for(int j=0;j<path.vertexNum;j++){

					 			person[i].pathtarget.x=path.vertex[j].x;

								person[i].pathtarget.z=path.vertex[j].z;		

								person[i].pathtarget.y=path.vertex[j].y;		

								person[i].pathtarget*=person[i].pathsize;

								person[i].pathtarget.x+=person[i].whichblockx*block_spacing;

								person[i].pathtarget.z+=person[i].whichblocky*block_spacing;

								if(findDistancefast(person[i].playercoords,person[i].pathtarget)<leastdistance&&findDistancefast(person[i].pathtarget,person[person[i].killtarget].playercoords)<person[i].lastdistancevictim&&j!=1&&blocksimple.LineCheck2(person[i].playercoords,person[i].pathtarget,&blah,move,cityrotation[person[i].whichblockx][person[i].whichblocky])==-1){

									leastdistance=findDistancefast(person[i].playercoords,person[i].pathtarget);

									person[i].lastdistancevictim=findDistancefast(person[i].pathtarget,person[person[i].killtarget].playercoords);

									closesttarget=j;

									finaltarget=person[i].pathtarget;

								}	

					 		}

					 		leastdistance=2000000;

					 		for(int j=0;j<path.vertexNum;j++){

					 			person[i].pathtarget.x=path.vertex[j].x;

								person[i].pathtarget.z=path.vertex[j].z;		

								person[i].pathtarget.y=path.vertex[j].y;		

								person[i].pathtarget*=person[i].pathsize;

								person[i].pathtarget.x+=person[i].whichblockx*block_spacing;

								person[i].pathtarget.z+=person[i].whichblocky*block_spacing;

								if(findDistancefast(person[i].playercoords,person[i].pathtarget)<leastdistance&&findDistancefast(person[i].pathtarget,person[person[i].killtarget].playercoords)<person[i].lastdistancevictim&&j!=1&&blocksimple.LineCheck2(person[i].playercoords,person[i].pathtarget,&blah,move,cityrotation[person[i].whichblockx][person[i].whichblocky])==-1){

									leastdistance=findDistancefast(person[i].playercoords,person[i].pathtarget);

									person[i].lastdistancevictim=findDistancefast(person[i].pathtarget,person[person[i].killtarget].playercoords);

									closesttarget=j;

									finaltarget=person[i].pathtarget;

								}	

					 		}

					 		//Check other blocks?

					 		if((closesttarget==person[i].pathnum)||closesttarget==-1){

					 			beginx=person[i].whichblockx-2;

								if(beginx<0)beginx=0;

								beginz=person[i].whichblocky-2;

								if(beginz<0)beginz=0;

								

								endx=person[i].whichblockx+2;

								if(endx>num_blocks-1)endx=num_blocks-1;

								endz=person[i].whichblocky+2;

								if(endz>num_blocks-1)endz=num_blocks-1;

								

								leastdistance=2000000;

					 			for(int l=beginx;l<=endx;l++){

									for(int m=beginz;m<=endz;m++){   //was beginx..endx: searched the wrong rows

								 		if(l!=person[i].whichblockx||m!=person[i].whichblocky){

								 		for(int j=0;j<path.vertexNum;j++){

								 			person[i].pathtarget.x=path.vertex[j].x;

											person[i].pathtarget.y=path.vertex[j].y;

											person[i].pathtarget.z=path.vertex[j].z;		

											person[i].pathtarget*=person[i].pathsize;

											person[i].pathtarget.x+=l*block_spacing;

											person[i].pathtarget.z+=m*block_spacing;

											if(findDistancefast(person[i].playercoords,person[i].pathtarget)<leastdistance&&findDistancefast(person[i].pathtarget,person[person[i].killtarget].playercoords)<findDistancefast(person[i].playercoords,person[person[i].killtarget].playercoords)&&j!=1&&blocksimple.LineCheck2(person[i].playercoords,person[i].pathtarget,&blah,move,cityrotation[l][m])==-1&&blocksimple.LineCheck2(person[i].playercoords,person[i].pathtarget,&blah,move,cityrotation[person[i].whichblockx][person[i].whichblocky])==-1){

												leastdistance=findDistancefast(person[i].playercoords,person[i].pathtarget);

												closesttarget=j;

												finaltarget=person[i].pathtarget;

												MovePersonToBlock(i,l,m);

					 						}

								 		}

								 		}

								 	}

								 }

					 		}

					 		if(closesttarget!=-1){

					 			person[i].onpath=1;

								person[i].pathnum=closesttarget;

								person[i].pathtarget=finaltarget;

							}

							}

							//Check killtargetvisible

							person[i].killtargetvisible=1;

							if(person[person[i].killtarget].health<=0)person[i].killtargetvisible=0;

							if(closesttarget!=-1&&findDistancefast(person[i].playercoords, person[person[i].killtarget].playercoords)>30000)person[i].killtargetvisible=0;

					 		if(person[i].killtarget==0&&visions==1)person[i].killtargetvisible=0;

					 		if(person[i].killtargetvisible){

						 		beginx=person[i].whichblockx-2;

								if(beginx<0)beginx=0;

								beginz=person[i].whichblocky-2;

								if(beginz<0)beginz=0;

								

								endx=person[i].whichblockx+2;

								if(endx>num_blocks-1)endx=num_blocks-1;

								endz=person[i].whichblocky+2;

								if(endz>num_blocks-1)endz=num_blocks-1;

								

						 		for(int l=beginx;l<=endx;l++){

									for(int m=beginz;m<=endz;m++){   //was beginx..endx: searched the wrong rows

								 		move.x=l*block_spacing;

										move.z=m*block_spacing;

										move.y=-3;

										if(person[i].killtargetvisible){

									 		if(blocksimple.LineCheck2(person[i].playercoords,person[person[i].killtarget].playercoords,&blah,move,cityrotation[l][m])!=-1)

				 							{

				 								person[i].killtargetvisible=0;

				 							}

				 						}

								 	}

								 }

								}

								if(person[i].type==eviltype){

									 if(!person[i].killtargetvisible&&person[i].targetanimation==idleanim){

									 	person[i].targetanimation=joganim;

									 }

									 if(!person[i].killtargetvisible){

									 	person[i].aiming=0;

									 }

									 if(person[i].killtargetvisible){

									 	person[i].onpath=0;

									 	person[i].lastdistancevictim=200000;

									 	person[i].pathnum=-1;

									 	if(person[i].whichgun==nogun){

									 		if(numpossibleguns>0)person[i].whichgun=possiblegun[abs(Random()%numpossibleguns)];

									 		person[i].reloads[person[i].whichgun]=1;

									 		if(person[i].whichgun==knife)person[i].speedmult=.8+.5*difficulty;

									 	}

									 	if(person[i].aiming==0)person[i].shotdelay=shotdelayamount/difficulty;

									 	person[i].aiming=1;

									 	if(person[i].reloading>0)person[i].aiming=0;

									 	if(person[i].whichgun==handgun1||person[i].whichgun==handgun2)person[i].playerrotation2=-10;

									 	if(person[i].whichgun==assaultrifle||person[i].whichgun==sniperrifle||person[i].whichgun==shotgun)person[i].playerrotation2=20;

									 	tooclose=1300;

									 	toofar=3000;

									 	if(person[i].whichgun==shotgun){

									 		tooclose=1400;

									 		toofar=5000;

									 	}

									 	if(person[i].whichgun==assaultrifle){

									 		tooclose=5000;

									 		toofar=9000;

									 	}

									 	if(person[i].whichgun==sniperrifle){

									 		tooclose=10000;

									 		toofar=20000;

									 	}

									 	if(person[i].whichgun==knife){

									 		tooclose=20;

									 		toofar=20000;

									 	}

									 	if(findDistancefast(person[i].playercoords, person[person[i].killtarget].playercoords)>toofar)

										 	person[i].targetanimation=joganim;

									 	if((findDistancefast(person[i].playercoords, person[person[i].killtarget].playercoords)<=tooclose&&person[person[i].killtarget].skeleton.free==0)||(tooclose>200&&findDistancefast(person[i].playercoords, person[person[i].killtarget].playercoords)<=200)||(tooclose<=200&&findDistancefast(person[i].playercoords, person[person[i].killtarget].playercoords)<tooclose)){

									 		if(person[i].targetanimation!=idleanim){

										 		person[i].targetanimation=idleanim;

												person[i].targetframe=0;

												person[i].target=0;

											}

											if(person[i].whichgun==knife&&person[person[i].killtarget].health==100){

												murderer=i;

												person[i].attacktarget=0;

												person[i].attackframe=0;

												float gLoc[3];

												gLoc[0]=(camera.position.x+((person[i].playercoords.x+flatfacing.x)-camera.position.x)/2)/soundscalefactor;

												gLoc[1]=(camera.position.y+((person[i].playercoords.y+flatfacing.y)-camera.position.y)/2)/soundscalefactor;

												gLoc[2]=(camera.position.z+((person[i].playercoords.z+flatfacing.z)-camera.position.z)/2)/soundscalefactor;

												if(person[person[i].killtarget].type!=zombietype)

												{

													alSourcefv(gSourceID[knifeslashsound], AL_POSITION, gLoc);

													alSourcePlay(gSourceID[knifeslashsound]);

													person[person[i].killtarget].bjoint1=&person[person[i].killtarget].skeleton.joints[person[person[i].killtarget].skeleton.jointlabels[neck]];

													person[person[i].killtarget].bjoint2=&person[person[i].killtarget].skeleton.joints[person[person[i].killtarget].skeleton.jointlabels[neck]];

													person[person[i].killtarget].bleeding=1;

													person[person[i].killtarget].bleeddelay=1;

													person[person[i].killtarget].health-=20;

													person[person[i].killtarget].targetanimation=chestpainanim;

													person[person[i].killtarget].targetframe=0;

													person[person[i].killtarget].target=0;

													person[person[i].killtarget].longdead=1;

												}

											}

										}

									 	finaltarget=person[person[i].killtarget].playercoords;

									 }

								}

								if(person[i].type==zombietype&&person[person[i].killtarget].health>0){

									 if(!person[i].killtargetvisible&&person[i].targetanimation==idleanim){

									 	person[i].targetanimation=zombiejoganim;

									 }

									 if(!person[i].killtargetvisible){

									 	person[i].aiming=0;

									 }

									 if(person[i].killtargetvisible){

									 	person[i].onpath=0;

									 	person[i].lastdistancevictim=200000;

									 	person[i].pathnum=-1;

									 	if(person[i].aiming==0)person[i].shotdelay=shotdelayamount/difficulty;

									 	if(findDistancefast(person[i].playercoords, person[person[i].killtarget].playercoords)>20||person[i].targetanimation!=idleanim)

										 	person[i].targetanimation=zombiejoganim;

									 	if(findDistancefast(person[i].playercoords, person[person[i].killtarget].playercoords)<=20){

									 		murderer=i;

									 		person[person[i].killtarget].health=0;

									 		person[person[i].killtarget].eaten=i;

										}

									 	finaltarget=person[person[i].killtarget].playercoords;

									 }

								}

							 if(person[i].killtargetvisible||realcheck)person[i].pathtarget=finaltarget;

							 if(realcheck)person[i].lastdistancevictim=findDistancefast(person[i].pathtarget,person[person[i].killtarget].playercoords);	 

							}

						}

						

						if(person[i].targetanimation!=zombieeatanim||person[i].type!=zombietype){

							towards=person[i].playercoords-person[i].pathtarget;

							Normalise(&towards);

							person[i].playerrotation=asin(0-towards.x)*360/6.28;

							if(towards.z>0)person[i].playerrotation=180-person[i].playerrotation;

						}

					}

			 	}

			 	MovePersonToBlock(i,CityBlock(person[i].playercoords.x),CityBlock(person[i].playercoords.z));

			 					

				if(!person[i].onground)person[i].velocity.y+=multiplier*gravity;

			 	if(!person[i].onground&&(i!=0||visions!=1))person[i].playercoords+=person[i].velocity*multiplier;

			 	//Death by bleeding/shock

		 		if(person[i].health<=0){

		 			person[i].skeleton.offsetted=0;

					person[i].skeleton.free=1;

					person[i].longdead=1;

					for(int j=0;j<person[i].skeleton.num_joints;j++){

						person[i].skeleton.joints[j].position+=person[i].skeleton.joints[j].offset;

						person[i].skeleton.joints[j].position=DoRotation(person[i].skeleton.joints[j].position,0,person[i].playerrotation,0);

						person[i].skeleton.joints[j].position+=person[i].playercoords;

						person[i].skeleton.joints[j].realoldposition=person[i].skeleton.joints[j].position;

						person[i].skeleton.joints[j].velocity=DoRotation(person[i].skeleton.joints[j].velocity,0,person[i].playerrotation,0);

						person[i].skeleton.joints[j].velocity+=person[i].velocity;

						person[i].skeleton.joints[j].velocity+=person[i].facing*4;

					}

				}

			 }

		 	//Rag doll

			if(person[i].skeleton.free==1&&person[i].longdead>0){

				MovePersonToBlock(i,CityBlock(person[i].skeleton.joints[0].position.x),CityBlock(person[i].skeleton.joints[0].position.z));

				move=0;

				move.x=person[i].whichblockx*block_spacing;

				move.z=person[i].whichblocky*block_spacing;

				person[i].skeleton.DoGravity();

				if(person[i].averageloc.y<=50)person[i].skeleton.DoConstraints(&blocksimplecollide[citytype[person[i].whichblockx][person[i].whichblocky]],&move,cityrotation[person[i].whichblockx][person[i].whichblocky]*90);

				if(person[i].averageloc.y>50)person[i].skeleton.DoConstraints(&blockcollide[citytype[person[i].whichblockx][person[i].whichblocky]],&move,cityrotation[person[i].whichblockx][person[i].whichblocky]*90);

				person[i].oldaverageloc=person[i].averageloc;

				person[i].averageloc=0;

				for(int j=0;j<person[i].skeleton.num_joints;j++){

					person[i].averageloc+=person[i].skeleton.joints[j].position;

				}

				person[i].averageloc/=person[i].skeleton.num_joints;

				person[i].playercoords=person[i].averageloc;

				if(person[i].longdead<multiplier/2&&person[i].longdead>0)person[i].DrawSkeleton(i);

				if(findDistancefast(person[i].averageloc,person[i].oldaverageloc)<.2*multiplier)person[i].longdead-=multiplier/2;

			}

			if(person[i].skeleton.free==1&&person[i].longdead<=0&&person[i].health>0&&person[i].longdead!=-1){

				person[i].longdead=1;

				person[i].skeleton.free=0;

				person[i].currentanimation=lyinganim;

				person[i].target=0;

				person[i].targetframe=0;

				//Get up from front or back?

				if(person[i].skeleton.forward.y>0)

				person[i].targetanimation=getupfrontanim;

				else

				person[i].targetanimation=getupbackanim;

				//Find playercoords

				person[i].playercoords=person[i].averageloc;

				for(int j=0;j<person[i].skeleton.num_joints;j++){

					if(person[i].skeleton.joints[j].position.y>person[i].playercoords.y)person[i].playercoords.y=person[i].skeleton.joints[j].position.y;

				}

				//Find orientation

				XYZ firsttop=person[i].skeleton.joints[person[i].skeleton.jointlabels[neck]].position-person[i].skeleton.joints[person[i].skeleton.jointlabels[groin]].position;

			 	Normalise(&firsttop);

				person[i].playerrotation=acos(0-firsttop.z);

				person[i].playerrotation*=360/6.28;

				if(0>firsttop.x)person[i].playerrotation=360-person[i].playerrotation;

				person[i].playerrotation*=-1;

				person[i].playervelocity=0;

				if(person[i].targetanimation==getupfrontanim)person[i].playerrotation+=180;

				for(int j=0;j<person[i].skeleton.num_joints;j++){

					person[i].tempanimation.position[j][0]=person[i].skeleton.joints[j].position-person[i].playercoords;

					person[i].tempanimation.position[j][0]=DoRotation(person[i].tempanimation.position[j][0],0,-person[i].playerrotation,0);

				}

			}

		}

		

		for(int i=1;i<numpeople;i++){

			person[i].cruising=0;

			if(person[i].pace>cruise_jog_pace&&person[i].skeleton.free<1){

				if(person[i].targetanimation==walkanim){person[i].targetanimation=joganim; person[i].cruising=1;}

			}

		}

		//Grenade

		if(Button()&&person[0].whichgun==grenade&&person[0].ammo>0&&person[0].reloading<=0&&person[0].attackframe<0&&person[0].targetanimation!=crouchanim){

			if(person[0].grenphase==0){

				XYZ soundsource=DoRotation(person[0].skeleton.joints[person[0].skeleton.jointlabels[righthand]].position,0,person[0].playerrotation,0)+person[0].playercoords;

				float gLoc[3];

				gLoc[0]=soundsource.x/soundscalefactor;

				gLoc[1]=soundsource.y/soundscalefactor;

				gLoc[2]=soundsource.z/soundscalefactor;

				alSourcefv(gSourceID[pinpullsound], AL_POSITION, gLoc);

				alSourcePlay(gSourceID[pinpullsound]);

				person[0].grenphase=1;

			}

		}

		if((!Button())&&person[0].whichgun==grenade){

			if(person[0].grenphase==1){

				person[0].grenphase=0;

				person[0].attackframe=0;

				person[0].attacktarget=0;

				person[0].killtarget=0;

			}

		}

		if(person[0].targetanimation==crouchanim){

			if(person[0].grenphase==1){

				person[0].grenphase=0;

				XYZ soundsource=DoRotation(person[0].skeleton.joints[person[0].skeleton.jointlabels[righthand]].position,0,person[0].playerrotation,0)+person[0].playercoords;

				float gLoc[3];

				gLoc[0]=soundsource.x/soundscalefactor;

				gLoc[1]=soundsource.y/soundscalefactor;

				gLoc[2]=soundsource.z/soundscalefactor;

				alSourcefv(gSourceID[pinreplacesound], AL_POSITION, gLoc);

				alSourcePlay(gSourceID[pinreplacesound]);

			}

		}

		

		//Get gun

		int temp;

		int temp2;

		bool switched=0;

		if(Button()&&!oldbutton&&(person[0].aiming==0||person[0].whichgun==grenade||person[0].whichgun==nogun||person[0].whichgun==knife)&&person[0].currentanimation==crouchanim){

			for(int i=0;i<numpeople;i++){

				if(!switched&&person[i].skeleton.free==1&&findDistancefast(person[0].playercoords,person[i].averageloc)<200){

					float gLoc[3];

					gLoc[0]=person[0].playercoords.x/soundscalefactor;

					gLoc[1]=person[0].playercoords.y/soundscalefactor;

					gLoc[2]=person[0].playercoords.z/soundscalefactor;



					alSourcefv(gSourceID[clicksound], AL_POSITION, gLoc);

					alSourcePlay(gSourceID[clicksound]);



					temp=person[0].whichgun;

					temp2=person[0].ammo;

					person[0].whichgun=person[i].whichgun;

					person[0].ammo=person[i].ammo;

					person[i].whichgun=temp;

					person[i].ammo=temp2;

					person[0].aiming=1;

					person[0].aimamount=0;

					

					switched=1;

				}

			}

		}

		

		//Throw

		if(Button()&&person[0].attackframe<0&&((person[0].whichgun==nogun||person[0].aiming==0)&&person[0].whichgun!=knife)&&person[0].currentanimation!=crouchanim&&person[0].targetanimation!=crouchanim&&person[0].targetanimation!=throwanim&&visions==0){

			if(person[0].targetanimation==idleanim||person[0].targetanimation==walkanim){

				bool attacking=0;

				person[0].killtarget=-1;

				float closedistance=-1;

				for(int i=1;i<numpeople;i++){				

					if(person[i].skeleton.free<1&&(person[i].whichgun!=nogun)&&findDistancefast(person[i].playercoords,person[0].playercoords+flatfacing)<12){

						attacking=1;

						if(person[0].killtarget==-1||findDistancefast(person[i].playercoords,person[0].playercoords)<closedistance){

							person[0].killtarget=i;

							closedistance=findDistancefast(person[i].playercoords,person[0].playercoords);

						}

					}

				}

				if(attacking){

					score+=150;

					person[0].aiming=1;

					person[0].whichgun=person[person[0].killtarget].whichgun;

					person[0].ammo=person[person[0].killtarget].ammo;

					person[person[0].killtarget].whichgun=nogun;

					person[person[0].killtarget].killtarget=-1;

					person[0].targetframe=0;

					person[0].targetanimation=throwanim;

					person[0].target=1;

					person[0].speed=1;

					person[person[0].killtarget].targetframe=0;

					person[person[0].killtarget].targetanimation=thrownanim;

					person[person[0].killtarget].target=1;

					person[person[0].killtarget].playercoords=person[0].playercoords;

					person[person[0].killtarget].playerrotation=person[0].playerrotation;

					person[person[0].killtarget].speed=person[0].speed;

					person[person[0].killtarget].speedmult=1;

				}

			}

		}

		

		//Gun whacking

		if(Button()&&(person[0].aiming==0||person[0].ammo<=0||person[0].whichgun==nogun||person[0].whichgun==knife||person[0].targetanimation==joganim)&&person[0].currentanimation!=crouchanim&&person[0].targetanimation!=throwanim&&person[0].whichgun!=grenade&&person[0].targetanimation!=crouchanim&&visions==0){

			if(person[0].attackframe==-1||person[0].killtarget<0||person[person[0].killtarget].skeleton.free==1){

				bool attacking=0;

				person[0].killtarget=-1;

				float closedistance=-1;

				for(int i=1;i<numpeople;i++){				

					if(person[i].existing&&person[i].type!=viptype&&person[i].skeleton.free<1&&findDistancefast(person[i].playercoords,person[0].playercoords+flatfacing)<12+(person[0].whichgun==knife)*10){

						if(person[0].killtarget==-1||findDistancefast(person[i].playercoords,person[0].playercoords)<closedistance){

							attacking=1;

							person[0].killtarget=i;

							closedistance=findDistancefast(person[i].playercoords,person[0].playercoords);

						}

					}

				}

				if(attacking){

					person[0].attacktarget=0;

					person[0].attackframe=0;

				}

				whacked=0;

			}

		}

		

		XYZ velocity;

		

		if(person[0].attackframe>1||(person[0].attackframe>=0&&person[0].currentanimation==joganim)){

			if(person[0].killtarget>0&&person[person[0].killtarget].skeleton.free<1&&(person[0].aiming<1||person[0].whichgun==nogun||person[0].whichgun==knife||person[0].targetanimation==joganim)){

				float gLoc[3];

				gLoc[0]=(camera.position.x+((person[0].playercoords.x+flatfacing.x)-camera.position.x)/2)/soundscalefactor;

				gLoc[1]=(camera.position.y+((person[0].playercoords.y+flatfacing.y)-camera.position.y)/2)/soundscalefactor;

				gLoc[2]=(camera.position.z+((person[0].playercoords.z+flatfacing.z)-camera.position.z)/2)/soundscalefactor;

				if(person[person[0].killtarget].type!=zombietype)

				{

					if(person[0].whichgun!=knife){

						alSourcefv(gSourceID[headwhacksound], AL_POSITION, gLoc);

						alSourcePlay(gSourceID[headwhacksound]);

					}

					if(person[0].whichgun==knife){

						alSourcefv(gSourceID[knifeslashsound], AL_POSITION, gLoc);

						alSourcePlay(gSourceID[knifeslashsound]);

						person[person[0].killtarget].bjoint1=&person[person[0].killtarget].skeleton.joints[person[person[0].killtarget].skeleton.jointlabels[neck]];

						person[person[0].killtarget].bjoint2=&person[person[0].killtarget].skeleton.joints[person[person[0].killtarget].skeleton.jointlabels[neck]];

						person[person[0].killtarget].bleeding=1;

						person[person[0].killtarget].bleeddelay=1;

						person[0].bjoint1=&person[0].skeleton.joints[person[0].skeleton.jointlabels[righthand]];

						person[0].bjoint2=&person[0].skeleton.joints[person[0].skeleton.jointlabels[righthand]];

						person[0].bleeding=1;

						person[0].bleeddelay=1;

						velocity=DoRotation(flatfacing,0,70,0)*50+person[0].velocity*2;

						velocity.y+=30;

						if(blood)sprites.MakeSprite(bloodspritedown, .8, 1, .2, .2,DoRotation(person[person[0].killtarget].skeleton.joints[person[person[0].killtarget].skeleton.jointlabels[neck]].position,0,person[person[0].killtarget].playerrotation,0)+person[person[0].killtarget].playercoords, velocity*.3, 2);

						if(blood)sprites.MakeSprite(bloodspritedown, .8, 1, .2, .2,DoRotation(person[person[0].killtarget].skeleton.joints[person[person[0].killtarget].skeleton.jointlabels[neck]].position,0,person[person[0].killtarget].playerrotation,0)+person[person[0].killtarget].playercoords, velocity*.2, 3);

						if(blood)sprites.MakeSprite(bloodspritedown, .8, 1, .2, .2,DoRotation(person[person[0].killtarget].skeleton.joints[person[person[0].killtarget].skeleton.jointlabels[neck]].position,0,person[person[0].killtarget].playerrotation,0)+person[person[0].killtarget].playercoords, velocity*.1, 4);					

					}

					person[person[0].killtarget].health-=100;

					person[person[0].killtarget].skeleton.free=1;

					person[person[0].killtarget].longdead=1;

					for(int j=0;j<person[person[0].killtarget].skeleton.num_joints;j++){

						person[person[0].killtarget].skeleton.joints[j].position=DoRotation(person[person[0].killtarget].skeleton.joints[j].position,0,person[person[0].killtarget].playerrotation,0);

						person[person[0].killtarget].skeleton.joints[j].position+=person[person[0].killtarget].playercoords;

						person[person[0].killtarget].skeleton.joints[j].realoldposition=person[person[0].killtarget].skeleton.joints[j].position;

						person[person[0].killtarget].skeleton.joints[j].velocity=person[person[0].killtarget].velocity;

						person[person[0].killtarget].skeleton.joints[j].velocity.x+=abs(Random()%10)-5;

						person[person[0].killtarget].skeleton.joints[j].velocity.y+=abs(Random()%10)-5;

						person[person[0].killtarget].skeleton.joints[j].velocity.z+=abs(Random()%10)-5;

					}

					if(person[0].whichgun!=knife){

						person[person[0].killtarget].skeleton.joints[person[person[0].killtarget].skeleton.jointlabels[head]].velocity+=DoRotation(flatfacing,0,40,0)*50;

						person[person[0].killtarget].skeleton.joints[person[person[0].killtarget].skeleton.jointlabels[head]].velocity+=person[0].velocity*2;

					}

				}

				else

				{

					if(whacked==0){

						whacked=1;

						if(person[0].whichgun!=knife){

							alSourcefv(gSourceID[headwhacksound], AL_POSITION, gLoc);

							alSourcePlay(gSourceID[headwhacksound]);

						}

						if(person[0].whichgun==knife){

							alSourcefv(gSourceID[knifeslashsound], AL_POSITION, gLoc);

							alSourcePlay(gSourceID[knifeslashsound]);

							person[person[0].killtarget].bjoint1=&person[person[0].killtarget].skeleton.joints[person[person[0].killtarget].skeleton.jointlabels[neck]];

							person[person[0].killtarget].bjoint2=&person[person[0].killtarget].skeleton.joints[person[person[0].killtarget].skeleton.jointlabels[neck]];

							person[person[0].killtarget].bleeding=1;

							person[person[0].killtarget].bleeddelay=1;

							person[0].bjoint1=&person[0].skeleton.joints[person[0].skeleton.jointlabels[righthand]];

							person[0].bjoint2=&person[0].skeleton.joints[person[0].skeleton.jointlabels[righthand]];

							person[0].bleeding=1;

							person[0].bleeddelay=1;

							velocity=DoRotation(flatfacing,0,70,0)*50+person[0].velocity*2;

							velocity.y+=30;

							if(blood)sprites.MakeSprite(bloodspritedown, .8, 1, .2, .2,DoRotation(person[person[0].killtarget].skeleton.joints[person[person[0].killtarget].skeleton.jointlabels[neck]].position,0,person[person[0].killtarget].playerrotation,0)+person[person[0].killtarget].playercoords, velocity*.3, 2);

							if(blood)sprites.MakeSprite(bloodspritedown, .8, 1, .2, .2,DoRotation(person[person[0].killtarget].skeleton.joints[person[person[0].killtarget].skeleton.jointlabels[neck]].position,0,person[person[0].killtarget].playerrotation,0)+person[person[0].killtarget].playercoords, velocity*.2, 3);

							if(blood)sprites.MakeSprite(bloodspritedown, .8, 1, .2, .2,DoRotation(person[person[0].killtarget].skeleton.joints[person[person[0].killtarget].skeleton.jointlabels[neck]].position,0,person[person[0].killtarget].playerrotation,0)+person[person[0].killtarget].playercoords, velocity*.1, 4);					

						}

						person[person[0].killtarget].health-=200;

						person[person[0].killtarget].maxhealth-=20;

						person[person[0].killtarget].skeleton.free=1;

						person[person[0].killtarget].longdead=1;

						for(int j=0;j<person[person[0].killtarget].skeleton.num_joints;j++){

							person[person[0].killtarget].skeleton.joints[j].position=DoRotation(person[person[0].killtarget].skeleton.joints[j].position,0,person[person[0].killtarget].playerrotation,0);

							person[person[0].killtarget].skeleton.joints[j].position+=person[person[0].killtarget].playercoords;

							person[person[0].killtarget].skeleton.joints[j].realoldposition=person[person[0].killtarget].skeleton.joints[j].position;

							person[person[0].killtarget].skeleton.joints[j].velocity=person[person[0].killtarget].velocity;

							person[person[0].killtarget].skeleton.joints[j].velocity.x+=abs(Random()%10)-5;

							person[person[0].killtarget].skeleton.joints[j].velocity.y+=abs(Random()%10)-5;

							person[person[0].killtarget].skeleton.joints[j].velocity.z+=abs(Random()%10)-5;

						}

						if(person[0].whichgun!=knife){

							person[person[0].killtarget].skeleton.joints[person[person[0].killtarget].skeleton.jointlabels[head]].velocity+=DoRotation(flatfacing,0,40,0)*50;

							person[person[0].killtarget].skeleton.joints[person[person[0].killtarget].skeleton.jointlabels[head]].velocity+=person[0].velocity*2;

						}

					}

				}

			}

		}

		

		//Tackle

		if(person[0].currentanimation==diveanim&&visions==0){

			for(int i=1;i<numpeople;i++){				

				if(person[i].skeleton.free<1&&findDistancefast(person[i].playercoords,person[0].playercoords+flatfacing)<22){

					float gLoc[3];

					gLoc[0]=(camera.position.x+((person[0].playercoords.x+flatfacing.x)-camera.position.x)/2)/soundscalefactor;

					gLoc[1]=(camera.position.y+((person[0].playercoords.y+flatfacing.y)-camera.position.y)/2)/soundscalefactor;

					gLoc[2]=(camera.position.z+((person[0].playercoords.z+flatfacing.z)-camera.position.z)/2)/soundscalefactor;

					alSourcefv(gSourceID[headwhacksound], AL_POSITION, gLoc);

					alSourcePlay(gSourceID[headwhacksound]);

					person[i].skeleton.free=1;

					person[i].longdead=1;

					for(int j=0;j<person[i].skeleton.num_joints;j++){

						person[i].skeleton.joints[j].position=DoRotation(person[i].skeleton.joints[j].position,0,person[i].playerrotation,0);

						person[i].skeleton.joints[j].position+=person[i].playercoords;

						person[i].skeleton.joints[j].realoldposition=person[i].skeleton.joints[j].position;

						person[i].skeleton.joints[j].velocity=person[0].velocity;

						person[i].skeleton.joints[j].velocity.y=-10;

						person[i].skeleton.joints[j].velocity.x+=abs(Random()%10)-5;

						person[i].skeleton.joints[j].velocity.y+=abs(Random()%10)-5;

						person[i].skeleton.joints[j].velocity.z+=abs(Random()%10)-5;

					}

				}

			}

		}

		

		//Fire/wing

		XYZ wallhit;

		XYZ start;

		XYZ finalwallhit;

		bool firing;

		float inaccuracy = 0.0;

		int numshots;

		XYZ hitnorm;

		XYZ hitmove;

		int hitpoly=0;

		float hitrotation = 0.0f;

		Model *model = NULL;

		for(int j=0;j<numpeople;j++){

			if(person[j].type==playertype||person[j].type==eviltype){

			firing=0;

			if(person[j].ammo>0&&person[j].reloading<=0&&person[j].targetanimation!=joganim){

				if(Button()&&!oldbutton&&j==0)firing=1;

				if(j!=0)firing=0;

				if(j!=0&&person[j].whichgun!=nogun&&person[j].whichgun!=knife&&person[j].killtargetvisible&&person[j].shotdelay<0)firing=1;

				if(j!=0&&person[j].whichgun!=nogun&&person[j].whichgun!=knife&&person[j].killtargetvisible&&person[j].shotdelay>0)person[j].shotdelay-=multiplier*.9;

				if(person[j].skeleton.free==1||person[j].targetanimation==getupfrontanim||person[j].targetanimation==getupbackanim)person[j].shotdelay=shotdelayamount/difficulty;

				if(j!=0&&person[j].whichgun!=nogun&&person[j].whichgun!=knife&&person[j].killtargetvisible)person[j].aiming=1;

				if(person[0].whichgun!=assaultrifle)oldbutton=1;

			}

			if(person[j].ammo==0&&person[j].reloads[person[j].whichgun]>0){

				person[j].ammo=-1;

				person[j].aiming=0;

				firing=0;

			}

			if(person[0].ammo<=0&&Button()&&!oldbutton&&person[0].targetanimation!=joganim&&person[0].whichgun!=nogun&&person[0].whichgun!=knife&&person[0].whichgun!=grenade&&person[0].aiming){

				oldbutton=1;

				float gLoc[3];

				gLoc[0]=person[0].playercoords.x/soundscalefactor;

				gLoc[1]=person[0].playercoords.y/soundscalefactor;

				gLoc[2]=person[0].playercoords.z/soundscalefactor;



				alSourcefv(gSourceID[clicksound], AL_POSITION, gLoc);

				alSourcePlay(gSourceID[clicksound]);

			}

			if(j==0&&visions==1&&firing){

				firing=0;

				alSourcePlay(gSourceID[soulinsound]);

			}

			if(person[j].aimamount<1)firing=0;

			if(firing){

				if(person[j].whichgun==handgun1||person[j].whichgun==handgun2)inaccuracy=8;

				if(person[j].whichgun==assaultrifle)inaccuracy=6;

				if(person[j].whichgun==sniperrifle)inaccuracy=2;

				if(person[j].whichgun==shotgun)inaccuracy=6;

				if(person[j].killtarget>-1&&person[person[j].killtarget].skeleton.free==1)inaccuracy*=3;



				person[j].shotdelay=shotdelayamount/difficulty;

				if(person[j].aiming>=1&&person[j].recoil<=0){

					//Firing

					person[j].litup=1;

					person[j].recoil=1;

					XYZ end, aim, oldend;

					HitStruct hitstruct,temphitstruct;

					float olddistance = 0.0f;

					float distance = 0.0f;

					float totalarea = 0.0f;

					int whichhit=-1;

					if(person[j].whichgun==shotgun)numshots=7;

					if(person[j].whichgun!=shotgun)numshots=1;

					if(person[j].whichgun!=grenade)person[j].ammo--;

					for(int p=0;p<numshots;p++){

					if(!zoom||j!=0){

						if(person[j].whichgun==assaultrifle)aim=DoRotation(person[j].skeleton.joints[(person[j].skeleton.jointlabels[lefthand])].position-person[j].skeleton.joints[(person[j].skeleton.jointlabels[righthand])].position,0,person[j].playerrotation-2.5,0);

						if(person[j].whichgun==sniperrifle)aim=DoRotation(person[j].skeleton.joints[(person[j].skeleton.jointlabels[lefthand])].position-person[j].skeleton.joints[(person[j].skeleton.jointlabels[righthand])].position,0,person[j].playerrotation+4,0);

						if(person[j].whichgun==shotgun)aim=DoRotation(person[j].skeleton.joints[(person[j].skeleton.jointlabels[lefthand])].position-person[j].skeleton.joints[(person[j].skeleton.jointlabels[righthand])].position,2+(float)(Random()%1000)/500,0,0);

						if(person[j].whichgun==shotgun)aim=DoRotation(aim,0,person[j].playerrotation-1+(float)(Random()%1000)/500,0);

						if(person[j].whichgun==handgun1&&!thirdperson&&j==0)aim=DoRotation(person[j].skeleton.joints[(person[j].skeleton.jointlabels[righthand])].position-(person[j].skeleton.joints[person[j].skeleton.jointlabels[head]].position*.65+person[j].skeleton.joints[person[j].skeleton.jointlabels[neck]].position*.35),0,person[j].playerrotation-.9,0);

						if(person[j].whichgun==handgun1&&(thirdperson||j!=0))aim=DoRotation(person[j].skeleton.joints[(person[j].skeleton.jointlabels[righthand])].position-(person[j].skeleton.joints[person[j].skeleton.jointlabels[head]].position*.35+person[j].skeleton.joints[person[j].skeleton.jointlabels[neck]].position*.65),0,person[j].playerrotation-.9,0);

						if(person[j].whichgun==handgun2&&!thirdperson&&j==0)aim=DoRotation(person[j].skeleton.joints[(person[j].skeleton.jointlabels[righthand])].position-(person[j].skeleton.joints[person[j].skeleton.jointlabels[head]].position*.65+person[j].skeleton.joints[person[j].skeleton.jointlabels[neck]].position*.35),0,person[j].playerrotation-.9,0);

						if(person[j].whichgun==handgun2&&(thirdperson||j!=0))aim=DoRotation(person[j].skeleton.joints[(person[j].skeleton.jointlabels[righthand])].position-(person[j].skeleton.joints[person[j].skeleton.jointlabels[head]].position*.35+person[j].skeleton.joints[person[j].skeleton.jointlabels[neck]].position*.65),0,person[j].playerrotation-.9,0);

						if(j!=0&&person[j].killtarget>-1&&person[person[j].killtarget].skeleton.free==0)aim=(DoRotation(person[person[j].killtarget].skeleton.joints[person[person[j].killtarget].skeleton.jointlabels[abdomen]].position,0,person[person[j].killtarget].playerrotation,0)+person[person[j].killtarget].playercoords)-(DoRotation(person[j].skeleton.joints[person[j].skeleton.jointlabels[lefthand]].position,0,person[j].playerrotation,0)+person[j].playercoords);

						if(j!=0&&person[j].killtarget>-1&&person[person[j].killtarget].skeleton.free!=0)aim=person[person[j].killtarget].skeleton.joints[person[person[j].killtarget].skeleton.jointlabels[abdomen]].position-(DoRotation(person[j].skeleton.joints[person[j].skeleton.jointlabels[lefthand]].position,0,person[j].playerrotation,0)+person[j].playercoords);

						if(j!=0)aim=DoRotation(DoRotation(DoRotation(aim,0,-person[j].playerrotation,0),(float)(abs(Random()%2000))/2000*inaccuracy-inaccuracy/2,(float)(abs(Random()%2000))/2000*inaccuracy-inaccuracy/2,0),0,person[j].playerrotation,0);

					}

					if(zoom&&j==0){

						start=camera.position;

						aim=facing;

					}

					Normalise(&aim);


					if(person[j].whichgun==sniperrifle){

						start=person[j].playercoords+DoRotation(person[j].skeleton.joints[(person[j].skeleton.jointlabels[lefthand])].position,0,person[j].playerrotation,0);

						float gLoc[3];

						

						start-=DoRotation(DoRotation(DoRotation(aim,0,-person[j].playerrotation,0),90,0,0),0,person[j].playerrotation,0)*.35;

						

						gLoc[0]=(camera.position.x+(start.x-camera.position.x)/4)/soundscalefactor;

						gLoc[1]=(camera.position.y+(start.y-camera.position.y)/4)/soundscalefactor;

						gLoc[2]=(camera.position.z+(start.z-camera.position.z)/4)/soundscalefactor;

						alSourcefv(gSourceID[riflesound], AL_POSITION, gLoc);

						alSourcePlay(gSourceID[riflesound]);

						if(j==0){

							if(person[j].currentanimation!=crouchanim)CameraRecoil(7);

							if(person[j].currentanimation==crouchanim)CameraRecoil(3);

						}

					}

					if(person[j].whichgun==shotgun){

						start=person[j].playercoords+DoRotation(person[j].skeleton.joints[(person[j].skeleton.jointlabels[lefthand])].position,0,person[j].playerrotation,0);

						float gLoc[3];

						

						start-=DoRotation(DoRotation(DoRotation(aim,0,-person[j].playerrotation,0),90,0,0),0,person[j].playerrotation,0)*.35;

						

						if(p==numshots-1){

						gLoc[0]=(camera.position.x+(start.x-camera.position.x)/4)/soundscalefactor;

						gLoc[1]=(camera.position.y+(start.y-camera.position.y)/4)/soundscalefactor;

						gLoc[2]=(camera.position.z+(start.z-camera.position.z)/4)/soundscalefactor;

						alSourcefv(gSourceID[shotgunsound], AL_POSITION, gLoc);

						alSourcePlay(gSourceID[shotgunsound]);

						if(j==0){

							if(person[j].currentanimation!=crouchanim)CameraRecoil(7);

							if(person[j].currentanimation==crouchanim)CameraRecoil(3);

						}

						}

					}

					if(person[j].whichgun==handgun1){

						start=person[j].playercoords+DoRotation(person[j].skeleton.joints[(person[j].skeleton.jointlabels[rightwrist])].position,0,person[j].playerrotation,0);

						start-=DoRotation(DoRotation(DoRotation(aim,0,-person[j].playerrotation,0),90,0,0),0,person[j].playerrotation,0)*.55;

						

						float gLoc[3];

						

						gLoc[0]=(camera.position.x+(start.x-camera.position.x)/4)/soundscalefactor;

						gLoc[1]=(camera.position.y+(start.y-camera.position.y)/4)/soundscalefactor;

						gLoc[2]=(camera.position.z+(start.z-camera.position.z)/4)/soundscalefactor;

						alSourcefv(gSourceID[pistol1sound], AL_POSITION, gLoc);

						alSourcePlay(gSourceID[pistol1sound]);

						if(j==0){

							if(person[j].currentanimation!=crouchanim)CameraRecoil(6);

							if(person[j].currentanimation==crouchanim)CameraRecoil(4);

						}

					}

					if(person[j].whichgun==handgun2){

						start=person[j].playercoords+DoRotation(person[j].skeleton.joints[(person[j].skeleton.jointlabels[rightwrist])].position,0,person[j].playerrotation,0);

						start-=DoRotation(DoRotation(DoRotation(aim,0,-person[j].playerrotation,0),90,0,0),0,person[j].playerrotation,0)*.55;

						

						float gLoc[3];

						

						gLoc[0]=(camera.position.x+(start.x-camera.position.x)/4)/soundscalefactor;

						gLoc[1]=(camera.position.y+(start.y-camera.position.y)/4)/soundscalefactor;

						gLoc[2]=(camera.position.z+(start.z-camera.position.z)/4)/soundscalefactor;

						alSourcefv(gSourceID[pistol2sound], AL_POSITION, gLoc);

						alSourcePlay(gSourceID[pistol2sound]);

						if(j==0){

							if(person[j].currentanimation!=crouchanim)CameraRecoil(5);

							if(person[j].currentanimation==crouchanim)CameraRecoil(3);

						}

					}

					if(person[j].whichgun==assaultrifle){

						start=person[j].playercoords+DoRotation(person[j].skeleton.joints[(person[j].skeleton.jointlabels[lefthand])].position,0,person[j].playerrotation,0);

						start-=DoRotation(DoRotation(DoRotation(aim,0,-person[j].playerrotation,0),90,0,0),0,person[j].playerrotation,0)*.25;

						

						float gLoc[3];

						gLoc[0]=(camera.position.x+(start.x-camera.position.x)/4)/soundscalefactor;

						gLoc[1]=(camera.position.y+(start.y-camera.position.y)/4)/soundscalefactor;

						gLoc[2]=(camera.position.z+(start.z-camera.position.z)/4)/soundscalefactor;

						alSourcefv(gSourceID[machinegunsound], AL_POSITION, gLoc);

						alSourcePlay(gSourceID[machinegunsound]);

						if(j==0){

							if(person[j].currentanimation!=crouchanim){

								CameraRecoil(2.3);

								camera.rotation+=(float)(Random()%100)/50;

							}

							if(person[j].currentanimation==crouchanim){

								CameraRecoil(1.5);

								camera.rotation+=(float)(Random()%100)/60;

							}

						}

					}

					if(j==0&&pointeraim&&!zoom)PointerShotStart(&start);

					end=start+aim*1000;

					if(debug){

					if(IsKeyDown(theKeyMap, MAC_G_KEY)&&j==0){

						sprites.MakeSprite(grenadesprite, 1, 1, 1, 1, start, aim*200, 1.01);

					}

					}

					if(!IsKeyDown(theKeyMap, MAC_G_KEY)||j!=0||!debug){

					int bulletstrength=1;

					int firstpass=-1;

					bool penetrate;

					for(int m=0;m<bulletstrength;m++){

						//People

						whichhit=-1;

						for(int i=0;i<numpeople;i++){

							if(i!=j&&i!=firstpass&&person[i].existing){

								temphitstruct=person[i].BulletCollideWithPlayer(i, start, end);	

								if(temphitstruct.collision){

									distance=findDistancefast(start,temphitstruct.hitlocation);

									if(distance<olddistance||whichhit==-1){

										end=temphitstruct.hitlocation;

										olddistance=distance;

										hitstruct=temphitstruct;

										whichhit=i;

									}

								}

							}

						}

						//Blocks

						wallhit=0;

						beginx=(person[j].playercoords.x+block_spacing/2)/block_spacing-3;

						if(beginx<0)beginx=0;

						beginz=(person[j].playercoords.z+block_spacing/2)/block_spacing-3;

						if(beginz<0)beginz=0;

						

						endx=(person[j].playercoords.x+block_spacing/2)/block_spacing+3;

						if(endx>num_blocks-1)endx=num_blocks-1;

						endz=(person[j].playercoords.z+block_spacing/2)/block_spacing+3;

						if(endz>num_blocks-1)endz=num_blocks-1;

						

						if(beginx<endx&&beginz<endz)

						finalwallhit=0;

						for(int i=beginx;i<=endx;i++)

							for(int j=beginz;j<=endz;j++){

								move=0;

								move.x=i*block_spacing;

								move.z=j*block_spacing;

								whichtri=blocks[citytype[i][j]].LineCheck2(start,end,&wallhit,move,cityrotation[i][j]*90);

								if(whichtri!=-1){

									whichhit=-1;

									end=wallhit;

									finalwallhit=wallhit;

									hitnorm=DoRotation(blocks[citytype[i][j]].normals[whichtri],0,cityrotation[i][j]*90,0);

									hitmove=move;

									hitrotation=cityrotation[i][j]*90;

									hitpoly=whichtri;

									model=&blocks[citytype[i][j]];

									if(j==0&&blocks[citytype[i][j]].normals[whichtri].y>.9)bulletstrength=2;

								}

							}

						wallhit=0;

						wallhit.x=camera.position.x;

						wallhit.z=camera.position.z;

						whichtri=Bigstreet.LineCheck2(start,end,&wallhit,wallhit,0);

						if(whichtri!=-1){

							end.y-=.5;

							end=wallhit;

							finalwallhit=wallhit;

							bulletstrength=2;

							hitnorm=0;

							hitnorm.y=1;

							hitmove=0;

							hitrotation=0;

						}

						if(m==0){

							if(j==0&&slomo==2){

								soundscalefactor=soundscalefactordefault;

								alSourceStop(gSourceID[whichsong]);

								alSourcef(gSourceID[whichsong], AL_MIN_GAIN, 0);

								alSourcef(gSourceID[whichsong], AL_MAX_GAIN, 0);

								if(person[0].whichgun==knife)whichsong=knifesong;

								if(person[0].whichgun!=knife)whichsong=shootsong;

								if(type==zombie_type)whichsong=zombiesong;

								alSourcef(gSourceID[whichsong], AL_PITCH, 1);

								alSourcePlay(gSourceID[whichsong]);

								alSourcef(gSourceID[whichsong], AL_MIN_GAIN, 1);

								alSourcef(gSourceID[whichsong], AL_MAX_GAIN, 1);

								slomo=0;

								if(whichhit==-1)alSourcePlay(gSourceID[disguisekillsound]);

								flashamount=.5;

								flashr=1;flashg=1;flashb=1;

							}

						}

						//Impact

						oldend=end;

						//with person

						if(whichhit!=-1&&whichhit!=firstpass){

							if(j==0)person[whichhit].dead=1;

							if(whichhit==1&&!revenge){   //person 1 is only the VIP outside revenge mode

								murderer=j;

							}

							if(person[whichhit].health==100&&j==0){

								if(person[whichhit].type==civiliantype)civkills++;

								if(person[whichhit].type==eviltype)goodkills++;

							}

							if(person[whichhit].health==100&&j!=0){

								badkills++;

							}

							//penetrate

								penetrate=abs(Random()%2)==1;

								if(numshots>1)penetrate=0;

								if(penetrate){bulletstrength=2;

								firstpass=whichhit;

								end=start+aim*1000;}

							if(person[j].whichgun==assaultrifle)person[whichhit].health-=20;

							if(person[j].whichgun==assaultrifle&&person[whichhit].type==zombietype)person[whichhit].health-=60;

							if(person[j].whichgun==handgun1){

								if(person[whichhit].type!=zombietype)person[whichhit].health-=100;

								if(person[whichhit].type==zombietype)person[whichhit].health-=100;

								person[whichhit].DoAnimations(whichhit);

							}

							if(person[j].whichgun==handgun2)person[whichhit].health-=20;

							if(person[j].whichgun==handgun2&&person[whichhit].type==zombietype)person[whichhit].health-=60;

							if(person[j].whichgun==sniperrifle&&m!=0)person[whichhit].health-=30;

							if(person[j].whichgun==shotgun)person[whichhit].health-=60;

							if(person[j].whichgun==sniperrifle&&m==0){

								if(person[whichhit].type!=zombietype)person[whichhit].health-=100;

								if(person[whichhit].type==zombietype)person[whichhit].health-=120;

								person[whichhit].DoAnimations(whichhit);

							}

							if(hitstruct.joint1->modelnum==headmodel&&person[whichhit].type!=zombietype){

								person[whichhit].health-=60;

							}

							if(person[whichhit].type==zombietype)person[whichhit].speedmult-=.05;

							if(person[whichhit].type==zombietype)person[whichhit].maxhealth-=10;

							/* In the normal game a bullet only knocks the bodyguard down.  In
							   revenge mode it hurts you like anyone else - and knocks you out
							   of the scope (from the Revenge Mod). */
							if(whichhit==0&&revenge){

								UnZoom();

							}

							if(whichhit==0&&!revenge){

								bulletstrength=1;

								person[0].health=100;

								flashr=0;

								flashg=0;

								flashb=0;

								flashamount=1;

								float gLoc[3];

								gLoc[0]=hitstruct.hitlocation.x/soundscalefactor;

								gLoc[1]=hitstruct.hitlocation.y/soundscalefactor;

								gLoc[2]=hitstruct.hitlocation.z/soundscalefactor;

								alSourcefv(gSourceID[bodywhacksound], AL_POSITION, gLoc);

								alSourcePlay(gSourceID[bodywhacksound]);

							}

							person[whichhit].longdead=1;

							

							if(person[whichhit].health<=0){

								person[whichhit].skeleton.offsetted=0;

								if(person[whichhit].skeleton.free!=1){

									person[whichhit].skeleton.free=1;

									totalarea=0;

									for(int j=0;j<person[whichhit].skeleton.num_joints;j++){

										person[whichhit].skeleton.joints[j].position=DoRotation(person[whichhit].skeleton.joints[j].position,0,person[whichhit].playerrotation,0);

										person[whichhit].skeleton.joints[j].position+=person[whichhit].playercoords;

										person[whichhit].skeleton.joints[j].realoldposition=person[whichhit].skeleton.joints[j].position;

										person[whichhit].skeleton.joints[j].velocity=person[whichhit].velocity;

										person[whichhit].skeleton.joints[j].velocity.x+=(float)(abs(Random()%20)-10)/2;

										person[whichhit].skeleton.joints[j].velocity.y+=(float)(abs(Random()%20)-10)/2;

										person[whichhit].skeleton.joints[j].velocity.z+=(float)(abs(Random()%20)-10)/2;

									}

								}

								for(int j=0;j<person[whichhit].skeleton.num_joints;j++){

									if(findDistancefast(person[whichhit].skeleton.joints[j].position,hitstruct.hitlocation)<200){

										totalarea+=HitWeight(person[whichhit].skeleton.joints[j].position,hitstruct.hitlocation);

									}

								}

								for(int j=0;j<person[whichhit].skeleton.num_joints;j++){

									if(findDistancefast(person[whichhit].skeleton.joints[j].position,hitstruct.hitlocation)<200){

										person[whichhit].skeleton.joints[j].velocity+=aim*(HitWeight(person[whichhit].skeleton.joints[j].position,hitstruct.hitlocation)/totalarea*200);

									}

								}

							}

							if(person[whichhit].health>0){

								if(person[whichhit].killtargetvisible==0&&person[whichhit].type!=zombietype&&person[whichhit].currentanimation!=getupfrontanim&person[whichhit].currentanimation!=getupbackanim){

									if(hitstruct.joint1->modelnum==headmodel)person[whichhit].targetanimation=headpainanim;

									if(hitstruct.joint1->modelnum==chestmodel)person[whichhit].targetanimation=chestpainanim;

									if(hitstruct.joint1->modelnum==abdomenmodel)person[whichhit].targetanimation=stomachpainanim;

									if(hitstruct.joint1->label==rightelbow||hitstruct.joint1->label==rightshoulder||hitstruct.joint1->label==rightwrist||hitstruct.joint1->label==righthand)person[whichhit].targetanimation=rightarmpainanim;

									if(hitstruct.joint1->label==leftelbow||hitstruct.joint1->label==leftshoulder||hitstruct.joint1->label==leftwrist||hitstruct.joint1->label==lefthand)person[whichhit].targetanimation=leftarmpainanim;

									if(hitstruct.joint1->label==rightknee||hitstruct.joint1->label==righthip||hitstruct.joint1->label==rightankle||hitstruct.joint1->label==rightfoot)person[whichhit].targetanimation=rightlegpainanim;

									if(hitstruct.joint1->label==leftknee||hitstruct.joint1->label==lefthip||hitstruct.joint1->label==leftankle||hitstruct.joint1->label==leftfoot)person[whichhit].targetanimation=leftlegpainanim;

									person[whichhit].targetframe=0;

									person[whichhit].target=0;

								}

								person[whichhit].skeleton.offsetted=1;

								for(int j=0;j<person[whichhit].skeleton.num_joints;j++){

									if(findDistancefast(DoRotation(person[whichhit].skeleton.joints[j].position,0,person[whichhit].playerrotation,0)+person[whichhit].playercoords,hitstruct.hitlocation)<200){

										totalarea+=HitWeight(DoRotation(person[whichhit].skeleton.joints[j].position,0,person[whichhit].playerrotation,0)+person[whichhit].playercoords,hitstruct.hitlocation);

									}

								}

								float offsetlength;

								for(int j=0;j<person[whichhit].skeleton.num_joints;j++){

									if(findDistancefast(DoRotation(person[whichhit].skeleton.joints[j].position,0,person[whichhit].playerrotation,0)+person[whichhit].playercoords,hitstruct.hitlocation)<200){

										person[whichhit].skeleton.joints[j].offset+=DoRotation(aim*(HitWeight(DoRotation(person[whichhit].skeleton.joints[j].position,0,person[whichhit].playerrotation,0)+person[whichhit].playercoords,hitstruct.hitlocation)/totalarea*10),0,-person[whichhit].playerrotation,0);

									}

									offsetlength=findLengthfast(person[whichhit].skeleton.joints[j].offset);

									if(offsetlength>36){

										Normalise(&person[whichhit].skeleton.joints[j].offset);

										person[whichhit].skeleton.joints[j].offset*=6;

									}

								}

							}

							if(hitstruct.joint1->modelnum==headmodel&&person[whichhit].health<=0){

								for(int j=0;j<person[whichhit].skeleton.num_joints;j++){

									if(&person[whichhit].skeleton.joints[j]==hitstruct.joint1||&person[whichhit].skeleton.joints[j]==hitstruct.joint2){

										if(j!=person[whichhit].skeleton.jointlabels[abdomen]&&j!=person[whichhit].skeleton.jointlabels[groin]&&j!=person[whichhit].skeleton.jointlabels[neck]){

											if(blood)sprites.MakeSprite(bloodspritedown, .8, 1, .2, .2,person[whichhit].skeleton.joints[j].position, person[whichhit].skeleton.joints[j].velocity/3, 9);

											if(blood)sprites.MakeSprite(bloodspritedown, .8, 1, .2, .2,person[whichhit].skeleton.joints[j].position, DoRotation(person[whichhit].skeleton.joints[j].velocity/3,Random()%360,Random()%360,0)/5, 5);

											if(blood)sprites.MakeSprite(bloodspritedown, .8, 1, .2, .2,person[whichhit].skeleton.joints[j].position, DoRotation(person[whichhit].skeleton.joints[j].velocity/3,Random()%360,Random()%360,0)/5, 5);

											if(blood)sprites.MakeSprite(bloodspritedown, .8, 1, .2, .2,person[whichhit].skeleton.joints[j].position, DoRotation(person[whichhit].skeleton.joints[j].velocity/3,Random()%360,Random()%360,0)/5, 5);

											if(blood)sprites.MakeSprite(bloodspritedown, .8, 1, .2, .2,person[whichhit].skeleton.joints[j].position, DoRotation(person[whichhit].skeleton.joints[j].velocity/3,Random()%360,Random()%360,0)/5, 5);

											person[whichhit].skeleton.DeleteJoint(j);

											person[whichhit].skeleton.broken=1;

											person[whichhit].health=-10000;

											person[whichhit].skeleton.joints[j].existing=0;

											if(person[whichhit].type==zombietype)score+=300;

										}

									}

								}

							}

							

							XYZ velocity;

							velocity=aim*-8;

							//blood - or, with Blood off in config.txt, a puff of smoke (Black Shades Enhanced)

							const int hitsprite=blood?bloodspritenoup:smokespritenoup;

							const float hitred=blood?1:0;

							if(hitstruct.joint1->modelnum!=headmodel){

							if(person[j].whichgun==sniperrifle)sprites.MakeSprite(hitsprite, 1, hitred, 0, 0, hitstruct.hitlocation, velocity*0, 5);

							if(person[j].whichgun==sniperrifle&&penetrate)sprites.MakeSprite(hitsprite, 1, hitred, 0, 0, hitstruct.hitlocation, velocity*-3, 7);

							if(person[j].whichgun==shotgun)sprites.MakeSprite(hitsprite, 1, hitred, 0, 0, hitstruct.hitlocation, velocity*0, 5);

							if(person[j].whichgun==shotgun&&penetrate)sprites.MakeSprite(hitsprite, 1, hitred, 0, 0, hitstruct.hitlocation, velocity*-3, 7);

							if(person[j].whichgun==assaultrifle)sprites.MakeSprite(hitsprite, 1, hitred, 0, 0, hitstruct.hitlocation, velocity*0, 3);

							if(person[j].whichgun==assaultrifle&&penetrate)sprites.MakeSprite(hitsprite, 1, hitred, 0, 0, hitstruct.hitlocation, velocity*-3, 7);

							if(person[j].whichgun==handgun1)sprites.MakeSprite(hitsprite, 1, hitred, 0, 0, hitstruct.hitlocation, velocity*0, 3);

							if(person[j].whichgun==handgun1&&penetrate)sprites.MakeSprite(hitsprite, 1, hitred, 0, 0, hitstruct.hitlocation, velocity*-3, 4);

							if(person[j].whichgun==handgun2)sprites.MakeSprite(hitsprite, 1, hitred, 0, 0, hitstruct.hitlocation, velocity*0, 3);

							if(person[j].whichgun==handgun2&&penetrate)sprites.MakeSprite(hitsprite, 1, hitred, 0, 0, hitstruct.hitlocation, velocity*-3, 4);

							}else{

								if(blood)sprites.MakeSprite(bloodspritenoup, 1, 1, .2, .2, hitstruct.hitlocation, velocity*0, 6);

								sprites.MakeSprite(hitsprite, 1, hitred, hitred*.5, hitred*.5, hitstruct.hitlocation, velocity*-2, 7);

								if(blood)sprites.MakeSprite(bloodspritenoup, 1, 1, .2, .2, hitstruct.hitlocation, velocity*-3, 10);

							}

							

							person[whichhit].bjoint1=hitstruct.joint1;

							person[whichhit].bjoint2=hitstruct.joint2;

							person[whichhit].bleeding=1;

							person[whichhit].bleeddelay=1;

							float gLoc[3];

							gLoc[0]=(camera.position.x+(hitstruct.hitlocation.x-camera.position.x)/4)/soundscalefactor;

							gLoc[1]=(camera.position.y+(hitstruct.hitlocation.y-camera.position.y)/4)/soundscalefactor;

							gLoc[2]=(camera.position.z+(hitstruct.hitlocation.z-camera.position.z)/4)/soundscalefactor;

							if(hitstruct.joint1->modelnum!=headmodel){

								if(!thirdperson)alSourcef(gSourceID[bodyhitsound], AL_MIN_GAIN, 1);

								if(thirdperson)alSourcef(gSourceID[bodyhitsound], AL_MIN_GAIN, .1);

								alSourcefv(gSourceID[bodyhitsound], AL_POSITION, gLoc);

								alSourcePlay(gSourceID[bodyhitsound]);

							}

							if(hitstruct.joint1->modelnum==headmodel){

								if(!thirdperson)alSourcef(gSourceID[headshotsound], AL_MIN_GAIN, 1);

								if(thirdperson)alSourcef(gSourceID[headshotsound], AL_MIN_GAIN, .1);

								alSourcefv(gSourceID[headshotsound], AL_POSITION, gLoc);

								alSourcePlay(gSourceID[headshotsound]);

							}

						}//with wall

						if(oldend==finalwallhit){

							decals.MakeDecal(bullethole, finalwallhit,.7,hitnorm, hitpoly, model, hitmove, hitrotation);

							XYZ velocity;

							velocity=aim*-4;

							velocity=hitnorm*3;

							if(person[j].whichgun==sniperrifle){

								sprites.MakeSprite(smokesprite, .4, 1, 1, 1, finalwallhit, velocity, 10);

								sprites.MakeSprite(muzzleflashsprite, 1, 1, 1, 1, finalwallhit, velocity, 2);

							}

							if(person[j].whichgun==shotgun){

								sprites.MakeSprite(smokesprite, .4, 1, 1, 1, finalwallhit, velocity, 5);

								sprites.MakeSprite(muzzleflashsprite, 1, 1, 1, 1, finalwallhit, velocity, .8);

							}

							if(person[j].whichgun==assaultrifle){

								sprites.MakeSprite(smokesprite, .4, 1, 1, 1, finalwallhit, velocity, 6);

								sprites.MakeSprite(muzzleflashsprite, 1, 1, 1, 1, finalwallhit, velocity, 1);

							}

							if(person[j].whichgun==handgun1){

								sprites.MakeSprite(smokesprite, .4, 1, 1, 1, finalwallhit, velocity, 6);

								sprites.MakeSprite(muzzleflashsprite, 1, 1, 1, 1, finalwallhit, velocity, 1);

							}

							if(person[j].whichgun==handgun2){

								sprites.MakeSprite(smokesprite, .4, 1, 1, 1, finalwallhit, velocity, 6);

								sprites.MakeSprite(muzzleflashsprite, 1, 1, 1, 1, finalwallhit, velocity, 1);

							}

							float gLoc[3];

							gLoc[0]=finalwallhit.x/soundscalefactor;

							gLoc[1]=finalwallhit.y/soundscalefactor;

							gLoc[2]=finalwallhit.z/soundscalefactor;

							alSourcefv(gSourceID[wallhitsound], AL_POSITION, gLoc);

							alSourcePlay(gSourceID[wallhitsound]);

						}

						lastshot[0]=start;

						lastshot[1]=oldend;

						velocity=aim*8;

						if(person[j].whichgun!=sniperrifle&&person[j].whichgun!=shotgun&&p==numshots-1)sprites.MakeSprite(smokesprite, .3, 1, 1, 1, start+aim*1.5, velocity, 3);

						if(person[j].whichgun==shotgun&&p==numshots-1)sprites.MakeSprite(smokesprite, .4, 1, 1, 1, start+aim*1.5, velocity, 5);

						if(person[j].whichgun==sniperrifle&&!zoom)sprites.MakeSprite(smokesprite, .3, 1, 1, 1, start+aim*2.2, velocity, 4);

						if(j!=0||zoom==0)sprites.MakeSprite(bullet, .07, 1, 1, .7, lastshot[0]+aim*1, lastshot[1], .2);

						//Nearby bullet whoosh		

						float dot_ta,dot_tb;

						XYZ *a,*b,*c,nearest;

						a=&lastshot[0];

						*a+=aim*1;

						b=&lastshot[1];

						c=&camera.position;

						nearest=0;
						bool nearestvalid=false;

						dot_ta = (c->x - a->x)*(b->x - a->x) + (c->y - a->y)*(b->y - a->y) + (c->z - a->z)*(b->z - a->z);

						dot_tb = (c->x - b->x)*(a->x - b->x) + (c->y - b->y)*(a->y - b->y) + (c->z - b->z)*(a->z - b->z);

						/* the camera is beside the bullet's path only if it projects onto the
						   segment between its two ends: both dot products positive */
						if (dot_ta > 0&&dot_tb > 0){

						

								nearest.x = a->x + ((b->x - a->x) * dot_ta)/(dot_ta + dot_tb);

								nearest.y = a->y + ((b->y - a->y) * dot_ta)/(dot_ta +dot_tb);

								nearest.z = a->z + ((b->z - a->z) * dot_ta)/(dot_ta +dot_tb);
								nearestvalid=true;

							}

						

						if(nearestvalid){

							if(findDistancefast(nearest,camera.position)<10&&(thirdperson==2||j!=0)){

								float gLoc[3];

								

								gLoc[0]=(camera.position.x+(nearest.x-camera.position.x))/soundscalefactor;

								gLoc[1]=(camera.position.y+(nearest.y-camera.position.y))/soundscalefactor;

								gLoc[2]=(camera.position.z+(nearest.z-camera.position.z))/soundscalefactor;

								alSourcefv(gSourceID[nearbulletsound], AL_POSITION, gLoc);

								alSourcePlay(gSourceID[nearbulletsound]);

							}

						}

					}

				}

				}

				}

			}

			}

		}

		

		if(!Button())oldbutton=0;

		

		//no laser in soul mode (z33ky, Black Shades Enhanced)
		if(lasersight&&visions==0&&person[0].whichgun!=grenade){

		for(int j=0;j<numpeople;j++){

			if(person[j].type==playertype){

				if(person[j].aiming>=1){

					//Firing

					XYZ end, aim, oldend;

					HitStruct hitstruct,temphitstruct;

					float olddistance=0;

					float distance;

					int whichhit=-1;

					if(!zoom||j!=0){

						if(person[j].whichgun==assaultrifle)aim=DoRotation(person[j].skeleton.joints[(person[j].skeleton.jointlabels[lefthand])].position-person[j].skeleton.joints[(person[j].skeleton.jointlabels[righthand])].position,0,person[j].playerrotation-2.5,0);

						if(person[j].whichgun==sniperrifle)aim=DoRotation(person[j].skeleton.joints[(person[j].skeleton.jointlabels[lefthand])].position-person[j].skeleton.joints[(person[j].skeleton.jointlabels[righthand])].position,0,person[j].playerrotation+4,0);

						/* the shotgun's laser used the sniper rifle's +4; it now points at the
						   middle of the spread the shotgun fires: 2 up, then -1 around, in
						   that order, as the firing code turns it (z33ky's fix in Black Shades
						   Enhanced, which did both turns in one DoRotation call - that turns
						   around first, and so tilts the wrong way facing along x) */
						if(person[j].whichgun==shotgun)aim=DoRotation(DoRotation(person[j].skeleton.joints[(person[j].skeleton.jointlabels[lefthand])].position-person[j].skeleton.joints[(person[j].skeleton.jointlabels[righthand])].position,2,0,0),0,person[j].playerrotation-1,0);

						if(person[j].whichgun==handgun1&&!thirdperson&&j==0)aim=DoRotation(person[j].skeleton.joints[(person[j].skeleton.jointlabels[righthand])].position-(person[j].skeleton.joints[person[j].skeleton.jointlabels[head]].position*.65+person[j].skeleton.joints[person[j].skeleton.jointlabels[neck]].position*.35),0,person[j].playerrotation-.9,0);

						if(person[j].whichgun==handgun1&&(thirdperson||j!=0))aim=DoRotation(person[j].skeleton.joints[(person[j].skeleton.jointlabels[righthand])].position-(person[j].skeleton.joints[person[j].skeleton.jointlabels[head]].position*.35+person[j].skeleton.joints[person[j].skeleton.jointlabels[neck]].position*.65),0,person[j].playerrotation-.9,0);

						if(person[j].whichgun==handgun2&&!thirdperson&&j==0)aim=DoRotation(person[j].skeleton.joints[(person[j].skeleton.jointlabels[righthand])].position-(person[j].skeleton.joints[person[j].skeleton.jointlabels[head]].position*.65+person[j].skeleton.joints[person[j].skeleton.jointlabels[neck]].position*.35),0,person[j].playerrotation-.9,0);

						if(person[j].whichgun==handgun2&&(thirdperson||j!=0))aim=DoRotation(person[j].skeleton.joints[(person[j].skeleton.jointlabels[righthand])].position-(person[j].skeleton.joints[person[j].skeleton.jointlabels[head]].position*.35+person[j].skeleton.joints[person[j].skeleton.jointlabels[neck]].position*.65),0,person[j].playerrotation-.9,0);

					}

					if(zoom&&j==0){

						start=camera.position;

						aim=facing;

					}

					Normalise(&aim);

					if(person[j].whichgun==sniperrifle){

						start=person[j].playercoords+DoRotation(person[j].skeleton.joints[(person[j].skeleton.jointlabels[lefthand])].position,0,person[j].playerrotation,0);

						start-=DoRotation(DoRotation(DoRotation(aim,0,-person[j].playerrotation,0),90,0,0),0,person[j].playerrotation,0)*(0-.4);

					}

					if(person[j].whichgun==shotgun){

						start=person[j].playercoords+DoRotation(person[j].skeleton.joints[(person[j].skeleton.jointlabels[lefthand])].position,0,person[j].playerrotation,0);

						start-=DoRotation(DoRotation(DoRotation(aim,0,-person[j].playerrotation,0),90,0,0),0,person[j].playerrotation,0)*(0-.4);

					}

					if(person[j].whichgun==handgun1){

						start=person[j].playercoords+DoRotation(person[j].skeleton.joints[(person[j].skeleton.jointlabels[rightwrist])].position,0,person[j].playerrotation,0);

						start-=DoRotation(DoRotation(DoRotation(aim,0,-person[j].playerrotation,0),90,0,0),0,person[j].playerrotation,0)*(.55-.4);

					}

					if(person[j].whichgun==handgun2){

						start=person[j].playercoords+DoRotation(person[j].skeleton.joints[(person[j].skeleton.jointlabels[rightwrist])].position,0,person[j].playerrotation,0);

						start-=DoRotation(DoRotation(DoRotation(aim,0,-person[j].playerrotation,0),90,0,0),0,person[j].playerrotation,0)*(.55-.4);

					}

					if(person[j].whichgun==assaultrifle){

						start=person[j].playercoords+DoRotation(person[j].skeleton.joints[(person[j].skeleton.jointlabels[lefthand])].position,0,person[j].playerrotation,0);

						start-=DoRotation(DoRotation(DoRotation(aim,0,-person[j].playerrotation,0),90,0,0),0,person[j].playerrotation,0)*(.25-.4);

					}

					/* the laser lands where the bullet will, which is now on
					   the crosshair rather than beside it */
					if(j==0&&pointeraim&&!zoom)PointerShotStart(&start);

					end=start+aim*1000;

					//Blocks

					wallhit=0;

					beginx=(person[j].playercoords.x+block_spacing/2)/block_spacing-2;

					if(beginx<0)beginx=0;

					beginz=(person[j].playercoords.z+block_spacing/2)/block_spacing-2;

					if(beginz<0)beginz=0;

					

					endx=(person[j].playercoords.x+block_spacing/2)/block_spacing+2;

					if(endx>num_blocks-1)endx=num_blocks-1;

					endz=(person[j].playercoords.z+block_spacing/2)/block_spacing+2;

					if(endz>num_blocks-1)endz=num_blocks-1;

					

					if(beginx<endx&&beginz<endz)

					finalwallhit=0;

					for(int i=beginx;i<=endx;i++)

						for(int j=beginz;j<=endz;j++){

							move=0;

							move.x=i*block_spacing;

							move.z=j*block_spacing;

							whichtri=blocks[citytype[i][j]].LineCheck2(start,end,&wallhit,move,cityrotation[i][j]*90);

							if(whichtri!=-1){

								end=wallhit;

								finalwallhit=wallhit;

							}

						}

					wallhit=0;

					wallhit.x=camera.position.x;

					wallhit.z=camera.position.z;

					whichtri=Bigstreet.LineCheck2(start,end,&wallhit,wallhit,0);

					if(whichtri!=-1){

						end=wallhit;

						finalwallhit=wallhit;

					}

					//People

					whichhit=-1;

					for(int i=0;i<numpeople;i++){

						if(i!=j&&findDistancefast(person[j].playercoords,person[i].playercoords)<20000){

							temphitstruct=person[i].BulletCollideWithPlayer(i, start, end);	

							if(temphitstruct.collision){

								distance=findDistancefast(start,temphitstruct.hitlocation);

								if(distance<olddistance||whichhit==-1){

									end=temphitstruct.hitlocation;

									olddistance=distance;

									hitstruct=temphitstruct;

									whichhit=i;

								}

							}

						}

					}

					aimer[0]=start;

					aimer[1]=end;

					sprites.MakeSprite(bulletinstant, .4, 1, 0, 0, aimer[0], aimer[1], .2);

					

				}

			}

		}

		}

		

		//Snow

		snowdelay-=multiplier;

		while(snowdelay<0&&environment==snowy_environment){

			snowdelay+=1/precipitationdensity*2;

			velocity=0;

			velocity.y=-5;

			start=camera.position;

			start.y+=precipitationvert;

			start.x+=Random()%(int)precipitationhorz;

			start.z+=Random()%(int)precipitationhorz;

			sprites.MakeSprite(snowsprite, 1, 1, 1, 1, start, velocity, 1.01);

		}

		while(snowdelay<0&&environment==rainy_environment){

			snowdelay+=1/precipitationdensity/4;

			velocity=0;

			velocity.y=-100;

			start=camera.position;

			start.y+=precipitationvert;

			start.x+=Random()%(int)precipitationhorz*.5;

			start.z+=Random()%(int)precipitationhorz*.5;

			sprites.MakeSprite(rainsprite, .5, 1, 1, 1, start, velocity, 2.00);

		}			

		

		//Grenade collision

		int wherex,wherey,whichsound,severwhat;

		bool impact;

		for(int i=0;i<sprites.howmanysprites;i++){

			if(sprites.type[i]==grenadesprite||sprites.type[i]==spoonsprite||sprites.type[i]==pinsprite){

				impact=0;

				if(sprites.type[i]!=grenadesprite){

					sprites.brightness[i]-=multiplier*.2;

				}

						

				if(findLengthfast(sprites.velocity[i])>0){

					wherex=CityBlock(sprites.location[i].x);

					wherey=CityBlock(sprites.location[i].z);

					move=0;

					move.x=wherex*block_spacing;

					move.z=wherey*block_spacing;

					whichtri=blocks[citytype[wherex][wherey]].LineCheck2(sprites.oldlocation[i],sprites.location[i],&wallhit,move,cityrotation[wherex][wherey]*90);

					if(whichtri!=-1){

						impact=1;

						normalrotated=DoRotation(blocks[citytype[wherex][wherey]].normals[whichtri],0,cityrotation[wherex][wherey]*90,0);

						if(sprites.size[i]>1)decals.MakeDecal(crater, wallhit,9,normalrotated, whichtri, &blocks[citytype[wherex][wherey]], move, cityrotation[wherex][wherey]*90);

						sprites.location[i]=wallhit+normalrotated*.02;

						ReflectVector(&sprites.velocity[i],&normalrotated);

						sprites.velocity[i]*=.3;

						if(sprites.type[i]==grenadesprite){

							float gLoc[3];

							gLoc[0]=(((sprites.location[i].x)-camera.position.x)/findLengthfast(sprites.velocity[i])*5+camera.position.x)/soundscalefactor;

							gLoc[1]=(((sprites.location[i].y)-camera.position.y)/findLengthfast(sprites.velocity[i])*5+camera.position.y)/soundscalefactor;

							gLoc[2]=(((sprites.location[i].z)-camera.position.z)/findLengthfast(sprites.velocity[i])*5+camera.position.z)/soundscalefactor;

							whichsound=abs(Random()%2);

							alSourcefv(gSourceID[bouncesound+whichsound], AL_POSITION, gLoc);

							if(sprites.size[i]<=1)alSourcePlay(gSourceID[bouncesound+whichsound]);			

						}

						if(findLengthfast(sprites.velocity[i])<=10)sprites.velocity[i]=0;

					}

					if(sprites.location[i].y<0){

						impact=1;

						sprites.velocity[i].y*=-1;

						sprites.velocity[i]*=.3;

						sprites.location[i].y=0;

						if(sprites.type[i]==grenadesprite){

							if(sprites.size[i]>1){

								move=0;

								sprites.location[i].y=-.5;

								XYZ normish;

								normish=0;

								normish.y=1;

								decals.MakeDecal(crater, sprites.location[i],9,normish, 0, &blocks[citytype[wherex][wherey]], move, 0);

							}

							float gLoc[3];

							gLoc[0]=(((sprites.location[i].x)-camera.position.x)/findLengthfast(sprites.velocity[i])*5+camera.position.x)/soundscalefactor;

							gLoc[1]=(((sprites.location[i].y)-camera.position.y)/findLengthfast(sprites.velocity[i])*5+camera.position.y)/soundscalefactor;

							gLoc[2]=(((sprites.location[i].z)-camera.position.z)/findLengthfast(sprites.velocity[i])*5+camera.position.z)/soundscalefactor;

							whichsound=abs(Random()%2);

							alSourcefv(gSourceID[bouncesound+whichsound], AL_POSITION, gLoc);

							if(sprites.size[i]<=1)alSourcePlay(gSourceID[bouncesound+whichsound]);			

						}

						if(findLengthfast(sprites.velocity[i])<=10)sprites.velocity[i]=0;

					}

					if(sprites.type[i]==grenadesprite&&findLengthfast(sprites.velocity[i])>20){

						HitStruct hitstruct;

						for(int j=0;j<numpeople;j++){

							if((j!=0||sprites.brightness[i]<.9)&&person[j].existing){

								hitstruct=person[j].BulletCollideWithPlayer(j, sprites.oldlocation[i], sprites.location[i]);	

								if(hitstruct.collision){

									impact=1;

									sprites.location[i]=hitstruct.hitlocation;

									float gLoc[3];

									gLoc[0]=(((sprites.location[i].x)-camera.position.x)/findLengthfast(sprites.velocity[i])*5+camera.position.x)/soundscalefactor;

									gLoc[1]=(((sprites.location[i].y)-camera.position.y)/findLengthfast(sprites.velocity[i])*5+camera.position.y)/soundscalefactor;

									gLoc[2]=(((sprites.location[i].z)-camera.position.z)/findLengthfast(sprites.velocity[i])*5+camera.position.z)/soundscalefactor;

									if(person[j].skeleton.free<1){

									if((hitstruct.joint1->label==head||hitstruct.joint2->label==head)&&person[j].type!=zombietype){

										alSourcefv(gSourceID[headwhacksound], AL_POSITION, gLoc);

										if(sprites.size[i]<=1)alSourcePlay(gSourceID[headwhacksound]);

										person[j].skeleton.free=1;

										person[j].longdead=1;

										for(int k=0;k<person[j].skeleton.num_joints;k++){

											person[j].skeleton.joints[k].position=DoRotation(person[j].skeleton.joints[k].position,0,person[j].playerrotation,0);

											person[j].skeleton.joints[k].position+=person[j].playercoords;

											person[j].skeleton.joints[k].realoldposition=person[j].skeleton.joints[k].position;

											person[j].skeleton.joints[k].velocity=person[j].velocity;

											person[j].skeleton.joints[k].velocity.x+=abs(Random()%10)-5;

											person[j].skeleton.joints[k].velocity.y+=abs(Random()%10)-5;

											person[j].skeleton.joints[k].velocity.z+=abs(Random()%10)-5;

										}

										hitstruct.joint1->velocity+=sprites.velocity[i];

										hitstruct.joint2->velocity+=sprites.velocity[i];

										if(person[j].type==civiliantype)civkills++;

										if(person[j].type==eviltype)goodkills++;

									}else{

										float totalarea=0;   /* was uninitialised: the two
									                        other copies of this loop
									                        (lines ~3121, ~3651) zero it */

										alSourcefv(gSourceID[bodywhacksound], AL_POSITION, gLoc);

										if(sprites.size[i]<=1)alSourcePlay(gSourceID[bodywhacksound]);

										person[j].skeleton.offsetted=1;

										for(int k=0;k<person[j].skeleton.num_joints;k++){

											if(findDistancefast(DoRotation(person[j].skeleton.joints[k].position,0,person[j].playerrotation,0)+person[j].playercoords,hitstruct.hitlocation)<200){

												totalarea+=HitWeight(DoRotation(person[j].skeleton.joints[k].position,0,person[j].playerrotation,0)+person[j].playercoords,hitstruct.hitlocation);

											}

										}

										float offsetlength;

										for(int k=0;k<person[j].skeleton.num_joints;k++){

											if(findDistancefast(DoRotation(person[j].skeleton.joints[k].position,0,person[j].playerrotation,0)+person[j].playercoords,hitstruct.hitlocation)<200){

												person[j].skeleton.joints[k].offset+=DoRotation(sprites.velocity[i]*.1*(HitWeight(DoRotation(person[j].skeleton.joints[k].position,0,person[j].playerrotation,0)+person[j].playercoords,hitstruct.hitlocation)/totalarea*10),0,-person[j].playerrotation,0);

											}

											offsetlength=findLengthfast(person[j].skeleton.joints[k].offset);

											if(offsetlength>9){

												Normalise(&person[j].skeleton.joints[k].offset);

												person[j].skeleton.joints[k].offset*=3;

											}

										}

									}}

									sprites.velocity[i]*=-.3;

								}

							}

						}

					}

					sprites.oldlocation[i]=sprites.location[i];

				}

				//Explode

				if(sprites.type[i]==grenadesprite){

					sprites.brightness[i]-=multiplier*.3;

					if(sprites.brightness[i]<=0||(impact&&sprites.size[i]>1)){

						sprites.brightness[i]=0;

						sprites.MakeSprite(smokesprite, 1, 1, 1, 1, sprites.location[i], facing*0, 60);

						sprites.MakeSprite(muzzleflashsprite, 1, 1, 1, 1, sprites.location[i], facing*0, 9);

						float gLoc[3];

						gLoc[0]=(((sprites.location[i].x)-camera.position.x)/3+camera.position.x)/soundscalefactor;

						gLoc[1]=(((sprites.location[i].y)-camera.position.y)/3+camera.position.y)/soundscalefactor;

						gLoc[2]=(((sprites.location[i].z)-camera.position.z)/3+camera.position.z)/soundscalefactor;

						alSourcefv(gSourceID[explosionsound], AL_POSITION, gLoc);

						alSourcePlay(gSourceID[explosionsound]);

						XYZ relation;

						camerashake=1-findDistance(person[0].playercoords,sprites.location[i])/200;

						//if(!sprites.size[i]>1){

							overpoint=sprites.location[i];

							overpoint.y+=3000;

							underpoint=sprites.location[i];

							underpoint.y-=3000;

							move=0;

							wherex=CityBlock(sprites.location[i].x);

							wherey=CityBlock(sprites.location[i].z);

							move.x=wherex*block_spacing;

							move.z=wherey*block_spacing;

							XYZ temp;

							whichtri=sidewalkcollide.LineCheck2(overpoint,underpoint,&temp,move,cityrotation[wherex][wherey]*90);

							XYZ normish;

							normish=0;

							normish.y=1;

							if(whichtri>=0){

								decals.MakeDecal(crater, sprites.location[i],9,normish, 0, &sidewalkcollide, move, cityrotation[wherex][wherey]*90);

							}

							if(whichtri==-1){

								temp=sprites.location[i];

								temp.y=-.5;

								move=0;

								decals.MakeDecal(crater, sprites.location[i],9,normish, 0, &sidewalkcollide, move, 0);

							}

						//}

						for(int k=0;k<numpeople;k++){		

							if(person[k].existing&&(person[k].longdead!=-1||person[k].skeleton.free<1)){		

							if((findDistancefast(person[k].playercoords,sprites.location[i])<700&&person[k].skeleton.free<1)||(findDistancefast(person[k].averageloc,sprites.location[i])<700&&person[k].skeleton.free>=1)){

							if(person[k].skeleton.free!=1){

							if(person[k].type==civiliantype)civkills++;

							if(person[k].type==eviltype)goodkills++;

							person[k].skeleton.free=1;

							person[k].killtargetvisible=0;

							if((findDistancefast(person[k].playercoords,sprites.location[i])<600&&person[k].skeleton.free<1)||(findDistancefast(person[k].averageloc,sprites.location[i])<600&&person[k].skeleton.free>=1)||person[k].type==playertype){

								person[k].health-=100;

								person[k].bleeding=1;

							}

							person[k].DoAnimations(k);

							person[k].longdead=1;

							

							person[k].bleeddelay=1;

							person[k].bjoint1=&person[k].skeleton.joints[person[k].skeleton.jointlabels[head]];

							person[k].bjoint2=&person[k].skeleton.joints[person[k].skeleton.jointlabels[neck]];

							

							

							for(int j=0;j<person[k].skeleton.num_joints;j++){

								person[k].skeleton.joints[j].position=DoRotation(person[k].skeleton.joints[j].position,0,person[k].playerrotation,0);

								person[k].skeleton.joints[j].position+=person[k].playercoords;

								person[k].skeleton.joints[j].realoldposition=person[k].skeleton.joints[j].position;

								person[k].skeleton.joints[j].velocity=DoRotation(person[k].skeleton.joints[j].velocity,0,person[k].playerrotation,0);

								person[k].skeleton.joints[j].velocity+=person[k].velocity;

								person[k].skeleton.joints[j].velocity.x+=abs(Random()%20)-10;

								person[k].skeleton.joints[j].velocity.y+=abs(Random()%20)-10;

								person[k].skeleton.joints[j].velocity.z+=abs(Random()%20)-10;

							}}

							for(int j=0;j<person[k].skeleton.num_joints;j++){

								relation=person[k].skeleton.joints[j].position-sprites.location[i];

								Normalise(&relation);

								if(findDistance(person[k].skeleton.joints[j].position,sprites.location[i])>1)person[k].skeleton.joints[j].velocity+=relation/findDistance(person[k].skeleton.joints[j].position,sprites.location[i])*300;

								else person[k].skeleton.joints[j].velocity+=relation*300;

							}

							person[k].longdead=1;

							for(int j=0;j<person[k].skeleton.num_joints;j++){

								//Sever stuff

								if(findLengthfast(person[k].skeleton.joints[j].velocity)>1500&&person[k].skeleton.joints[j].existing==1&&abs(Random()%3)!=1){

									if(blood)sprites.MakeSprite(bloodspritedown, .8, 1, .2, .2,person[k].skeleton.joints[j].position, person[k].skeleton.joints[j].velocity/3, 9);

									person[k].skeleton.DeleteJoint(j);

									person[k].skeleton.broken=2;

									person[k].health=-10000;

									person[k].skeleton.joints[j].existing=0;

								}

							}

							}

							}

						}

									

					}

				}

			}

		}

		

		//camera shake

		camerashake-=multiplier;

		if(camerashake<0)camerashake=0;

		

		//camera position

		XYZ average;

		if(!zoom)average=person[0].skeleton.joints[(person[0].skeleton.jointlabels[head])].position*(person[0].aimamount/2+.5)+person[0].skeleton.joints[(person[0].skeleton.jointlabels[neck])].position*((1-person[0].aimamount)/2);

		if(zoom)average=person[0].skeleton.joints[(person[0].skeleton.jointlabels[righthand])].position;

		

		if(person[0].skeleton.free==0&&thirdperson!=2)camera.position=person[0].playercoords+DoRotation(average,0,person[0].playerrotation,0);

		if(person[0].skeleton.free==1&&thirdperson!=2)camera.position=average;

		

		//Restraints

		if(camera.position.y<.1)camera.position.y=.1;

		

		if(thirdperson!=2){

			oldrot=camera.visrotation;

			oldrot2=camera.visrotation2;

		}

		

		//Kill count

		for(int i=0;i<numpeople;i++){

			if(person[i].oldhealth>0&&person[i].health<=0){

				if(i==1)alSourcePlay(gSourceID[losesound]);	

				if(person[i].type==civiliantype){

					alSourcePlay(gSourceID[disguisekillsound]);

					score-=300;	

				}

				if(person[i].type==eviltype){

					alSourcePlay(gSourceID[soulinsound]);	

					score+=75;

					if(person[i].whichgun==knife)score+=50;

				}

				person[i].firstlongdead=0;

			}

			person[i].oldhealth=person[i].health;

		}

		

		if(slomo==2){

			psychicpower-=multiplier*15;

			if(psychicpower<0){

				soundscalefactor=soundscalefactordefault;

				alSourceStop(gSourceID[whichsong]);

				alSourcef(gSourceID[whichsong], AL_MIN_GAIN, 0);

				alSourcef(gSourceID[whichsong], AL_MAX_GAIN, 0);

				if(person[0].whichgun==knife)whichsong=knifesong;

				if(person[0].whichgun!=knife)whichsong=shootsong;

				if(type==zombie_type)whichsong=zombiesong;

				alSourcef(gSourceID[whichsong], AL_PITCH, 1);

				alSourcePlay(gSourceID[whichsong]);

				alSourcef(gSourceID[whichsong], AL_MIN_GAIN, 1);

				alSourcef(gSourceID[whichsong], AL_MAX_GAIN, 1);

				slomo=0;

				alSourcePlay(gSourceID[soulinsound]);

				psychicpower=0;

				flashamount=.5;

				flashr=1;flashg=1;flashb=1;

			}

		}

		psychicpower+=multiplier*5;

		if(psychicpower>10)psychicpower=10;

		

		//3d sound

		ALint tempInt;

		float gLoc[3];

		gLoc[0]=camera.position.x/soundscalefactor;

		gLoc[1]=camera.position.y/soundscalefactor;

		gLoc[2]=camera.position.z/soundscalefactor;

		//Set listener position

		alListenerfv(AL_POSITION, gLoc);

		

		//Set orientation with forward and up vectors

		XYZ upvector;

		upvector=0;

		upvector.z=-1;

		

		upvector=DoRotation(upvector,-camera.rotation2+90,0,0);

		upvector=DoRotation(upvector,0,0-camera.rotation,0);

		

		/* OpenAL builds the listener's "right" as at x up (right-handed).  The
		   original negated x here, which with any spec-following OpenAL (MiniAL,
		   OpenAL Soft, Apple's) swapped left and right whenever you faced along
		   the x axis and centred everything on the diagonals. */
		float ori[6];

	   	ori[0] = facing.x;

	   	ori[1] = facing.y;

	   	ori[2] = facing.z;

	   	ori[3] = upvector.x;

	   	ori[4] = upvector.y;

	   	ori[5] = upvector.z;

		alListenerfv(AL_ORIENTATION, ori);

		

		if(person[0].currentanimation==throwanim||person[0].currentanimation==diveanim||paused){

			MoveMouse(oldmouseloc.h,oldmouseloc.v,&mouseloc);

			GetMouse(&mouseloc);

		}

		oldmouseloc=mouseloc;

		

		if(score<0)score=0;

	}

}
