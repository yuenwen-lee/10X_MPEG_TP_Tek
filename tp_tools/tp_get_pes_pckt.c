#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <arpa/inet.h>

#include "tp_head_tools.h"




// **********************************************************************************
// **********************************************************************************
// **********************************************************************************

uint32_t tp_search_pes_header(uint32_t pid_req, uint32_t *tp_buff_p, uint32_t tp_numb)
{
    uint32_t        *tp_p;
    uint32_t         tp_indx;
    tp_head_info_t   tp_head_info;

    tp_indx = 0;
    while (tp_indx < tp_numb) {

        tp_p = tp_buff_p + (tp_indx * TP_SIZE_WORD);

        // get the TP head info
        tp_head_info_get(tp_p, &tp_head_info);
        tp_head_info_dump(&tp_head_info);
        if (tp_head_info.pid != pid_req) {
            ++tp_indx;
            continue;
        }

        // get the payload start flag
        if (tp_head_info.payload_unit_start_indicator) {
            
        }

        ++tp_indx;
    }

    return 1;
}


