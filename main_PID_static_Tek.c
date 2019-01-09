#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include "tp_tools/udp_tools.h"
#include "tp_tools/tp_head_tools.h"
#include "tp_tools/tp_af_tools.h"
#include "tp_tools/tp_streaming_tools.h"

#include "tp_tools/ng_video_diag.h"



int read_tek_ts_file(char *path, char **buff_p, uint32_t *size_p, char **t_stamp_p);



int main(int argc, char *argv[])
{
    uint32_t  *buff, buff_size;
    uint32_t  *tp_buff, tp_buff_size;
    uint32_t  *tek_t_stamp;      // Tektronics Extra Time Stamp
    int32_t    tp_loc_byte;
    uint32_t   tp_numb;

    if (read_tek_ts_file(argv[1],
                         (char **) &buff, &buff_size,
                         (char **) &tek_t_stamp) < 0) {
        printf("can not open file %s\n", argv[1]);
        exit(-1);
    }

    tp_loc_byte = tp_search_sync(buff, buff_size);

    tp_buff = buff + tp_loc_byte/sizeof(uint32_t);
    tp_buff_size = buff_size - tp_loc_byte;
    tp_numb = tp_buff_size / TP_SIZE_BYTE;

    get_pid_stat(tp_buff, tp_numb);

    exit(0);
}


