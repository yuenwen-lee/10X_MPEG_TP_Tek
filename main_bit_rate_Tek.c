#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include "tp_tools/tp_head_tools.h"
#include "tp_tools/tp_af_tools.h"
#include "tp_tools/tp_tek_t_stamp.h"
#include "tp_tools/ng_video_diag.h"




int read_tek_ts_file(char *path, char **buff_p, uint32_t *size_p, char **t_stamp_p);
void tp_dump_bit_rate_between_pcr_Tek(uint32_t *tp_buff, uint32_t tp_buff_size, int32_t pid, uint32_t flag,
                                      uint32_t *tek_t_stamp);



int main(int argc, char *argv[])
{
    char      *f_name;
    int32_t    pid_pcr = -1;
    uint32_t   flag_exclude_NULL = 0;

    uint32_t  *buff, *tp_buff;
    uint32_t  *tek_t_stamp;
    uint32_t   buff_size, tp_buff_size;
    uint32_t   tp_numb;
    int32_t    tp_loc_byte;

    if (argc == 1) {
        printf("usuage: %s f_name [pcr_pid] [flag_null]\n", argv[0]);
        printf("  f_name   : file name\n");
        printf("  pcr_pid  : specify the PCR [defalut to use any PCR in the file]\n");
        printf("  flag_null: 0 include NULL, 1 exclude NULL [default include NULL]\n");
        exit(1);
    }

    f_name = argv[1];
    if (argc >= 3) {
        pid_pcr = atoi(argv[2]);
    }
    if (argc >= 4) {
        flag_exclude_NULL = atoi(argv[3]);
    }
    printf("  f_name   : %s\n", f_name);
    printf("  pcr_pid  : %d\n", pid_pcr);
    printf("  flag_NULL: %d\n", flag_exclude_NULL);
    printf("\n");

    if (read_tek_ts_file(f_name,
                         (char **) &buff, &buff_size,
                         (char **) &tek_t_stamp) < 0) {
        printf("can not open file %s\n", f_name);
        exit(-1);
    }

    tp_loc_byte = tp_search_sync(buff, buff_size);
    printf("Find the location %d (in byte)\n", tp_loc_byte);

    tp_buff = buff + tp_loc_byte/sizeof(uint32_t);
    tp_buff_size = buff_size - tp_loc_byte;
    tp_numb = tp_buff_size / TP_SIZE_BYTE;

    printf("\n\n");
    tp_dump_bit_rate_between_pcr_Tek(tp_buff, tp_buff_size, pid_pcr, flag_exclude_NULL,
                                     (uint32_t *) tek_t_stamp);

    exit(0);
}



void tp_dump_bit_rate_between_pcr_Tek(uint32_t *tp_buff, uint32_t tp_buff_size,
                                      int32_t target_pcr_pid, uint32_t flag_exclude_NULL,
                                      uint32_t *tek_t_stamp)
{
    uint32_t  *tp_p, tp_numb;
    uint32_t   pcr_45k, pcr_ext;
    uint64_t   pcr_27m, pcr_27m_prev;
    uint32_t   time_diff_usec, pcr_27m_diff;
    uint32_t   tp_indx, tp_indx_prev;
    uint32_t   hour, min, sec, msec, usec;
    uint32_t   tp_cont, bit_rate;
    tp_head_info_t tp_hd_info;

    uint32_t   rt;

    tp_p    = tp_buff;
    tp_numb = tp_buff_size / TP_SIZE_BYTE;
    pcr_27m_prev = (uint64_t) -1;

    tp_cont = 0;
    for (tp_indx = 0; tp_indx < tp_numb; ++tp_indx) {

        tp_head_info_get(tp_p, &tp_hd_info);
        if (flag_exclude_NULL) {
            // exclude NULL packet
            if (tp_hd_info.pid != 0x1FFF)
                tp_cont++;
        } else {
            // include NULL packet
            tp_cont++;
        }

        if (target_pcr_pid > 0) {
            if (tp_hd_info.pid != target_pcr_pid) {
                tp_p += TP_SIZE_WORD;
                continue;
            }
        }

        rt = tp_get_pcr(tp_p, &pcr_45k, &pcr_ext);
        if (rt) {

            uint32_t msec_ref;

            tp_pcr_2_time(pcr_45k, pcr_ext,
                          &hour, &min, &sec, &msec, &usec);
            msec_ref = tp_pcr_2_msec(pcr_45k, pcr_ext);
            printf("-%1d-  %8u: %02d:%02d:%02d.%03d.%03d, (%u, %3u), %u msec",
                   rt, tp_indx, hour, min, sec, msec, usec, pcr_45k, pcr_ext, msec_ref);

            pcr_27m = (uint64_t) pcr_45k * 600LL + (uint64_t) pcr_ext;

            if (pcr_27m_prev < (uint64_t) -1) {
                pcr_27m_diff = (uint32_t) (pcr_27m - pcr_27m_prev);
                time_diff_usec = pcr_27m_diff / 27;
                bit_rate = ((tp_cont * TP_SIZE_BYTE * 8) * 1000000LL) / time_diff_usec;
                printf(", %5d usec, tp_cont %3d, %8u bps", time_diff_usec, tp_cont, bit_rate);
            }

            printf(", %u,", tp_tek_t_stamp_get(tek_t_stamp, tp_indx));

            printf("  ");

            pcr_27m_prev = pcr_27m;
            tp_indx_prev = tp_indx;

            // zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz //
            {
                tp_diag_t tp_diag;

                tp_2_tp_diag(tp_p, &tp_diag);
                tp_dec_dump_jib_command_t_tbo(&tp_diag.jib_cmd);
                printf("\n");
//              tp_hex_dump_tp_diag(&tp_diag);
//              printf("\n");
            }

            tp_cont = 0;
        }

        tp_p += TP_SIZE_WORD;
    }
}
