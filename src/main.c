#include <stdio.h>
#include <unistd.h>
#include "lvgl.h"

/* LVGL v9 SDL driver headers */
#include "src/drivers/sdl/lv_sdl_window.h"
#include "src/drivers/sdl/lv_sdl_mouse.h"
#include "src/drivers/sdl/lv_sdl_mousewheel.h"
#include "src/drivers/sdl/lv_sdl_keyboard.h"

static void log_cb(lv_log_level_t level, const char * buf)
{
    (void)level;
    fputs(buf, stderr);
    fputc('\n', stderr);
}

int main(void)
{
    lv_init();

#if LV_USE_LOG
    lv_log_register_print_cb(log_cb);
#endif

    /* Create SDL window + input devices */
    lv_display_t * disp = lv_sdl_window_create(480, 272);
    (void)disp;
    lv_sdl_mouse_create();
    lv_sdl_mousewheel_create();
    lv_sdl_keyboard_create();

    /* Simple UI sanity check */
    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "LVGL sim is running");
    lv_obj_center(label);

    while(1) {
        lv_timer_handler();
        lv_tick_inc(5);
        usleep(5000);
    }

    return 0;
}