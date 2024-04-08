SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
     PARAM_FILE:=../../Makefile.param
     include $(PARAM_FILE)
endif

OPENSSL_PATH=../../3rdparty/openssl
SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I$(MW_INC) -I$(KERNEL_INC) -I$(SYS_INC) -I$(OPENSSL_PATH)/include -I.
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
ifeq ($(SDK_VER), 32bit)
LIBS = $(OPENSSL_PATH)/libcrypto-arm.a
OPT_LEVEL = -O3
else ifeq ($(SDK_VER), uclibc)
LIBS = $(OPENSSL_PATH)/libcrypto-uclibc.a
OPT_LEVEL = -O3
else ifeq ($(SDK_VER), glibc_riscv64)
LIBS = $(OPENSSL_PATH)/libcrypto-riscv64.a
OPT_LEVEL = -O3
else ifeq ($(SDK_VER), musl_riscv64)
LIBS = $(OPENSSL_PATH)/libcrypto-musl_riscv64.a
OPT_LEVEL = -O3
else
LIBS = $(OPENSSL_PATH)/libcrypto.a
OPT_LEVEL = -Os
endif
TARGET_A = $(abspath  $(MW_LIB)/libcipher.a)
TARGET_SO = $(abspath  $(MW_LIB)/libcipher.so)
TMP_A = cipher_tmp.a

EXTRA_CFLAGS = $(INCS)

.PHONY : clean all
all : $(TARGET_A) $(TARGET_SO)

CFLAGS += -ggdb $(OPT_LEVEL) -fno-omit-frame-pointer
CIPHER_LDFLAGS := $(filter-out --gc-sections,${LDFLAGS})
CIPHER_LDFLAGS +=  -ldl

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET_A): $(OBJS)
	@$(LD) -relocatable $(EXTRA_LDFLAGS) $(OBJS) $(LIBS) -o cipher.o
	@SYMS=$$(${NM} cipher.o | awk '$$3 ~ /./ { print $$3 }' | sort -u | grep -v '^CVI_' | sed 's/^/-L /'); \
		$(OBJCOPY) $$SYMS cipher.o cipher.o
	@$(AR) $(ARFLAGS) $(TARGET_A) cipher.o
	@echo -e $(YELLOW)[LINK]$(END)[$(notdir $(AR))] $(notdir $(TARGET_A))

$(TARGET_SO): $(OBJS)
	@$(CC) -Wl,--version-script=libcipher.version $(CIPHER_LDFLAGS) $(EXTRA_LDFLAGS) -o $(TARGET_SO) -Wl,--start-group $(OBJS) $(LIBS) -Wl,--end-group
	@echo -e $(GREEN)[LINK]$(END)[$(notdir $(LD))] $(notdir $(TARGET_SO))

clean:
	@rm -f $(OBJS) $(DEPS) $(TARGET_A) $(TARGET_SO) *.o

-include $(DEPS)
