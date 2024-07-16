include ../../Makefile.def
OUT = ../../bin
NAME = testEVO
SOURCES = testEvo.c

LIBS += Elementary vg_lite vg_lite_util
CFLAGS += -I./../Elementary/inc -I./../Elementary/src -I../util -I$(SDK_DIR)/inc
LDFLAGS += -L$(OUT) $(addprefix -l,$(LIBS)) -L $(SDK_DIR)/drivers
.PHONY:  all clean

all : $(OUT)/$(NAME)

clean :
	@rm -f $(OUT)/$(NAME)

$(OUT) :
	@mkdir $(OUT)

$(OUT)/libElementary.so :
	@make -C ../Elementary

$(OUT)/$(NAME) : $(OUT) $(SOURCES) $(OUT)/libElementary.so
	@echo $(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)
	@$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)
