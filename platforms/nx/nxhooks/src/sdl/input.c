#include <nxhooks/sdl/input.h>
#include <nxhooks/screen.h>
#include <nxhooks/nmouse.h>
#include <nxhooks/touch.h>
#include <switch.h>
#include <stdbool.h>

#define NX_MAX_CONTROLLERS 8
#define NX_AXIS_DEADZONE 3000
#define NX_AXIS_FAST_SPEEDSTEP (INT16_MAX / NX_SCREEN_WIDTH * 50)
#define NX_AXIS_SLOW_SPEEDSTEP (NX_AXIS_FAST_SPEEDSTEP * 3)

/* SDL's touch->mouse conversion uses SDL_TOUCH_MOUSEID. The stick's synthetic
 * mouse uses its own ID so the touch coordinate fix below never scales it. */
#define NX_STICK_MOUSEID 2

static SDL_GameController* g_controllers[NX_MAX_CONTROLLERS];
static SDL_TimerID g_controllermouse_timer;
static int16_t g_axis_x = 0;
static int16_t g_axis_y = 0;
static bool g_axis_fastspeed = false;
static bool g_speedtoggle_held = false;
static bool g_clicktoggle_held = false;

SDL_Window* SDL_GetFocusWindow(void);
int SDL_SendMouseMotion(SDL_Window* window, int mouseID, int relative, int x, int y);
int SDL_SendMouseButton(SDL_Window* window, int mouseID, uint8_t state, uint8_t button);
int SDL_SendKeyboardKey(uint8_t state, SDL_Scancode scancode);

static void _mapJoycons() {
	for (int i = 0; i < NX_MAX_CONTROLLERS; i++) {
		hidSetNpadJoyAssignmentModeDual((HidNpadIdType)i);
	}
	hidSetNpadJoyHoldType(HidNpadJoyHoldType_Vertical);
}

static bool _isSdlTouchMouseEvent(SDL_Event* event) {
#ifdef __SWITCH2__
	/* SDL's Switch touch backend divides by a hardcoded 1280x720, so its
	 * touch->mouse events only cover the top-left of our 1920x1080 window.
	 * nxHooksTouchPoll() supplies replacement events from HID directly. */
	if (event->type == SDL_MOUSEMOTION && event->motion.which == SDL_TOUCH_MOUSEID) {
		return true;
	}
	if ((event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP) &&
	    event->button.which == SDL_TOUCH_MOUSEID) {
		return true;
	}
#endif
	return false;
}

uint32_t _timerSendControllerMouseMotionEvents(uint32_t interval, void *param) {
	if(g_axis_x != 0 || g_axis_y != 0) {
		int16_t speedstep = g_speedtoggle_held ? NX_AXIS_FAST_SPEEDSTEP : NX_AXIS_SLOW_SPEEDSTEP;

		SDL_SendMouseMotion(NULL, NX_STICK_MOUSEID, 1, g_axis_x / speedstep, g_axis_y / speedstep);
	}

	return interval;
}

static void _mapControllerButton(SDL_GameControllerButton button, bool down) {
	switch(button) {
		case SDL_CONTROLLER_BUTTON_LEFTSTICK:
		case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
				g_axis_fastspeed = !g_axis_fastspeed;
			break;
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
			g_clicktoggle_held = down;
			break;
		case SDL_CONTROLLER_BUTTON_BACK:
		case SDL_CONTROLLER_BUTTON_GUIDE:
		case SDL_CONTROLLER_BUTTON_START:
			if(down) {
				SDL_StopTextInput();
				SDL_StartTextInput();
			}
			break;
		case SDL_CONTROLLER_BUTTON_B: // switch A
			SDL_SendKeyboardKey(down, SDL_SCANCODE_RETURN);
			break;
		case SDL_CONTROLLER_BUTTON_A: // switch B
			SDL_SendKeyboardKey(down, SDL_SCANCODE_SPACE);
			break;
		case SDL_CONTROLLER_BUTTON_X: // switch Y
			SDL_SendKeyboardKey(down, SDL_SCANCODE_LCTRL);
			break;
		case SDL_CONTROLLER_BUTTON_Y: // switch X
			SDL_SendKeyboardKey(down, SDL_SCANCODE_BACKSPACE);
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
			SDL_SendKeyboardKey(down, SDL_SCANCODE_DOWN);
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
			SDL_SendKeyboardKey(down, SDL_SCANCODE_LEFT);
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
			SDL_SendKeyboardKey(down, SDL_SCANCODE_RIGHT);
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
			SDL_SendKeyboardKey(down, SDL_SCANCODE_UP);
			break;
	}
}

static void _SendControllerMouseClick(bool down) {
	if(down) {
		SDL_SendMouseButton(NULL, NX_STICK_MOUSEID, SDL_PRESSED, g_clicktoggle_held ? SDL_BUTTON_RIGHT : SDL_BUTTON_LEFT);
	} else {
		SDL_SendMouseButton(NULL, NX_STICK_MOUSEID, SDL_RELEASED, SDL_BUTTON_LEFT);
		SDL_SendMouseButton(NULL, NX_STICK_MOUSEID, SDL_RELEASED, SDL_BUTTON_RIGHT);
	}
}

int nxHooksSDLPollEvents(SDL_Event* event) {
	/* Switch 2 input sources: native pointer and full-resolution touch. */
	nxHooksNmousePoll();
	nxHooksTouchPoll();

	int result = SDL_PollEvent(event);

	if(result) {
		if (_isSdlTouchMouseEvent(event)) {
			return 0; /* replaced by nxHooksTouchPoll()'s events */
		}

		switch(event->type) {
			case SDL_CONTROLLERAXISMOTION:
				if(event->caxis.axis == SDL_CONTROLLER_AXIS_LEFTX || event->caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX) {
					g_axis_x = abs(event->caxis.value) >= NX_AXIS_DEADZONE ? event->caxis.value : 0;
				} else if(event->caxis.axis == SDL_CONTROLLER_AXIS_LEFTY || event->caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY) {
					g_axis_y = abs(event->caxis.value) >= NX_AXIS_DEADZONE ? event->caxis.value : 0;
				} else if(event->caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || event->caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
					_SendControllerMouseClick(abs(event->caxis.value) >= NX_AXIS_DEADZONE);
				}
				result = 0;
				break;
			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
				_mapControllerButton(event->cbutton.button, event->cbutton.state);
				result = 0;
				break;
		}
	}

	return result;
}

void nxHooksSDLInputInit() {
	SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "1");
	if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "[nxhooks] Couldn't initialize required SDL subsystems: %s\n", SDL_GetError());
	}

#ifdef __SWITCH2__
	/* Must be after SDL_Init: the Switch SDL backend sets this hint itself at
	 * init, and its touch->mouse conversion uses a hardcoded 1280x720 range.
	 * Turning it off at the source matters because MilkyTracker coalesces
	 * motion events with SDL_PeepEvents(), bypassing our poll-time filter;
	 * any bogus event left in the queue makes the cursor jump back to the
	 * 720p position. Touch is supplied by nxHooksTouchPoll() from HID. */
	SDL_SetHintWithPriority(SDL_HINT_TOUCH_MOUSE_EVENTS, "0", SDL_HINT_OVERRIDE);
#endif

	_mapJoycons();

	int numJoysticks = SDL_NumJoysticks();
	for (int i = 0; i < NX_MAX_CONTROLLERS; i++) {
		if(i < numJoysticks && SDL_IsGameController(i)) {
			g_controllers[i] = SDL_GameControllerOpen(i);
		} else {
			g_controllers[i] = NULL;
		}
		fprintf(stderr, "[nxhooks] Controller %d: %p\n", g_controllers[i]);
	}

	g_controllermouse_timer = SDL_AddTimer(16, _timerSendControllerMouseMotionEvents, NULL);
}

void nxHooksSDLInputExit() {
	SDL_RemoveTimer(g_controllermouse_timer);

	SDL_QuitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_TIMER);
}
