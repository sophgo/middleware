#!/bin/bash

echo "start running 2de ut..."

ut_list=(
    16pixels_align
    arc_path
    clear
    clock
)

for ut in ${ut_list[*]}; do
    echo $ut
    ./$ut
    diff ${ut}.png golden/$ut.png
    if [ $? -ne 0 ]; then
        echo "${ut} failed!"
        exit -1
    fi
done

echo "end running 2de ut..."