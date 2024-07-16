
third_list := $(shell find ./ -maxdepth 2 -mindepth 2 -name "Makefile")
dep_list := ./vglite/Makefile
indep_list := $(filter-out ${dep_list}, ${third_list})

all:
	@echo "#################################################"
	@echo "#                                               #"
	@echo "#   Compiling '3rdparty libs' Configs as below  #"
	@echo "#                                               #"
	@echo "#################################################"
	@mkdir -p $(shell pwd)/../include/3rd
	@for x in ${dep_list}; \
	    do cd `dirname $$x`; if [ $$? ]; then make || exit 1; cd -; fi; done
	@for x in ${indep_list}; \
	    do cd `dirname $$x`; if [ $$? ]; then make || exit 1; cd -; fi; done

clean:
	@rm -rf $(shell pwd)/../include/3rd
	@for x in `find ./ -maxdepth 2 -mindepth 2 -name "Makefile" `; \
	    do cd `dirname $$x`; if [ $$? ]; then make clean; cd -; fi; done
