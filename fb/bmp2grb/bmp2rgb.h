#ifndef __BMP2GRB_H__
#define __BMP2GRB_H__

#include "../common/public.h"

#define MAX_WIDTH   3840
#define MAX_HEIGHT  2160
#define MAX_FILE    (MAX_WIDTH * MAX_HEIGHT * 3) //for max rgb888


int get_bmp_head(FILE* fd, BMP_head_t *bmp_head);
int spare_bmp_head(FILE * fd, int *width, int *height, int *data_size);
int bmp24_2_rgb565(Pho_conver_t *conver, image_inf_t  *inf);
int conv_data_to_rgb565(FILE *src_fd, FILE *dst_fd, int width, int height, int body_size, char image);

int nornal_disp(int width, int height, char *srcdata, char *dast_data);
int vert_mirr_disp(int width, int height, char *srcdata, char *dast_data);
int hor_verti_disp(int width, int height, char *srcdata, char *dast_data);


#endif
