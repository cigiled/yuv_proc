#ifndef __FB_DEVICE_H__
#define __FB_DEVICE_H__

#include "../common/public.h"

int start_fb();
int show_image(char *rgbdata, image_inf_t  imag);
void drawRect_rgb16 (int x0, int y0, int width, int height, int color);
int destroy_fb();

#endif