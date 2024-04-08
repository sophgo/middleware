#!/bin/sh
log_result="/tmp/aplay_log.txt"
tmp_result="/tmp/aplay_curr_log.txt"
output_result="/tmp/aplay_result.txt"
pass_keyword=TEST-PASS
res_dir="res"
pass_count=0

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

function _pre_set_tmp_result()
{
#	if [ -f $1 ]; then
#	rm $1
#	fi

	if [ -f $tmp_result ]; then
	rm $tmp_result
	fi

	[ -e $tmp_result ] && rm $tmp_result
}

function _check_file_exist()
{
#$1 file_name to check exist and size greater 0
#$2 the command
#$3 result file name
#$4 log file name
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
		echo "[TEST-PASS]" $2 >> $3
		echo "[TEST-PASS]" $2 >> $4
	else
		echo "[NG]" $2 >> $3
		echo "[NG]" $2 >> $4
	fi
}

function _check_keyworkd_exist2()
{
#$1 filename
#$2 keyword
#$3 function name
#$4 result file name
#$5 log file name
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
		echo "[TEST-PASS]" $3 >> $4
		echo "[TEST-PASS]" $3 >> $5
		pass_count=$((pass_count+1))
	else
		echo "[NG]" $3 >> $4
		echo "[NG]" $3 >> $5
	fi
}


function aud_getpcm_data()
{

    echo "[------------aud_getpcm_data-----start-----]" >> $output_result
    echo "GET_PCM_DATA:" >> $output_result


	for sr in 8000 16000 32000
	do
		_pre_set_tmp_result
		CMD="./cvi_auto_record -r ${sr} -c 2 -p 320 -n 4 -T 5 -R ${sr} -o record.raw"
		${CMD} | tee -a $log_result $tmp_result
		_check_file_exist "record.raw" "${CMD}" $output_result $log_result
		mv record.raw auto_${sr}_2chn.raw

		_pre_set_tmp_result
		CMD="./cvi_auto_record -r ${sr} -c 1 -p 320 -n 4 -T 5 -R ${sr} -o record.raw"
		${CMD} | tee -a $log_result $tmp_result
		_check_file_exist "record.raw" "${CMD}" $output_result $log_result
		mv record.raw auto_${sr}_1chn.raw

	done


	for sr in 44100
	do
		_pre_set_tmp_result
		CMD="./cvi_auto_record -r 8000 -c 2 -p 320 -n 4 -T 5 -R ${sr} -o record.raw"
		${CMD} | tee -a $log_result $tmp_result
		_check_file_exist "record.raw" "${CMD}" $output_result $log_result
		mv record.raw auto_${sr}_2chn.raw

		_pre_set_tmp_result
		CMD="./cvi_auto_record -r 8000 -c 1 -p 320 -n 4 -T 5 -R ${sr} -o record.raw"
		${CMD} | tee -a $log_result $tmp_result
		_check_file_exist "record.raw" "${CMD}" $output_result $log_result
		mv record.raw auto_${sr}_1chn.raw

	done


	echo "[------------aud_getpcm_data------end------]" >> $output_result
}



function aud_aplay_test()
{
    echo "[------------aud_aplay_test-----start-----]" >> $output_result

    echo "PLAY_ONE_CHN_DATA:" >> $output_result

	for sr in 8000 16000 32000 44100
	do
		for chn in 1 2
		do
		CMD="./cvi_auto_play -r ${sr} -c ${chn} -p 320 -n 4"
		CMD="${CMD} -i ${res_dir}/auto_${sr}_2chn.raw -C 2 -b 0 -R ${sr}"

		_pre_set_tmp_result
		${CMD} | tee -a $log_result $tmp_result
		_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result

		CMD="./cvi_auto_play -r ${sr} -c ${chn} -p 320 -n 4"
		CMD="${CMD} -i ${res_dir}/auto_${sr}_1chn.raw -C 1 -b 0 -R ${sr}"

		_pre_set_tmp_result
		${CMD} | tee -a $log_result $tmp_result
		_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
		done
	done


	# echo "PLAY_TWO_CHN_DATA:" >> $output_result

	# for sr in 8000 16000 32000 44100
	# do
	# 	for chn1 in 1 2
	# 	do
	# 	for chn2 in 1 2
	# 	do
	# 	CMD="./cvi_auto_play -r ${sr} -c 2 -p 320 -n 4 --numChn=2"
	# 	CMD="${CMD} --chn=0 -b 0 -R ${sr} -C ${chn1} -i ${res_dir}/auto_${sr}_${chn1}chn.raw"
	# 	CMD="${CMD} --chn=1 -b 0 -R ${sr} -C ${chn2} -i ${res_dir}/auto_${sr}_${chn2}chn.raw"

	# 	_pre_set_tmp_result
	# 	${CMD} | tee -a $log_result $tmp_result
	# 	_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result


	# 	CMD="./cvi_auto_play -r ${sr} -c 1 -p 320 -n 4 --numChn=2"
	# 	CMD="${CMD} --chn=0 -b 0 -R ${sr} -C ${chn1} -i ${res_dir}/auto_${sr}_${chn1}chn.raw"
	# 	CMD="${CMD} --chn=1 -b 0 -R ${sr} -C ${chn2} -i ${res_dir}/auto_${sr}_${chn2}chn.raw"

	# 	_pre_set_tmp_result
	# 	${CMD} | tee -a $log_result $tmp_result
	# 	_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
	# 	done
	# 	done
	# done

	# echo "PLAY_THREE_CHN_DATA:" >> $output_result
	# for sr in 8000 16000 32000 44100
	# do
	# 	for chn1 in 1 2
	# 	do
	# 	for chn2 in 1 2
	# 	do
	# 	for chn3 in 1 2
	# 	do
	# 	CMD="./cvi_auto_play -r ${sr} -c 2 -p 320 -n 4 --numChn=3"
	# 	CMD="${CMD} --chn=0 -b 0 -R ${sr} -C ${chn1} -i ${res_dir}/auto_${sr}_${chn1}chn.raw"
	# 	CMD="${CMD} --chn=1 -b 0 -R ${sr} -C ${chn2} -i ${res_dir}/auto_${sr}_${chn2}chn.raw"
	# 	CMD="${CMD} --chn=2 -b 0 -R ${sr} -C ${chn3} -i ${res_dir}/auto_${sr}_${chn3}chn.raw"

	# 	_pre_set_tmp_result
	# 	${CMD} | tee -a $log_result $tmp_result
	# 	_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}"  $output_result $log_result


	# 	CMD="./cvi_auto_play -r ${sr} -c 1 -p 320 -n 4 --numChn=3"
	# 	CMD="${CMD} --chn=0 -b 0 -R ${sr} -C ${chn1} -i ${res_dir}/auto_${sr}_${chn1}chn.raw"
	# 	CMD="${CMD} --chn=1 -b 0 -R ${sr} -C ${chn2} -i ${res_dir}/auto_${sr}_${chn2}chn.raw"
	# 	CMD="${CMD} --chn=2 -b 0 -R ${sr} -C ${chn3} -i ${res_dir}/auto_${sr}_${chn3}chn.raw"

	# 	_pre_set_tmp_result
	# 	${CMD} | tee -a $log_result $tmp_result
	# 	_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
	# 	done
	# 	done
	# 	done
	# done

    echo "[------------aud_aplay_test------end------]" >> $output_result
}



function aud_aoresample_test()
{
    echo "[------------aud_aoresample_test-----start-----]" >> $output_result

    echo "PLAY_ONE_CHN_RESAMPLE:" >> $output_result

	for AoSr in 8000 16000 32000 44100
	do
		echo "OTHRE_SR_TO_${AoSr}:" >> $output_result
		for sr in 8000 16000 32000 44100
		do
		CMD="./cvi_auto_play -r $AoSr -c 2 -p 320 -n 4  -i ${res_dir}/auto_${sr}_2chn.raw -b 0 -C 2 -R ${sr}"

		_pre_set_tmp_result
		${CMD} | tee -a $log_result $tmp_result
		_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result

		CMD="./cvi_auto_play -r $AoSr -c 1 -p 320 -n 4  -i ${res_dir}/auto_${sr}_1chn.raw -b 0 -C 1 -R ${sr}"

		_pre_set_tmp_result
		${CMD} | tee -a $log_result $tmp_result
		_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result

		done
	done


	echo "PLAY_TWO_CHN_RESAMPLE:" >> $output_result

	for AoSr in 8000 16000 32000 44100
	do
		echo "OTHRE_SR_TO_${AoSr}:" >> $output_result

		for sr in 8000 16000 32000 44100
		do
			for two_sr in 8000 16000 32000 44100
			do
			CMD="./cvi_auto_play -r ${AoSr} -c 2 -p 320 -n 4 --numChn=2"
			CMD="${CMD} --chn=0 -b 0 -R ${sr} -C 2 -i ${res_dir}/auto_${sr}_2chn.raw"
			CMD="${CMD} --chn=1 -b 0 -R ${two_sr} -C 2 -i ${res_dir}/auto_${two_sr}_2chn.raw"

			_pre_set_tmp_result
			${CMD} | tee -a $log_result $tmp_result
			_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result


			CMD="./cvi_auto_play -r ${AoSr} -c 1 -p 320 -n 4 --numChn=2"
			CMD="${CMD} --chn=0 -b 0 -R ${sr} -C 1 -i ${res_dir}/auto_${sr}_1chn.raw"
			CMD="${CMD} --chn=1 -b 0 -R ${two_sr} -C 1 -i ${res_dir}/auto_${two_sr}_1chn.raw"

			_pre_set_tmp_result
			${CMD} | tee -a $log_result $tmp_result
			_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
			done
		done

	done

	echo "PLAY_THREE_CHN_RESAMPLE:" >> $output_result

	for AoSr in 8000 16000 32000 44100
	do
		echo "OTHRE_SR_TO_${AoSr}:" >> $output_result

		for sr in 8000 16000 32000 44100
		do
			for two_sr in 8000 16000 32000 44100
			do
			for thr_sr in 8000 16000 32000 44100
			do
			CMD="./cvi_auto_play -r ${AoSr} -c 2 -p 320 -n 4 --numChn=3"
			CMD="${CMD} --chn=0 -b 0 -R ${sr} -C 2 -i ${res_dir}/auto_${sr}_2chn.raw"
			CMD="${CMD} --chn=1 -b 0 -R ${two_sr} -C 2 -i ${res_dir}/auto_${two_sr}_2chn.raw"
			CMD="${CMD} --chn=2 -b 0 -R ${thr_sr} -C 2 -i ${res_dir}/auto_${thr_sr}_2chn.raw"

			_pre_set_tmp_result
			${CMD} | tee -a $log_result $tmp_result
			_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result


			CMD="./cvi_auto_play -r ${AoSr} -c 1 -p 320 -n 4 --numChn=3"
			CMD="${CMD} --chn=0 -b 0 -R ${sr} -C 1 -i ${res_dir}/auto_${sr}_1chn.raw"
			CMD="${CMD} --chn=1 -b 0 -R ${two_sr} -C 1 -i ${res_dir}/auto_${two_sr}_1chn.raw"
			CMD="${CMD} --chn=2 -b 0 -R ${thr_sr} -C 1 -i ${res_dir}/auto_${thr_sr}_1chn.raw"

			_pre_set_tmp_result
			${CMD} | tee -a $log_result $tmp_result
			_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
			done
			done
		done

	done

    echo "[------------aud_aoresample_test------end------]" >> $output_result
}



function aud_getadec_data()
{
    echo "[------------aud_getadec_data-----start-----]" >> $output_result
    echo "GET_ADEC_DATA:" >> $output_result

	for chn in 1 2
	do

	CMD="./cvi_auto_record -r 8000 -c ${chn} -p 320 -n 4 -T 5 -R 8000 -f g726 -b 0 -o out.g726"
	_pre_set_tmp_result
	${CMD} | tee -a $log_result $tmp_result
	_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
	mv out.g726 out_${chn}chn.g726

	done


	for chn in 1 2
	do
		for sr in 8000 16000
		do
		CMD="./cvi_auto_record -r ${sr} -c ${chn} -p 320 -n 4 -T 5 -R ${sr} -f g711a -b 0 -o out.g711a"
		_pre_set_tmp_result
		${CMD} | tee -a $log_result $tmp_result
		_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
		mv out.g711a out_${sr}_${chn}chn.g711a
		done
	done

	for chn in 1 2
	do
		for sr in 8000 16000
		do
		CMD="./cvi_auto_record -r ${sr} -c ${chn} -p 320 -n 4 -T 5 -R ${sr} -f g711u -b 0 -o out.g711u"
		_pre_set_tmp_result
		${CMD} | tee -a $log_result $tmp_result
		_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
		mv out.g711u out_${sr}_${chn}chn.g711u
		done
	done



	for chn in 1 2
	do

	CMD="./cvi_auto_record -r 8000 -c ${chn} -p 320 -n 4 -T 5 -R 8000 -f adpcm -b 0 -o out.adpcm"
	_pre_set_tmp_result
	${CMD} | tee -a $log_result $tmp_result
	_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
	mv out.adpcm out_${chn}chn.adpcm

	done


	for chn in 1 2
	do
		for sr in 8000 16000 32000
		do
		CMD="./cvi_auto_record -r ${sr} -c ${chn} -p 1024 -n 4 -T 5 -R ${sr} -f aac -b 0 -o out.aac"
		_pre_set_tmp_result
		${CMD} | tee -a $log_result $tmp_result
		_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
		mv out.aac out_${sr}_${chn}chn.aac
		done
	done

	for chn in 1 2
	do
		for sr in 44100
		do
		CMD="./cvi_auto_record -r 8000 -c ${chn} -p 1024 -n 4 -T 5 -R ${sr} -f aac -b 0 -o out.aac"
		_pre_set_tmp_result
		${CMD} | tee -a $log_result $tmp_result
		_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
		mv out.aac out_${sr}_${chn}chn.aac
		done
	done




	echo "[------------aud_getadec_data------end------]" >> $output_result
}


function aud_adec_test()
{
#g726 only support 8k
#g711a/u only support 8k/16k

    echo "[------------aud_adec_test-----start-----]" >> $output_result

    echo "G726:" >> $output_result

	for bm in 0 1
	do
		if [ ${bm} -eq 0 ]; then
			echo "G726:UNBIND_MODE:" >> $output_result
		else
			echo "G726:BIND_MODE:" >> $output_result
		fi

		for chn in 1 2
		do
			echo "G726:${chn}_CHN:" >> $output_result

		CMD="./cvi_auto_play -r 8000 -c ${chn} -p 320 -n 4"
		CMD="${CMD} -i ${res_dir}/out_${chn}chn.g726 -C ${chn} -b ${bm} -R 8000"

		_pre_set_tmp_result
		${CMD} | tee -a $log_result $tmp_result
		_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result

		CMD_TWO_CHN="./cvi_auto_play -r 8000 -c ${chn} -p 320 -n 4 --numChn=2"
		CMD_TWO_CHN="${CMD_TWO_CHN} --chn=0 -b ${bm} -R 8000 -C ${chn} -i ${res_dir}/out_${chn}chn.g726"
		CMD_TWO_CHN="${CMD_TWO_CHN} --chn=1 -b ${bm} -R 8000 -C ${chn} -i ${res_dir}/out_${chn}chn.g726"
		_pre_set_tmp_result
		${CMD_TWO_CHN}| tee -a $log_result $tmp_result
		_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD_TWO_CHN}" $output_result $log_result


		CMD_THREE_CHN="./cvi_auto_play -r 8000 -c ${chn} -p 320 -n 4 --numChn=3"
		CMD_THREE_CHN="${CMD_THREE_CHN} --chn=0 -b ${bm} -R 8000 -C ${chn} -i ${res_dir}/out_${chn}chn.g726"
		CMD_THREE_CHN="${CMD_THREE_CHN} --chn=1 -b ${bm} -R 8000 -C ${chn} -i ${res_dir}/out_${chn}chn.g726"
		CMD_THREE_CHN="${CMD_THREE_CHN} --chn=2 -b ${bm} -R 8000 -C ${chn} -i ${res_dir}/out_${chn}chn.g726"
		_pre_set_tmp_result
		${CMD_THREE_CHN} | tee -a $log_result $tmp_result
		_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD_THREE_CHN}" $output_result $log_result

		done

	done

    echo "G711A:" >> $output_result

	for bm in 0 1
	do
		if [ ${bm} -eq 0 ]; then
			echo "G711A:UNBIND_MODE:" >> $output_result
		else
			echo "G711A:BIND_MODE:" >> $output_result
		fi

		for chn in 1 2
		do
			echo "G711A:${chn}_CHN:" >> $output_result
			for sr in 8000 16000
			do
			CMD="./cvi_auto_play -r ${sr} -c ${chn} -p 320 -n 4 -i ${res_dir}/out_${sr}_${chn}chn.g711a"
			CMD="${CMD} -C ${chn} -b ${bm} -R ${sr}"
			_pre_set_tmp_result
			${CMD} | tee -a $log_result $tmp_result
			_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result


			CMD_TWO_CHN="./cvi_auto_play -r ${sr} -c ${chn} -p 320 -n 4 --numChn=2"
			CMD_TWO_CHN="${CMD_TWO_CHN} --chn=0 -b ${bm} -R ${sr} -C ${chn}"
			CMD_TWO_CHN="${CMD_TWO_CHN} -i ${res_dir}/out_${sr}_${chn}chn.g711a"

			CMD_TWO_CHN="${CMD_TWO_CHN} --chn=1 -b ${bm} -R ${sr} -C ${chn}"
			CMD_TWO_CHN="${CMD_TWO_CHN} -i ${res_dir}/out_${sr}_${chn}chn.g711a"
			_pre_set_tmp_result
			${CMD_TWO_CHN}| tee -a $log_result $tmp_result
			_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD_TWO_CHN}" $output_result $log_result


			CMD_THREE_CHN="./cvi_auto_play -r ${sr} -c ${chn} -p 320 -n 4 --numChn=3"
			CMD_THREE_CHN="${CMD_THREE_CHN} --chn=0 -b ${bm} -C ${chn} -R ${sr}"
			CMD_THREE_CHN="${CMD_THREE_CHN} -i ${res_dir}/out_${sr}_${chn}chn.g711a"

			CMD_THREE_CHN="${CMD_THREE_CHN} --chn=1 -b ${bm} -C ${chn} -R ${sr}"
			CMD_THREE_CHN="${CMD_THREE_CHN} -i ${res_dir}/out_${sr}_${chn}chn.g711a"

			CMD_THREE_CHN="${CMD_THREE_CHN} --chn=2 -b ${bm} -C ${chn} -R ${sr}"
			CMD_THREE_CHN="${CMD_THREE_CHN} -i ${res_dir}/out_${sr}_${chn}chn.g711a"
			_pre_set_tmp_result
			${CMD_THREE_CHN} | tee -a $log_result $tmp_result
			_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD_THREE_CHN}" $output_result $log_result

			done
		done
	done

    echo "G711U:" >> $output_result
	for bm in 0 1
	do
		if [ ${bm} -eq 0 ]; then
			echo "G711U:UNBIND_MODE:" >> $output_result
		else
			echo "G711U:BIND_MODE:" >> $output_result
		fi

		for chn in 1 2
		do
			echo "G711U:${chn}_CHN:" >> $output_result
			for sr in 8000 16000
			do
			CMD="./cvi_auto_play -r ${sr} -c ${chn} -p 320 -n 4 -i ${res_dir}/out_${sr}_${chn}chn.g711u"
			CMD="${CMD} -C ${chn} -b ${bm} -R ${sr}"
			_pre_set_tmp_result
			${CMD} | tee -a $log_result $tmp_result
			_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result

			CMD_TWO_CHN="./cvi_auto_play -r ${sr} -c ${chn} -p 320 -n 4 --numChn=2"
			CMD_TWO_CHN="${CMD_TWO_CHN} --chn=0 -b ${bm} -C ${chn} -R ${sr}"
			CMD_TWO_CHN="${CMD_TWO_CHN} -i ${res_dir}/out_${sr}_${chn}chn.g711u"

			CMD_TWO_CHN="${CMD_TWO_CHN} --chn=1 -b ${bm} -C ${chn} -R ${sr}"
			CMD_TWO_CHN="${CMD_TWO_CHN} -i ${res_dir}/out_${sr}_${chn}chn.g711u"

			_pre_set_tmp_result
			${CMD_TWO_CHN}| tee -a $log_result $tmp_result
			_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD_TWO_CHN}" $output_result $log_result


			CMD_THREE_CHN="./cvi_auto_play -r ${sr} -c ${chn} -p 320 -n 4 --numChn=3"
			CMD_THREE_CHN="${CMD_THREE_CHN} --chn=0 -b ${bm} -C ${chn} -R ${sr}"
			CMD_THREE_CHN="${CMD_THREE_CHN} -i ${res_dir}/out_${sr}_${chn}chn.g711u"

			CMD_THREE_CHN="${CMD_THREE_CHN} --chn=1 -b ${bm} -C ${chn} -R ${sr}"
			CMD_THREE_CHN="${CMD_THREE_CHN} -i ${res_dir}/out_${sr}_${chn}chn.g711u"

			CMD_THREE_CHN="${CMD_THREE_CHN} --chn=2 -b ${bm} -C ${chn} -R ${sr}"
			CMD_THREE_CHN="${CMD_THREE_CHN} -i ${res_dir}/out_${sr}_${chn}chn.g711u"
			_pre_set_tmp_result
			${CMD_THREE_CHN} | tee -a $log_result $tmp_result
			_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD_THREE_CHN}" $output_result $log_result

			done
		done
	done

	echo "ADPCM:" >> $output_result

	for bm in 0 1
	do
		if [ ${bm} -eq 0 ]; then
			echo "ADPCM:UNBIND_MODE:" >> $output_result
		else
			echo "ADPCM:BIND_MODE:" >> $output_result
		fi

		for chn in 1 2
		do
			echo "ADPCM:${chn}_CHN:" >> $output_result

		CMD="./cvi_auto_play -r 8000 -c ${chn} -p 320 -n 4"
		CMD="${CMD} -i ${res_dir}/out_${chn}chn.adpcm -C ${chn} -b ${bm} -R 8000"
		_pre_set_tmp_result
		${CMD} | tee -a $log_result $tmp_result
		_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result

		CMD_TWO_CHN="./cvi_auto_play -r 8000 -c ${chn} -p 320 -n 4 --numChn=2"
		CMD_TWO_CHN="${CMD_TWO_CHN} --chn=0 -b ${bm} -R 8000 -C ${chn} -i ${res_dir}/out_${chn}chn.adpcm"
		CMD_TWO_CHN="${CMD_TWO_CHN} --chn=1 -b ${bm} -R 8000 -C ${chn} -i ${res_dir}/out_${chn}chn.adpcm"
		_pre_set_tmp_result
		${CMD_TWO_CHN}| tee -a $log_result $tmp_result
		_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD_TWO_CHN}" $output_result $log_result


		CMD_THREE_CHN="./cvi_auto_play -r 8000 -c ${chn} -p 320 -n 4 --numChn=3"
		CMD_THREE_CHN="${CMD_THREE_CHN} --chn=0 -b ${bm} -C ${chn} -R 8000 -i ${res_dir}/out_${chn}chn.adpcm"
		CMD_THREE_CHN="${CMD_THREE_CHN} --chn=1 -b ${bm} -C ${chn} -R 8000 -i ${res_dir}/out_${chn}chn.adpcm"
		CMD_THREE_CHN="${CMD_THREE_CHN} --chn=2 -b ${bm} -C ${chn} -R 8000 -i ${res_dir}/out_${chn}chn.adpcm"
		_pre_set_tmp_result
		${CMD_THREE_CHN} | tee -a $log_result $tmp_result
		_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD_THREE_CHN}" $output_result $log_result

		done

	done

	echo "AAC:" >> $output_result

	for bm in 0 1
	do
		if [ ${bm} -eq 0 ]; then
			echo "AAC:UNBIND_MODE:" >> $output_result
		else
			echo "AAC:BIND_MODE:" >> $output_result
		fi

		for chn in 1 2
		do
			echo "AAC:${chn}_CHN:" >> $output_result
			for sr in 8000 16000 32000 44100
			do
			CMD="./cvi_auto_play -r ${sr} -c ${chn} -p 320 -n 4  -i ${res_dir}/out_${sr}_${chn}chn.aac"
			CMD="${CMD} -C ${chn} -b ${bm} -R ${sr}"
			_pre_set_tmp_result
			${CMD} | tee -a $log_result $tmp_result
			_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result

#			CMD_TWO_CHN="./cvi_auto_play -r ${sr} -c ${chn} -p 320 -n 4 --numChn=2"
#			CMD_TWO_CHN="${CMD_TWO_CHN} --chn=0 -b ${bm} -R ${sr} -C ${chn} -i out_${sr}_${chn}chn.aac"
#			CMD_TWO_CHN="${CMD_TWO_CHN} --chn=1 -b ${bm} -R ${sr} -C ${chn} -i out_${sr}_${chn}chn.aac"
#			_pre_set_tmp_result
#			${CMD_TWO_CHN}| tee -a $log_result $tmp_result
#			_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD_TWO_CHN}" $output_result $log_result


#			CMD_THREE_CHN="./cvi_auto_play -r ${sr} -c ${chn} -p 320 -n 4 --numChn=3"
#			CMD_THREE_CHN="${CMD_THREE_CHN} --chn=0 -b ${bm} -C ${chn} -R ${sr} -i out_${sr}_${chn}chn.aac"
#			CMD_THREE_CHN="${CMD_THREE_CHN} --chn=1 -b ${bm} -C ${chn} -R ${sr} -i out_${sr}_${chn}chn.aac"
#			CMD_THREE_CHN="${CMD_THREE_CHN} --chn=2 -b ${bm} -C ${chn} -R ${sr} -i out_${sr}_${chn}chn.aac"
#			_pre_set_tmp_result
#			${CMD_THREE_CHN} | tee -a $log_result $tmp_result
#			_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD_THREE_CHN}" $output_result $log_result

			done
		done
	done


    echo "[------------aud_adec_test------end------]" >> $output_result
}

function aud_specialPlay_test()
{
	CMD="./cvi_auto_special 2"
	_pre_set_tmp_result
	${CMD} | tee -a $log_result $tmp_result
	_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
}

function main_function()
{

	#echo "audio basic unit test-------------------start" >> $output_result
	case "$1" in
		"AudAo")
			echo "Audio Unit Test:AudAo"
#			aud_getpcm_data
			aud_aplay_test

		;;
        "AoResample")
            echo "Audio Test:AoResample"
			aud_getpcm_data
            aud_aoresample_test
        ;;
		"AudDec")
			echo "Audio Unit Test:AudDec"
#			aud_getadec_data
			aud_adec_test

		;;
        "AudSpecialTest")
            echo "Audio Test:AudSpecialTest"
			aud_specialPlay_test
        ;;
		"unitest")
			echo "Audio Unit: run all script"
			aud_specialPlay_test
#			aud_getpcm_data
            aud_aoresample_test
#			aud_getadec_data
			aud_adec_test
		;;
		*)
			echo "[Aud Script input Error]You have failed to specify what to do correctly."
			exit 1
			;;
	esac
	if [[ "$pass_count" == 12 ]]; then
		echo "[ALL TEST-PASS, PASS-COUNT=$pass_count]" 
	else
		echo "[NG]"
	fi
	#echo "audio_downlink_test.....................end" >> $output_result
#	audio_leave_test
}




#----------------------------------start call
#audio test start---here
#export cviaudio_level=2
if [ -f $tmp_result ]; then
rm $tmp_result
fi

if [ -f $output_result ]; then
rm $output_result
fi

if [ -f $log_result ]; then
rm $log_result
fi

main_function $1
#cat $output_result
#----------------------------------stop call

