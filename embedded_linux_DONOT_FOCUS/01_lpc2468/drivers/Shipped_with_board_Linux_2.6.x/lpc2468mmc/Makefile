# Change it here or specify it on the "make" command line

ifneq ($(KERNELRELEASE),)
CFLAGS += -I$(SUBDIRS)/../include

	obj-m := lpc2468mmc.o
	lpc2468mmc-objs := dma.o mci.o mmc_hw.o ea-mmc_26.o readfifo.o
else

KERNELDIR = ../../../../../../linux-2.6.x
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) SUBDIRS=$(PWD) modules

endif

clean:
	rm -f *.o *~ *.ko core *.mod.c *.ko.cmd *.o.cmd .*.cmd Module.symvers -r .tmp_versions
