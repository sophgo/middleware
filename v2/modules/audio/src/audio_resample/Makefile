.PHONY: CREATE_DIR
ifeq ($(PARAM_FILE), )
ifeq ($(MW_VER), )
PARAM_FILE:=../../../../Makefile.param
include $(PARAM_FILE)
else
PARAM_FILE:=../../../../../$(shell echo $(MW_VER))/Makefile.param
include $(PARAM_FILE)
endif
endif


SRCS = $(wildcard $(PWD)/*.c)
EXTRA_SRC =
SRCS += $(EXTRA_SRC)
INCS = -I$(MW_INC) -I$(KERNEL_INC)
INCS += -I ../../include  -I .
OBJS = $(SRCS:.c=.o)

TARGET = $(MW_LIB)/libcvi_RES1.a
TARGET_S = $(MW_LIB)/libcvi_RES1.so

ARFLAGS = rcs
ARFLAGS_S = -shared -fPIC -o
CFLAGS =  -O3 -fPIC
CFLAGS += -DCVI_MODIFIED
CFLAGS += -D__FILENAME__=\"$(notdir $<)\"

#for execution bin --start
LIB_DIR = -L$(PWD)/
LIBS +=  -lm -lpthread
OBJS_EXE := $(SRCS:%.c=%.o)
#for execution bin --end

OBJECTPATHS_SRC=$(addprefix $(PWD)/,$(notdir $(OBJS)))

BUILD_LIST =  $(TARGET)
all : $(BUILD_LIST)

$(OBJS):
	$(CC) $(SRCS) $(CFLAGS) -c  $(INCS)

$(TARGET) : $(OBJS)
	$(AR) $(ARFLAGS) $(TARGET) $(OBJECTPATHS_SRC)
	$(CC) $(ARFLAGS_S) $(TARGET_S) $(OBJECTPATHS_SRC)

.PHONY : clean
clean:
	rm -f $(OBJS) $(TARGET) $(TARGET_S) $(OBJECTPATHS_SRC)



