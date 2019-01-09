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



#define PID_NUM_MAX   100



int32_t pid_list[PID_NUM_MAX];




int read_tek_ts_file(char *path, char **buff_p, uint32_t *size_p, char **t_stamp_p);
void tp_dump_tp_diag(uint32_t *tp_buff, uint32_t tp_buff_size, uint32_t *tek_t_stamp,
                     int32_t  *target_pid_p, uint32_t target_pid_numb);



int main(int argc, char *argv[])
{
    uint32_t  *buff, buff_size;
    uint32_t  *tp_buff, tp_buff_size;
    uint32_t  *tek_t_stamp;      // Tektronics Extra Time Stamp
    uint32_t   tp_numb;
    int32_t    tp_loc_byte;

    char      *f_name;
    uint32_t   n, pid_num;


    if (argc < 3) {
        printf("usuage: %s f_name pid_1 [pid_2, pid_3, ...]\n", argv[0]);
        printf("    f_name  : file name\n");
        printf("    pid_0   : target PID 0\n");
        printf("    pid_1 ..: extra target PID (optional)\n");
        exit(1);
    }

    pid_num = argc - 2;
    if (pid_num > PID_NUM_MAX) {
        pid_num = PID_NUM_MAX;
    }

    f_name = argv[1];
    for (n = 0; n < pid_num; ++n) {
        pid_list[n] = atoi(argv[n + 2]);
    }

    printf("  f_name   : %s\n", f_name);
    printf("  pid list :");
    for (n = 0; n < pid_num; ++n) {
        printf(" %d", pid_list[n]);
    }
    printf("\n");
    printf("  Total %d pid\n", pid_num);
    printf("\n");

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

    tp_dump_tp_diag(tp_buff, tp_buff_size, (uint32_t *) tek_t_stamp, &pid_list[0], pid_num);

    exit(0);
}


// *****************************************************************************
// *****************************************************************************
// *****************************************************************************

void tp_dump_tp_diag(uint32_t *tp_buff, uint32_t tp_buff_size, uint32_t *tek_t_stamp,
                     int32_t  *target_pid_p, uint32_t target_pid_numb)
{
    uint32_t        *tp_p, tp_numb;
    uint32_t         tp_indx;
    tp_head_info_t   tp_hd_info;
    tp_diag_t        tp_diag;
    uint32_t         n;

    uint32_t         pcr_45k, pcr_ext, pcr_flag;


    tp_p    = tp_buff;
    tp_numb = tp_buff_size / TP_SIZE_BYTE;

    for (tp_indx = 0; tp_indx < tp_numb; ++tp_indx) {

        tp_head_info_get(tp_p, &tp_hd_info);
        for (n = 0; n < target_pid_numb; ++n) {
            if (tp_hd_info.pid == target_pid_p[n])
                break;
        }

        if (n >= target_pid_numb || tp_hd_info.pid == 0x1FFF) {
            tp_p += TP_SIZE_WORD;
            continue;
        }

        pcr_flag = tp_get_pcr(tp_p, &pcr_45k, &pcr_ext);
        if (pcr_flag == 0) {
            pcr_45k = pcr_ext = 0;
        }

        printf("%10u ", tp_tek_t_stamp_get(tek_t_stamp, tp_indx));

        printf("%7d  %5d  %1d ", tp_indx, tp_hd_info.pid, pcr_flag);
//      printf("%lld  ", ((uint64_t) pcr_45k * 600) + (uint64_t) pcr_ext);

        tp_2_tp_diag(tp_p, &tp_diag);
        tp_dec_dump_jib_command_t_tbo(&tp_diag.jib_cmd);
        printf("%10u ", tp_diag.jib_cmd.out_time);
        printf("%10u ", tp_diag.time & ((1 << 20) - 1));
#if 0
        tp_hex_dump_tp_diag(&tp_diag);
        printf("\n");
#endif

        printf("\n");

        tp_p += TP_SIZE_WORD;
    }
}



