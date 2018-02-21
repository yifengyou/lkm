obj-m += hook_sys.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
	rm -rf  *.o *.mod.c  modules.order Module.symvers  .tmp*
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
