#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
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
	{ 0x88d2b212, "module_layout" },
	{ 0x3ce4ca6f, "disable_irq" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0xfbc74f64, "__copy_from_user" },
	{ 0xf3d777e4, "_raw_spin_unlock" },
	{ 0x67c2fa54, "__copy_to_user" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0xad8f4bf1, "del_timer" },
	{ 0x9a82d814, "__alloc_workqueue_key" },
	{ 0x47229b5c, "gpio_request" },
	{ 0x3dfab183, "schedule_work" },
	{ 0x88f3dcfa, "down_interruptible" },
	{ 0x432fd7f6, "__gpio_set_value" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0xd697e69a, "trace_hardirqs_on" },
	{ 0x3f0376ca, "__init_waitqueue_head" },
	{ 0x58fe2e86, "wait_for_completion" },
	{ 0x5e23e398, "misc_register" },
	{ 0xfa2a45e, "__memzero" },
	{ 0x5f754e5a, "memset" },
	{ 0xde75b689, "set_irq_type" },
	{ 0x9d38e1e0, "down_trylock" },
	{ 0xea147363, "printk" },
	{ 0xa8f59416, "gpio_direction_output" },
	{ 0xc291205c, "destroy_workqueue" },
	{ 0x859c6dc7, "request_threaded_irq" },
	{ 0x1bef1d61, "flush_workqueue" },
	{ 0x5f81d45, "boot_tvec_bases" },
	{ 0x11f447ce, "__gpio_to_irq" },
	{ 0xe9ce8b95, "omap_ioremap" },
	{ 0x15331242, "omap_iounmap" },
	{ 0xa94dfcb8, "__raw_spin_lock_init" },
	{ 0x9eb6c46, "_raw_spin_lock" },
	{ 0xfe990052, "gpio_free" },
	{ 0xfcec0987, "enable_irq" },
	{ 0x37a0cba, "kfree" },
	{ 0x9d669763, "memcpy" },
	{ 0x16c7f7e4, "up" },
	{ 0x16f9b492, "lockdep_init_map" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x3cd4fa57, "complete" },
	{ 0xec3d2e1b, "trace_hardirqs_off" },
	{ 0xbf0b8fcc, "misc_deregister" },
	{ 0x4327ad77, "queue_delayed_work" },
	{ 0x9e7d6bd0, "__udelay" },
	{ 0xf20dabd8, "free_irq" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "533BB7E5866E52F63B9ACCB");
