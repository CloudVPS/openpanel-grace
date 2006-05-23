// ========================================================================
// gcrypto.cpp: Implementation of the gcrypto base objects
//
// (C) Copyright 2004 Pim van Riezen <pi@madscience.nl>
//                    Madscience Labs, Rotterdam 
// ========================================================================

#include <gcrypto/gcrypto.h>

// ==========================================================================
// The crypto class is an empty virtual base class and should always
// be used in an inherited form.
// ==========================================================================

crypto::crypto (void)
{
}

crypto::~crypto (void)
{
}

bool crypto::privatekey (const string &pemdate, const string &pass)
{
	return false;
}

string *crypto::privatekey (void)
{
	return NULL;
}

bool crypto::keygen (int sz, const string &pass)
{
	return false;
}

string *crypto::encrypt (const string &data)
{
	return NULL;
}

string *crypto::decrypt (const string &data)
{
	return NULL;
}

// ==========================================================================
// The pubcrypto class is an empty virtual base class and should always
// be used in an inherited form.
// ==========================================================================

pubcrypto::pubcrypto (void)
{
}

pubcrypto::~pubcrypto (void)
{
}

bool pubcrypto::publickey (const string &pemdata)
{
	return false;
}

string *pubcrypto::publickey (void)
{
	return NULL;
}

bool pubcrypto::keygen (int sz, const string &pass)
{
	return false;
}

string *pubcrypto::encrypt (const string &data)
{
	return NULL;
}

string *pubcrypto::decrypt (const string &data)
{
	return NULL;
}

string *pubcrypto::sign (const string &data)
{
	return NULL;
}

bool pubcrypto::verify (const string &data, const string &sig)
{
	return false;
}
