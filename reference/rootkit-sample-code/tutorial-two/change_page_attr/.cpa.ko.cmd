cmd_/root/lkm/reference/rootkit-sample-code/tutorial-two/change_page_attr/cpa.ko := ld -r -m elf_i386 -T /usr/src/linux-headers-3.13.0-24-generic/scripts/module-common.lds --build-id  -o /root/lkm/reference/rootkit-sample-code/tutorial-two/change_page_attr/cpa.ko /root/lkm/reference/rootkit-sample-code/tutorial-two/change_page_attr/cpa.o /root/lkm/reference/rootkit-sample-code/tutorial-two/change_page_attr/cpa.mod.o