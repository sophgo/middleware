#!/bin/sh

function env_check() {
    if [ -z ${UT_BIN_DIR} ]; then
        echo "UT_BIN_DIR is not set";
        exit 1
    fi

    if [ ! -d "${UT_BIN_DIR}" ]; then
        echo "UT_BIN_DIR: [$UT_BIN_DIR] is not exist";
        exit 1
    fi
}

function sample_check() {
    sample_bin=${UT_BIN_DIR}/$1

    if [ ! -x ${sample_bin} ]; then
        echo "${sample_bin} is not exist";
        usage
        exit 1
    fi
}