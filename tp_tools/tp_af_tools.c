#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <arpa/inet.h>
#include <string.h>

#include "tp_head_tools.h"
#include "tp_af_tools.h"



void tp_af_info_get(uint32_t *tp_p, tp_af_info_t *tp_af_info_p)
{
#define TP_AF_DATA_SIZE_BYTE   16
    uint8_t  *af_data_p;
    uint8_t   af_flag;
    uint32_t  n;

    memset((void *) tp_af_info_p, 0, sizeof(tp_af_info_t));

    af_data_p = ((uint8_t *) tp_p) + TP_HEAD_SIZE_BYTE;

    // AF length
    tp_af_info_p->adaptation_field_length = *af_data_p++;
    if (tp_af_info_p->adaptation_field_length == 0)
        return;

    // AF Flags
    af_flag = *af_data_p++;
    tp_af_info_p->discontinuity_indicator              = ((af_flag & TP_AF_DISCON_IND_MASK)           != 0);
    tp_af_info_p->random_access_indicator              = ((af_flag & TP_AF_RAND_ACCES_IND_MASK)       != 0);
    tp_af_info_p->elementary_stream_priority_indicator = ((af_flag & TP_AF_ES_PRIO_IND_MASK)          != 0);
    tp_af_info_p->PCR_flag                             = ((af_flag & TP_AF_PCR_FLAG_MASK)             != 0);
    tp_af_info_p->OPCR_flag                            = ((af_flag & TP_AF_OPCR_FLAG_MASK)            != 0);
    tp_af_info_p->splicing_point_flag                  = ((af_flag & TP_AF_SPLICE_PT_FLAG_MASK)       != 0);
    tp_af_info_p->transport_private_data_flag          = ((af_flag & TP_AF_TRANS_PRIV_DATA_FLAG_MASK) != 0);
    tp_af_info_p->adaptation_field_extension_flag      = ((af_flag & TP_AF_AF_EXT_FLAG_MASK)          != 0);

    // PCR
    if (tp_af_info_p->PCR_flag) {
        for (n = 0; n < 6; ++n) {
            tp_af_info_p->pcr_data[n] = *af_data_p++;
        }
    }

    // OPCR
    if (tp_af_info_p->OPCR_flag) {
        for (n = 0; n < 6; ++n) {
            tp_af_info_p->opcr_data[n] = *af_data_p++;
        }
    }

    // Splice Countdown
    if (tp_af_info_p->transport_private_data_flag) {
        tp_af_info_p->splice_countdown = *af_data_p++;
    }
}



//
//  pcr_45k: 32 bit in 45 KHz
//  pcr_ext: 10 bit in 27 MHz (value between 0 ~ 599)
//  Return : 0 --> no PCR
//           1 --> PCR
//           2 --> PCR + discontinuity_indicator set
uint32_t tp_get_pcr(uint32_t *tp_p, uint32_t *pcr_45k_p, uint32_t *pcr_ext_p)
{
    uint32_t  head;
    uint32_t  adpt_filed_1, adpt_filed_2;
    uint32_t  adpt_ctrl, adpt_len;
    uint32_t  pcr_flag, dsc_flag;
    uint32_t  pcr_45k, pcr_ext;

    head = ntohl(tp_p[0]);
    adpt_ctrl = get_bit_field(head, 4, 2);

    if (adpt_ctrl == 0x2 || adpt_ctrl == 0x3) {
        adpt_filed_1 = ntohl(tp_p[1]);
        adpt_len = get_bit_field(adpt_filed_1, 24, 8);

        if (adpt_len) {
            // bit 20 is the PCR flag
            pcr_flag = get_bit_field(adpt_filed_1, 20, 1);

            if (pcr_flag) {
                adpt_filed_2 = ntohl(tp_p[2]);	     /** bytes 8-11 of TP **/

                // PCR 45 KHz (32 bits)
                pcr_45k = get_bit_field(adpt_filed_1,  0, 16);
                pcr_45k = (pcr_45k << 16) | get_bit_field(adpt_filed_2, 16, 16);
                // PCR 27 MHz (10 bits, 0 ~ 599 tick)
                pcr_ext = 300 * get_bit_field(adpt_filed_2, 15, 1); 
                pcr_ext = pcr_ext	+ get_bit_field(adpt_filed_2, 0, 9);

                *pcr_45k_p = pcr_45k;
                *pcr_ext_p = pcr_ext;

                // bit 23 is the discontinuity_indicator flag
                dsc_flag = get_bit_field(adpt_filed_1, 23, 1);
                if (dsc_flag) {
                    return 2;  // discontinuity_indicator flag is 1
                } else {
                    return 1;  // discontinuity_indicator flag is 0
                }
            }
        }
    }

    *pcr_45k_p = 0;
    *pcr_ext_p = 0;
    return 0;  // no PCR
}


void tp_pcr_2_time(uint32_t pcr_45k, uint32_t pcr_ext,
                   uint32_t *hour_p, uint32_t *min_p, uint32_t *sec_p,
                   uint32_t *msec_p, uint32_t *usec_p)
{
    uint64_t  tick_27_mhz;
    uint64_t  usec, msec, sec, min, hour;

    tick_27_mhz = (uint64_t) pcr_45k * 600LL + (uint64_t) pcr_ext;

    usec = tick_27_mhz / 27;

    msec = usec / 1000;
    usec = usec % 1000;

    sec  = msec / 1000;
    msec = msec % 1000;

    min  = sec / 60;
    sec  = sec % 60;

    hour = min / 60;
    min  = min % 60;

    *usec_p  = (uint32_t) usec;
    *msec_p  = (uint32_t) msec;
    *sec_p   = (uint32_t) sec;
    *min_p   = (uint32_t) min;
    *hour_p  = (uint32_t) hour;
}


uint32_t tp_pcr_2_msec(uint32_t pcr_45k, uint32_t pcr_ext)
{
    uint64_t  tick_27_mhz;
    uint64_t  msec;

    tick_27_mhz = (uint64_t) pcr_45k * 600LL + (uint64_t) pcr_ext;

    msec = (uint32_t) (tick_27_mhz / 27000LL);
    return msec;
}
