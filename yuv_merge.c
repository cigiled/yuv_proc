#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAX (1024 *1024 *4)
#define ALIGN           4
#define ALIGN_PROC(X)  (X/ALIGN*ALIGN)

//将多个画面拼接到一幅画上. 这里不打算将图片进行缩放后再拼接，
// 因此必须清楚计算，目标图片是否能容纳得下这所有拼接的图片.
typedef unsigned char uint8;
typedef unsigned int uint;

enum
{
	PIX_Y = 0,
	PIX_U,
	PIX_V,
};

typedef struct fd_
{
	FILE* fd;
	char* mem;
	uint  sz;
	uint  img_w;
	uint  img_h;	
}fdinf_t;

typedef struct 
{
	uint     chns;
	fdinf_t* src_fd;
	fdinf_t  dst_fd;
}fd_t;

typedef struct 
{
	uint8* fname;
	uint   width;
	uint   height;
}finf_t;

typedef struct
{
	uint x;
	uint y;
	uint w;
	uint h;
}Pix_t;

finf_t  finf_m[] = {
					{"960x720_f_YUV420.yuv", 960, 720}, {"960x540_YUV420.yuv", 960, 540},
					{"460x720_YUV420.yuv", 460, 720},   {"960x540_f_YUV420.yuv", 960, 540},
					{"960x720_YUV420.yuv", 960, 720},
					{"1920x1080_splic.yuv", 1920, 1080}	
				 };

//通道坐标默认设置.
Pix_t pix_m[] = {
				  {0,0, 720, 576},   {720,0, 720, 576}, {1500,0, 420, 540},
				  {0,576, 400, 504}, {400,576, 900, 504},
				};

void para_align(int *width, int *height);
int  int_res(fdinf_t *tmpft, finf_t *finf, char bread);
int  create_res(fd_t *ft, int chn);

int  free_res(fdinf_t *tmpft);
void destroy_res(fd_t *ft, int chn);

int rgb_covern_yuv(int rgb_color, int *yuvval);
int  gener_dst_bg(fdinf_t * dfd, int bg_color);

void para_align(int *width, int *height);
int adjust_w_h(uint s_w, uint s_h, uint d_w, uint d_h, uint *adj_w, uint *adj_h);
int splic_pics(fd_t *ft, Pix_t* pix);
//方式1:
int judgm_chn_num(int currx, int curry, Pix_t* pix);
int set_coord_chn_pix(Pix_t* pix, int chn, fd_t *ft, char pix_ty);
//方式2:
int split_chn_pic(Pix_t* pix, fdinf_t *sft, fdinf_t *dft);


int main()
{
	int ret;
	fd_t ft;
	int chns  = sizeof(finf_m) / sizeof(finf_t);
    ret = create_res(&ft, chns);
	if(ret < 0)
	{
		exit(-1);
	}
	
	ret = gener_dst_bg(&ft.dst_fd, 0x600030); //0x600030 rgb值.
	if(ret < 0)
	{
		goto END;
	}
		
	splic_pics(&ft, pix_m);

END:

	//释放内存
	destroy_res(&ft, chns);
	printf("-gdfgd-->\n");

	return 0;
}

int create_res(fd_t *ft, int chn)
{	
    int i = 0;
	 
	ft->src_fd = (fdinf_t*)malloc((chn-1) * sizeof(fdinf_t)); //目标图片， 这里不用为其开辟空间
	if(!ft->src_fd)
	{
		printf("Malloc src_fd  struct mem failed: %s\n", strerror(errno));
		return -1;
	}
	
	for(; i < chn-1; i++)
	{
		if(int_res(&ft->src_fd[i], &finf_m[i], 1) < 0)
		{
			printf("src file res init error, exit !\n");
			return -1;
		}
	}

	if(int_res(&ft->dst_fd, &finf_m[chn-1], 0) < 0)
	{
		printf("dest file res init error, exit !\n");
		return -1;
	}

	ft->chns = chn;
	return 0;
}

int int_res(fdinf_t *tmpft, finf_t *finf, char bread)
{
	int sz;
	int ret = 0;
	tmpft->fd = fopen(finf->fname, bread ? "r+" :"w+"); 
	if(tmpft->fd <= 0)
	{
		printf("Fopen file[%s] failed: %s\n", finf->fname, strerror(errno));
		return -1;
	}
	
	tmpft->img_w = finf->width;
	tmpft->img_h = finf->height;
	para_align(&tmpft->img_w, &tmpft->img_h);
	
	sz = tmpft->img_w *tmpft->img_h* 3/2;  //处理的都是yuv420格式的yuv.
	tmpft->sz = sz;
	tmpft->mem = (char*)malloc(sz);
	if(!tmpft->mem)
	{
		fclose(tmpft->fd);
		printf("Malloc file[%s] mem failed: %s\n",finf->fname, strerror(errno));
		return -1;
	}

	ret = fread(tmpft->mem, 1, tmpft->sz, tmpft->fd);
	if(ret < 0)
	{
		printf("Fread file[%s] failed: %s\n", finf->fname, strerror(errno));
		return -1;
	}
	printf("==:[%s],{%d %d %d %d}\n", finf->fname, tmpft->img_w, tmpft->img_h, tmpft->sz, ret);
	return 0;
}

int free_res(fdinf_t *tmpft)
{
	if(tmpft->fd)
		fclose(tmpft->fd);

	if(tmpft->mem)
		free(tmpft->mem);
	
	return 0;
}

void destroy_res(fd_t *ft, int chn)
{
	int i = 0;
	for(; i <chn-1; i++)
	{
		free_res(&ft->src_fd[i]);
	}

	if(ft->dst_fd.fd)
		fclose(ft->dst_fd.fd);
	if(ft->dst_fd.mem)
		free(ft->dst_fd.mem);

	free(ft->src_fd);
}

int rgb_covern_yuv(int rgb_color, int *yuvval)
{
	//若不定义为unsigned 类型, 计算出的rgb可能会出现负数,导致下面yuv计算出错.
	unsigned char r = rgb_color >> 16;
	unsigned char g = (rgb_color>>8) & 0x0000ff;
	unsigned char b = rgb_color & 0x0000ff;

	printf("src:%02x, r:%02x|%d, g:%02x|%d, b:%02x|%d\n", rgb_color, r,r, g,g, b,b);
	unsigned char y = 0.299*r + 0.587*g + 0.114*b;
	unsigned char u = -0.1687*r - 0.3313*g + 0.5*b + 128;
	unsigned char v = 0.5*r - 0.4187*g - 0.0813*b + 128;
	*yuvval  &= 0x0;
	*yuvval = y<<16 | u<<8 | v;

	printf("y:%d, u:%d, v:%d\n", y,u,v);
	return 0;
}

int gener_dst_bg(fdinf_t     * dfd, int bg_color)
{
	int ret;
	int len = dfd->img_w *dfd->img_h;
	char *yadd = dfd->mem; 
	char *uadd = dfd->mem + len; 
	char *vadd = dfd->mem + len *5/4; 

	int yuv;
	rgb_covern_yuv(bg_color, &yuv);
	unsigned char y = yuv>>16;
	unsigned char u = (yuv>>8) & 0xff;
	unsigned char v = yuv & 0xff;

	int i = 0;
	while(i < len*3/2)
	{
		if(i < len)
			yadd[i++] = y; //填充y分量值.
		else if((i >= len) && (i < len*5/4) )
			yadd[i++] = u;	
		else
			yadd[i++] = v;	
	}
	
	ret = fwrite(dfd->mem, 1, dfd->sz, dfd->fd); //写到目标图片上作为背景使用.
	if(ret <= 0)
	{
		printf("fwrite dest file[%d] failed  error: %s\n", fileno(dfd->fd), strerror(errno));
		return -1;
	}
	
	fseek(dfd->fd, 0, SEEK_SET);
	return 0;
}


int adjust_w_h(uint s_w, uint s_h, uint d_w, uint d_h, uint *adj_w, uint *adj_h)
{
	if(s_w > d_w)
		*adj_w = d_w;
	else
		*adj_w = s_w;

	if(s_h > d_h)
		*adj_h = d_h;
	else
		*adj_h = s_h;

	return 0;
}

#if defined WAY1
int judgm_chn_num(int currx, int curry, Pix_t* pix)
{
	int number = 0;
	if(currx < pix[0]->w) 
	{ 
		if(curry < pix[0]->y)
			number = 0;  //第一个通道.
		else if(currx < pix[3]->w)
			number = 3;
		else
			number = 4;
	}
	else if(currx < pix[2]->x)
	{
		if(curry < pix[0]->y)
			number = 1;
		else
			number = 4;
	}
	else
		number = 2;

	return number;
}


//方式1:一整行 一整行的 拼接
//设置通道坐标上的像素.
int set_coord_chn_pix(Pix_t* pix, int chn, fd_t *ft, char pix_ty)
{	
	int sj =0, si =0, adjw = 0, adjh = 0;
	int dw = pix[chn]->w; //目标通道宽度
	int dh = pix[chn]->h;
	int sw = ft->src_fd[chn]->img_w;//源图像宽度
	int sh = ft->src_fd[chn]->img_w;

	adjust_w_h(sw, sh, dw, dh, &adjw, &adjh, pix_ty);

	for(; sj < adjh; sj++)
	{
		for(si = 0; si < adjw; si++)
		{
			 
		}
	}
	
	return 0;
}

int splic_pics(fd_t *ft, Pix_t* pix)
{
	int j = 0, i =0, chn = 0;
	char *srcdata = ft->src_fd[chn]->mem;
	char *dstdata = ft->dst_fd->mem;

	//y:
	for(; j < ft->dst_fd.img_h; j++)
	{
		for(; i < ft->dst_fd.img_h; i++)
		{		
			chn = judgm_chn_num(i, j, pix);
			set_coord_chn_pix(pix, chn, ft, PIX_Y);	
		}
	}
	return 0;
}

#else
//方式2: 每个通道逐个拼接.
int split_chn_pic(Pix_t* pix, fdinf_t *sft, fdinf_t *dft)
{
	int sj =0, si =0, adjw = 0, adjh = 0;
	int chnw = pix->w; //目标通道宽度
	int chnh = pix->h;
	int sw = sft->img_w;//源图像宽度
	int sh = sft->img_h;
	int dw = dft->img_w;
	int dh = dft->img_h;
	char *srcdata = sft->mem;
	char *dstdata = dft->mem;
	
	adjust_w_h(sw, sh, chnw, chnh, &adjw, &adjh);

	//y:
	char *yadd = dstdata + pix->x + pix->y*dw;
	for(sj = 0; sj < adjh; sj++){
		memcpy(yadd + sj*dw, srcdata + sj*sw, adjw);
	}

	//u
	char *uadd = dstdata + dw*dh + pix->x/2 + pix->y/2*dw/2;
	for(sj = 0; sj < adjh/2; sj++){
		memcpy(uadd + sj*dw/2, srcdata + sw*sh + sj*sw/2, adjw/2);
	}

	//v
	char *vadd = dstdata + dw*dh*5/4 + pix->x/2 + pix->y/2*dw/2;
	for(sj = 0; sj < adjh/2; sj++){
		memcpy(vadd + sj*dw/2, srcdata + sw*sh*5/4 + sj*sw/2, adjw/2);
	}

	printf("==x[%d]==w[%d:%d]vs[%d:%d]h===\n", pix->x, adjw, adjh, sw, sh);
	return 0;
}

int splic_pics(fd_t *ft, Pix_t* pix)
{
	if(!ft || !pix)
	{
		printf("Input parameter ft or pix is NULL, exit !\n");
		return -1;
	}
	
	int ret, i = 0;
	
	for(; i < ft->chns-1; i++)
	{
		//printf("[%d]==[%d]:[%d]==\n", i, ft->src_fd[i].img_w, ft->src_fd[i].img_h);
		split_chn_pic(&pix[i], &ft->src_fd[i], &ft->dst_fd);
	}

	printf("\n");
	ret = fwrite(ft->dst_fd.mem, 1, ft->dst_fd.sz, ft->dst_fd.fd);
	if(ret <= 0)
	{
		perror("write  error:");
		return -1;
	}
	return 0;
}

#endif

void para_align(int *width, int *height)
{
	*width = ALIGN_PROC(*width);
	*height = ALIGN_PROC(*height);
}