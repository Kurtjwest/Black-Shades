#include "Game.h"	



extern double multiplier;

extern int visions;

extern unsigned int gSourceID[100];

extern unsigned int gSampleSet[100];

extern Camera camera;

extern float rad2deg;

extern Fog fog;

extern int environment;

extern int slomo;

/********************> HandleKeyDown() <*****/

void	Game::HandleKeyDown( char theChar )

{

	XYZ facing;

	

	if(!mainmenu){

	switch( theChar )

	{

		case 'l': 
		        if(!lasersight==1){lasersight=1;}else{lasersight=0;} 
			
			break;

		case 'k':

			if(debug)timeremaining=0;

			break;

		

		case 'b':

			if(debug){

			alSourcePlay(gSourceID[soulinsound]);

			if(!slomo)slomo=1;

			else slomo=0;}

			if(slomo){

				alSourcef(gSourceID[knifesong], AL_PITCH, (ALfloat)(.5));

				alSourcef(gSourceID[shootsong], AL_PITCH, (ALfloat)(.5));

				alSourcef(gSourceID[zombiesong], AL_PITCH, (ALfloat)(.5));

			}

			if(!slomo){

				alSourcef(gSourceID[knifesong], AL_PITCH, (ALfloat)(1));

				alSourcef(gSourceID[shootsong], AL_PITCH, (ALfloat)(1));

				alSourcef(gSourceID[zombiesong], AL_PITCH, (ALfloat)(1));

			}

			break;

		case 'B':

			if(debug){

			alSourcePlay(gSourceID[soulinsound]);

			paused=1-paused;}

			break;

		case 'f':

			if(debug){

			alSourcePlay(gSourceID[souloutsound]);

			//Facing

			facing=0;

			facing.z=-1;

			

			facing=DoRotation(facing,-camera.rotation2,0,0);

			facing=DoRotation(facing,0,0-camera.rotation,0);

			for(int i=1;i<numpeople;i++){				

				if(person[i].skeleton.free!=1){

				if(findDistancefast(person[i].playercoords,person[0].playercoords)<1000){

				person[i].skeleton.free=1;

				person[i].longdead=1;

				for(int j=0;j<person[i].skeleton.num_joints;j++){

					person[i].skeleton.joints[j].position=DoRotation(person[i].skeleton.joints[j].position,0,person[i].playerrotation,0);

					person[i].skeleton.joints[j].position+=person[i].playercoords;

					person[i].skeleton.joints[j].realoldposition=person[i].skeleton.joints[j].position;

					person[i].skeleton.joints[j].velocity=DoRotation(person[i].skeleton.joints[j].velocity,0,person[i].playerrotation,0);

					person[i].skeleton.joints[j].velocity+=person[i].velocity;

					person[i].skeleton.joints[j].velocity+=facing*50;

					person[i].skeleton.joints[j].velocity.x+=abs(Random()%20)-10;

					person[i].skeleton.joints[j].velocity.y+=abs(Random()%20)-10;

					person[i].skeleton.joints[j].velocity.z+=abs(Random()%20)-10;

				}}}

			}}

			break;

		case 'X':

			if(debug){

			if(person[0].grenphase==0){

				person[0].ammo=-1;

				person[0].whichgun++;

				person[0].grenphase=0;

				person[0].reloads[person[0].whichgun]=3;

				if(person[0].whichgun>7)person[0].whichgun=0;

			}}

			break;

	}

	}

}



/********************> DoEvent() <*****/

#ifdef OS9 
void	Game::DoEvent( EventRecord *event )

{

	

	char	theChar;

	

	switch ( event->what )

	{

		case keyDown:

		case autoKey:

			theChar = event->message & charCodeMask;	// Get the letter of the key pressed from the event message

			HandleKeyDown( theChar );					// Only some key presses are handled here because it is slower and less responsive

			break;

	}

	

	

}
#endif

#ifndef OS9 
/* ------------------------------------------------------------------
   Keyboard.

   The game thinks in Mac OS virtual key codes: GetKeys() fills a 16-byte
   bitmap that IsKeyDown() then tests.  Mac virtual key codes describe key
   *positions*, so the modern equivalent is an SDL scancode rather than a
   keycode - which also keeps WASD under the same fingers on an AZERTY
   keyboard, exactly like the original did on a Mac.
   ------------------------------------------------------------------ */

static int mapinit = 0;
static int scancodemap[SDL_NUM_SCANCODES];

static unsigned char ourkeys[16];

/* and what a game controller is holding, kept apart so that letting go of a
   stick never releases a key the keyboard has down (see PadUpdate below) */
static unsigned char padkeys[16];

/* Keys pressed during the current frame's events, and presses that also
   ended during it.  The game only looks at which keys are down once a frame,
   so a tap shorter than a frame (easy at a low frame rate) used to vanish:
   Esc, say, never opened the menu.  Such a key now stays down until the next
   frame's events. */
static unsigned char pressedthisframe[16];
static unsigned char releaselater[16];

static void BeginKeyFrame()
{
	for (int i = 0; i < 16; i++) {
		ourkeys[i] &= ~releaselater[i];
		releaselater[i] = 0;
		pressedthisframe[i] = 0;
	}
}

static void init_scancodemap()
{
	int i;

	for (i = 0; i < SDL_NUM_SCANCODES; i++) {
		scancodemap[i] = -1;
	}

	scancodemap[SDL_SCANCODE_1] = MAC_1_KEY;
	scancodemap[SDL_SCANCODE_2] = MAC_2_KEY;
	scancodemap[SDL_SCANCODE_3] = MAC_3_KEY;
	scancodemap[SDL_SCANCODE_4] = MAC_4_KEY;
	scancodemap[SDL_SCANCODE_5] = MAC_5_KEY;
	scancodemap[SDL_SCANCODE_6] = MAC_6_KEY;
	scancodemap[SDL_SCANCODE_7] = MAC_7_KEY;
	scancodemap[SDL_SCANCODE_8] = MAC_8_KEY;
	scancodemap[SDL_SCANCODE_9] = MAC_9_KEY;
	scancodemap[SDL_SCANCODE_0] = MAC_0_KEY;
	scancodemap[SDL_SCANCODE_KP_1] = MAC_NUMPAD_1_KEY;
	scancodemap[SDL_SCANCODE_KP_2] = MAC_NUMPAD_2_KEY;
	scancodemap[SDL_SCANCODE_KP_3] = MAC_NUMPAD_3_KEY;
	scancodemap[SDL_SCANCODE_KP_4] = MAC_NUMPAD_4_KEY;
	scancodemap[SDL_SCANCODE_KP_5] = MAC_NUMPAD_5_KEY;
	scancodemap[SDL_SCANCODE_KP_6] = MAC_NUMPAD_6_KEY;
	scancodemap[SDL_SCANCODE_KP_7] = MAC_NUMPAD_7_KEY;
	scancodemap[SDL_SCANCODE_KP_8] = MAC_NUMPAD_8_KEY;
	scancodemap[SDL_SCANCODE_KP_9] = MAC_NUMPAD_9_KEY;
	scancodemap[SDL_SCANCODE_KP_0] = MAC_NUMPAD_0_KEY;
	scancodemap[SDL_SCANCODE_A] = MAC_A_KEY;
	scancodemap[SDL_SCANCODE_B] = MAC_B_KEY;
	scancodemap[SDL_SCANCODE_C] = MAC_C_KEY;
	scancodemap[SDL_SCANCODE_D] = MAC_D_KEY;
	scancodemap[SDL_SCANCODE_E] = MAC_E_KEY;
	scancodemap[SDL_SCANCODE_F] = MAC_F_KEY;
	scancodemap[SDL_SCANCODE_G] = MAC_G_KEY;
	scancodemap[SDL_SCANCODE_H] = MAC_H_KEY;
	scancodemap[SDL_SCANCODE_I] = MAC_I_KEY;
	scancodemap[SDL_SCANCODE_J] = MAC_J_KEY;
	scancodemap[SDL_SCANCODE_K] = MAC_K_KEY;
	scancodemap[SDL_SCANCODE_L] = MAC_L_KEY;
	scancodemap[SDL_SCANCODE_M] = MAC_M_KEY;
	scancodemap[SDL_SCANCODE_N] = MAC_N_KEY;
	scancodemap[SDL_SCANCODE_O] = MAC_O_KEY;
	scancodemap[SDL_SCANCODE_P] = MAC_P_KEY;
	scancodemap[SDL_SCANCODE_Q] = MAC_Q_KEY;
	scancodemap[SDL_SCANCODE_R] = MAC_R_KEY;
	scancodemap[SDL_SCANCODE_S] = MAC_S_KEY;
	scancodemap[SDL_SCANCODE_T] = MAC_T_KEY;
	scancodemap[SDL_SCANCODE_U] = MAC_U_KEY;
	scancodemap[SDL_SCANCODE_V] = MAC_V_KEY;
	scancodemap[SDL_SCANCODE_W] = MAC_W_KEY;
	scancodemap[SDL_SCANCODE_X] = MAC_X_KEY;
	scancodemap[SDL_SCANCODE_Y] = MAC_Y_KEY;
	scancodemap[SDL_SCANCODE_Z] = MAC_Z_KEY;
	scancodemap[SDL_SCANCODE_F1] = MAC_F1_KEY;
	scancodemap[SDL_SCANCODE_F2] = MAC_F2_KEY;
	scancodemap[SDL_SCANCODE_F3] = MAC_F3_KEY;
	scancodemap[SDL_SCANCODE_F4] = MAC_F4_KEY;
	scancodemap[SDL_SCANCODE_F5] = MAC_F5_KEY;
	scancodemap[SDL_SCANCODE_F6] = MAC_F6_KEY;
	scancodemap[SDL_SCANCODE_F7] = MAC_F7_KEY;
	scancodemap[SDL_SCANCODE_F8] = MAC_F8_KEY;
	scancodemap[SDL_SCANCODE_F9] = MAC_F9_KEY;
	scancodemap[SDL_SCANCODE_F10] = MAC_F10_KEY;
	scancodemap[SDL_SCANCODE_F11] = MAC_F11_KEY;
	scancodemap[SDL_SCANCODE_F12] = MAC_F12_KEY;
	scancodemap[SDL_SCANCODE_RETURN] = MAC_RETURN_KEY;
	scancodemap[SDL_SCANCODE_KP_ENTER] = MAC_ENTER_KEY;
	scancodemap[SDL_SCANCODE_TAB] = MAC_TAB_KEY;
	scancodemap[SDL_SCANCODE_SPACE] = MAC_SPACE_KEY;
	scancodemap[SDL_SCANCODE_BACKSPACE] = MAC_DELETE_KEY;
	scancodemap[SDL_SCANCODE_ESCAPE] = MAC_ESCAPE_KEY;
	scancodemap[SDL_SCANCODE_LCTRL] = MAC_CONTROL_KEY;
	scancodemap[SDL_SCANCODE_RCTRL] = MAC_CONTROL_KEY;
	scancodemap[SDL_SCANCODE_LSHIFT] = MAC_SHIFT_KEY;
	scancodemap[SDL_SCANCODE_RSHIFT] = MAC_SHIFT_KEY;
	scancodemap[SDL_SCANCODE_CAPSLOCK] = MAC_CAPS_LOCK_KEY;
	scancodemap[SDL_SCANCODE_LALT] = MAC_OPTION_KEY;
	scancodemap[SDL_SCANCODE_RALT] = MAC_OPTION_KEY;
	/* the command key: the game checks for cmd-Q itself */
	scancodemap[SDL_SCANCODE_LGUI] = MAC_COMMAND_KEY;
	scancodemap[SDL_SCANCODE_RGUI] = MAC_COMMAND_KEY;
	scancodemap[SDL_SCANCODE_PAGEUP] = MAC_PAGE_UP_KEY;
	scancodemap[SDL_SCANCODE_PAGEDOWN] = MAC_PAGE_DOWN_KEY;
	scancodemap[SDL_SCANCODE_INSERT] = MAC_INSERT_KEY;
	scancodemap[SDL_SCANCODE_DELETE] = MAC_DEL_KEY;
	scancodemap[SDL_SCANCODE_HOME] = MAC_HOME_KEY;
	scancodemap[SDL_SCANCODE_END] = MAC_END_KEY;
	scancodemap[SDL_SCANCODE_LEFTBRACKET] = MAC_LEFT_BRACKET_KEY;
	scancodemap[SDL_SCANCODE_RIGHTBRACKET] = MAC_RIGHT_BRACKET_KEY;
	scancodemap[SDL_SCANCODE_UP] = MAC_ARROW_UP_KEY;
	scancodemap[SDL_SCANCODE_DOWN] = MAC_ARROW_DOWN_KEY;
	scancodemap[SDL_SCANCODE_LEFT] = MAC_ARROW_LEFT_KEY;
	scancodemap[SDL_SCANCODE_RIGHT] = MAC_ARROW_RIGHT_KEY;

	mapinit = 1;
}

void GetKeys(unsigned long *keys)
{
	/* the keyboard's keys and the controller's, whichever is holding them */
	unsigned char *out = (unsigned char *)keys;
	for (size_t i = 0; i < sizeof(ourkeys); i++) out[i] = ourkeys[i] | padkeys[i];
}

/* Dropping every key is the right thing to do when the window stops being
   the one receiving them, otherwise the player keeps running after alt-tab. */
static void ForgetKeys()
{
	memset(ourkeys, 0, sizeof(ourkeys));
	memset(padkeys, 0, sizeof(padkeys));
	memset(pressedthisframe, 0, sizeof(pressedthisframe));
	memset(releaselater, 0, sizeof(releaselater));
}

static void DoSDLKey(Game *g, SDL_Event *event)
{
	int press = (event->type == SDL_KEYDOWN) ? 1 : 0;
	SDL_Scancode scancode = event->key.keysym.scancode;

	if (mapinit == 0) {
		init_scancodemap();
	}

	if (scancode > 0 && scancode < SDL_NUM_SCANCODES) {
		int mackey = scancodemap[scancode];

		/* Both Ctrl keys (and both Shifts, Options, Commands) map to one Mac
		   key, so letting go of one of a pair used to release it while the
		   other was still held - dropping you out of a crouch, and the sniper
		   scope.  SDL's modifier state knows about the other one (Black Shades
		   Enhanced's issue #7 read Ctrl from it too). */
		bool held = press != 0;
		if (!held) {
			const SDL_Keymod mods = SDL_GetModState();
			if (mackey == MAC_CONTROL_KEY && (mods & KMOD_CTRL)) held = true;
			if (mackey == MAC_SHIFT_KEY && (mods & KMOD_SHIFT)) held = true;
			if (mackey == MAC_OPTION_KEY && (mods & KMOD_ALT)) held = true;
			if (mackey == MAC_COMMAND_KEY && (mods & KMOD_GUI)) held = true;
		}

		if (mackey != -1) {
			int index = mackey / 8;
			int mask = 1 << (mackey % 8);

			if (held) {
				ourkeys[index] |= mask;
				if (press) {
					pressedthisframe[index] |= mask;
					releaselater[index] &= ~mask;   /* pressed again: still down */
				}
			} else if (pressedthisframe[index] & mask) {
				releaselater[index] |= mask;   /* let the game see the tap once */
			} else {
				ourkeys[index] &= ~mask;
			}
		}
	}

	/* HandleKeyDown() takes a character.  Deriving it from the keycode (rather
	   than using SDL_TEXTINPUT) keeps auto-repeat from flipping these toggles
	   over and over while a key is held down. */
	if (press && !event->key.repeat) {
		SDL_Keycode sym = event->key.keysym.sym;

		if (sym >= SDLK_a && sym <= SDLK_z) {
			char c = (char)sym;
			if (event->key.keysym.mod & KMOD_SHIFT) {
				c = (char)(c - 'a' + 'A');
			}
			g->HandleKeyDown(c);
		}
	}
}

/* ----------------------------------------------------------------------
   Game controllers, on every platform.  The game's own controls are in the
   original Readme; these are all of them, in the same shape:

     left stick        walk (the keys config.txt binds); pushed all the way,
                       run - and in the menu it moves the pointer
     right stick       look around
     right trigger     click: fire while aiming, smash while running, pick up
                       a gun while crouched over a body, otherwise disarm
     left trigger      crouch, which is also the sniper scope
     bottom button     dive (space), and click in the menu
     left button       reload
     top button        aim and un-aim (the q key: you shoot while aiming and
                       pick guns up while not)
     right button      laser sight on and off
     left shoulder     psychic aim - hold it for the slow-motion (e)
     right shoulder    soul release on and off (z)
     d-pad up          the debug weapon key, which does nothing otherwise
     start (+)         the menu, as Esc
     back (-)          save and quit (Switch only: elsewhere there is Cmd-Q)

   Buttons are named by POSITION above, because that is what you press: the
   bottom button is A on an Xbox pad and B on a Nintendo one.  Which of the
   two SDL reports depends on the driver - macOS hands a Nintendo pad over by
   label, the Switch's own driver reports positions - so the port asks SDL
   what kind of controller it is and swaps the pairs for a Nintendo one,
   except on the Switch itself.  config.txt's "Controller face buttons"
   overrides that guess with 0 (as SDL reports them) or 1 (swapped).

   What the pad holds down is kept in its own key map, ORed with the
   keyboard's in GetKeys(), so that letting go of a stick never releases a
   key the keyboard is holding (and the other way round).
   ---------------------------------------------------------------------- */
static SDL_GameController *g_pad = NULL;
static bool g_padswap = false;   /* swap A/B and X/Y for this pad */
static bool g_padswapguess = false;   /* what the pad looks like it needs */

/* the keys config.txt binds (Globals.cpp owns them) */
extern int forwardskey, backwardskey, leftkey, rightkey, aimkey, psychicaimkey, psychickey;

enum PadPosition { pad_bottom, pad_right, pad_top, pad_left };

/* A/B are the bottom and right buttons and X/Y the left and top ones, unless
   the swap is on, which exchanges each pair - that is the difference between
   an Xbox pad's labels and a Nintendo pad's. */
static SDL_GameControllerButton PadFace(PadPosition where)
{
	switch (where) {
		case pad_bottom: return g_padswap ? SDL_CONTROLLER_BUTTON_B : SDL_CONTROLLER_BUTTON_A;
		case pad_right:  return g_padswap ? SDL_CONTROLLER_BUTTON_A : SDL_CONTROLLER_BUTTON_B;
		case pad_top:    return g_padswap ? SDL_CONTROLLER_BUTTON_X : SDL_CONTROLLER_BUTTON_Y;
		case pad_left:   return g_padswap ? SDL_CONTROLLER_BUTTON_Y : SDL_CONTROLLER_BUTTON_X;
	}
	return SDL_CONTROLLER_BUTTON_A;
}

static void PadOpen(int joystickindex)
{
	if (g_pad || !SDL_IsGameController(joystickindex)) return;
	g_pad = SDL_GameControllerOpen(joystickindex);
	if (!g_pad) return;

#ifdef __SWITCH__
	/* libnx reports its buttons by position, whatever they are labelled */
	g_padswapguess = false;
#else
	/* SDL hands a Nintendo pad over by label unless its driver honours the
	   hint set in InitGL, which not all of them do (macOS's does not) */
	const SDL_GameControllerType type = SDL_GameControllerGetType(g_pad);
	g_padswapguess = type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO ||
	                 type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT ||
	                 type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT ||
	                 type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR;
#endif
	fprintf(stderr, "Controller: %s - face buttons %s\n", SDL_GameControllerName(g_pad),
	        g_padswapguess ? "swapped: the bottom button is SDL's B" :
	                         "as SDL reports them: the bottom button is SDL's A");
}

static void PadClose(SDL_JoystickID which)
{
	if (!g_pad) return;
	SDL_Joystick *stick = SDL_GameControllerGetJoystick(g_pad);
	if (stick && SDL_JoystickInstanceID(stick) != which) return;
	SDL_GameControllerClose(g_pad);
	g_pad = NULL;
	memset(padkeys, 0, sizeof(padkeys));
}

static void PadSetKey(int mackey, bool down)
{
	if (mackey < 0 || mackey >= (int)sizeof(padkeys) * 8) return;
	const int index = mackey / 8, mask = 1 << (mackey % 8);
	if (down) padkeys[index] |= mask;
	else padkeys[index] &= ~mask;
}

static float PadAxis(SDL_GameControllerAxis axis)
{
	const float v = SDL_GameControllerGetAxis(g_pad, axis) / 32767.f;
	return (v > -.25f && v < .25f) ? 0.f : v;   //deadzone
}

static bool PadButton(SDL_GameControllerButton button)
{
	return SDL_GameControllerGetButton(g_pad, button) != 0;
}

static void PadUpdate(Game *g)
{
	memset(padkeys, 0, sizeof(padkeys));   //rebuilt from what is held right now

	/* config.txt has the last word; -1 leaves it to what the pad looks like */
	g_padswap = g->swapfacebuttons < 0 ? g_padswapguess : g->swapfacebuttons != 0;

	if (!g_pad) {
		/* nothing plugged in yet, or plugged in before we were listening */
		for (int i = 0; i < SDL_NumJoysticks() && !g_pad; i++) PadOpen(i);
		if (!g_pad) return;
	}

	/* the sticks and buttons as they are now, not as they were last frame */
	SDL_GameControllerUpdate();

	const float movex = PadAxis(SDL_CONTROLLER_AXIS_LEFTX);
	const float movey = PadAxis(SDL_CONTROLLER_AXIS_LEFTY);
	const bool inmenu = g->mainmenu != 0;
	const bool fire = SDL_GameControllerGetAxis(g_pad, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 8000;
	const bool dive = PadButton(PadFace(pad_bottom));

	/* Esc and the click work in the menu; everything else is playing */
	PadSetKey(MAC_ESCAPE_KEY, PadButton(SDL_CONTROLLER_BUTTON_START));
	PlatformPadButton(fire || (inmenu && dive));

	if (inmenu) {
		/* the menu is a pointer and a click, so the left stick drives both */
		PlatformPadPointerMove((int)(movex * 14), (int)(movey * 14));
		return;
	}

	PadSetKey(forwardskey,  movey < -.35f);
	PadSetKey(backwardskey, movey >  .35f);
	PadSetKey(leftkey,      movex < -.35f);
	PadSetKey(rightkey,     movex >  .35f);
	/* all the way over is a run, so the stick alone paces you */
	PadSetKey(MAC_SHIFT_KEY, movex * movex + movey * movey > .72f);

	/* Squared, so that a small push turns you slowly and the edge of the
	   stick still whips round: pad_look is the whole sensitivity knob (about
	   185 degrees a second at the edge, before config.txt's own mouse
	   sensitivity). */
	const float lookx = PadAxis(SDL_CONTROLLER_AXIS_RIGHTX);
	const float looky = PadAxis(SDL_CONTROLLER_AXIS_RIGHTY);
	const float pad_look = 14;
	PlatformPadMouse((int)(lookx * fabs(lookx) * pad_look),
	                 (int)(looky * fabs(looky) * pad_look));

	PadSetKey(MAC_CONTROL_KEY,
	          SDL_GameControllerGetAxis(g_pad, SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 8000);
	PadSetKey(MAC_SPACE_KEY, dive);
	PadSetKey(aimkey,        PadButton(PadFace(pad_top)));
	PadSetKey(MAC_R_KEY,     PadButton(PadFace(pad_left)));
	PadSetKey(psychicaimkey, PadButton(SDL_CONTROLLER_BUTTON_LEFTSHOULDER));
	PadSetKey(psychickey,    PadButton(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER));

	/* the two that are characters rather than keys, once per press */
	static bool oldlaser = false, oldweapon = false, oldquit = false;
	const bool laser = PadButton(PadFace(pad_right));
	const bool weapon = PadButton(SDL_CONTROLLER_BUTTON_DPAD_UP);
	const bool quit = PadButton(SDL_CONTROLLER_BUTTON_BACK);
	if (laser && !oldlaser) g->HandleKeyDown('l');
	if (weapon && !oldweapon) g->HandleKeyDown('X');   //debug only, as the key is
#ifdef __SWITCH__
	if (quit && !oldquit) g->RequestQuit();            //the only way out of a .nro
#endif
	oldlaser = laser; oldweapon = weapon; oldquit = quit;
}

static void ProcessSDLEvents(Game *g)
{
	SDL_Event event;

	BeginKeyFrame();

	PadUpdate(g);

	while (SDL_PollEvent(&event)) {
		switch(event.type) {

#ifdef __SWITCH__
			case SDL_FINGERDOWN:
			case SDL_FINGERMOTION: {
				int w = 1280, h = 720;
				SDL_GetWindowSize(g_window, &w, &h);
				PlatformPadPointerTo((int)(event.tfinger.x * w), (int)(event.tfinger.y * h));
				PlatformPadButton(true);
				break;
			}
#endif
			case SDL_CONTROLLERDEVICEADDED:
				PadOpen(event.cdevice.which);
				break;

			case SDL_CONTROLLERDEVICEREMOVED:
				PadClose(event.cdevice.which);
				break;

			case SDL_QUIT:
				g->RequestQuit();
				break;

			case SDL_WINDOWEVENT:
				switch (event.window.event) {
					case SDL_WINDOWEVENT_SIZE_CHANGED:
					case SDL_WINDOWEVENT_RESIZED:
						PlatformUpdateViewportSize(&g->screenwidth, &g->screenheight);
						break;
					case SDL_WINDOWEVENT_FOCUS_LOST:
						ForgetKeys();
						PlatformSetRelativeMouse(false);
						break;
				}
				break;

			case SDL_KEYDOWN:
				/* window shortcuts first, so the game never sees them */
				if (event.key.keysym.sym == SDLK_RETURN &&
					(event.key.keysym.mod & (KMOD_ALT | KMOD_GUI)))
				{
					PlatformToggleFullscreen();
					break;
				}
				if (event.key.keysym.sym == SDLK_f &&
					(event.key.keysym.mod & KMOD_GUI))
				{
					PlatformToggleFullscreen();
					break;
				}
				if (event.key.keysym.sym == SDLK_g &&
					(event.key.keysym.mod & KMOD_CTRL))
				{
					/* let go of the pointer without leaving the game */
					g->mousegrab = !g->mousegrab;
					break;
				}
				DoSDLKey(g, &event);
				break;

			case SDL_KEYUP:
				DoSDLKey(g, &event);
				break;
		}
	}
}

/* Keep the score somewhere the player can actually write to (inside an .app
   bundle everything is read-only and signed). */
void Game::SaveHighscore()
{
	/* the score only counts towards the mode it was played in */
	if (score > ModeHighscore()) ModeHighscore() = score;

	char path[1024];
	PlatformPrefPath("Highscore", path, sizeof(path));

	ofstream opstream(path);
	if (opstream) {
		opstream << highscore;
		opstream << "\n";
		opstream << beatgame;
		opstream.close();
	}

	PlatformPrefPath("HighscoreRevenge", path, sizeof(path));

	ofstream opstream2(path);
	if (opstream2) {
		opstream2 << revengehighscore;
		opstream2 << "\n";
		opstream2 << revengebeatgame;
		opstream2.close();
	}
	PlatformCommitSave();
}

void Game::RequestQuit()
{
	SaveHighscore();
	gQuit = true;
}

/* The pointer belongs to the game while you are playing, and to you while you
   are in the menu (which draws its own cursor). */
void Game::UpdateMouseGrab()
{
	bool focused = g_window &&
		(SDL_GetWindowFlags(g_window) & SDL_WINDOW_INPUT_FOCUS) != 0;

	PlatformSetRelativeMouse(!mainmenu && mousegrab && focused);
}

#endif


/********************> EventLoop() <*****/

void	Game::EventLoop( void )

{

#ifdef OS9 
	EventRecord		event;
#endif

	unsigned char	theKeyMap[16];

	int colaccuracy,i;

	double oldmult;

	gQuit = false;

	while ( gQuit == false )

	{

#ifdef OS9
		if ( GetNextEvent( everyEvent, &event ) )

			DoEvent( &event );
#else
		ProcessSDLEvents(this);

		UpdateMouseGrab();
#endif


		start=TimerGetTime(&theTimer);

		

		/* framespersecond is 0 until the first frame has been timed; sps/0 is
		   infinite, and converting that to an int is undefined */
		colaccuracy=framespersecond>0?sps/framespersecond+1:1;

		if(colaccuracy>sps){colaccuracy=sps;}

		

		oldmult=multiplier;

		multiplier/=colaccuracy;

		for(i=0;i<(int)(colaccuracy+.5);i++){

			Tick();

		}

		multiplier=oldmult;

		

		if ( DrawGLScene())

#ifdef OS9
			aglSwapBuffers( gOpenGLContext );
#else
			PlatformSwapBuffers();
#endif

		else

			gQuit = true;

		oldmult=multiplier;	

		

		end=TimerGetTime(&theTimer);

		timetaken=end-start;

		framespersecond=600000000/timetaken;

		/* Frame limiter.  The original spun here; sleeping through the bulk
		   of the wait keeps a core (and a laptop battery) free while landing
		   on the same frame time. */
		while(framespersecond>maxfps){

			double timeleft=600000000/(double)maxfps-timetaken;

			if(timeleft>2000000)SDL_Delay(1);

			end=TimerGetTime(&theTimer);

			timetaken=end-start;

			framespersecond=600000000/timetaken;

		}

		multiplier5=multiplier4;

		multiplier4=multiplier3;

		multiplier3=multiplier2;

		multiplier2=1/framespersecond;

		multiplier=(multiplier2+multiplier3+multiplier4+multiplier5)/4;

		if(multiplier>1)multiplier=1;

		if(multiplier<.00001)multiplier=.00001;

		if(visions==1&&mainmenu==0)multiplier/=3;

		if(slomo)multiplier*=.2;

		if(paused)multiplier=0;

		GetKeys( ( unsigned long * )theKeyMap );

		if ( IsKeyDown( theKeyMap, MAC_COMMAND_KEY )&&IsKeyDown( theKeyMap, MAC_Q_KEY )){

			RequestQuit();

		}

		/* once per press, and only from the game: this used to run every frame
		   Esc was held - in the menu too - restarting the menu music each time */
		static bool escapewasdown=false;
		const bool escapedown=IsKeyDown( theKeyMap, MAC_ESCAPE_KEY );
		const bool escapepressed=escapedown&&!escapewasdown;
		escapewasdown=escapedown;
		if ( escapepressed&&!mainmenu ){

			alSourcePause(gSourceID[rainsound]);

		

			mainmenu=1;

			alSourcePlay(gSourceID[souloutsound]);

			flashamount=1;

			flashr=1;flashg=1;flashb=1;

			alSourceStop(gSourceID[visionsound]);

			whichsong=mainmenusong;

			alSourceStop(gSourceID[knifesong]);

			alSourceStop(gSourceID[shootsong]);

			alSourceStop(gSourceID[zombiesong]);

			alSourceStop(gSourceID[mainmenusong]);

			alSourcef(gSourceID[knifesong], AL_MIN_GAIN, 0);

			alSourcef(gSourceID[shootsong], AL_MIN_GAIN, 0);

			alSourcef(gSourceID[zombiesong], AL_MIN_GAIN, 0);

			alSourcef(gSourceID[mainmenusong], AL_MIN_GAIN, 0);

			alSourcePlay(gSourceID[whichsong]);

			alSourcef(gSourceID[whichsong], AL_MIN_GAIN, 1);

		}

	}

}
