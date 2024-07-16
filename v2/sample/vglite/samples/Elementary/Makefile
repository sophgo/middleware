include ../../Makefile.def
OUT = ../../bin
NAME = libElementary.so

SOURCES = \
          ./src/elm_buffer.c\
          ./src/elm_init.c \
          ./src/elm_draw.c \
          ./src/elm_object.c

LIBS += vg_lite vg_lite_util
CFLAGS +=-I$(SDK_DIR)/inc -I../util -I./inc
LDFLAGS += -fPIC -shared -Wl,-z,defs
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
