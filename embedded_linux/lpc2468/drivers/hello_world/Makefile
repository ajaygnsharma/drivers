obj-m += helloworld.o

KERNEL_DIR ?= $(HOME)/uClinux-dist/linux-2.6.x

all:
	make -C $(KERNEL_DIR) ARCH=arm CROSS_COMPILE=arm-elf- SUBDIRS=$(PWD) modules

clean:
	make -C  $(KERNEL_DIR) ARCH=arm CROSS_COMPILE=arm-elf- SUBDIRS=$(PWD) clean


