// ========================================================================
// des.cpp: GCRYPTO httpsocket class
//
// (C) Copyright 2004 Pim van Riezen <pi@madscience.nl>
//                    Madscience Labs, Rotterdam 
// ========================================================================

#include <grace/str.h>
#include <gcrypto/gcrypto.h>
#include <gcrypto/des.h>

DEScrypto::DEScrypto (void)
{
	key = new DES_cblock;
	keyInitialized = false;
}

DEScrypto::~DEScrypto (void)
{
	if (key) delete key;
}

// ==========================================================================
// METHOD ::privatekey (PROPERTY SET)
// ----------------------------------
// Sets an explicit private key provided in PEM format. It's not expected
// for DES keys to be further encoded with a passphrase; DES keys are not
// normally kept in files and the only real reason for this method to exist
// beyond satisfying the object hierarchy is that there needs to be a way
// to communicate a DES key to another party (for example through an RSA
// channel) in situations where public key ciphers would hinder performance.
// ==========================================================================

bool DEScrypto::privatekey (const string &pemdata, const string &pass)
{
	
}

// ==========================================================================
// METHOD ::privatekey (PROPERTY GET)
// ----------------------------------
// Encodes the current DES key as a PEM string. If the key was generated
// with a passphrase, this should technically be included with the string.
// It makes sense to do this but the requirement is secondary.
// ==========================================================================

string *DEScrypto::privatekey (void)
{
}

// ==========================================================================
// METHOD ::keygen
// ---------------
// Generates a new DES key, associated with this crypto object.
// ==========================================================================

bool DEScrypto::keygen (int sz, const string &passphrase)
{
}

// ==========================================================================
// METHOD ::encrypt
// ----------------
// Encrypts a string with the current key, returns the encrypted string.
// ==========================================================================

string *DEScrypto::encrypt (const string &data)
{
}

// ==========================================================================
// METHOD ::decrypt
// ----------------
// Decrypts a string with the current key, returns the decrypted string.
// ==========================================================================

string *DEScrypto::decrypt (const string &data)
{
}

