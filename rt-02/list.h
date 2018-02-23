#ifndef __LIST__
#define __LIST__
#include <linux/file.h>
#include <linux/dirent.h>
#include <linux/fs_struct.h>
#include <linux/sched.h>
#include <linux/fdtable.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <net/tcp.h>
#include <linux/in.h>
#include <linux/module.h> /* module_init module_exit */
#include <linux/init.h> /* __init __exit */
#include <linux/syscalls.h> /* sys_close __NR_close __NR_mkdir */
#include <linux/delay.h> /* loops_per_jiffy */
#include <asm/bitops.h> /* set_bit clear_bit */
#include "main.h"

struct list_int
{
    int data;
    struct list_head list;
};

#define MAX_STR         MAX_PATH + 4
struct list_chars
{
    char data[MAX_STR];
    struct list_head list;
};


#define MAX_SIZE 20

struct list_ints
{
    int size;
    int data[MAX_SIZE];
    struct list_head list;
};

void insert_int(struct list_head *list, int i);
int first_int(struct list_head *list);
int get_int(struct list_head *list, int i);
int search_int(struct list_head *list, int i);
void delete_int(struct list_head *list, int i);
void destory_int(struct list_head *list);
void display_int(struct list_head *list, char *buf, unsigned long len);

void insert_chars(struct list_head *list, char *str);
int search_chars(struct list_head *list, char *str);
void delete_chars(struct list_head *list, char *str);
void destory_chars(struct list_head *list);
void display_chars(struct list_head *list, char *buf, unsigned long len);

//void insert_ints(struct list_head *list, int *i, int size);
//int search_ints(struct list_head *list, int *i, int size);
//void delete_ints(struct list_head *list, int *i, int size);
//void destory_ints(struct list_head *list);
//void display_ints(struct list_head *list, char *buf, unsigned long len);

int count_list(struct list_head *list);

#endif // __RT_LIST__
