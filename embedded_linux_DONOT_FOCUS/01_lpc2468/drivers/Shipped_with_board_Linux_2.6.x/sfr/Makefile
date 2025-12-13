
# to generate a lst file add the line below to CFLAGS
#-Wa,-ahlms=$(<:.c=.lst)

ifneq ($(KERNELRELEASE),)
CFLAGS += -march=armv4t -I$(SUBDIRS)/../include

	obj-m := sfr.o
	sfr-objs := ea_sfr.o
else

KERNELDIR = ../../../../../../linux-2.6.x
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) SUBDIRS=$(PWD) modules

endif

clean:
	rm -f *.o *~ *.ko core *.mod.c *.ko.cmd *.o.cmd .*.cmd Module.symvers -r .tmp_versions
