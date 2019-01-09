#ifndef _TP_STREAMING_TOOLS_H
#define _TP_STREAMING_TOOLS_H



void tp_mark_tx_time(uint32_t *tp_buff, uint32_t tp_buff_size, uint32_t *tx_time, uint8_t *tx_time_type);
void tp_dump_tx_time(uint32_t *tx_time, uint8_t *tx_time_type, uint32_t tp_numb);

uint32_t get_local_time_in_msec(void);

void tp_ts_stream_tx(char *ip_addr_dst, uint32_t udp_port_dst,
                     uint32_t *tp_buff, uint32_t tp_buff_size, uint32_t tp_send_req);



#endif // _TP_STREAMING_TOOLS_H
