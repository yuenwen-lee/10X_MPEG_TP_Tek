#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include "tp_head_tools.h"




struct sockaddr_in locl_addr;
struct sockaddr_in peer_addr;
int  s_id;




void udp_socket_setup(char *ip_addr_dst, uint32_t udp_port_dst, uint32_t udp_port_src)
{
    // source address
    memset(&locl_addr, 0, sizeof (locl_addr));
    locl_addr.sin_family      = AF_INET;
    locl_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    locl_addr.sin_port        = htons(udp_port_src);

    // destination address
    memset(&peer_addr, 0, sizeof (peer_addr));
    peer_addr.sin_family      = AF_INET;
    peer_addr.sin_addr.s_addr = inet_addr(ip_addr_dst);
    peer_addr.sin_port        = htons(udp_port_dst);

    s_id = socket(PF_INET, SOCK_DGRAM, 0);
    if (s_id < 0) {
        perror("SOCKET");
        exit(1);
    }

#if 0
    {
        int  rc;
        rc = bind(s_id, (struct sockaddr *) &locl_addr, sizeof(locl_addr));
        if (rc < 0) {
            perror("bind() failed");
            exit(1);
        }
    }
#endif
}


void udp_send_mpeg(uint32_t *tp_buff, uint32_t tp_indx_beg, uint32_t tp_numb)
{
    uint32_t  tp_indx, tp_indx_end;
    uint32_t  tp_numb_in_udp;
    uint8_t  *udp_pyld;
    uint32_t  udp_pyld_size;
    int32_t   ret;

    tp_indx     = tp_indx_beg;
    tp_indx_end = tp_indx_beg + tp_numb;

    while (tp_indx < tp_indx_end) {

        tp_numb_in_udp = tp_indx_end - tp_indx;
        if (tp_numb_in_udp > 7) {
            tp_numb_in_udp = 7;
        }

        udp_pyld = (uint8_t *) (tp_buff + (tp_indx * TP_SIZE_WORD));
        udp_pyld_size = tp_numb_in_udp * TP_SIZE_BYTE;

        ret = sendto(s_id, udp_pyld, udp_pyld_size, 0,
                     (struct sockaddr *) &peer_addr, sizeof (peer_addr));
        if (ret < 0) {
            perror("SENDTO");
            exit(1);
        }

        tp_indx += tp_numb_in_udp;
    }
}
