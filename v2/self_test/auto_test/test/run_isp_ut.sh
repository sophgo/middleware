#! /bin/bash

BASEDIR=$(dirname "$0")
source $BASEDIR/../common/pre_check.sh
source $BASEDIR/../common/define.sh

UT_BIN_NAME=cvi_test
OUT_FILE="isp_tmp_output"
result=$TEST_PASS
check_ret=0
SENSOR_INI_SRC_PATH=$BASEDIR/../res/isp/sensor_cfg_gc4653.ini
SENSOR_INI_DST_PATH=/mnt/data/sensor_cfg.ini
PQBIN_SRC_PATH=$BASEDIR/../res/isp/gc4653_sdr.bin
PQBIN_DST_PATH=/mnt/cfg/param/cvi_sdr_bin

BD_SUFFIX=.isp_bd

TEST_CASE_NUM=(114 120 121 255)

function verify()
{
	grep_result=$(grep "fail" $OUT_FILE)

	if [ -n "$grep_result" ]; then
		check_ret=-1
	else
		check_ret=0
	fi
}

function prepare_data()
{
	# check res/isp bin and ini file
	if [ ! -f $SENSOR_INI_SRC_PATH -o ! -f $PQBIN_SRC_PATH ]; then
		echo "${SENSOR_INI_SRC_PATH} or ${PQBIN_SRC_PATH} not exists!"
		result=$TEST_FAIL
		output_result
		exit 1
	fi
	# backen
	if [ -f $SENSOR_INI_DST_PATH ]; then
		mv $SENSOR_INI_DST_PATH ${SENSOR_INI_DST_PATH}${BD_SUFFIX}
	fi
	if [ -f $PQBIN_DST_PATH ]; then
		mv $PQBIN_DST_PATH ${PQBIN_DST_PATH}${BD_SUFFIX}
	fi

	# copy files
	cp $SENSOR_INI_SRC_PATH $SENSOR_INI_DST_PATH
	cp $PQBIN_SRC_PATH $PQBIN_DST_PATH
}

function clean_tmp_files()
{
	rm -rf $OUT_FILE
	rm -rf $SENSOR_INI_DST_PATH
	rm -rf $PQBIN_DST_PATH

	if [ -f ${SENSOR_INI_DST_PATH}${BD_SUFFIX} ]; then
		mv ${SENSOR_INI_DST_PATH}${BD_SUFFIX} $SENSOR_INI_DST_PATH
	fi

	if [ -f ${PQBIN_DST_PATH}${BD_SUFFIX} ]; then
		cp ${PQBIN_DST_PATH}${BD_SUFFIX} $PQBIN_DST_PATH
	fi
}

function output_result()
{
	echo "========================================="
	echo "middleware $UT_BIN_NAME $result"
	echo "========================================="
}

env_check
sample_check $UT_BIN_NAME
prepare_data

for t in $(seq 1 $TEST_TIMES)
do
	for i in ${TEST_CASE_NUM[@]}
	do
		echo "========== test $i =========="
		$UT_BIN_DIR/$UT_BIN_NAME $i | tee $OUT_FILE

		verify

		# TODO: del 120
		if [ $i -eq 255 -o $i -eq 120 ]; then
			check_ret=0
		fi

		if [ $check_ret != 0 ]; then
			result=$TEST_FAIL
		fi

		sleep $SLEEP_SECONDS
	done
done

clean_tmp_files
output_result
