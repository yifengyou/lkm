cmd_/root/lkm/reference/kmod_hooking_sct/entry_32.o := gcc -Wp,-MD,/root/lkm/reference/kmod_hooking_sct/.entry_32.o.d  -nostdinc -isystem /usr/lib/gcc/i686-linux-gnu/4.8/include  -I./arch/x86/include -Iarch/x86/include/generated/uapi -Iarch/x86/include/generated  -Iinclude -I./arch/x86/include/uapi -Iarch/x86/include/generated/uapi -I./include/uapi -Iinclude/generated/uapi -include ./include/linux/kconfig.h -Iubuntu/include  -D__KERNEL__  -fno-pie   -D__ASSEMBLY__ -fno-PIE  -fno-pie -m32 -DCONFIG_AS_CFI=1 -DCONFIG_AS_CFI_SIGNAL_FRAME=1 -DCONFIG_AS_CFI_SECTIONS=1  -DCONFIG_AS_SSSE3=1 -DCONFIG_AS_CRC32=1 -DCONFIG_AS_AVX=1 -DCONFIG_AS_AVX2=1 -DCONFIG_AS_SHA1_NI=1 -DCONFIG_AS_SHA256_NI=1  -DCC_HAVE_ASM_GOTO           -DMODULE  -c -o /root/lkm/reference/kmod_hooking_sct/entry_32.o /root/lkm/reference/kmod_hooking_sct/entry_32.S

source_/root/lkm/reference/kmod_hooking_sct/entry_32.o := /root/lkm/reference/kmod_hooking_sct/entry_32.S

deps_/root/lkm/reference/kmod_hooking_sct/entry_32.o := \
    $(wildcard include/config/as/cfi.h) \
  arch/x86/include/asm/dwarf2.h \
    $(wildcard include/config/as/cfi/signal/frame.h) \
    $(wildcard include/config/as/cfi/sections.h) \
  arch/x86/include/asm/asm-offsets.h \
  include/generated/asm-offsets.h \
  include/linux/linkage.h \
  include/linux/compiler.h \
    $(wildcard include/config/sparse/rcu/pointer.h) \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/kasan.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
    $(wildcard include/config/kprobes.h) \
  include/linux/stringify.h \
  include/linux/export.h \
    $(wildcard include/config/have/underscore/symbol/prefix.h) \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/unused/symbols.h) \
  arch/x86/include/asm/linkage.h \
    $(wildcard include/config/x86/32.h) \
    $(wildcard include/config/x86/64.h) \
    $(wildcard include/config/x86/alignment/16.h) \

/root/lkm/reference/kmod_hooking_sct/entry_32.o: $(deps_/root/lkm/reference/kmod_hooking_sct/entry_32.o)

$(deps_/root/lkm/reference/kmod_hooking_sct/entry_32.o):
