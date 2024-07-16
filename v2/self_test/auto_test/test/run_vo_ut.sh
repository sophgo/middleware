#!/bin/sh

BASEDIR=$(dirname "$0")
source $BASEDIR/../common/pre_check.sh
source $BASEDIR/../common/define.sh

UT_BIN_NAME=vo_ut
OUT_FILE="tmp_output"
INPUT_FILE="tmp_input"
result=$TEST_PASS
check_ret=0

function verify() {
    grep_result=$(grep "] pass" $OUT_FILE)

    if [ -z "$grep_result" ]; then
        check_ret=-1
    fi
}

function clean_tmp_files() {
    rm -rf $OUT_FILE $INPUT_FILE
}

env_check
sample_check $UT_BIN_NAME
touch $INPUT_FILE

VO_DEVICE=${VO_DEVICE:=1}
PANEL_DEVICE=${PANEL_DEVICE:=--device=1}
VO_PANEL=${VO_PANEL:=--panel=HX8394_EVB}
VO_CONTROL_PINS=${VO_CONTROL_PINS:=--control_pins=399,304,400}
VO_INTF=${VO_INTF:=0}

./sample_panel $PANEL_DEVICE $VO_PANEL $VO_CONTROL_PINS

for t in $(seq 1 $TEST_TIMES)
do
    for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 30 100;
    do
        echo "========== test $i =========="
        $UT_BIN_DIR/$UT_BIN_NAME $VO_DEVICE $VO_INTF $i < $INPUT_FILE | tee $OUT_FILE
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
