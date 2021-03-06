#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <arpa/inet.h>

#include "tp_head_tools.h"
#include "tp_af_tools.h"




int32_t tp_search_sync(uint32_t *buff_p, uint32_t buff_size_byte)
{
#define TP_SEARCH_MATCH_COUNT  8
    uint32_t  head;
    uint32_t  buff_size_word;;
    uint32_t  match_count;
    uint32_t  loc, n;

    buff_size_word = buff_size_byte / sizeof(uint32_t);

    loc = 0;
    while (loc < buff_size_word) {

        for (loc = 0; loc < buff_size_word; loc++) {
            head = ntohl(buff_p[loc]);
            if ((head >> 24) == TP_SYNC_BYTE) {
                match_count = 0;
                for (n = loc; n < buff_size_word; n += TP_SIZE_WORD) {
                    head = ntohl(buff_p[n]);
                    if ((head >> 24) == TP_SYNC_BYTE) {
                        if (++match_count >= TP_SEARCH_MATCH_COUNT) {
                            return (loc * sizeof(uint32_t));
                        }
                    } else {
                        break;
                    }
                }
            }
        }

        loc++;
    }

    return -1;
}


void tp_head_info_get(uint32_t *tp_p, tp_head_info_t *tp_hd_info_p)
{
    uint32_t  tp_head;

    tp_head = ntohl(tp_p[0]);

    tp_hd_info_p->sync_byte                    = get_bit_field(tp_head,
                                                               TP_HEAD_SYNC_BIT_LOC,
                                                               TP_HEAD_SYNC_BIT_NUMB);
    tp_hd_info_p->transport_error_indicator    = get_bit_field(tp_head,
                                                               TP_HEAD_TRANS_ERR_BIT_LOC,
                                                               TP_HEAD_TRANS_ERR_BIT_NUMB);
    tp_hd_info_p->payload_unit_start_indicator = get_bit_field(tp_head,
                                                               TP_HEAD_PYLD_START_BIT_LOC,
                                                               TP_HEAD_PYLD_START_BIT_NUMB);
    tp_hd_info_p->transport_priority           = get_bit_field(tp_head,
                                                               TP_HEAD_TRANS_PRI_BIT_LOC,
                                                               TP_HEAD_TRANS_PRI_BIT_NUMB);
    tp_hd_info_p->pid                          = get_bit_field(tp_head,
                                                               TP_HEAD_PID_BIT_LOC,
                                                               TP_HEAD_PID_BIT_NUMB);
    tp_hd_info_p->transport_scrambling_control = get_bit_field(tp_head,
                                                               TP_HEAD_SCRMB_CTRL_BIT_LOC,
                                                               TP_HEAD_SCRMB_CTRL_BIT_NUMB);
    tp_hd_info_p->adaptation_field_control     = get_bit_field(tp_head,
                                                               TP_HEAD_AF_CTRL_BIT_LOC,
                                                               TP_HEAD_AF_CTRL_BIT_NUMB);
    tp_hd_info_p->continuity_counter           = get_bit_field(tp_head,
                                                               TP_HEAD_CC_BIT_LOC,
                                                               TP_HEAD_CC_BIT_NUMB);
}


void tp_head_info_dump(tp_head_info_t *tp_head_info_p)
{
    printf("0x%08x:  %02x.%1x.%1x.%1x.%04x.%1x.%1x.%1x (%d)\n",
           (*((uint32_t *) tp_head_info_p)),
           tp_head_info_p->sync_byte,
           tp_head_info_p->transport_error_indicator,
           tp_head_info_p->payload_unit_start_indicator,
           tp_head_info_p->transport_priority,
           tp_head_info_p->pid,
           tp_head_info_p->transport_scrambling_control,
           tp_head_info_p->adaptation_field_control,
           tp_head_info_p->continuity_counter,
           tp_head_info_p->pid);
}


uint32_t tp_head_payload_loc(uint32_t *tp_p, tp_head_info_t *tp_head_info_p)
{
    uint32_t  adpt_ctrl, adpt_len;
    uint32_t  adpt_filed_1;

    adpt_ctrl = tp_head_info_p->adaptation_field_control;
    if (adpt_ctrl == 0x2 || adpt_ctrl == 0x3) {
        adpt_filed_1 = ntohl(tp_p[1]);
        adpt_len = get_bit_field(adpt_filed_1, 24, 8);
        return TP_HEAD_SIZE_BYTE + adpt_len + 1;
    } else {
        return TP_HEAD_SIZE_BYTE;
    }
}



// **********************************************************************************
// **********************************************************************************
// **********************************************************************************

#define PID_MAX_NUMB   (1 << TP_HEAD_PID_BIT_NUMB)



typedef struct tp_stat_ {
    uint32_t  pid;
    uint32_t  pid_type;
    uint32_t  numb;      // 32 bit should be enough
    uint32_t  cc;
    uint32_t  cc_err_cntr;
    uint32_t  prev_indx;
} tp_stat_t;



tp_stat_t tp_stat[PID_MAX_NUMB];



static void dump_pid_stat(tp_stat_t *tp_stat_p)
{
    uint32_t  pid;
    uint32_t  act_pid_numb;
    uint32_t  tp_numb_total;

    tp_numb_total = 0;
    act_pid_numb  = 0;
    for (pid = 0; pid < PID_MAX_NUMB; ++pid) {
        tp_numb_total += tp_stat_p[pid].numb;
        if (tp_stat_p[pid].numb)
            act_pid_numb++;
    }

    printf("PID Statistic\n");
    printf("  total %d pid\n", act_pid_numb);
    printf("  total %d tp\n", tp_numb_total);
    for (pid = 0; pid < PID_MAX_NUMB; ++pid) {

        if (tp_stat_p[pid].numb == 0)
            continue;
        printf("  %4d - 0x%04x: %5.2f%%, %d %d\n",
               pid, pid, 100.0 * ((float) tp_stat_p[pid].numb / (float) tp_numb_total),
               tp_stat_p[pid].numb, tp_stat_p[pid].cc_err_cntr);
    }
    printf("\n");
}


void get_pid_stat(uint32_t *tp_buff_p, uint32_t tp_numb)
{
    uint32_t        *tp_p;
    uint32_t         tp_indx;
    tp_head_info_t   tp_head_info;
    tp_af_info_t     tp_af_info;
    tp_stat_t       *tp_stat_p;
    uint32_t         cc_err;

    printf("   Packet Index       CC                    \n");
    printf("    now    prev     n  p    AF    DI     PID\n");
    printf("------- -------    -- --    --    --    ----\n");

    tp_indx = 0;
    while (tp_indx < tp_numb) {

        tp_p = tp_buff_p + (tp_indx * TP_SIZE_WORD);

        // get the TP head info
        tp_head_info_get(tp_p, &tp_head_info);

	if (tp_head_info.sync_byte != 0x47) {
	    uint32_t  data;
	    int n;
	    for (n = 0; n < 47; ++n) {
	        if ((n % 8) == 0) {
		    printf("\n");
		}
		data = ntohl(tp_p[n]);
		printf("%08x ", data);
	    }
	    printf("\n");

	    tp_indx++;
	    continue;
	}
	tp_stat_p = &tp_stat[tp_head_info.pid];

//	if (tp_head_info.sync_byte != TP_SYNC_BYTE) {
//	    printf("[%6u %6u] BAD SYNC byte 0x%02x\n",
//		   (tp_indx * TP_SIZE_WORD), tp_indx, tp_head_info.sync_byte);
//	    tp_loc_byte = tp_search_sync(tp_p, (tp_numb - tp_indx)*188);
//	}

        if (tp_head_info.adaptation_field_control == 0x2 ||
            tp_head_info.adaptation_field_control == 0x3) {
	    // with AF .....
	    tp_af_info_get(tp_p, &tp_af_info);
        }

        if (tp_stat_p->numb != 0) {
            // checking the continuity_counter

            cc_err = 0;
            if (tp_head_info.pid != 0x1FFF) {
                switch (tp_head_info.adaptation_field_control) {
                case 0x0:  // 00
                    // No AF, no payload, should have the same CC (and the
                    // TP should be dropped)
                    if (tp_head_info.continuity_counter != tp_stat_p->cc) {
                        cc_err = 1;
                    }
                    break;

		case 0x1:  // 01
                    // Payload only
		    if (tp_head_info.continuity_counter != ((tp_stat_p->cc + 1) & 0xF)) {
                        cc_err = 1;
                    }
                    break;

                case 0x2:  // 10
                    // AF only and no payload, should have the same CC
                    // as previous TP with the same PID
                    if (tp_head_info.continuity_counter != tp_stat_p->cc) {
                        cc_err = 1;
                    }
                    break;

                case 0x3:  // 11
                    // AF + Payload
                    if (tp_af_info.discontinuity_indicator == 0 &&
                        tp_head_info.continuity_counter != ((tp_stat_p->cc + 1) & 0xF)) {
                        cc_err = 1;
                    }
                    break;
                }
            }

            if (cc_err) {
                tp_stat_p->cc_err_cntr++;
		if (tp_head_info.adaptation_field_control == 0x2 ||
		    tp_head_info.adaptation_field_control == 0x3) {
		    // with AF .....
		    printf("%7d %7d    %2d %2d    %2x    %2x    %4d\n",
			   tp_indx, tp_stat_p->prev_indx,
			   tp_head_info.continuity_counter, tp_stat_p->cc,
			   tp_head_info.adaptation_field_control,
			   tp_af_info.discontinuity_indicator,
			   tp_head_info.pid);

		} else {
		    // NO AF .....
		    printf("%7d %7d    %2d %2d    %2x    ??    %4d\n",
			   tp_indx, tp_stat_p->prev_indx,
			   tp_head_info.continuity_counter, tp_stat_p->cc,
			   tp_head_info.adaptation_field_control,
			   tp_head_info.pid);
		}
            }
	}

        tp_stat_p->numb++;
        tp_stat_p->cc = tp_head_info.continuity_counter;
        tp_stat_p->prev_indx = tp_indx;

        ++tp_indx;
    }
    printf("\n\n");

    dump_pid_stat(tp_stat);
}
			      
			      

