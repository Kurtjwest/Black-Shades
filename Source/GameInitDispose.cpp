#include <ctype.h>
#include "Textures.h"

#ifndef OS9 
#include <time.h>
#include <new>
#include <string>
#endif

#include "Game.h"	


extern unsigned int gSourceID[100];

extern unsigned int gSampleSet[100];

extern Camera camera;

extern Skeleton testskeleton;

extern Sprites sprites;

extern Decals decals;

/*********************> InitGame()<*****/

extern Model skeletonmodels[10];

extern Model gunmodels[11];

extern Costume costume[10];

extern Animation animation[30];

extern int visions;

extern float rad2deg;

extern Fog fog;

extern bool blood;

extern float fogcolorr;

extern float fogcolorg;

extern float fogcolorb;

extern int environment;

extern float precipitationhorz;

extern float precipitationvert;

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



void LoadSounds(bool musictoggle);

void LoadSounds(bool musictoggle)

{
	char *pBuffer1;

	unsigned int lBuffer1Len;

	ALenum formatBuffer1;

	ALsizei freqBuffer1;
	
	Str255 sFileName;

	short fRefNum;

	SInt16 iNumSources, iNumSampleSets;


	// initialize the audio output (MiniAL, or the system OpenAL)

	Audio_Init();

	

	

	// load up some audio data...

	// generate ten OpenAL sample sets and two sources

	iNumSources = 37;

	iNumSampleSets = 37;

	alGenSources(iNumSources, &gSourceID[0]);

	alGenBuffers(iNumSampleSets, &gSampleSet[0]);

	LoadOGG_CFH((const char *)":Data:Sounds:underwater.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[visionsound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[visionsound], AL_BUFFER, gSampleSet[visionsound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	

	LoadOGG_CFH((const char *)":Data:Sounds:soulin.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[soulinsound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[soulinsound], AL_BUFFER, gSampleSet[soulinsound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	

	LoadOGG_CFH((const char *)":Data:Sounds:soulout.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[souloutsound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[souloutsound], AL_BUFFER, gSampleSet[souloutsound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	

	LoadOGG_CFH((const char *)":Data:Sounds:footstep1.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[footstepsound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[footstepsound], AL_BUFFER, gSampleSet[footstepsound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1); 

	

	LoadOGG_CFH((const char *)":Data:Sounds:footstep2.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[footstepsound+1], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[footstepsound+1], AL_BUFFER, gSampleSet[footstepsound+1]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	

	LoadOGG_CFH((const char *)":Data:Sounds:footstep3.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[footstepsound+2], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[footstepsound+2], AL_BUFFER, gSampleSet[footstepsound+2]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	

	LoadOGG_CFH((const char *)":Data:Sounds:footstep4.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[footstepsound+3], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[footstepsound+3], AL_BUFFER, gSampleSet[footstepsound+3]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	

	LoadOGG_CFH((const char *)":Data:Sounds:footstep5.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[footstepsound+4], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[footstepsound+4], AL_BUFFER, gSampleSet[footstepsound+4]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	

	LoadOGG_CFH((const char *)":Data:Sounds:bodyland.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[bodylandsound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[bodylandsound], AL_BUFFER, gSampleSet[bodylandsound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);



	LoadOGG_CFH((const char *)":Data:Sounds:headland.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[headlandsound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[headlandsound], AL_BUFFER, gSampleSet[headlandsound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	

	LoadOGG_CFH((const char *)":Data:Sounds:sniperrifle.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[riflesound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[riflesound], AL_BUFFER, gSampleSet[riflesound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	

	LoadOGG_CFH((const char *)":Data:Sounds:BodyHit.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[bodyhitsound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[bodyhitsound], AL_BUFFER, gSampleSet[bodyhitsound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);



	LoadOGG_CFH((const char *)":Data:Sounds:WallHit.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[wallhitsound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[wallhitsound], AL_BUFFER, gSampleSet[wallhitsound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	

	LoadOGG_CFH((const char *)":Data:Sounds:machinegun.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[machinegunsound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[machinegunsound], AL_BUFFER, gSampleSet[machinegunsound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	

	LoadOGG_CFH((const char *)":Data:Sounds:Nearbullet.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[nearbulletsound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[nearbulletsound], AL_BUFFER, gSampleSet[nearbulletsound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	

	LoadOGG_CFH((const char *)":Data:Sounds:riflewhack.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[headwhacksound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[headwhacksound], AL_BUFFER, gSampleSet[headwhacksound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);



	LoadOGG_CFH((const char *)":Data:Sounds:headshot.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[headshotsound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[headshotsound], AL_BUFFER, gSampleSet[headshotsound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	

	LoadOGG_CFH((const char *)":Data:Sounds:reload.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[reloadsound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[reloadsound], AL_BUFFER, gSampleSet[reloadsound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	

	LoadOGG_CFH((const char *)":Data:Sounds:click.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[clicksound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[clicksound], AL_BUFFER, gSampleSet[clicksound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	

	LoadOGG_CFH((const char *)":Data:Sounds:SW.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[pistol1sound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[pistol1sound], AL_BUFFER, gSampleSet[pistol1sound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	

	LoadOGG_CFH((const char *)":Data:Sounds:glock.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[pistol2sound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[pistol2sound], AL_BUFFER, gSampleSet[pistol2sound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	

	LoadOGG_CFH((const char *)":Data:Sounds:pinpull.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[pinpullsound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[pinpullsound], AL_BUFFER, gSampleSet[pinpullsound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	

	LoadOGG_CFH((const char *)":Data:Sounds:pinreplace.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[pinreplacesound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[pinreplacesound], AL_BUFFER, gSampleSet[pinreplacesound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	

	LoadOGG_CFH((const char *)":Data:Sounds:handlerelease.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[grenadethrowsound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[grenadethrowsound], AL_BUFFER, gSampleSet[grenadethrowsound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	

	LoadOGG_CFH((const char *)":Data:Sounds:bounce.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[bouncesound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[bouncesound], AL_BUFFER, gSampleSet[bouncesound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	

	LoadOGG_CFH((const char *)":Data:Sounds:bounce2.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[bounce2sound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[bounce2sound], AL_BUFFER, gSampleSet[bounce2sound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	

	LoadOGG_CFH((const char *)":Data:Sounds:explosion.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[explosionsound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[explosionsound], AL_BUFFER, gSampleSet[explosionsound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	

	LoadOGG_CFH((const char *)":Data:Sounds:headland.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[bodywhacksound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[bodywhacksound], AL_BUFFER, gSampleSet[bodywhacksound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);



	LoadOGG_CFH((const char *)":Data:Sounds:rain.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[rainsound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[rainsound], AL_BUFFER, gSampleSet[rainsound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);



	LoadOGG_CFH((const char *)":Data:Sounds:Lose.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[losesound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[losesound], AL_BUFFER, gSampleSet[losesound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);



	LoadOGG_CFH((const char *)":Data:Sounds:Disguisekill.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[disguisekillsound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[disguisekillsound], AL_BUFFER, gSampleSet[disguisekillsound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);



	LoadOGG_CFH((const char *)":Data:Sounds:knifeslash.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[knifeslashsound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[knifeslashsound], AL_BUFFER, gSampleSet[knifeslashsound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);



	LoadOGG_CFH((const char *)":Data:Sounds:shotgun.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[shotgunsound], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[shotgunsound], AL_BUFFER, gSampleSet[shotgunsound]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);



	if(musictoggle){

	LoadOGG_CFH((const char *)":Data:Sounds:mainmenusong.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[mainmenusong], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[mainmenusong], AL_BUFFER, gSampleSet[mainmenusong]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);



	LoadOGG_CFH((const char *)":Data:Sounds:shootsong.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[shootsong], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[shootsong], AL_BUFFER, gSampleSet[shootsong]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);



	LoadOGG_CFH((const char *)":Data:Sounds:zombiesong.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[zombiesong], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[zombiesong], AL_BUFFER, gSampleSet[zombiesong]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);



	LoadOGG_CFH((const char *)":Data:Sounds:knifesong.ogg",&formatBuffer1, (void **) &pBuffer1,&lBuffer1Len,&freqBuffer1);

	alBufferData(gSampleSet[knifesong], formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	alSourcei(gSourceID[knifesong], AL_BUFFER, gSampleSet[knifesong]);

	FreeOGG(formatBuffer1, pBuffer1, lBuffer1Len, freqBuffer1);

	}




	float gLoc[3];

	

	gLoc[0]=0;

	gLoc[1]=0;

	gLoc[2]=0;

	

	alSourcefv(gSourceID[visionsound], AL_POSITION, gLoc);

	alSourcei(gSourceID[visionsound], AL_LOOPING, 1);

	alSourcef(gSourceID[visionsound], AL_MIN_GAIN, 1);

	

	alSourcefv(gSourceID[soulinsound], AL_POSITION, gLoc);

	alSourcei(gSourceID[soulinsound], AL_LOOPING, 0);

	alSourcef(gSourceID[soulinsound], AL_MIN_GAIN, 1);

	

	alSourcefv(gSourceID[souloutsound], AL_POSITION, gLoc);

	alSourcei(gSourceID[souloutsound], AL_LOOPING, 0);

	alSourcef(gSourceID[souloutsound], AL_MIN_GAIN, 1);

	

	for(int i=0;i<5;i++){

		alSourcefv(gSourceID[footstepsound+i], AL_POSITION, gLoc);

		alSourcei(gSourceID[footstepsound+i], AL_LOOPING, 0);

		alSourcef(gSourceID[footstepsound+i], AL_MIN_GAIN, 0);

	}	

	

	alSourcefv(gSourceID[bodylandsound], AL_POSITION, gLoc);

	alSourcei(gSourceID[bodylandsound], AL_LOOPING, 0);

	alSourcef(gSourceID[bodylandsound], AL_MIN_GAIN, 0);

	

	alSourcefv(gSourceID[headlandsound], AL_POSITION, gLoc);

	alSourcei(gSourceID[headlandsound], AL_LOOPING, 0);

	alSourcef(gSourceID[headlandsound], AL_MIN_GAIN, 0);

	

	alSourcefv(gSourceID[riflesound], AL_POSITION, gLoc);

	alSourcei(gSourceID[riflesound], AL_LOOPING, 0);

	alSourcef(gSourceID[riflesound], AL_MIN_GAIN, 0);

	

	alSourcefv(gSourceID[bodyhitsound], AL_POSITION, gLoc);

	alSourcei(gSourceID[bodyhitsound], AL_LOOPING, 0);

	alSourcef(gSourceID[bodyhitsound], AL_MIN_GAIN, .1);

	

	alSourcefv(gSourceID[wallhitsound], AL_POSITION, gLoc);

	alSourcei(gSourceID[wallhitsound], AL_LOOPING, 0);

	alSourcef(gSourceID[wallhitsound], AL_MIN_GAIN, 0);

	alSourcef(gSourceID[wallhitsound], AL_MAX_GAIN, .6);

	

	alSourcefv(gSourceID[machinegunsound], AL_POSITION, gLoc);

	alSourcei(gSourceID[machinegunsound], AL_LOOPING, 0);

	alSourcef(gSourceID[machinegunsound], AL_MIN_GAIN,0);

	

	alSourcefv(gSourceID[nearbulletsound], AL_POSITION, gLoc);

	alSourcei(gSourceID[nearbulletsound], AL_LOOPING, 0);

	alSourcef(gSourceID[nearbulletsound], AL_MIN_GAIN,0);

	

	alSourcefv(gSourceID[headwhacksound], AL_POSITION, gLoc);

	alSourcei(gSourceID[headwhacksound], AL_LOOPING, 0);

	alSourcef(gSourceID[headwhacksound], AL_MIN_GAIN,0);

	

	alSourcefv(gSourceID[headshotsound], AL_POSITION, gLoc);

	alSourcei(gSourceID[headshotsound], AL_LOOPING, 0);

	alSourcef(gSourceID[headshotsound], AL_MIN_GAIN, 0);

	

	alSourcefv(gSourceID[reloadsound], AL_POSITION, gLoc);

	alSourcei(gSourceID[reloadsound], AL_LOOPING, 0);

	alSourcef(gSourceID[reloadsound], AL_MIN_GAIN, 0);

	

	alSourcefv(gSourceID[clicksound], AL_POSITION, gLoc);

	alSourcei(gSourceID[clicksound], AL_LOOPING, 0);

	alSourcef(gSourceID[clicksound], AL_MIN_GAIN, 0);

	

	alSourcefv(gSourceID[pistol1sound], AL_POSITION, gLoc);

	alSourcei(gSourceID[pistol1sound], AL_LOOPING, 0);

	alSourcef(gSourceID[pistol1sound], AL_MIN_GAIN, 0);

	

	alSourcefv(gSourceID[pistol2sound], AL_POSITION, gLoc);

	alSourcei(gSourceID[pistol2sound], AL_LOOPING, 0);

	alSourcef(gSourceID[pistol2sound], AL_MIN_GAIN, 0);

	

	alSourcefv(gSourceID[pinpullsound], AL_POSITION, gLoc);

	alSourcei(gSourceID[pinpullsound], AL_LOOPING, 0);

	alSourcef(gSourceID[pinpullsound], AL_MIN_GAIN,0);

	

	alSourcefv(gSourceID[pinreplacesound], AL_POSITION, gLoc);

	alSourcei(gSourceID[pinreplacesound], AL_LOOPING, 0);

	alSourcef(gSourceID[pinreplacesound], AL_MIN_GAIN,0);

	

	alSourcefv(gSourceID[grenadethrowsound], AL_POSITION, gLoc);

	alSourcei(gSourceID[grenadethrowsound], AL_LOOPING, 0);

	alSourcef(gSourceID[grenadethrowsound], AL_MIN_GAIN,0);

	

	alSourcefv(gSourceID[bouncesound], AL_POSITION, gLoc);

	alSourcei(gSourceID[bouncesound], AL_LOOPING, 0);

	alSourcef(gSourceID[bouncesound], AL_MIN_GAIN,0);

	

	alSourcefv(gSourceID[bounce2sound], AL_POSITION, gLoc);

	alSourcei(gSourceID[bounce2sound], AL_LOOPING, 0);

	alSourcef(gSourceID[bounce2sound], AL_MIN_GAIN,0);

	

	alSourcefv(gSourceID[explosionsound], AL_POSITION, gLoc);

	alSourcei(gSourceID[explosionsound], AL_LOOPING, 0);

	alSourcef(gSourceID[explosionsound], AL_MIN_GAIN,0);



	alSourcefv(gSourceID[bodywhacksound], AL_POSITION, gLoc);

	alSourcei(gSourceID[bodywhacksound], AL_LOOPING, 0);

	alSourcef(gSourceID[bodywhacksound], AL_MIN_GAIN, 0);

	

	alSourcefv(gSourceID[rainsound], AL_POSITION, gLoc);

	alSourcei(gSourceID[rainsound], AL_LOOPING, 1);

	alSourcef(gSourceID[rainsound], AL_MIN_GAIN, .3);

	

	alSourcefv(gSourceID[losesound], AL_POSITION, gLoc);

	alSourcei(gSourceID[losesound], AL_LOOPING, 0);

	alSourcef(gSourceID[losesound], AL_MIN_GAIN, 1);

	

	alSourcefv(gSourceID[disguisekillsound], AL_POSITION, gLoc);

	alSourcei(gSourceID[disguisekillsound], AL_LOOPING, 0);

	alSourcef(gSourceID[disguisekillsound], AL_MIN_GAIN, 1);

	

	alSourcefv(gSourceID[knifeslashsound], AL_POSITION, gLoc);

	alSourcei(gSourceID[knifeslashsound], AL_LOOPING, 0);

	alSourcef(gSourceID[knifeslashsound], AL_MIN_GAIN,0);

	

	alSourcefv(gSourceID[shotgunsound], AL_POSITION, gLoc);

	alSourcei(gSourceID[shotgunsound], AL_LOOPING, 0);

	alSourcef(gSourceID[shotgunsound], AL_MIN_GAIN, 0);

	

	alSourcefv(gSourceID[knifesong], AL_POSITION, gLoc);

	alSourcei(gSourceID[knifesong], AL_LOOPING, 1);

	alSourcef(gSourceID[knifesong], AL_MIN_GAIN, 1);

	

	alSourcefv(gSourceID[mainmenusong], AL_POSITION, gLoc);

	alSourcei(gSourceID[mainmenusong], AL_LOOPING, 1);

	alSourcef(gSourceID[mainmenusong], AL_MIN_GAIN, 1);

	

	alSourcefv(gSourceID[zombiesong], AL_POSITION, gLoc);

	alSourcei(gSourceID[zombiesong], AL_LOOPING, 1);

	alSourcef(gSourceID[zombiesong], AL_MIN_GAIN, 1);

	

	alSourcefv(gSourceID[shootsong], AL_POSITION, gLoc);

	alSourcei(gSourceID[shootsong], AL_LOOPING, 1);

	alSourcef(gSourceID[shootsong], AL_MIN_GAIN, 1);

	

	

}



void Game::LoadingScreen(float percent)

{

	float size=1;

	/* like the menu: the window may have changed size (or gone fullscreen)
	   since the GL context was created */
	PlatformSetViewport();

	glLoadIdentity();

	//Clear to black

	glClearColor(0,0,0,1);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	

	//Background

	

	glDisable(GL_TEXTURE_2D);

	glDisable(GL_DEPTH_TEST);							// Disables Depth Testing

	glDisable(GL_CULL_FACE);

	glDisable(GL_LIGHTING);

	glDepthMask(0);

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix

	glPushMatrix();										// Store The Projection Matrix

	glLoadIdentity();									// Reset The Projection Matrix

	glOrtho(0,640,0,480,-100,100);						// Set Up An Ortho Screen

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix

	glPushMatrix();										// Store The Modelview Matrix

	for(int i=19;i>=0;i--){

		glLoadIdentity();								// Reset The Modelview Matrix

		glTranslatef(120-i*1,190-i*1,0);

		glScalef(400+i*2,30+i*2,1);

		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_BLEND);

		if(i)glColor4f(1-(float)i/20-percent/100,1-(float)i/20-percent/100,1-(float)i/20-percent/100,1);

		if(!i)glColor4f(0,0,0,1);

		glBegin(GL_QUADS);

			glVertex3f(0,		0, 	 0.0f);

			glVertex3f(1,	0, 	 0.0f);

			glVertex3f(1,	1, 0.0f);

			glVertex3f(0, 	1, 0.0f);

		glEnd();

	}

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix

	glPopMatrix();										// Restore The Old Projection Matrix

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix

	glPopMatrix();										// Restore The Old Projection Matrix

	glDisable(GL_BLEND);

	glDepthMask(1);

	

	//Progress

	

	glDisable(GL_DEPTH_TEST);							// Disables Depth Testing

	glDisable(GL_CULL_FACE);

	glDisable(GL_LIGHTING);

	glDisable(GL_TEXTURE_2D);

	glDepthMask(0);

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix

	glPushMatrix();										// Store The Projection Matrix

	glLoadIdentity();									// Reset The Projection Matrix

	glOrtho(0,640,0,480,-100,100);						// Set Up An Ortho Screen

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix

	glPushMatrix();										// Store The Modelview Matrix

	for(int i=19;i>=0;i--){

		glLoadIdentity();								// Reset The Modelview Matrix

		glTranslatef(120,190,0);

		if(4*percent+i*2<400)glScalef(4*percent+i*2,30,1);

		if(4*percent+i*2>=400)glScalef(400,30,1);

		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_BLEND);

		glColor4f(1,0,0,.1);

		glBegin(GL_QUADS);

			glVertex3f(0,		0, 	 0.0f);

			glVertex3f(1,	0, 	 0.0f);

			glVertex3f(1,	1, 0.0f);

			glVertex3f(0, 	1, 0.0f);

		glEnd();

	}

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix

	glPopMatrix();										// Restore The Old Projection Matrix

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix

	glPopMatrix();										// Restore The Old Projection Matrix

	glDisable(GL_BLEND);

	glDepthMask(1);

	

	//Text

	

	glEnable(GL_TEXTURE_2D);

	glColor4f(.6-.6*percent/100,0,0,1);

	static char string[256]="";

	sprintf (string, "LOADING...");

	text.glPrint(280,195,string,1,1,640,480);

	

	
#ifdef OS9 
	aglSwapBuffers( gOpenGLContext );
#else
	PlatformSwapBuffers();
#endif

}



void LoadPersonSpriteTexture(char *fileName, GLuint *textureid);

void LoadPersonSpriteTexture(char *fileName, GLuint *textureid)

{
	*textureid = loadTexture(fileName);
	glBindTexture(GL_TEXTURE_2D, *textureid);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}



void Game::InitGame()										

{

	//Setup loading screen

	float loadingscreenamount=0;

	float loadingscreenamounttotal=200;

	if(initialized)loadingscreenamounttotal=20;

	

	if(!initialized)LoadingScreen(loadingscreenamount/loadingscreenamounttotal*100);

	

	//Set up rain and snow

	precipitationhorz=60;

	precipitationvert=40;

	precipitationdensity=25;

	

	//Bodyguard stats

	if(revenge){

		//bullets can kill you in revenge mode, so you may start the mission dead (Revenge Mod)

		person[0].health=100;

		person[0].targetanimation=idleanim;

		person[0].targetframe=0;

	}

	person[0].playercoords=camera.position;

	person[0].oldplayercoords=person[0].playercoords;

	person[0].type=playertype;

	person[0].existing=1;

	for(int i=0;i<10;i++){

		person[0].reloads[i]=0;

	}

	

	//Level setup

	killedinnocent=0; //Haven't shot any civilians yet...

	murderer=-1; //Nobody has killed the VIP yet either

	

	if(customlevels){ //Load custom levels

		nummissions=1; //Default level in case of load failure

		type=randomshoot_type;

		possiblegun[0]=handgun1;

		possiblegun[1]=handgun2;

		possiblegun[2]=shotgun;

		numpossibleguns=3;

		evilprobability=6;

		person[0].whichgun=knife;

		person[0].reloads[person[0].whichgun]=6;	

		if(!gameinprogress)score=0;

		timeremaining=50;

		difficulty=.8;

		/* Custom levels: the player's own copy wins, otherwise the one
		   that ships with the game. */

		char customlevelspath[1024];

		const char *customlevelsfile=PlatformUserFileForRead("customlevels.txt",":Data:customlevels.txt",customlevelspath,sizeof(customlevelspath));

		ifstream ipstream(customlevelsfile?customlevelsfile:"");

	    if(!ipstream) {

	        customlevels=0;

	    }

		if(ipstream){

			ipstream.ignore(256,'\n');//ignore descriptive text

		    ipstream >> nummissions;

		    for(int j=0;j<mission+1;j++){

		    ipstream.ignore(256,'\n');

		    ipstream.ignore(256,'\n');

		    ipstream >> type;

		    ipstream.ignore(256,'\n');

		    ipstream.ignore(256,'\n');

		    ipstream >> environment;		

		    ipstream.ignore(256,'\n');

		    ipstream.ignore(256,'\n');

		    ipstream >> numpossibleguns;

		    ipstream.ignore(256,'\n');

		    ipstream.ignore(256,'\n');

		    if(numpossibleguns<0)numpossibleguns=0;
		    if(numpossibleguns>64)numpossibleguns=64;   //a garbled file, not a list of guns
		    for(int i=0;i<numpossibleguns;i++){

		    int gun=nogun;
		    ipstream >> gun;
		    if(gun<nogun||gun>shotgun)gun=nogun;
		    if(i<6)possiblegun[i]=gun;   //possiblegun[] holds 6

		    }
		    if(numpossibleguns>6)numpossibleguns=6;

		    ipstream.ignore(256,'\n');

		    ipstream.ignore(256,'\n');

		    ipstream >> evilprobability;
		    if(evilprobability<1)evilprobability=1;

		    ipstream.ignore(256,'\n');

		    ipstream.ignore(256,'\n');

		    ipstream >> person[0].whichgun;
		    if(person[0].whichgun<nogun||person[0].whichgun>shotgun)person[0].whichgun=nogun;

		    ipstream.ignore(256,'\n');

		    ipstream.ignore(256,'\n');

		    ipstream >> person[0].reloads[person[0].whichgun];

		    ipstream.ignore(256,'\n');

		    ipstream.ignore(256,'\n');

		    ipstream >> timeremaining;

		    ipstream.ignore(256,'\n');

		    ipstream.ignore(256,'\n');

		    ipstream >> difficulty;
		    if(!(difficulty>=.01))difficulty=.01;   //0 ("easiest") would divide by zero

		    ipstream.ignore(256,'\n');

		    }

		    ipstream.close();

		}

	}

	

	if(!customlevels){ //Setup hardcoded default levels

	if(mission==0){		

		environment=sunny_environment;

		type=randomshoot_type;

		possiblegun[0]=handgun1;

		possiblegun[1]=handgun2;

		possiblegun[2]=shotgun;

		numpossibleguns=3;

		evilprobability=6;

		person[0].whichgun=assaultrifle;

		person[0].reloads[person[0].whichgun]=6;	

		if(!gameinprogress)score=0;

		timeremaining=50;

		difficulty=.6;

	}

	if(mission==1){		

		environment=snowy_environment;

		type=randomshoot_type;

		possiblegun[0]=knife;

		possiblegun[1]=assaultrifle;

		numpossibleguns=2;

		evilprobability=5;

		person[0].whichgun=handgun2;

		person[0].reloads[person[0].whichgun]=3;	

		if(!gameinprogress)score=0;

		timeremaining=40;

		difficulty=.6;

	}

	if(mission==2){		

		environment=foggy_environment;

		type=randomshoot_type;

		possiblegun[0]=sniperrifle;

		numpossibleguns=1;

		evilprobability=5;

		person[0].whichgun=sniperrifle;

		person[0].reloads[person[0].whichgun]=4;	

		if(!gameinprogress)score=0;

		timeremaining=50;

		difficulty=0.9;

	}

	if(mission==3){		

		environment=firey_environment;

		type=zombie_type;

		numpossibleguns=0;

		evilprobability=5;

		person[0].whichgun=shotgun;

		person[0].reloads[person[0].whichgun]=5;	

		if(!gameinprogress)score=0;

		timeremaining=35;

		difficulty=.7;

	}

	if(mission==4){	

		environment=snowy_environment;

		type=randomshoot_type;

		possiblegun[0]=sniperrifle;

		possiblegun[1]=assaultrifle;

		numpossibleguns=2;

		evilprobability=5;

		person[0].whichgun=grenade;

		person[0].reloads[person[0].whichgun]=20;	

		if(!gameinprogress)score=0;

		timeremaining=30;

		difficulty=.5;

	}

	if(mission==5){	

		environment=rainy_environment;

		type=randomshoot_type;

		possiblegun[0]=handgun1;

		possiblegun[1]=shotgun;

		possiblegun[2]=assaultrifle;

		numpossibleguns=3;

		evilprobability=6;

		person[0].whichgun=knife;

		person[0].reloads[person[0].whichgun]=3;	

		if(!gameinprogress)score=0;

		timeremaining=40;

		difficulty=.8;

	}

	if(mission==6){		

		environment=night_environment;

		type=randomshoot_type;

		possiblegun[1]=handgun1;

		possiblegun[2]=handgun2;

		possiblegun[3]=shotgun;

		numpossibleguns=3;

		evilprobability=5;

		person[0].whichgun=handgun1;

		person[0].reloads[person[0].whichgun]=4;	

		if(!gameinprogress)score=0;

		timeremaining=30;

		difficulty=1;

	}

	if(mission==7){		

		environment=firey_environment;

		type=zombie_type;

		person[0].whichgun=assaultrifle;

		person[0].reloads[person[0].whichgun]=5;	

		if(!gameinprogress)score=0;

		timeremaining=30;

		difficulty=1;

	}

	if(mission==8){		

		environment=rainy_environment;

		type=randomshoot_type;

		possiblegun[0]=handgun1;

		possiblegun[1]=handgun2;

		possiblegun[2]=shotgun;

		possiblegun[3]=sniperrifle;

		possiblegun[4]=assaultrifle;

		numpossibleguns=5;

		evilprobability=5;

		person[0].whichgun=nogun;

		person[0].reloads[person[0].whichgun]=3;	

		if(!gameinprogress)score=0;

		timeremaining=40;

		difficulty=.8;

	}

	if(mission==9){		

		environment=snowy_environment;

		type=randomshoot_type;

		possiblegun[0]=knife;

		possiblegun[1]=handgun1;

		possiblegun[2]=handgun2;

		possiblegun[3]=shotgun;

		possiblegun[4]=sniperrifle;

		possiblegun[5]=assaultrifle;

		numpossibleguns=6;

		evilprobability=4;

		person[0].whichgun=handgun1;

		person[0].reloads[person[0].whichgun]=3;	

		if(!gameinprogress)score=0;

		timeremaining=90;

		difficulty=1;

	}

	if(mission==10){		

		environment=night_environment;

		type=randomshoot_type;

		possiblegun[0]=sniperrifle;

		numpossibleguns=1;

		evilprobability=5;

		person[0].whichgun=sniperrifle;

		person[0].reloads[person[0].whichgun]=4;	

		if(!gameinprogress)score=0;

		timeremaining=30;

		difficulty=1.3;

	}

	if(mission==11){		

		environment=sunny_environment;

		type=randomshoot_type;

		possiblegun[0]=knife;

		possiblegun[1]=sniperrifle;

		numpossibleguns=2;

		evilprobability=4;

		person[0].whichgun=knife;

		if(!gameinprogress)score=0;

		timeremaining=30;

		difficulty=1.5;

	}

	if(mission==12){		

		environment=firey_environment;

		type=zombie_type;

		possiblegun[0]=knife;

		possiblegun[1]=sniperrifle;

		person[0].whichgun=handgun2;

		person[0].reloads[person[0].whichgun]=10;	

		if(!gameinprogress)score=0;

		timeremaining=60;

		difficulty=1.5;

	}

	

	nummissions=13;

	}

	

	//Setup fast radian to degree conversion

	rad2deg= 56.54866776;

	visions=0;

	

	//Setup bounding cylinder model

	float boundingscale=3;

	

	if(!initialized){		

		boundingpoints[0]=0;

		boundingpoints[0].z=boundingscale;

		boundingpoints[0].y=0;

		for(int i=1;i<8;i++){

			boundingpoints[i]=DoRotation(boundingpoints[0],0,i*360/7,0);

		}

	}

	civkills=0;

	badkills=0;

	goodkills=0;

	enemystate=2;

	

	if(!initialized){	

		/* Keys are read by position (SDL scancodes, like the Mac virtual key codes
		   the game was written for), so these are the WASD *positions* on every
		   layout - ZQSD on an AZERTY keyboard.  The "Azerty keyboard" line in
		   config.txt swapped to the ZQSD positions for ports that read characters
		   instead; here it would only move the controls off again, so it is
		   still read and written but no longer used. */
		forwardskey=MAC_W_KEY;
		backwardskey=MAC_S_KEY;
		leftkey=MAC_A_KEY;
		rightkey=MAC_D_KEY;
		aimkey=MAC_Q_KEY;
		psychicaimkey=MAC_E_KEY;
		psychickey=MAC_Z_KEY;

	

		soundscalefactor=soundscalefactordefault; //Setup sound falloff

	

		gQuit=false;

		

		//Sounds

		LoadSounds(musictoggle);

		//Play correct song

		if(environment==rainy_environment)alSourcePlay(gSourceID[rainsound]);

		if(environment!=rainy_environment)alSourcePause(gSourceID[rainsound]);

		alSourceStop(gSourceID[whichsong]);

		alSourcef(gSourceID[whichsong], AL_MIN_GAIN, 0);

		alSourcef(gSourceID[whichsong], AL_MAX_GAIN, 0);

		whichsong=mainmenusong;

		alSourceStop(gSourceID[whichsong]);

		alSourcef(gSourceID[whichsong], AL_PITCH, 1);

		alSourcePlay(gSourceID[whichsong]);

		alSourcef(gSourceID[whichsong], AL_MIN_GAIN, 1);

		alSourcef(gSourceID[whichsong], AL_MAX_GAIN, 1);	

	}

	

	loadingscreenamount+=5;

	if(!initialized)LoadingScreen(loadingscreenamount/loadingscreenamounttotal*100);

	

	//Setup random seed

#ifdef OS9 
	qd.randSeed = TickCount();
#else
	srand(time(NULL));
#endif

	gamespeed=1;

	

	//Setup camera

	camera.position=0;

	camera.position.x=num_blocks/2*block_spacing+block_spacing/2;

	camera.position.z=num_blocks/2*block_spacing+block_spacing/2;

	camera.position.y=30;

	camera.oldposition=camera.position;

	numpeople=1;

	prepopulate=true;   //the next tick fills the streets

	

	//Setup path to walk around blocks

	path.load((unsigned char *)":Data:Models:path.solid");

	path.Rotate(90,0,0);

	path.Scale(.8,.8,.8);

	path.CalculateNormals();

	

	loadingscreenamount+=5;

	if(!initialized)LoadingScreen(loadingscreenamount/loadingscreenamounttotal*100);

	

	person[0].speedmult=1.3;

	

	//Add vip	

	if(!revenge){

	person[numpeople].playerrotation=0;

	

	person[numpeople].whichcostume=vipcostume; 

	

	person[numpeople].whichblockx=CityBlock(person[0].playercoords.x);

	person[numpeople].whichblocky=CityBlock(person[0].playercoords.z);

	

	person[numpeople].pathnum=-1;

	person[numpeople].oldpathnum=-1;

	person[numpeople].oldoldpathnum=-1;

	person[numpeople].oldoldoldpathnum=-1;

	while(person[numpeople].pathnum<0||person[numpeople].pathnum>=path.vertexNum||person[numpeople].pathnum==1){

		person[numpeople].pathnum=Random()%path.vertexNum;

	}

	person[numpeople].pathtarget.x=path.vertex[person[numpeople].pathnum].x;

	person[numpeople].pathtarget.z=path.vertex[person[numpeople].pathnum].z;			

	person[numpeople].pathsize=.98+float(abs(Random()%20))/400;

	person[numpeople].pathtarget*=person[numpeople].pathsize;

	person[numpeople].pathtarget.x+=person[numpeople].whichblockx*block_spacing;

	person[numpeople].pathtarget.z+=person[numpeople].whichblocky*block_spacing;

	

	

	person[numpeople].playercoords=person[numpeople].pathtarget;

	person[numpeople].oldplayercoords=person[numpeople].playercoords;

	

	person[0].playercoords=person[numpeople].playercoords;

	person[0].playercoords.x+=1;

	person[0].playercoords.z+=1;

	person[0].oldplayercoords=person[0].playercoords;

	

	person[numpeople].skeleton.free=0;

	

	person[numpeople].targetanimation=walkanim;

	person[numpeople].speed=1;

	person[numpeople].existing=0;

	person[numpeople].speedmult=1;

	if(type==zombie_type)person[numpeople].speedmult=.8;

	person[numpeople].health=100;	

	person[numpeople].playerrotation2=0;//20;

	

	person[numpeople].lastdistancevictim=200000;

	

	if(person[numpeople].skeleton.broken)person[numpeople].skeleton.Load((char *)":Data:Skeleton:Basic Figure");

	

	person[numpeople].type=viptype;

	person[numpeople].whichgun=nogun; person[numpeople].aiming=0; person[numpeople].killtarget=-1;

	person[numpeople].existing=1;

	

	citypeoplenum[person[numpeople].whichblockx][person[numpeople].whichblocky]++;

	numpeople++;

	spawndelay=.1;

	

	XYZ vipdistance;

	vipdistance=0;

	vipdistance.x=10000000;

	vipgoal=person[1].playercoords+DoRotation(vipdistance,0,Random()%360,0);

	} else {

	/* Revenge mode: no VIP.  You start on the sidewalk of the block you were
	   in, where the VIP would have started (the Revenge Mod put you in the
	   corner block of the city every time, (block_spacing/2)/block_spacing
	   being 0 in integer arithmetic). */

	person[0].whichblockx=CityBlock(person[0].playercoords.x);

	person[0].whichblocky=CityBlock(person[0].playercoords.z);

	int startpath=-1;

	while(startpath<0||startpath>=path.vertexNum||startpath==1){

		startpath=Random()%path.vertexNum;

	}

	XYZ start;

	start=0;

	start.x=path.vertex[startpath].x;

	start.z=path.vertex[startpath].z;

	start*=.98+float(abs(Random()%20))/400;

	start.x+=person[0].whichblockx*block_spacing;

	start.z+=person[0].whichblocky*block_spacing;

	person[0].playercoords=start;

	person[0].oldplayercoords=person[0].playercoords;

	vipgoal=person[0].playercoords;

	spawndelay=.1;

	}

	

	//Setup block models	

	if(!initialized){

		blocks[0].load((unsigned char *)":Data:Models:Block1.solid");

		blocks[0].Rotate(90,0,0);

		blocks[0].Scale(.8,.8,.8);

		blocks[0].CalculateNormals();

		

		blocks[1].load((unsigned char *)":Data:Models:Block2.solid");

		blocks[1].Rotate(90,0,0);

		blocks[1].Scale(.8,.8,.8);

		blocks[1].CalculateNormals();

		

		loadingscreenamount+=5;

		if(!initialized)LoadingScreen(loadingscreenamount/loadingscreenamounttotal*100);

		

		blocks[2].load((unsigned char *)":Data:Models:Block3.solid");

		blocks[2].Rotate(90,0,0);

		blocks[2].Scale(.8,.8,.8);

		blocks[2].CalculateNormals();

		

		blocks[3].load((unsigned char *)":Data:Models:Block4.solid");

		blocks[3].Rotate(90,0,0);

		blocks[3].Scale(.8,.8,.8);

		blocks[3].CalculateNormals();

		

		loadingscreenamount+=5;

		if(!initialized)LoadingScreen(loadingscreenamount/loadingscreenamounttotal*100);

	

		sidewalkcollide.load((unsigned char *)":Data:Models:Lowheightcollide.solid");

		sidewalkcollide.Rotate(90,0,0);

		sidewalkcollide.Scale(.8,.8,.8);

		sidewalkcollide.CalculateNormals();

		

		blockwalls[0].load((unsigned char *)":Data:Models:Block1collide.solid");

		blockwalls[0].Rotate(90,0,0);

		blockwalls[0].Scale(.8,.75,.8);

		blockwalls[0].CalculateNormals();

		

		blockwalls[1].load((unsigned char *)":Data:Models:Block2collide.solid");

		blockwalls[1].Rotate(90,0,0);

		blockwalls[1].Scale(.8,.75,.8);

		blockwalls[1].CalculateNormals();

		

		blockwalls[2].load((unsigned char *)":Data:Models:Block3collide.solid");

		blockwalls[2].Rotate(90,0,0);

		blockwalls[2].Scale(.8,.75,.8);

		blockwalls[2].CalculateNormals();

		

		blockwalls[3].load((unsigned char *)":Data:Models:Block4collide.solid");

		blockwalls[3].Rotate(90,0,0);

		blockwalls[3].Scale(.8,.75,.8);

		blockwalls[3].CalculateNormals();

		

		loadingscreenamount+=5;

		if(!initialized)LoadingScreen(loadingscreenamount/loadingscreenamounttotal*100);

		

		blockroofs[0].load((unsigned char *)":Data:Models:Highblock1collide.solid");

		blockroofs[0].Rotate(90,0,0);

		blockroofs[0].Scale(.8,.8,.8);

		blockroofs[0].CalculateNormals();

		

		blockroofs[1].load((unsigned char *)":Data:Models:Highblock2collide.solid");

		blockroofs[1].Rotate(90,0,0);

		blockroofs[1].Scale(.8,.8,.8);

		blockroofs[1].CalculateNormals();

		

		blockroofs[2].load((unsigned char *)":Data:Models:Highblock3collide.solid");

		blockroofs[2].Rotate(90,0,0);

		blockroofs[2].Scale(.8,.8,.8);

		blockroofs[2].CalculateNormals();

		

		blockroofs[3].load((unsigned char *)":Data:Models:Highblock4collide.solid");

		blockroofs[3].Rotate(90,0,0);

		blockroofs[3].Scale(.8,.8,.8);

		blockroofs[3].CalculateNormals();

		

		blockcollide[0].load((unsigned char *)":Data:Models:block1complete.solid");

		blockcollide[0].Rotate(90,0,0);

		blockcollide[0].Scale(.8,.8,.8);

		blockcollide[0].CalculateNormals();

		

		blockcollide[1].load((unsigned char *)":Data:Models:block2complete.solid");

		blockcollide[1].Rotate(90,0,0);

		blockcollide[1].Scale(.8,.8,.8);

		blockcollide[1].CalculateNormals();

		

		blockcollide[2].load((unsigned char *)":Data:Models:block3complete.solid");

		blockcollide[2].Rotate(90,0,0);

		blockcollide[2].Scale(.8,.8,.8);

		blockcollide[2].CalculateNormals();

		

		blockcollide[3].load((unsigned char *)":Data:Models:block4complete.solid");

		blockcollide[3].Rotate(90,0,0);

		blockcollide[3].Scale(.8,.8,.8);

		blockcollide[3].CalculateNormals();

		

		loadingscreenamount+=5;

		if(!initialized)LoadingScreen(loadingscreenamount/loadingscreenamounttotal*100);

	

		

		blocksimplecollide[0].load((unsigned char *)":Data:Models:lowsimplecollide1.solid");

		blocksimplecollide[0].Rotate(90,0,0);

		blocksimplecollide[0].Scale(.8,.8,.8);

		blocksimplecollide[0].CalculateNormals();

		

		blocksimplecollide[1].load((unsigned char *)":Data:Models:lowsimplecollide2.solid");

		blocksimplecollide[1].Rotate(90,0,0);

		blocksimplecollide[1].Scale(.8,.8,.8);

		blocksimplecollide[1].CalculateNormals();

		

		blocksimplecollide[2].load((unsigned char *)":Data:Models:lowsimplecollide3.solid");

		blocksimplecollide[2].Rotate(90,0,0);

		blocksimplecollide[2].Scale(.8,.8,.8);

		blocksimplecollide[2].CalculateNormals();

		

		blocksimplecollide[3].load((unsigned char *)":Data:Models:lowsimplecollide4.solid");

		blocksimplecollide[3].Rotate(90,0,0);

		blocksimplecollide[3].Scale(.8,.8,.8);

		blocksimplecollide[3].CalculateNormals();

		

		loadingscreenamount+=5;

		if(!initialized)LoadingScreen(loadingscreenamount/loadingscreenamounttotal*100);

	

		blockocclude.load((unsigned char *)":Data:Models:blockocclude.solid");

		blockocclude.Rotate(90,0,0);

		blockocclude.Scale(.8,.8,.8);

		blockocclude.CalculateNormals();



		blocksimple.load((unsigned char *)":Data:Models:blocksimple.solid");

		blocksimple.Rotate(90,0,0);

		blocksimple.Scale(.8,2,.8);

		blocksimple.CalculateNormals();



		street.load((unsigned char *)":Data:Models:streetsubdivided2.solid");

		street.Rotate(90,0,0);

		street.Scale(.01,.01,.01);

		street.CalculateNormals();

		

		Bigstreet=street;

		Bigstreet.Scale(10000,10000,10000);

		

		loadingscreenamount+=5;

		if(!initialized)LoadingScreen(loadingscreenamount/loadingscreenamounttotal*100);

	

		path.load((unsigned char *)":Data:Models:path.solid");

		path.Rotate(90,0,0);

		path.Scale(.8,.8,.8);

		path.CalculateNormals();

	

		//Fix block radius

		for(int i=0;i<4;i++){

			blocks[i].boundingspherecenter.x=0;

			blocks[i].boundingspherecenter.z=0;

			blocks[i].boundingsphereradius=0;

			for(int x=0;x<blocks[i].vertexNum;x++){

				if(findDistancefast(blocks[i].boundingspherecenter,blocks[i].vertex[x])>blocks[i].boundingsphereradius)blocks[i].boundingsphereradius=findDistancefast(blocks[i].boundingspherecenter,blocks[i].vertex[x]);

			}

			blocks[i].boundingsphereradius=sqrt(blocks[i].boundingsphereradius);

		}

	}	

	mousesensitivity=1;

	

	//init city block rotations

	for(int i=0;i<num_blocks;i++){

		for(int j=0;j<num_blocks;j++){

			cityrotation[i][j]=Random()%4;

			citytype[i][j]=abs(Random()%4);   //Random() is signed: plain %4 plus the clamp below made 62.5% of the city block type 0

			citypeoplenum[i][j]=0;

			if(citytype[i][j]<0)citytype[i][j]=0;

			if(citytype[i][j]>3)citytype[i][j]=3;

		}

	}



		if(!initialized){

		//Load player model

		skeletonmodels[0].load((unsigned char *)":Data:Models:Head.solid");

		skeletonmodels[0].Rotate(90,0,0);

		skeletonmodels[0].Scale(.02,.02,.02);

		skeletonmodels[0].CalculateNormals();

		

		skeletonmodels[1].load((unsigned char *)":Data:Models:Chest.solid");

		skeletonmodels[1].Rotate(90,0,0);

		skeletonmodels[1].Scale(.02,.02,.02);

		skeletonmodels[1].CalculateNormals();

		

		skeletonmodels[2].load((unsigned char *)":Data:Models:Abdomen.solid");

		skeletonmodels[2].Rotate(90,0,0);

		skeletonmodels[2].Scale(.02,.02,.02);

		skeletonmodels[2].CalculateNormals();

		

		skeletonmodels[3].load((unsigned char *)":Data:Models:Upper arm.solid");

		skeletonmodels[3].Rotate(90,0,0);

		skeletonmodels[3].Scale(.02,.02,.02);

		skeletonmodels[3].CalculateNormals();

		

		skeletonmodels[4].load((unsigned char *)":Data:Models:Lower arm.solid");

		skeletonmodels[4].Rotate(90,0,0);

		skeletonmodels[4].Scale(.02,.02,.02);

		skeletonmodels[4].CalculateNormals();

		

		skeletonmodels[5].load((unsigned char *)":Data:Models:Hand.solid");

		skeletonmodels[5].Rotate(90,0,0);

		skeletonmodels[5].Scale(.02,.02,.02);

		skeletonmodels[5].CalculateNormals();

		

		skeletonmodels[6].load((unsigned char *)":Data:Models:Upper leg.solid");

		skeletonmodels[6].Rotate(90,0,0);

		skeletonmodels[6].Scale(.02,.02,.02);

		skeletonmodels[6].CalculateNormals();

		

		skeletonmodels[7].load((unsigned char *)":Data:Models:Lower leg.solid");

		skeletonmodels[7].Rotate(90,0,0);

		skeletonmodels[7].Scale(.02,.02,.02);

		skeletonmodels[7].CalculateNormals();

		

		skeletonmodels[8].load((unsigned char *)":Data:Models:Foot.solid");

		skeletonmodels[8].Rotate(90,0,0);

		skeletonmodels[8].Scale(.02,.02,.02);

		skeletonmodels[8].CalculateNormals();

		

		skeletonmodels[9].load((unsigned char *)":Data:Models:Shades.solid");

		skeletonmodels[9].Rotate(90,0,0);

		skeletonmodels[9].Scale(.02,.02,.02);

		skeletonmodels[9].CalculateNormals();

		

		//Load gun models

		loadingscreenamount+=5;

		if(!initialized)LoadingScreen(loadingscreenamount/loadingscreenamounttotal*100);

	

		gunmodels[sniperriflemodel].load((unsigned char *)":Data:Models:sniperrifle.solid");

		gunmodels[sniperriflemodel].Rotate(0,0,90);

		gunmodels[sniperriflemodel].Scale(.001,.001,.001);

		gunmodels[sniperriflemodel].CalculateNormals();

		

		gunmodels[assaultriflemodel].load((unsigned char *)":Data:Models:assaultrifle.solid");

		gunmodels[assaultriflemodel].Rotate(0,0,90);

		gunmodels[assaultriflemodel].Scale(.01,.01,.01);

		gunmodels[assaultriflemodel].CalculateNormals();

		

		gunmodels[handgunbasemodel].load((unsigned char *)":Data:Models:Handgunbase.solid");

		gunmodels[handgunbasemodel].Rotate(0,0,90);

		gunmodels[handgunbasemodel].Rotate(180,0,0);

		gunmodels[handgunbasemodel].Scale(.014,.014,.014);

		gunmodels[handgunbasemodel].CalculateNormals();

		gunmodels[handgunbasemodel].MultColor(.6);

		loadingscreenamount+=5;

		if(!initialized)LoadingScreen(loadingscreenamount/loadingscreenamounttotal*100);

	

		gunmodels[handgunslidemodel].load((unsigned char *)":Data:Models:Handgunslide.solid");

		gunmodels[handgunslidemodel].Rotate(0,0,90);

		gunmodels[handgunslidemodel].Rotate(180,0,0);

		gunmodels[handgunslidemodel].Scale(.014,.014,.014);

		gunmodels[handgunslidemodel].CalculateNormals();

		gunmodels[handgunslidemodel].MultColor(.6);

		

		gunmodels[handgun2basemodel].load((unsigned char *)":Data:Models:glockbase.solid");

		gunmodels[handgun2basemodel].Rotate(0,0,90);

		gunmodels[handgun2basemodel].Rotate(180,0,0);

		gunmodels[handgun2basemodel].Scale(.014,.014,.014);

		gunmodels[handgun2basemodel].CalculateNormals();

		gunmodels[handgun2basemodel].MultColor(.6);

		

		gunmodels[handgun2slidemodel].load((unsigned char *)":Data:Models:glockslide.solid");

		gunmodels[handgun2slidemodel].Rotate(0,0,90);

		gunmodels[handgun2slidemodel].Rotate(180,0,0);

		gunmodels[handgun2slidemodel].Scale(.014,.014,.014);

		gunmodels[handgun2slidemodel].CalculateNormals();

		gunmodels[handgun2slidemodel].MultColor(.6);

		loadingscreenamount+=5;

		if(!initialized)LoadingScreen(loadingscreenamount/loadingscreenamounttotal*100);

	

		gunmodels[grenadebasemodel].load((unsigned char *)":Data:Models:grenadebase.solid");

		gunmodels[grenadebasemodel].Rotate(0,0,90);

		gunmodels[grenadebasemodel].Rotate(180,0,0);

		gunmodels[grenadebasemodel].Scale(.014,.014,.014);

		gunmodels[grenadebasemodel].CalculateNormals();

		

		gunmodels[grenadepinmodel].load((unsigned char *)":Data:Models:grenadepin.solid");

		gunmodels[grenadepinmodel].Rotate(0,0,90);

		gunmodels[grenadepinmodel].Rotate(180,0,0);

		gunmodels[grenadepinmodel].Scale(.014,.014,.014);

		gunmodels[grenadepinmodel].CalculateNormals();

		

		gunmodels[grenadespoonmodel].load((unsigned char *)":Data:Models:grenadespoon.solid");

		gunmodels[grenadespoonmodel].Rotate(0,0,90);

		gunmodels[grenadespoonmodel].Rotate(180,0,0);

		gunmodels[grenadespoonmodel].Scale(.014,.014,.014);

		gunmodels[grenadespoonmodel].CalculateNormals();

		

		gunmodels[knifemodel].load((unsigned char *)":Data:Models:Knife.solid");

		gunmodels[knifemodel].Rotate(0,0,90);

		gunmodels[knifemodel].Rotate(180,0,0);

		gunmodels[knifemodel].Scale(.014,.014,.014);

		gunmodels[knifemodel].CalculateNormals();

		

		gunmodels[shotgunmodel].load((unsigned char *)":Data:Models:shotgun.solid");

		gunmodels[shotgunmodel].Rotate(0,0,90);

		gunmodels[shotgunmodel].Scale(.001,.001,.001);

		gunmodels[shotgunmodel].CalculateNormals();

		gunmodels[shotgunmodel].MultColor(.6);

		}

		

		loadingscreenamount+=5;

		if(!initialized)LoadingScreen(loadingscreenamount/loadingscreenamounttotal*100);

	

		//Setup costumes

		float headcolor[3];

		float footcolor[3];

		float handcolor[3];

		float topcolor[3];

		float bottomcolor[3];

		//Police

		headcolor[0]=(float)240/255;

		headcolor[1]=(float)183/255;

		headcolor[2]=(float)132/255;

		

		footcolor[0]=(float)119/255;

		footcolor[1]=(float)68/255;

		footcolor[2]=(float)18/255;

		

		handcolor[0]=(float)240/255;

		handcolor[1]=(float)183/255;

		handcolor[2]=(float)132/255;

		

		topcolor[0]=(float)14/255;

		topcolor[1]=(float)18/255;

		topcolor[2]=(float)195/255;

		

		bottomcolor[0]=(float)14/255;

		bottomcolor[1]=(float)18/255;

		bottomcolor[2]=(float)195/255;

		

		//Greenish skin if zombies

		if(type==zombie_type){

			headcolor[0]=(float)223/255;

			headcolor[1]=(float)243/255;

			headcolor[2]=(float)197/255;

			

			handcolor[0]=(float)223/255;

			handcolor[1]=(float)243/255;

			handcolor[2]=(float)197/255;

		}

		

		costume[policecostume].headcolor[0]=headcolor[0];

		costume[policecostume].headcolor[1]=headcolor[1];

		costume[policecostume].headcolor[2]=headcolor[2];

		

		costume[policecostume].handcolor[0]=handcolor[0];

		costume[policecostume].handcolor[1]=handcolor[1];

		costume[policecostume].handcolor[2]=handcolor[2];

		

		costume[policecostume].chestcolor[0]=topcolor[0];

		costume[policecostume].chestcolor[1]=topcolor[1];

		costume[policecostume].chestcolor[2]=topcolor[2];

		

		costume[policecostume].abdomencolor[0]=topcolor[0];

		costume[policecostume].abdomencolor[1]=topcolor[1];

		costume[policecostume].abdomencolor[2]=topcolor[2];

		

		costume[policecostume].upperarmcolor[0]=topcolor[0];

		costume[policecostume].upperarmcolor[1]=topcolor[1];

		costume[policecostume].upperarmcolor[2]=topcolor[2];

		

		costume[policecostume].lowerarmcolor[0]=topcolor[0];

		costume[policecostume].lowerarmcolor[1]=topcolor[1];

		costume[policecostume].lowerarmcolor[2]=topcolor[2];

		

		costume[policecostume].upperlegcolor[0]=bottomcolor[0];

		costume[policecostume].upperlegcolor[1]=bottomcolor[1];

		costume[policecostume].upperlegcolor[2]=bottomcolor[2];

		

		costume[policecostume].lowerlegcolor[0]=bottomcolor[0];

		costume[policecostume].lowerlegcolor[1]=bottomcolor[1];

		costume[policecostume].lowerlegcolor[2]=bottomcolor[2];

		

		costume[policecostume].footcolor[0]=footcolor[0];

		costume[policecostume].footcolor[1]=footcolor[1];

		costume[policecostume].footcolor[2]=footcolor[2];

		

		//casual

		topcolor[0]=(float)14/255;

		topcolor[1]=(float)200/255;

		topcolor[2]=(float)30/255;

		

		bottomcolor[0]=(float)14/255;

		bottomcolor[1]=(float)18/255;

		bottomcolor[2]=(float)195/255;

		

		costume[casualcostumes].headcolor[0]=headcolor[0];

		costume[casualcostumes].headcolor[1]=headcolor[1];

		costume[casualcostumes].headcolor[2]=headcolor[2];

		

		costume[casualcostumes].handcolor[0]=handcolor[0];

		costume[casualcostumes].handcolor[1]=handcolor[1];

		costume[casualcostumes].handcolor[2]=handcolor[2];

		

		costume[casualcostumes].chestcolor[0]=topcolor[0];

		costume[casualcostumes].chestcolor[1]=topcolor[1];

		costume[casualcostumes].chestcolor[2]=topcolor[2];

		

		costume[casualcostumes].abdomencolor[0]=topcolor[0];

		costume[casualcostumes].abdomencolor[1]=topcolor[1];

		costume[casualcostumes].abdomencolor[2]=topcolor[2];

		

		costume[casualcostumes].upperarmcolor[0]=topcolor[0];

		costume[casualcostumes].upperarmcolor[1]=topcolor[1];

		costume[casualcostumes].upperarmcolor[2]=topcolor[2];

		

		costume[casualcostumes].lowerarmcolor[0]=handcolor[0];

		costume[casualcostumes].lowerarmcolor[1]=handcolor[1];

		costume[casualcostumes].lowerarmcolor[2]=handcolor[2];

		

		costume[casualcostumes].upperlegcolor[0]=bottomcolor[0];

		costume[casualcostumes].upperlegcolor[1]=bottomcolor[1];

		costume[casualcostumes].upperlegcolor[2]=bottomcolor[2];

		

		costume[casualcostumes].lowerlegcolor[0]=bottomcolor[0];

		costume[casualcostumes].lowerlegcolor[1]=bottomcolor[1];

		costume[casualcostumes].lowerlegcolor[2]=bottomcolor[2];

		

		costume[casualcostumes].footcolor[0]=footcolor[0];

		costume[casualcostumes].footcolor[1]=footcolor[1];

		costume[casualcostumes].footcolor[2]=footcolor[2];

		

		//casual 2

		topcolor[0]=(float)140/255;

		topcolor[1]=(float)55/255;

		topcolor[2]=(float)4/255;

		

		bottomcolor[0]=(float)14/255;

		bottomcolor[1]=(float)18/255;

		bottomcolor[2]=(float)135/255;

		

		costume[casualcostumes+1].headcolor[0]=headcolor[0];

		costume[casualcostumes+1].headcolor[1]=headcolor[1];

		costume[casualcostumes+1].headcolor[2]=headcolor[2];

		

		costume[casualcostumes+1].handcolor[0]=handcolor[0];

		costume[casualcostumes+1].handcolor[1]=handcolor[1];

		costume[casualcostumes+1].handcolor[2]=handcolor[2];

		

		costume[casualcostumes+1].chestcolor[0]=topcolor[0];

		costume[casualcostumes+1].chestcolor[1]=topcolor[1];

		costume[casualcostumes+1].chestcolor[2]=topcolor[2];

		

		costume[casualcostumes+1].abdomencolor[0]=topcolor[0];

		costume[casualcostumes+1].abdomencolor[1]=topcolor[1];

		costume[casualcostumes+1].abdomencolor[2]=topcolor[2];

		

		costume[casualcostumes+1].upperarmcolor[0]=topcolor[0];

		costume[casualcostumes+1].upperarmcolor[1]=topcolor[1];

		costume[casualcostumes+1].upperarmcolor[2]=topcolor[2];

		

		costume[casualcostumes+1].lowerarmcolor[0]=topcolor[0];

		costume[casualcostumes+1].lowerarmcolor[1]=topcolor[1];

		costume[casualcostumes+1].lowerarmcolor[2]=topcolor[2];

		

		costume[casualcostumes+1].upperlegcolor[0]=bottomcolor[0];

		costume[casualcostumes+1].upperlegcolor[1]=bottomcolor[1];

		costume[casualcostumes+1].upperlegcolor[2]=bottomcolor[2];

		

		costume[casualcostumes+1].lowerlegcolor[0]=bottomcolor[0];

		costume[casualcostumes+1].lowerlegcolor[1]=bottomcolor[1];

		costume[casualcostumes+1].lowerlegcolor[2]=bottomcolor[2];

		

		costume[casualcostumes+1].footcolor[0]=footcolor[0];

		costume[casualcostumes+1].footcolor[1]=footcolor[1];

		costume[casualcostumes+1].footcolor[2]=footcolor[2];

		

		//casual 3

		topcolor[0]=(float)134/255;

		topcolor[1]=(float)80/255;

		topcolor[2]=(float)3/255;

		

		bottomcolor[0]=(float)30/255;

		bottomcolor[1]=(float)30/255;

		bottomcolor[2]=(float)30/255;

		

		footcolor[0]=(float)20/255;

		footcolor[1]=(float)20/255;

		footcolor[2]=(float)20/255;

		

		

		costume[casualcostumes+2].headcolor[0]=headcolor[0];

		costume[casualcostumes+2].headcolor[1]=headcolor[1];

		costume[casualcostumes+2].headcolor[2]=headcolor[2];

		

		costume[casualcostumes+2].handcolor[0]=handcolor[0];

		costume[casualcostumes+2].handcolor[1]=handcolor[1];

		costume[casualcostumes+2].handcolor[2]=handcolor[2];

		

		costume[casualcostumes+2].chestcolor[0]=topcolor[0];

		costume[casualcostumes+2].chestcolor[1]=topcolor[1];

		costume[casualcostumes+2].chestcolor[2]=topcolor[2];

		

		costume[casualcostumes+2].abdomencolor[0]=topcolor[0];

		costume[casualcostumes+2].abdomencolor[1]=topcolor[1];

		costume[casualcostumes+2].abdomencolor[2]=topcolor[2];

		

		costume[casualcostumes+2].upperarmcolor[0]=topcolor[0];

		costume[casualcostumes+2].upperarmcolor[1]=topcolor[1];

		costume[casualcostumes+2].upperarmcolor[2]=topcolor[2];

		

		costume[casualcostumes+2].lowerarmcolor[0]=topcolor[0];

		costume[casualcostumes+2].lowerarmcolor[1]=topcolor[1];

		costume[casualcostumes+2].lowerarmcolor[2]=topcolor[2];

		

		costume[casualcostumes+2].upperlegcolor[0]=bottomcolor[0];

		costume[casualcostumes+2].upperlegcolor[1]=bottomcolor[1];

		costume[casualcostumes+2].upperlegcolor[2]=bottomcolor[2];

		

		costume[casualcostumes+2].lowerlegcolor[0]=bottomcolor[0];

		costume[casualcostumes+2].lowerlegcolor[1]=bottomcolor[1];

		costume[casualcostumes+2].lowerlegcolor[2]=bottomcolor[2];

		

		costume[casualcostumes+2].footcolor[0]=footcolor[0];

		costume[casualcostumes+2].footcolor[1]=footcolor[1];

		costume[casualcostumes+2].footcolor[2]=footcolor[2];

		

		//casual 4

		topcolor[0]=(float)228/255;

		topcolor[1]=(float)220/255;

		topcolor[2]=(float)0/255;

		

		bottomcolor[0]=(float)20/255;

		bottomcolor[1]=(float)20/255;

		bottomcolor[2]=(float)20/255;

		

		footcolor[0]=(float)119/255;

		footcolor[1]=(float)68/255;

		footcolor[2]=(float)18/255;

		

		costume[casualcostumes+3].headcolor[0]=headcolor[0];

		costume[casualcostumes+3].headcolor[1]=headcolor[1];

		costume[casualcostumes+3].headcolor[2]=headcolor[2];

		

		costume[casualcostumes+3].handcolor[0]=handcolor[0];

		costume[casualcostumes+3].handcolor[1]=handcolor[1];

		costume[casualcostumes+3].handcolor[2]=handcolor[2];

		

		costume[casualcostumes+3].chestcolor[0]=topcolor[0];

		costume[casualcostumes+3].chestcolor[1]=topcolor[1];

		costume[casualcostumes+3].chestcolor[2]=topcolor[2];

		

		costume[casualcostumes+3].abdomencolor[0]=topcolor[0];

		costume[casualcostumes+3].abdomencolor[1]=topcolor[1];

		costume[casualcostumes+3].abdomencolor[2]=topcolor[2];

		

		costume[casualcostumes+3].upperarmcolor[0]=topcolor[0];

		costume[casualcostumes+3].upperarmcolor[1]=topcolor[1];

		costume[casualcostumes+3].upperarmcolor[2]=topcolor[2];

		

		costume[casualcostumes+3].lowerarmcolor[0]=handcolor[0];

		costume[casualcostumes+3].lowerarmcolor[1]=handcolor[1];

		costume[casualcostumes+3].lowerarmcolor[2]=handcolor[2];

		

		costume[casualcostumes+3].upperlegcolor[0]=bottomcolor[0];

		costume[casualcostumes+3].upperlegcolor[1]=bottomcolor[1];

		costume[casualcostumes+3].upperlegcolor[2]=bottomcolor[2];

		

		costume[casualcostumes+3].lowerlegcolor[0]=bottomcolor[0];

		costume[casualcostumes+3].lowerlegcolor[1]=bottomcolor[1];

		costume[casualcostumes+3].lowerlegcolor[2]=bottomcolor[2];

		

		costume[casualcostumes+3].footcolor[0]=footcolor[0];

		costume[casualcostumes+3].footcolor[1]=footcolor[1];

		costume[casualcostumes+3].footcolor[2]=footcolor[2];

	

		if(!initialized){

		//vip

		topcolor[0]=(float)235/255;

		topcolor[1]=(float)235/255;

		topcolor[2]=(float)235/255;

		

		bottomcolor[0]=(float)200/255;

		bottomcolor[1]=(float)200/255;

		bottomcolor[2]=(float)200/255;

		

		footcolor[0]=(float)119/255;

		footcolor[1]=(float)68/255;

		footcolor[2]=(float)18/255;

		

		headcolor[0]=(float)240/255;

		headcolor[1]=(float)183/255;

		headcolor[2]=(float)132/255;

		

		footcolor[0]=(float)119/255;

		footcolor[1]=(float)68/255;

		footcolor[2]=(float)18/255;

		

		handcolor[0]=(float)240/255;

		handcolor[1]=(float)183/255;

		handcolor[2]=(float)132/255;

		

		costume[vipcostume].headcolor[0]=headcolor[0];

		costume[vipcostume].headcolor[1]=headcolor[1];

		costume[vipcostume].headcolor[2]=headcolor[2];

		

		costume[vipcostume].handcolor[0]=handcolor[0];

		costume[vipcostume].handcolor[1]=handcolor[1];

		costume[vipcostume].handcolor[2]=handcolor[2];

		

		costume[vipcostume].chestcolor[0]=topcolor[0];

		costume[vipcostume].chestcolor[1]=topcolor[1];

		costume[vipcostume].chestcolor[2]=topcolor[2];

		

		costume[vipcostume].abdomencolor[0]=topcolor[0];

		costume[vipcostume].abdomencolor[1]=topcolor[1];

		costume[vipcostume].abdomencolor[2]=topcolor[2];

		

		costume[vipcostume].upperarmcolor[0]=topcolor[0];

		costume[vipcostume].upperarmcolor[1]=topcolor[1];

		costume[vipcostume].upperarmcolor[2]=topcolor[2];

		

		costume[vipcostume].lowerarmcolor[0]=topcolor[0];

		costume[vipcostume].lowerarmcolor[1]=topcolor[1];

		costume[vipcostume].lowerarmcolor[2]=topcolor[2];

		

		costume[vipcostume].upperlegcolor[0]=bottomcolor[0];

		costume[vipcostume].upperlegcolor[1]=bottomcolor[1];

		costume[vipcostume].upperlegcolor[2]=bottomcolor[2];

		

		costume[vipcostume].lowerlegcolor[0]=bottomcolor[0];

		costume[vipcostume].lowerlegcolor[1]=bottomcolor[1];

		costume[vipcostume].lowerlegcolor[2]=bottomcolor[2];

		

		costume[vipcostume].footcolor[0]=footcolor[0];

		costume[vipcostume].footcolor[1]=footcolor[1];

		costume[vipcostume].footcolor[2]=footcolor[2];

		

		//Bodyguard

		topcolor[0]=(float)50/255;

		topcolor[1]=(float)50/255;

		topcolor[2]=(float)50/255;

		

		bottomcolor[0]=(float)30/255;

		bottomcolor[1]=(float)30/255;

		bottomcolor[2]=(float)30/255;

		

		footcolor[0]=(float)20/255;

		footcolor[1]=(float)20/255;

		footcolor[2]=(float)20/255;

		

		costume[bodyguardcostume].headcolor[0]=headcolor[0];

		costume[bodyguardcostume].headcolor[1]=headcolor[1];

		costume[bodyguardcostume].headcolor[2]=headcolor[2];

		

		costume[bodyguardcostume].handcolor[0]=handcolor[0];

		costume[bodyguardcostume].handcolor[1]=handcolor[1];

		costume[bodyguardcostume].handcolor[2]=handcolor[2];

		

		costume[bodyguardcostume].chestcolor[0]=topcolor[0];

		costume[bodyguardcostume].chestcolor[1]=topcolor[1];

		costume[bodyguardcostume].chestcolor[2]=topcolor[2];

		

		costume[bodyguardcostume].abdomencolor[0]=topcolor[0];

		costume[bodyguardcostume].abdomencolor[1]=topcolor[1];

		costume[bodyguardcostume].abdomencolor[2]=topcolor[2];

		

		costume[bodyguardcostume].upperarmcolor[0]=topcolor[0];

		costume[bodyguardcostume].upperarmcolor[1]=topcolor[1];

		costume[bodyguardcostume].upperarmcolor[2]=topcolor[2];

		

		costume[bodyguardcostume].lowerarmcolor[0]=topcolor[0];

		costume[bodyguardcostume].lowerarmcolor[1]=topcolor[1];

		costume[bodyguardcostume].lowerarmcolor[2]=topcolor[2];

		

		costume[bodyguardcostume].upperlegcolor[0]=bottomcolor[0];

		costume[bodyguardcostume].upperlegcolor[1]=bottomcolor[1];

		costume[bodyguardcostume].upperlegcolor[2]=bottomcolor[2];

		

		costume[bodyguardcostume].lowerlegcolor[0]=bottomcolor[0];

		costume[bodyguardcostume].lowerlegcolor[1]=bottomcolor[1];

		costume[bodyguardcostume].lowerlegcolor[2]=bottomcolor[2];

		

		costume[bodyguardcostume].footcolor[0]=footcolor[0];

		costume[bodyguardcostume].footcolor[1]=footcolor[1];

		costume[bodyguardcostume].footcolor[2]=footcolor[2];

		

		//Load animations

		loadingscreenamount+=5;

		if(!initialized)LoadingScreen(loadingscreenamount/loadingscreenamounttotal*100);

	

		testskeleton.Load((char *)":Data:Skeleton:Basic Figure");

		animation[idleanim].Load((char *)":Data:Animations:Breathe");

		animation[joganim].Load((char *)":Data:Animations:Run");

		animation[pistolaimanim].Load((char *)":Data:Animations:PistolAim");

		loadingscreenamount+=5;

		if(!initialized)LoadingScreen(loadingscreenamount/loadingscreenamounttotal*100);

		animation[walkanim].Load((char *)":Data:Animations:Walk");

		animation[rifleholdanim].Load((char *)":Data:Animations:Riflehold");

		animation[rifleaimanim].Load((char *)":Data:Animations:Rifleaim");

		animation[assaultrifleaimanim].Load((char *)":Data:Animations:AssaultRifleaim");

		animation[crouchanim].Load((char *)":Data:Animations:Crouch");

		loadingscreenamount+=5;

		if(!initialized)LoadingScreen(loadingscreenamount/loadingscreenamounttotal*100);

		animation[headpainanim].Load((char *)":Data:Animations:Headshot");

		animation[chestpainanim].Load((char *)":Data:Animations:Chestshot");

		animation[stomachpainanim].Load((char *)":Data:Animations:Stomachshot");

		animation[rightarmpainanim].Load((char *)":Data:Animations:Rightarmshot");

		animation[leftarmpainanim].Load((char *)":Data:Animations:Leftarmshot");

		loadingscreenamount+=5;

		if(!initialized)LoadingScreen(loadingscreenamount/loadingscreenamounttotal*100);

		animation[rightlegpainanim].Load((char *)":Data:Animations:Rightlegshot");

		animation[leftlegpainanim].Load((char *)":Data:Animations:Leftlegshot");

		animation[riflehitanim].Load((char *)":Data:Animations:Riflehit");

		animation[grenadeaimanim].Load((char *)":Data:Animations:grenadeaim");

		animation[grenadechargeanim].Load((char *)":Data:Animations:grenadecharge");

		loadingscreenamount+=5;

		if(!initialized)LoadingScreen(loadingscreenamount/loadingscreenamounttotal*100);

		animation[grenadethrowanim].Load((char *)":Data:Animations:grenadethrow");

		animation[zombieeatanim].Load((char *)":Data:Animations:Zombiemunch");

		animation[zombiejoganim].Load((char *)":Data:Animations:ZombieRun");

		animation[zombiewalkanim].Load((char *)":Data:Animations:Zombiewalk");

		loadingscreenamount+=5;

		if(!initialized)LoadingScreen(loadingscreenamount/loadingscreenamounttotal*100);

		animation[getupfrontanim].Load((char *)":Data:Animations:Getupfromfront");

		animation[getupbackanim].Load((char *)":Data:Animations:Getupfromback",180);

		animation[diveanim].Load((char *)":Data:Animations:Dive");

		animation[throwanim].Load((char *)":Data:Animations:Aikidothrow");

		animation[thrownanim].Load((char *)":Data:Animations:Aikidothrown");

	}

	

	//Setup people (as many as the view distance's crowd needs)

	const int loadingstep=(maxpeople+17)/18;   //the loading bar still moves 18 times, as for 90

	for(int i=0;i<maxpeople;i++){

		if(i==0){

			person[i].whichcostume=bodyguardcostume;

		}		

		

		if(i>1){

			person[i].whichcostume=casualcostumes+abs(Random())%numcasual;

		}	

		//person[i].firstlongdead=0;

		person[i].dead=0;

		person[i].health=100;

		person[i].skeleton.free=0;

		person[i].ammo=0;
		person[i].pace=0;   //you and the VIP keep the original's speeds
		person[i].cruising=0;

		person[i].velocity=0;

		//Load skeleton structure

		if(!initialized)person[i].skeleton.Load((char *)":Data:Skeleton:Basic Figure");

		

		if(i%loadingstep==0){

			loadingscreenamount+=5;

			if(!initialized)LoadingScreen(loadingscreenamount/loadingscreenamounttotal*100);

		}

	}

	

	loadingscreenamount+=5;

	if(!initialized)LoadingScreen(loadingscreenamount/loadingscreenamounttotal*100);

	

	if(initialized)person[0].skeleton.Load((char *)":Data:Skeleton:Basic Figure");

	

	person[0].attackframe=-1;

	

	spawndelay=0;



	fog.SetFog(fogcolorr,fogcolorg,fogcolorb,0,viewdistance*.8,.1);

	

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	

	

	//light

	GLfloat LightAmbient[]=		{ .3, .3, .3, 1.0f};

	GLfloat LightDiffuse[]=		{ 1, 1, 1, 1.0f };

	

	glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);		

	glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);		

	glEnable(GL_LIGHT0);	

	

	loadingscreenamount+=5;

	

	//Load some textures

	if(!initialized){

		LoadingScreen(loadingscreenamount/loadingscreenamounttotal*100);

		

		

		LoadPersonSpriteTexture(":Data:Textures:Personsprite.png",&personspritetextureptr);

		LoadPersonSpriteTexture(":Data:Textures:DeadPersonsprite.png",&deadpersonspritetextureptr);

		LoadPersonSpriteTexture(":Data:Textures:Scope.png",&scopetextureptr);

		LoadPersonSpriteTexture(":Data:Textures:Flare.png",&flaretextureptr);

		

		sprites.LoadFlareTexture(":Data:Textures:HitFlash.png");

		sprites.LoadMuzzleFlareTexture(":Data:Textures:MuzzleFlash.png");

		sprites.LoadSmokeTexture(":Data:Textures:Smoke.png");

		sprites.LoadBloodTexture(":Data:Textures:Blood.png");

		sprites.LoadRainTexture(":Data:Textures:rain.png");

		sprites.LoadSnowTexture(":Data:Textures:snow.png");

		decals.LoadBulletHoleTexture(":Data:Textures:BulletHole.png");

		decals.LoadCraterTexture(":Data:Textures:Crater.png");

		decals.LoadBloodTexture(":Data:Textures:Blood:Blood1.png",0);

		decals.LoadBloodTexture(":Data:Textures:Blood:Blood2.png",1);

		decals.LoadBloodTexture(":Data:Textures:Blood:Blood3.png",2);

		decals.LoadBloodTexture(":Data:Textures:Blood:Blood4.png",3);

		decals.LoadBloodTexture(":Data:Textures:Blood:Blood5.png",4);

		decals.LoadBloodTexture(":Data:Textures:Blood:Blood6.png",5);

		decals.LoadBloodTexture(":Data:Textures:Blood:Blood7.png",6);

		decals.LoadBloodTexture(":Data:Textures:Blood:Blood8.png",7);

		decals.LoadBloodTexture(":Data:Textures:Blood:Blood9.png",8);

		decals.LoadBloodTexture(":Data:Textures:Blood:Blood10.png",9);

		decals.LoadBloodTexture(":Data:Textures:Blood:Blood11.png",10);

	}

		

	//Setup clip plane equation

	eqn[0]=0;

	eqn[1]=1;

	eqn[2]=0;

	eqn[3]=0;

	

	glClearColor(fogcolorr,fogcolorg,fogcolorb,1);



	if(!initialized)InitMouse();

	

	//Draw city one frame to fix evil menu bug

	if(!initialized)mainmenu=2;

	

	if(!initialized){

		LoadingScreen(loadingscreenamount/loadingscreenamounttotal*100);

		flashamount=1;

		flashr=1;flashg=1;flashb=1;

		alSourcePlay(gSourceID[soulinsound]);

	}

	

	initialized=1;

	

	loadingscreenamount+=5;

	

	//Sync to refresh rate

	if(vblsync){	

		GLint swapInt = 1;

#ifdef OS9 
 	   	aglSetInteger(gOpenGLContext, AGL_SWAP_INTERVAL, &swapInt);
#else
 
#endif


               

	}

	/*

	for(int i=0;i<sprites.howmanysprites;i++){

		sprites.DeleteSprite(0);

	}

	

	for(int i=0;i<decals.howmanydecals;i++){

		decals.DeleteDecal(0);

	}*/

	decals.howmanydecals=0;

	sprites.howmanysprites=0;

	

	losedelay=1;

		

	oldscore=score;

}



/*********************> InitGL() <*****/





static const char facebuttonslabel[]="Controller face buttons (-1 = work it out, 0 = as SDL reports them, 1 = swap A/B and X/Y):";

static const char assassinslabel[]="Assassins (1 = original; each mission's own number, times this):";

static const char viewdistancelabel[]="View distance (1 = original, no upper limit; the fog, the crowd and the assassins follow):";

/* config.txt, in the order InitGL() reads it back.  The last five lines are
   newer than the original game; a config without them still loads. */
void Game::WriteConfig(const char *path)
{
	ofstream opstream(path);
	if(!opstream)return;
	opstream << "Screenwidth:\n";
	opstream << screenwidth;
	opstream << "\nScreenheight:\n";
	opstream << screenheight;
	opstream << "\nMouse sensitivity:\n";
	opstream << usermousesensitivity;
	opstream << "\nShow fps and other info:\n";
	opstream << debug;
	opstream << "\nVBL sync:\n";
	opstream << vblsync;
	opstream << "\nBlood:\n";
	opstream << blood;
	opstream << "\nBlur:\n";
	opstream << blurness;
	opstream << "\nMain Menu:\n";
	opstream << mainmenuness;
	opstream << "\nCustom levels:\n";
	opstream << customlevels;
	opstream << "\nMusic:\n";
	opstream << musictoggle;
	opstream << "\nAzerty keyboard:\n";
	opstream << azertykeyboard;
	opstream << "\nAntialiasing (0, 2, 4 or 8):\n";
	opstream << antialiasing;
	opstream << "\nStart fullscreen:\n";
	opstream << startfullscreen;
	opstream << "\nFps (frames per second) limit:\n";
	opstream << fpslimit;
	opstream << "\nFOV (Field of View):\n";
	opstream << fov;
	opstream << "\n" << viewdistancelabel << "\n";
	opstream << viewscale;
	opstream << "\nPopulation density (1 = original; people per block, times this):\n";
	opstream << populationdensity;
	opstream << "\n" << assassinslabel << "\n";
	opstream << assassinmultiplier;
	opstream << "\n" << facebuttonslabel << "\n";
	opstream << swapfacebuttons;
	opstream << "\n";
	opstream.close();
	PlatformCommitSave();
}

int Game::InitGL(void)

{

	//Config

	if(!initialized){

		PlatformInitPaths();

		//Defaults, used when there is no config file yet

		screenwidth = 1280;

		screenheight = 720;

		usermousesensitivity=.7;

		debug=0;

		vblsync=1;

		blood = 1;

		blurness = 1;   //on by default, as in Black Shades Enhanced

		mainmenuness=1;

	    customlevels=0;

	    musictoggle=1;

		azertykeyboard=0;

		antialiasing=0;

		startfullscreen=0;

		fpslimit=300;   //Black Shades Enhanced: was a fixed 90

		fov=100;        //Black Shades Enhanced: was a fixed 90

		viewscale=1;    //view, fog and crowd distance, 1 = the original

		populationdensity=1;   //people per block, 1 = the original

		assassinmultiplier=1;  //each mission's own number of assassins, times this

		swapfacebuttons=-1;    //work out whether this pad reports positions or labels

		mousegrab=true;

		/* Settings live in the user's own directory now - on the Mac that is
		   ~/Library/Application Support/Black Shades/ - because everything
		   inside a signed .app bundle is read-only. */

		char configpath[1024];

		PlatformPrefPath("config.txt", configpath, sizeof(configpath));

		ifstream ipstream(configpath);

	    if(ipstream) {

			ipstream.ignore(256,'\n');

		    ipstream >> screenwidth;

		    ipstream.ignore(256,'\n');

		    ipstream.ignore(256,'\n');

		    ipstream >> screenheight;

		    ipstream.ignore(256,'\n');

		    ipstream.ignore(256,'\n');

		    ipstream >> usermousesensitivity;

		    ipstream.ignore(256,'\n');

		    ipstream.ignore(256,'\n');

		    ipstream >> debug;

		    ipstream.ignore(256,'\n');

		    ipstream.ignore(256,'\n');

		    ipstream >> vblsync;

		    ipstream.ignore(256,'\n');

		    ipstream.ignore(256,'\n');

		    ipstream >> blood;

		    ipstream.ignore(256,'\n');

		    ipstream.ignore(256,'\n');

		    ipstream >> blurness;

		    ipstream.ignore(256,'\n');

		    ipstream.ignore(256,'\n');

		    ipstream >> mainmenuness;

		    ipstream.ignore(256,'\n');

		    ipstream.ignore(256,'\n');

		    ipstream >> customlevels;

		    ipstream.ignore(256,'\n');

		    ipstream.ignore(256,'\n');

		    ipstream >> musictoggle;

		    ipstream.ignore(256,'\n');

		    ipstream.ignore(256,'\n');

		    ipstream >> azertykeyboard;

		    ipstream.ignore(256,'\n');

		    ipstream.ignore(256,'\n');

		    /* Settings added since the original.  A config written by an
		       older version simply runs out of lines here: each one is read
		       into a temporary, so a missing line keeps its default instead
		       of the 0 a failed >> stores, and the file is then rewritten
		       with the new lines so they can be found and changed. */

		    bool complete=true;

		    int tempint;

		    float tempfloat;

		    if(ipstream >> tempint)antialiasing=tempint; else complete=false;

		    ipstream.ignore(256,'\n');

		    ipstream.ignore(256,'\n');

		    if(ipstream >> tempint)startfullscreen=tempint; else complete=false;

		    ipstream.ignore(256,'\n');

		    ipstream.ignore(256,'\n');

		    if(ipstream >> tempint)fpslimit=tempint; else complete=false;

		    ipstream.ignore(256,'\n');

		    ipstream.ignore(256,'\n');

		    if(ipstream >> tempfloat)fov=tempfloat; else complete=false;

		    ipstream.ignore(256,'\n');

		    std::string label;

		    std::getline(ipstream,label);

		    if(label!=viewdistancelabel)complete=false;   //an older label ("up to 4"): write out the current one

		    if(ipstream >> tempfloat)viewscale=tempfloat; else complete=false;

		    ipstream.ignore(256,'\n');

		    ipstream.ignore(256,'\n');

		    if(ipstream >> tempfloat)populationdensity=tempfloat; else complete=false;

		    ipstream.ignore(256,'\n');

		    std::getline(ipstream,label);

		    if(ipstream >> tempfloat){

		        assassinmultiplier=tempfloat;

		        if(label.find('%')!=std::string::npos)assassinmultiplier/=100;   //it was a percentage for a while

		        if(label!=assassinslabel)complete=false;

		    } else complete=false;

		    ipstream.ignore(256,'\n');

		    std::getline(ipstream,label);

		    if(ipstream >> tempint)swapfacebuttons=tempint; else complete=false;

		    if(label!=facebuttonslabel)   //an older line, which meant something else
		    {

		        swapfacebuttons=-1;

		        complete=false;

		    }

		    ipstream.close();

		    if(!complete)WriteConfig(configpath);

	    } else {

	        //No config yet: write one out with the defaults

	        WriteConfig(configpath);

	    }

		if(fpslimit<30)fpslimit=30;

		if(!(fov>=30))fov=30;   //also catches a NaN

		if(fov>160)fov=160;

		if(!(viewscale>=.5))viewscale=.5;   //also catches a NaN

		if(viewscale>10000)viewscale=10000;   //no limit that shows (the whole city is in view long before), but keeps the draw maths in range

		if(!(populationdensity>=0))populationdensity=1;   //negative or NaN: the original's

		if(populationdensity>1000)populationdensity=1000;   //keeps the head count in range (memory runs out long before)

		if(!(assassinmultiplier>=0))assassinmultiplier=1;

		if(swapfacebuttons<0)swapfacebuttons=-1;

		if(swapfacebuttons>1)swapfacebuttons=1;

		/* what the game is actually going to run with, which is worth seeing
		   on a machine you cannot easily poke at (the Switch, over nxlink) */
		fprintf(stderr,"Settings: view distance %g, density %g, assassins %g, face buttons %s\n",
		        viewscale,populationdensity,assassinmultiplier,
		        swapfacebuttons<0?"worked out":(swapfacebuttons?"swapped":"as SDL reports them"));

		/* Room for the biggest crowd a mission can need: clear skies, where
		   people are seen furthest (the spawner in GameTick.cpp works out each
		   mission's own).  Each person is about 125 KB - at density 1, 810 of
		   them (100 MB) at a View distance of 1, 6250 (780 MB) at 4, the whole
		   city 98010 (12 GB) - so a crowd that cannot be had falls back to a
		   thinner one rather than failing. */
		int radius=(int)ceil((PeopleVisibleDistance(sunny_environment)+block_spacing/2)/block_spacing);

		if(radius<1)radius=1;

		if(radius>max_spawn_radius)radius=max_spawn_radius;

		const int side=2*radius+1<num_blocks-1?2*radius+1:num_blocks-1;

		auto crowdsize=[&](){

			blockpeople=block_people*populationdensity;

			int people=(int)(blockpeople*side*side+.5);

#ifdef __SWITCH__
			/* hbmenu leaves homebrew a few hundred MB, and a person is about
			   125 KB, so the crowd stops well short of that however long the
			   View distance is */
			const int budget=(int)(112*1024*1024/sizeof(Person));

			if(people>budget)people=budget;
#endif

			return people>2?people:2;

		};

		maxpeople=crowdsize();

		while(!(person=new(std::nothrow) Person[maxpeople]())&&maxpeople>90){   //() zeroes them, as the old fixed array was

			populationdensity/=2;

			maxpeople=crowdsize();

		}

		

		//Read the high scores: one for the normal game, one for revenge mode

		char scorepath[1024];

		const char *scoreread = PlatformUserFileForRead("Highscore", ":Data:Highscore", scorepath, sizeof(scorepath));

		highscore=0;

		beatgame=0;

		if(scoreread){

			ifstream ipstream2(scoreread);

			if(ipstream2){

			    ipstream2 >> highscore;

			    ipstream2.ignore(256,'\n');

			    ipstream2 >> beatgame;

			    ipstream2.close();

			}

		}

		revengehighscore=0;

		revengebeatgame=0;

		PlatformPrefPath("HighscoreRevenge", scorepath, sizeof(scorepath));

		ifstream ipstream3(scorepath);

		if(ipstream3){

		    int tempscore;

		    bool tempbeat=0;

		    if(ipstream3 >> tempscore)revengehighscore=tempscore;

		    ipstream3.ignore(256,'\n');

		    if(ipstream3 >> tempbeat)revengebeatgame=tempbeat;

		    ipstream3.close();

		}



		sps=40;

		maxfps=fpslimit;

		

		disttest=1;

		cubetest=1;

	}

	//Setup screen
#ifdef OS9 
	if(screenwidth<640||screenheight<480)

		theScreen = SetupScreen( 640, 480 );

	else

		theScreen = SetupScreen( screenwidth, screenheight );



	gOpenGLContext = SetupAGL( ( AGLDrawable )theScreen );

	if ( !gOpenGLContext )

		return 0;
#else
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "SDL video init failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	/* Buttons by position rather than by the label printed on them, so that a
	   Switch Pro pad's bottom button is the same button as an Xbox pad's A.
	   Drivers that ignore this (libnx's) are sorted out by PadFace() in
	   GameLoop.cpp. */
	SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "0");

	/* A controller is optional on a desktop and the only input a Switch has;
	   either way, not finding one is not a reason to stop. */
	if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0)
		fprintf(stderr, "No game controller support: %s\n", SDL_GetError());

	if (screenwidth < 640) screenwidth = 640;
	if (screenheight < 480) screenheight = 480;

	/* Don't open a window bigger than the display it has to fit on. */
	SDL_DisplayMode desktop;
	if (SDL_GetDesktopDisplayMode(0, &desktop) == 0) {
		if (screenwidth > desktop.w) screenwidth = desktop.w;
		if (screenheight > desktop.h) screenheight = desktop.h;
	}

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	if (antialiasing > 1) {
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, antialiasing);
	}

	Uint32 windowflags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
	                     SDL_WINDOW_ALLOW_HIGHDPI;
	if (startfullscreen) windowflags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

#ifdef __SWITCH__
	/* The screen is the window: 1280x720 handheld, 1920x1080 docked, and SDL
	   says when that changes - the size-changed event resizes the viewport
	   exactly as it does on a desktop. */
	windowflags = SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN;
	SDL_DisplayMode switchmode;
	if (SDL_GetCurrentDisplayMode(0, &switchmode) == 0) {
		screenwidth = switchmode.w;
		screenheight = switchmode.h;
	} else {
		screenwidth = 1280;
		screenheight = 720;
	}
	/* Mesa hands homebrew a core profile unless it is asked otherwise, and
	   this renderer is fixed-function through and through. */
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif

	g_window = SDL_CreateWindow("Black Shades",
	                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
	                            screenwidth, screenheight, windowflags);

	if (!g_window && antialiasing > 1) {
		/* fall back to no multisampling rather than refusing to start */
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
		antialiasing = 0;
		g_window = SDL_CreateWindow("Black Shades",
		                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		                            screenwidth, screenheight, windowflags);
	}

	if (!g_window) {
		fprintf(stderr, "Could not create the window: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	g_glcontext = SDL_GL_CreateContext(g_window);

#ifdef __SWITCH__
	if (!g_glcontext) {
		/* no compatibility profile on offer: take whatever there is, so that
		   PlatformLoadGL() can say which calls are missing */
		fprintf(stderr, "No compatibility profile (%s); trying the default\n", SDL_GetError());
		SDL_GL_ResetAttributes();
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		g_glcontext = SDL_GL_CreateContext(g_window);
	}
#endif

	if (!g_glcontext) {
		fprintf(stderr, "Could not create an OpenGL context: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	SDL_GL_MakeCurrent(g_window, g_glcontext);

#ifdef __SWITCH__
	if (!PlatformLoadGL()) exit(EXIT_FAILURE);
#endif

	/* vblsync in config.txt still means what it always did */
	SDL_GL_SetSwapInterval(vblsync ? 1 : 0);

	PlatformUpdateViewportSize(&screenwidth, &screenheight);

	/* The game draws its own pointer on the menu screen. */
	SDL_ShowCursor(SDL_DISABLE);

	if (antialiasing > 1) glEnable(GL_MULTISAMPLE);

	printf("Black Shades: %s (%s)\n", glGetString(GL_RENDERER), glGetString(GL_VERSION));
	printf("              data in %s\n", PlatformDataRoot());
	printf("              settings in %s\n", PlatformPrefRoot());
#endif
		

	text.LoadFontTexture(":Data:Textures:Font.png");

	text.BuildFont();

	glAlphaFunc(GL_GREATER, 0.01);

	glDepthFunc(GL_LESS);

	

	glPolygonOffset(-8,0);

	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	

	return TRUE;

}



//***************> Dispose() <******/

void Game::Dispose()										

{
#ifdef OS9 
	CleanupAGL( gOpenGLContext );

	ShutdownScreen( theScreen );

	ShowCursor();
#else
	//Delete sound sources, then shut the mixer and the window down in order

	alDeleteSources(37, gSourceID);

	Audio_Shutdown();

	if (g_glcontext) {
		SDL_GL_DeleteContext(g_glcontext);
		g_glcontext = NULL;
	}

	if (g_window) {
		SDL_DestroyWindow(g_window);
		g_window = NULL;
	}

	SDL_Quit();
#endif

}



//***************> ResizeGLScene() <******/

GLvoid Game::ReSizeGLScene(float fov, float near)		

{

	if (screenheight==0)										

	{

		screenheight=1;									

	}

	/* screenwidth/screenheight are in window points; on a Retina display the
	   drawable is bigger than that, and the viewport wants pixels. */

	PlatformSetViewport();

	glMatrixMode(GL_PROJECTION);						

	glLoadIdentity();								

	/* what gluPerspective() used to do, minus the dependency on GLU */

	double frustumheight=tan(fov*(3.14159265358979/360.0))*near;

	double frustumwidth=frustumheight*((double)screenwidth/(double)screenheight);

	glFrustum(-frustumwidth,frustumwidth,-frustumheight,frustumheight,near,viewdistance);

	glMatrixMode(GL_MODELVIEW);							

	glLoadIdentity();									

}
