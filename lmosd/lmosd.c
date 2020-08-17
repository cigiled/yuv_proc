#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "lmosd.h"

file_t  fltm[MAX_FT];
yuv_t  bgcolor; 
yuv_t  ftcolor;
yuv_t  *tmpcolor = NULL; 
uint test_num = 0;

int Lm_char_ty_check(char *data);
int Lm_get_typeface(uchar *indata, uchar ty, char *outdata);
int Lm_font_zoom(char *data, uchar size);
int Lm_all_wtite(font_t *ft);
int Lm_part_write(font_t   *ft); 
int Lm_set_YUV420_pixel(font_t   *ft, yuv_t *color, int i, int j, uint isnew);
int Lm_set_YUV_pixel(font_t   *ft,      char *data, uchar type);
int Lm_set_board(char *data);

void print_tnt(char *buf, int len, const char *comment)
{
	int i = 0;
	printf("%s start buf:%p len:%d\n", comment, buf, len);
	for(i = 0; i < len; i++)
	{
    	if((i>0) && (i%16 == 0))
			printf("\r\n");
    	 printf("%02x ", buf[i] & 0xff); //&0xff 是为了防止64位平台中打印0xffffffa0 这样的，实际打印0xa0即可
	}
	printf("\n");
}

int Lm_get_fontfile_stat(uchar language) //查看对应语言字库文件是否已经打开过.
{
	int i = 0;
	do
	{
		if(fltm[i].langu_ty == language)
		{
			if(fltm[i].fd > 0)
				return 0;
		}
	}while (i++ <LAN_MAX);
		
	return 1;
}

int Lm_start_font_file(const uchar *file, uchar language)
{
	static int inder = 0;
	int seqnum = 0;
	if(language >= LAN_MAX)
	{
       return 0;
	}
	
	if(Lm_get_fontfile_stat(language) == 0)
	{
		return 0;
	}

	if(inder >= MAX_FT) //当切换语言时,释放上一种语言的内存,ascall 除外.
	{
		if(fltm[0].langu_ty == LAN_ASCALL)
			seqnum = 1;
		else 
			seqnum = 0;

		fclose(fltm[seqnum].fd);
		if(fltm[seqnum].mem)
			free(fltm[seqnum].mem);
		memset(&fltm[seqnum], 0, sizeof(file_t));
	}
	
	if(!file)
	{
		printf("input file is NULL !\n");
		return -1;
	}

	int ret = 0;
	fltm[inder].langu_ty = language;
	fltm[inder].fd = fopen(file, "rb");
	if(fltm[inder].fd <= 0)
	{
		printf("fopen file:%s  failed: %s !\n", file, strerror(errno));
		return -1;
	}

	struct stat  st;
	if(lstat(file, &st) <0)
	{
		printf("lstat failed !\n");
		return -1;
	}

	fltm[inder].isopen = 1;
	fltm[inder].flen = st.st_size;
	fltm[inder].mem  = malloc(fltm[inder].flen);
	if(fltm[inder].mem == NULL)
	{
		fclose(fltm[inder].fd);
		memset(&fltm[inder], 0, sizeof(file_t));
		printf("malloc file:%s  mem failed !\n", file);
		return -1;
	}
	
	ret = fread(fltm[inder].mem, 1, fltm[inder].flen, fltm[inder].fd);
	if(ret <= 0)
	{
		fclose(fltm[inder].fd);
		if(fltm[inder].mem)
			free(fltm[inder].mem);
		memset(&fltm[inder], 0, sizeof(file_t));
		printf("Read file:%s failed. error:%s !\n", file, strerror(errno));
		return -1;
	}
	
	inder++;
	return 0;
}


//点阵, 每8个作为1个字节保存在字库文件中.
//在16*16的点阵中, 一个字符实际有:16*16/8个字节
//这里将字库中的某个字符的某个字节中的某个bit 用1字节来标识。
//这样就还原出一个16*16的点阵数据.
#if 0

inline void Lm_bit_2_byte(char byte, char* buff)
{
	char tmp = byte;
	uint  j;

	for(j = 0; j < 8; j++)
	{
		*buff++ = tmp&0x01;
		tmp >>= 1;
	}
}


void Lm_font_2_byte(char* bbuff, char* bitbuff)
{
	int j = 0;
	for(; j < ZK_FSZ; j++)
	{
		Lm_bit_2_byte(bbuff[j], &bitbuff[j*8]);
	}
}
#else
inline void Lm_bit_2_byte( char bit, char* buff)
{
	int j;
	unsigned char vl;
	unsigned char mask[2] = {0x00,0x01};
	unsigned char maskbit[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};  //取设置字节位用的掩码

	char *tmp =  buff;
	for(j = 0; j < 8; j++)
	{
		vl = (bit & maskbit[7-j]) ? 1 : 0;
		*buff = *buff | mask[vl];
		buff++;
	}
}

void Lm_font_2_byte(char* bbuff, char* bitbuff, int hight)
{
	int j = 0;
	for(; j < hight; j++)
	{
		Lm_bit_2_byte(bbuff[j], &bitbuff[j*8]);
	}
}

#endif

int Lm_get_typeface(uchar *indata, uchar ty, char *outdata)
{
	int check = 0;
	if(fltm[0].langu_ty == ty)
		check = 0;
	else if(fltm[1].langu_ty == ty)
		check = 1;
	else
		check = -1;
		
	if(check < 0)
	{
		printf("Font library file is not open, exit !\n");
		return -1;
	}

	int offse = 0;
	char bitbuf[ZK_BSZ] = {'\0'};
	char fdata[ZK_FSZ] = {'\0'};;
	switch(ty)
	{	
		case LAN_JTZW: //中文
				offse = ((indata[0] - 0xA1) * 94 + indata[1] - 0xA1) * ZK_FSZ; //计算出偏移
				memcpy(fdata, fltm[check].mem + offse, ZK_FSZ);
		
				//printf("===[%02x]:[%02x]===\n", indata[0], indata[1]);
				//print_tnt(fltm[check].mem + offse, ZK_FSZ, "chinese font Byte:");
				
				Lm_font_2_byte(fdata, bitbuf, ZK_FSZ);
				//print_tnt(bitbuf, ZK_BSZ, "chinese font bit:");
				
				memcpy(outdata, bitbuf, ZK_BSZ);
				break;
					
		case LAN_ASCALL://ascall
				offse = indata[0] * ZK_FSZ/2;
				memcpy(fdata, fltm[check].mem + offse, ZK_FSZ/2);
				
				//printf("===[%d]=====\n", indata[0]);
				//print_tnt(fltm[check].mem  + offse, ZK_FSZ/2, "ascall font Byte:");
				
				Lm_font_2_byte(fdata, bitbuf, ZK_FSZ/2);
				//print_tnt(bitbuf, ZK_BSZ/2, "ascall font bit:");
				
				memcpy(outdata, bitbuf, ZK_BSZ/2);
				break;
					
		default:
				break;
	}

	return 0;
}

int Lm_char_ty_check(char *data)
{
	if((*data & 0xff) > 0xa0)
	{
		return LAN_JTZW;
	}
	else if((*data & 0xff) < 0x80)   //ASCII处理
	{
       return LAN_ASCALL;
	}
	else
	{
		return -1;
	   //.....暂时不添加
	}
	return 0;
}

int Lm_set_YUV420_pixel(font_t   *ft, yuv_t *color, int i, int j, uint isnew)
{
	test_num++;
	static uchar *data = NULL;
	static uint  ybase = 0;
	static uint  ubase = 0;
	static uint  vbase = 0;

	if(isnew)
	{
	    data  = ft->yuvadd;
		ybase = ft->cd.y*ft->yuv_w + ft->cd.x;
		ubase = ft->ysize + ft->cd.y*ft->yuv_w /4 + ft->cd.x/2;
		vbase = ft->ysize *5/4 + ft->cd.y*ft->yuv_w /4 + ft->cd.x/2;
	}
		
	if(j%2 == 0)
	{
		*(data + ybase + i + j*ft->yuv_w) = color->y;
		*(data + ubase + i/2 + j*ft->yuv_w /4) = color->u;
		*(data + vbase + i/2 + j*ft->yuv_w /4) = color->v;
	}
	else
	{
		*(data + ybase + i + j*ft->yuv_w) = color->y;
	}
	
	//printf("--[%d]==[%d]:[%d]==[%d]:[%d]:[%d]==:[%d]--->\n", isnew, i, j,  ybase, ubase, vbase, j*ft->yuv_w);
}

int Lm_set_YUV_pixel(font_t   *ft,      char *data, uchar type)
{
	int i = 0, j = 0, font_w = 0, bnew = 1;
	int zoom = ft->iSize;

	
	//print_tnt(data, ZK_BSZ, "font bit:");
	font_w = (type== LAN_JTZW ? ZK_H : ZK_H/2);
	
	//printf("[%d]:[%d]:[%d]:[%d]===[%d]:[%d]==\n", ft->cd.x, ft->cd.y, ft->cd.w, ft->cd.h, ft->yuv_w, ft->yuv_h);
	for(; j < ZK_H*zoom; j++)
	{
		for(i=0; i < font_w *zoom; i++)
		{
			if(data[i + j*font_w*zoom] == 1){//字体处理.
				tmpcolor = &ftcolor;
			}
			else 
			{//背景处理
				if(ft->backGround > 0)
					tmpcolor = &bgcolor; //设置背景
				else
					tmpcolor = NULL; //不设置背景,即透明
			}

			if(ft->yuv_ty) //yuv420
			{
				if(tmpcolor) // 0:字体, 1:背景, -1:不执行
				{
					Lm_set_YUV420_pixel(ft, tmpcolor, i, j, bnew);
					bnew = 0;
				}	
			}
			else //yuv422
			{
				
			   //........
			}

			tmpcolor = NULL; 
		}	
	}

	//printf("-----dsgddskiu---[%d]--->\n", test_num);
	test_num = 0;
	return 0;
}

void Lm_set_color(uint colorval, yuv_t      *color)
{
	int r, g, b;
	r = (colorval & 0xff0000) >>16;
	g = (colorval & 0x00ff00) >>8;
	b = colorval & 0x0000ff;
	
	if(r == 255 && g == 255 && b == 255)
	{		
		color->y = 255;
		color->u = 128;
		color->v = 128;
	}else
	{
		//白色转换失真使用上面值
		color->y = 0.257*r   + 0.504*g + 0.098*b + 16;
		color->u = -0.148*r - 0.291*g + 0.439*b + 128;
		color->v = 0.439*r   - 0.368*g - 0.071*b + 128;
	}

}

/* 放大演示example:
	 *1 2
	 *3 4
	 *放大2倍: ==>isize = 2
	 *		1 1 2 2
	 * 		1 1 2 2
	 *		3 3 4 4
	 *		3 3 4 4
	 *放大3倍: ==>isize = 3
	 *  	1 1 1 2 2 2
	 *		1 1 1 2 2 2
	 *		1 1 1 2 2 2
	 *		3 3 3 4 4 4
	 *		3 3 3 4 4 4
	 *		3 3 3 4 4 4
 */
//点阵数据放大.
int Lm_font_zoom(char *data, uchar size)
{
	if(size >MFSZ)
		size = MFSZ;

	char tmp[ZK_ZOOMSZ];

	memcpy(tmp, data, ZK_ZOOMSZ);
	memset(data, 0, ZK_ZOOMSZ);
	
	int j = 0, i = 0;
	do
	{
		for(j =0; j< size; j++)
		{
			data[i +j] = tmp[i];
		}
	}while(i++ < ZK_BSZ);
	
	return 0;
}

int Lm_all_wtite(font_t *ft)
{
	int ret;
	char *file = NULL;
	char *tempfile = "./Lm_all_test.yuv";
	if(!ft->dumpfile)
		file = tempfile;
	else
		file = ft->dumpfile;

	uint basesz = ft->yuv_w*ft->yuv_h *3/2;
	FILE* fd = fopen(file, "wb");
	if(fd == NULL)
	{
		printf("fopen dump file:%s  failed: %s !\n", file, strerror(errno));
		return -1;
	}
	
	ret = fwrite(ft->yuvadd, 1, basesz, fd);
	if(fd == NULL)
	{
		printf("ffwrite dump file:%s  failed: %s !\n", file, strerror(errno));
		return -1;
	}
}
//osd 叠加只是部分yuv数据被修改,采用局部写可以提高效率.
int Lm_part_write(font_t *ft)
{
	uint yoffse = ft->cd.y*ft->yuv_w;
	uint xoffse = ft->cd.x;
	uint basesz = ft->yuv_w*ft->yuv_h;

	uint size = ft->cd.w * ft->cd.h;
	char *yadd = ft->yuvadd + yoffse + xoffse;
	char *uadd = ft->yuvadd + basesz + yoffse/4 + xoffse/2;
	char *vadd = ft->yuvadd + basesz*5/4 + yoffse/4 + xoffse/2;;

	int ret;
	char *file = NULL;
	char *tempfile = "./Lm_part_test.yuv";
	if(!ft->dumpfile)
		file = tempfile;
	else
		file = ft->dumpfile;

	FILE* fd = fopen(file, "r+");
	if(fd == NULL)
	{
		printf("fopen dump file:%s  failed: %s !\n", file, strerror(errno));
		return -1;
	}
	
	ret = fwrite(yadd, 1, size, fd);
	ret = fwrite(uadd, 1, size/4, fd);
	ret = fwrite(vadd, 1, size/4, fd);
	if(ret <= 0)
	{
		printf("fwrite dump file:%s  failed: %s !\n", file, strerror(errno));
		return -1;
	}
	
	return 0;
}

int  Lm_set_string(font_t *ft)
{
	int ret, i = 0, ftbnum = 0, tmpw = 0, tmph = 1;
	char fron_ty;
	uint xbase = ft->cd.x, ybase = ft->cd.y;
	
	Lm_set_color(ft->fontcolor, &ftcolor);
	
    if(ft->backGround > 0)
		Lm_set_color(ft->bordcolor, &bgcolor);
	
	uchar *data = ft->str;
	char tmp[ZK_ZOOMSZ]; //一个字符占用空间:字符字模的宽*高*长宽放大倍数

	while(data[i] != '\0')
	{
		if(data[i] == '\n')
		{ //换行.
			ft->cd.y += tmph *ZK_H*ft->iSize;
			ftbnum = 0;
			tmph++;
			i++;
		}
		else if(data[i] == ' ')
		{//空格 跳过，不应执行下面
			//printf("----tttttt----[%02x]:[%02x]-->\n", data[i], data[i+1]);
			ft->cd.x = xbase + ftbnum*ZK_W/2*ft->iSize;
			i++;
			ftbnum++;
			
			continue;
		}
		
		fron_ty = Lm_char_ty_check(&data[i]);
		if(fron_ty < 0)
		{
			i++;
			continue;
		}
		
		ret = Lm_get_typeface(&data[i], fron_ty, tmp);
		if(ret < 0)
			return -1;

		 //====just for adjust disply position.====
		if(ft->iSize > 1)
		{
			Lm_font_zoom(tmp, ft->iSize);
		}

		if(data[i-1] == ' ')
			tmpw = ZK_W/2; //上一个是空格, ftbnum要
		else
			tmpw = (fron_ty == LAN_JTZW) ? ZK_W : ZK_W/2;
		
		ft->cd.x = xbase;
		if(ftbnum)
			ft->cd.x += ftbnum*tmpw*ft->iSize;

		ftbnum++;
		//====just for adjust disply position.====
		
		Lm_set_YUV_pixel(ft, tmp, fron_ty);
		
		if(fron_ty == LAN_JTZW)
			i += 2;
		else
			i++;
	}

	if(ft->bdunmp) //针对文件的处理,实时的yuv，则可不比写文件.
		//ret = Lm_part_write(ft);
		Lm_all_wtite(ft);
	
	if(ret < 0)
		return -1;
	return 0;
}

int Lm_destroy_lmosd()
{
	int i = 0;
	do
	{
		fclose(fltm[i].fd);
		fltm[i].fd   = NULL;
		fltm[i].flen = 0;
		fltm[i].isopen = 0;
		if(fltm[i].mem)
			free(fltm[i].mem);
	}while(i++ < MAX_FT -1);
		
	return 0;
}
