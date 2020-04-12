static gchar *fifopath;

void control_startup(gboolean reconfigure)
{
    if (reconfigure) return;

    ObtPaths *paths = obt_paths_new();
    fifopath = g_build_filename(obt_paths_cache_home(paths), obt_display, NULL);
    obt_paths_unref(paths);

    if(!(remove(fifopath) || !mkfifo(fifopath)))
            g_message("Unable to create FIFO. Obxctl won't work");
}

void control_shutdown(gboolean reconfigure)
{
    if (recofigure) return;
    remove(fifopath);
}
