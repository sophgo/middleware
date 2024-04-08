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
include ../self_test.mk
include $(MW_PATH)/modules/audio/audio.mk

ts := $(shell /bin/date "+%Y-%m-%d---%H-%M-%S")
SDIR = $(PWD)

BUILDLIST = audio_aec_test
BUILDLIST += audio_anr_test
BUILDLIST += audio_pcm_test
BUILDLIST += audio_resample_test
BUILDLIST += audio_transcode_test
BUILDLIST += audio_auto_test_sample
#BUILDLIST += audio_aac_recorder
#BUILDLIST += audio_mp3_recorder

.PHONY: clean

all: pre_check $(BUILDLIST) post_check

pre_check:
	@echo [aduio self_test]start Timestamp is $(ts)

post_check:
	@echo [aduio self_test]end Timestamp is $(ts)

audio_aec_test:
	@echo "self_tet audio_aec"
	@pushd ./audio_aec/; make ; popd

audio_anr_test:
	@pushd $(SDIR)/audio_anr/; make ; popd

audio_pcm_test:
	@pushd $(SDIR)/audio_pcm/; make ; popd

audio_resample_test:
	@pushd $(SDIR)/audio_resample/; make ; popd

audio_transcode_test:
	@pushd $(SDIR)/audio_transcode/; make ; popd

audio_aac_recorder:
	@pushd $(SDIR)/cvi_aac_recorder/; make ; popd

audio_mp3_recorder:
	@pushd $(SDIR)/cvi_mp3_recorder/; make ; popd

audio_auto_test_sample:
	@pushd $(SDIR)/aud_auto_test_sample/; make ; popd

install:
	@echo "[audio step]COPY LIB!! "



clean:
	@pushd $(SDIR)/audio_aec/; make clean; popd
	@pushd $(SDIR)/audio_anr/; make clean; popd
	@pushd $(SDIR)/audio_pcm/; make clean; popd
	@pushd $(SDIR)/audio_resample/; make clean; popd
	@pushd $(SDIR)/audio_transcode/; make clean; popd
#	@pushd $(SDIR)/cvi_aac_recorder/; make clean; popd
	@pushd $(SDIR)/cvi_mp3_recorder/; make clean; popd
	@pushd $(SDIR)/aud_auto_test_sample/; make clean; popd
