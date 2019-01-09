#
# 'make depend' uses makedepend to automatically generate dependencies
#               (dependencies are added to end of Makefile)
# 'make'        build executable file 'mycc'
# 'make clean'  removes all .o and executable files
#

# define the C compiler to use
CC = cc

# define any compile-time flags
CFLAGS = -Wall -O2

# define any directories containing header files other than /usr/include
#
INCLUDES = -I./ -I./tp_tools

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS =

# define any libraries to link into executable:
#   if I want to link in libraries (libx.so or libx.a) I use the -llibname
#   option, something like (this will link in libmylib.so and libm.so:
LIBS =

# define the C source files
SRCS_APP =

SRCS_TOOL =  \
	tp_tools/read_video_file.c \
	tp_tools/tp_af_tools.c \
	tp_tools/tp_get_pes_pckt.c \
	tp_tools/tp_head_tools.c \
	tp_tools/tp_psi_tools.c \
	tp_tools/tp_streaming_tools.c \
	tp_tools/tp_tek_t_stamp.c \
	tp_tools/udp_tools.c

# define the C object files
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#
OBJS = $(SRCS_APP:.c=.o) $(SRCS_TOOL:.c=.o)

# define the executable file
PID_STAT = ts_PID_static
BIT_RATE = ts_bit_rate
TX_STREAM = ts_tx_stream

#
# The following part of the makefile is generic; it can be used to
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean

all:    $(PID_STAT) $(BIT_RATE) $(TX_STREAM)
	@echo 'All applications have been compiled'

$(PID_STAT): $(OBJS) main_PID_static.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $(PID_STAT) main_PID_static.o $(OBJS) $(LFLAGS) $(LIBS)

$(BIT_RATE): $(OBJS) main_bit_rate.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $(BIT_RATE) main_bit_rate.o $(OBJS) $(LFLAGS) $(LIBS)

$(TX_STREAM): $(OBJS) main_tx_stream.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TX_STREAM) main_tx_stream.o $(OBJS) $(LFLAGS) $(LIBS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file)
# (see the gnu make manual section about automatic variables)
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) $(OBJS) *.o $(PID_STAT) $(BIT_RATE) $(TX_STREAM)

depend: $(SRCS_APP) $(SRCS_TOOL)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it

tp_tools/read_video_file.o: tp_tools/tp_head_tools.h
tp_tools/read_video_file.o: tp_tools/tp_tek_t_stamp.h
tp_tools/tp_af_tools.o: tp_tools/tp_head_tools.h tp_tools/tp_af_tools.h
tp_tools/tp_get_pes_pckt.o: tp_tools/tp_head_tools.h
tp_tools/tp_head_tools.o: tp_tools/tp_head_tools.h tp_tools/tp_af_tools.h
tp_tools/tp_psi_tools.o: tp_tools/tp_head_tools.h tp_tools/tp_psi_tools.h
tp_tools/tp_streaming_tools.o: tp_tools/udp_tools.h tp_tools/time_os.h
tp_tools/tp_streaming_tools.o: tp_tools/tp_head_tools.h
tp_tools/tp_streaming_tools.o: tp_tools/tp_af_tools.h
tp_tools/tp_streaming_tools.o: tp_tools/tp_streaming_tools.h
tp_tools/tp_tek_t_stamp.o: tp_tools/tp_head_tools.h tp_tools/tp_tek_t_stamp.h
tp_tools/udp_tools.o: tp_tools/tp_head_tools.h
