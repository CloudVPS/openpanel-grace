// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _GCRYPTO_RSA_H
#define _GCRYPTO_RSA_H 1

class RSAcrypto : public pubcrypto
{
public:
				 RSAcrypto (void);
				~RSAcrypto (void);
				
	bool		 privatekey (const string &, const string &);
	string		*privatekey (void);
	bool		 publickey (const string &);
	string		*publickey (void);
	
	bool		 keygen (int, const string &);
	
	string		*encrypt (const string &);
	string		*decrypt (const string &);
	string		*sign (const string &);
	bool		 verify (const string &, const string &);
	
protected:
	DES_cblock	*obj;
};

#endif
