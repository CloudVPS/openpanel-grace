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
