/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   actions/tile.c for the Openbox window manager
   Copyright (c) 2020        mzc

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

#include "openbox/actions.h"
#include "openbox/misc.h"
#include "openbox/client.h"

typedef struct {
    ObDirection dir;
} Options;

static gpointer setup_func(xmlNodePtr node);
static void free_func(gpointer o);
static gboolean run_func_on(ObActionsData *data, gpointer options);
static gboolean run_func_off(ObActionsData *data, gpointer options);

void action_tile_startup(void)
{
    actions_register("Tile", setup_func, free_func, run_func_on);
    actions_register("Untile", setup_func, free_func, run_func_off);
}

static gpointer setup_func(xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_slice_new0(Options);
    o->dir = OB_DIRECTION_NORTHWEST;

    if ((n = obt_xml_find_node(node, "direction"))) {
        gchar *s = obt_xml_node_string(n);
        if (!g_ascii_strcasecmp(s, "north") ||
            !g_ascii_strcasecmp(s, "n"))
            o->dir = OB_DIRECTION_NORTH;
        else if (!g_ascii_strcasecmp(s, "northeast") ||
            !g_ascii_strcasecmp(s, "ne"))
            o->dir = OB_DIRECTION_NORTHEAST;
        else if (!g_ascii_strcasecmp(s, "east") ||
            !g_ascii_strcasecmp(s, "e"))
            o->dir = OB_DIRECTION_EAST;
        else if (!g_ascii_strcasecmp(s, "southeast") ||
            !g_ascii_strcasecmp(s, "se"))
            o->dir = OB_DIRECTION_SOUTHEAST;
        else if (!g_ascii_strcasecmp(s, "south") ||
            !g_ascii_strcasecmp(s, "s"))
            o->dir = OB_DIRECTION_SOUTH;
        else if (!g_ascii_strcasecmp(s, "southwest") ||
            !g_ascii_strcasecmp(s, "sw"))
            o->dir = OB_DIRECTION_SOUTHWEST;
        else if (!g_ascii_strcasecmp(s, "west") ||
            !g_ascii_strcasecmp(s, "w"))
            o->dir = OB_DIRECTION_WEST;
        else if (!g_ascii_strcasecmp(s, "northwest") ||
            !g_ascii_strcasecmp(s, "nw"))
            o->dir = OB_DIRECTION_NORTHWEST;
        g_free(s);
    }

    return o;
}

static void free_func(gpointer o)
{
    g_slice_free(Options, o);
}

/* Always return FALSE because its not interactive */
static gboolean run_func_on(ObActionsData *data, gpointer options)
{
    Options *o = options;
    if (data->client) {
        actions_client_move(data, TRUE);
        client_tile(data->client, TRUE, o->dir);
        actions_client_move(data, FALSE);
    }
    return FALSE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func_off(ObActionsData *data, gpointer options)
{
    Options *o = options;
    if (data->client) {
        actions_client_move(data, TRUE);
        client_tile(data->client, FALSE, o->dir);
        actions_client_move(data, FALSE);
    }
    return FALSE;
}
