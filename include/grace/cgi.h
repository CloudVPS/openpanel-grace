#ifndef _CGI_H
#define _CGI_H 1

#include <grace/exception.h>
#include <grace/str.h>
#include <grace/application.h>
#include <grace/xmlschema.h>
#include <grace/dictionary.h>

$exception (cgiEndOfFileException, "CGI End of File");
$exception (cgiPostFormatException, "CGI Post Format Error");

//typedef dictionary<xmlschema> schemadict;

/// An application subclass for CGI apps.
/// The cgi class makes it easier to create applications that adhere to the
/// CGI specification. It can handle both POST and GET methods. For
/// POST methods, both regular x-www-urlencoded and multipart submits
/// (used by file uploads) are allowed. There is an upper limit to the
/// size of submit data. Currently this is handled by the compile-time
/// default DEFAULT_LIM_CGI_POSTSIZE in the defaults.h header.
///
/// The class keeps an output buffer that can be used to create CGI
/// programs that are compatible with HTTP keepalive (adding the
/// proper Content-length header).
class cgi : public application
{
public:
				 /// Default constructor
				 /// Picks up CGI-related environment variables and
				 /// handles POST/GET data.
				 /// \param appname the application-id [tld.domain.foo.bar]
				 cgi (const char *appname);
				~cgi (void);
	
	virtual int	 main (void);

protected:
	value		 headers; ///< Output headers
	string		 buffer; ///< Output buffer
	//schemadict	 schemas;
	
				 /// Flushes the output-buffer.
	void		 sendpage (void);
				 /// TODO.
	void		 addschema (const statstring &name,
							const string &path="");
};

class rpccgi : public application
{
public:
				 rpccgi (const char *appname);
				~rpccgi (void);
				
	virtual int	 main (void);

protected:
	value		 headers;
	string		 buffer;
	
	void		 sendpage (void);
};

class cgitemplate
{
public:
				 cgitemplate (void);
				~cgitemplate (void);
				
	bool		 load (const string &);
	string		*parse (const string &section, value &env);

protected:
	value		 tmpl;
};

#endif
