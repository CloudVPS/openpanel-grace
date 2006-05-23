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
