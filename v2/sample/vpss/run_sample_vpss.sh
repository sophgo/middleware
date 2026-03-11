#!/bin/sh

TEST_PASS="VPSS-TEST-PASS"
TEST_FAIL="VPSS-TEST-FAIL"

SAMPLE_BIN_NAME=sample_vpss
OUT_FILE="tmp_output"
INPUT_FILE="tmp_input"
result=$TEST_PASS
check_ret=0

function verify() {
    grep_result=$(grep -E "SAMPLE_VPSS exit success" $OUT_FILE)

    if [ -z "$grep_result" ]; then
        check_ret=-1
    fi
}


touch $INPUT_FILE

for i in 0 1 2 3 4
do
	echo "========== vpss test case$i =========="
	./$SAMPLE_BIN_NAME $i < $INPUT_FILE | tee $OUT_FILE
	verify

	if [ $check_ret != 0 ]; then
		result=$TEST_FAIL
		break
	fi
	sleep 1
done

rm -rf $OUT_FILE $INPUT_FILE

echo "========================================="
echo "$result"
echo "========================================="