#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include "udp_tools.h"
#include "time_os.h"
#include "tp_head_tools.h"
#include "tp_af_tools.h"
#include "tp_streaming_tools.h"




#define SLEEP_PERIOD_USEC  5000    // 5,000 use sleep




uint32_t   udp_port_src = 9999;

uint32_t  *tx_time_buff;
uint8_t   *tx_time_type_buff;




void tp_ts_stream_tx(char *ip_addr_dst, uint32_t udp_port_dst,
                     uint32_t *tp_buff, uint32_t tp_buff_size, uint32_t tp_send_req)
{
    uint32_t  tp_numb;
    uint32_t  tp_indx;
    uint32_t  tp_sent_total;
    uint32_t  loop_cont;
    uint32_t  local_t_offset_msec;


    tp_numb = tp_buff_size / TP_SIZE_BYTE;

    // allocate the time_stamp array
    tx_time_buff = (uint32_t *) malloc(tp_numb * sizeof(uint32_t));
    tx_time_type_buff = (uint8_t *) malloc(tp_numb * sizeof(uint8_t));

    // calculate PCR for each TP
    tp_mark_tx_time(tp_buff, tp_buff_size, tx_time_buff, tx_time_type_buff);
//  tp_dump_tx_time(tx_time_buff, tx_time_type_buff, tp_numb);

    udp_socket_setup(ip_addr_dst, udp_port_dst, udp_port_src);

    loop_cont = 0;
    tp_sent_total = 0;
    tp_indx = 0;
    local_t_offset_msec = 0;

    while (tp_sent_total < tp_send_req) {

        uint32_t  local_t_msec;
        uint32_t  send_t_msec;
        uint32_t  tp_indx_send;
        uint32_t  tp_send_cont;

        loop_cont++;

        local_t_msec = get_local_time_in_msec();
        if (tp_indx == 0) {
            // calibrate the time base between the stream and
            // the host local time
            local_t_offset_msec = tx_time_buff[0] - local_t_msec;
        }
        send_t_msec = local_t_msec + local_t_offset_msec;

        tp_indx_send = tp_indx;
        tp_send_cont = 0;

        while (1) {

            if (((int32_t) (tx_time_buff[tp_indx] - send_t_msec)) > 0) {
                // when TP_pcr > send_t_msec
                break;
            }
            tp_send_cont++;

            tp_indx++;
            if (tp_indx >= tp_numb) {
                // stream wrap around
                tp_indx = 0;
                break;
            }
        }

        // send the packet
//	    printf("%6d: lcl: %10u - snd: %10u   tp: %10u\n",
//	           loop_cont, local_t_msec, send_t_msec, tp_send_cont);
        udp_send_mpeg(tp_buff, tp_indx_send, tp_send_cont);

        tp_sent_total += tp_send_cont;
        usleep(SLEEP_PERIOD_USEC);
    }

}


// *****************************************************************************
// *****************************************************************************
// *****************************************************************************

void tp_mark_tx_time (uint32_t *tp_buff, uint32_t tp_buff_size,
                      uint32_t *tx_time, uint8_t *tx_time_type)
{
    uint32_t  *tp_p, tp_numb;
    uint32_t   pcr_45k, pcr_ext;
    uint32_t   pcr_msec, pcr_msec_prev;
    int32_t    pcr_diff_msec, pcr_offset_msec;
    int32_t    tp_time_gap_usec, tp_time_gap_usec_1st;
    uint32_t   tp_time_gap_usec_1st_flag;
    uint32_t   tp_indx, tp_indx_prev_pcr;
    int32_t    tp_cont;
    int32_t    m, rt;

    tp_p    = tp_buff;
    tp_numb = tp_buff_size / TP_SIZE_BYTE;

    tp_time_gap_usec = 0;
    tp_time_gap_usec_1st_flag = 0;
    tp_indx_prev_pcr = (uint32_t) -1;

    for (tp_indx = 0; tp_indx < tp_numb; ++tp_indx) {

        rt = tp_get_pcr(tp_p, &pcr_45k, &pcr_ext);      

        if (rt) {
            // TP has PCR

            pcr_msec = tp_pcr_2_msec(pcr_45k, pcr_ext);
            tx_time[tp_indx] = pcr_msec;	
            tx_time_type[tp_indx] = (rt << 1);   // PCR time stamp

            if (tp_indx_prev_pcr != (uint32_t) -1) {
                // Have previous PCR as reference

                if (rt == 1) {
                    // PCR discontinuity_indicator flag is NOT set
                    pcr_diff_msec = (int32_t) (pcr_msec - pcr_msec_prev);

                    // do time-interpolation on previous TP
                    tp_cont = (int32_t) (tp_indx - tp_indx_prev_pcr);
                    for (m = (tp_indx_prev_pcr + 1); m < tp_indx; ++m) {
                        pcr_offset_msec = (pcr_diff_msec * (m - tp_indx_prev_pcr)) / tp_cont;
                        tx_time[m] = pcr_msec_prev + pcr_offset_msec;
                        tx_time_type[m] = 1;   // estimated time stamp
                    }

                    // for extrapolation
                    tp_time_gap_usec = (pcr_diff_msec * 1000) / (tp_indx - tp_indx_prev_pcr);
                    if (tp_time_gap_usec_1st_flag == 0) {
                        tp_time_gap_usec_1st = tp_time_gap_usec;
                        tp_time_gap_usec_1st_flag = 1;
                    }
                }
            }

            tp_indx_prev_pcr = tp_indx;
            pcr_msec_prev = pcr_msec;

        } else {
            // TP has no PCR

            if (tp_indx_prev_pcr != (uint32_t) -1) {
                // Have previous PCR as reference

                // do time-extrapolation on previous TP
                pcr_offset_msec = (tp_time_gap_usec * (tp_indx - tp_indx_prev_pcr)) / 1000;
                tx_time[tp_indx] = pcr_msec_prev + pcr_offset_msec;
                tx_time_type[tp_indx] = 1;   // estimated time stamp

            } else {
                // No reference
                tx_time[tp_indx] = 0;	
                tx_time_type[tp_indx] = 0;   // NO time stamp
            }
        }

        tp_p += TP_SIZE_WORD;
    }

    // need to handle the tx timing of TP before the very 1st PCR
    for (tp_indx = 0; tp_indx < tp_numb; ++tp_indx) {
        if (tx_time_type[tp_indx] == 2 ||
            tx_time_type[tp_indx] == 4)  {

            pcr_msec = tx_time[tp_indx];

            for (m = (tp_indx - 1); m >= 0; --m) {
                pcr_offset_msec = (tp_time_gap_usec_1st * (tp_indx - m)) / 1000;
                tx_time[m] = pcr_msec - pcr_offset_msec;
                tx_time_type[m] = 1;   // estimated time stamp
            }
            break;
        }
    }
}


void tp_dump_tx_time(uint32_t *tx_time, uint8_t *tx_time_type, uint32_t tp_numb)
{
    uint32_t  n;

    for (n = 0; n < tp_numb; ++n) {
        printf("%6d  %1d  %d\n", n, tx_time_type[n], tx_time[n]);
    }
}


// *****************************************************************************
// *****************************************************************************
// *****************************************************************************

uint32_t get_local_time_in_msec(void)
{
    struct timespec sys_time;
    uint64_t  sys_time_nsec;
    uint32_t  sys_time_msec;

    if (clock_gettime(CLOCK_REALTIME, &sys_time) < 0) {
        printf("Fail to get system time\n");
        sys_time_msec = 0;

    } else {
        sys_time_nsec = ((uint64_t) sys_time.tv_sec * 1000000000LL + 
                         (uint64_t) sys_time.tv_nsec);
        sys_time_msec = (uint32_t) ((sys_time_nsec / 1000000LL) & 0xFFFFFFFF);
#if 0
        printf("system time is\n");
        printf("    tv_sec : %lu\n", sys_time.tv_sec);
        printf("    tv_nsec: %lu\n", sys_time.tv_nsec);
        printf("    msec   : %u\n",  sys_time_msec);
#endif
    }
    return(sys_time_msec);
}



// *****************************************************************************
// *****************************************************************************
// *****************************************************************************

void tp_track_bit_rate(uint32_t local_t_msec, uint32_t tp_send_numb)
{
}
