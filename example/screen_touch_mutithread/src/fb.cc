#include "fb.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static int fb_fd;
static struct fb_var_screeninfo vinfo;
static struct fb_fix_screeninfo finfo;
static char *fbp;
static int screensize;

void fb_init(const char *fbdev)
{
    fb_fd = open(fbdev, O_RDWR);
    if (fb_fd < 0)
    {
        perror("open fb");
        exit(1);
    }

    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
    ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);
    screensize = vinfo.yres_virtual * finfo.line_length;
    fbp = static_cast<char *>(mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0));
    if ((int)fbp == -1)
    {
        perror("mmap");
        exit(1);
    }
}

void fb_close()
{
    munmap(fbp, screensize);
    close(fb_fd);
}

void fb_clear(unsigned int color)
{
    for (int y = 0; y < vinfo.yres; y++)
        for (int x = 0; x < vinfo.xres; x++)
            fb_draw_rect(x, y, 1, 1, color);
}

void fb_draw_rect(int x, int y, int w, int h, unsigned int color)
{
    for (int j = y; j < y + h; j++)
    {
        for (int i = x; i < x + w; i++)
        {
            if (i < 0 || j < 0 || i >= vinfo.xres || j >= vinfo.yres)
                continue;
            long loc = j * finfo.line_length + i * (vinfo.bits_per_pixel / 8);
            *((unsigned int *)(fbp + loc)) = color;
        }
    }
}

int fb_width() { return vinfo.xres; }
int fb_height() { return vinfo.yres; }
