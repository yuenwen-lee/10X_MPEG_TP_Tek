#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>

#include "tp_head_tools.h"
#include "tp_psi_tools.h"




//###############################################################//
//
// PSI section buff tools
//
//###############################################################//

void tp_psi_sec_buff_init(psi_sec_buff_t *psi_sec_buff_p)
{
    memset((void *) psi_sec_buff_p, 0, sizeof(psi_sec_buff_t));
}


psi_sec_stat_t tp_psi_sec_buff_save(uint8_t *tp_p, tp_head_info_t *tp_head_info_p,
                                    psi_sec_buff_t *psi_sec_buff_p)
{
    uint32_t   pyld_loc;
    uint8_t   *data_p;
    uint32_t   data_size;
    uint32_t   next_sec_flag;
    uint32_t   n;

    // get the location of the TP payload
    pyld_loc = tp_head_payload_loc((uint32_t *) tp_p, tp_head_info_p);
    data_p = tp_p + pyld_loc;

    next_sec_flag = 0;

    if (psi_sec_buff_p->buff_wr_indx == 0) {
        // waiting for the new PSI section

        if (tp_head_info_p->payload_unit_start_indicator == 0) {
            // no new PSI section in this TP
            return BUFF_NOT_READY;
        }
        data_p += (1 + tp_p[pyld_loc]);

    } else {
        // already start getting PSI sec info in previous TP

        if (tp_head_info_p->payload_unit_start_indicator) {
            data_p++;
            next_sec_flag = 1;
        }
    }

    data_size = (uint32_t) ((tp_p + TP_SIZE_BYTE) - data_p);

    // copy the data after pointer_field into pat section buffer
    for (n = 0; n < data_size; ++n) {

        psi_sec_buff_p->buff[psi_sec_buff_p->buff_wr_indx] = data_p[n];
        psi_sec_buff_p->buff_wr_indx++;

        if (psi_sec_buff_p->buff_wr_indx == 3) {
            // section size is now available after got 3 bytes
            psi_sec_buff_p->sec_size = ((psi_sec_buff_p->buff[1] << 8) |
                                        psi_sec_buff_p->buff[2]) & 0xFFF;
        }

        if (psi_sec_buff_p->buff_wr_indx >= (psi_sec_buff_p->sec_size + 3)) {
            // we got the complete PSI section in buffer
            psi_sec_buff_p->sec_ready = 1;
            if (next_sec_flag)
                return BUFF_READY_WITH_NEXT_SEC;
            else
                return BUFF_READY;
        }
    }

    return BUFF_NOT_READY;
}


uint8_t tp_psi_sec_buff_get_table_id(psi_sec_buff_t *psi_sec_buff_p)
{
    return psi_sec_buff_p->buff[0];
}


uint32_t tp_psi_sec_buff_get_section_length(psi_sec_buff_t *psi_sec_buff_p)
{
    return (((psi_sec_buff_p->buff[1] << 8) | (psi_sec_buff_p->buff[2] << 0)) & 0xFFF);
}


uint32_t tp_psi_sec_buff_get_crc(psi_sec_buff_t *psi_sec_buff_p)
{
    uint32_t sec_len;
    uint32_t crc_loc;

    sec_len = tp_psi_sec_buff_get_section_length(psi_sec_buff_p);
    crc_loc = 3 + sec_len - 4;
    return ((psi_sec_buff_p->buff[crc_loc + 0] << 24) |
            (psi_sec_buff_p->buff[crc_loc + 1] << 16) |
            (psi_sec_buff_p->buff[crc_loc + 2] <<  8) |
            (psi_sec_buff_p->buff[crc_loc + 3] <<  0));
}


void tp_psi_sec_buff_dump(psi_sec_buff_t *psi_sec_buff_p)
{
    uint8_t  *sec_data_p;
    uint32_t  sec_len;
    int32_t   n;

    sec_len = tp_psi_sec_buff_get_section_length(psi_sec_buff_p);

    printf("PSI sec buff\n");
    printf("  table_id:                 %d\n", tp_psi_sec_buff_get_table_id(psi_sec_buff_p));
    printf("  section_syntax_indicator: %d\n", (psi_sec_buff_p->buff[1] >> 7) & 0x1);
    printf("  '0':                      %d\n", (psi_sec_buff_p->buff[1] >> 6) & 0x1);
    printf("  section_length:           %d\n", sec_len);
    printf("  CRC:                      0x%08x\n", tp_psi_sec_buff_get_crc(psi_sec_buff_p));
    printf("  sec_data: ");
    sec_data_p = &psi_sec_buff_p->buff[3];
    for (n = 0; n < (int32_t) (sec_len - 4); ++n) {
        if ((n % 16) == 0) {
            printf("\n      ");
        }
        printf("%02x ", sec_data_p[n]);
    }
    printf("\n\n");
}




//###############################################################//
//
// PAT tools
//
//###############################################################//

void tp_pat_info_init(pat_info_t *pat_info_p)
{
    uint32_t   n;

    memset((void *) pat_info_p, 0, sizeof(pat_info_t));

    for (n = 0; n < TP_PSI_PAT_PROGRAM_NUMB_MAX; ++n) {
        pat_info_p->pat_prog_info[n].program_number = 0;
        pat_info_p->pat_prog_info[n].pmt_pid        = TP_PID_NULL_TP;
    }
}


psi_pat_stat_t tp_pat_info_get(psi_sec_buff_t *psi_sec_buff_p, pat_info_t *pat_info_p)
{
    uint8_t   *buff_p;
    uint8_t    ver_numb;
    uint32_t   pmt_numb, n;

    if (psi_sec_buff_p->sec_ready == 0)
        return NOT_READY;
    buff_p = psi_sec_buff_p->buff;

    // get the version number 1st
    ver_numb = (buff_p[5] >> 1) & 0x1F;
    // check the version numb
    if (ver_numb != pat_info_p->version_number) {
        // new version, do the clean up
        tp_pat_info_init(pat_info_p);
    }

    pat_info_p->table_id               = tp_psi_sec_buff_get_table_id(psi_sec_buff_p);
    pat_info_p->section_length         = tp_psi_sec_buff_get_section_length(psi_sec_buff_p);
    pat_info_p->transport_stream_id    = (buff_p[3] << 8) | buff_p[4];
    pat_info_p->version_number         = (buff_p[5] >> 1) & 0x1F;
    pat_info_p->current_next_indicator = (buff_p[5] >> 0) & 0x01;
    pat_info_p->section_number         = (buff_p[6]);
    pat_info_p->last_section_number    = (buff_p[7]);
    pat_info_p->crc32                  = tp_psi_sec_buff_get_crc(psi_sec_buff_p);

    pmt_numb = (pat_info_p->section_length - 9) / 4;
    for (n = 0; n < pmt_numb; ++n) {
        if (n >= TP_PSI_PAT_PROGRAM_NUMB_MAX)
            break;
        pat_info_p->pat_prog_info[n].program_number = (buff_p[ 8 + 4*n] << 8 | buff_p[ 9 + 4*n]);
        pat_info_p->pat_prog_info[n].pmt_pid        = (buff_p[10 + 4*n] << 8 | buff_p[11 + 4*n]);
    }

    return PAT_NO_CHANGE;
}


void tp_pat_info_dump(pat_info_t *pat_info_p)
{
    uint32_t n;

    printf("PAT section info\n");
    printf("  table_id:               %d\n", pat_info_p->table_id);
    printf("  section_length:         %d\n", pat_info_p->section_length);
    printf("  transport_stream_id:    %d\n", pat_info_p->transport_stream_id);
    printf("  version_number:         %d\n", pat_info_p->version_number);
    printf("  current_next_indicator: %d\n", pat_info_p->current_next_indicator);
    printf("  section_number:         %d\n", pat_info_p->section_number);
    printf("  last_section_number:    %d\n", pat_info_p->last_section_number);
    printf("  crc:                    0x%08x\n", pat_info_p->crc32);
    printf("  PMT table\n");
    for (n = 0; n < TP_PSI_PAT_PROGRAM_NUMB_MAX; ++n) {
        if (pat_info_p->pat_prog_info[n].pmt_pid == TP_PID_NULL_TP)
            break;
        printf("    %3d - %d (0x%04x)\n",
               pat_info_p->pat_prog_info[n].program_number,
               pat_info_p->pat_prog_info[n].pmt_pid,
               pat_info_p->pat_prog_info[n].pmt_pid);
    }
    printf("\n");
}



//###############################################################//
//
// PMT tools
//
//###############################################################//

char *stream_type_name[] = {
    "ITU-T | ISO/IEC Reserved",                               // 0x00
    "ISO/IEC 11172-2 Video",                                  // 0x01
    "ITU-T Rec. H.262 | ISO/IEC 13818-2 Video",               // 0x02
    "ISO/IEC 11172-3 Audio",                                  // 0x03
    "ISO/IEC 13818-3 Audio",                                  // 0x04
    "ITU-T Rec. H.222.0 | ISO/IEC 13818-1 private_sections",  // 0x05
    "ITU-T Rec. H.222.0 | ISO/IEC 13818-1 PES packets containing private data",  // 0x06
    "ISO/IEC 13522 MHEG",                                     // 0x07
    "ITU-T Rec. H.222.0 | ISO/IEC 13818-1 Annex A DSM-CC",    // 0x08
    "ITU-T Rec. H.222.1",                                     // 0x09
    "ISO/IEC 13818-6 type A",                                 // 0x0A
    "ISO/IEC 13818-6 type B",                                 // 0x0B
    "ISO/IEC 13818-6 type C",                                 // 0x0C
    "ISO/IEC 13818-6 type D",                                 // 0x0D
    "ITU-T Rec. H.222.0 | ISO/IEC 13818-1 auxiliary",         // 0x0E
    "ISO/IEC 13818-7 Audio with ADTS transport syntax",       // 0x0F
    "ISO/IEC 14496-2 Visual",                                 // 0x10
    "ISO/IEC 14496-3 Audio with the LATM transport syntax as defined in ISO/IEC 14496-3",
    "ISO/IEC 14496-1 SL-packetized stream or FlexMux stream carried in PES packets",
    "ISO/IEC 14496-1 SL-packetized stream or FlexMux stream carried in ISO/IEC 14496_sections",
    "ISO/IEC 13818-6 Synchronized Download Protocol",         // 0x14
    "Metadata carried in PES packets",                        // 0x15
    "Metadata carried in metadata_sections",                  // 0x16
    "Metadata carried in ISO/IEC 13818-6 Data Carousel",      // 0x17
    "Metadata carried in ISO/IEC 13818-6 Object Carousel",    // 0x18
    "Metadata carried in ISO/IEC 13818-6 Synchronized Download Protocol",
    "IPMP stream (defined in ISO/IEC 13818-11, MPEG-2 IPMP)", // 0x1A
    "AVC video stream as defined in ITU-T Rec. H.264 | ISO/IEC 14496-10 Video",  // 0x1B
    "ITU-T Rec. H.222.0 | ISO/IEC 13818-1 Reserved",          // 0x1C ~ 0x7E
    "IPMP stream",                                            // 0x7F
    "User Private"                                            // 0x80 ~ 0xFF
};


char *tp_pmt_stream_type_name_get(uint32_t stream_type)
{
    uint32_t   stream_type_name_numb;
    char      *name_p;

    stream_type_name_numb = sizeof(stream_type_name)/sizeof((char *) NULL);
    assert(0x1E < stream_type_name_numb);

    if (stream_type < 0x1C) {
        name_p = stream_type_name[stream_type];
    } else if (stream_type < 0x7F) {
        name_p = stream_type_name[0x1C];
    } else if (stream_type < 0x80) {
        name_p = stream_type_name[0x1D];
    } else {
        name_p = stream_type_name[0x1E];
    }

    return stream_type_name[stream_type];
}


void tp_pmt_info_init(pmt_info_t *pmt_info_p)
{
    uint32_t   n;

    memset((void *) pmt_info_p, 0, sizeof(pmt_info_t));

    for (n = 0; n < PMT_ES_NUMB_MAX; ++n) {
        pmt_info_p->pmt_es_info[n].stream_type    = 0;
        pmt_info_p->pmt_es_info[n].elementary_PID = TP_PID_NULL_TP;
    }
}


void tp_pmt_info_get(psi_sec_buff_t *psi_sec_buff_p, pmt_info_t *pmt_info_p)
{
    uint8_t    *sec_buff_p;   
    uint16_t    ver_numb;

    if (psi_sec_buff_p->sec_ready == 0)
        return;
    sec_buff_p = psi_sec_buff_p->buff;

    // get the version number 1st
    ver_numb = (sec_buff_p[5] >> 1) & 0x1F;
    // check the version numb
    if (ver_numb != pmt_info_p->version_number) {
        // new version, do the clean up
        tp_pmt_info_init(pmt_info_p);
    }

    pmt_info_p->table_id = tp_psi_sec_buff_get_table_id(psi_sec_buff_p);
    pmt_info_p->section_length = tp_psi_sec_buff_get_section_length(psi_sec_buff_p);

    pmt_info_p->program_number = (sec_buff_p[3] << 8) | sec_buff_p[4];

    pmt_info_p->version_number = (sec_buff_p[5] >> 1) & 0x1F;

    pmt_info_p->section_number      =  sec_buff_p[6];
    pmt_info_p->last_section_number =  sec_buff_p[7];

    pmt_info_p->pcr_PID = ((sec_buff_p[8] << 8) | sec_buff_p[9]) & 0x1FFF;
}




// **********************************************************************************
// **********************************************************************************
// **********************************************************************************

psi_sec_buff_t psi_sec_buff;
pat_info_t     pat_info;



uint32_t tp_psi_debug(uint32_t pid_req, uint32_t *tp_buff_p, uint32_t tp_numb)
{
    uint32_t        *tp_p;
    uint32_t         tp_indx;
    tp_head_info_t   tp_head_info;
    psi_sec_stat_t   rt;

    tp_psi_sec_buff_init(&psi_sec_buff);

    tp_indx = 0;
    while (tp_indx < tp_numb) {

        tp_p = tp_buff_p + (tp_indx * TP_SIZE_WORD);

        // get the TP head info
        tp_head_info_get(tp_p, &tp_head_info);
//      tp_head_info_dump(&tp_head_info);
        if (tp_head_info.pid != pid_req) {
            ++tp_indx;
            continue;
        }

        rt = tp_psi_sec_buff_save((uint8_t *) tp_p, &tp_head_info, &psi_sec_buff);
        if (rt == BUFF_READY_WITH_NEXT_SEC || rt == BUFF_READY) {
            tp_psi_sec_buff_dump(&psi_sec_buff);

            tp_pat_info_init(&pat_info);
            tp_pat_info_get(&psi_sec_buff, &pat_info);
            tp_pat_info_dump(&pat_info);

            tp_psi_sec_buff_init(&psi_sec_buff);            
        }

        ++tp_indx;
    }

    tp_pmt_stream_type_name_get(0);
    return 1;
}
