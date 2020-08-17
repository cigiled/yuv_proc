#include <stdio.h>
#include <stdlib.h>

#define MAX (1024 *1024 *2)

typedef unsigned char uint8_t;
int IMG_WIDTH;
int IMG_HEIGHT;
/*
x1, y1, 要截取的方形区域的，左上角坐标
crop_width, crop_height, 要截取的宽、高
*/

void get_yuv420p_crop_img(uint8_t *img_buffer, uint8_t *out_buffer, int x1, int y1, int crop_width, int crop_height)
{
    int i, j, k = 0;

    int orig_y_size = IMG_WIDTH * IMG_HEIGHT,
        orig_u_size = orig_y_size >> 2;

    int crop_x1, crop_y1, crop_x2, crop_y2;
    int h_s, h_e, w_s, w_e;

    int loop_y_s1, loop_y_e1, loop_uv_s1, loop_uv_e1;
    int loop_y_s2, loop_y_e2, loop_uv_s2, loop_uv_e2;

    crop_x1 = x1;
    crop_y1 = y1;
    crop_x2 = x1+crop_width;
    crop_y2 = y1+crop_height;

    loop_y_s1 = y1; //y分量的 第一个 像素点对应的竖坐标位置
    loop_y_e1 = y1+crop_height;  //y分量的  最后一个  像素点对应的竖坐标结束位置
    loop_y_s2 = x1; //y分量的 第一个 像素点对应的横坐标位置
    loop_y_e2 = x1+crop_width; //y分量的 最后一个 像素点对应的横坐标位置

	//u分量:   坐标区域:{x,y,  w, h}===>( loop_uv_s2,  loop_uv_s1,  loop_uv_e2,  loop_uv_e1);
    loop_uv_s1 = loop_y_s1/2; //u,v分量是y分量的 位置的一半.
    loop_uv_e1 = loop_y_e1/2;
	
	//v分量:   坐标区域:{x,y,  w, h}===>( loop_uv_s2,  loop_uv_s1,  loop_uv_e2,  loop_uv_e1);
    loop_uv_s2 = loop_y_s2/2; 
    loop_uv_e2 = loop_y_e2/2;

    /* copy Y section */
    for (i = loop_y_s1; i < loop_y_e1; i++)  //height
	{
        for (j = loop_y_s2; j < loop_y_e2; j++)  //width
		{
            out_buffer[k] = img_buffer[i * IMG_WIDTH + j];
            k++;
        }
    }

	
    /* copy u section */
    for (i = loop_uv_s1; i < loop_uv_e1; i++) 
	{
        for (j = loop_uv_s2; j < loop_uv_e2; j++)
		{
            out_buffer[k] = img_buffer[orig_y_size + i * (IMG_WIDTH/2) + j];
            k++;
        }
    }

	
    /* copy v section */
    for (i = loop_uv_s1; i < loop_uv_e1; i++) 
    {
        for (j = loop_uv_s2; j < loop_uv_e2; j++) 
	    {
            out_buffer[k] = img_buffer[orig_y_size + orig_u_size + i * (IMG_WIDTH/2) + j];
            k++;
        }
    }
	
	
}

int main()
{
	int ret;
	FILE *infd = NULL;
	FILE *oufd = NULL;
	
	int x = 220;
	int y = 140;
	int w = 700;
	int h = 280;
	
	int len = w*h*3/2;
	
	IMG_WIDTH  = 1280;
	IMG_HEIGHT = 720;
	
	infd = fopen("720p_YUV420_1.yuv", "rb");
	oufd = fopen("out.yuv", "wb");
	
	uint8_t *inbuff = malloc(MAX);
	uint8_t *outbuff = malloc(MAX);
	
	ret = fread(inbuff, 1, MAX, infd);

	get_yuv420p_crop_img(inbuff, outbuff, x, y, w, h);
	 
	ret = fwrite(outbuff, 1, len, oufd);
		
	free(inbuff);
	free(outbuff);
	
	return 0;
}


