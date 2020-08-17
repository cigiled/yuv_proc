#include <stdio.h>
#include<sys/stat.h>

#include "bmp2grb/bmp2rgb.h"
#include "fb_show/fb_device.h"


#define SERFILR  "hdtn.bmp"
#define DSTFILR  "hdtn.bin"


Pho_conver_t  conver_m;

int argv_process(int args, char *argv[])
{

}

int main(int args, char *argv[])
{
	memset(&conver_m, 0, sizeof(conver_m));
	
	//argv_process(args, argv);
	int ret ;
	image_inf_t  imag;
	memset(&imag, 0,  sizeof(image_inf_t));

	
	conver_m.type = BMP24_T_RGB565;
	memcpy(conver_m.src_fname, SERFILR, strlen(SERFILR));
	memcpy(conver_m.dest_fname, DSTFILR, strlen(DSTFILR));
	
	if(conver_m.type == BMP24_T_RGB565){
		bmp24_2_rgb565(&conver_m, &imag);
	}


	FILE *fd = fopen(DSTFILR, "rb");
	if(fd <= 0){
		printf("fopen bin file failed, exit!\n");
		return -1;
	}

	char data[imag.bfSize];
	memset(data, '\0', imag.bfSize);
	printf("file:[%d],w:[%d],H:[%d]\n", imag.bfSize, imag.bwidth, imag.bheight);
	
	ret = fread(data, imag.bfSize, 1, fd);
	if(ret <= 0){
		printf("fread bin file failed, error:%m.\n");
		return -1;
	}
	
	start_fb();
	show_image(data, imag);

	sleep(4);
	destroy_fb();
	return 0;
}
