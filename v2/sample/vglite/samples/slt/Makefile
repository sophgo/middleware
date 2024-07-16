include ../../Makefile.def

OUT = ../../bin
NAME = slt_2de
SOURCES = ./src/main.c \
          ./src/Common.c \
          ./src/util.c         \
          ./src/Cases/Clear.c \
          ./src/Cases/Combination.c  \
          ./src/Cases/Draw_Image.c \
          ./src/Cases/Tessellation.c \
          ./src/Cases/Matrix.c \

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
