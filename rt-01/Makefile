MODULE_NAME = rootkit
KDIR := /lib/modules/`uname -r`/build 
PWD := $(shell pwd)
INCLUDESDIR := ./
KBUILD_CFLAGS   += -Wno-sizeof-pointer-memaccess
EXTRA_CFLAGS += -I $(INCLUDESDIR) 

$(MODULE_NAME)-objs = Rtonline.o Rtsyscall.o Rtwrite.o Rtcmd.o \
Rtstring.o Rtlist.o Rtpath.o

obj-m += $(MODULE_NAME).o 

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	rm -f *.o *.ko *.mod.c  modules.order Module.symvers
