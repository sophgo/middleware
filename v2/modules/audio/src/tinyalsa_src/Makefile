#PARAM_FILE:=../../../../../Makefile.param
#include $(PARAM_FILE)
SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I./include
COMM_OBJS = pcm.o mixer.o
PLAY_OBJS = $(COMM_OBJS) tinyplay.o
MIXER_OBJS = $(COMM_OBJS) tinymix.o
CAP_OBJS = $(COMM_OBJS) tinycap.o
DEPS = $(SRCS:.c=.d)
#CROSS_COMPILE =		aarch64-linux-gnu-
#CROSS_COMPILE =arm-linux-gnueabihf-
#CROSS_COMPILE =riscv64-unknown-linux-gnu-
CROSS_COMPILE =riscv64-unknown-linux-musl-
CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
TARGET_PLAY = tinyplay
TARGET_CAP = tinycap
TARGET_MIXER = tinymix
TARGET_LIB = libtinyalsa.so
TARGET_A = libtinyalsa.a
TARGET_PATH := $(PWD)

LIBS :=

CFLAGS +=  $(INCS) -fPIC

all : $(TARGET_PLAY)  $(TARGET_MIXER) $(TARGET_CAP) $(TARGET_LIB) $(TARGET_A)

$(SDIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS)  -o $@ -c $<


$(TARGET_PLAY) : $(PLAY_OBJS)
	$(CC) -o $(TARGET_PATH)/$@ $(PLAY_OBJS) $(LIBS)

$(TARGET_CAP) : $(CAP_OBJS)
	$(CC) -o $(TARGET_PATH)/$@ $(CAP_OBJS) $(LIBS)

$(TARGET_MIXER) : $(MIXER_OBJS)
	$(CC) -o $(TARGET_PATH)/$@ $(MIXER_OBJS) $(LIBS)

$(TARGET_LIB) : $(COMM_OBJS)
	$(CC) -o $(TARGET_PATH)/$@ $(COMM_OBJS) $(LIBS) $(CFLAGS) --shared

$(TARGET_A) : $(COMM_OBJS)
	$(AR) rcs $(TARGET_PATH)/$@ $(COMM_OBJS) $(LIBS)

.PHONY : clean
clean:
	rm -f *.o  $(TARGET) $(TARGET_PLAY) $(TARGET_CAP) $(TARGET_MIXER) $(TARGET_LIB) $(TARGET_A)
