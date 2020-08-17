#ifndef __PUBLIC_H__
#define __PUBLIC_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


typedef unsigned int   uint;
typedef unsigned short u16;


typedef enum
{
	BMP24_T_RGB565,
	BMP24_T_ARGB1555,
	BMP24_T_ARGB4444,
	BMP24_T_ARGB8888,
}Conv_type;

typedef struct 
{
	char src_fname[128];
	char dest_fname[128];
	Conv_type   type;
}Pho_conver_t;

typedef struct 
{ 
	uint   bitSize;   	      //本结构所占用字节数 
	uint   bitWidth;          //位图的宽度，以像素为单位 
	uint   gbitHeight;        //位图的高度，以像素为单位 
	u16    biPlanes;		  //目标设备的级别，必须为1 
	u16    bitBitCount;		  //每个像素所需的位数，必须是1(双色), 4(16色)，8(256色)或24(真彩色)之一 
	uint   biCompression;     //位图压缩类型，必须是   0(不压缩),  1(BI_RLE8压缩类型)或2(BI_RLE4压缩类型)之一 
	uint   biSizeImage;        //位图的大小，以字节为单位 
	uint   gbiXPelsPerMeter;   //位图水平分辨率，每米像素数 
	uint   gbiYPelsPerMeter;   //位图垂直分辨率，每米像素数 
	uint   biClrUsed;          //位图实际使用的颜色表中的颜色数 
	uint   biClrImportant;    //位图显示过程中重要的颜色数 
}__attribute__ ((packed))BMP_date_info_t; 

typedef struct
{ 
	u16  bfType;
    uint bfSize;
    uint bfReserved;
    uint bfOffBits;
}__attribute__ ((packed)) BMP_head_info_t; 

typedef struct 
{ 
	BMP_head_info_t  	head_info;
	BMP_date_info_t		date_info;
}BMP_head_t;


typedef struct
{ 
    uint bfSize;
    uint bwidth;
    uint bheight;
} image_inf_t; 


void print_hexP(char *buf, int len, const char *comment);


#endif
