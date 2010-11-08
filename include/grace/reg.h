// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef REG_H
#define REG_H 1
#include <sys/types.h>
int _wild_match(unsigned char *, unsigned char *);
int match (char *, char *);
int wild_match (char *, char *);
int wild_match_end (char *, char *);
#endif
