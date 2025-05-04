#include <stdint.h>
#include <stdio.h>
#include "drm.h"

int slider_value = 0;  // 新增：当前进度值

void draw_rect(void *fb, int x, int y, int w, int h, uint32_t color) {
    uint32_t *p = (uint32_t *)fb;
    for (int j = y; j < y + h; ++j) {
        for (int i = x; i < x + w; ++i) {
            if (i >= 0 && i < drm_width && j >= 0 && j < drm_height)
                p[j * drm_width + i] = color;
        }
    }
}


void slider_draw(void *fb) {
    int bar_x = drm_width / 8;
    int bar_y = drm_height / 2;
    int bar_w = drm_width * 3 / 4;
    int bar_h = 20;

    int filled_w = bar_w * slider_value / 100;

    draw_rect(fb, bar_x, bar_y, bar_w, bar_h, 0x888888);
    draw_rect(fb, bar_x, bar_y, filled_w, bar_h, 0x00FF00);
}

void slider_update(int touch_x) {
    int bar_x = drm_width / 8;
    int bar_w = drm_width * 3 / 4;

    if (touch_x < bar_x) touch_x = bar_x;
    if (touch_x > bar_x + bar_w) touch_x = bar_x + bar_w;

    slider_value = (touch_x - bar_x) * 100 / bar_w;
}
