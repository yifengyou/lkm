MODULE_NAME = rootkit
KDIR := /lib/modules/`uname -r`/build 
PWD := $(shell pwd)
INCLUDESDIR := ./
KBUILD_CFLAGS   += -Wno-sizeof-pointer-memaccess
EXTRA_CFLAGS += -I $(INCLUDESDIR) 

$(MODULE_NAME)-objs = RTmain.o

obj-m += $(MODULE_NAME).o 

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules 
	rm -rf *.o *.mod.c  modules.order Module.symvers .RT* .rootkit* .tmp*
clean:
	rm -rf *.o *.ko *.mod.c  modules.order Module.symvers .Rt* .rootkit* .tmp*
