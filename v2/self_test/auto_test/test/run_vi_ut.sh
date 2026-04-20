#!/bin/sh

BASEDIR=$(dirname "$0")
source $BASEDIR/../common/pre_check.sh
source $BASEDIR/../common/define.sh

UT_BIN_NAME=vi_ut
OUT_FILE="tmp_output"
INPUT_FILE="tmp_input"
result=$TEST_PASS
check_ret=0

# ION memory leak detection
# Return value example:
# Summary:
# [0] carveout heap size:78643200 bytes, used:8343552 bytes
# usage rate:11%, memory usage peak 61083648 bytes
#
# Details:
#          heap_id   alloc_buf_size         phy_addr         kmap_cnt      buffer name
#                0          8294400         8b50c000                1      gfbg_layer0
#                0            49152         8b500000                1         enc_iapu
#
# minimum ion allocate unit = 4096
# free memory regions:
#          heap_id            start              end           length
#                0         8bcf5000         90000000         70299648
# ION Heap path definition
ION_HEAP_VPP="/sys/kernel/debug/ion/cvi_vpp_heap_dump/summary"

# Global variable: record initial memory usage for each heap
INITIAL_ION_VPP_USED=""

function verify() {
    if [ $1 != 0 ]; then
        check_ret=-1
    fi
}

function clean_tmp_files() {
    rm -rf $INPUT_FILE
}

# ION memory leak detection function (single heap)
# Parameters: $1=heap path, $2=heap name, $3=check mode(0=check only,1=record initial value,2=detect leak), $4=initial value variable name(for mode 2)
function check_single_ion_heap() {
    local heap_path=$1
    local heap_name=$2
    local check_leak=${3:-0}
    local initial_var_name=$4

    echo "[INFO] Checking ION heap: $heap_name"
    local ion_summary=$(cat "$heap_path" 2>/dev/null)
    if [ -z "$ion_summary" ]; then
        echo "[WARN] Unable to read ION heap summary: $heap_name"
        return 0
    fi
    # Extract current used memory (bytes)
    local current_used=$(echo "$ion_summary" | grep "used:" | sed 's/.*used:\([0-9]*\).*/\1/')

    if [ -z "$current_used" ]; then
        echo "[WARN] Unable to parse ION memory usage: $heap_name"
        return 0
    fi
    # Execute different operations based on check mode
    case $check_leak in
        1)
            # Record initial memory usage
            eval "$initial_var_name=$current_used"
            echo "[INFO] Recorded initial $heap_name memory usage: $current_used bytes"
            ;;
        2)
            # Detect memory leak
            local initial_used=$(eval echo \$$initial_var_name)
            if [ -n "$initial_used" ]; then
                local mem_diff=$((current_used - initial_used))
                # If memory growth exceeds threshold (e.g. 1MB = 1048576 bytes), report potential leak
                local leak_threshold=1048576
                if [ $mem_diff -gt $leak_threshold ]; then
                    echo "[ERROR] $heap_name: Potential memory leak detected! Memory increased by $mem_diff bytes (from $initial_used to $current_used)"
                    return 1
                elif [ $mem_diff -gt 0 ]; then
                    echo "[WARN] $heap_name: Memory usage increased by $mem_diff bytes (from $initial_used to $current_used)"
                else
                    echo "[PASS] [$heap_name] No memory leak detected (current: $current_used bytes)"
                fi
            else
                echo "[WARN] $heap_name: No initial memory baseline recorded"
            fi
            ;;
        *)
            # Only show current status
            echo "[INFO] $heap_name: Current ION memory usage: $current_used bytes"
            ;;
    esac
    return 0
}

# ION memory leak detection function
# Parameters: $1=check mode(0=check only,1=record initial value,2=detect leak)
function check_ion_heap() {
    local check_leak=${1:-0}
    local overall_result=0

    # Check VPP heap
    if ! check_single_ion_heap "$ION_HEAP_VPP" "VPP Heap" "$check_leak" "INITIAL_ION_VPP_USED"; then
        overall_result=1
    fi

    return $overall_result
}

env_check
sample_check $UT_BIN_NAME
touch $INPUT_FILE

for t in $(seq 1 $TEST_TIMES)
do
    for i in $(seq 1 26)
    do
        echo "========== test $i =========="

        check_ion_heap 1

        $UT_BIN_DIR/$UT_BIN_NAME $i < $INPUT_FILE
        verify $?

        if [ $check_ret != 0 ]; then
            result=$TEST_FAIL
            break
        fi

        echo "========== Memory leak check =========="
        if ! check_ion_heap 2; then
            echo "[FAIL] Memory leak detected for test case $i"
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
