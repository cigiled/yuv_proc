#include "public.h"

void print_hexP(char *buf, int len, const char *comment)
{
	int i = 0;
	printf("\r\n%s start buf:%p len:%d \r\n", comment, buf, len);
	for(i = 0; i < len; i++)
	{
	if(i%16 == 0)
		printf("\r\n");
	 printf("%02x ", buf[i]);
	}
}

