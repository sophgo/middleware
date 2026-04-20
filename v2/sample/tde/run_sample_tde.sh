#!/bin/sh

TEST_PASS="TDE-TEST-PASS"
TEST_FAIL="TDE-TEST-FAIL"

SAMPLE_BIN_NAME=sample_tde
OUT_FILE="tmp_output"
INPUT_FILE="tmp_input"
result=$TEST_PASS
check_ret=0

function verify() {
    grep_result=$(grep -E "SAMPLE_TDE exit success!" $OUT_FILE)

    if [ -z "$grep_result" ]; then
        check_ret=-1
    fi
}


touch $INPUT_FILE

echo "========== tde test case=========="
./$SAMPLE_BIN_NAME < $INPUT_FILE | tee $OUT_FILE
verify

if [ $check_ret != 0 ]; then
	result=$TEST_FAIL
	break
fi
sleep 1


rm -rf $OUT_FILE $INPUT_FILE

echo "========================================="
echo "$result"
echo "========================================="