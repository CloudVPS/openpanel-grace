// ========================================================================
// sslsocket.cpp: Implementation of iocodec for SSL
//
// (C) Copyright 2005-2006 Pim van Riezen <pi@madscience.nl>
//                         Madscience Labs, Rotterdam 
// ========================================================================
#include <grace/sslsocket.h>

// =================================================================
// FUNCTION setupMatrixSSL
// =================================================================
void setupMatrixSSL (void)
{
	static bool initialized = false;
	if (! initialized)
	{
		//::printf ("calling matrixsslopen\n");
		if (matrixSslOpen() == 0) initialized = true;
		else
		{
			//::printf ("init failure\n");
		}
	}
}

// =================================================================
// CONSTRUCTOR sslclientcodec
// =================================================================
sslclientcodec::sslclientcodec (void)
	: iocodec ()
{
	inbuf.buf = new unsigned char[16384];
	inbuf.start = inbuf.end = inbuf.buf;
	inbuf.size = 16384;
	
	insock.buf = new unsigned char[16384];
	insock.start = insock.end = insock.buf;
	insock.size = 16384;
	
	outsock.buf = new unsigned char[16384];
	outsock.start = outsock.end = outsock.buf;
	outsock.size = 16384;
	
	handshakedone = false;
	
	setupMatrixSSL();
	//::printf ("session created\n");

}

// =================================================================
// DESTRUCTOR sslclientcodec
// =================================================================
sslclientcodec::~sslclientcodec (void)
{
	delete[] inbuf.buf;
	delete[] insock.buf;
	delete[] outsock.buf;
}

// =================================================================
// METHOD setup
// =================================================================
bool sslclientcodec::setup (void)
{
	// Initialize the library
	int rc;
	if (! handshakedone)
	{
		inbuf.start = inbuf.end = inbuf.buf;
		insock.start = insock.end = insock.buf;
		outsock.start = outsock.end = outsock.buf;
		
		//::printf ("setting up client hello\n");
		if (matrixSslNewSession (&ssl, NULL, NULL /*&session*/, 0) < 0)
		{
			//::printf ("ssl session creation failed\n");
			err = "MatrixSSL Session Error";
			throw (EX_SSL_INIT);
		}
		::printf ("setup2\n");
		rc = matrixSslEncodeClientHello (ssl, &outsock, 0);
		if (rc < 0)
		{
			err = "MatrixSSL Handshake Error";
			return false;
		}
		::printf ("setup3\n");
		handshakedone = true;
	}
	return true;
}

// =================================================================
// METHOD reset
// =================================================================
void sslclientcodec::reset (void)
{
	inbuf.start = inbuf.end = inbuf.buf;
	insock.start = insock.end = insock.buf;
	outsock.start = outsock.end = outsock.buf;
	
	if (handshakedone)
	{
		handshakedone = false;
		matrixSslDeleteSession (ssl);
		ssl = NULL;
	}
}

// =================================================================
// METHOD addinput
// =================================================================
bool sslclientcodec::addinput (const char *data, size_t sz)
{
	if ((insock.size - (insock.end - insock.buf)) < sz) return false;
	memcpy (insock.end, data, sz);
	insock.end += sz;
	return true;
}

// =================================================================
// METHOD fetchinput
// =================================================================
void sslclientcodec::fetchinput (ringbuffer &into)
{
	int rc;
	unsigned char myerror[256], alertLevel, alertDescription;
	
	alertLevel = 0;
	alertDescription = 0;
	
	::printf ("fetchinput()\n");

	if (insock.buf < insock.start)
	{
		if (insock.start == insock.end)
		{
			insock.start = insock.end = insock.buf;
		}
		else
		{
			memmove (insock.buf, insock.start, insock.end-insock.start);
			insock.end -= (insock.start - insock.buf);
			insock.start = insock.buf;
		}
	}


again:
	rc = matrixSslDecode (ssl, &insock, &inbuf, myerror, &alertLevel,
						  &alertDescription);
	switch (rc)
	{
		case SSL_SUCCESS:
			::printf ("fetchinput SSL_SUCCESS\n");
			if (insock.end - insock.start) goto again;
			return;
			
		case SSL_PROCESS_DATA:
			::printf ("fetchinput SSL_PROCESS_DATA\n");
			into.add ((const char *) inbuf.start, inbuf.end-inbuf.start);
			inbuf.start = inbuf.end = inbuf.buf;
			if (insock.end - insock.start) goto again;
			return;
			
		case SSL_SEND_RESPONSE:
			::printf ("fetchinput SSL_SEND_RESPONSE\n");
			if ((outsock.size-(outsock.end - outsock.buf)) < 
			    (inbuf.end - inbuf.start))
			{
				err = "buffer snafu #18472";
				throw (EX_SSL_BUFFER_SNAFU);
			}
			
			memcpy (outsock.end, inbuf.start, inbuf.end-inbuf.start);
			outsock.end += (inbuf.end - inbuf.start);
			inbuf.start = inbuf.end = inbuf.buf;
			return;
			
		case SSL_ERROR:
			::printf ("fetchinput SSL_ERROR\n");
			err.crop();
			err.printf ("MatrixSSL Protocol Error: %s", myerror);
			inbuf.start = inbuf.end = inbuf.buf;
			throw (EX_SSL_PROTOCOL_ERROR);
			
		case SSL_ALERT:
			::printf ("alert\n");
			inbuf.start = inbuf.end = inbuf.buf;
			if (alertDescription == SSL_ALERT_CLOSE_NOTIFY)
				return;
			
			err.crop();
			err.printf ("MatrixSSL Client Alert: %d/%d",
						alertLevel, alertDescription);
			throw (EX_SSL_CLIENT_ALERT);
		
		case SSL_PARTIAL:
			::printf ("partial\n");
			return;
			
		case SSL_FULL:
			::printf ("full\n");
			throw (EX_SSL_BUFFER_SNAFU);
	}
	::printf ("wtf omg bbq: rc=%i\n", rc);
}

// =================================================================
// METHOD addclose
// =================================================================
void sslclientcodec::addclose (void)
{
	if (handshakedone)
	{
		handshakedone = false;
		matrixSslDeleteSession (ssl);
		ssl = NULL;
	}
}

// =================================================================
// METHOD addoutput
// =================================================================
bool sslclientcodec::addoutput (const char *dat, size_t sz)
{
	int rc;
	if (outsock.buf < outsock.start)
	{
		if (outsock.start == outsock.end)
		{
			outsock.start = outsock.end = outsock.buf;
		}
		else
		{
			memmove (outsock.buf, outsock.start, outsock.end-outsock.start);
			outsock.end -= (outsock.start - outsock.buf);
			outsock.start = outsock.buf;
		}
	}
	
	if (! handshakedone)
	{
		throw (EX_SSL_NO_HANDSHAKE);
	}
	
	rc = matrixSslEncode (ssl, (unsigned char*)dat, sz, &outsock);
	switch (rc)
	{
		case SSL_ERROR:
			err = "MatrixSSL Encoding Error";
			return false;
			
		case SSL_FULL:
			err = "MatrixSSL Buffer Error";
			throw (EX_SSL_BUFFER_SNAFU);
	}
	//::printf ("added output\n");
	return true;
}

// =================================================================
// METHOD canoutput
// =================================================================
bool sslclientcodec::canoutput (unsigned int sz)
{
	if (((outsock.end - outsock.buf)+sz) >= outsock.size)
		return false;
	return true;
}

// =================================================================
// METHOD peekoutput
// =================================================================
void sslclientcodec::peekoutput (string &into)
{
	unsigned int nsz;
	
	nsz = outsock.end-outsock.start;
	if (! nsz) return;
	
	into.crop();
	into.strcat ((const char *)outsock.start, nsz);
}

// =================================================================
// METHOD doneoutput
// =================================================================
void sslclientcodec::doneoutput (unsigned int sz)
{
	if ((outsock.start + sz) > outsock.end)
		throw (EX_SSL_BUFFER_SNAFU);
	
	outsock.start += sz;
	if (outsock.start == outsock.end)
	{
		outsock.start = outsock.end = outsock.buf; 
	}
}

// =================================================================
// CONSTRUCTOR sslsocket
// =================================================================
sslsocket::sslsocket (void)
{
	codec = new sslclientcodec;
}

// =================================================================
// DESTRUCTOR sslsocket
// =================================================================
sslsocket::~sslsocket (void)
{
	if (codec) delete codec;
	codec = NULL;
}

// =================================================================
// OPERATOR sslsocket =
// =================================================================
sslsocket &sslsocket::operator= (sslsocket *orig)
{
	codec = orig->codec;
	orig->codec = NULL;
	((tcpsocket)*this) = (tcpsocket *) orig;
	return *this;
}

