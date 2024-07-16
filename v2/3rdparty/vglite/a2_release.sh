#!/bin/bash

shopt -s globstar
shopt -s extglob

COLOR_END="\033[0m"
COLOR_RED="\033[1;31;40m"
COLOR_GREEN="\033[1;32;40m"

pr_info()
{
	echo -e ${COLOR_GREEN}${1}${COLOR_END}
}

pr_error()
{
	echo -e ${COLOR_RED}${1}${COLOR_END}
}

pushd $PWD &>/dev/null

# ----------------------------------------
# env check
# ----------------------------------------
if [ -z $TOP_DIR ]; then
	pr_error "Error: You have to config the build environment and then build all first!"
	popd &>/dev/null && exit 1
fi

# check 2d_engine
ls lib/*.so &>/dev/null
if [ ! $? -eq 0 ]; then
	pr_error "Error: release lib: libvg_lite.so must be required! Please build all first!"
	popd &>/dev/null && exit 1
fi

ls | grep -v "a2_release.sh\|Makefile\|lib" | xargs rm -rf

if [ -f Makefile.sdk_release ]; then
	mv Makefile.sdk_release Makefile
fi

popd &>/dev/null