#!/bin/sh

if test $# -ne 1
then
  echo 'Usuage:' $0 'f_name'
  exit
fi


### get the discontinuous flag
cut -d:  -f 1 $1 | cut -f2 -d'-' > tp_scf


### get the index
cut -d:  -f 1 $1 | cut -f3 -d'-' > tp_indx


### get the PCR Lo
cut -d,  -f 2 $1 > tmp_data
cut -d\( -f 2 tmp_data > tp_pcr_lo
rm tmp_data


### get the PCR Hi
cut -d,  -f 3 $1 > tmp_data
cut -d\) -f 1 tmp_data > tp_pcr_hi
rm tmp_data


### get the Rate
cut -d,  -f 7 $1 > tmp_data
cut -db  -f 1 tmp_data > tp_rate
rm tmp_data


### get the Tek T_Stamp
cut -d,  -f 8 $1 > tp_t_stamp

paste tp_scf tp_indx tp_pcr_lo tp_pcr_hi tp_rate tp_t_stamp > tp_all_cntr

wc tp_scf tp_indx tp_pcr_lo tp_pcr_hi tp_rate tp_t_stamp tp_all_cntr

