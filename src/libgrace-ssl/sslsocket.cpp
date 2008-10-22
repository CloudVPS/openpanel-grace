// ========================================================================
// sslsocket.cpp: Implementation of iocodec for SSL
//
// (C) Copyright 2005-2006 Pim van Riezen <pi@madscience.nl>
//                         Madscience Labs, Rotterdam 
// ========================================================================
#include <grace/sslsocket.h>
#include <grace/filesystem.h>

int dummyCertValidator (sslCertInfo_t *certInfo, void *arg)
{
	return 1;
}

sslKeys_t *MATRIXSSLKEYS = NULL;
extern const char *GRACE_BUILTIN_CERTS;

// =================================================================
// FUNCTION setupMatrixSSL
// =================================================================
void setupMatrixSSL (void)
{
	static bool initialized = false;
	if (! initialized)
	{
		value certlist;
		certlist = strutil::split (GRACE_BUILTIN_CERTS, "-----BEGIN CERTIFICATE-----\n");
		string cacerts;
		
		foreach (encodedcert, certlist)
		{
			string mycert = encodedcert;
			if (! mycert) continue;
			mycert.cropat ("---");
			string decoded = mycert.decode64();
			
			unsigned char d2 = (unsigned char) decoded[2];
			unsigned char d3 = (unsigned char) decoded[3];
			int hdrlen = d3 + (256 * d2);
			
			if (decoded.strlen() != (hdrlen + 4))
			{
				continue;
			}
			
			cacerts.strcat (decoded);
		}
		
		//fs.save ("certs.der", mycert);
		//int i = matrixSslReadKeys (&MATRIXSSLKEYS, NULL, NULL, NULL, "cacert");
		matrixSslReadKeysMem (&MATRIXSSLKEYS, NULL, 0, NULL, 0,
							  (unsigned char *) cacerts.str(),
							  cacerts.strlen());
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
	
	insock.buf = new unsigned char[32768];
	insock.start = insock.end = insock.buf;
	insock.size = 32768;
	
	outsock.buf = new unsigned char[16384];
	outsock.start = outsock.end = outsock.buf;
	outsock.size = 16384;
	
	handshakedone = false;
	disablecerts = false;
	
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
		if (matrixSslNewSession (&ssl, MATRIXSSLKEYS, NULL /*&session*/, 0) < 0)
		{
			//::printf ("ssl session creation failed\n");
			err = "MatrixSSL Session Error";
			throw (EX_SSL_INIT);
		}
		
		if (disablecerts)
			matrixSslSetCertValidator (ssl, dummyCertValidator, NULL);
			
		//::printf ("%08x setup2\n", this);
		rc = matrixSslEncodeClientHello (ssl, &outsock, 0);
		if (rc < 0)
		{
			err = "MatrixSSL Handshake Error";
			return false;
		}
		//::printf ("%08x setup3\n", this);
		handshakedone = true;
	}
	return true;
}

void sslclientcodec::nocertcheck (void)
{
	disablecerts = true;
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
bool sslclientcodec::fetchinput (ringbuffer &into)
{
	int rc;
	unsigned int room = 0;
	unsigned char myerror, alertLevel, alertDescription;
	
	myerror = 0;
	alertLevel = 0;
	alertDescription = 0;
	
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

	//::printf ("%08x fetchinput() insock.size=%i\n", this, insock.end-insock.start);

again:
	rc = matrixSslDecode (ssl, &insock, &inbuf, &myerror, &alertLevel,
						  &alertDescription);

	switch (rc)
	{
		case SSL_SUCCESS:
			//::printf ("fetchinput SSL_SUCCESS\n");
			if (insock.end - insock.start) goto again;
			return false;
		
		case SSL_PARTIAL:
			
			if (inbuf.start == inbuf.end) return true;
			
			//::printf ("%08x start=%08x end=%08x size=%i\n", this, inbuf.start, inbuf.end, inbuf.end-inbuf.start);

		case SSL_PROCESS_DATA:
			//::printf ("%08x fetchinput SSL_PROCESS_DATA\n", this);
			/*room = into.room();
			if (room< (inbuf.end-inbuf.start))
			{
				into.add ((const char *) inbuf.start, room);
				inbuf.start += room;
				return true;
			} */
			
			into.add ((const char *) inbuf.start, inbuf.end-inbuf.start);
			inbuf.start = inbuf.end = inbuf.buf;
			//if (insock.end - insock.start) goto again;
			return false;
			
		case SSL_SEND_RESPONSE:
			//::printf ("fetchinput SSL_SEND_RESPONSE\n");
			if ((outsock.size-(outsock.end - outsock.buf)) < 
			    (inbuf.end - inbuf.start))
			{
				err = "buffer snafu #18472";
				throw (EX_SSL_BUFFER_SNAFU);
			}
			
			memmove (outsock.start+(inbuf.end-inbuf.start), outsock.start, outsock.end-outsock.start);
			memcpy (outsock.start, inbuf.start, inbuf.end-inbuf.start);
			outsock.end += (inbuf.end - inbuf.start);
			inbuf.start = inbuf.end = inbuf.buf;
			return true;
			
		case SSL_ERROR:
			switch (myerror)
			{
				case SSL_ALERT_UNEXPECTED_MESSAGE:
					err = "Unexpected SSL message"; break;
				
				case SSL_ALERT_BAD_RECORD_MAC:
					err = "SSL bad record MAC"; break;
				
				case SSL_ALERT_DECOMPRESSION_FAILURE:
					err = "Decompression failure"; break;
				
				case SSL_ALERT_HANDSHAKE_FAILURE:
					err = "Handshake failure"; break;
					
				case SSL_ALERT_NO_CERTIFICATE:
					err = "No certificate"; break;
				
				case SSL_ALERT_BAD_CERTIFICATE:
					err = "Bad certificate"; break;
				
				case SSL_ALERT_UNSUPPORTED_CERTIFICATE:
					err = "Unsupported certificate"; break;
					
				case SSL_ALERT_CERTIFICATE_REVOKED:
					err = "Certificate revoked"; break;
				
				case SSL_ALERT_CERTIFICATE_EXPIRED:
					err = "Certificate expired"; break;
				
				case SSL_ALERT_CERTIFICATE_UNKNOWN:
					err = "Unknown certificate"; break;
				
				default:
					err = "Unknown MatrixSSL error"; break;
			}
			inbuf.start = inbuf.end = inbuf.buf;
			throw (EX_SSL_PROTOCOL_ERROR);
			
		case SSL_ALERT:
			inbuf.start = inbuf.end = inbuf.buf;
			if (alertDescription == SSL_ALERT_CLOSE_NOTIFY)
				return false;
			
			err.crop();
			err.printf ("MatrixSSL Client Alert: %d/%d",
						alertLevel, alertDescription);
			throw (EX_SSL_CLIENT_ALERT);
		
		case SSL_FULL:
			//::printf ("full\n");
			throw (EX_SSL_BUFFER_SNAFU);
	}
	::printf ("wtf omg bbq: rc=%i\n", rc);
	return false;
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

