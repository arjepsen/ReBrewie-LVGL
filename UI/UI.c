#include "UI.h"

void ui_init()
{
    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "LVGL sim is running");
    lv_obj_center(label);
}