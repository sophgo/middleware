#!/bin/sh

TEST_PASS="DPU-TEST-PASS"
TEST_FAIL="DPU-TEST-FAIL"

SAMPLE_BIN_NAME=sample_dpu
result=$TEST_PASS
check_ret=0


for i in 0 1 2 3 4 5 6 7 8 9
do
	echo "========== dpu test case$i =========="
	./$SAMPLE_BIN_NAME $i

    if [ $? -ne 0 ]; then
        result=$TEST_FAIL
		break
    fi
	sleep 1
done


echo "========================================="
echo "$result"
echo "========================================="