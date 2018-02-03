cmd_/root/lkm/rt-01/rootkit.ko := ld -r -m elf_i386 -T ./scripts/module-common.lds --build-id  -o /root/lkm/rt-01/rootkit.ko /root/lkm/rt-01/rootkit.o /root/lkm/rt-01/rootkit.mod.o
