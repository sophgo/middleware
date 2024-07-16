#!/bin/sh

TEST_PASS="VIVO_1_LOOPBACK_TEST_PASS"
TEST_FAIL="VIVO_1_LOOPBACK_TEST_FAIL"

OUT_FILE="tmp_output"
INPUT_FILE="tmp_input"
result=$TEST_PASS
check_ret=0

function verify() {
    grep_result=$(grep "vivo_slt_test pass" $OUT_FILE)

    if [ -z "$grep_result" ]; then
        check_ret=-1
    fi
}

touch $INPUT_FILE

echo "========== test VIVO loopback =========="
cvi_pinmux -w PWR_UART_TX/PWR_GPIO16
cvi_pinmux -w UART4_RX/IIC8_SCL
cvi_pinmux -w UART4_TX/IIC8_SDA

./sample_panel --dev=0 --panel=LT9611_1920x1080_60 --laneid=4,3,0,2,1 --pnswap=1,1,1,1,1

./lt9611 --port=b --resolution=1920x1080_60HZ

./sample_hdmi 97 594000 0 0 0 0 0 0 0 0 0 0 0

devmem 0x67004094 32 0x0701000a
devmem 0x67005094 32 0x0701000a

cp sensor_cfg.ini /mnt/data/

./vivo_slt 1 10 < $INPUT_FILE | tee $OUT_FILE
verify

if [ $check_ret != 0 ]; then
	result=$TEST_FAIL
	break
fi

rm -rf $OUT_FILE $INPUT_FILE

echo "========================================="
echo "$result"
echo "========================================="
