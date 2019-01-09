#ifndef _TP_AF_TOOLS_H_
#define _TP_AF_TOOLS_H_



#define TP_SIZE_BYTE	                 188
#define TP_SIZE_WORD                     (TP_SIZE_BYTE / sizeof(uint32_t))
#define TP_HEAD_SIZE_BYTE                4     // TP header is 4 Byte long

#define  TP_AF_DISCON_IND_MASK           0x80  // discontinuity indicator
#define  TP_AF_RAND_ACCES_IND_MASK       0x40  // random_access indicator
#define  TP_AF_ES_PRIO_IND_MASK          0x20  // elementary_stream priority indicator
#define  TP_AF_PCR_FLAG_MASK             0x10  // PCR flag
#define  TP_AF_OPCR_FLAG_MASK            0x08  // OPCR flag
#define  TP_AF_SPLICE_PT_FLAG_MASK       0x04  // splicing point flag
#define  TP_AF_TRANS_PRIV_DATA_FLAG_MASK 0x02  // transport private data flag
#define  TP_AF_AF_EXT_FLAG_MASK          0x01  // adaptation field extension flag



typedef struct tp_af_info_ {
    uint8_t  adaptation_field_length;
    uint8_t  discontinuity_indicator:1;
    uint8_t  random_access_indicator:1;
    uint8_t  elementary_stream_priority_indicator:1;
    uint8_t  PCR_flag:1;
    uint8_t  OPCR_flag:1;
    uint8_t  splicing_point_flag:1;
    uint8_t  transport_private_data_flag:1;
    uint8_t  adaptation_field_extension_flag:1;
    uint8_t  pcr_data[6];       // valid if PCR_flag set
    uint8_t  opcr_data[6];      // valid if OPCR_flag set
    uint8_t  splice_countdown;  // valid if splicing_point_flag set
} tp_af_info_t;



void tp_af_info_get(uint32_t *tp_p, tp_af_info_t *tp_af_info_p);

uint32_t tp_get_pcr(uint32_t *tp_p, uint32_t *pcr_45k_p, uint32_t *pcr_ext_p);
void tp_pcr_2_time(uint32_t pcr_45k, uint32_t pcr_ext,
                   uint32_t *hour_p, uint32_t *min_p, uint32_t *sec_p,
                   uint32_t *msec_p, uint32_t *usec_p);
uint32_t tp_pcr_2_msec(uint32_t pcr_45k, uint32_t pcr_ext);



#endif // _TP_AF_TOOLS_H_

