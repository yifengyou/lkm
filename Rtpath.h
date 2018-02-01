#ifndef __RT_PATH__
#define __RT_PATH__

#include "Rtonline.h"

const char *get_fullpath(char *fullpath, int size, struct dentry *pd);
const char *get_path(char *fullpath, int size, unsigned int fd);

#endif // __RT_PATH__
