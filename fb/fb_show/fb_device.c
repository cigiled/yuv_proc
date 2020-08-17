#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h> 

#include <sys/ioctl.h>

#include "fb_device.h"

int fb_fd = 0;
char *mmpadd = NULL;
unsigned int spreen_size = 0;
int width  = 0;
int height = 0;
int x_off  = 0;
int y_off  = 0;


int start_fb( )
{
	int fbsize = 0;
	
	struct fb_var_screeninfo vinfo;
	memset(&vinfo, 0, sizeof(vinfo));

	fb_fd = open("/dev/fb0", O_RDWR);
	if(fb_fd<= 0 ){
		printf("[start_fb]: open fb device failed, error:%m.\n");
		return -1;
	}

	if(ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo)) {
		printf("Bad vscreeninfo ioctl/n");
		return -1;
	}

	fbsize = vinfo.xres*vinfo.yres*(vinfo.bits_per_pixel/8);
	printf("==121===[%d]={[%d]=[%d]}=[%d]==[%d]=\n", vinfo.bits_per_pixel/8, vinfo.xres, vinfo.yres, vinfo.xoffset, vinfo.yoffset);
	mmpadd = (char *)mmap(0, fbsize, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
	if(mmpadd == (char *)-1){
		printf("[start_fb]: mmap fb hal adder failed, error:%m.\n");
		return -1;	
	}

	spreen_size = fbsize;
	width  = vinfo.xres;
	height = vinfo.yres;
	x_off  = vinfo.xoffset;
	y_off  = vinfo.yoffset;
	
	return 0;
}

int show_image(char *rgbdata, image_inf_t imag)
{
	if( (rgbdata == NULL) || (imag.bfSize <= 0) ){
		printf("[show_image]: Input [rgbdata] is NULL, or [size] <= 0, exit!\n");
		return -1;
	}

	int size = 0;
	int x, y;
	int x0 = 50;
	int y0 = 20;
	 short *dest = NULL;
    const int stride = width *2;//TNT
	int i = 0;
	int line =  imag.bwidth * 2;
	
	dest = (short *) (mmpadd)  + (y0 + y_off) * stride + (x0 + x_off);
	 
    for (y = 0; y < imag.bheight; y++)
    {
    	if(line > imag.bfSize)
			break;
		
		memcpy(dest, rgbdata + line *y, line);
        dest += stride;
		size += line;
		
    }	
	
	return 0;
}

int destroy_fb()
{
	if(mmpadd != NULL){
		munmap(mmpadd, spreen_size);
		close(fb_fd);
		mmpadd = NULL;
	}

	return 0;
}

#if 0
int fb_pixel(void *fbmem, int width, int height,
             int x, int y, unsigned int color)
{
    if ((x > width) || (y > height))
        return (-1);

    unsigned int *dst = ((unsigned int *) fbmem + y * width + x);

    *dst = color;
    return 0;
}


int clear_screen(void* fbmem,unsigned int fb_w,unsigned int fb_h,unsigned int color)
{
    unsigned int loop_x;
    unsigned int loop_y;

    for(loop_y = 0;loop_y < fb_h;loop_y++)
    {
        for(loop_x = 0;loop_x < fb_w;loop_x++)
        {
            fb_pixel(fbmem,fb_w,fb_h,loop_x,loop_y,color);
        }
    }
    return 0;
}

int clear_area(void* fbmem,unsigned int fb_w,unsigned int fb_h,unsigned int p_x,
               unsigned int p_y,unsigned int area_w,unsigned int area_h,unsigned int color)
{
    unsigned int loop_x;
    unsigned int loop_y;

    for(loop_y = 0;loop_y < area_h;loop_y++)
    {
        for(loop_x = 0;loop_x < area_w;loop_x++)
        {
            fb_pixel(fbmem,fb_w,fb_h,loop_x+p_x,loop_y+p_y,color);
        }
    }
    return 0;
}
#endif

void drawRect_rgb16 (int x0, int y0, int width, int height, int color)
{

}


