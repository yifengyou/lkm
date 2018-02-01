cmd_/root/lkm/rootkit.ko := ld -r -m elf_i386 -T ./scripts/module-common.lds --build-id  -o /root/lkm/rootkit.ko /root/lkm/rootkit.o /root/lkm/rootkit.mod.o
