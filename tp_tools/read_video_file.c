#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>

#include "tp_head_tools.h"
#include "tp_tek_t_stamp.h"



// *****************************************************************************                                      
// *****************************************************************************                                      
// *****************************************************************************                                      

int read_ts_file (char *path, char **buff_p, uint32_t *size_p)
{
    struct stat  f_stat;
    FILE   *fp;
    int     tp_numb;
    int     video_size;
    char   *video_data;

    printf("video file path: %s\n", path);
    if (stat(path, &f_stat) < 0) {
        return -1;
    }
    printf("Video file size: %u byte\n", (uint32_t) f_stat.st_size);
    printf("\n");

    video_data = (char *) malloc(f_stat.st_size);

    tp_numb    = f_stat.st_size / TP_SIZE_BYTE;
    video_size = tp_numb * TP_SIZE_BYTE;
    if (video_size != f_stat.st_size) {
        printf("WARNING, fragment TP in file\n");
    }

    fp = fopen(path, "r");
    if (fp != NULL) {
        video_size = fread(video_data, sizeof(char), video_size, fp);
        fclose(fp);
    } else {
        printf("Can not open file %s to read\n", path);
    }

    *buff_p = video_data;
    *size_p = video_size;

    return 0;
}
