#!/bin/sh

BASEDIR=$(dirname "$0")
source $BASEDIR/../common/pre_check.sh
source $BASEDIR/../common/define.sh


UT_BIN_NAME_1=ive_1st_ut
UT_BIN_NAME_2=ive_2nd_ut
UT_BIN_NAME_3=ive_bg_ut
OUT_FILE="tmp_output"
INPUT_FILE="tmp_input"
result=$TEST_PASS
check_ret=0


function verify() {
    grep_result=$(grep "TEST-PASS" $OUT_FILE)
    if [ -z "$grep_result" ]; then
        check_ret=-1
    fi
}

function clean_tmp_files() {
    rm -rf $OUT_FILE $INPUT_FILE
}

env_check
sample_check $UT_BIN_NAME_1
sample_check $UT_BIN_NAME_2
sample_check $UT_BIN_NAME_3
touch $INPUT_FILE



for t in $(seq 1 $TEST_TIMES)
do
    for ut_bin in $UT_BIN_NAME_1 $UT_BIN_NAME_2 $UT_BIN_NAME_3;
    do
        echo "========== test $ut_bin =========="
        $UT_BIN_DIR/$ut_bin 0 < $INPUT_FILE | tee $OUT_FILE
        verify

        if [ $check_ret != 0 ]; then
            result=$TEST_FAIL
            break
        fi

        sleep $SLEEP_SECONDS
    done
done


clean_tmp_files

echo "========================================="
echo "middleware $UT_BIN_NAME_1 $UT_BIN_NAME_2 $UT_BIN_NAME_3 $result"
echo "========================================="
