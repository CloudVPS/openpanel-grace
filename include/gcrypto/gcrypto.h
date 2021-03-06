// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _GCRYPTO_H
#define _GCRYPTO_H 1

#include <grace/str.h>

class crypto
{
public:
						 crypto (void);
	virtual				~crypto (void);
	
	virtual bool		 privatekey (const string &pemdata,
									 const string &pass);
	virtual string 		*privatekey (void);
	
	virtual bool		 keygen (int sz, const string &pass);
	
	virtual string		*encrypt (const string &data);
	virtual string		*decrypt (const string &data);

protected:
	string				 passphrase;
};

class pubcrypto : public crypto
{
public:
						 pubcrypto (void);
	virtual				~pubcrypto (void);
	
	virtual bool		 publickey (const string &pemdata);
	virtual string		*publickey (void);
	
	virtual bool		 keygen (int sz, const string &pass);
	
	virtual string		*encrypt (const string &data);
	virtual string		*decrypt (const string &data);
	
	virtual string		*sign (const string &data);
	virtual bool		 verify (const string &data,
								 const string &signature);

};

#endif
