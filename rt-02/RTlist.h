#include "RTmain.h"

#ifndef __RT_LIST__
#define __RT_LIST__

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
