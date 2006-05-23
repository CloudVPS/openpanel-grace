#ifndef _CHECKSUM_H
#define _CHECKSUM_H 1

/// A 32 bits hash.
/// This hash is case-insensitive, low-collision.
/// \param str The string to be hashed.
unsigned int checksum (const char *str);

/// A 64 bits hash.
/// Basically an extension of the 32 bits version, but more suitable for
/// situations with millions of entries.
/// \param str The string to be hashed.
unsigned long long checksum64 (const char *str);

/// A 64 bit hash.
/// Implementation of the djb hash in a 64 bits variant. Gets more collisions
/// but is 5-10% faster than checksum64.
/// \param str The string to be hashed.
unsigned long long djbhash64 (const char *str);

/// OBSOLETE: convert string of four characters to a 24 bit integer.
/// \param str The string.
unsigned int resid (const char *str);

/// OBSOLETE: convert a 24 bit integer to a four character string.
/// \param i The integer.
char restochar (int i);

#endif
