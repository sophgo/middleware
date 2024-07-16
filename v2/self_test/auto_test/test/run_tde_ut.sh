#!/bin/sh

BASEDIR=$(dirname "$0")
source $BASEDIR/../common/pre_check.sh
source $BASEDIR/../common/define.sh

UT_BIN_NAME=tde_ut
OUT_FILE="tmp_output"
INPUT_FILE="tmp_input"
TMP_RES_FOLDER="tmp_res_folder"
result=$TEST_PASS
check_ret=0

function verify() {
    if [ -d "./$TMP_RES_FOLDER" ]; then
	rm -rf $TMP_RES_FOLDER
    fi
    mkdir $TMP_RES_FOLDER
    mv *.bmp $TMP_RES_FOLDER/

    diff $TMP_RES_FOLDER ./res/tde

    if [ $? -ne 0 ]; then
        check_ret=-1
    fi
}

function clean_tmp_files() {
    rm -rf $OUT_FILE $INPUT_FILE
    rm -rf *.raw *.bmp
}

env_check
sample_check $UT_BIN_NAME
clean_tmp_files
touch $INPUT_FILE

for t in $(seq 1 $TEST_TIMES)
do
    for i in 99;
    do
        echo "========== test $i =========="
        $UT_BIN_DIR/$UT_BIN_NAME $i 1 < $INPUT_FILE | tee $OUT_FILE
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
echo "middleware $UT_BIN_NAME $result"
echo "========================================="
