#include <nxhooks/touch.h>

#ifdef __SWITCH2__

#include <switch.h>
#include <SDL.h>

/* Separate from SDL_TOUCH_MOUSEID (SDL's own, mis-scaled touch->mouse path)
 * and from the stick and native-pointer IDs. */
#define NX_TOUCH_MOUSEID 3

static u64 g_last_sample = 0;
static bool g_touching = false;
static int g_last_x = 0;
static int g_last_y = 0;

SDL_Window* SDL_GetFocusWindow(void);
int SDL_SendMouseMotion(SDL_Window* window, int mouseID, int relative, int x, int y);
int SDL_SendMouseButton(SDL_Window* window, int mouseID, Uint8 state, Uint8 button);

void nxHooksTouchPoll(void)
{
	HidTouchScreenState state;
	size_t total = hidGetTouchScreenStates(&state, 1);
	if (total == 0 || state.sampling_number == g_last_sample) {
		return;
	}
	g_last_sample = state.sampling_number;

	SDL_Window* window = SDL_GetFocusWindow();
	if (window == NULL) {
		return;
	}

	if (state.count > 0) {
		/* hidSetTouchScreenOutputRanges() (called at startup) makes these
		 * coordinates span the full 1920x1080 panel. */
		g_last_x = (int)state.touches[0].x;
		g_last_y = (int)state.touches[0].y;
		SDL_SendMouseMotion(window, NX_TOUCH_MOUSEID, 0, g_last_x, g_last_y);

		if (!g_touching) {
			SDL_SendMouseButton(window, NX_TOUCH_MOUSEID, SDL_PRESSED, SDL_BUTTON_LEFT);
			g_touching = true;
		}
	} else if (g_touching) {
		SDL_SendMouseMotion(window, NX_TOUCH_MOUSEID, 0, g_last_x, g_last_y);
		SDL_SendMouseButton(window, NX_TOUCH_MOUSEID, SDL_RELEASED, SDL_BUTTON_LEFT);
		g_touching = false;
	}
}

#else /* !__SWITCH2__ */

void nxHooksTouchPoll(void) {}

#endif
