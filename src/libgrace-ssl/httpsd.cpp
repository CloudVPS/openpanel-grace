
#include <grace/httpd.h>
#include <grace/sslsocket.h>

class nonsslredirect : public httpdobject
{
public:
	nonsslredirect(httpd &s) : httpdobject (s, "*") {}

	int run (string &uri, string &postbody, value &inhdr, string &out,
			 value &outhdr, value &env, tcpsocket &s)
	{
		if (s.codec == NULL)
		{
			string host = inhdr["Host"];
			int port = s.local_port;
			
			if (host.strchr(':') >= 0)
			{
				string portstr=host;
				portstr.cropafter(':');
				port = portstr.toint();
				
				host.cropat(':');
			}
			
			string safe_uri = port == 443 ?
				"https://%s%s" %format(host,uri) :
				"https://%s:%i%s" %format(host,port,uri) ;

			outhdr["Connection"] = "close";
			
			out = "<html><h1>This service requires https</h1>"
				  "This service required encryption, provided through https<br>"
			   	  "Please use the ecrypted connection <a href=\"%s\">here</a>" %format(safe_uri);

			// requests without side effects can get an http redirect
			if ( env["method"] == "GET" || env["method"] == "HEAD" )
			{
				outhdr["Location"] = safe_uri;
				return 301;
			}
			else
			{
				// This request could have had a side effect, browser support 
				// for redirects on POST and PUT varies, and we can't really 
				// rely on it.
				
				// pad the output for IE, otherwise the user will be presented 
				// with a "friendly" error
				
				while (out.strlen() < 512)
					out.strcat("\r\n<!-- Dear internet explorer, please don't show a \"friendly\" error message, will you? -->");
				
				return 405;
			}
		}
		
		return 0;
	}
};

httpsd::httpsd (const string &DomainSocket,  int inmint, int inmaxt)
{
	if (inmaxt < inmint) inmaxt = inmint;
	_maxpostsize = defaults::lim::httpd::postsize;
	minthr = inmint;
	maxthr = inmaxt;
	eventmask = 0;
	load.o = 0;
	first = NULL;
	firsthandler = NULL;
	_shutdown = false;

	listener = NULL;
	createlistener();
	listener->listento( DomainSocket );
	
	nonsslredirecthandler = new nonsslredirect(*this);
}

httpsd::httpsd (int listenport, int inmint, int inmaxt)
{
	if (inmaxt < inmint) inmaxt = inmint;
	_maxpostsize = defaults::lim::httpd::postsize;
	minthr = inmint;
	maxthr = inmaxt;
	eventmask = 0;
	load.o = 0;
	first = NULL;
	firsthandler = NULL;
	_shutdown = false;
	
	listener = NULL;
	createlistener();
	listener->listento( listenport );

	nonsslredirecthandler = new nonsslredirect(*this);
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

	nonsslredirecthandler = new nonsslredirect(*this);
}

httpsd::~httpsd ()
{
	delete 	nonsslredirecthandler;
	nonsslredirecthandler = NULL;
}
					 
void httpsd::loadkeyfile( const string& cert, const string& priv )
{

	cert_data = fs.load(cert);		
	priv_data = fs.load(priv?priv:cert);		
	
	if (listener)
	{
		((ssllistener*)listener)->loadkeystring( cert_data, priv_data );
	}
}

void httpsd::loadkeystring( const string& cert, const string& priv )
{
	cert_data = cert;		
	priv_data = priv ? priv : cert;		

	if (listener)
	{
		((ssllistener*)listener)->loadkeystring( cert_data, priv_data );
	}
}

void httpsd::createlistener()
{
	if (listener) delete listener;
	
	ssllistener* newlistener = new ssllistener();
	newlistener->setallowpassthrough( true );
	listener = newlistener;
	
	if( cert_data && priv_data )
	{
		((ssllistener*)listener)->loadkeystring( cert_data, priv_data );
	}
}

