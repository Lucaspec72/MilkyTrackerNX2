#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Switch 2 native mouse (libounce nmouse) -> SDL mouse events.
 * These are no-ops / return 0 in non-Switch 2 builds. */
int  nxHooksNmouseInit(int width, int height);
void nxHooksNmouseExit(void);
void nxHooksNmousePoll(void);
int  nxHooksNmouseActive(void);

#ifdef __cplusplus
}
#endif
