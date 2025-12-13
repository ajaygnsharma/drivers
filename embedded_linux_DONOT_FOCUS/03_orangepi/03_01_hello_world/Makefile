obj-m += hello.o
KERNEL_DIR ?= $(HOME)/dev/orangepi_zero_plus/buildroot/output/build/linux-6.1.53/
HOSTTOOLS=$(HOME)/dev/orangepi_zero_plus/buildroot/output/host/bin/

all:
	make -C $(KERNEL_DIR) ARCH=arm64 CROSS_COMPILE=$(HOSTTOOLS)/aarch64-buildroot-linux-gnu- M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) ARCH=arm64 CROSS_COMPILE=$(HOSTTOOLS)/aarch64-buildroot-linux-gnu- M=$(PWD) clean

deploy:
	scp *.ko root@10.0.0.10:
