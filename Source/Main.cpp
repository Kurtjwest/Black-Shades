/********************> Headers <*****/

#include "Game.h"

#ifdef __SWITCH__
#include <switch.h>
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
	/* The build defines SDL_MAIN_HANDLED, so SDL does not rename main() out
	   from under us; this is the handshake that replaces that. */

	SDL_SetMainReady();

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
#endif




	game.InitGL();



	game.InitGame();



	game.EventLoop();



	game.Dispose();

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
