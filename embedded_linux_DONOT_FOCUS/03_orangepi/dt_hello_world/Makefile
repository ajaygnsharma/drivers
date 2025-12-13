obj-m += dt_hello_world.o

KERNEL_DIR ?= $(HOME)/dev/buildroot-orange-pi-zero-plus/output/build/linux-5.10.10
HOSTTOOLS=$(HOME)/dev/buildroot-orange-pi-zero-plus/output/host/bin/

all:
	make -C $(KERNEL_DIR) \
		ARCH=arm64 CROSS_COMPILE=$(HOSTTOOLS)/aarch64-buildroot-linux-uclibc- M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) \
		ARCH=arm64 CROSS_COMPILE=$(HOSTTOOLS)/aarch64-buildroot-linux-uclibc- M=$(PWD) clean

