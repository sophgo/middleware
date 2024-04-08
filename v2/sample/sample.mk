################# select sensor type for your sample ###############################
include $(BUILD_PATH)/.config
include $(MW_PATH)/sample/Kbuild

SENSOR0_TYPE ?= SONY_IMX327_MIPI_2M_30FPS_12BIT
SENSOR1_TYPE ?= SONY_IMX327_MIPI_2M_30FPS_12BIT

SNS_LIB = -lsns_full

COMMON_DIR ?= $(PWD)/../common

CFLAGS += -DSENSOR0_TYPE=$(SENSOR0_TYPE)
CFLAGS += -DSENSOR1_TYPE=$(SENSOR1_TYPE)

ifeq ($(DEBUG), 1)
CFLAGS += -g -O0
endif

ifeq ($(SAMPLE_STATIC), 1)
ELFFLAGS += -static
endif

CFLAGS += $(KBUILD_DEFINES)
PANEL_INC =$(COMMON_DIR)/../../component/panel/$(shell echo $(CVIARCH) | tr A-Z a-z)

#########################################################################
COMM_SRC := $(wildcard $(COMMON_DIR)/*.c)
COMM_OBJ := $(COMM_SRC:%.c=%.o)
COMM_INC := -I$(COMMON_DIR) -I$(PANEL_INC)
COMM_DEPS = $(COMM_SRC:.c=.d)
