#include "nxhooks/init.h"
#include "nxhooks/sdl/input.h"
#include "nxhooks/screen.h"
#include "nxhooks/nmouse.h"
#include <switch.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

static char g_initial_directory_name[PATH_MAX+1];

void nxHooksInit() {
	Result rc = socketInitializeDefault();
	int stdioRes = -1;

#ifdef __SWITCH2__
	// The Switch 2 panel is 1920x1080; libounce's HID otherwise reports raw
	// touch coordinates at the panel resolution. Set this before SDL (or any
	// other consumer) calls hidInitializeTouchScreen so normalized SDL touch
	// events map onto the 1920x1080 window MilkyTracker creates (NX_SCREEN_*).
	Result touchRc = hidSetTouchScreenOutputRanges(NX_SCREEN_WIDTH, NX_SCREEN_HEIGHT);
	if (R_FAILED(touchRc)) {
		printf("[nxhooks] hidSetTouchScreenOutputRanges failed with rc: %x\n", touchRc);
	}

	/* Native Switch 2 pointer (Joy-Con / USB mouse) -> SDL mouse events. */
	nxHooksNmouseInit(NX_SCREEN_WIDTH, NX_SCREEN_HEIGHT);
#endif

	if(R_SUCCEEDED(rc)) {
		stdioRes = nxlinkStdio();
	}

	if(R_FAILED(rc)) {
		printf("[nxhooks] socketInitialize failed with rc: %x\n", rc);
	}

	rc = appletInitializeGamePlayRecording();
	if(R_FAILED(rc)) {
		printf("[nxhooks] appletInitializeGamePlayRecording failed with rc: %x\n", rc);
	}
	appletSetGamePlayRecordingState(1);

	memset(g_initial_directory_name, 0, sizeof(g_initial_directory_name));
	getcwd(g_initial_directory_name, sizeof(g_initial_directory_name));
	printf("[nxhooks] initial_directory_name: %s\n", g_initial_directory_name);

	nxHooksSDLInputInit();
}

void nxHooksExit() {
	nxHooksSDLInputExit();

#ifdef __SWITCH2__
	nxHooksNmouseExit();
#endif

	socketExit();
}

char* nxHooksGetInitialDirectoryName() {
	return g_initial_directory_name;
}