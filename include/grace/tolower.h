#ifndef _TOLOWER_H
#define _TOLOWER_H 1

#include <sys/types.h>

extern u_char lower_tab[256];
#define _tolower(y) lower_tab[y]

#endif
