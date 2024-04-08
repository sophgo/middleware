.PHONY: CREATE_DIR
ifeq ($(MW_VER), )
     PARAM_FILE:=../../../../Makefile.param
     include $(PARAM_FILE)
else
     PARAM_FILE:=../../../../../$(shell echo $(MW_VER))/Makefile.param
     include $(PARAM_FILE)
endif

include $(AUDIO_PARAM_FILE)
SDK_VER ?= uclibc
#SDK_VER ?= 64bit
#SDK_VER = 32bit

COMMON_INC=../../../common/include
AUD_INC=../../include

CUR = $(PWD)/../../prebuilt

#aac encoder -------------------------------------------start
AACENC_SRC += ./aacenc_interface_empty.cpp
AACENC_INCS =
AACENC_INCS += -I$(MW_INC)
AACENC_INCS += -I$(KERNEL_INC)
OBJS_ENC = $(AACENC_SRC:.cpp=.o)
#aac encoder -------------------------------------------end

#aac decoder -------------------------------------------start
AACDEC_SRC += ./aacdec_interface_empty.cpp
AACDEC_INCS =
AACDEC_INCS += -I$(MW_INC)
AACDEC_INCS += -I$(KERNEL_INC)
OBJS_DEC = $(AACDEC_SRC:.cpp=.o)
#aac decoder -------------------------------------------end

#aac common -------------------------------------------start
AACCOMM_SRC += ./aac_comm_empty.cpp
AACCOMM_INCS =
AACCOMM_INCS += -I$(MW_INC)
AACCOMM_INCS += -I$(KERNEL_INC)
OBJS_COMM = $(AACCOMM_SRC:.cpp=.o)
#aac common -------------------------------------------end

#aac sbr encoder -------------------------------------------start
AACSBRENC_SRC =
AACSBRENC_SRC += ./aac_sbrenc_empty.cpp
OBJS_SBRENC = $(AACSBRENC_SRC:.cpp=.o)
#aac sbr encoder -------------------------------------------end
#sbr sbr decoder -------------------------------------------start
AACSBRDEC_SRC =
AACSBRDEC_SRC += ./aac_sbrdec_empty.cpp
OBJS_SBRDEC = $(AACSBRDEC_SRC:.cpp=.o)
#sbr sbr decoder -------------------------------------------end

#AAC  test sample code --------------------------start
AAC_TEST =  $(PWD)/sample_audio_aac
AAC_TEST_SRC += $(PWD)/cvi_audio_aac_adp.c
AAC_TEST_SRC += $(MW_PATH)/sample/audio/cvi_audio_dl_adp.c
AAC_LDFLAGS =
#AAC_LDFLAGS += -L$(MW_LIB) -L$(MW_LIB)/3rd
ifeq ($(SDK_VER), 64bit)
AAC_LDFLAGS += -L $(PWD)/64bit
endif
ifeq ($(SDK_VER), 32bit)
AAC_LDFLAGS += -L $(PWD)/32bit
endif

AAC_LDLIBS =
AAC_LDLIBS += -lpthread
AAC_LDLIBS += -lm -ldl
AAC_LDLIBS += -lstdc++
AAC_LDLIBS += -laacdec2
AAC_LDLIBS += -laacenc2

AAC_INCLUDES = -I$(MW_INC)
AAC_INCLUDES += -I$(PWD)
DEFINES += -DCVI_MODIFIED
DEFINES += -D__FILENAME__=\"$(notdir $<)\"
#DEFINES += -DSUPPORT_EXTERNAL_AAC

AAC_CFLAGS += -I . -fPIC -Wno-unused-result  -Wno-format -Wl,--fatal-warning $(AAC_INCLUDES) $(DEFINES)
#AAC test sample code ----------------------------end

ifeq ($(SDK_VER), 64bit)
AACENC_TARGET = $(CUR)/lib64/fdk_aac_fake/libaacenc2.a
AACENC_TARGET_S = $(CUR)/lib64/fdk_aac_fake/libaacenc2.so

AACDEC_TARGET = $(CUR)/lib64/fdk_aac_fake/libaacdec2.a
AACDEC_TARGET_S = $(CUR)/lib64/fdk_aac_fake/libaacdec2.so

AACCOMM_TARGET = $(CUR)/lib64/fdk_aac_fake/libaaccomm2.a
AACCOMM_TARGET_S = $(CUR)/lib64/fdk_aac_fake/libaaccomm2.so

AACSBRENC_TARGET = $(CUR)/lib64/fdk_aac_fake/libaacsbrenc2.a
AACSBRENC_TARGET_S = $(CUR)/lib64/fdk_aac_fake/libaacsbrenc2.so

AACSBRDEC_TARGET = $(CUR)/lib64/fdk_aac_fake/libaacsbrdec2.a
AACSBRDEC_TARGET_S = $(CUR)/lib64/fdk_aac_fake/libaacsbrdec2.so
endif

ifeq ($(SDK_VER), 32bit)
AACENC_TARGET = $(CUR)/lib32/fdk_aac_fake/libaacenc2.a
AACENC_TARGET_S = $(CUR)/lib32/fdk_aac_fake/libaacenc2.so

AACDEC_TARGET = $(CUR)/lib32/fdk_aac_fake/libaacdec2.a
AACDEC_TARGET_S = $(CUR)/lib32/fdk_aac_fake/libaacdec2.so

AACCOMM_TARGET = $(CUR)/lib32/fdk_aac_fake/libaaccomm2.a
AACCOMM_TARGET_S = $(CUR)/lib32/fdk_aac_fake/libaaccomm2.so

AACSBRENC_TARGET = $(CUR)/lib32/fdk_aac_fake/libaacsbrenc2.a
AACSBRENC_TARGET_S = $(CUR)/lib32/fdk_aac_fake/libaacsbrenc2.so

AACSBRDEC_TARGET = $(CUR)/lib32/fdk_aac_fake/libaacsbrdec2.a
AACSBRDEC_TARGET_S = $(CUR)/lib32/fdk_aac_fake/libaacsbrdec2.so
endif


ifeq ($(SDK_VER), uclibc)
AACENC_TARGET = $(CUR)/uclibc/fdk_aac_fake/libaacenc2.a
AACENC_TARGET_S = $(CUR)/uclibc/fdk_aac_fake/libaacenc2.so

AACDEC_TARGET = $(CUR)/uclibc/fdk_aac_fake/libaacdec2.a
AACDEC_TARGET_S = $(CUR)/uclibc/fdk_aac_fake/libaacdec2.so

AACCOMM_TARGET = $(CUR)/uclibc/fdk_aac_fake/libaaccomm2.a
AACCOMM_TARGET_S = $(CUR)/uclibc/fdk_aac_fake/libaaccomm2.so

AACSBRENC_TARGET = $(CUR)/uclibc/fdk_aac_fake/libaacsbrenc2.a
AACSBRENC_TARGET_S = $(CUR)/uclibc/fdk_aac_fake/libaacsbrenc2.so

AACSBRDEC_TARGET = $(CUR)/uclibc/fdk_aac_fake/libaacsbrdec2.a
AACSBRDEC_TARGET_S = $(CUR)/uclibc/fdk_aac_fake/libaacsbrdec2.so
endif

ifeq ($(SDK_VER), glibc_riscv64)
AACENC_TARGET = $(CUR)/glibc_riscv64/fdk_aac_fake/libaacenc2.a
AACENC_TARGET_S = $(CUR)/glibc_riscv64/fdk_aac_fake/libaacenc2.so

AACDEC_TARGET = $(CUR)/glibc_riscv64/fdk_aac_fake/libaacdec2.a
AACDEC_TARGET_S = $(CUR)/glibc_riscv64/fdk_aac_fake/libaacdec2.so

AACCOMM_TARGET = $(CUR)/glibc_riscv64/fdk_aac_fake/libaaccomm2.a
AACCOMM_TARGET_S = $(CUR)/glibc_riscv64/fdk_aac_fake/libaaccomm2.so

AACSBRENC_TARGET = $(CUR)/glibc_riscv64/fdk_aac_fake/libaacsbrenc2.a
AACSBRENC_TARGET_S = $(CUR)/glibc_riscv64/fdk_aac_fake/libaacsbrenc2.so

AACSBRDEC_TARGET = $(CUR)/glibc_riscv64/fdk_aac_fake/libaacsbrdec2.a
AACSBRDEC_TARGET_S = $(CUR)/glibc_riscv64/fdk_aac_fake/libaacsbrdec2.so
endif

ifeq ($(SDK_VER), musl_riscv64)
AACENC_TARGET = $(CUR)/musl_riscv64/fdk_aac_fake/libaacenc2.a
AACENC_TARGET_S = $(CUR)/musl_riscv64/fdk_aac_fake/libaacenc2.so

AACDEC_TARGET = $(CUR)/musl_riscv64/fdk_aac_fake/libaacdec2.a
AACDEC_TARGET_S = $(CUR)/musl_riscv64/fdk_aac_fake/libaacdec2.so

AACCOMM_TARGET = $(CUR)/musl_riscv64/fdk_aac_fake/libaaccomm2.a
AACCOMM_TARGET_S = $(CUR)/musl_riscv64/fdk_aac_fake/libaaccomm2.so

AACSBRENC_TARGET = $(CUR)/musl_riscv64/fdk_aac_fake/libaacsbrenc2.a
AACSBRENC_TARGET_S = $(CUR)/musl_riscv64/fdk_aac_fake/libaacsbrenc2.so

AACSBRDEC_TARGET = $(CUR)/musl_riscv64/fdk_aac_fake/libaacsbrdec2.a
AACSBRDEC_TARGET_S = $(CUR)/musl_riscv64/fdk_aac_fake/libaacsbrdec2.so
endif


ARFLAGS = rcs
ARFLAGS_S = -shared -fPIC -o
CFLAGS = -fPIC -c -Os
CFLAGS += -DCVI_MODIFIED
CFLAGS += -D__FILENAME__=\"$(notdir $<)\"
CFLAGS += -Wall

AAC_TEST_LIB = $(AAC_LDLIBS)
AAC_TEST_LIB += -lcvi_audio -lcvi_vqe -lcvi_VoiceEngine -lcvi_RES
USE_TINYALSA = yes
ifeq ($(USE_ALSA), yes)
AAC_TEST_LIB += -lasound
endif
ifeq ($(USE_TINYALSA), yes)
AAC_TEST_LIB += -ltinyalsa
endif
AAC_TEST_LIB +=  -lcvi_AEC -laec

BUILDLIST=
#ifeq ($(USE_QUICK_AACDEC_BUILD), yes)
#BUILDLIST += TARGET_PREBUILD
#else
BUILDLIST += $(AACENC_TARGET)
BUILDLIST += $(AACDEC_TARGET)
BUILDLIST += $(AACCOMM_TARGET)
BUILDLIST += $(AACSBRENC_TARGET)
BUILDLIST += $(AACSBRDEC_TARGET)
##BUILDLIST += $(AAC_TEST)
#endif

OBJECTPATHS_ENCSRC=$(addprefix $(PWD)/,$(notdir $(OBJS_ENC)))
OBJECTPATHS_DECSRC=$(addprefix $(PWD)/,$(notdir $(OBJS_DEC)))
OBJECTPATHS_COMMSRC=$(addprefix $(PWD)/,$(notdir $(OBJS_COMM)))
OBJECTPATHS_SBRENCSRC=$(addprefix $(PWD)/,$(notdir $(OBJS_SBRENC)))
OBJECTPATHS_SBRDECSRC=$(addprefix $(PWD)/,$(notdir $(OBJS_SBRDEC)))

all:$(BUILDLIST)


$(OBJS_ENC):
	$(CC) $(AACENC_SRC) $(CFLAGS) $(AACENC_INCS)

$(OBJS_DEC):
	$(CC) $(AACDEC_SRC) $(CFLAGS) $(AACDEC_INCS)

$(OBJS_COMM):
	$(CC) $(AACCOMM_SRC) $(CFLAGS) $(AACCOMM_INCS)

$(OBJS_SBRENC):
	$(CC) $(AACSBRENC_SRC) $(CFLAGS) $(AACENC_INCS)

$(OBJS_SBRDEC):
	$(CC) $(AACSBRDEC_SRC) $(CFLAGS) $(AACDEC_INCS)



$(AACENC_TARGET) : $(OBJS_ENC)
	$(AR) $(ARFLAGS) $(AACENC_TARGET) $(OBJECTPATHS_ENCSRC)
	$(CC) $(ARFLAGS_S)  $(AACENC_TARGET_S) $(OBJECTPATHS_ENCSRC)
	rm -rf  $(OBJECTPATHS_ENCSRC)

$(AACDEC_TARGET) : $(OBJS_DEC)
	$(AR) $(ARFLAGS) $(AACDEC_TARGET) $(OBJECTPATHS_DECSRC)
	$(CC) $(ARFLAGS_S)  $(AACDEC_TARGET_S) $(OBJECTPATHS_DECSRC)
	rm -rf  $(OBJECTPATHS_DECSRC)


$(AACCOMM_TARGET) : $(OBJS_COMM)
	$(AR) $(ARFLAGS) $(AACCOMM_TARGET) $(OBJECTPATHS_COMMSRC)
	$(CC) $(ARFLAGS_S)  $(AACCOMM_TARGET_S) $(OBJECTPATHS_COMMSRC)
	$(STRIP) $(AACCOMM_TARGET_S)
	rm -rf  $(OBJECTPATHS_COMMSRC)

$(AACSBRENC_TARGET) : $(OBJS_SBRENC)
	$(AR) $(ARFLAGS) $(AACSBRENC_TARGET) $(OBJECTPATHS_SBRENCSRC)
	$(CC) $(ARFLAGS_S)  $(AACSBRENC_TARGET_S) $(OBJECTPATHS_SBRENCSRC)
	$(STRIP) $(AACSBRENC_TARGET_S)
	rm -rf  $(OBJECTPATHS_SBRENCSRC)

$(AACSBRDEC_TARGET) : $(OBJS_SBRDEC)
	$(AR) $(ARFLAGS) $(AACSBRDEC_TARGET) $(OBJECTPATHS_SBRDECSRC)
	$(CC) $(ARFLAGS_S)  $(AACSBRDEC_TARGET_S) $(OBJECTPATHS_SBRDECSRC)
	$(STRIP) $(AACSBRDEC_TARGET_S)
	rm -rf  $(OBJECTPATHS_SBRDECSRC)

$(AAC_TEST):
	$(CC) -o   $@ $(AAC_LDFLAGS) $(AAC_TEST_SRC) $(AAC_TEST_LIB) $(AAC_CFLAGS)


.PHONY : clean
clean:
	rm -f    $(OBJECTPATHS_ENCSRC) $(OBJECTPATHS_DECSRC) $(OBJECTPATHS_COMMSRC) $(OBJECTPATHS_ENCSRC) $(OBJECTPATHS_SBRDECSRC) $(AAC_TEST)




