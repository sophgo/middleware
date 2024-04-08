#!/bin/sh

TEST_CASE_NODE=/sys/module/cv186x_vi/parameters/vi_ip_test_case
BALLON_PIC=/mnt/sd/pic/2k_ballon_bayer_12_GR.bin
DPC_STATIC_PIC=/mnt/sd/pic/static_dpc_0_0_0_20_0_40_0_60_4ps_bayer_12_GR.bin
DPC_DYNAMIC_PIC=/mnt/sd/pic/2k_dpc_bayer_12_GR.bin
BNR_PIC=/mnt/sd/pic/color_noise_jpg_bayer_12_GR.bin
LCAC_PIC=/mnt/sd/pic/purple_edge_bayer_12_GR.bin
DHZ_PIC=/mnt/sd/pic/dhz_face_bayer_12_GR.bin
FUSION_LE_PIC=/mnt/sd/pic/fusion_le_1920_1080.raw
FUSION_SE_PIC=/mnt/sd/pic/fusion_se_1920_1080.raw
LSC_STRONG_BIN=/mnt/sd/pic/lsc_strong_mars.bin
UT_BIN=/mnt/sd/vi_ut

# scenario

# old version
# RAW_REPLAY=5
# SENSOR_FE_DRAM_BE_POST_DRAM=8
# PATGEN_FE_DRAM_BE_POST_DRAM=10
# HDR_PATGEN_FE_BE_DRAM_POST_DRAM=11

# new version
RAW_REPLAY=4
SENSOR_FE_DRAM_BE_POST_DRAM=8
PATGEN_FE_DRAM_BE_POST_DRAM=10
HDR_PATGEN_FE_BE_DRAM_POST_DRAM=13


function test_case_rgbgamma()
{
  for case_no in 1 2 51
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    $UT_BIN -c $RAW_REPLAY -t -y -p $BALLON_PIC
  done
}

function test_case_lcac()
{
  for case_no in 5 6
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    $UT_BIN -c $RAW_REPLAY -t -y -p $LCAC_PIC
  done
}

function test_case_bnr()
{
  for case_no in 7 8
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    $UT_BIN -c $RAW_REPLAY -t -y -p $BNR_PIC
  done
}

function test_case_dhz()
{
  for case_no in 10
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    $UT_BIN -c $RAW_REPLAY -t -y -p $DHZ_PIC
  done
}

function test_case_blc()
{
  for case_no in 11 12 13 14 15
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    $UT_BIN -c $RAW_REPLAY -t -y -p $BALLON_PIC
  done
}

function test_case_wbg()
{
  for case_no in 16
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    $UT_BIN -c $RAW_REPLAY -t -y -p $BALLON_PIC
  done
}

# TOCHECK
function test_case_dpc()
{
  local SRC_PIC
  for case_no in 18 19
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    if [ $case_no == 18 ] ; then
      SRC_PIC=$DPC_STATIC_PIC
    else
      SRC_PIC=$DPC_DYNAMIC_PIC
    fi
    $UT_BIN -c $RAW_REPLAY -t -y -p $SRC_PIC
  done
}

function test_case_pree_ee()
{
  for case_no in 71 20 21
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    $UT_BIN -c $RAW_REPLAY -t -y -p $BALLON_PIC
  done
}

function test_case_ccm()
{
  for case_no in 22
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    $UT_BIN -c $RAW_REPLAY -t -y -p $BALLON_PIC
  done
}

# maybe user check
function test_case_ygamma()
{
  for case_no in 24 25 26 27
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    if [[ $case_no == 26 || $case_no == 27 ]]; then
        dmesg -c
        $UT_BIN -c $RAW_REPLAY -t -y -p $BALLON_PIC
	touch  /mnt/sd/fpga/Ygamma/Ygamma.log
        dmesg | grep "Ygamma LUT" >> /mnt/sd/fpga/Ygamma/Ygamma.log
    else
       $UT_BIN -c $RAW_REPLAY -t -y -p $BALLON_PIC
    fi
  done
}

# dump ae
function test_case_ae()
{
  for case_no in 31
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    $UT_BIN -c $HDR_PATGEN_FE_BE_DRAM_POST_DRAM -t -u
  done
}

# reboot 48 need run first
function test_case_histv()
{
  for case_no in 48 32 34 33 49 50
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    $UT_BIN -c $HDR_PATGEN_FE_BE_DRAM_POST_DRAM -t -y -u
  done
}

function test_case_fusion()
{
  for case_no in 29 30
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    $UT_BIN -c $RAW_REPLAY -h -t -y -p $FUSION_LE_PIC -q $FUSION_SE_PIC
  done
}

function test_case_ltm()
{
  for case_no in 37 38 39 40
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    $UT_BIN -c $RAW_REPLAY -h -t -y -p $FUSION_LE_PIC -q $FUSION_SE_PIC
  done
}

function test_case_gms()
{
  for case_no in 41
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    $UT_BIN -c $PATGEN_FE_DRAM_BE_POST_DRAM -t -y -u
  done
}

# TODO use case 6
function test_case_af()
{
  for case_no in 42
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    $UT_BIN -c $PATGEN_FE_DRAM_BE_POST_DRAM -t -y -u
  done
}

# reboot
function test_case_lscm()
{
  for case_no in 43
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    $UT_BIN -c $RAW_REPLAY -t -y -p $BALLON_PIC -l $LSC_STRONG_BIN -u
  done
}

function test_case_rgbcac()
{
  for case_no in 46 47
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    $UT_BIN -c $RAW_REPLAY -t -y -p $LCAC_PIC
  done
}

# need 3dnr
function test_case_dither()
{
  for case_no in 52 53 54 55
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    $UT_BIN -c $RAW_REPLAY -t -y -p $BALLON_PIC
  done
}

function test_case_clut()
{
  for case_no in 56
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    $UT_BIN -c $RAW_REPLAY -t -y -p $BALLON_PIC
  done
}

# reboot dump data
function test_case_dci()
{
  for case_no in 36 35 57 58
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    $UT_BIN -c $PATGEN_FE_DRAM_BE_POST_DRAM -t -y -u
  done
}

# TODO use case 6
function test_case_ldci()
{
  for case_no in 45
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    $UT_BIN -c $PATGEN_FE_DRAM_BE_POST_DRAM -t -y
  done
}

# CA/CP/CALite
function test_case_cacp()
{
  for case_no in 59 60 61 62 63
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    if [[ $case_no == 59 || $case_no == 60 || $case_no == 61 ]]; then
       $UT_BIN -c $RAW_REPLAY -t -y -p $BALLON_PIC
    else
       $UT_BIN -c $PATGEN_FE_DRAM_BE_POST_DRAM -t -y  #TODO wait use case 6
    fi
  done
}

# cnr
function test_case_cnr()
{
  for case_no in 64 3 4 65
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    $UT_BIN -c $RAW_REPLAY -t -y -p $BNR_PIC
  done
}

# YNR need reboot
function test_case_ynr()
{
  for case_no in 66 9 67 68
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    $UT_BIN -c $RAW_REPLAY -t -y -p $BNR_PIC
  done
}

function test_case_ycurve()
{
  for case_no in 23
  do
    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    $UT_BIN -c $RAW_REPLAY -t -y -p $BALLON_PIC
  done
}

function test_case_crop()
{
  for case_no in 69 70
  do

    cmd="echo "$case_no" > "$TEST_CASE_NODE
    echo $cmd
    eval $cmd
    if [ $case_no == 69 ] ; then
      $UT_BIN -c $PATGEN_FE_DRAM_BE_POST_DRAM -t -y -u
    else
      $UT_BIN -c $PATGEN_FE_DRAM_BE_POST_DRAM -t -r -u
    fi
  done
}

function getfilesordir(){
    echo "search dir: "$1
    for dir in `ls $1`
    do
        echo $dir
        if test -d $1/$dir
        then
            for file in `ls $1/$dir`
	    do
                echo $file
                if test -f $1/$dir/$file
                then
                    md5sum $1/$dir/$file >> md5sum.log
                fi
            done
        fi
    done
}

function test_case_all()
{
	test_case_wbg

	test_case_blc
	test_case_dpc
	test_case_af

	test_case_bnr
	test_case_lscm
	test_case_ae
	test_case_gms
	test_case_rgbcac
	test_case_lcac

	test_case_ccm
	test_case_histv
	test_case_fusion
	test_case_ltm
	test_case_ygamma
	test_case_rgbgamma
	test_case_dhz
	test_case_dither
	test_case_clut

	test_case_dci
	test_case_ldci
	test_case_cacp
	test_case_cnr
	test_case_crop
	test_case_pree_ee # TODO check
	test_case_ynr
	test_case_ycurve
	# dpcm
	# max size
}

test_case_all

cd /mnt/sd/fpga/
path="./"
getfilesordir $path
cd /mnt/sd/





