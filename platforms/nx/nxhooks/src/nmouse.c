#include <nxhooks/nmouse.h>

#ifdef __SWITCH2__

#include <switch.h>
#include <SDL.h>
#include <stdio.h>

/* Dedicated SDL mouse ID so the native pointer's events don't collide with
 * the synthetic touch / controller mouse events used by the NX port. */
#define NX_NMOUSE_MOUSEID 1
#define NX_NMOUSE_MAX_SAMPLES 16

static bool g_nmouse_initialized = false;
static bool g_has_handle = false;
static HidNmouseHandle g_merged_handle;
static u64 g_last_sample = 0;
static bool g_have_sample = false;
static u32 g_last_buttons = 0;

SDL_Window* SDL_GetFocusWindow(void);
int SDL_SendMouseMotion(SDL_Window* window, int mouseID, int relative, int x, int y);
int SDL_SendMouseButton(SDL_Window* window, int mouseID, Uint8 state, Uint8 button);
int SDL_SendMouseWheel(SDL_Window* window, int mouseID, int x, int y, SDL_MouseWheelDirection direction);

static void _sendButtonChanges(SDL_Window* window, u32 buttons, u32 oldButtons)
{
	static const struct { u32 hid; Uint8 sdl; } buttonMap[] = {
		{ HidMouseButton_Left,    SDL_BUTTON_LEFT   },
		{ HidMouseButton_Right,   SDL_BUTTON_RIGHT  },
		{ HidMouseButton_Middle,  SDL_BUTTON_MIDDLE },
		{ HidMouseButton_Forward, SDL_BUTTON_X1     },
		{ HidMouseButton_Back,    SDL_BUTTON_X2     },
	};

	for (size_t i = 0; i < sizeof(buttonMap) / sizeof(buttonMap[0]); i++) {
		if ((buttons ^ oldButtons) & buttonMap[i].hid) {
			SDL_SendMouseButton(window, NX_NMOUSE_MOUSEID,
				(buttons & buttonMap[i].hid) ? SDL_PRESSED : SDL_RELEASED,
				buttonMap[i].sdl);
		}
	}
}

int nxHooksNmouseInit(int width, int height)
{
	Result rc = hidInitializeNmouse(NULL, 0, width, height, HidNmousePointerType_Default);
	if (R_FAILED(rc)) {
		fprintf(stderr, "[nxhooks] hidInitializeNmouse failed with rc: %x\n", rc);
		return 0;
	}

	/* The merged pointer follows Joy-Con left/right pointers and a standard
	 * USB mouse, so one read covers every Switch 2 pointer source. */
	rc = hidSetMergedNmouseSourceTypes(HidNmouseType_JoyLeft |
	                                   HidNmouseType_JoyRight |
	                                   HidNmouseType_StandardMouse);
	if (R_FAILED(rc)) {
		fprintf(stderr, "[nxhooks] hidSetMergedNmouseSourceTypes failed with rc: %x\n", rc);
	}

	g_nmouse_initialized = true;
	return 1;
}

int nxHooksNmouseActive(void)
{
	return g_nmouse_initialized && g_has_handle;
}

void nxHooksNmousePoll(void)
{
	if (!g_nmouse_initialized) {
		return;
	}

	/* Devices can connect after startup: refresh the merged handle each poll. */
	HidNmouseHandle handle;
	if (R_SUCCEEDED(hidGetMergedNmouseHandle(&handle))) {
		g_merged_handle = handle;
		g_has_handle = true;
	} else {
		/* No pointer device (yet): leave the stick/touch cursor in charge. */
		g_has_handle = false;
		g_have_sample = false;
		return;
	}

	HidNmouseStateWithDelta states[NX_NMOUSE_MAX_SAMPLES];
	size_t total = 0;
	if (R_FAILED(hidGetNmouseStatesWithDelta(g_merged_handle, states,
	                                        NX_NMOUSE_MAX_SAMPLES, &total)) ||
	    total == 0) {
		return;
	}

	/* Newest sample first. Skip samples we've already consumed, summing the
	 * wheel movement of the new ones. */
	int wheel = 0;
	for (size_t i = 0; i < total; i++) {
		if (g_have_sample && states[i].sampling_number <= g_last_sample) {
			break;
		}
		wheel += states[i].wheel_delta_y;
	}
	if (g_have_sample && states[0].sampling_number <= g_last_sample) {
		return;
	}

	g_last_sample = states[0].sampling_number;
	g_have_sample = true;

	SDL_Window* window = SDL_GetFocusWindow();
	if (window == NULL) {
		return;
	}

	/* nmouse coordinates are already in the pointer area we registered
	 * (1920x1080), so send them as absolute window positions. */
	SDL_SendMouseMotion(window, NX_NMOUSE_MOUSEID, 0, states[0].x, states[0].y);
	_sendButtonChanges(window, states[0].buttons, g_last_buttons);
	g_last_buttons = states[0].buttons;

	if (wheel != 0) {
		SDL_SendMouseWheel(window, NX_NMOUSE_MOUSEID, 0, wheel, SDL_MOUSEWHEEL_NORMAL);
	}
}

void nxHooksNmouseExit(void)
{
	if (g_nmouse_initialized) {
		hidFinalizeNmouse();
		g_nmouse_initialized = false;
		g_has_handle = false;
		g_have_sample = false;
	}
}

#else /* !__SWITCH2__ */

int  nxHooksNmouseInit(int width, int height) { (void)width; (void)height; return 0; }
void nxHooksNmouseExit(void) {}
void nxHooksNmousePoll(void) {}
int  nxHooksNmouseActive(void) { return 0; }

#endif
