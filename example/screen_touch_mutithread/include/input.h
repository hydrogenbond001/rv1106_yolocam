#ifndef INPUT_H
#define INPUT_H

void input_init(const char *dev);
void input_close();
int input_read(int *x, int *y, int *touching);

#endif
