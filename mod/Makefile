

export ARCH=arm
export CROSS_COMPILE=/opt/acr890/toolchain/bin/arm-arago-linux-gnueabi-

AR=$(CROSS_COMPILE)ar
LD=$(CROSS_COMPILE)ld
CC=$(CROSS_COMPILE)gcc
EXTRA_CFLAGS := -O0 -g -DEXPORT_SYMTAB
LOCAL_STRIP_MODULE := false
DEBFLAGS=-02
PWD := $(shell pwd)
src:=src

obj-m := pcdm.o
pcdm-objs := 	delay.o pcd_config.o pn512.o\
		iso14443_typeA.o iso14443_typeB.o \
		iso14444.o felica.o pcsc.o  mifare.o topaz.o \
		 pcd.o\
		  

#KERNELDIR?=/opt/acr910/kernel/
KERNELDIR?=/opt/acr890/linux-2.6.37/

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD)
#	@echo 'copy pcdm.ko to share file folder'
#	@cp -f *.ko /mnt/hgfs/sharefiles
#	@echo 'Module pcdm.ko already generated'

clean:
	rm -rf $(srcdir)*.o $(srcdir)*.mod.o $(srcdir)*.ko $(srcdir)*.mod.c \
	*.o *.mod.o *.ko *.mod.c *.symvers *.order
