
#include <grace/httpd.h>
#include <grace/sslsocket.h>

httpsd::httpsd (const string &DomainSocket,  int inmint, int inmaxt)
{
	listener = new ssllistener;
	listener->listento( DomainSocket );
	if (inmaxt < inmint) inmaxt = inmint;
	_maxpostsize = defaults::lim::httpd::postsize;
	minthr = inmint;
	maxthr = inmaxt;
	eventmask = 0;
	load.o = 0;
	first = NULL;
	firsthandler = NULL;
	_shutdown = false;

}

httpsd::httpsd (int listenport, int inmint, int inmaxt)
{
	listener = new ssllistener;
	listener->listento( listenport );
	if (inmaxt < inmint) inmaxt = inmint;
	_maxpostsize = defaults::lim::httpd::postsize;
	minthr = inmint;
	maxthr = inmaxt;
	eventmask = 0;
	load.o = 0;
	first = NULL;
	firsthandler = NULL;
	_shutdown = false;
}

httpsd::httpsd (void)
{
	listener = NULL;
	_maxpostsize = defaults::lim::httpd::postsize;
	minthr = 2;
	maxthr = 4;
	eventmask = 0;
	load.o = 0;
	first = NULL;
	firsthandler = NULL;
	_shutdown = false;
}
					 
void httpsd::loadkeyfile( const string& cert, const string& priv )
{

	cert_data = fs.load(cert);		
	priv_data = fs.load(priv?priv:cert);		
	
	if (listener)
		((ssllistener*)listener)->loadkeystring( cert_data, priv_data );
}

void httpsd::loadkeystring( const string& cert, const string& priv )
{
	cert_data = cert;		
	priv_data = priv ? priv : cert;		

	if (listener)
		((ssllistener*)listener)->loadkeystring( cert_data, priv_data );
}

void httpsd::createlistener()
{
	if (listener) delete listener;
	
	listener = new ssllistener();
	
	if( cert_data && priv_data )
		((ssllistener*)listener)->loadkeystring( cert_data, priv_data );
}

