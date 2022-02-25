obj-m += helloworld.o

KERNEL_DIR ?= $(HOME)/buildroot-2018.02/output/build/linux-4.11/
TOOLCHAIN_PATH=/home/user1/buildroot-2018.02/output/host/bin

all:
	make -C $(KERNEL_DIR) ARCH=arm CROSS_COMPILE=$(TOOLCHAIN_PATH)/arm-buildroot-uclinux-uclibcgnueabi- SUBDIRS=$(PWD) modules

clean:
	make -C  $(KERNEL_DIR) ARCH=arm CROSS_COMPILE=$(TOOLCHAIN_PATH)/arm-buildroot-uclinux-uclibcgnueabi- SUBDIRS=$(PWD) clean


