#ifndef _TP_HEAD_TOOLS_H_
#define _TP_HEAD_TOOLS_H_



#define TP_SIZE_BYTE	             188
#define TP_SIZE_WORD                 (TP_SIZE_BYTE / sizeof(uint32_t))

#define TP_HEAD_SIZE_BYTE            4  // TP header is 4 Byte long

#define TP_SYNC_BYTE                 0x47

#define TP_HEAD_SYNC_BIT_LOC        24  // SYNC byte
#define TP_HEAD_TRANS_ERR_BIT_LOC   23  // transport error indicator
#define TP_HEAD_PYLD_START_BIT_LOC  22  // payload unit start indicator
#define TP_HEAD_TRANS_PRI_BIT_LOC   21  // transport priority
#define TP_HEAD_PID_BIT_LOC          8  // start from 8 (LSB is 0)
#define TP_HEAD_SCRMB_CTRL_BIT_LOC   6  // transport scrambling control
#define TP_HEAD_AF_CTRL_BIT_LOC      4  // adaptation field control
#define TP_HEAD_CC_BIT_LOC           0  // continuity counter

#define TP_HEAD_SYNC_BIT_NUMB        8  // SYNC byte
#define TP_HEAD_TRANS_ERR_BIT_NUMB   1  // transport error indicator
#define TP_HEAD_PYLD_START_BIT_NUMB  1  // payload unit start indicator
#define TP_HEAD_TRANS_PRI_BIT_NUMB   1  // transport priority
#define TP_HEAD_PID_BIT_NUMB        13  // start from 8 (LSB is 0)
#define TP_HEAD_SCRMB_CTRL_BIT_NUMB  2  // transport scrambling control
#define TP_HEAD_AF_CTRL_BIT_NUMB     2  // adaptation field control
#define TP_HEAD_CC_BIT_NUMB          4  // continuity counter

#define TP_PID_NULL_TP               0x1FFF  // 8191




typedef struct tp_head_info_ {
    uint32_t sync_byte                     :8;  //  8 bits, 0x47
    uint32_t transport_error_indicator     :1;  //  1 bit
    uint32_t payload_unit_start_indicator  :1;  //  1 bit
    uint32_t transport_priority            :1;  //  1 bit
    uint32_t pid                           :13; // 13 bit
    uint32_t transport_scrambling_control  :2;  //  2 bits
    uint32_t adaptation_field_control      :2;  //  2 bits
    uint32_t continuity_counter            :4;  //  4 bits
} tp_head_info_t;



int32_t tp_search_sync(uint32_t *buff_p, uint32_t buff_size_byte);

void tp_head_info_get(uint32_t *tp_p, tp_head_info_t *tp_hd_info_p);
void tp_head_info_dump(tp_head_info_t *tp_head_info_p);

uint32_t tp_head_payload_loc(uint32_t *tp_p, tp_head_info_t *tp_head_info_p);

void get_pid_stat(uint32_t *tp_buff_p, uint32_t tp_numb);



static inline uint32_t get_bit_field (uint32_t data, uint32_t loc, uint32_t bit_numb)
{
    assert(loc < 31);
    assert((loc + bit_numb) <= 32);
    return ((data << (32 - loc - bit_numb)) >> (32 - bit_numb));
}



#endif // _TP_HEAD_TOOLS_H_
