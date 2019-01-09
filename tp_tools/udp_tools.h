#ifndef _UDP_TOOLS_H_
#define _UDP_TOOLS_H_



void udp_socket_setup(char *ip_addr_dst, uint32_t udp_port_dst, uint32_t udp_port_src);
void udp_send_mpeg(uint32_t *tp_buff, uint32_t tp_indx_beg, uint32_t tp_numb);



#endif // _UDP_TOOLS_H_


