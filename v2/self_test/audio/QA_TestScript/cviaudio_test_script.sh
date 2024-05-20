#!/bin/sh

#[utility function]-----------------------------------------------------------
tmp_result="/tmp/aud_tmp.txt"
output_result="/tmp/aud_result.txt"
pass_keyword=TEST-PASS

function _pre_set()
{
	find . -maxdepth 1 -type f -name "*.wav" -exec rm -rfv {} \;

	find . -maxdepth 1 -type f -name "*.raw" -exec rm -rfv {} \;

	if [ -f $tmp_result ]; then
	rm $tmp_result
	fi

	[ -e $tmp_result ] && rm $tmp_result
}

function _check_keyworkd_exist()
{
# $1 filename
# $2 keyword

	if grep -q "$2" "$1"
	then
		echo "keyword" $2 "exist"
		return 1
	else
		echo "keyword" $2  "not found"
		return 0
	fi
}

#------------------------------------------------------------
function _pre_set_record()
{
	if [ -f sample_record.raw ]; then
	rm sample_record.raw
	fi

	if [ -f $tmp_result ]; then
	rm $tmp_result
	fi

	[ -e $tmp_result ] && rm $tmp_result
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
		echo $2 "keyword TEST-PASS exist" >> $3
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
	if grep -q "$2" "$1"
	then
		echo "keyword" $2 "exist"
		check=1
	else
		echo "keyword" $2  "not found"
		check=0
	fi

	if [[ "$check" == 1 ]]; then
		echo $3 "keyword TEST-PASS exist" >> $4
	else
		echo $3 "NG" >> $4
	fi
}

function _audio_record_vqe_test()
{
	_pre_set_record
	./sample_audio_internal  19 --list -c 1 -r 8000 -p 320 -V 1
	_check_file_exist "sample_record.raw" "_audio_record_vqe_test"   $output_result

	_pre_set_record
	./sample_audio_internal  19 --list -c 2 -r 8000 -p 320 -V 1
	_check_file_exist "sample_record.raw" "_audio_record_vqe_test"  $output_result

	_pre_set_record
	./sample_audio_internal  19 --list -c 1 -r 16000 -p 320 -V 1
	_check_file_exist "sample_record.raw" "_audio_record_vqe_test"   $output_result

	_pre_set_record
	./sample_audio_internal  19 --list -c 2 -r 16000 -p 320 -V 1
	_check_file_exist "sample_record.raw" "_audio_record_vqe_test"   $output_result
}

function _pre_set_encode()
{
	find . -maxdepth 1 -type f -name "*.g726" -exec rm -rfv {} \;

	find . -maxdepth 1 -type f -name "*.g711a" -exec rm -rfv {} \;

	find . -maxdepth 1 -type f -name "*.g711u" -exec rm -rfv {} \;

	find . -maxdepth 1 -type f -name "*.adpcm" -exec rm -rfv {} \;

	[ -e $tmp_result ] && rm $tmp_result
}

function _audio_encode_bindmode_test()
{
	#mix bindmode with different codec/rate/channel/period
	_pre_set_encode
	./sample_audio_internal 1 --list -c 1 -r 8000 -p 480 -C 0 -T 10 -b 1
	_check_file_exist "fd_aenc_out_chn0.g726" "_audio_encode_bindmode_test"   $output_result

	_pre_set_encode
	./sample_audio_internal 1 --list -c 2 -r 16000 -p 480 -C 1 -T 10 -b 1
	_check_file_exist "fd_aenc_out_chn0.g711a" "_audio_encode_bindmode_test"   $output_result


	_pre_set_encode
	./sample_audio_internal  1 --list -c 1 -r 16000 -p 320  -C 2 -T 10 -b 1
	_check_file_exist "fd_aenc_out_chn0.g711u" "_audio_encode_bindmode_test"   $output_result

	_pre_set_encode
	./sample_audio_internal  1 --list -c 2 -r 8000 -p 320  -C 3 -T 10 -b 1
	_check_file_exist "fd_aenc_out_chn0.adpcm" "_audio_encode_bindmode_test"   $output_result
}

function _audio_encode_usermode_test()
{
	#mix bindmode with different codec/rate/channel/period
	_pre_set_encode
	./sample_audio_internal 1 --list -c 2 -r 8000 -p 480 -C 0 -T 10 -b 0
	_check_file_exist "fd_aenc_out_chn0.g726"  "_audio_encode_usermode_test"   $output_result

	_pre_set_encode
	./sample_audio_internal 1 --list -c 1 -r 16000 -p 480 -C 1 -T 10 -b 0
	_check_file_exist "fd_aenc_out_chn0.g711a" "_audio_encode_usermode_test"   $output_result


	_pre_set_encode
	./sample_audio_internal  1 --list -c 2 -r 16000 -p 320  -C 2 -T 10 -b 0
	_check_file_exist "fd_aenc_out_chn0.g711u" "_audio_encode_usermode_test"   $output_result

	_pre_set_encode
	./sample_audio_internal  1 --list -c 1 -r 8000 -p 320  -C 3 -T 10 -b 0
	_check_file_exist "fd_aenc_out_chn0.adpcm"  "_audio_encode_usermode_test"   $output_result
}

function _audio_encode_vqe_aec_test()
{
	_pre_set_encode
	./sample_audio_internal 1 --list -c 2 -r 8000 -p 480 -C 0 -T 10 -b 0 -V 1 -A 1
	_check_file_exist "fd_aenc_out_chn0.g726" "_audio_encode_vqe_aec_test"   $output_result

	_pre_set_encode
	./sample_audio_internal 1 --list -c 2 -r 16000 -p 480 -C 1 -T 10 -b 0 -V 1 -A 1
	_check_file_exist "fd_aenc_out_chn0.g711a" "_audio_encode_vqe_aec_test"   $output_result

	#check with resample on
	_pre_set_encode
	./sample_audio_internal  1 --list -c 2 -r 16000 -p 320  -C 2 -T 10 -b 0 -V 1 -A 1 -R 1
	_check_file_exist "fd_aenc_out_chn0.g711u" "_audio_encode_vqe_aec_test"   $output_result

	_pre_set_encode
	./sample_audio_internal  1 --list -c 2 -r 8000 -p 320  -C 3 -T 10 -b 0 -V 1 -A 1 -R 1
	_check_file_exist "fd_aenc_out_chn0.adpcm" "_audio_encode_vqe_aec_test"   $output_result
}

function _audio_record_encode_test()
{
	_pre_set_encode
	./sample_audio_internal  1 --list -c 1 -r 8000 -p 320  -C 0 -T 10
	_check_file_exist "fd_aenc_out_chn0.g726" "_audio_record_encode_test"   $output_result

	_pre_set_encode
	./sample_audio_internal  1 --list -c 1 -r 8000 -p 320  -C 1 -T 10
	_check_file_exist "fd_aenc_out_chn0.g711a" "_audio_record_encode_test"   $output_result

	_pre_set_encode
	./sample_audio_internal  1 --list -c 1 -r 8000 -p 320  -C 2 -T 10
	_check_file_exist "fd_aenc_out_chn0.g711u" "_audio_record_encode_test"   $output_result

	_pre_set_encode
	./sample_audio_internal  1 --list -c 1 -r 8000 -p 320  -C 3 -T 10
	_check_file_exist "fd_aenc_out_chn0.adpcm" "_audio_record_encode_test"   $output_result
}

function _audio_record_test()
{
	_pre_set_record
	./sample_audio_internal  19 --list -c 1 -r 8000 -p 320
	_check_file_exist "sample_record.raw"  "_audio_record_test"   $output_result

	_pre_set_record
	./sample_audio_internal  19 --list -c 2 -r 8000 -p 320
	_check_file_exist "sample_record.raw" "_audio_record_test"   $output_result

	_pre_set_record
	./sample_audio_internal  19 --list -c 1 -r 16000 -p 320
	_check_file_exist "sample_record.raw" "_audio_record_test"   $output_result

	_pre_set_record
	./sample_audio_internal  19 --list -c 2 -r 16000 -p 320
	_check_file_exist "sample_record.raw" "_audio_record_test"   $output_result

	_pre_set_record
	./sample_audio_internal  19 --list -c 2 -r 32000 -p 480
	_check_file_exist "sample_record.raw" "_audio_record_test"   $output_result

	_pre_set_record
	./sample_audio_internal  19 --list -c 1 -r 32000 -p 480
	_check_file_exist "sample_record.raw" "_audio_record_test"   $output_result

	_pre_set_record
	./sample_audio_internal  19 --list -c 1 -r 48000 -p 480
	_check_file_exist "sample_record.raw" "_audio_record_test"   $output_result

	_pre_set_record
	./sample_audio_internal  19 --list -c 2 -r 48000 -p 480
	_check_file_exist "sample_record.raw" "_audio_record_test"   $output_result


}

function _audio_play_test()
{
	#prepare the file first-------------------
	_pre_set_record
	./sample_audio_internal  19 --list -c 1 -r 8000 -T 7 -p 320
	mv sample_record.raw record8k1ch.raw

	./sample_audio_internal  19 --list -c 1 -r 16000 -T 7 -p 320
	mv sample_record.raw record16k1ch.raw

	./sample_audio_internal  19 --list -c 1 -r 32000 -T 7 -p 320
	mv sample_record.raw record32k1ch.raw

	./sample_audio_internal  19 --list -c 1 -r 48000 -T 7 -p 320
	mv sample_record.raw record48k1ch.raw

	./sample_audio_internal  19 --list -c 2 -r 8000 -T 7 -p 320
	mv sample_record.raw record8k2ch.raw

	./sample_audio_internal  19 --list -c 2 -r 16000 -T 7 -p 320
	mv sample_record.raw record16k2ch.raw

	./sample_audio_internal  19 --list -c 2 -r 32000 -T 7 -p 320
	mv sample_record.raw record32k2ch.raw

	./sample_audio_internal  19 --list -c 2 -r 48000 -T 7 -p 320
	mv sample_record.raw record48k2ch.raw
	#start to playout file---------------------
	_pre_set_record
	./sample_audio_internal 20 --list --name record8k1ch.raw -c 1 -r 8000 -p 320 >> $tmp_result
	_check_keyworkd_exist2 $tmp_result "stop playing" "_audio_play_test_8k_1ch" $output_result

	_pre_set_record
	./sample_audio_internal 20 --list --name record8k2ch.raw -c 2 -r 8000 -p 320 >> $tmp_result
	_check_keyworkd_exist2 $tmp_result "stop playing" "_audio_play_test_8k_2ch" $output_result

	_pre_set_record
	./sample_audio_internal 20 --list --name record16k1ch.raw -c 1 -r 16000 -p 320 >> $tmp_result
	_check_keyworkd_exist2 $tmp_result "stop playing" "_audio_play_test_16k_1ch" $output_result

	_pre_set_record
	./sample_audio_internal 20 --list --name record16k2ch.raw -c 2 -r 16000 -p 320 >> $tmp_result
	_check_keyworkd_exist2 $tmp_result "stop playing" "_audio_play_test_16k_2ch" $output_result

	_pre_set_record
	./sample_audio_internal 20 --list --name record32k2ch.raw -c 2 -r 32000 -p 320 >> $tmp_result
	_check_keyworkd_exist2 $tmp_result "stop playing" "_audio_play_test_32k_2ch" $output_result

	_pre_set_record
	./sample_audio_internal 20 --list --name record48k2ch.raw -c 2 -r 48000 -p 320 >> $tmp_result
	_check_keyworkd_exist2 $tmp_result "stop playing" "_audio_play_test_48k_2ch" $output_result

}

function _audio_resample_play_test()
{
	_pre_set_encode
	./sample_audio_internal  1 --list -c 1 -r 8000 -p 320  -C 0 -T 10
	./sample_audio_internal  2 --list -c 1 -r 8000 -p 320  -C 0 -R 1 -T 20 >> $tmp_result
	_check_keyworkd_exist2 $tmp_result "TEST-PASS" "_audio_resample_play_test" $output_result
}

function unit_pcm_write_test()
{
	_pre_set
	./sample_audio_internal 9 -c 2 -r 16000 -t 10
	./sample_audio_internal 10 -c 2 -r 16000 -t 10 --name record.raw >> $tmp_result

	_check_keyworkd_exist $tmp_result $pass_keyword
	if [ "$?" = "1" ]
	then
		echo "unit_pcm_write_test TEST-PASS" >> $output_result
	else
		echo "unit_pcm_write_test NG" >> $output_result
	fi
	#sampe_audio 10 unit TEST-PASS
}

function unit_pcm_read_test()
{
	_pre_set
	./sample_audio_internal  9 -c 2 -r 8000  >> $tmp_result
	_check_keyworkd_exist $tmp_result "TEST-PASS"
	if [ "$?" = "1" ]
	then
		echo "unit_pcm_read_test"  "TEST-PASS" >> $output_result
	else
		echo "unit_pcm_read_test"  "NG" >> $output_result
	fi

	#sampe_audio 9 unit TEST-PASS
}

function unit_transcode_test()
{
	_pre_set
	#encode unit test
	./sample_audio_internal 9 -c 2 -r 16000 -t 10
	./sample_audio_transcode --unittest encode record.raw 3 >> $tmp_result
	_check_keyworkd_exist $tmp_result "TEST-PASS"
	if [ "$?" = "1" ]
	then
		echo "unit_transcode_test" "encode TEST-PASS" >> $output_result
	else
		echo "unit_transcode_test" "encode NG" >> $output_result
	fi
	#cvi_transcode unit TEST-PASS

	#decode unit test
	_pre_set
	./sample_audio_transcode --unittest decode record.raw.g726 3 >> $tmp_result
	_check_keyworkd_exist $tmp_result "TEST-PASS"
	if [ "$?" = "1" ]
	then
		echo "unit_transcode_test" "decode TEST-PASS" >> $output_result
	else
		echo "unit_transcode_test" "decode NG" >> $output_result
	fi
	#cvi_transcode unit TEST-PASS
}

function unit_resample_test()
{
	#record the file
	_pre_set
	./sample_audio_internal 9 -c 2 -r 16000 -t 10
	./sample_audio_resample record.raw 16000 48000 2  >> $tmp_result
	_check_keyworkd_exist $tmp_result "TEST-PASS"
	if [ "$?" = "1" ]
	then
		echo "unit_resample_test" "resample TEST-PASS" >> $output_result
	else
		echo "unit_resample_test" "resample NG" >> $output_result
	fi

	./sample_audio_resample record.raw 16000 32000 2
	_check_file_exist "outsample_file_32000.raw" "unit_resample_test_16_to_32" $output_result

	./sample_audio_resample record.raw 16000 44100 2
	_check_file_exist "outsample_file_32000.raw" "unit_resample_test_16_to_441" $output_result

	_pre_set
	./sample_audio_internal 9 -c 1 -r 8000 -t 10

	./sample_audio_resample record.raw 8000 16000 1
	_check_file_exist "outsample_file_16000.raw" "unit_resample_test_8_to_16" $output_result

	./sample_audio_resample record.raw 8000 32000 1
	_check_file_exist "outsample_file_32000.raw" "unit_resample_test_8_to_32" $output_result

	./sample_audio_resample record.raw 8000 48000 1
	_check_file_exist "outsample_file_48000.raw" "unit_resample_test_8_to_48" $output_result

	./sample_audio_resample record.raw 8000 44100 1
	_check_file_exist "outsample_file_44100.raw" "unit_resample_test_8_to_441" $output_result

}

function unit_aec_test()
{
	_pre_set
	./sample_audio_internal 19 --list -c 2 -r 8000 -p 320 -T 10
	mv sample_record.raw sample_record8k.raw
	./sample_audio_internal 19 --list -c 2 -r 16000 -p 320 -T 10
	mv sample_record.raw sample_record16k.raw

	./sample_audio_aec  sample_record8k.raw  aec_out8k.raw 8000 -default
	_check_file_exist "aec_out8k.raw" "unit_aec_test_8k"   $output_result

	./sample_audio_aec  sample_record16k.raw  aec_out16k.raw 16000 -default
	_check_file_exist "aec_out16k.raw" "unit_aec_test_16k"   $output_result

}

function unit_anr_test()
{
	_pre_set
	./sample_audio_internal 19 --list -c 1 -r 8000 -p 320 -T 10
	mv sample_record.raw sample_record8k.raw
	./sample_audio_internal 19 --list -c 1 -r 16000 -p 320 -T 10
	mv sample_record.raw sample_record16k.raw

	./sample_audio_nr  sample_record8k.raw 8000 -default
	_check_file_exist "NR_AGC_sample_record8k.raw.pcm" "unit_anr_test_8k" $output_result

	./sample_audio_nr  sample_record16k.raw 16000 -default
	_check_file_exist "NR_AGC_sample_record16k.raw.pcm" "unit_anr_test_16k" $output_result

}

function audio_downlink_test()
{
	_audio_play_test
	_audio_resample_play_test
}

function audio_leave_test()
{
	find . -maxdepth 1 -type f -name "*.wav" -exec rm -rfv {} \;
	find . -maxdepth 1 -type f -name "*.raw" -exec rm -rfv {} \;
	find . -maxdepth 1 -type f -name "*.pcm" -exec rm -rfv {} \;
	find . -maxdepth 1 -type f -name "*.g711a" -exec rm -rfv {} \;
	find . -maxdepth 1 -type f -name "*.g711u" -exec rm -rfv {} \;
	find . -maxdepth 1 -type f -name "*.g726" -exec rm -rfv {} \;
	find . -maxdepth 1 -type f -name "*.adpcm" -exec rm -rfv {} \;
	find . -maxdepth 1 -type f -name "*.aac" -exec rm -rfv {} \;
}

function audio_version_log()
{
	echo "libcvi_audio.so  version check" >> /tmp/aud_testVer.txt
	./sample_audio_internal 7  >> /tmp/aud_testVer.txt
	echo "cviaudio test script version 20211229D"
}

function main_function()
{
	audio_version_log
	#echo "audio basic unit test-------------------start" >> $output_result
	case "$1" in
		"AudIn")
			echo "Audio Unit Test:AudIn"
			unit_pcm_read_test
		;;
		"AudOut")
			echo "Audio Unit Test:AudOut"
			unit_pcm_write_test
		;;
		"AudTranscode")
			echo "Audio Unit Test:AudTranscode"
			unit_transcode_test
			;;
		"AudResample")
			echo "Audio Unit Test:AudResample"
			unit_resample_test
			;;
		"AudAEC")
			echo "Audio Unit Test:AudAEC"
			unit_aec_test
			;;
		"AudANR")
			echo "Audio Unit Test:AudANR"
			unit_anr_test
			;;
		"AudUpRecord")
			echo "Audio Uplink Test:AudUpRecord"
			_audio_record_test
			;;
		"AudUpVQE")
			echo "Audio Uplink Test:AudUpVQE"
			_audio_record_vqe_test
			;;
		"AudUpEnc")
			echo "Audio Uplink Test:AudUpEnc"
			_audio_record_encode_test
			;;
		"AudUpBind")
			echo "Audio Uplink Test:AudUpBind"
			_audio_encode_bindmode_test
			;;
		"AudUpUser")
			echo "Audio Uplink Test:AudUpUser"
			_audio_encode_usermode_test
			;;
		"AudUpVqeAec")
			echo "Audio Uplink Test:AudUpVqeAec"
			_audio_encode_vqe_aec_test
			;;
		"AudPlayOut")
			echo "Audio Downlink Test:AudPlayOut"
			_audio_play_test
			;;
		"AudResampleOut")
			echo "Audio Downlink Test:AudResampleOut"
			_audio_resample_play_test
			;;
		"self_test")
			echo "Audio self_test: run all script"
			unit_pcm_read_test
			unit_pcm_write_test
			unit_transcode_test
			unit_resample_test
			unit_aec_test
			unit_anr_test
			_audio_record_test
			_audio_record_vqe_test
			_audio_record_encode_test
			_audio_encode_bindmode_test
			_audio_encode_usermode_test
			_audio_encode_vqe_aec_test
			_audio_play_test
			_audio_resample_play_test
			;;
		*)
			echo "[Aud Script input Error]You have failed to specify what to do correctly."
			exit 1
			;;
	esac
	#echo "audio_downlink_test.....................end" >> $output_result
	audio_leave_test
}

function check_all_pass()
{
	if grep -q NG "$output_result" || ! grep -q TEST-PASS "$output_result"
	then
		echo $1 "TEST-FAILD, check file $output_result for more details"
	else
		echo $1 "TEST-ALL-PASS"
	fi
}

#----------------------------------start call
#audio test start---here
export cviaudio_level=2
if [ -f $tmp_result ]; then
rm $tmp_result
fi

if [ -f $output_result ]; then
rm $output_result
fi
main_function $1
cat $output_result
check_all_pass $1
#----------------------------------stop call
