cmd_/root/rootkit-sample-code/tutorial-one/lkm1.ko := ld -r -m elf_i386 -T /root/linux-2.6.32/scripts/module-common.lds --build-id -o /root/rootkit-sample-code/tutorial-one/lkm1.ko /root/rootkit-sample-code/tutorial-one/lkm1.o /root/rootkit-sample-code/tutorial-one/lkm1.mod.o
