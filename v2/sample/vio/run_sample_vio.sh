#!/bin/sh

TEST_PASS="VIO-TEST-PASS"
TEST_FAIL="VIO-TEST-FAIL"

SAMPLE_BIN_NAME=sample_vio
OUT_FILE="tmp_output"
result=$TEST_PASS

if [ $# -ne 1 ] || ! echo "$1" | grep -E -q '^[12]$'; then
    echo "Usage: $0 [1|2]"
    echo "  1 - single sensor"
    echo "  2 - dual sensor"
    exit 1
fi

if [ "$1" = "2" ]; then
    CASES="5 7"
else
    CASES="0 1 2 3 4 6"
fi

for i in $CASES
do
    echo "========== vio test case $i =========="

    SLEEP_TIME=10

    if [ "$i" -eq 5 ]; then
        (
            sleep 10
            echo "1"
            sleep 10
            echo "0"
            sleep 5
            echo "255"
        ) | ./$SAMPLE_BIN_NAME $i | tee $OUT_FILE
    elif [ "$i" -eq 4 ] || [ "$i" -eq 6 ]; then
        (
            echo "1920"
            sleep 1
            echo "1080"
            sleep 1
            echo "exit"
        ) | ./$SAMPLE_BIN_NAME $i | tee $OUT_FILE
    else
        (
            sleep $SLEEP_TIME
            echo "exit"
        ) | ./$SAMPLE_BIN_NAME $i | tee $OUT_FILE
    fi

    grep "sample_vio exit success" $OUT_FILE > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        result=$TEST_FAIL
        break
    fi
    sleep 1
done

rm -rf $OUT_FILE

echo "========================================="
echo "$result"
echo "========================================="
