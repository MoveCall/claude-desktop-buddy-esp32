#pragma once

#include <lvgl.h>

void buddy_touch_init(lv_display_t* display);
void buddy_touch_update(bool has_prompt, bool approval_sent);
