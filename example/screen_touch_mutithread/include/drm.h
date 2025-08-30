#ifndef DRM_H
#define DRM_H

#include <stdint.h>

// 显示设备的宽度和高度（根据实际屏幕分辨率调整）
extern int drm_width;
extern int drm_height;
extern void *drm_fb_map;
extern uint32_t drm_fb_id;
extern int drm_fd;

// 函数声明
int drm_setup();                // 初始化 DRM 设备
void drm_cleanup();             // 清理 DRM 资源
void drm_draw_pixel(int x, int y, uint32_t color); // 在屏幕上绘制像素

// 滑块控制
void slider_draw(void *framebuffer);  // 绘制滑块
void slider_update(int x);            // 更新滑块位置

// 触摸输入处理
int touch_setup();                  // 初始化触摸设备
int touch_poll(int *x, int *y);     // 获取触摸坐标
void touch_cleanup();               // 清理触摸设备

#endif // DRM_H
