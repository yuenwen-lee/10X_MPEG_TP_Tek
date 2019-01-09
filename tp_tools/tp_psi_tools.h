#ifndef _TP_PSI_TOOLS_H_
#define _TP_PSI_TOOLS_H_




//###############################################################//
//
// PSI section buff tools
//
//###############################################################//

#define PSI_SEC_TABLE_ID_PAT    0x0
#define PSI_SEC_TABLE_ID_CAT    0x1
#define PSI_SEC_TABLE_ID_PMT    0x2



typedef enum psi_sec_stat_ {
    BUFF_NOT_READY = 0,        // section buff is not ready
    BUFF_READY,                // section buff is ready
    BUFF_READY_WITH_NEXT_SEC   // section buff is ready, but is
                               //   followed by next section 
} psi_sec_stat_t;


typedef struct psi_sec_buff_ {
    uint32_t   sec_ready;
    uint32_t   sec_size;
    uint32_t   buff_wr_indx;
    uint8_t    buff[1024];
} psi_sec_buff_t;



void tp_psi_sec_buff_init(psi_sec_buff_t *psi_sec_buff_p);
psi_sec_stat_t tp_psi_sec_buff_save(uint8_t *tp_p, tp_head_info_t *tp_head_info_p,
                                    psi_sec_buff_t *psi_sec_buff_p);

uint8_t tp_psi_sec_buff_get_table_id(psi_sec_buff_t *psi_sec_buff_p);
uint32_t tp_psi_sec_buff_get_section_length(psi_sec_buff_t *psi_sec_buff_p);
uint32_t tp_psi_sec_buff_get_crc(psi_sec_buff_t *psi_sec_buff_p);

void tp_psi_sec_buff_dump(psi_sec_buff_t *psi_sec_buff_p);

uint32_t tp_psi_debug(uint32_t pid_req, uint32_t *tp_buff_p, uint32_t tp_numb);





//###############################################################//
//
// PAT tools
//
//###############################################################//

#define TP_PSI_PAT_PROGRAM_NUMB_MAX     32



typedef enum psi_pat_stat_ {
    BAD_PAT_SEC = -1,
    NOT_READY = 0,        // PAT is not ready
    PAT_NO_CHANGE,        // PAT has no change
    PAT_CHANGE_VER,       // PAT new version
    PAT_CHANGE_ADD_PMT,   // PAT add new PMT
    PAT_CHANGE_OTHER,     // PAT has new content
} psi_pat_stat_t;


#if 0
typedef enum pat_stat_ {
    NOT_READY = 0,        // PAT is not ready
    READY,                // PAT is ready, get all section
} pat_stat_t;
#endif


typedef struct pat_prog_info_ {
    uint16_t   program_number;   // 16 bits
    uint16_t   reserved:3;       //  3 bits
    uint16_t   pmt_pid:13;       // 13 bits
} pat_prog_info_t;


typedef struct pat_info_ {
#if 0
    // ***************************************************** //
    //                None-spec info                         //
    // ***************************************************** //
    pat_stat_t pat_stat;
    uint32_t   sec_total;
    uint32_t   sec_rx;
#endif

    // ***************************************************** //
    //                PSI table info                         //
    // ***************************************************** //
    uint8_t    table_id;                    //  8 bits     ( 0)

    uint16_t   section_syntax_indicator:1;  //  1 bit      ( 1)
    uint16_t   stuff_0:1;                   //  1 bit
    uint16_t   reserved_1:2;                //  2 bits
    uint16_t   section_length:12;           // 12 bits

    uint16_t   transport_stream_id;         // 16 bits     ( 3)

    uint8_t    reserved_2:2;                //  2 bits     ( 5)
    uint8_t    version_number:5;            //  5 bits
    uint8_t    current_next_indicator:1;    //  1 bit

    uint8_t    section_number;              //  8 bits     ( 6)

    uint8_t    last_section_number;         //  8 bits     ( 7)

    pat_prog_info_t  pat_prog_info[TP_PSI_PAT_PROGRAM_NUMB_MAX];  // ( 8)

    uint32_t   crc32;
} pat_info_t;



//###############################################################//
//
// PMT tools
//
//###############################################################//

#define PMT_ES_NUMB_MAX                    16 // max number of ES in program
#define PMT_PROG_INFO_DESCRP             1024 // descriptors after program_info_length 
#define PMT_ES_INFO_DESCRP               1024 // descriptors after ES_info_length 




typedef enum pmt_stream_type_ {
    ISO_Reserved = 0x00,                      // 0x00  Reserved
    ISO_11172_2_VIDEO,                        // 0x01  MPEG-1 video
    ISO_13818_2_VIDEO,                        // 0x02  MPEG-2 video
    ISO_11172_3_AUDIO,                        // 0x03  MPEG-1 audio
    ISO_13818_3_AUDIO,                        // 0x04  MPEG-2 audio
    ISO_13818_1_PRIVATE_SECTIONS,             // 0x05  
    ISO_13818_1_PES_PACKET_CONTAINING_PRIVATE_DATA,    // 0x06
    ISO_13522_MHEG,                           // 0x07  ISO/IEC 13522 MHEG
    ISO_13818_1_ANNEX_A_DSM_CC,               // 0x08  Annex A DSM-CC
    ITU_T_REC_H_222_1                         // 0x09  
} pmt_stream_type_t;


typedef struct pmt_es_info_ {
    uint8_t    stream_type;                 //  8 bits    ( 0)

    uint16_t   reserved_0:3;                //  3 bits    ( 1)
    uint16_t   elementary_PID:13;           // 13 bits

    uint16_t   reserved_1:4;                //  4 bits    ( 2)
    uint16_t   ES_info_length:12;           // 12 bits

    uint8_t    descriptor[PMT_PROG_INFO_DESCRP];
                                            // 8L bits    (11)
} pmt_es_info_t;


typedef struct pmt_info_ {
    uint8_t    table_id;                    //  8 bits    ( 0)

    uint16_t   section_syntax_indicator:1;  //  1 bit     ( 1)
    uint16_t   stuff_0:1;                   //  1 bit
    uint16_t   reserved_0:2;                //  2 bits
    uint16_t   section_length:12;           // 12 bits

    uint16_t   program_number;              // 16 bits    ( 3)

    uint8_t    reserved_1:2;                //  2 bits    ( 5)
    uint8_t    version_number:5;            //  5 bits
    uint8_t    current_next_indicator:1;    //  1 bits

    uint8_t    section_number;              //  8 bits    ( 6)

    uint8_t    last_section_number;         //  8 bits    ( 7)

    uint16_t   reserved_2:3;                //  3 bits    ( 8)
    uint16_t   pcr_PID:13;                  // 13 bits

    uint16_t   reserved_3:4;                //  4 bits    (10)
    uint16_t   program_info_length:12;      // 12 bits

    uint8_t    descriptor[PMT_PROG_INFO_DESCRP];
                                            // 8N bits    (11)

    pmt_es_info_t  pmt_es_info[PMT_ES_NUMB_MAX];
                                            // 40M bits   (11 + N)
    uint32_t   CRC_32;
} pmt_info_t;





#endif // _TP_PSI_TOOLS_H_
