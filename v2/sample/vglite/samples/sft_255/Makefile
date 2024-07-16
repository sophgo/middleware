include ../../Makefile.def

OUT = ../../bin
NAME = ft255
SOURCES = ./src/main.c \
          ./src/Common.c \
          ./src/util.c         \
          ./src/SFT_Cases/SFT_Clear.c \
          ./src/SFT_Cases/SFT_Blit.c  \
          ./src/SFT_Cases/SFT_Path_Draw.c

LIBS += vg_lite vg_lite_util
CFLAGS += -I$(SDK_DIR)/inc -I../util
CFLAGS += -D__LINUX__
LDFLAGS += -L$(SDK_DIR)/drivers -L$(OUT) $(addprefix -l,$(LIBS))

all : $(OUT)/$(NAME)

clean :
	@rm -f $(OUT)/$(NAME)

$(OUT) :
	@mkdir $(OUT)

$(OUT)/libvg_lite_util.so :
	@make -C ../util

$(OUT)/$(NAME) : $(OUT) $(SOURCES) $(OUT)/libvg_lite_util.so
	@echo $(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)
	@$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)
