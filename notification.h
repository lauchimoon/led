#ifndef LED_NOTIFICATION
#define LED_NOTIFICATION

#include "state.h"

#define NOTIFICATION_DURATION 3.0

void notify(LedState *state, const char *msg);
void draw_notification(LedState *state);

#endif // LED_NOTIFICATION
