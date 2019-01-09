#ifndef _TP_TEK_T_STAMP_H_
#define _TP_TEK_T_STAMP_H_



#define TEK_T_STAMP_SIZE     16   // extra 16 byte of time stamp



uint32_t tp_tek_t_stamp_get(uint32_t *tek_t_stamp, uint32_t tp_indx);
void tp_tek_t_stamp_dump(uint32_t *tek_t_stamp, uint32_t tp_num);
void tp_tek_t_stamp_stat_dump(uint32_t *tek_t_stamp, uint32_t tp_num);
int read_tek_ts_file(char *path,
		     char **buff_p, uint32_t *size_p, char **t_stamp_p);




#endif // _TP_TEK_T_STAMP_H_
