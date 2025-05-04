#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <string.h>

#define FB_DEV "/dev/fb0"
#define INPUT_DEV "/dev/input/event0"

struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
char *fbp = NULL;
int fb_fd = -1;
int screen_size = 0;

// 按钮定义
#define BTN_X 100
#define BTN_Y 100
#define BTN_W 200
#define BTN_H 100

// 颜色定义
#define COLOR_BTN  0x00A0A0  // 浅蓝
#define COLOR_BG   0x000000  // 黑
#define COLOR_DOWN 0xFF0000  // 红色
#define COLOR_TEXT 0xFFFFFF  // 白

void put_pixel(int x, int y, unsigned int color) {
    if (x < 0 || y < 0 || x >= vinfo.xres || y >= vinfo.yres)
        return;
    long location = (x + vinfo.xoffset) * (vinfo.bits_per_pixel / 8)
                    + (y + vinfo.yoffset) * finfo.line_length;
    *((unsigned int*)(fbp + location)) = color;
}

void draw_rect(int x, int y, int w, int h, unsigned int color) {
    for (int j = y; j < y + h; j++)
        for (int i = x; i < x + w; i++)
            put_pixel(i, j, color);
}

void init_fb() {
    fb_fd = open(FB_DEV, O_RDWR);
    if (fb_fd < 0) {
        perror("open fb");
        exit(1);
    }

    ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);
    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
    screen_size = vinfo.yres_virtual * finfo.line_length;

    fbp = (char *)mmap(0, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if ((int)fbp == -1) {
        perror("mmap");
        exit(1);
    }

    memset(fbp, 0, screen_size); // 清屏
}

void close_fb() {
    munmap(fbp, screen_size);
    close(fb_fd);
}

int in_button(int x, int y) {
    return x >= BTN_X && x <= (BTN_X + BTN_W) && y >= BTN_Y && y <= (BTN_Y + BTN_H);
}

int main() {
    struct input_event ev;
    int x = -1, y = -1;
    int touching = 0;

    int input_fd = open(INPUT_DEV, O_RDONLY);
    if (input_fd < 0) {
        perror("open input");
        return 1;
    }

    init_fb();

    // 初始界面：绘制按钮
    draw_rect(BTN_X, BTN_Y, BTN_W, BTN_H, COLOR_BTN);

    while (1) {
        if (read(input_fd, &ev, sizeof(ev)) == sizeof(ev)) {
            if (ev.type == EV_ABS) {
                if (ev.code == ABS_MT_POSITION_X || ev.code == ABS_X)
                    x = ev.value;
                else if (ev.code == ABS_MT_POSITION_Y || ev.code == ABS_Y)
                    y = ev.value;
            } else if (ev.type == EV_KEY && ev.code == BTN_TOUCH) {
                touching = ev.value;
                if (!touching) {
                    // 抬起时还原按钮颜色
                    draw_rect(BTN_X, BTN_Y, BTN_W, BTN_H, COLOR_BTN);
                }
            } else if (ev.type == EV_SYN && touching && x >= 0 && y >= 0) {
                if (in_button(x, y)) {
                    // 绘制按下效果
                    draw_rect(BTN_X, BTN_Y, BTN_W, BTN_H, COLOR_DOWN);
                    printf("按钮被点击!\n");
                    fflush(stdout);
                    usleep(200000); // 简单防抖+视觉反馈
                }
            }
        }
    }

    close_fb();
    close(input_fd);
    return 0;
}
