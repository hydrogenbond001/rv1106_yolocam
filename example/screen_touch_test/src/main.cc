#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "drm.h"
#include "touch.h"
#include "slider.h"
#include <string.h>

int main() {
    if (drm_setup() != 0) {
        fprintf(stderr, "DRM setup failed\n");
        return -1;
    }

    if (touch_setup() != 0) {
        fprintf(stderr, "Touch setup failed\n");
        drm_cleanup();
        return -1;
    }
    
    int x = 0, y = 0;
    while (1) {
        if (touch_poll(&x, &y)) {
            // 更新滑块位置
            slider_update(x);
            // 清空并重新绘制
            memset(drm_fb_map, 0, drm_width * drm_height * 4); // 清空屏幕
            slider_draw(drm_fb_map);  // 绘制滑块
        }
        usleep(10000);  // 每10ms更新一次
    }

    touch_cleanup();
    drm_cleanup();
    return 0;
}
