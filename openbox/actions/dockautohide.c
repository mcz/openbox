#include "openbox/actions.h"
#include "openbox/dock.h"
#include "openbox/config.h"

static gboolean run_func(ObActionsData *data, gpointer options);

void action_dockautohide_startup(void)
{
    actions_register("ToggleDockAutoHide",
                     NULL, NULL,
                     run_func);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(G_GNUC_UNUSED ObActionsData *data, G_GNUC_UNUSED gpointer options)
{
    config_dock_hide = !config_dock_hide;
    dock_configure();

    return FALSE;
}
