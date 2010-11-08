// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _GCRYPTO_DES_H
#define _GCRYPTO_DES_H 1

class DEScrypto : public crypto
{
public:
					 DEScrypto (void);
					~DEScrypto (void);
	
	bool			 privatekey (const string &key,
								 const string &pass = "");
	string			*privatekey (void);
	bool			 keygen (int sz, const string &pass = "");
	string			*encrypt (const string &data);
	string			*decrypt (const string &data);
	
protected:
	DES_cblock		*key;
	bool			 keyInitialized;
};

#endif
