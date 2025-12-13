obj-m += ledRGB_rpi_platform.o
KERNEL_DIR ?= $(HOME)/ext_ssd/rpi/buildroot/output/build/linux-custom/
HOSTTOOLS=$(HOME)/ext_ssd/rpi/buildroot/output/host/bin/

all:
	make -C $(KERNEL_DIR) ARCH=arm CROSS_COMPILE=$(HOSTTOOLS)/arm-buildroot-linux-gnueabihf- M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) ARCH=arm CROSS_COMPILE=$(HOSTTOOLS)/arm-buildroot-linux-gnueabihf- M=$(PWD) clean

deploy:
	scp *.ko root@10.0.0.10:
