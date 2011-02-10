// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _CHECKSUM_H
#define _CHECKSUM_H 1

/// A 32 bits hash.
/// This hash is case-insensitive, low-collision.
/// \param str The string to be hashed.
unsigned int checksum (const char *str);

/// OBSOLETE: convert string of four characters to a 24 bit integer.
/// \param str The string.
unsigned int resid (const char *str);

/// OBSOLETE: convert a 24 bit integer to a four character string.
/// \param i The integer.
char restochar (int i);
#endif
