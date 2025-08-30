#include "input.h"
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static int input_fd;

void input_init(const char *dev) {
    input_fd = open(dev, O_RDONLY);
    if (input_fd < 0) {
        perror("open input");
        exit(1);
    }
}

void input_close() {
    close(input_fd);
}

// 返回 1 表示有更新，0 表示无
int input_read(int *x, int *y, int *touching) {
    struct input_event ev;
    static int tx = -1, ty = -1;
    static int t = 0;
    int updated = 0;

    while (read(input_fd, &ev, sizeof(ev)) == sizeof(ev)) {
        if (ev.type == EV_ABS) {
            if (ev.code == ABS_MT_POSITION_X || ev.code == ABS_X)
                tx = ev.value;
            else if (ev.code == ABS_MT_POSITION_Y || ev.code == ABS_Y)
                ty = ev.value;
        } else if (ev.type == EV_KEY && ev.code == BTN_TOUCH)
            t = ev.value;
        else if (ev.type == EV_SYN) {
            if (x && y && touching) {
                *x = tx;
                *y = ty;
                *touching = t;
                updated = 1;
            }
            break;
        }
    }

    return updated;
}
