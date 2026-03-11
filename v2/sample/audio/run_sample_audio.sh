#!/bin/sh

export LD_LIBRARY_PATH=/mnt/sd/auto_test/lib:/mnt/sd/auto_test/lib/3rd:$LD_LIBRARY_PATH
export UT_BIN_DIR=$(pwd)

TEST_PASS="AUDIO-TEST-PASS"
TEST_FAIL="AUDIO-TEST-FAIL"
SAMPLE_PANEL_BIN=sample_audio
OUT_FILE="tmp_output"
INPUT_FILE="tmp_input"
result=$TEST_PASS
check_ret=0

function env_check() {
    if [ -z ${UT_BIN_DIR} ]; then
        echo "UT_BIN_DIR is not set";
        exit 1
    fi

    if [ ! -d "${UT_BIN_DIR}" ]; then
        echo "UT_BIN_DIR: [$UT_BIN_DIR] is not exist";
        exit 1
    fi
}

function sample_check() {
    sample_bin=${UT_BIN_DIR}/$1

    if [ ! -x ${sample_bin} ]; then
        echo "${sample_bin} is not exist";
        usage
        exit 1
    fi
}
function verify() {
    grep_result_i=$(grep "AudioInputThread exit" $OUT_FILE)
    grep_result_o=$(grep "AudioOutputThread out" $OUT_FILE)

    if [ -z "$grep_result_i" ] && [ -z "$grep_result_o" ]; then
        check_ret=-1
    fi
}

function verify_get_vol() {
    grep_result=$(grep "Get Volume Aout" $OUT_FILE)
    if [ -z "$grep_result" ]; then
        check_ret=-1
    fi

    grep_result=$(grep "Get Volume Ain" $OUT_FILE)
    if [ -z "$grep_result" ]; then
        check_ret=-1
    fi
}

function verify_set_vol() {
    grep_result=$(grep "SET VOLUME!...end" $OUT_FILE)

    if [ -z "$grep_result" ]; then
        check_ret=-1
    fi
}

function clean_tmp_files() {
    rm -rf $OUT_FILE $INPUT_FILE
}

env_check
touch $INPUT_FILE

function run_all_sample_audio() {
    cmds_list="'sample_audio 0 --list -D 0 -d 0 -r 8000 -R 8000 -c 0 -p 480 -C 0 -V 0 -F Cvi_8_8k_2chn.g726 -T 10'
        'sample_audio 0 --list -D 0 -d 0 -r 16000 -R 8000 -c 2 -p 320 -C 1 -V 0 -F Cvi_16_8k_2chn.g711a -T 10'
        'sample_audio 0 --list -D 0 -d 0 -r 32000 -R 8000 -c 2 -p 320 -C 2 -V 0 -F Cvi_32_8k_2chn.g711mu -T 10'
        'sample_audio 1 --list -D 0 -d 0 -r 16000 -R 8000 -c 2 -p 320 -C 0 -V 0 -F Cvi_16_8k_2chn.g726 -T 10'
        'sample_audio 1 --list -D 0 -d 0 -r 32000 -R 8000 -c 2 -p 320 -C 1 -V 0 -F Cvi_32_8k_2chn.g711a -T 10'
        'sample_audio 1 --list -D 0 -d 0 -r 16000 -R 8000 -c 2 -p 320 -C 2 -V 0 -F Cvi_16_18k_2chn.g711mu -T 10'
        'sample_audio 2 --list -D 1 -d 0 -r 8000 -R 8000 -c 2 -p 480 -C 0 -V 0 -F Cvi_8_8k_2chn.g726 -T 10'
        'sample_audio 2 --list -D 1 -d 0 -r 8000 -R 16000 -c 2 -p 320 -C 1 -V 0 -F Cvi_16_8k_2chn.g711a -T 10'
        'sample_audio 2 --list -D 1 -d 0 -r 8000 -R 32000 -c 2 -p 320 -C 2 -V 0 -F Cvi_32_8k_2chn.g711mu -T 10'
        'sample_audio 3 --list -D 1 -d 0 -r 8000 -R 16000 -c 2 -p 320 -C 0 -V 0 -F Cvi_16_8k_2chn.g726 -T 10'
        'sample_audio 3 --list -D 1 -d 0 -r 8000 -R 32000 -c 2 -p 320 -C 1 -V 0 -F Cvi_32_8k_2chn.g711a -T 10'
        'sample_audio 3 --list -D 1 -d 0 -r 8000 -R 8000 -c 2 -p 320 -C 1 -V 0 -F Cvi_32_8k_2chn.g711a -T 10'
        'sample_audio 4 --list -D 0 -d 0 -r 8000 -R 8000 -c 2 -p 320 -C 0 -V 0 -F Cvi_8k_2chn.raw -T 10'
        'sample_audio 5 --list -D 1 -d 0 -r 8000 -R 8000 -c 2 -p 320 -C 0 -V 0 -F Cvi_8k_2chn.raw -T 10'
        'sample_audio 6'
        'sample_audio 8'"

    while IFS= read -r cmd_str; do
        cmd_str_tmp="$cmd_str"
        cmd=$(echo "$cmd_str_tmp" | tr -d "'")
        cmd=$(echo "$cmd_str_tmp" | xargs)
        cmd_str_purn=$cmd
        sample_bin_name=$(echo $cmd | cut -d' ' -f1)
        echo "========== test $cmd =========="
        sample_check $sample_bin_name
        rm -f $OUT_FILE

        if echo "$cmd_str_purn" | grep -q "sample_audio 8";then
            $UT_BIN_DIR/$cmd | tee $OUT_FILE
            rm -f $INPUT_FILE
            echo -e 0\n > $INPUT_FILE
             $UT_BIN_DIR/$cmd < $INPUT_FILE | tee $OUT_FILE
            rm -f $INPUT_FILE
            verify_get_vol
        elif echo "$cmd_str_purn" | grep -q "sample_audio 6";then
            rm -f $INPUT_FILE
            echo -e 0\n32\n0 > $INPUT_FILE
            $UT_BIN_DIR/$cmd < $INPUT_FILE | tee $OUT_FILE
            verify_set_vol

            rm -f $INPUT_FILE
            echo -e 0\n32\n1 > $INPUT_FILE
            $UT_BIN_DIR/$cmd < $INPUT_FILE | tee $OUT_FILE
            verify_set_vol
            rm -f $INPUT_FILE
        else
            $UT_BIN_DIR/$cmd | tee $OUT_FILE
            verify
        fi

        if [ $check_ret != 0 ]; then
            result=$TEST_FAIL
            break
        fi

        sleep $SLEEP_SECONDS
    done <<EOF
$cmds_list
EOF
}
clean_tmp_files

for f in $(seq 1 $TEST_TIMES)
do
    run_all_sample_audio
done

# clean_tmp_files

echo "========================================="
echo "$result"
echo "========================================="