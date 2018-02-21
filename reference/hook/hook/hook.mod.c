#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xa9b402ff, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0xd46eaf77, __VMLINUX_SYMBOL_STR(pv_cpu_ops) },
	{ 0xba497f13, __VMLINUX_SYMBOL_STR(loops_per_jiffy) },
	{ 0x268cc6a2, __VMLINUX_SYMBOL_STR(sys_close) },
	{ 0xd19df0a5, __VMLINUX_SYMBOL_STR(commit_creds) },
	{ 0x5e2f9433, __VMLINUX_SYMBOL_STR(abort_creds) },
	{ 0x9c74325, __VMLINUX_SYMBOL_STR(prepare_creds) },
	{ 0x4538fc18, __VMLINUX_SYMBOL_STR(make_kgid) },
	{ 0x89facc81, __VMLINUX_SYMBOL_STR(make_kuid) },
	{ 0x50eedeb8, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xc063f856, __VMLINUX_SYMBOL_STR(current_task) },
	{ 0xb4390f9a, __VMLINUX_SYMBOL_STR(mcount) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "605A5A22AC7254D48EA314D");
