#ifndef FB_H
#define FB_H

#include <linux/fb.h>

void fb_init(const char *fbdev);
void fb_close();
void fb_draw_rect(int x, int y, int w, int h, unsigned int color);
void fb_clear(unsigned int color);
int fb_width();
int fb_height();

#endif
