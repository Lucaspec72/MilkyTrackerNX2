#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Switch 2: read HID touch (1920x1080 output range) and emit SDL mouse
 * events. No-op in non-Switch 2 builds. */
void nxHooksTouchPoll(void);

#ifdef __cplusplus
}
#endif
