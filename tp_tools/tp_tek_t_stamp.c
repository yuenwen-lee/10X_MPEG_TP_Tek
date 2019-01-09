#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>

#include "tp_head_tools.h"
#include "tp_tek_t_stamp.h"



// *****************************************************************************
// *****************************************************************************
// *****************************************************************************

uint32_t tp_tek_t_stamp_get(uint32_t *tek_t_stamp, uint32_t tp_indx)
{
    return(ntohl(tek_t_stamp[tp_indx * 4 + 3]));
}


void tp_tek_t_stamp_dump(uint32_t *tek_t_stamp, uint32_t tp_num)
{
    uint32_t  t, n;

    for (n = 0; n < tp_num; ++n) {
        t = tp_tek_t_stamp_get(tek_t_stamp, n);
        printf("%u\n", t);
    }
}


void tp_tek_t_stamp_stat_dump(uint32_t *tek_t_stamp, uint32_t tp_num)
{
    uint32_t   t, t_last;
    uint32_t   diff, diff_max, diff_min;
    uint64_t   diff_total;
    uint32_t   n;

    diff_max = 0;
    diff_min = (uint32_t) -1;
    diff_total = 0;

    t_last = tp_tek_t_stamp_get(tek_t_stamp, 0);
    for (n = 1; n < tp_num; ++n) {
        t = tp_tek_t_stamp_get(tek_t_stamp, n);
        diff = t - t_last;

        t_last = t;

        if (diff_max < diff)
            diff_max = diff;
        if (diff_min > diff)
            diff_min = diff;
        diff_total += diff;
    }
    diff_total /= (tp_num - 1);

    printf("Avrg: %llu\n", diff_total);
    printf(" Max: %u\n",  diff_max);
    printf(" min: %u\n",  diff_min);
}


// *****************************************************************************
// *****************************************************************************
// *****************************************************************************

int read_tek_ts_file (char *path,
		      char **buff_p, uint32_t *size_p, char **t_stamp_p)
{
    struct stat  f_stat;
    FILE   *fp;
    int     tp_numb;
    int     video_size, t_stamp_size;
    char   *video_data;
    char   *t_stamp_data;
    int     n;

    printf("video file path: %s\n", path);
    if (stat(path, &f_stat) < 0) {
        return -1;
    }
    printf("Video file size: %u byte\n", (uint32_t) f_stat.st_size);
    printf("\n");

    tp_numb      = f_stat.st_size / (TP_SIZE_BYTE + TEK_T_STAMP_SIZE);
    video_size   = tp_numb * TP_SIZE_BYTE;
    t_stamp_size = tp_numb * TEK_T_STAMP_SIZE;
    if ((video_size + t_stamp_size) != f_stat.st_size) {
        printf("WARNING, fragment TP in file\n");
    }

    video_data   = (char *) malloc(video_size);
    t_stamp_data = (char *) malloc(t_stamp_size);

    fp = fopen(path, "r");
    if (fp == NULL) {
        printf("Can not open file %s to read\n", path);
        exit(1);
    }

    for (n = 0; n < tp_numb; ++n) {
        // read 188 Byte MPEG TP
        fread((video_data + (n * TP_SIZE_BYTE)), sizeof(char), TP_SIZE_BYTE, fp);
        // read 188 Byte MPEG TP
        fread((t_stamp_data + (n * TEK_T_STAMP_SIZE)), sizeof(char), TEK_T_STAMP_SIZE, fp);
    }

    fclose(fp);

    *buff_p    = video_data;
    *size_p    = video_size;
    *t_stamp_p = t_stamp_data;

    return 0;
}
