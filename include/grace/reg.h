#ifndef REG_H
#define REG_H 1
#include <sys/types.h>
int _wild_match(unsigned char *, unsigned char *);
int match (char *, char *);
int wild_match (char *, char *);
int wild_match_end (char *, char *);
#endif
