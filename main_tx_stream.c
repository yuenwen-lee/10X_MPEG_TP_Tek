#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include "tp_tools/udp_tools.h"
#include "tp_tools/tp_head_tools.h"
#include "tp_tools/tp_af_tools.h"
#include "tp_tools/tp_streaming_tools.h"




int read_ts_file(char *path, char **buff_p, uint32_t *size_p);




int main(int argc, char *argv[])
{
    char      *f_name;
    char      *ip_addr_dst = "127.0.0.1";
    uint32_t   udp_port_dst = 1122;

    uint32_t  *buff, buff_size;
    uint32_t  *tp_buff, tp_buff_size;
    int32_t    tp_loc_byte;
    uint32_t   tp_send_req = 1000000;


    if (argc < 2) {
        printf("usuage: %s ts_file [dst_ip] [dst_udp]\n", argv[0]);
        printf("  ts_file: TS file (188 TP)\n");
        printf("  dst_ip : destination IP address (default 127.0.0.1)\n");
        printf("  dst_udp: destination UDP port   (default 1122)\n");
        exit(1);
    }

    f_name = argv[1];
    if (argc >= 3)
        ip_addr_dst = argv[2];
    if (argc >= 4)
        udp_port_dst = atoi(argv[3]);
    printf("  file   : %s\n", f_name);
    printf("  dst IP : %s\n", ip_addr_dst);
    printf("  dst UDP: %d\n", udp_port_dst);


    if (read_ts_file(f_name, (char **) &buff, &buff_size) < 0) {
        printf("can not open file %s\n", f_name);
        exit(-1);
    }

    tp_loc_byte = tp_search_sync(buff, buff_size);
    printf("Find the location %d (in byte)\n", tp_loc_byte);

    tp_buff = buff + tp_loc_byte/sizeof(uint32_t);
    tp_buff_size = buff_size - tp_loc_byte;

    tp_ts_stream_tx(ip_addr_dst, udp_port_dst, tp_buff, tp_buff_size, tp_send_req);

    exit(0);
}


