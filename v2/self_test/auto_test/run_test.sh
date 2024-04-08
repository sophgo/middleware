#!/bin/sh

UT_BIN_DIR=$(cd "$(dirname "$0")"; pwd)

BASEDIR=$(dirname "$0")
. $BASEDIR/env

function usage()
{
    echo "$0 [TEST CASE]"
    echo "              all"
    for f in $BASEDIR/test/*.sh;
    do
        echo "              ${f}"
    done
    echo ""
    echo "Mandatory env"
    echo "    UT_BIN_DIR                   - middleware unit test binary directory"
    echo ""
    echo "Optional env"
    echo "    TEST_TIMES                   - repeat times for each test case, default 1"
    echo "    SLEEP_SECONDS                - sleep seconds between each test case, default 3"
    echo ""

    exit 1
}

function run_all()
{
    for f in $BASEDIR/test/*.sh;
    do
        echo "=================== run $f ==================="
        $f
        sleep $SLEEP_SECONDS
    done
}

function run_single()
{
    $1
}

if [ -z $UT_BIN_DIR ]; then
    echo "UT_BIN_DIR is not set"
    echo ""
    usage
fi

export UT_BIN_DIR TEST_TIMES SLEEP_SECONDS

if [ "$1" == "all" ]; then
    run_all
elif [ -n $1 -a -x "$1" ]; then
    run_single "$1"
else
    usage
fi
