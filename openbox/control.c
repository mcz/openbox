/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   control.c for the Openbox window manager
   Copyright (c) 2020        mcz

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   See the COPYING file for a copy of the GNU General Public License.
*/

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
#ifdef HAVE_FCNTL_H
#   include <fcntl.h>
#endif
#ifdef HAVE_SYS_STAT_H
#  include <sys/stat.h>
#endif

int pfd = -1;

void control_startup(gboolean reconfigure)
{
    if (reconfigure) return;

    gchar * fifopath;
    gboolean pipe_exists = FALSE;

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

    if (pipe_exists)
        pfd = open(fifopath, (O_RDONLY | O_NONBLOCK));

    g_free(fifopath);
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

static void control_run(GSList *actions)
{
    if (menu_frame_visible)
        menu_frame_hide_all();
    actions_run_acts(actions, OB_USER_ACTION_OBXCTL, 0, -1, -1, 0,
                     OB_FRAME_CONTEXT_NONE, focus_client);
}

void control_main(void)
{
    if (!pfd) return;

    xmlDocPtr dptr = NULL;
    xmlNodePtr root = NULL;
    GSList *actions = NULL;

    xmlResetLastError();

    dptr = xmlReadFd(pfd, "pipe", NULL, (XML_PARSE_NOBLANKS | XML_PARSE_RECOVER));
    xmlXIncludeProcessFlags(dptr, (XML_PARSE_NOBLANKS | XML_PARSE_RECOVER));

    if (dptr) {
        root = xmlDocGetRootElement(dptr);
        if (root && !(xmlStrcmp(root->name, "obxctl"))) {
            control_parse(root, &actions);
            control_run(actions);
            g_slist_free(actions);
        }
        xmlFreeDoc(dptr);
    }
}

void control_shutdown(gboolean reconfigure)
{
    if (reconfigure || !pfd) return;

    close(pfd);
}
