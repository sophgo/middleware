# ----------------------------------------------------------------------
#
# Project: cvi_audio lib release and sample code exe bin
#
# ----------------------------------------------------------------------
SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
ifeq ($(MW_VER), )
     PARAM_FILE:=../../Makefile.param
     include $(PARAM_FILE)
else
     PARAM_FILE:=../../../$(shell echo $(MW_VER))/Makefile.param
     include $(PARAM_FILE)
endif
endif

AUDIO_PARAM_FILE:=./audio.mk
include $(AUDIO_PARAM_FILE)

SAMPLE_DIR ?= $(MW_PATH)/sample/audio
ROOTFS_INSTALL_DIR ?= /home/fgao/nfsroot/decoder
DEBUG ?= 1
OS ?= linux
DEFINES =
DESTDIR ?= ./install/lib
RM=rm -f
MKDIR=mkdir -p
INSTALL_LIBDIR = $(MW_LIB)
INSTALL_3rdLIBDIR = $(MW_3RD_LIB)
SELF_DIR = $(MW_PATH)/self_test/audio

ifeq ($(SDK_VER), 64bit)
PREBUILD_LIBDIR = prebuilt/lib64
else ifeq ($(SDK_VER), uclibc)
PREBUILD_LIBDIR = prebuilt/uclibc
DEFINES += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
else ifeq ($(SDK_VER), glibc_riscv64)
PREBUILD_LIBDIR = prebuilt/glibc_riscv64
DEFINES += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
else ifeq ($(SDK_VER), musl_riscv64)
PREBUILD_LIBDIR = prebuilt/musl_riscv64
DEFINES += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
else
PREBUILD_LIBDIR = prebuilt/lib32
DEFINES += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
endif

ifeq ($(USE_ALSA), yes)
PREBUILD_LIBDIR_ALSA ?= $(PREBUILD_LIBDIR)/alsa_lib
else  ifeq ($(USE_TINYALSA), yes)
PREBUILD_LIBDIR_ALSA ?= $(PREBUILD_LIBDIR)/tinyalsa_lib
endif

ifeq ($(USE_ALGO), yes)
DEFINES += -DUSE_ALGO_SOURCE
INCLUDES += -I./include
endif

ifeq ($(USE_AiMultiMic), yes)
DEFINES += -DAiMultiMic
endif

ifeq ($(USE_CYCLEBUFFER), yes)
DEFINES += -DUSE_CYCLE_BUFFER
endif

ifeq ($(USE_NEON_SIMD), yes)
DEFINES += -DNEON_OPT
endif

ifeq ($(USE_SYSTRACE), yes)
DEFINES += -DENABLE_FUNCTION_SYSTRACE
endif

ifeq ($(USE_CVI_AEC), no)
DEFINES += -DEXTERNAL_AEC
else
DEFINES += -DUSE_CVI_AEC
endif

DEFINES += -DCHIP_CV1835
DEFINES += -DCVI_MODIFIED
ifeq ($(USE_ALSA), yes)
DEFINES += -DUSE_ALSA
endif

ifeq ($(USE_SYS_GLOBAL_LOG), yes)
DEFINES += -DAUDIO_PRINT_WITH_GLOBAL_COMM_LOG
endif

#$(shell cp sample_v2/component_list_all.h sample_v2/component/component_list.h)
ifeq ($(OS),linux)
ifeq ($(PRODUCTFORM),soc)
BUILD_CONFIGURATION = EmbeddedLinux
else # pcie
BUILD_CONFIGURATION = NativeLinux
endif
endif

ifeq ($(OS),none)
ifeq ($(MAKECMDGOALS), test)
$(error test is only supported in linux environment)
endif
BUILD_CONFIGURATION = NonOS
endif

SLIBNAME        = libcvi_audio.so
LIBAUDIO_A=$(INSTALL_LIBDIR)/libcvi_audio.a
LIBAUDIO_SO=$(INSTALL_LIBDIR)/libcvi_audio.so
$(info, "cvi_audio platform EmbeddedLinux")

INCLUDES = -Iinclude -I$(MW_INC)

INCLUDES += -I $(MW_PATH)/sample/common/ -I$(KERNEL_INC)
INCLUDES += -I $(MW_PATH)/modules/audio/src/


ALGOINCLUDES =
ALGOINCLUDES += -I./include -I./../../include
ALGOINCLUDES += -I$(KERNEL_INC) -I$(SYS_INC)
CVI_NR_INCLUDES =
CVI_NR_INCLUDES += -I./include
CVI_NR_INCLUDES += -I./include -I./../../include

RES_INCLUDES =
RES_INCLUDES +=  -I ./include -I ../../include

BD_INCLUDES =
BD_INCLUDES += $(INCLUDES)

ifeq ($(DEBUG), 0)
CFLAGS +=
else ifeq ($(DEBUG), 1)
CFLAGS += -g -O0
endif
CFLAGS  += -I. -fPIC -Wno-unused-result -Wno-implicit-function-declaration -Wno-format -Wl,--fatal-warning $(INCLUDES) $(DEFINES)


ifeq ($(USE_CVI_AEC), yes)
AEC_INCLUDES += $(ALGOINCLUDES)
endif

EXTRA_CFLAGS = $(AEC_INCLUDES) $(DEFINES)

ifeq ($(SDK_VER), 64bit)
EXTRA_CFLAGS += -DSDK_VER=64
else ifeq ($(SDK_VER), riscv64)
EXTRA_CFLAGS += -DSDK_VER=64
else ifeq ($(SDK_VER), uclibc)
EXTRA_CFLAGS += -DSDK_VER=32
else ifeq ($(SDK_VER), musl_riscv64)
EXTRA_CFLAGS += -DSDK_VER=64
else
EXTRA_CFLAGS += -DSDK_VER=32
endif

RES_CFLAGS += -I . -fPIC -Wno-unused-result  -Wno-format -Wl,--fatal-warning $(RES_INCLUDES) $(DEFINES)
BD_CFLAGS += -I . -fPIC -Wno-unused-result  -Wno-format -Wl,--fatal-warning $(BD_INCLUDES) $(DEFINES)
TMP_LDFLAGS =

ifeq ($(USE_ALSA), yes)
EXTRA_LDFLAGS += -L$(PREBUILD_LIBDIR)
EXTRA_LDFLAGS += -L$(PREBUILD_LIBDIR)/alsa_lib
endif

ifeq ($(USE_TINYALSA), yes)
EXTRA_LDFLAGS += -L$(PREBUILD_LIBDIR)
EXTRA_LDFLAGS += -L$(PREBUILD_LIBDIR)/tinyalsa_lib
endif

ifeq ($(USE_CVI_AEC), yes)
#EXTRA_LDFLAGS += -L$(PREBUILD_LIBDIR)/aec
endif

EXTRA_LDFLAGS += -L$(INSTALL_LIBDIR)
EXTRA_LDFLAGS += -L$(PREBUILD_LIBDIR)/cvi_resample

RES_LDFLAGS =
RES_LDFLAGS += -L$(PREBUILD_LIBDIR)/cvi_resample
RES_LDFLAGS += -L$(INSTALL_LIBDIR)

BD_LDFLAGS =
BD_LDFLAGS += -L$(INSTALL_LIBDIR)

LDLIBS = -lcvi_audio
ifeq ($(USE_SYS_GLOBAL_LOG), yes)
LDLIBS += -lsys
endif
TMP_LDLIBS =
AEC_LDLIBS =
ifeq ($(USE_CVI_AEC), no)
AEC_TEST_LDLIBS +=  -lfsp.v1.0.1.Release
else
AEC_TEST_LDLIBS += -laec
endif
RES_LDLIBS =

BD_LDLIBS =
BD_LDLIBS += -lcvi_audio
BD_LDLIBS += -lcvi_ANR
BD_LDLIBS += -lasound
BD_LDLIBS += -lcvi_RES1

ifeq ($(USE_CVI_AEC), no)
LDLIBS += -lfsp.v1.0.1.Release
else
#LDLIBS += -laec
endif
LDLIBS += -lcvi_vqe
LDLIBS += -lcvi_VoiceEngine
TMP_LDLIBS  += -lcvi_VoiceEngine
LDLIBS += -lcvi_RES1
LDLIBS += -lrt

ifeq ($(USE_CVI_AEC), yes)
#LDLIBS += -laec
endif

ifeq ($(USE_ALSA), yes)
LDLIBS += -lasound
endif

ifeq ($(USE_TINYALSA), yes)
LDLIBS += -ltinyalsa
endif

ifeq ($(USE_NEXT_SSP_DUAL), yes)
EXTRA_CFLAGS += -DNEXT_SSP_ALGO
endif

ifeq ($(USE_SONIC_LIB), yes)
EXTRA_CFLAGS += -DUSE_SONIC_LIB
endif

AEC_LDLIBS += -laec.v1.0.4
RES_LDLIBS += -lcvi_RES1


BD_LDLIBS += -laec
BD_LDLIBS += -lcvi_vqe
BD_LDLIBS += -lcvi_dnvqe
BD_LDLIBS += -lcvi_VoiceEngine

AEC_LDFLAGS = -L$(INSTALL_LIBDIR)

BUILDLIST=
ifeq ($(OS),linux)
BUILDLIST += libssp20220128
BUILDLIST += libdnssp
BUILDLIST += libDnVqe #this should be removed !

ifeq ($(USE_SONIC_LIB), yes)
BUILDLIST += libsonic
endif
BUILDLIST += $(LIBAUDIO_A)
BUILDLIST += $(LIBAUDIO_SO)
BUILDLIST += libvqe
BUILDLIST += libvoice_engine
BUILDLIST += libcvi_RES1
else
$(info, "[Error]OS not Linux ...No build list")
endif

AUDIO_SRCS = src/cvi_aud_internal.c
ifeq ($(USE_ALSA), yes)
AUDIO_SRCS += src/cvi_audio_interface.c
endif
ifeq ($(USE_TINYALSA), yes)
AUDIO_SRCS += src/cvi_audio_interface_tinyalsa.c
AUDIO_SRCS += src/cvi_ao_internal.c
endif

EXTRA_DEF =
ifeq ($(CONFIG_ENABLE_FREERTOS),y)
ifeq ($(CVIARCH), $(filter $(CVIARCH), CV181X CV180X))
AUDIO_SRCS += src/audio_kernel_mode/cviaudio_kernel_mode.c
EXTRA_DEF += -DAUD_SUPPORT_KERNEL_MODE
endif
endif

#TO BE ADD for cviaudio_core.ko command
#AUDIO_SRCS += src/audio_kernel_mode/cviaudio_kernel_mode.c
ifeq ($(USE_CYCLEBUFFER), yes)
AUDIO_SRCS += src/cyclebuffer.c
endif
AUDIO_SRCS += src/cvi_audio_dec.c
AUDIO_SRCS += src/cvi_audio_enc.c
AUDIO_SRCS += src/cvi_audio_threads.c
AUDIO_SRCS += src/cvi_audio_common.c
AUDIO_SRCS += src/cvi_audio_fader.c
AUDIO_SRCS += src/sharebuffer.c
AUDIO_SRCS += src/volume_ctrl.c
AUDIO_SRCS += src/cvi_ai_internal.c

ifeq ($(USE_SPEED), yes)
AUDIO_SRCS += src/cvi_audio_speed.c
endif

ifeq ($(MULTI_PROCESS_SUPPORT_AUDIO), yes)
INCLUDES += -I ./src/audio_rpc
AUDIO_SRCS += ./src/audio_rpc/cvi_audio_rpc.c
EXTRA_CFLAGS += -DRPC_MULTI_PROCESS_AUDIO
ifeq ($(USE_SHARE_MEM_IN_AUDIO_RPC), yes)
EXTRA_CFLAGS += -DSHARE_MEM_IN_AUDIO_RPC
AUDIO_SRCS += ./src/audio_rpc/cvi_audio_shm.c
endif
endif

ifeq ($(USE_CVITEK_SOFTWARE_AEC), yes)
EXTRA_CFLAGS += -DCVIAUDIO_SOFTWARE_AEC
endif



AUDIO_OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(AUDIO_SRCS)))
DEP_PATHS += $(patsubst %.c,%.d,$(patsubst %.cpp,%.d,$(AUDIO_SRCS)))


SOURCE_TEST = $(SAMPLE_DIR)/cvi_sample_audio.c
SOURCE_TEST += $(SAMPLE_DIR)/cvi_sample_comm_audio.c
SOURCE_TEST += $(SAMPLE_DIR)/cvi_audio_dl_adp.c

OBJECTNAMES_AUDIO=$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(ADUIO_SRCS)))
OBJECTPATHS_AUDIO=$(addprefix ./src/,$(notdir $(OBJECTNAMES_AUDIO)))

ts := $(shell /bin/date "+%Y-%m-%d---%H-%M-%S")

.PHONY: clean all install
all: prepare $(BUILDLIST) install

%.o: %.c
	echo $(KERNEL_INC)
	@echo $(KERNEL_INC)
	@echo $(MW_INC)
	$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(EXTRA_DEF) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(LIBAUDIO_A): $(AUDIO_OBJS)
	@$(AR) $(ARFLAGS) $@ $(AUDIO_OBJS)
	@echo -e $(YELLOW)[LINK]$(END)[$(notdir $(AR))] $(notdir $@)

$(LIBAUDIO_SO): $(AUDIO_OBJS)
	@$(LD) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@ --start-group $(AUDIO_OBJS) --end-group
	@echo -e $(GREEN)[LINK]$(END)[$(notdir $(LD))] $(notdir $@)


libvoice_engine:
	@cd ./src/audio_codec; make all

libvqe:
	@cd ./src/audio_vqe; make all

libDnVqe:
	@cd ./src/audio_dnvqe; make clean; make all

libcvi_RES1:
	@cd ./src/audio_resample; make clean; make all

libssp20220128:
	@echo "build libssp version 20220128"
	@cd ./src/algo/SSP_Algorithm_20220128; make clean; make all
libdnssp:
	@echo "build libssp2 version 20231218"
	@cd ./src/algo/downlink; make clean; make all

libsonic:
	@echo "build libsonic"
	@cd ./src/algo/sonic; make clean; make all

algo_uclibc:
	@echo "uclibc prebuld algo"
	@cp -rf ./prebuilt/uclibc/algo/* $(INSTALL_LIBDIR)/

install:
	@echo "[audio step]COPY LIB!! "
	@echo "[audio insatll>>>>] copy lib to:" $(INSTALL_LIBDIR)
	@cp include/cvi_audio_vqe.h include/cvi_resampler_api.h $(SELF_DIR)/include
	@cp -rf $(PREBUILD_LIBDIR)/fdk_aac_fake/* $(INSTALL_LIBDIR)/
	@cp -rf $(PREBUILD_LIBDIR_ALSA)/* $(INSTALL_LIBDIR)/
#	@cp $(NANOMSG_PATH)/* $(INSTALL_3rdLIBDIR)/
	@(/bin/date "+%Y-%m-%d---%H-%M-%S")
	@echo "[audio step]build lib finished!! "


$(TEST): prepare
	$(CC) -o   $@ $(LDFLAGS) $(SOURCE_TEST) $(LDLIBS) $(CFLAGS)

clean:
	@echo "[audio clean>>>>] remove lib from:" $(MW_LIB)
	@$(RM) $(TARGET)
	@$(RM) $(DEP_PATHS)
	@$(RM) $(AUDIO_OBJS)
	@$(RM) $(LIBAUDIO_A)
	@$(RM) $(LIBAUDIO_SO)
	@$(RM) $(MW_INC)/cvi_transcode_interface.h
	@$(RM) $(SELF_DIR)/include/*.h
#	@$(RM) $(INSTALL_3rdLIBDIR)/libnanomsg*
	@$(RM) $(INSTALL_LIBDIR)/libcvi_RES*
	@$(RM) $(INSTALL_LIBDIR)/libcvi_ssp*
	@$(RM) $(INSTALL_LIBDIR)/libdnvqe*
	@$(RM) $(INSTALL_LIBDIR)/libtinyalsa*
	@$(RM) $(INSTALL_LIBDIR)/libaac*
	@cd ./src/audio_rpc; $(RM) *.o
	@cd ./src/audio_codec; make clean
	@cd ./src/audio_vqe; make clean
	@cd ./src/algo/SSP_Algorithm_20220128; make clean
	@cd ./src/audio_dnvqe; make clean
	@cd ./src/algo/sonic; make clean

prepare:
	@echo [aduio Start] Timestamp is $(ts)
	echo $(KERNEL_INC)
	@echo $(KERNEL_INC)
	echo $(SDK_VER)
	@echo $(SDK_VER)
	@$(MKDIR) $(INSTALL_LIBDIR)

-include $(DEP_PATHS)
