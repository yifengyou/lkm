MODULE_NAME = rootkit
KDIR := /lib/modules/`uname -r`/build 
PWD := $(shell pwd)
INCLUDESDIR := ./
KBUILD_CFLAGS   += -Wno-sizeof-pointer-memaccess
EXTRA_CFLAGS += -I $(INCLUDESDIR) 

<<<<<<< HEAD
$(MODULE_NAME)-objs = syscalltable.o list.o cmd.o  util.o main.o  
=======
$(MODULE_NAME)-objs = main.o syscalltable.o list.o cmd.o  
>>>>>>> 884f7fbed93b1f333a0ad3295fe448314d32affa

obj-m += $(MODULE_NAME).o 

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules 
	rm -rf  *.mod.c  modules.order Module.symvers  .rootkit* .tmp*
clean:
	rm -rf *.o *.ko *.mod.c  modules.order Module.symvers .rootkit* .tmp*
