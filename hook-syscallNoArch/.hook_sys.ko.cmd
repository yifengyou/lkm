cmd_/root/lkm/hook-syscallNoArch/hook_sys.ko := ld -r -m elf_i386 -T ./scripts/module-common.lds --build-id  -o /root/lkm/hook-syscallNoArch/hook_sys.ko /root/lkm/hook-syscallNoArch/hook_sys.o /root/lkm/hook-syscallNoArch/hook_sys.mod.o
