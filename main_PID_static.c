#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include "read_video_file.h"
#include "tp_head_tools.h"
#include "tp_af_tools.h"
#include "tp_streaming_tools.h"
// #include "udp_tools.h"




int main(int argc, char *argv[])
{
    uint32_t  *buff, buff_size;
    uint32_t  *tp_buff, tp_buff_size;
    int32_t    tp_loc_byte;
    uint32_t   tp_numb;

    if (read_ts_file(argv[1], (char **) &buff, &buff_size) < 0) {
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


