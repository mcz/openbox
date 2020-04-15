#ifndef ob__control_h
#define ob__control_h

#include <glib.h>

void control_startup(gboolean reconfigure);
void control_main(void);
void control_shutdown(gboolean reconfigure);

#endif
