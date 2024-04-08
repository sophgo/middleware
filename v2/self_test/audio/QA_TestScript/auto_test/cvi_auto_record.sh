#!/bin/sh
log_result="/tmp/acap_log.txt"
tmp_result="/tmp/acap_curr_log.txt"
output_result="/tmp/acap_result.txt"
pass_keyword=TEST-PASS
pass_count=0;

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



function aud_acap_test()
{
    echo "[------------aud_acap_test-----start-----]" >> $output_result

    echo "ACAP_ONE_CHN_DATA:" >> $output_result

	_pre_set_tmp_result
	for sr in 8000 16000 32000
	do
		CMD="./cvi_auto_record -r ${sr} -c 2 -p 320 -n 4 -T 5 -R ${sr} -v 0 -o auto1_${sr}_2chn.raw"
		${CMD} | tee -a $log_result $tmp_result
		_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result

		CMD="./cvi_auto_record -r ${sr} -c 1 -p 320 -n 4 -T 5 -R ${sr} -v 0 -o auto1_${sr}_1chn.raw"
		${CMD} | tee -a $log_result $tmp_result
		_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result

	done


	#echo "ACAP_TWO_CHN_DATA:" >> $output_result

	#for sr in 8000 16000 32000
	#do
	#	CMD="./cvi_auto_record -r ${sr} -c 2 -p 320 -n 4 -T 5 --numChn=2"
    #     CMD="${CMD} --chn=0 -v 0 -R ${sr} -o auto2_one${sr}_2chn.raw"
    #     CMD="${CMD} --chn=1 -v 0 -R ${sr} -o auto2_two${sr}_2chn.raw"

	# 	${CMD} | tee -a $log_result $tmp_result
	# 	_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result

	# 	CMD="./cvi_auto_record -r ${sr} -c 1 -p 320 -n 4 -T 5 --numChn=2"
    #     CMD="${CMD} --chn=0 -v 0 -R ${sr} -o auto2_one${sr}_1chn.raw"
    #     CMD="${CMD} --chn=1 -v 0 -R ${sr} -o auto2_two${sr}_1chn.raw"

	# 	${CMD} | tee -a $log_result $tmp_result
	# 	_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result

	# done

	# echo "ACAP_THREE_CHN_DATA:" >> $output_result
	# for sr in 8000 16000 32000
	# do
	# 	CMD="./cvi_auto_record -r ${sr} -c 2 -p 320 -n 4 -T 5 --numChn=3"
    #     CMD="${CMD} --chn=0 -v 0 -R ${sr} -o auto3_one${sr}_2chn.raw"
    #     CMD="${CMD} --chn=1 -v 0 -R ${sr} -o auto3_two${sr}_2chn.raw"
    #     CMD="${CMD} --chn=2 -v 0 -R ${sr} -o auto3_three${sr}_2chn.raw"

	# 	${CMD} | tee -a $log_result $tmp_result
	# 	_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result


	# 	CMD="./cvi_auto_record -r ${sr} -c 1 -p 320 -n 4 -T 5 --numChn=3"
    #     CMD="${CMD} --chn=0 -v 0 -R ${sr} -o auto3_one${sr}_1chn.raw"
    #     CMD="${CMD} --chn=1 -v 0 -R ${sr} -o auto3_two${sr}_1chn.raw"
    #     CMD="${CMD} --chn=2 -v 0 -R ${sr} -o auto3_three${sr}_1chn.raw"

	# 	${CMD} | tee -a $log_result $tmp_result
	# 	_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result

	# done
    audio_leave_test
    echo "[------------aud_acap_test------end------]" >> $output_result
}



function aud_airesample_test()
{
    echo "[------------aud_airesample_test-----start-----]" >> $output_result

    echo "ACAP_ONE_CHN_RESAMPLE:" >> $output_result

	for AoSr in 8000 16000 32000
	do
		echo "${AoSr}_TO_OTHRE_SR:" >> $output_result
		for sr in 8000 16000 32000
		do
        CMD="./cvi_auto_record -r $AoSr -c 2 -p 320 -n 4 -T 5 -R ${sr} -v 0 -o Res1${AoSr}to_${sr}_2chn.raw"
        _pre_set_tmp_result
        ${CMD} | tee -a $log_result $tmp_result
        _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result

        CMD="./cvi_auto_record -r $AoSr -c 1 -p 320 -n 4 -T 5 -R ${sr} -v 0 -o Res1${AoSr}to_${sr}_1chn.raw"
        _pre_set_tmp_result
        ${CMD} | tee -a $log_result $tmp_result
        _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result

		done
	done

    audio_leave_test
	echo "ACAP_TWO_CHN_RESAMPLE:" >> $output_result

	for AoSr in 8000 16000 32000
	do
		echo "${AoSr}_TO_OTHRE_SR:" >> $output_result

		for sr in 8000 16000 32000
		do
            for two_sr in 8000 16000 32000
            do
            CMD="./cvi_auto_record -r ${AoSr} -c 2 -p 320 -n 4 -T 5 --numChn=2"
            CMD="${CMD} --chn=0 -v 0 -R ${sr} -o Res2${AoSr}to_${sr}_2chn.raw"
            CMD="${CMD} --chn=1 -v 0 -R ${two_sr} -o Res2${AoSr}to_${two_sr}_two_2chn.raw"

			_pre_set_tmp_result
			${CMD} | tee -a $log_result $tmp_result
			_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result

            CMD="./cvi_auto_record -r ${AoSr} -c 1 -p 320 -n 4 -T 5 --numChn=2"
            CMD="${CMD} --chn=0 -v 0 -R ${sr} -o Res2${AoSr}to_${sr}_1chn.raw"
            CMD="${CMD} --chn=1 -v 0 -R ${two_sr} -o Res2${AoSr}to_${two_sr}_two_1chn.raw"

			_pre_set_tmp_result
			${CMD} | tee -a $log_result $tmp_result
			_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
            done
		done

	done

    audio_leave_test
	echo "ACAP_THREE_CHN_RESAMPLE:" >> $output_result

	for AoSr in 8000 16000 32000
	do
		echo "${AoSr}_TO_OTHRE_SR:" >> $output_result

		for sr in 8000 16000 32000
		do
            for two_sr in 8000 16000 32000
		    do
            for thr_sr in 8000 16000 32000
		    do
            CMD="./cvi_auto_record -r ${AoSr} -c 2 -p 320 -n 4 -T 5 --numChn=3"
            CMD="${CMD} --chn=0 -v 0 -R ${sr} -o Res3${AoSr}to_${sr}_2chn.raw"
            CMD="${CMD} --chn=1 -v 0 -R ${two_sr} -o Res3${AoSr}to_${two_sr}_two_2chn.raw"
            CMD="${CMD} --chn=2 -v 0 -R ${thr_sr} -o Res3${AoSr}to_${thr_sr}_three_2chn.raw"

			_pre_set_tmp_result
			${CMD} | tee -a $log_result $tmp_result
			_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result


            CMD="./cvi_auto_record -r ${AoSr} -c 1 -p 320 -n 4 -T 5 --numChn=3"
            CMD="${CMD} --chn=0 -v 0 -R ${sr} -o Res3${AoSr}to_${sr}_1chn.raw"
            CMD="${CMD} --chn=1 -v 0 -R ${two_sr} -o Res3${AoSr}to_${two_sr}_two_1chn.raw"
            CMD="${CMD} --chn=2 -v 0 -R ${thr_sr} -o Res3${AoSr}to_${thr_sr}_three_1chn.raw"

			_pre_set_tmp_result
			${CMD} | tee -a $log_result $tmp_result
			_check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
            done
            done
		done

	done
    audio_leave_test
    echo "[------------aud_aoresample_test------end------]" >> $output_result
}



function aud_getaenc_data()
{
    echo "[------------aud_getaenc_data-----start-----]" >> $output_result
    echo "GET_AENC_DATA:" >> $output_result
    echo "ONE_CHN:" >> $output_result
    echo "8000:" >> $output_result

    period_size=320

    for chn in 1 2
    do
        for fm in g726 adpcm g711a g711u aac
        do
        for bm in 0 1
        do
            if [ $fm = aac ]; then
                period_size=1024
            else
                period_size=320
            fi
        CMD="./cvi_auto_record -r 8000 -c ${chn} -p ${period_size} -n 4 -T 5 -R 8000 -f ${fm} -b ${bm}"
        CMD="${CMD} -o b${bm}_8k_${chn}chn.${fm}"
        _pre_set_tmp_result
        ${CMD} | tee -a $log_result $tmp_result
        _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
        done
        done
    done

    echo "16000:" >> $output_result
    for chn in 1 2
    do
        for fm in g711a g711u aac
        do
        for bm in 0 1
        do
            if [ $fm = aac ]; then
                period_size=1024
            else
                period_size=320
            fi
        CMD="./cvi_auto_record -r 16000 -c ${chn} -p ${period_size} -n 4 -T 5 -R 16000 -f ${fm} -b ${bm}"
        CMD="${CMD} -o b${bm}16k_${chn}chn.${fm}"
        _pre_set_tmp_result
        ${CMD} | tee -a $log_result $tmp_result
        _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
        done
        done
    done


    for sr in 32000
    do
        echo "${sr}:" >> $output_result
        for chn in 1 2
        do  for fm in aac
            do
            for bm in 0 1
            do
                if [ $fm = aac ]; then
                    period_size=1024
                else
                    period_size=320
                fi
            CMD="./cvi_auto_record -r ${sr} -c ${chn} -p ${period_size} -n 4 -T 5 -R ${sr} -f ${fm} -b ${bm}"
            CMD="${CMD} -o b${bm}${sr}_${chn}chn.${fm}"
            _pre_set_tmp_result
            ${CMD} | tee -a $log_result $tmp_result
            _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
            done
            done
        done
    done
    audio_leave_test
    echo "TWO_CHN:" >> $output_result
    echo "8000:" >> $output_result
    for chn in 1 2
    do
        for one_fm in g726 raw
        do
            for two_fm in adpcm g711a g711u aac
            do
                for bm1 in 0 1
                do
                for bm2 in 0 1
                do
                    if [ $two_fm = aac ]; then
                        period_size=1024
                    else
                        period_size=320
                    fi
                CMD="./cvi_auto_record -r 8000 -c ${chn} -p ${period_size} -n 4 -T 5 --numChn=2 "
                CMD="${CMD} --chn=0 -R 8000 -f ${one_fm} -b ${bm1} -o ${bm1}_8k_${chn}chn.${one_fm}"
                CMD="${CMD} --chn=1 -R 8000 -f ${two_fm} -b ${bm2} -o ${bm2}_8k_${chn}chn.${two_fm}"

                _pre_set_tmp_result
                ${CMD} | tee -a $log_result $tmp_result
                _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
                done
                done
            done
        done
    done


    for chn in 1 2
    do
        one_fm=adpcm
            for two_fm in g726 g711a g711u aac
            do
                for bm1 in 0 1
                do
                for bm2 in 0 1
                do
                    if [ $two_fm = aac ]; then
                        period_size=1024
                    else
                        period_size=320
                    fi
                CMD="./cvi_auto_record -r 8000 -c ${chn} -p ${period_size} -n 4 -T 5 --numChn=2 "
                CMD="${CMD} --chn=0 -R 8000 -f ${one_fm} -b ${bm1} -o ${bm1}_8k_${chn}chn.${one_fm}"
                CMD="${CMD} --chn=1 -R 8000 -f ${two_fm} -b ${bm2} -o ${bm2}_8k_${chn}chn.${two_fm}"

                _pre_set_tmp_result
                ${CMD} | tee -a $log_result $tmp_result
                _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
                done
                done
            done
    done

    for chn in 1 2
    do
        one_fm=g711a
            for two_fm in g726 adpcm g711u aac
            do
                for bm1 in 0 1
                do
                for bm2 in 0 1
                do
                    if [ $two_fm = aac ]; then
                        period_size=1024
                    else
                        period_size=320
                    fi
                CMD="./cvi_auto_record -r 8000 -c ${chn} -p ${period_size} -n 4 -T 5 --numChn=2 "
                CMD="${CMD} --chn=0 -R 8000 -f ${one_fm} -b ${bm1} -o ${bm1}_8k_${chn}chn.${one_fm}"
                CMD="${CMD} --chn=1 -R 8000 -f ${two_fm} -b ${bm2} -o ${bm2}_8k_${chn}chn.${two_fm}"

                _pre_set_tmp_result
                ${CMD} | tee -a $log_result $tmp_result
                _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
                done
                done
            done
    done

    for chn in 1 2
    do
        one_fm=g711u
            for two_fm in g726 adpcm g711a aac
            do
                for bm1 in 0 1
                do
                for bm2 in 0 1
                do
                    if [ $two_fm = aac ]; then
                        period_size=1024
                    else
                        period_size=320
                    fi
                CMD="./cvi_auto_record -r 8000 -c ${chn} -p ${period_size} -n 4 -T 5 --numChn=2 "
                CMD="${CMD} --chn=0 -R 8000 -f ${one_fm} -b ${bm1} -o ${bm1}_8k_${chn}chn.${one_fm}"
                CMD="${CMD} --chn=1 -R 8000 -f ${two_fm} -b ${bm2} -o ${bm2}_8k_${chn}chn.${two_fm}"

                _pre_set_tmp_result
                ${CMD} | tee -a $log_result $tmp_result
                _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
                done
                done
            done
    done

    for chn in 1 2
    do
        one_fm=aac
            for two_fm in g726 adpcm g711a g711u
            do
                for bm1 in 0 1
                do
                for bm2 in 0 1
                do
                    if [ $one_fm = aac ]; then
                        period_size=1024
                    else
                        period_size=320
                    fi
                CMD="./cvi_auto_record -r 8000 -c ${chn} -p ${period_size} -n 4 -T 5 --numChn=2 "
                CMD="${CMD} --chn=0 -R 8000 -f ${one_fm} -b ${bm1} -o ${bm1}_8k_${chn}chn.${one_fm}"
                CMD="${CMD} --chn=1 -R 8000 -f ${two_fm} -b ${bm2} -o ${bm2}_8k_${chn}chn.${two_fm}"

                _pre_set_tmp_result
                ${CMD} | tee -a $log_result $tmp_result
                _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
                done
                done
            done
    done

    echo "16000:" >> $output_result
    for chn in 1 2
    do
        for one_fm in g711a raw
        do
            for two_fm in g711u aac
            do
                for bm1 in 0 1
                do
                for bm2 in 0 1
                do
                    if [ $two_fm = aac ]; then
                        period_size=1024
                    else
                        period_size=320
                    fi
                CMD="./cvi_auto_record -r 16000 -c ${chn} -p ${period_size} -n 4 -T 5 --numChn=2 "
                CMD="${CMD} --chn=0 -R 16000 -f ${one_fm} -b ${bm1} -o b${bm1}16k_${chn}chn.${one_fm}"
                CMD="${CMD} --chn=1 -R 16000 -f ${two_fm} -b ${bm2} -o b${bm1}16k_${chn}chn.${two_fm}"

                _pre_set_tmp_result
                ${CMD} | tee -a $log_result $tmp_result
                _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
                done
                done
            done
        done
    done


    for chn in 1 2
    do
        for one_fm in g711u raw
        do
            for two_fm in g711a aac
            do
                for bm1 in 0 1
                do
                for bm2 in 0 1
                do
                    if [ $two_fm = aac ]; then
                        period_size=1024
                    else
                        period_size=320
                    fi
                CMD="./cvi_auto_record -r 16000 -c ${chn} -p ${period_size} -n 4 -T 5 --numChn=2 "
                CMD="${CMD} --chn=0 -R 16000 -f ${one_fm} -b ${bm1} -o b${bm1}16k_${chn}chn.${one_fm}"
                CMD="${CMD} --chn=1 -R 16000 -f ${two_fm} -b ${bm2} -o b${bm1}16k_${chn}chn.${two_fm}"

                _pre_set_tmp_result
                ${CMD} | tee -a $log_result $tmp_result
                _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
                done
                done
            done
        done
    done

    for chn in 1 2
    do
        for one_fm in aac raw
        do
            for two_fm in g711u g711a
            do
                for bm1 in 0 1
                do
                for bm2 in 0 1
                do
                    if [ $one_fm = aac ]; then
                        period_size=1024
                    else
                        period_size=320
                    fi
                CMD="./cvi_auto_record -r 16000 -c ${chn} -p ${period_size} -n 4 -T 5 --numChn=2 "
                CMD="${CMD} --chn=0 -R 16000 -f ${one_fm} -b ${bm1} -o b${bm1}16k_${chn}chn.${one_fm}"
                CMD="${CMD} --chn=1 -R 16000 -f ${two_fm} -b ${bm2} -o b${bm1}16k_${chn}chn.${two_fm}"

                _pre_set_tmp_result
                ${CMD} | tee -a $log_result $tmp_result
                _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
                done
                done
            done
        done
    done

    for sr in 32000
    do
        echo "${sr}:" >> $output_result
        for chn in 1 2
        do
            for bm1 in 0 1
            do
            for bm2 in 0 1
            do

            CMD="./cvi_auto_record -r ${sr} -c ${chn} -p 1024 -n 4 -T 5 --numChn=2 "
            CMD="${CMD} --chn=0 -R ${sr} -f raw -b ${bm1} -o unb2_${sr}_${chn}chn.raw"
            CMD="${CMD} --chn=1 -R ${sr} -f aac -b ${bm2} -o unb2_${sr}_${chn}chn.aac"

            _pre_set_tmp_result
            ${CMD} | tee -a $log_result $tmp_result
            _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
            done
            done
        done
    done

    audio_leave_test
    echo "THREE_CHN:" >> $output_result
    echo "8000:" >> $output_result
    for chn in 1 2
    do
        one_fm=g726
        two_fm=adpcm
        for thr_fm in g711a g711u aac
        do
            for bm1 in 0 1
            do
            for bm2 in 0 1
            do
            for bm3 in 0 1
            do
                if [ $thr_fm = aac ]; then
                    period_size=1024
                else
                    period_size=320
                fi
            CMD="./cvi_auto_record -r 8000 -c ${chn} -p ${period_size} -n 4 -T 5 --numChn=3 "
            CMD="${CMD} --chn=0 -R 8000 -f ${one_fm} -b ${bm1} -o unb3_8k_${chn}chn.${one_fm}"
            CMD="${CMD} --chn=1 -R 8000 -f ${two_fm} -b ${bm2} -o unb3_8k_${chn}chn.${two_fm}"
            CMD="${CMD} --chn=2 -R 8000 -f ${thr_fm} -b ${bm3} -o unb3_8k_${chn}chn.${thr_fm}"

            _pre_set_tmp_result
            ${CMD} | tee -a $log_result $tmp_result
            _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
            done
            done
            done
        done
    done

    for chn in 1 2
    do
        one_fm=g711a
        two_fm=g711u
        for thr_fm in aac raw
        do
            for bm1 in 0 1
            do
            for bm2 in 0 1
            do
            for bm3 in 0 1
            do
                if [ $thr_fm = aac ]; then
                    period_size=1024
                else
                    period_size=320
                fi
            CMD="./cvi_auto_record -r 8000 -c ${chn} -p ${period_size} -n 4 -T 5 --numChn=3 "
            CMD="${CMD} --chn=0 -R 8000 -f ${one_fm} -b ${bm1} -o unb3_8k_${chn}chn.${one_fm}"
            CMD="${CMD} --chn=1 -R 8000 -f ${two_fm} -b ${bm2} -o unb3_8k_${chn}chn.${two_fm}"
            CMD="${CMD} --chn=2 -R 8000 -f ${thr_fm} -b ${bm3} -o unb3_8k_${chn}chn.${thr_fm}"

            _pre_set_tmp_result
            ${CMD} | tee -a $log_result $tmp_result
            _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
            done
            done
            done
        done
    done


    echo "16000:" >> $output_result
    for chn in 1 2
    do
        one_fm=g711a
        two_fm=g711u
        for thr_fm in aac raw
        do

            for bm1 in 0 1
            do
            for bm2 in 0 1
            do
            for bm3 in 0 1
            do
                if [ $thr_fm = aac ]; then
                    period_size=1024
                else
                    period_size=320
                fi
            CMD="./cvi_auto_record -r 16000 -c ${chn} -p ${period_size} -n 4 -T 5 --numChn=3 "
            CMD="${CMD} --chn=0 -R 16000 -f ${one_fm} -b ${bm1} -o unb3_16k_${chn}chn.${one_fm}"
            CMD="${CMD} --chn=1 -R 16000 -f ${two_fm} -b ${bm2} -o unb3_16k_${chn}chn.${two_fm}"
            CMD="${CMD} --chn=2 -R 16000 -f ${thr_fm} -b ${bm3} -o unb3_16k_${chn}chn.${thr_fm}"

            _pre_set_tmp_result
            ${CMD} | tee -a $log_result $tmp_result
            _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result
            done
            done
            done
        done
    done

    for sr in 32000
    do
        echo "${sr}:" >> $output_result
        for chn in 1 2
        do
            fm=raw
            two_fm=raw
            for bm1 in 0 1
            do
            for bm2 in 0 1
            do
            for bm3 in 0 1
            do

            CMD="./cvi_auto_record -r ${sr} -c ${chn} -p 1024 -n 4 -T 5 --numChn=3 "
            CMD="${CMD} --chn=0 -R ${sr} -f ${fm} -b ${bm1} -o unb3_${sr}_${chn}chn.${fm}"
            CMD="${CMD} --chn=1 -R ${sr} -f ${two_fm} -b ${bm2} -o unb3_${sr}_${chn}chn.${two_fm}"
            CMD="${CMD} --chn=2 -R ${sr} -f aac -b ${bm3} -o unb3_${sr}_${chn}chn.aac"

            _pre_set_tmp_result
            ${CMD} | tee -a $log_result $tmp_result
            _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result

            done
            done
            done
        done
    done
    audio_leave_test
    echo "[------------aud_getaenc_data------end------]" >> $output_result
}





function aud_vqe_test()
{
    echo "[------------aud_vqe_test-----start-----]" >> $output_result
    echo "VQE_ONE_CHN:" >> $output_result

    echo "PCM:" >> $output_result
    period_size=320

	for sr in 8000 16000
	do
		CMD="./cvi_auto_record -r ${sr} -c 2 -p 320 -n 4 -T 5 -R ${sr} -v 1 -o vqe_${sr}_record.raw"
        _pre_set_tmp_result
		${CMD} | tee -a $log_result $tmp_result
        _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result

	done

    for fm in g726 adpcm
    do
        echo "${fm}:" >> $output_result
        for bm in 0 1
        do
        CMD="./cvi_auto_record -r 8000 -c 2 -p 320 -n 4 -T 5 -R 8000 -v 1 -f ${fm} -b ${bm} -o vqe8kb${bm}.${fm} "
        _pre_set_tmp_result
        ${CMD} | tee -a $log_result $tmp_result
        _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result

        done
    done

    for fm in g711a g711u aac
    do
        echo "${fm}:" >> $output_result
        for bm in 0 1
        do
            for sr in 8000 16000
            do
                if [ $fm = aac ]; then
                    period_size=1024
                else
                    period_size=320
                fi
            CMD="./cvi_auto_record -r ${sr} -c 2 -p ${period_size} -n 4 -T 5 -R ${sr} -v 1 -f ${fm} -b ${bm}"
            CMD="${CMD} -o vqe${sr}b${bm}.${fm}"
            _pre_set_tmp_result
            ${CMD} | tee -a $log_result $tmp_result
            _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result

            done
        done
    done
    audio_leave_test
    echo "VQE_TWO_CHN:" >> $output_result
        echo "pcm" >> $output_result
        for sr in 8000 16000
        do
            CMD="./cvi_auto_record -r ${sr} -c 2 -p 320 -n 4 -T 5 --numChn=2 "
            CMD="${CMD} --chn=0 -v 1 -R ${sr} -o vqe2_${sr}.raw"
            CMD="${CMD} --chn=1 -v 0 -R ${sr} -o vqe2_two_${sr}.raw"
            _pre_set_tmp_result
            ${CMD} | tee -a $log_result $tmp_result
            _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result

        done


    for fm in g726 adpcm
    do
        echo "${fm}" >> $output_result
        for bm in 0 1
        do
            for sr in 8000
            do

            CMD="./cvi_auto_record -r ${sr} -c 2 -p 320 -n 4 -T 5 --numChn=2 "
            CMD="${CMD} --chn=0 -v 0 -R ${sr} -o vqe2_one_${fm}${sr}.raw"
            CMD="${CMD} --chn=1 -v 1 -R ${sr} -f ${fm} -b ${bm} -o vqe2two_${sr}_b${bm}.${fm}"
            _pre_set_tmp_result
            ${CMD} | tee -a $log_result $tmp_result
            _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result

            CMD="./cvi_auto_record -r ${sr} -c 2 -p 320 -n 4 -T 5 --numChn=2 "
            CMD="${CMD} --chn=0 -v 1 -R ${sr} -o vqe2_one__${fm}${sr}.raw"
            CMD="${CMD} --chn=1 -v 0 -R ${sr} -f ${fm} -b ${bm} -o vqe2two_${sr}_b${bm}.${fm}"
            _pre_set_tmp_result
            ${CMD} | tee -a $log_result $tmp_result
            _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result

            done
        done
    done



    for fm in g711a g711u aac
    do
        echo "${fm}" >> $output_result
        for bm in 0 1
        do
            for sr in 8000 16000
            do
                if [ $fm = aac ]; then
                    period_size=1024
                else
                    period_size=320
                fi
            CMD="./cvi_auto_record -r ${sr} -c 2 -p ${period_size} -n 4 -T 5 --numChn=2 "
            CMD="${CMD} --chn=0 -v 0 -R ${sr} -o vqe2_one_${fm}${sr}.raw"
            CMD="${CMD} --chn=1 -v 1 -R ${sr} -f ${fm} -b ${bm} -o vqe2two_${sr}_b${bm}.${fm}"
            _pre_set_tmp_result
            ${CMD} | tee -a $log_result $tmp_result
            _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result

            CMD="./cvi_auto_record -r ${sr} -c 2 -p ${period_size} -n 4 -T 5 --numChn=2 "
            CMD="${CMD} --chn=0 -v 1 -R ${sr} -o vqe2_one_${fm}${sr}.raw"
            CMD="${CMD} --chn=1 -v 0 -R ${sr} -f ${fm} -b ${bm} -o vqe2two_${sr}_b${bm}.${fm}"
            _pre_set_tmp_result
            ${CMD} | tee -a $log_result $tmp_result
            _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result

            done
        done
    done

    audio_leave_test
    echo "[------------aud_vqe_test------end------]" >> $output_result
}

function aud_specialRec_test()
{
    CMD="./cvi_auto_special 1"
    _pre_set_tmp_result
    ${CMD} | tee -a $log_result $tmp_result
    _check_keyworkd_exist2 $tmp_result $pass_keyword "${CMD}" $output_result $log_result

}

function main_function()
{

	#echo "audio basic unit test-------------------start" >> $output_result
	case "$1" in
		"AudAi")
			echo "Audio Unit Test:AudAi"
			aud_acap_test

		;;
        "AiResample")
            echo "Audio Test:AiResample"
            aud_airesample_test
        ;;
        "AudAenc")
			echo "Audio Unit Test:AudAenc"
			aud_getaenc_data
		;;
        "AudVqe")
			echo "Audio Unit Test:AudVqe"
			aud_vqe_test
		;;
        "AudSpecialTest")
			echo "Audio Unit Test:AudSpecialTest"
			aud_specialRec_test
		;;
		"unitest")
			echo "Audio Unit: run all script"
            aud_specialRec_test
			aud_acap_test
            aud_airesample_test
			aud_getaenc_data
            aud_vqe_test
		;;
		*)
			echo "[Aud Script input Error]You have failed to specify what to do correctly."
			exit 1
			;;
	esac
	#echo "audio_downlink_test.....................end" >> $output_result

	audio_leave_test
    if [[ "$pass_count" == 6 ]]; then
		echo "[ALL TEST-PASS, PASS-COUNT=$pass_count]" 
	else
		echo "[NG]"
	fi
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
