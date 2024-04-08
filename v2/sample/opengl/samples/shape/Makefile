include ../../Makefile.def
OUT = ../../bin
NAME = shape
SOURCES = shape.c

LIBS += gl vg_lite vg_lite_util
CFLAGS += -I$(SDK_DIR)/inc -I$(MW_INC)
LDFLAGS += -L$(SDK_DIR)/drivers -L$(OUT) $(addprefix -l,$(LIBS))

all : $(OUT)/shape
	#@cp -f $(OUT)

clean :
	@rm -f $(OUT)/$(NAME) *.o

$(OUT) :
	@mkdir $(OUT)

$(OUT)/$(NAME) : $(OUT) $(SOURCES)
	@echo $(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)
	@$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)
