#include <stdio.h>
#include <stdlib.h>

#define MAX (1024 *1024 *4)

enum
{
  TY_SET_BG_BYPIC = 0,
  TY_SET_BG_BYFILL,
  TY_GET_FRONT,
  TY_MAX
};

typedef unsigned char uint8_t;
int IMG_WIDTH;
int IMG_HEIGHT;

FILE *srcfd = NULL;
FILE *dstfd = NULL;
char *srcdata = NULL;
char *dstdata = NULL;

void create_res();
void destroy_res();

int  set_bg_by_fill(int val);
int  bg_fill(char *date, char fill_val);
char find_bg_val(char *data, int *sum, int val);

//修改背景图
void  bg_cp_pic(char *sda, char *dda, int w, int h, int sw, int sh, int dw, int dh);
int   set_bg_by_pic(int x, int y, int w, int h, int srcw, int srch, int dstw, int dsth);


int main()
{
	int ret,x,y,w,h,len, srcw,srch, dstw,dsth;
	int opt_ty = TY_SET_BG_BYFILL;

    create_res();
	
	//提取前景图
	if(opt_ty == TY_GET_FRONT)
	{
		get_foreg_pic();
	}
	else if(opt_ty == TY_SET_BG_BYPIC)
	{ //设置背景,通过合并图片
		x = 100;
		y = 600;
		w = 700;
		h = 280;
		len = w*h*3/2;	

		IMG_WIDTH  = 700; 
		IMG_HEIGHT = 280;
		
		srcw = IMG_WIDTH;
		srch = IMG_HEIGHT; 
		dstw = 1920;
		dsth = 1080;

		srcfd = fopen("700x280_1.yuv", "r+"); // 提取它的部分数据，然后写到目标文件上.
		dstfd = fopen("1920x1080_YUV420_1.yuv", "r+"); //[TWT]:设置为"rw"时, fseek，fwrite 报错:"write error:: Bad file descriptor"
		dstdata = malloc(MAX);
		
		set_bg_by_pic(x, y, w, h, srcw, srch, dstw, dsth);		
	}
    else 
	{//设置背景,通过填充背景颜色.
		IMG_WIDTH  = 1280;
		IMG_HEIGHT = 720;
		srcfd = fopen("720p_YUV420_1.yuv", "r+");
		set_bg_by_fill(0x80);
	}
	
	//释放内存
	destroy_res();

	return 0;
}

void create_res()
{	
	//开辟内存.
	srcdata = malloc(MAX);
}

void destroy_res()
{
	if(srcdata)
		free(srcdata);
	if(dstdata)
		free(dstdata);

	if(srcfd)
		fclose(srcfd);
	
	if(dstfd)
		fclose(dstfd);

	srcdata = NULL;
	dstdata = NULL;
	srcfd = NULL;
	dstfd = NULL;
}

//提取前景图
int get_foreg_pic(char *date)
{
	int ret;
	ret = fread(srcdata, 1, MAX, srcfd);
	get_foreg_pic(srcdata);
	ret = fwrite(dstdata, 1, ret, dstfd);
	return 0;
}

//=======将一张图复制到另外一张图上,并实现部分透明[复合色, 可透明]============
void bg_cp_pic(char *sda, char *dda, int w, int h, int sw, int sh, int dw, int dh)
{
	char val;
	int i = 0, j = 0;
	int t = 0;
	val = find_bg_val(sda, &i, 10);
	printf("bg val:[%02x]  w:[%d][%d]:h  [%d]:[%d] \n", val, w, h, sw, sh);

	//提取非 背景颜色的颜色值 到 目标图片上.
   //y:
  	for(j = 0; j < h; j++)
  	{
  		for(i = 0; i < w; i++)
  		{
			if( *(sda + i + j*sw) != val)
			{	 
				//printf("[@%d:%02x] ", i+j*sw, 0xff & (*(dda + i + j*sw)));
				*(dda + i + j*dw) = *(sda + i + j*sw);
				//printf("[#%d:%02x] ", i+j*sw, 0xff & (*(dda + i + j*sw)));
				t++;
			}
  		}
	}
	
	//u
	char* ds_uaddr = dda + dw*dh;
	char* sr_uaddr = sda + sw*sh;
	for(j = 0; j < h/2; j++)
  	{
  		for(i = 0; i < w/2; i++)
  		{
			if( *(sr_uaddr + i + j*sw) != val)
			{	 
				*(ds_uaddr + i + j*dw) = *(sr_uaddr + i + j*sw);
				t++;
			}
  		}
	}

	#if 1
	//v
	char* ds_vaddr = dda + dw*dh*5/4;
	char* sr_vaddr = sda + sw*sh*5/4;
	for(j = 0; j < h/2; j++)
  	{
  		for(i = 0; i < w/2; i++)
  		{
			if( *(sr_vaddr + i + j*sw) != val)
			{	 
				*(ds_vaddr + i + j*dw) = *(sr_vaddr + i + j*sw);
				t++;
			}
  		}
	}
	#endif
}

int para_check( int x, int y, int w, int h, int srcw, int srch, int dstw, int dsth)
{
	if(x > dstw)
	{
		printf("x[%d] coordinate should be smaller than dest width[%d]\n", x, dstw);
		return -1;
	}

	if(y > dsth)
	{
		printf("y[%d] coordinate should be smaller than dest height[%d]\n", x, dsth);
		return -1;
	}

	if(w > srcw  || w > dstw)
	{
		printf("width [%d] should be smaller than dest width[%d] or sours  width[%d] \n", w, dstw, srcw);
		return -1;
	}

	if(h > srch  || h > dsth)
	{
		printf("hight [%d] should be smaller than dest hight[%d] or sours  hight[%d] \n", w, dsth, srch);
		return -1;
	}

	return 0;
}

//修改背景图
int set_bg_by_pic(int x, int y, int w, int h, int srcw, int srch, int dstw, int dsth)
{
	int ret, dstart, len;
	ret = fread(srcdata, 1, MAX, srcfd);
	if(ret <= 0)
	{
		perror("read src file error:");
		return -1;
	}
	ret = fread(dstdata, 1, MAX, dstfd);	
	if(ret <= 0)
	{
		perror("read dst file error:");
		return -1;
	}

	printf("11==[%d]:[%d]=====s:[%d] d:[%d]\n", ret, w*h, fileno(srcfd), fileno(dstfd));
	if(para_check(x, y, w,  h, srcw, srch, dstw, dsth) < 0)
	{
		printf("parameter error, exit !\n");
		return -1;
	}

	dstart = dstw *y +x;	
	bg_cp_pic(srcdata, &dstdata[dstart], w, h, srcw, srch, dstw, dsth);
	
	//定位到开始修改的像素位置,从新写.
	fseek(dstfd, dstart, SEEK_SET);
	len = w*h*5/2; //为什么是 5/2, 而不是3/2呢? 不明白??????
	ret = fwrite(&dstdata[dstart], 1, len, dstfd);
	if(ret <= 0)
	{
		perror("write error:");
		return -1;
	}
	
	return 0;
}


//================修改背景图为其他颜色[单色,无法透明]===========================
char find_bg_val(char *data, int *sum, int val)
{
	if(*data == *(data + 1))
	{
		if(val == *sum)
		{
			return *data;
		}
		else
		{
			*sum +=1;
			find_bg_val(data+1, sum, val);
		}
	}
	else
	{
		*sum = 0;
		find_bg_val(data +1, sum, val);
	}
	
}

int bg_fill(char *date, char fill_val)
{
	int j ,i;
	int sum = 0;
	
	//找到背景颜色值.  [这里处理比较简单, 认为前面连续的几个像素的y值相同，就认为该值是背景颜色的值 ]
	char bgval = find_bg_val(date, &sum, 10);
	
	//开始将背景颜色值填充为其他颜色值.
	//y分量部分:
	for(j=0; j < IMG_WIDTH * IMG_HEIGHT; j++)
	{
		if(date[j] == bgval)
			date[j] = fill_val;
	}
	
	return 0;
}

int set_bg_by_fill(int val)
{
	int ret;
	ret = fread(srcdata, 1, MAX, srcfd);
	if(ret <= 0)
	{
		perror("fread src file error:");
		return -1;
	}
	bg_fill(srcdata, val); 
	
	//定位到开头,从新写.
	fseek(srcfd, 0, SEEK_SET);
	ret = fwrite(srcdata, 1, ret, srcfd);
	if(ret <= 0)
	{
		perror("write  error:");
		return -1;
	}

	return 0;
}
