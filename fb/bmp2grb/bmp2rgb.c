#include "bmp2rgb.h"



int nornal_disp(int width, int height, char *srcdata, char *dast_data)
{
	if( (srcdata == NULL) || (dast_data == NULL) ){
		printf("[nornal_disp]: Input [srcdata] or [dast_data] is NULL, exit !\n");
		return -1;
	}
	
	int i,j;
	int h = 0;
	char red = 0;
	char geen = 0;
	char blue = 0;
	int line_size = width * 3;
	unsigned short tmpdata = 0;
	
	for( j = height; j > 0 ; j--) 
	{	
		for( i = 0; i < width * 3;)
		{
			blue = srcdata[line_size * j + i] >> 3;
			geen = srcdata[line_size * j + i + 1] >> 2;
			red = srcdata[line_size * j + i + 2] >> 3;
			i +=3;
			
			tmpdata = (red<<11)  + (geen<<5) + blue;
			
			dast_data[h] = tmpdata ;
			dast_data[h+1] = tmpdata >> 8;
			h+=2;	
		}
	}

	return h;
}


int vert_mirr_disp(int width, int height, char *srcdata, char *dast_data)
{
	if( (srcdata == NULL) || (dast_data == NULL) ){
		printf("[nornal_disp]: Input [srcdata] or [dast_data] is NULL, exit !\n");
		return -1;
	}
	
	int i,j;
	int h = 0;
	char red = 0;
	char geen = 0;
	char blue = 0;
	int line_size = width * 3;
	unsigned short tmpdata = 0;
	
	for( j = 0; j < height; j++)
	{	
		for( i = 0; i < width * 3;)
		{
			red  = srcdata[line_size * j + i] >> 3;
			geen = srcdata[line_size * j + i + 1] >> 2;
			blue = srcdata[line_size * j + i + 2] >> 3;
			i +=3;
			
			tmpdata = (red<<11)  + (geen<<5) + blue;
			
			dast_data[h] = (tmpdata >> 8) &0xff;
			dast_data[h+1] = tmpdata &0xff;
			h+=2;	
		}
	}


	return h;
}

int hor_verti_disp(int width, int height, char *srcdata, char *dast_data)
{
	if( (srcdata == NULL) || (dast_data == NULL) ){
		printf("[nornal_disp]: Input [srcdata] or [dast_data] is NULL, exit !\n");
		return -1;
	}
	
	int i,j;
	int h = 0;
	char red = 0;
	char geen = 0;
	char blue = 0;
	int line_size = width * 3;
	unsigned short tmpdata = 0;
	
	for( j = height; j > 0 ; j--)
	{	
		for( i = width * 3; i >0 ;)
		{
			red  = srcdata[line_size * j + i] >> 3;
			geen = srcdata[line_size * j + i + 1] >> 2;
			blue = srcdata[line_size * j + i + 2] >> 3;
			i -=3;
			
			tmpdata = (red<<11)  + (geen<<5) + blue;
			
			dast_data[h] = (tmpdata >> 8) &0xff;
			dast_data[h+1] = tmpdata &0xff;
			h+=2;	
		}
	}

	return h;
}


int get_bmp_head(FILE* fd, BMP_head_t *bmp_head)
{
	if(bmp_head == NULL){
		printf("[get_bmp_head]: Input [bmp_head] is NULL, exit !\n");
		return -1;
	}

	int ret = 0;
	char buf[1024] = {'\0'};
	ret = fread(buf, 1, sizeof(BMP_head_t), fd);
	if(ret <= 0){
		printf("[get_bmp_head]: read bmp head failed, error:%m\n");
		return -1;
	}

	memcpy(bmp_head, buf, ret);
	return ret;
}

int spare_bmp_head(FILE * fd, int *width, int *height, int *data_size)
{
	BMP_head_t bmp_head;
	memset(&bmp_head, 0, sizeof(BMP_head_t));
	
	if(get_bmp_head(fd, &bmp_head) <0){
		printf("[spare_bmp_head]: get bmp head failed, exit !\n");
		return -1;
	}

	printf("bitSize:[%d]==bitcou8nt:[%d]==bitWidth:[%d]==gbitHeight:[%d]====body:[%d]\n", \
			bmp_head.date_info.bitSize, bmp_head.date_info.bitBitCount, \
			bmp_head.date_info.bitWidth, bmp_head.date_info.gbitHeight, bmp_head.date_info.biSizeImage);

	if( (bmp_head.date_info.bitWidth <= 0 )  || (bmp_head.date_info.bitWidth > MAX_WIDTH )){
		printf("[spare_bmp_head]: get bmp file bitWidth error, exit !\n");
		return -1;
	}

	if( (bmp_head.date_info.gbitHeight <= 0 )  || (bmp_head.date_info.gbitHeight > MAX_HEIGHT )){
		printf("[spare_bmp_head]: get bmp file gbitHeight error, exit !\n");
		return -1;
	}

	if( (bmp_head.date_info.bitBitCount <= 0 )  || (bmp_head.date_info.bitBitCount > 32 )){
		printf("[spare_bmp_head]: get bmp file bps error, exit !\n");
		return -1;
	}
	
	if( (bmp_head.date_info.biSizeImage <= 0 )  || (bmp_head.date_info.biSizeImage > MAX_FILE )){
		printf("[spare_bmp_head]: get bmp file bps error, exit !\n");
		return -1;
	}

	if( (bmp_head.head_info.bfOffBits <= 0 )  || (bmp_head.head_info.bfOffBits > 128 )){
		printf("[spare_bmp_head]: get bmp file offsize error, exit !\n");
		return -1;
	}
	
	*width = bmp_head.date_info.bitWidth;
	*height = bmp_head.date_info.gbitHeight;
	*data_size = bmp_head.date_info.biSizeImage;
	
	fseek(fd, bmp_head.head_info.bfOffBits, SEEK_SET);

	return 0;
}

int conv_data_to_rgb565(FILE *src_fd, FILE *dst_fd, int width, int height, int body_size, char image)
{
	int i,j;
	int ret = 0;
	int line_size = width * 3;
	
	char data[body_size];
	int dst_size = width * height *2;
	char dast_data[dst_size] ;
	
	memset(data, '\0',  body_size);
	memset(dast_data, '\0',  dst_size);
	
	ret = fread(data, body_size, 1, src_fd);
	if(ret <= 0){
			printf("[conv_data_to_rgb565]: get bmp file body data error, exit !\n");
		return -1;
	}	

	printf("bmp file:[%d]-[%d]--{[%d]:[%d]}\n", height, width, body_size, dst_size);

	if(image == 0) {//正常显示
		ret = nornal_disp(width, height, data, dast_data);
		printf("@@@---[NORNAL]->\n");
	}
	else if(image == 1){//水平镜像
		ret =  hor_verti_disp(width, height, data, dast_data);
	}
	else if(image == 2){//垂直镜像
		ret = vert_mirr_disp(width, height, data, dast_data);
	}

	if(ret <= 0){
			printf("[conv_data_to_rgb565]: get rgb565 data error, exit !\n");
		return -1;
	}
	
	printf("rgb565 file size:[%d k]\n", ret / 1024);
	if (fwrite(dast_data, ret, 1, dst_fd) <= 0){
			printf("[conv_data_to_rgb565]: write rgb565 data failed, exit !\n");
		return -1;
	}

	return ret;
}

int bmp24_2_rgb565(Pho_conver_t *conver, image_inf_t  *inf)
{
	if(conver == NULL || inf == NULL){
		printf("[bmp24_2_rgb16]: Input [conver] is NULL, exit !\n");
		return -1;
	}

	if( (conver->src_fname[0] == '\0') ){
		printf("[bmp24_2_rgb16]: Input [conver.src_fname] is '\\0', exit !\n");
		return -1;
	}

	if( (conver->dest_fname[0] == '\0') ){
		printf("[bmp24_2_rgb16]: Input [conver.src_fname] is '\\0', exit !\n");
		memset(conver->dest_fname,  '\0',  sizeof(conver->dest_fname));
		sprintf(conver->dest_fname, "%s-2-rgb565.bin", conver->src_fname);
	}

	int ret = 0;
	int width = 0;
	int height = 0;
	int data_body = 0;
	image_inf_t  img;
	memset(&img, 0, sizeof(img));
	
	FILE *src_fd, *dst_fd;
	src_fd = fopen(conver->src_fname, "r");
	if(src_fd <= 0){
		printf("[bmp24_2_rgb16]: open src file failed, error:%m !\n");
		return -1;
	}
	
	dst_fd = fopen(conver->dest_fname, "w+");
	if(src_fd <= 0){
		printf("[bmp24_2_rgb16]: open dest file failed, error:%m !\n");
		return -1;
	}

	if(spare_bmp_head(src_fd, &width, &height, &data_body) < 0){
		printf("[bmp24_2_rgb16]: Get bmp header info failed, exit!\n");
		return -1;
	}

	ret = conv_data_to_rgb565(src_fd, dst_fd, width, height, data_body, 0);
	if(ret <= 0){
			printf("[bmp24_2_rgb16]: conver rgb888 to rgb565 failed, exit!\n");
		return -1;
	}
	
	img.bfSize  = ret;
	img.bwidth  = width;
	img.bheight = height;

	*inf  = img;
	
	fclose(src_fd);
	fclose(dst_fd);

	
	return 0;
}


