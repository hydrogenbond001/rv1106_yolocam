#ifndef SLIDER_H
#define SLIDER_H

#include <stdint.h>

typedef struct {
    int x, y;        // 滑块的位置 (左上角)
    int w, h;        // 滑块的宽度和高度
    int min_value;   // 最小值
    int max_value;   // 最大值
    int current_value;  // 当前值
    int knob_width;  // 滑块的按钮宽度
} slider_t;

// 初始化滑块
void slider_init(slider_t* slider, int x, int y, int w, int h);

// 设置滑块的当前值
void slider_set_value(slider_t* slider, int value);

// 判断触摸点是否在滑块范围内
int slider_contains(slider_t* slider, int tx, int ty);

// 处理滑块的触摸事件，返回更新后的值
int slider_handle_touch(slider_t* slider, int tx);

// 绘制滑块到 framebuffer 中
void draw_slider(slider_t* slider, uint32_t* framebuffer, int fb_width, int fb_height);

#endif // SLIDER_H
