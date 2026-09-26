/********************> Headers <*****/

#include "Game.h"

#ifdef __SWITCH__
#include <switch.h>
#endif



/********************> Globals <*****/



Game game;



/********************> main() <*****/

int 	main( int argc, char *argv[] )

	{

	(void)argc; (void)argv;


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
