/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   indicator_frame.c for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
   Copyright (c) 2003-2007   Dana Jansens

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

#include "geom.h"
#include "indicator_frame.h"
#include "window.h"
#include "openbox.h"
#include "frame.h"
#include "event.h"
#include "obrender/render.h"

#include <X11/Xlib.h>
#include <glib.h>

#define INDICATOR_FRAME_WIDTH 6

static struct
{
    ObInternalWindow top;
    ObInternalWindow left;
    ObInternalWindow right;
    ObInternalWindow bottom;
} indicator_frame;

static RrAppearance *a_indicator_frame;
static RrColor      *color_white;
static gboolean      visible;

static Window create_window(Window parent, gulong mask,
                            XSetWindowAttributes *attrib)
{
    return XCreateWindow(obt_display, parent, 0, 0, 1, 1, 0,
                         RrDepth(ob_rr_inst), InputOutput,
                         RrVisual(ob_rr_inst), mask, attrib);

}

void indicator_frame_startup(gboolean reconfig)
{
    XSetWindowAttributes attr;

    visible = FALSE;

    if (reconfig) return;

    indicator_frame.top.type = OB_WINDOW_CLASS_INTERNAL;
    indicator_frame.left.type = OB_WINDOW_CLASS_INTERNAL;
    indicator_frame.right.type = OB_WINDOW_CLASS_INTERNAL;
    indicator_frame.bottom.type = OB_WINDOW_CLASS_INTERNAL;

    attr.override_redirect = True;
    attr.background_pixel = BlackPixel(obt_display, ob_screen);
    indicator_frame.top.window =
        create_window(obt_root(ob_screen),
                      CWOverrideRedirect | CWBackPixel, &attr);
    indicator_frame.left.window =
        create_window(obt_root(ob_screen),
                      CWOverrideRedirect | CWBackPixel, &attr);
    indicator_frame.right.window =
        create_window(obt_root(ob_screen),
                      CWOverrideRedirect | CWBackPixel, &attr);
    indicator_frame.bottom.window =
        create_window(obt_root(ob_screen),
                      CWOverrideRedirect | CWBackPixel, &attr);

    stacking_add(INTERNAL_AS_WINDOW(&indicator_frame.top));
    stacking_add(INTERNAL_AS_WINDOW(&indicator_frame.left));
    stacking_add(INTERNAL_AS_WINDOW(&indicator_frame.right));
    stacking_add(INTERNAL_AS_WINDOW(&indicator_frame.bottom));
    window_add(&indicator_frame.top.window,
               INTERNAL_AS_WINDOW(&indicator_frame.top));
    window_add(&indicator_frame.left.window,
               INTERNAL_AS_WINDOW(&indicator_frame.left));
    window_add(&indicator_frame.right.window,
               INTERNAL_AS_WINDOW(&indicator_frame.right));
    window_add(&indicator_frame.bottom.window,
               INTERNAL_AS_WINDOW(&indicator_frame.bottom));

    color_white = RrColorNew(ob_rr_inst, 0xff, 0xff, 0xff);

    a_indicator_frame = RrAppearanceNew(ob_rr_inst, 4);
    a_indicator_frame->surface.grad = RR_SURFACE_SOLID;
    a_indicator_frame->surface.relief = RR_RELIEF_FLAT;
    a_indicator_frame->surface.primary = RrColorNew(ob_rr_inst,
                                                    0, 0, 0);
    a_indicator_frame->texture[0].type = RR_TEXTURE_LINE_ART;
    a_indicator_frame->texture[0].data.lineart.color = color_white;
    a_indicator_frame->texture[1].type = RR_TEXTURE_LINE_ART;
    a_indicator_frame->texture[1].data.lineart.color = color_white;
    a_indicator_frame->texture[2].type = RR_TEXTURE_LINE_ART;
    a_indicator_frame->texture[2].data.lineart.color = color_white;
    a_indicator_frame->texture[3].type = RR_TEXTURE_LINE_ART;
    a_indicator_frame->texture[3].data.lineart.color = color_white;
}

void indicator_frame_shutdown(gboolean reconfig)
{
    if (reconfig) return;

    RrColorFree(color_white);

    RrAppearanceFree(a_indicator_frame);

    window_remove(indicator_frame.top.window);
    window_remove(indicator_frame.left.window);
    window_remove(indicator_frame.right.window);
    window_remove(indicator_frame.bottom.window);

    stacking_remove(INTERNAL_AS_WINDOW(&indicator_frame.top));
    stacking_remove(INTERNAL_AS_WINDOW(&indicator_frame.left));
    stacking_remove(INTERNAL_AS_WINDOW(&indicator_frame.right));
    stacking_remove(INTERNAL_AS_WINDOW(&indicator_frame.bottom));

    XDestroyWindow(obt_display, indicator_frame.top.window);
    XDestroyWindow(obt_display, indicator_frame.left.window);
    XDestroyWindow(obt_display, indicator_frame.right.window);
    XDestroyWindow(obt_display, indicator_frame.bottom.window);
}

void indicator_frame_update(Rect *area)
{
        if (visible)
            indicator_frame_draw(area);
}

void indicator_frame_draw(Rect *area)
{
    if (!area && visible) {
        gulong ignore_start;

        /* kill enter events cause by this unmapping */
        ignore_start = event_start_ignore_all_enters();

        XUnmapWindow(obt_display, indicator_frame.top.window);
        XUnmapWindow(obt_display, indicator_frame.left.window);
        XUnmapWindow(obt_display, indicator_frame.right.window);
        XUnmapWindow(obt_display, indicator_frame.bottom.window);

        event_end_ignore_all_enters(ignore_start);

        visible = FALSE;
    }
    else if (area) {
        gint x, y, w, h;
        gint wt, wl, wr, wb;
        gulong ignore_start;

        wt = wl = wr = wb = INDICATOR_FRAME_WIDTH;

        x = area->x;
        y = area->y;
        w = area->width;
        h = wt;

        /* kill enter events cause by this moving */
        ignore_start = event_start_ignore_all_enters();

        XMoveResizeWindow(obt_display, indicator_frame.top.window,
                          x, y, w, h);
        a_indicator_frame->texture[0].data.lineart.x1 = 0;
        a_indicator_frame->texture[0].data.lineart.y1 = h-1;
        a_indicator_frame->texture[0].data.lineart.x2 = 0;
        a_indicator_frame->texture[0].data.lineart.y2 = 0;
        a_indicator_frame->texture[1].data.lineart.x1 = 0;
        a_indicator_frame->texture[1].data.lineart.y1 = 0;
        a_indicator_frame->texture[1].data.lineart.x2 = w-1;
        a_indicator_frame->texture[1].data.lineart.y2 = 0;
        a_indicator_frame->texture[2].data.lineart.x1 = w-1;
        a_indicator_frame->texture[2].data.lineart.y1 = 0;
        a_indicator_frame->texture[2].data.lineart.x2 = w-1;
        a_indicator_frame->texture[2].data.lineart.y2 = h-1;
        a_indicator_frame->texture[3].data.lineart.x1 = (wl-1);
        a_indicator_frame->texture[3].data.lineart.y1 = h-1;
        a_indicator_frame->texture[3].data.lineart.x2 = w - wr;
        a_indicator_frame->texture[3].data.lineart.y2 = h-1;
        RrPaint(a_indicator_frame, indicator_frame.top.window,
                w, h);

        x = area->x;
        y = area->y;
        w = wl;
        h = area->height;

        XMoveResizeWindow(obt_display, indicator_frame.left.window,
                          x, y, w, h);
        a_indicator_frame->texture[0].data.lineart.x1 = w-1;
        a_indicator_frame->texture[0].data.lineart.y1 = 0;
        a_indicator_frame->texture[0].data.lineart.x2 = 0;
        a_indicator_frame->texture[0].data.lineart.y2 = 0;
        a_indicator_frame->texture[1].data.lineart.x1 = 0;
        a_indicator_frame->texture[1].data.lineart.y1 = 0;
        a_indicator_frame->texture[1].data.lineart.x2 = 0;
        a_indicator_frame->texture[1].data.lineart.y2 = h-1;
        a_indicator_frame->texture[2].data.lineart.x1 = 0;
        a_indicator_frame->texture[2].data.lineart.y1 = h-1;
        a_indicator_frame->texture[2].data.lineart.x2 = w-1;
        a_indicator_frame->texture[2].data.lineart.y2 = h-1;
        a_indicator_frame->texture[3].data.lineart.x1 = w-1;
        a_indicator_frame->texture[3].data.lineart.y1 = wt-1;
        a_indicator_frame->texture[3].data.lineart.x2 = w-1;
        a_indicator_frame->texture[3].data.lineart.y2 = h - wb;
        RrPaint(a_indicator_frame, indicator_frame.left.window,
                w, h);

        x = area->x + area->width - wr;
        y = area->y;
        w = wr;
        h = area->height ;

        XMoveResizeWindow(obt_display, indicator_frame.right.window,
                          x, y, w, h);
        a_indicator_frame->texture[0].data.lineart.x1 = 0;
        a_indicator_frame->texture[0].data.lineart.y1 = 0;
        a_indicator_frame->texture[0].data.lineart.x2 = w-1;
        a_indicator_frame->texture[0].data.lineart.y2 = 0;
        a_indicator_frame->texture[1].data.lineart.x1 = w-1;
        a_indicator_frame->texture[1].data.lineart.y1 = 0;
        a_indicator_frame->texture[1].data.lineart.x2 = w-1;
        a_indicator_frame->texture[1].data.lineart.y2 = h-1;
        a_indicator_frame->texture[2].data.lineart.x1 = w-1;
        a_indicator_frame->texture[2].data.lineart.y1 = h-1;
        a_indicator_frame->texture[2].data.lineart.x2 = 0;
        a_indicator_frame->texture[2].data.lineart.y2 = h-1;
        a_indicator_frame->texture[3].data.lineart.x1 = 0;
        a_indicator_frame->texture[3].data.lineart.y1 = wt-1;
        a_indicator_frame->texture[3].data.lineart.x2 = 0;
        a_indicator_frame->texture[3].data.lineart.y2 = h - wb;
        RrPaint(a_indicator_frame, indicator_frame.right.window,
                w, h);

        x = area->x;
        y = area->y + area->height - wb;
        w = area->width;
        h = wb;

        XMoveResizeWindow(obt_display, indicator_frame.bottom.window,
                          x, y, w, h);
        a_indicator_frame->texture[0].data.lineart.x1 = 0;
        a_indicator_frame->texture[0].data.lineart.y1 = 0;
        a_indicator_frame->texture[0].data.lineart.x2 = 0;
        a_indicator_frame->texture[0].data.lineart.y2 = h-1;
        a_indicator_frame->texture[1].data.lineart.x1 = 0;
        a_indicator_frame->texture[1].data.lineart.y1 = h-1;
        a_indicator_frame->texture[1].data.lineart.x2 = w-1;
        a_indicator_frame->texture[1].data.lineart.y2 = h-1;
        a_indicator_frame->texture[2].data.lineart.x1 = w-1;
        a_indicator_frame->texture[2].data.lineart.y1 = h-1;
        a_indicator_frame->texture[2].data.lineart.x2 = w-1;
        a_indicator_frame->texture[2].data.lineart.y2 = 0;
        a_indicator_frame->texture[3].data.lineart.x1 = wl-1;
        a_indicator_frame->texture[3].data.lineart.y1 = 0;
        a_indicator_frame->texture[3].data.lineart.x2 = w - wr;
        a_indicator_frame->texture[3].data.lineart.y2 = 0;
        RrPaint(a_indicator_frame, indicator_frame.bottom.window,
                w, h);

        XMapWindow(obt_display, indicator_frame.top.window);
        XMapWindow(obt_display, indicator_frame.left.window);
        XMapWindow(obt_display, indicator_frame.right.window);
        XMapWindow(obt_display, indicator_frame.bottom.window);

        event_end_ignore_all_enters(ignore_start);

        visible = TRUE;
    }
}
