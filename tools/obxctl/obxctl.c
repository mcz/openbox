#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <glib.h>
#include <X11/Xlib.h>

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

void send_xevent(Display *d)
{
    XEvent ce;
    ce.xclient.type = ClientMessage;
    ce.xclient.message_type = XInternAtom(d, "_OB_CONTROL", FALSE);
    ce.xclient.display = d;
    ce.xclient.window = DefaultRootWindow(d);
    ce.xclient.format = 32;
    ce.xclient.data.l[0] = 4;
    ce.xclient.data.l[1] = 0;
    ce.xclient.data.l[2] = 0;
    ce.xclient.data.l[3] = 0;
    ce.xclient.data.l[4] = 0;
    XSendEvent(d, DefaultRootWindow(d), FALSE, SubstructureNotifyMask | SubstructureRedirectMask, &ce);
}

void send_data(FILE *pipe, FILE *data)
{
    int c;

    while ((c = getc(data)) != EOF) {
        fputc(c, pipe);
    }
    return;
}

int main(int argc, char **argv)
{
    Display *d;
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

    send_xevent(d);

    const gchar *cache_home = g_get_user_cache_dir();

    gchar *pipepath;
    pipepath = g_build_filename(cache_home, "openbox", dname, NULL);

    if (!(pipe = fopen(pipepath, "w")))
        return fail ("Can't open pipe for writing");

    g_free(pipepath);

    send_data(pipe, data);
    fclose(pipe);
    fclose(data);
    return 0;
}
