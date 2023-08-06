/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   indicator_frame.h for the Openbox window manager
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

#ifndef __indicator_frame_h
#define __indicator_frame_h

struct _Rect;

void indicator_frame_startup(gboolean reconfig);
void indicator_frame_shutdown(gboolean reconfig);

void indicator_frame_update(struct _Rect *area);
void indicator_frame_draw(struct _Rect *area);

#endif
