#include <string.h>
#include <stdlib.h>
#include "lmosd.h"

//注:本程序默认处理的是 gb2312 encoder 的字符串,
//所有 sourceinsight 调整成 gb2312编码 显示，不然可能解析错误.
int main()
{
 	int ret = 0;
	ret = Lm_start_font_file("./font/asc_16x16.zk", LAN_ASCALL);
	ret = Lm_start_font_file("./font/jtzw_16x16.zk", LAN_JTZW);
	if(ret < 0)
	{
		printf("Lm_start_font_file function failed !\n");
		exit(-1);
	}

	int yuv_wdith = 960;
	int yuv_hight = 720;
	int size = yuv_wdith*yuv_hight*3/2;
	char *yuv_buf = malloc(size);
	FILE *fd = fopen("960x720_YUV420.yuv", "rb");
	ret = fread(yuv_buf, 1, size, fd);
	if(ret < 0)
	{
		free(yuv_buf);
		printf("fread yuv file failed !\n");
		exit(-1);
	}
		
	char buff[1024*2] = {'\0'};
	font_t  ft;
	
	ft.cd.x = 10;
	ft.cd.y = 10;
	ft.cd.w = 10;
	ft.cd.h = 10;
	ft.fontcolor  = 0x0000ff; //0x00ff00; //瀛椾綋棰滆壊: 缁胯壊
	ft.bordcolor  = -1; //鑳屾櫙鑹?
	ft.backGround =	BGC_NO; // 鏃犺儗鏅壊,鍒欓?忔槑
	ft.isAddEdge  = 0;
	
	ft.iSize    = 1;
	ft.bdunmp   = 1;
	ft.dumpfile = "./test1.yuv";
	ft.str = buff;
	char *temp0 = "2019年12月15号, This is a  new day !;";
	memcpy(ft.str, temp0, strlen(temp0));
	
	ft.yuv_ty = 1;//0:yuv422, 1:yuv420, 杩欓噷涓嶇粏鍒?.
	ft.yuv_w  = yuv_wdith;
	ft.yuv_h  = yuv_hight;
	ft.ysize  = yuv_wdith *yuv_hight;
	ft.yuvadd = yuv_buf; // 瑕佸彔鍔犵殑yuv鍦板潃
	ret = Lm_set_string(&ft);
	if(ret < 0)
	{
		Lm_destroy_lmosd();
		free(yuv_buf);
		printf("fread yuv file failed !\n");
		exit(-1);
	}

	ft.cd.x = 10;
	ft.cd.y = 200;
	ft.cd.w = 10;
	ft.cd.h = 10;
	ft.fontcolor  = 0x0000ff; //0xbc1717; //瀛椾綋棰滆壊: 鐚╃孩鑹?
	ft.bordcolor  = -1;//0x00; //鑳屾櫙鑹?
	ft.backGround =	BGC_NO;//BGC_DO;
	ft.isAddEdge  = 0;
	
	ft.iSize    = 1;
	ft.bdunmp   = 1;
	ft.dumpfile = "./test1.yuv";
	ft.str 	    = buff;
	char *temp1 = "咋滴啦，吃你家方拉，我胖跟你有关系吗？";
	memcpy(ft.str, temp1, strlen(temp1));

	//鍦ㄥ悓涓?寮爕uv涓婂啓osd,杩欓噷涓嶅啀閲嶅璧嬪??.
	ft.yuvadd = yuv_buf;
	ret = Lm_set_string(&ft);
	if(ret < 0)
	{
		Lm_destroy_lmosd();
		free(yuv_buf);
		printf("fread yuv file failed !\n");
		exit(-1);
	}

	ft.cd.x = 560;
	ft.cd.y = 190;
	ft.cd.w = 10;
	ft.cd.h = 10;
	ft.fontcolor  = 0x00;//0x7f00ff; //瀛椾綋棰滆壊: 涓煶鏉胯摑鑹?
	ft.bordcolor  = -1;//0x215E21; //鑳屾櫙鑹?
	ft.backGround =	BGC_NO;//BGC_DO;
	ft.isAddEdge  = 0;
	ft.iSize    = 1;
	ft.bdunmp   = 1;
	ft.dumpfile = "./test1.yuv";
	ft.str = buff;
	char *temp2 = "        杜牧 【朝代】唐                \n"
 				  "千里莺啼绿映红，水村山郭酒旗风。\n"
 				  "南朝四百八十寺，多少楼台烟雨中。\n";
	memcpy(ft.str, temp2, strlen(temp2));
	
	ft.yuvadd  = yuv_buf;
	ret = Lm_set_string(&ft);
	if(ret < 0)
	{
		Lm_destroy_lmosd();
		free(yuv_buf);
		printf("fread yuv file failed !\n");
		exit(-1);
	}

	//閿?姣乴mosd璧勬簮.	
	Lm_destroy_lmosd();
	free(yuv_buf);

	return 0;
}
