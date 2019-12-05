# Hisilicon Hi35xx sample Makefile

include ../Makefile.param
#ifeq ($(SAMPLE_PARAM_FILE), )
#     SAMPLE_PARAM_FILE:=../Makefile.param
#     include $(SAMPLE_PARAM_FILE)
#endif

# target source
SRC  := $(wildcard *.c) #.c 
OBJ  := $(SRC:%.c=%.o) #.o
$(warning $(SRC) $(OBJ))

#TARGET := $(OBJ:%.o=%)
TARGET := sample_vio

.PHONY : clean all
$(warning $(TARGET))

all: $(TARGET)

MPI_LIBS := $(REL_LIB)/libmpi.a
MPI_LIBS += $(REL_LIB)/libhdmi.a
$(warning $(MPI_LIBS))

$(TARGET):%:%.o $(COMM_OBJ) $(OBJ)
	$(CC) $(CFLAGS) -lpthread -lm -o $@ $^ $(MPI_LIBS) $(AUDIO_LIBA) $(JPEGD_LIBA)
#	$(warning $(MPI_LIBS))
#	$(CC) $(CFLAGS) -lpthread -lm -o $@ $^ $(MPI_LIBS) $(AUDIO_LIBA) $(JPEGD_LIBA)
#	cp sample_vio /root/Workspace/NFS/Hi3531DV100/
	rm *.o
clean:
	@rm -f $(TARGET)
	@rm -f $(OBJ)
	@rm -f $(COMM_OBJ)

cleanstream:
	@rm -f *.h264
	@rm -f *.jpg
	@rm -f *.mjp
	@rm -f *.mp4
