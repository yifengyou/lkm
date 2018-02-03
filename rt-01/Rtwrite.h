#ifndef __RT_WRITE__
#define __RT_WRITE__

#include "Rtonline.h"

// clear WP bit of CR0
void write_begin(void);

// set CR0 with new value
void write_end(void);

#endif // __RT_WRITE__
