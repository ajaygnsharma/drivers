#!/bin/bash
set -x
TMP=$PWD
UCLINUX_DIST=/home/user1/uclinux-dist/
echo "make sure that sudo is installed and user1 is sudoers list with no passwd"

#----------------
# Install toolchain
# As root
#-----------------
toolchain_install(){
sudo apt-get update
sudo apt-get install -y build-essential
sudo apt-get install -y libncurses5-dev zlib1g-dev
sudo apt-get install -y mtd-tools
sudo apt-get install -y bzip2

cd /usr/local/bin/
sudo ln -s /usr/sbin/mkfs.jffs2 mkfs.jffs2
sudo chgrp users /usr/sbin/mkfs.jffs2
cd $TMP
sudo cp mkcramfs /usr/local/bin/
sudo chmod a+x /usr/local/bin/mkcramfs
cd /
sudo tar -zxf $TMP/arm-linux-tools-20061213.tar.gz
sudo tar -xf $TMP/arm-elf-tools-20040427.tar
cd $TMP
}

#------------
# u-boot
# Chapter 10
#------------
uboot_install(){
mkdir -p $UCLINUX_DIST
tar -jxf $TMP/u-boot-1.1.6.tar.bz2 -C $UCLINUX_DIST
cd $UCLINUX_DIST/u-boot-1.1.6/
gunzip -c $TMP/u-boot-1.1.6-ea_v1_9_1.diff.gz | patch -p1
gunzip -c $TMP/u-boot-1.1.6-ea_v1_9_1_incr1.diff.gz | patch -p1
make LPC2468OEM_Board_32bit_config
make
arm-linux-objcopy -I binary -O ihex u-boot.bin u-boot.hex
sudo cp $UCLINUX_DIST/u-boot-1.1.6/tools/mkimage /usr/local/bin
}

kernel_install(){
#-----------------
# Chapter 11
# Build linux kernel and the whole dist
#-----------------
cd $TMP
tar -zxf $TMP/uClinux-dist-20070130.tar.gz -C $UCLINUX_DIST
rm -rf $UCLINUX_DIST/uClinux-dist/linux-2.6.x/
tar -zxf $TMP/linux-2.6.21.tar.gz -C $UCLINUX_DIST/uClinux-dist
cd $UCLINUX_DIST/uClinux-dist
mv linux-2.6.21/ linux-2.6.x
gunzip -c $TMP/ea-uClinux-081020.diff.gz | patch -p1
gunzip -c $TMP/ea-v3_1_incr1.diff.gz | patch -p1
sed -i '1s/^/#include"limits.h"\n/' linux-2.6.x/scripts/mod/sumversion.c
#cd linux-2.6.x 
make menuconfig
make
}

toolchain_install;
uboot_install;
kernel_install;

