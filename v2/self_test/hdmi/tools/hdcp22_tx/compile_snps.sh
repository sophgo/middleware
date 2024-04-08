#!/bin/bash
export PATH=$BUILDROOT_HOME/output/host/usr/bin/:$PATH
export ARCH=arc
export CROSS_COMPILE=arc-buildroot-linux-uclibc-
export KERNEL_PATH=$BUILDROOT_HOME/output/build/linux-arc-3.18-2/

mkdir -p bin

make samples CPREF=$BUILDROOT_HOME/output/host/usr/bin/arc-buildroot-linux-uclibc- KDIR=$KERNEL_PATH
make mem_access CPREF=$BUILDROOT_HOME/output/host/usr/bin/arc-buildroot-linux-uclibc- KDIR=$KERNEL_PATH

