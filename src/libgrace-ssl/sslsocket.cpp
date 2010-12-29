// This file is part of the Grace library, ssl addon (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU General Public License along with 
// Grace library. If not, see <http://www.gnu.org/licenses/>.

// ========================================================================
// sslsocket.cpp: Implementation of iocodec for SSL
// ========================================================================
#include <grace/sslsocket.h>
#include <grace/filesystem.h>



#include <matrixssl/matrixSsl.h>

extern void setupMatrixSSL (void);

/// An iocodec implementing SSL client traffic through MatrixSSL.
/// Used by sslsocket as the iocodec of choice.
class sslcodec : public iocodec
{
public:
					 sslcodec ( bool server, sslKeys_t* keys=0 );
					~sslcodec (void);
	
					 /// Initiates the SSL handshake.
	bool			 setup (void);
	
					 /// Reset all buffers and handshake data.
	void			 reset (void);
	
					 /// Adds SSL-coded data to the input buffer.
	bool			 addinput (const char *, size_t);
	
					 /// Adds plain data to the output buffer for SSL coding.
	bool			 addoutput (const char *, size_t);
	
					 /// Adds an SSL close event to the output buffer.
	void			 addclose (void);
	
					 /// Loads decoded data into a ringbuffer.
	bool			 fetchinput (ringbuffer &);
	
					 /// Copies currently encoded output data to a string.
	void			 peekoutput (string &);
	
					 /// Used to report the number of coded output bytes
					 /// that have been succesfully flushed.
	void			 doneoutput (unsigned int);
	
					 /// Determines if there is enough room in the output
					 /// buffer for a given number of bytes.
	bool			 canoutput (unsigned int);
	
					 /// If called, certificate checking will be disabled.
	void			 nocertcheck (void);

	bool			 handshakedone;
	
protected:
	ssl_t			*ssl;
	sslKeys_t		*keys;
	sslBuf_t		 inbuf;
	sslBuf_t		 insock;
	sslBuf_t		 outsock;
	sslSessionId_t	 session;
	bool			 disablecerts;
	bool 			 server;
	
};


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
// CONSTRUCTOR sslcodec
// =================================================================
sslcodec::sslcodec (bool server, sslKeys_t* keys)
	: iocodec ()
	, keys( keys )
	, server (server)
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
	
	if (!this->keys)
		this->keys = MATRIXSSLKEYS;
	//::printf ("session created\n");

}

// =================================================================
// DESTRUCTOR sslcodec
// =================================================================
sslcodec::~sslcodec (void)
{
	delete[] inbuf.buf;
	delete[] insock.buf;
	delete[] outsock.buf;
}

// =================================================================
// METHOD setup
// =================================================================
bool sslcodec::setup (void)
{
	// Initialize the library
	int rc;
	if (! handshakedone)
	{
		inbuf.start = inbuf.end = inbuf.buf;
		insock.start = insock.end = insock.buf;
		outsock.start = outsock.end = outsock.buf;
		//::printf ("setting up client hello\n");
		if (matrixSslNewSession (&ssl, keys, NULL /*&session*/, server?SSL_FLAGS_SERVER:0 ) < 0)
		{
			//::printf ("ssl session creation failed\n");
			err = "MatrixSSL Session Error";
			throw (EX_SSL_INIT);
		}
		
		if (disablecerts)
			matrixSslSetCertValidator (ssl, dummyCertValidator, NULL);
			
		if (!server)
		{
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
	}
	return true;
}

void sslcodec::nocertcheck (void)
{
	disablecerts = true;
}

// =================================================================
// METHOD reset
// =================================================================
void sslcodec::reset (void)
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
bool sslcodec::addinput (const char *data, size_t sz)
{
	if ((insock.size - (insock.end - insock.buf)) < sz) return false;
	memcpy (insock.end, data, sz);
	insock.end += sz;
	return true;
}

// =================================================================
// METHOD fetchinput
// =================================================================
bool sslcodec::fetchinput (ringbuffer &into)
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
			if (server && !handshakedone && matrixSslHandshakeIsComplete(ssl) != 0) 
				handshakedone=true;
			
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
			
			if (server && !handshakedone && matrixSslHandshakeIsComplete(ssl) != 0) 
				handshakedone=true;
			
			into.add ((const char *) inbuf.start, inbuf.end-inbuf.start);
			inbuf.start = inbuf.end = inbuf.buf;
			if (insock.end - insock.start) goto again;
			return false;
			
		case SSL_SEND_RESPONSE:
			//::printf ("fetchinput SSL_SEND_RESPONSE\n");
			if ((outsock.size-(outsock.end - outsock.buf)) < 
			    (inbuf.end - inbuf.start))
			{
				err = "buffer snafu #18472";
				throw (EX_SSL_BUFFER_SNAFU);
			}

			if (server && !handshakedone && matrixSslHandshakeIsComplete(ssl) != 0) 
				handshakedone=true;
			
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
void sslcodec::addclose (void)
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
bool sslcodec::addoutput (const char *dat, size_t sz)
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
bool sslcodec::canoutput (unsigned int sz)
{
	if (((outsock.end - outsock.buf)+sz) >= outsock.size)
		return false;
	return true;
}

// =================================================================
// METHOD peekoutput
// =================================================================
void sslcodec::peekoutput (string &into)
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
void sslcodec::doneoutput (unsigned int sz)
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
	codec = new sslcodec( false );
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

void ssllistener::loadkeyfile( const string& cert, const string& priv )
{
	string cert_data = fs.load(cert);
	string priv_data = fs.load(priv ? priv : cert );
	
	loadkeystring( cert_data, priv_data );
}

void ssllistener::loadkeystring( const string& cert, const string& priv )
{

	string cert_base64 = cert;
	cert_base64.cropafter("-----BEGIN CERTIFICATE-----");
	cert_base64.cropat("-----END CERTIFICATE-----");
	string cert_data = cert_base64.decode64();
	
	string priv_base64 = priv ? priv : cert;
	priv_base64.cropafter("-----BEGIN RSA PRIVATE KEY-----");
	priv_base64.cropat("-----END RSA PRIVATE KEY-----");
	string priv_data = priv_base64.decode64();
	
	if (keys)
	{
		matrixSslFreeKeys( (sslKeys_t*)keys );
	}

	matrixSslReadKeysMem( (sslKeys_t**)&keys,
		(unsigned char*)cert_data.str(), cert_data.strlen(),
		(unsigned char*)priv_data.str(), priv_data.strlen(),
		0,0
	 );
}


tcpsocket *ssllistener::accept (void)
{
	tcpsocket* result = tcplistener::accept();
	if (result)
	{
		sslcodec* codec = new sslcodec( true, (sslKeys_t*)keys );
		result->codec = codec;
		codec->setup();
		
		while (!codec->handshakedone )
		{
			result->readbuffer( 128, 0 );
			result->flush();
		}
	}
	return result;
}
	
tcpsocket *ssllistener::tryaccept(double timeout)
{
	tcpsocket* result = tcplistener::tryaccept( timeout );
	if (result)
	{
		sslcodec* codec = new sslcodec( true, (sslKeys_t*)keys );
		result->codec = codec;
		codec->setup();
		
		while (!codec->handshakedone )
		{
			result->readbuffer( 128, 0 );
			result->flush();
		}
	}
	return result;
}





// =================================================================
// CONSTRUCTOR httpssocket
// =================================================================
httpssocket::httpssocket() : httpsocket ()
{
	_sock.codec = new sslcodec( false );
}

// =================================================================
// METHOD codecerror
// =================================================================
const string	&httpssocket::codecerror (void)
{
	return _sock.codec->error();
}
					 
// =================================================================
// METHOD nocertcheck
// =================================================================
void httpssocket::nocertcheck (void)
{
	_sock.codec->nocertcheck();
}



