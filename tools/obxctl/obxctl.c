/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   obxctl.c for Obxctl, a tool for the Openbox window manager
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <glib.h>
#include <X11/Xlib.h>

Display *d;

gint fail(const gchar *s) {
    if (s)
        fprintf(stderr, "%s\n", s);
    else
        fprintf
            (stderr,
             "Usage: obxctl [OPTIONS] [--] [FILE]\n\n"
             "Options:\n"
             "    --help              Display this help and exit\n"
             "    --display DISPLAY   Connect to this X display\n");
    return 1;
}

void empty_pipe(const char *pipepath)
{
    FILE *pipe;
    int c, pfd;
    pfd = open(pipepath, (O_RDONLY | O_NONBLOCK));
    pipe = fdopen(pfd, "r");
    while ((c = getc(pipe) != EOF));
    fclose(pipe);
}

void send_xevent(void)
{
    Window root;
    root = DefaultRootWindow(d);

    XEvent ce;
    ce.xclient.type = ClientMessage;
    ce.xclient.message_type = XInternAtom(d, "_OB_CONTROL", FALSE);
    ce.xclient.display = d;
    ce.xclient.window = root;
    ce.xclient.format = 32;
    ce.xclient.data.l[0] = 4;
    ce.xclient.data.l[1] = 0;
    ce.xclient.data.l[2] = 0;
    ce.xclient.data.l[3] = 0;
    ce.xclient.data.l[4] = 0;
    XSendEvent(d, root, FALSE, (SubstructureNotifyMask | SubstructureRedirectMask), &ce);
    XSync(d, FALSE);
}

void send_data(FILE *pipe, FILE *data)
{
    int c;
    gboolean event_sent = FALSE;

    while ((c = getc(data)) != EOF) {
        /* write error, because pipe buffer is full, so send XEvent so
           openbox starts reading from pipe */
        if (fputc(c, pipe) == EOF) {
            if (!event_sent) {
                send_xevent();
                event_sent = TRUE;
                /* try sending the next character every 10 ms for 1 second */
                for (int i = 0; (fputc(c, pipe) == EOF) && (i < 100); ++i)
                    g_usleep(10000);
            } else
                break;
        }
    }
    if (!event_sent)
        send_xevent();

    return;
}

int main(int argc, char **argv)
{
    FILE *pipe;
    FILE *data;
    data = stdin;
    int i;
    char *dname = NULL;

    for (i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--help")) {
            return fail(NULL);
        } else if (!strcmp(argv[i], "--display")) {
            if (++i == argc)
                return fail(NULL);
            dname = argv[i];
        } else if (!strcmp(argv[i], "-")) {
            break;
        } else if (*argv[i] != '-') {
            data = NULL;
            break;
        } else if (!strcmp(argv[i], "--")) {
            data = NULL;
            i++;
            break;
        } else {
            break;
        }
    }

    if (!data && !((data = fopen(argv[i], "r"))))
        return fail ("Unable to open XML file for reading");

    d = XOpenDisplay(dname);
    if (!d) {
        return fail("Unable to find an X display. "
                    "Ensure you have permission to connect to the display.");
    }
    dname = XDisplayName(dname);

    const gchar *cache_home = g_get_user_cache_dir();

    gchar *pipepath;
    pipepath = g_build_filename(cache_home, "openbox", dname, NULL);

    empty_pipe(pipepath);
    if (!(pipe = fopen(pipepath, "w")))
        return fail ("Can't open pipe for writing");

    g_free(pipepath);

    send_data(pipe, data);
    fclose(pipe);
    fclose(data);
    return 0;
}
