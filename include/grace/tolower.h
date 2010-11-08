// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _TOLOWER_H
#define _TOLOWER_H 1

#include <sys/types.h>

extern u_char lower_tab[256];
#define _tolower(y) ((char) lower_tab[y])

#endif
