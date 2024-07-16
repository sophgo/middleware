include ../../Makefile.def
OUT = ../../bin
NAME = tiled
SOURCES = main.c

LIBS += vg_lite vg_lite_util
CFLAGS += -I$(SDK_DIR)/inc -I../util
LDFLAGS += -L$(SDK_DIR)/drivers -L$(OUT) $(addprefix -l,$(LIBS))

all : $(OUT)/$(NAME)
	@cp -f *.raw $(OUT)

clean :
	@rm -f $(OUT)/$(NAME) $(OUT)/icons

$(OUT) :
	@mkdir $(OUT)


$(OUT)/libvg_lite_util.so :
	@make -C ../util

$(OUT)/icons : icons
	@cp icons $@ -rf

$(OUT)/$(NAME) : $(OUT) $(SOURCES) $(OUT)/libvg_lite_util.so
	@echo $(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)
	@$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)
