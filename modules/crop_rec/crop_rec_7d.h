/* EOS 7D 2.0.6 native RAW crop lifecycle. Included by crop_rec.c.
 * Canon configures both DIGICs through PROP_LV_DISPSIZE; no sensor hooks.
 * GPL, same terms as crop_rec.c. */

static int crop_7d_saved_zoom;
static int crop_7d_blocked;
static int crop_7d_request_pending;
static int crop_7d_last_selection = -1;
static int crop_7d_last_lv;
static int crop_7d_busy;

static int crop_7d_begin_transition(void)
{
    int irq = cli();
    int acquired = !crop_7d_busy;
    if (acquired) crop_7d_busy = 1;
    sei(irq);
    return acquired;
}

static int crop_7d_wanted(void)
{
    return lv && is_movie_mode() && !video_mode_crop &&
        CROP_PRESET_MENU == CROP_PRESET_CANON_1X1;
}

static int crop_7d_active(void)
{
    /* Selecting OFF during recording only queues restoration. The sensor
     * remains in 1:1 until the zoom really changes. */
    return lv && is_movie_mode() && !video_mode_crop &&
        (crop_7d_wanted() || crop_7d_saved_zoom) &&
        (lv_dispsize == 5 || lv_dispsize == 10);
}

static void crop_7d_sync(void)
{
    crop_preset = crop_7d_active() ? CROP_PRESET_CANON_1X1 : CROP_PRESET_OFF;
}

static int crop_7d_idle(void)
{
    return !RECORDING && !gui_menu_shown() && job_state_ready_to_take_pic();
}

static int crop_7d_request_zoom(int zoom)
{
    /* Called only by ShootTask/deinit, never by a property handler. */
    if (!lv || !crop_7d_idle()) return 0;
    crop_7d_request_pending = 1;
    int ack = prop_request_change_wait(PROP_LV_DISPSIZE, &zoom, sizeof(zoom), 1000);
    if (!ack || lv_dispsize != zoom)
    {
        crop_7d_blocked = 1;
        NotifyBox(3000, "Crop zoom request failed; toggle OFF/ON to retry");
        return 0;
    }
    crop_7d_request_pending = 0;
    return 1;
}

static int crop_7d_restore(void)
{
    if (!crop_7d_saved_zoom) return 1;
    if (!crop_7d_request_pending && (lv_dispsize == crop_7d_saved_zoom ||
        (lv_dispsize != 5 && lv_dispsize != 10)))
    {
        crop_7d_saved_zoom = 0;
        return 1;
    }
    if (!lv || RECORDING || !job_state_ready_to_take_pic()) return 0;
    if (!crop_7d_request_zoom(crop_7d_saved_zoom)) return 0;
    crop_7d_saved_zoom = 0;
    return 1;
}

static void crop_7d_update(void)
{
    if (!crop_presets_count) return;
    int selection = CROP_PRESET_MENU;
    if (selection != crop_7d_last_selection || lv != crop_7d_last_lv)
        crop_7d_blocked = 0;
    crop_7d_last_selection = selection;
    crop_7d_last_lv = lv;
    crop_7d_sync();

    if (!crop_7d_idle()) return;
    if (crop_7d_blocked) return;
    if (!lv) return;
    if (!crop_7d_wanted())
    {
        crop_7d_restore();
        return;
    }
    if (crop_7d_active()) return;
    /* Save only zoom changed by this module. If the user was already in
     * x5/x10, disabling the preset must leave their zoom alone. */
    if (lv_dispsize != 1) return;
    if (!crop_7d_saved_zoom) crop_7d_saved_zoom = lv_dispsize;
    crop_7d_request_zoom(5);
    crop_7d_sync();
}

static void crop_7d_poll(void)
{
    if (!crop_7d_begin_transition()) return;
    crop_7d_update();
    crop_7d_busy = 0;
}

static int crop_7d_deinit(void)
{
    /* Keep restoration state if a recording or a failed request prevents it.
     * The module loader currently retains code/data after deinit. */
    if (!crop_7d_begin_transition()) return 1;
    int restored = crop_7d_restore();
    if (restored) crop_preset = CROP_PRESET_OFF;
    crop_7d_busy = 0;
    return !restored;
}
