#ifndef TOUCH_H
#define TOUCH_H

#include <stdint.h>
#include <stdbool.h>

// 触摸事件类型
typedef enum {
    TOUCH_EVENT_NONE,
    TOUCH_EVENT_PRESS,  // 按下
    TOUCH_EVENT_RELEASE,  // 松开
    TOUCH_EVENT_MOVE,  // 滑动
} touch_event_t;

// 触摸点结构体
typedef struct {
    int x;
    int y;
} touch_point_t;

// 初始化触摸屏
int touch_init(const char* device);

// 读取触摸事件
touch_event_t touch_read(touch_point_t* point);

// 关闭触摸屏
void touch_close();

// 假设返回触摸坐标的函数
int touch_poll(int* x, int* y);

// 获取滑动的值（例如：手指滑动的距离或比例）
int touch_get_slide_value(int start_x, int end_x);

#endif // TOUCH_H
