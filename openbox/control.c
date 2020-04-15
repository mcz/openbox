#include "control.h"
#include "actions.h"
#include "menuframe.h"
#include "frame.h"
#include "client.h"
#include "focus.h"
#include "obt/display.h"
#include "obt/paths.h"
#include "obt/xml.h"

#include <stdio.h>
#include <glib.h>
#include <libxml/xinclude.h>

#ifdef HAVE_UNISTD_H
#  include <sys/types.h>
#  include <unistd.h>
#endif
#ifdef HAVE_SYS_STAT_H
#  include <sys/stat.h>
#endif

static gchar *fifopath;
gboolean pipe_exists = FALSE;

void control_startup(gboolean reconfigure)
{
    if (reconfigure) return;

    ObtPaths *paths = obt_paths_new();
    fifopath = g_build_filename(obt_paths_cache_home(paths), "openbox",
                                DisplayString(obt_display), NULL);
    obt_paths_unref(paths);

    struct stat sb;
    stat(fifopath, &sb);
    if (!S_ISFIFO(sb.st_mode))
    {
        remove(fifopath);
        if (!mkfifo(fifopath, 0777)) {
            g_message("Unable to create FIFO. Obxctl won't work");
            pipe_exists = FALSE;
        } else
            pipe_exists = TRUE;
    } else
        pipe_exists = TRUE;

}

static void control_parse(xmlNodePtr node, GSList **actions)
{
    xmlNodePtr n;

    if ((n = obt_xml_find_node(node->children, "action"))) {
        while (n) {
            ObActionsAct *action;

            action = actions_parse(n);
            if (action)
                *actions = g_slist_append(*actions, action);
            n = obt_xml_find_node(n->next, "action");
        }
    }
}

static void control_run(GSList *actions, ObClient *client)
{
    if (menu_frame_visible)
        menu_frame_hide_all();
    actions_run_acts(actions, OB_USER_ACTION_OBXCTL, 0, -1, -1, 0,
                     OB_FRAME_CONTEXT_NONE, client);
}

void control_main(void)
{
    if (!pipe_exists) return;

    xmlDocPtr dptr = NULL;
    xmlNodePtr root = NULL;
    GSList *actions = NULL;
    int pipe;

    xmlResetLastError();

    dptr = xmlReadFile(fifopath, NULL, (XML_PARSE_NOBLANKS | XML_PARSE_RECOVER));
    xmlXIncludeProcessFlags(dptr, (XML_PARSE_NOBLANKS | XML_PARSE_RECOVER));

    if (dptr) {
        root = xmlDocGetRootElement(dptr);
        if (root && !(xmlStrcmp(root->name, "obxctl"))) {
            control_parse(root, &actions);
            control_run(actions, focus_client);
            g_slist_free(actions);
        }
        xmlFreeDoc(dptr);
    }
}
