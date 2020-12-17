obj-m += hello.o
KERNEL_DIR ?= $(HOME)/dev/buildroot-at91/output/build/linux-linux4sam-2020.04

all:
	make -C $(KERNEL_DIR) ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabihf- M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabihf- M=$(PWD) clean

deploy:
	scp *.ko root@10.0.0.10: