#!/bin/sh

#utility function
tmp_result="/tmp/aud_tmp.txt"
output_result="/tmp/aud_result.txt"
pass_keyword=TEST-PASS

function _check_keyworkd_exist()
{
# $1 filename
# $2 keyword

	if grep -q $2 "$1"
	then
		echo "keyword" $2 "exist"
		return 1
	else
		echo "keyword" $2  "not found"
		return 0
	fi
}

function _check_file_exist()
{
#$1 file_name to check exist and size greater 0
#$2 function name
#$3 result file name
	check=0
	minimumsize=0

	if [ -f "$1" ]; then
		check=1
	fi

	actualsize=$(wc -c <"$1")
	if [ $actualsize -gt $minimumsize ]; then
		if [[ "$check" == 1 ]]; then
			check=2
		fi
	fi

	if [[ "$check" == 2 ]]; then
		echo $2 "TEST_PASS" >> $3
	else
		echo $2 "NG" >> $3
	fi
}

function _check_keyworkd_exist2()
{
#$1 filename
#$2 keyword
#$3 function name
#$4 result file name
	check=0
	if grep -q $2 "$1"
	then
		echo "keyword" $2 "exist"
		check=1
	else
		echo "keyword" $2  "not found"
		check=0
	fi

	if [[ "$check" == 1 ]]; then
		echo $3 "TEST_PASS" >> $4
	else
		echo $3 "NG" >> $4
	fi
}


function audio_leave_test()
{
	find . -type f -name "*.wav" -exec rm -rfv {} \;
	find . -type f -name "*.raw" -exec rm -rfv {} \;
	find . -type f -name "*.pcm" -exec rm -rfv {} \;
	find . -type f -name "*.g711a" -exec rm -rfv {} \;
	find . -type f -name "*.g711u" -exec rm -rfv {} \;
	find . -type f -name "*.g726" -exec rm -rfv {} \;
	find . -type f -name "*.adpcm" -exec rm -rfv {} \;
	find . -type f -name "*.aac" -exec rm -rfv {} \;
}

function audio_version_log()
{
	echo "libcvi_audio.so  version check" >> /tmp/aud_testVer.txt
	./sample_audio_internal 7  >> /tmp/aud_testVer.txt
	echo "cviaudio performance script version 20211228"
}

function main_function()
{
	audio_version_log
	#echo "audio basic unit test-------------------start" >> $output_result
	#record a file first
	./sample_audio_internal 19 --list -c 2 -T 60 -r 16000 -p 320
	mv sample_record.raw sample_record_16k.raw


	#sample_audio_aec test by case
	./sample_audio_aec  QA_16k.qa_pcm aec_out16k.raw 16000 -default 1
	./sample_audio_aec  QA_16k.qa_pcm aec_out16k.raw 16000 -default 2
	./sample_audio_aec  QA_16k.qa_pcm  aec_out16k.raw 16000 -default 3
	./sample_audio_aec  QA_16k.qa_pcm aec_out16k.raw 16000 -default 4
	./sample_audio_aec  sample_record_16k.raw aec_out16k.raw 16000 -default 5
	./sample_audio_aec  sample_record_16k.raw aec_out16k.raw 16000 -default 6
	./sample_audio_aec  QA_16k.qa_pcm aec_out16k.raw 16000 -default 7
	./sample_audio_aec  QA_16k.qa_pcm aec_out16k.raw 16000 -default 8
	./sample_audio_aec  QA_16k.qa_pcm aec_out16k.raw 16000 -default 9
	./sample_audio_aec  QA_16k.qa_pcm aec_out16k.raw 16000 -default 10

	./sample_audio_transcode --unittest encode QA_8k.qa_pcm 1
	./sample_audio_transcode --unittest encode QA_8k.qa_pcm 2

	audio_leave_test
}

#----------------------------------start call
#audio test start---here
export cviaudio_level=0
if [ -f $tmp_result ]; then
rm $tmp_result
fi

if [ -f $output_result ]; then
rm $output_result
fi
main_function
cat $output_result
#----------------------------------stop call
