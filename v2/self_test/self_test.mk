ifeq ($(PARAM_FILE), )
     PARAM_FILE:=../../Makefile.param
     include $(PARAM_FILE)
endif

include $(BUILD_PATH)/.config
include $(MW_PATH)/sample/Kbuild
################# select sensor type for your sample ###############################
SENSOR0_TYPE ?= SONY_IMX327_MIPI_2M_30FPS_12BIT
SENSOR1_TYPE ?= SONY_IMX327_MIPI_2M_30FPS_12BIT

SNS_LIB = -lsns_full

COMMON_DIR ?= $(PWD)/../../sample/common

CFLAGS += -DSENSOR0_TYPE=$(SENSOR0_TYPE)
CFLAGS += -DSENSOR1_TYPE=$(SENSOR1_TYPE)
CFLAGS += $(KBUILD_DEFINES)
#########################################################################
COMM_SRC := $(wildcard $(COMMON_DIR)/*.c)
COMM_OBJ := $(COMM_SRC:%.c=%.o)
COMM_INC := $(COMMON_DIR)
COMM_DEPS = $(COMM_SRC:.c=.d)

