cmd_/root/lkm/reference/hook-syscall-of-linux64/hook_sys_mkdir.ko := ld -r -m elf_i386 -T ./scripts/module-common.lds --build-id  -o /root/lkm/reference/hook-syscall-of-linux64/hook_sys_mkdir.ko /root/lkm/reference/hook-syscall-of-linux64/hook_sys_mkdir.o /root/lkm/reference/hook-syscall-of-linux64/hook_sys_mkdir.mod.o