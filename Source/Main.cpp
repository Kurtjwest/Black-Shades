/********************> Headers <*****/

#include "Game.h"

#ifdef __SWITCH__
#include <switch.h>
#endif

#ifdef __wii__
#include <gccore.h>
#endif



/********************> Globals <*****/



Game game;



/********************> main() <*****/

/* A browser has no command line to pass in, and taking argc/argv renames
   main() to __main_argc_argv - which some Emscripten releases (Debian's 3.1.6
   among them) then fail to find, linking a module with nothing in it. */
#ifdef __EMSCRIPTEN__
int	main( void )
#else
int 	main( int argc, char *argv[] )
#endif

	{

#ifndef __EMSCRIPTEN__
	(void)argc; (void)argv;
#endif


#ifdef OS9
	ToolboxInit();



	if ( HasAppearance() )

		RegisterAppearanceClient();
#else
#ifndef __wii__
	/* Everywhere but the Wii the build defines SDL_MAIN_HANDLED, so SDL does
	   not rename main() out from under us; this is the handshake that
	   replaces that.  On a Wii SDL's own main() has already run - it brings
	   up IOS, the Wiimotes (with the IR pointer) and the SD card - and this
	   function is what it calls. */

	SDL_SetMainReady();
#endif

#ifdef __SWITCH__
	/* Data lives in the .nro's romfs; without this nothing can be opened. */
	Result romfs = romfsInit();
	if (R_FAILED(romfs)) {
		fprintf(stderr, "romfsInit() failed (%08x): no Data to load\n", (unsigned)romfs);
		return 1;
	}
#ifdef NXLINK
	/* printf() over the network: run "nxlink -s blackshades.nro" to see it */
	socketInitializeDefault();
	nxlinkStdio();
#endif
#endif

	PlatformInitPaths();

#ifdef __wii__
	/* Start the log fresh, and say what there is to work with: the crowd is
	   the whole memory question on this console. */
	remove("sd:/apps/blackshades/blackshades.log");
	PlatformLogf("Black Shades starting: MEM1 arena %u KB, MEM2 arena %u KB\n",
	             (unsigned)(SYS_GetArenaSize()/1024),
	             (unsigned)(SYS_GetArena2Size()/1024));
	/* which binary wrote this log: without it there is no telling a report
	   from a build that has the fix from one that has not */
	PlatformLogf("built %s %s\n", __DATE__, __TIME__);
#endif
#endif




	game.InitGL();



	game.InitGame();



	game.EventLoop();



	game.Dispose();

	PlatformShutdown();   /* on a Wii: flush and unmount the card */

#ifdef __SWITCH__
#ifdef NXLINK
	socketExit();
#endif
	romfsExit();
#endif


#ifdef OS9
	if ( HasAppearance() )

		UnregisterAppearanceClient();



	FlushEvents( everyEvent, 0 );

	ExitToShell();
#endif


	return 0;

}
