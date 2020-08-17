#ifndef __LMOSD_H__
#define __LMOSD_H__

#include <stdio.h>

#ifdef __cplusplus
	extern "C"
	{
#endif

typedef unsigned char uchar;
typedef unsigned int  uint;

#define MAX_SZ   (1024*1024*1.5) //分给处理字库的内存,最大为1.5M
#define MFSZ     (3) //最大3倍字体，太大会有锯齿,影响美观.
					//[可以在制作点阵文件时设置字体大小,再经过这里的放大,可以显示出更大,更好的的字符来]
//默认点阵的长宽为16x16.
#define ZK_W	  16  //字体宽
#define ZK_H  	  16

#define ZK_BSZ    (ZK_W*ZK_H)  //每个字模 在显示或者放大前bit的size.
#define ZK_FSZ    (ZK_BSZ/8)   //存储在点阵文件里的, 每个字模的byte的size.


#define ZK_ZOOMSZ ZK_BSZ*MFSZ*MFSZ

#define FSZ      (1024*280)
#define MAX_FT   (2) //最大同时支持的语言字库.

enum
{
	 BGC_NO = 0, //Don't  have back ground colour
	 BGC_DO,
	 BGC_MAX,
};

enum
{
	FONT_KT = 0,//楷体.
	FONT_ST,    //宋体
	FONT_WRYH,  //微软雅黑
	FONT_MAX,  
};

enum
{
	LAN_ASCALL = 0,//基础功能的.
	LAN_JTZW,  //简体中文
	LAN_HY,    //韩国
	LAN_JA,   //日语
	LAN_MAX,  
};

typedef struct
{
	int y;
	int u;
	int v;
}yuv_t;

typedef struct
{//坐标
	uint  x; 
	uint  y;
	uint  w;
	uint  h;
}cd_t;

typedef struct
{	
	uchar isAddEdge;   //是否描边
	uchar backGround; //0无背景 1纯色背景 2点状背景 3 线状背景
	uchar iSize;	  //字号 1--3
	uchar yuv_ty;

	uchar bdunmp;    //是否需要dump下来查看.
	char  *dumpfile;
	uint  fontcolor;
	uint  bordcolor;
	cd_t  cd;

	uint  yuv_w;
	uint  yuv_h;
	uint  ysize;
	
	uchar *str; //要显示的字符
	uchar *yuvadd; // 要叠加的yuv地址
}font_t;

typedef struct
{
	FILE  *fd;   //字库文件 fd.
	uint  flen;
	uchar *mem; //存字库的内存.
	uchar langu_ty; //语言类型
	uchar isopen; //是否被打开过.
}file_t;

int Lm_get_fontfile_stat(uchar language);

int Lm_start_font_file(const uchar *file, uchar language);
int Lm_set_string(font_t *ft);
int Lm_destroy_lmosd();

#ifdef __cplusplus
	}
#endif

#endif
