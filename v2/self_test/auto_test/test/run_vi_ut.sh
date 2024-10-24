#!/bin/sh

BASEDIR=$(dirname "$0")
source $BASEDIR/../common/pre_check.sh
source $BASEDIR/../common/define.sh

UT_BIN_NAME=vi_ut
OUT_FILE="tmp_output"
INPUT_FILE="tmp_input"
result=$TEST_PASS
check_ret=0

function verify() {
    if [ $1 != 0 ]; then
        check_ret=-1
    fi
}

function clean_tmp_files() {
    rm -rf $INPUT_FILE
}

env_check
sample_check $UT_BIN_NAME
touch $INPUT_FILE

for t in $(seq 1 $TEST_TIMES)
do
    for i in $(seq 1 26)
    do
        echo "========== test $i =========="
        $UT_BIN_DIR/$UT_BIN_NAME $i < $INPUT_FILE
        verify $?

        if [ $check_ret != 0 ]; then
            result=$TEST_FAIL
            break
        fi

        sleep $SLEEP_SECONDS
    done
done

clean_tmp_files

echo "========================================="
echo "middleware $UT_BIN_NAME $result"
echo "========================================="
