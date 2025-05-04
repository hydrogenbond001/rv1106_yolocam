#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

int touch_fd;

int touch_setup() {
    touch_fd = open("/dev/input/event0", O_RDONLY);
    if (touch_fd < 0) {
        perror("Failed to open touch device");
        return -1;
    }
    return 0;
}

int touch_poll(int *x, int *y) {
    struct input_event ev;
    while (read(touch_fd, &ev, sizeof(ev)) > 0) {
        if (ev.type == EV_ABS) {
            if (ev.code == ABS_X) {
                *x = ev.value;
            } else if (ev.code == ABS_Y) {
                *y = ev.value;
            }
        } else if (ev.type == EV_SYN) {
            return 1;  // Touch event complete
        }
    }
    return 0;
}

void touch_cleanup() {
    close(touch_fd);
}
