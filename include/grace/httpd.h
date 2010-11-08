// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _HTTPD_H
#define _HTTPD_H 1

#include <grace/thread.h>
#include <grace/tcpsocket.h>
#include <grace/cmdtoken.h>
#include <grace/dictionary.h>
#include <grace/filesystem.h>
#include <grace/defaults.h>
#include <grace/ipaddress.h>

#define HTTPD_ACCESS 1
#define HTTPD_ERROR 2
#define HTTPD_INFO 4
typedef int httpdeventclass;

// ------------------------------------------------------------------------
// CLASS httpdobject: Base object for server behavior with a provided uri
//                    scope.
// ------------------------------------------------------------------------

/// Base httpd handler class.
/// A httpdobject performs an action on a request. It's generally part
/// of a chain of request handlers that handle certain parts of the problem.
/// An example class derived from httpdobject to return some static
/// data for any matching URIs:
/// \verbinclude httpd_ex2.cpp
class httpdobject
{
public:
					 /// Constructor.
					 /// \param pparent Reference to the parent httpd.
					 /// \param purimatch The match definition for the
					 ///                  uri part of the request.
					 httpdobject (class httpd &pparent,
					 			  const string &purimatch);
	virtual			~httpdobject (void);

					 /// Implementation.
					 /// A child class can either manipulate elements
					 /// of the request's environment, for example by
					 /// rewriting the uri or adding data to the
					 /// variable space, or act as a last node in the
					 /// httpd's chain of handler objects. In the latter
					 /// case, the body data should be either composed
					 /// or sent to the socket and the return value
					 /// should indicate the final status code for the
					 /// request.
					 /// \param uri The uri of the request
					 /// \param postbody The posted body data
					 /// \param inhdr The received headers
					 /// \param out The output buffer
					 /// \param outhdr The output headers
					 /// \param env The request environment
					 /// \param s The tcpsocket handling the request
					 /// \return Status code as an integer. The value is
					 ///         \b 0 if more processing should be done
					 ///         by other objects in the chain. If the
					 ///         value is \e positive, the return body
					 ///         has been put in \b out and the httpd
					 ///         code will send it to the client. If
					 ///         the value is \e negative, the data has
					 ///         already been sent to the socket directly.
					 
	virtual int		 run (string &uri, string &postbody,
						  value &inhdr, string &out, value &outhdr,
						  value &env, tcpsocket &s);
	
	httpdobject		*next; ///< Linked list pointer.
	string			 urimatch; ///< Match criterium.
	class httpd		*parent; ///< Pointer to the parent httpd object.
};

/// Data handler for a file type. Objects derived from this class can
/// be linked as a child into a httpdfileshare object. This makes it possible
/// to create separate handlers for dynamic filetypes like CGI or
/// scripts.
class httpdfiletypehandler
{
public:
					 /// Constructor. Links to the parent httpdfileshare.
					 /// \param pparent Reference to the parent fileshare.
					 /// \param filetype File extension this class acts
					 ///                 upon.
					 httpdfiletypehandler (class httpdfileshare &pparent,
					 					   const string &filetype);
					 					   
	virtual			~httpdfiletypehandler (void);
	
					 /// Implementation.
					 /// Child classes should pick up the file from
					 /// the argument, perform any necessary parsing or
					 /// transformation voodoo and then either stick
					 /// the results in the buffer or send them to the
					 /// socket directly.
					 /// \param path Filesystem path of the object
					 /// \param postbody The posted body data
					 /// \param inhdr The received headers
					 /// \param out The output buffer
					 /// \param outhdr The output headers
					 /// \param env The request environment
					 /// \param s The tcpsocket handling the request
					 /// \return Status code as an integer. If the
					 ///         value is \e positive, the return body
					 ///         has been put in \b out and the httpd
					 ///         code will send it to the client. If
					 ///         the value is \e negative, the data has
					 ///         already been sent to the socket directly.
	virtual int		 run (string &path, string &postbody,
						  value &inhdr, string &out, value &outhdr,
						  value &env, tcpsocket &s);
						  
protected:
	class httpdfileshare	*parent; ///< Link to the parent.
};

/// Handler for scriptparser scripts.
class httpdscriptparser : public httpdfiletypehandler
{
public:
					 /// Constructor.
					 /// \param pparent Parent fileshare reference.
					 /// \param config Configuration data.
					 httpdscriptparser (class httpdfileshare &pparent,
					 					value &config);
					~httpdscriptparser (void);
	
					 /// Implementation.
					 /// Parses the file in path using a scriptparser
					 /// object and puts the result in out.
					 /// \return Status code (200 or 404)
	virtual int		 run (string &path, string &postbody,
						  value &inhdr, string &out, value &outhdr,
						  value &env, tcpsocket &s);
						  
protected:
	value			 config;	///< Configuration data
	dictionary<scriptparser>
					 scriptcache; ///< Cache of parsed scripts (indexed by path)
};

/// Convenient base class for a dynamic page.
/// Wraps up HTTP POST data into an environment, then calls its virtual
/// serverpage::execute() method.
class serverpage : public httpdobject
{
public:
					 /// Constructor.
					 /// \param pparent Reference to parent httpd.
					 /// \param uri The URI to match requests to.
					 serverpage (class httpd &pparent, const string &uri);
					~serverpage (void);

					 /// Picks up the request.
					 /// Parses post/get data into a value object,
					 /// then calls execute() to do further processing.
					 /// \return The returned statuscode from execute().
	int				 run (string &uri, string &postbody,
						  value &inhdr, string &out, value &outhdr,
						  value &env, tcpsocket &s);

					 /// Implementation.
					 /// Subclasses should override this method.
					 /// \param env Service environment.
					 /// \param var GET/POST variables.
					 /// \param out Output buffer.
					 /// \param outhdr Output headers.
					 /// \return HTTP status code.
	virtual int		 execute (value &env, value &argv,
							  string &out, value &outhdr);	
};

/// Publish a directory. Normally used at the end of the chain. Publishes
/// all files under a configured root directory with a mapping from file
/// extensions to mimetypes (or custom handlers).
class httpdfileshare : public httpdobject
{
public:
					 /// Constructor.
					 /// \param pparent Reference to the parent httpd
					 /// \param purimatch Match criterium for the request URI
					 /// \param rootdir The shared root directory.
					 httpdfileshare (class httpd &pparent,
					 				 const string &purimatch,
					 				 const string &rootdir);
					~httpdfileshare (void);
	
					 /// Main implementation.
					 /// Resolves the URI from the rootdir provided in
					 /// the constructor. Returns \e 500 if the URI was
					 /// gibberish. If no file was found on the resolved
					 /// location, either the default 404 document is
					 /// sent and \e -404 is returned, or a very terse
					 /// default error is placed in the output buffer
					 /// and \e 404 is returned. If the URI points to
					 /// a directory, "index.html" is appended.
					 /// A resolved and readable file is sent directly
					 /// to the socket using tcpsocket::sendfile() and
					 /// \e -200 is returned.
	virtual int		 run (string &uri, string &postbody,
						  value &inhdr, string &out, value &outhdr,
						  value &env, tcpsocket &s);
	
	value			 mimedb; ///< Mapping from file extensions to mime.
	
					 /// Add a custom httpdfiletypehandler for a file
					 /// extension.
					 /// \param h The handler object.
					 /// \param x The extension (ie "php").
	void			 addhandler (httpdfiletypehandler *h, const statstring &x);
	
protected:
	dictionary <httpdfiletypehandler *>
					 filetypes; ///< Filetype handlers
	string			 root; ///< The root directory
	bool			 roothasvolume; ///< True if root dir uses an alias path.
};

// ------------------------------------------------------------------------
// CLASS httpdbasicauth: Implements http basic authentication for a uri
//                       realm with a provided userdb. Succesful logins
//                       will pass through with the username set in the
//                       environment variable "user".
// ------------------------------------------------------------------------

/// Authenticate requests. Uses the http basic authentication mechanism
/// to authenticate a request. Must be created together with an
/// object subclassed from httpdauthenticator (to handle the actual
/// username/password validation).
class httpdbasicauth : public httpdobject
{
public:
					 /// Constructor.
					 /// Sets up parameters for the run method.
					 /// The httpd will call the object if a request's
					 /// URI matches with the one specified at creation
					 /// time. You should create an authenticator object
					 /// before creating an instance of this class.
					 /// \param pparent Reference to the parent httpd.
					 /// \param purimatch Wildcard for uri matches.
					 /// \param outrealm Authentication realm name.
					 /// \param a The authentication class/object to use.
					 httpdbasicauth (class httpd &pparent,
					 				 const string &purimatch,
					 				 const string &outrealm,
					 				 class httpdauthenticator &a);
					~httpdbasicauth (void);
					
					 /// Implementation.
					 /// Adds the WWW-Authenticate string to the output
					 /// headers with the configured realm.
					 /// If no Authorization header is sent, a terse
					 /// error message is put in the output buffer
					 /// and 401 is returned. Otherwise the header
					 /// will be decoded into a username and password
					 /// part and sent to the httpdauthenticator object
					 /// that was passed to the constructor. If the
					 /// authentication failed, a terse error message
					 /// is put into the output buffer and 401 is returned.
					 /// On succesful authentication, the authenticated
					 /// username is put into env["user"] and 0 is returned
					 /// allowing the chain to continue.
					 /// \return Status code (401 or 0).
	virtual int		 run (string &uri, string &postbody,
						  value &inhdr, string &out, value &outhdr,
						  value &env, tcpsocket &s);

					 /// Set redirect target on failed non-empty
					 /// auth.
	void			 redirectto (const string &uri);

protected:
	httpdauthenticator	*auth; ///< The authenticator class.
	string				 realm; ///< The realm name.
	
						 /// If set, this string points to a URI that
						 /// will be used to redirect the user in case
						 /// a non-empty authentication token was
						 /// offered that did not authenticate.
	string				 redirurl;
};

// ------------------------------------------------------------------------
// CLASS httpdauthenticator: Abstract class representing an object that
//                           provides authentication to a httpd object.
// ------------------------------------------------------------------------

/// Abstract authenticator.
/// Defines behavior for authenticating requests against a user database.
/// Derived classes cover a specific implementation for getting to the
/// proper data, locally or remotely.
class httpdauthenticator
{
public:
						 httpdauthenticator (void);
	virtual				~httpdauthenticator (void);
	
						 /// Validate username/password.
						 /// \param user The username.
						 /// \param pass The password.
						 /// \return Status, \b true if validated.
	virtual	bool		 authenticate (const string &user, const string &pass,
									   const string &uri);
	
						 /// Get user data.
						 /// \param user The username.
						 /// \return The user object.
	virtual value		*getuser (const string &user);

};

// ------------------------------------------------------------------------
// CLASS valueauth: Implementation of httpdauthenticator that uses a
//                  database stored inside a value object for
//                  authentication.
// ------------------------------------------------------------------------

/// Authenticator against a value class.
/// Takes a value object containing a list keyed by username. Each child
/// object's value is the crypted password. Extra data for a user can be
/// kept in its attributes.
class valueauth : public httpdauthenticator
{
public:
						 /// Constructor.
						 /// Copies a user database from a provided value
						 /// object.
						 /// \param db The user database (will be copied).
						 valueauth (const value &db);
						~valueauth (void);
						
						 /// Validate username/password.
						 /// \param name The username.
						 /// \param pass The password.
	bool				 authenticate (const string &name, const string &pass,
									   const string &uri);
	
						 /// Get user data.
						 /// \param user The username.
						 /// \return The user object.
	value				*getuser (const string &);
	
protected:
	value				 userdb; ///< Local instance of the user database.
};

// ------------------------------------------------------------------------
// CLASS pwfileauth: Implementation of httpdauthenticator that uses a
//                   passwd file.
// ------------------------------------------------------------------------

/// Authenticator using passwd-style files.
/// Extra fields in the password database can also be named.
class pwfileauth : public httpdauthenticator
{
public:
						 /// Constructor.
						 /// \param path Location of the passwd file.
						 pwfileauth (const string &path);
						 
						 /// Constructor.
						 /// \param path Location of the passwd file.
						 /// \param fieldOne Key of the first extra field.
						 pwfileauth (const string &path,
						 			 const string &fieldOne);

						 /// Constructor.
						 /// \param path Location of the passwd file.
						 /// \param fieldOne Key of the first extra field.
						 /// \param fieldTwo Key of the second extra field.
						 pwfileauth (const string &path,
						 			 const string &fieldOne,
						 			 const string &fieldTwo);

						 /// Constructor.
						 /// \param path Location of the passwd file.
						 /// \param fieldOne Key of the first extra field.
						 /// \param fieldTwo Key of the second extra field.
						 /// \param fieldThree Key of the third extra field.
						 pwfileauth (const string &path,
						 			 const string &fieldOne,
						 			 const string &fieldTwo,
						 			 const string &fieldThree);

						 /// Constructor.
						 /// \param path Location of the passwd file.
						 /// \param fieldOne Key of the first extra field.
						 /// \param fieldTwo Key of the second extra field.
						 /// \param fieldThree Key of the third extra field.
						 /// \param fieldFour Key of the fourth extra field.
						 pwfileauth (const string &path,
						 			 const string &fieldOne,
						 			 const string &fieldTwo,
						 			 const string &fieldThree,
						 			 const string &fieldFour);

						 /// Constructor.
						 /// \param path Location of the passwd file.
						 /// \param fieldOne Key of the first extra field.
						 /// \param fieldTwo Key of the second extra field.
						 /// \param fieldThree Key of the third extra field.
						 /// \param fieldFour Key of the fourth extra field.
						 /// \param fieldFive Key of the fifth extra field.
						 pwfileauth (const string &path,
						 			 const string &fieldOne,
						 			 const string &fieldTwo,
						 			 const string &fieldThree,
						 			 const string &fieldFour,
						 			 const string &fieldFive);

						 /// Constructor.
						 /// \param path Location of the passwd file.
						 /// \param fieldOne Key of the first extra field.
						 /// \param fieldTwo Key of the second extra field.
						 /// \param fieldThree Key of the third extra field.
						 /// \param fieldFour Key of the fourth extra field.
						 /// \param fieldFive Key of the fifth extra field.
						 /// \param fieldSix Key of the sixth extra field.
						 pwfileauth (const string &path,
						 			 const string &fieldOne,
						 			 const string &fieldTwo,
						 			 const string &fieldThree,
						 			 const string &fieldFour,
						 			 const string &fieldFive,
						 			 const string &fieldSix);
						~pwfileauth (void);
						
						 /// Authenticate a user.
						 /// Looks at pwfileauth::lastcheck to see how
						 /// long ago the file was inspected. If it
						 /// could be considered expired (or was never
						 /// loaded yet) the file's modification time
						 /// is compared against pwfileauth::lastmod,
						 /// if the disk file is newer it is loaded into the
						 /// cache at pwfileauth::userdb.
						 /// Returns true if the username and password
						 /// were valid.
						 /// \param user The username.
						 /// \param pass The password.
	bool				 authenticate (const string &user, const string &pass,
									   const string &uri);
	value				*getuser (const string &);
	
protected:
	void				 checkrecord (void);
	void				 loadfile (void);
	lock<int>			 lck;
	value				 userdb;	///< Cached user database
	time_t				 lastmod;	///< Last modified date last time we checked.
	time_t				 lastcheck; ///< Last time we checked the original file.
	string				 filename; ///< Location of the passwd file.
	value				 fieldnames; ///< Array of the extra field keys.
};

// ------------------------------------------------------------------------
// CLASS httpdrewrite: Implements an URI rewriting mechanism with
//                     regular expressions.
// ------------------------------------------------------------------------

/// A path rewriting class.
class httpdrewrite : public httpdobject
{
public:
					 /// Constructor.
					 /// \param pparent Reference to the parent httpd.
					 /// \param purimatch Wildcard match.
					 /// \param pregexp Regular expression to use for rewriting.
					 httpdrewrite (class httpd &pparent,
					 			   const string &purimatch,
					 			   const string &pregexp);
					~httpdrewrite (void);
	
					 /// Implementation.
					 /// Applies the regexp in httpdrewrite::rule to
					 /// the uri.
					 /// \return Always \b 0.
	virtual int		 run (string &uri, string &postbody,
						  value &inhdr, string &out, value &outhdr,
						  value &env, tcpsocket &s);

protected:
	string			 rule; ///< The regular expression.
};

// ------------------------------------------------------------------------
// CLASS httpdvhost: Implements virtual hosts mapping to a subdirectory
//                   inside the webroot. Performs this trick with URI
//                   rewriting.
// ------------------------------------------------------------------------

/// Rewriting engine for virtual host.
/// Keeps a database that maps a HTTP host header to a sub-directory.
/// The uri is rewritten with this sub-directory added at the root level.
/// If no match is found, the key "*" is used as a default.
class httpdvhost : public httpdobject
{
public:
					 /// Constructor.
					 /// \param pparent Parent httpd.
					 /// \param phostdb The vhosts database.
					 httpdvhost (class httpd &pparent,
					 			 const value &phostdb);
					~httpdvhost (void);
					
					 /// Implementation.
					 /// Looks up the host header in the hostdb and
					 /// inserts the value of the record with that
					 /// key to the left of uri. If no record for
					 /// the host header was found inside hostdb
					 /// it will refer to a special with the key "*".
					 /// If this is also not found, the uri is left
					 /// untouched.
					 /// \return Always \b 0.
	virtual int		 run (string &uri, string &postbody,
						  value &inhdr, string &out, value &outhdr,
						  value &env, tcpsocket &s);

protected:
	value			 hostdb; ///< Internal vhosts database.
};


// ------------------------------------------------------------------------
// CLASS httpdeventhandler: Base class for objects that want to handle
//                          generic 
// ------------------------------------------------------------------------

/// Abstract class for event loggers.
class httpdeventhandler
{
public:
						 /// Constructor.
						 /// \param parent The parent httpd.
						 /// \param inclasses A logical OR of the classes
						 ///                  the object will handle.
						 httpdeventhandler (httpd &parent,
						 					httpdeventclass inclasses);
						
	virtual				~httpdeventhandler (void)
						 {
						 }
					 
					 	 /// Virtual handler method.
					 	 /// Events are value objects with an attribute
					 	 /// "class" that is one of:
					 	 ///   - "info" with the following child keys:
					 	 ///     - type: "threadstarted", "threadstopped",
					 	 ///               "connectionaccepted",
					 	 ///               "connectionclosed"
					 	 ///     - thread: string with thread id.
					 	 ///     - load: connection load (not for thread messages)
					 	 ///     - ip: peer address (not for thread messages)
					 	 ///   - "access":
					 	 ///     - method: HTTP method (POST or GET)
					 	 ///     - httpver: HTTP version used (1.0, 1.1)
					 	 ///     - uri: Requested URI
					 	 ///     - file: Path to requested file
					 	 ///     - ip: Peer address
					 	 ///     - user: Authenticated user (if any)
					 	 ///     - referrer: HTTP referrer (if any)
					 	 ///     - useragent: HTTP user-agent
					 	 ///     - status: HTTP status code
					 	 ///     - bytes: Bytes transferred
					 	 ///   - "error":
					 	 ///     - ip: Peer address
					 	 ///     - text: Error text
					 	 /// 
					 	 /// These are some example events:
					 	 /// \verbinclude httpd_events.xml
	virtual int			 handle (const value &ev);
	
						 /// Which event classes to apply. Is a combination
						 /// of HTTPD_ACCESS, HTTPD_ERROR and HTTPD_INFO.
	int					 classmatch;
	httpdeventhandler	*next; ///< Linked list pointer.
	
protected:
	httpd				*parent; ///< Link to parent httpd.
};

// ------------------------------------------------------------------------
// CLASS httpdlogger: A httpdeventhandler that writes ncsa-style logs.
// ------------------------------------------------------------------------

/// Apache-style access and error log writer.
class httpdlogger : public httpdeventhandler
{
public:
						 /// Constructor.
						 /// \param parent Link to parent httpd.
						 /// \param accesslog Path to access logfile.
						 /// \param errorlog Path to error logfile (empty for none)
						 /// \param ms Maximum size of a logfile before rotating.
						 httpdlogger (httpd &parent,
						 			  const string &accesslog,
						 			  const string &errorlog="",
						 			  unsigned int ms = defaults::sz::logfile);
						 
						 /// Destructor.
						~httpdlogger (void);

						 /// Implementation.
						 /// Writes access events to the access log file.
						 /// If haserrorlog is true, error events are
						 /// also written to the error log file.
	virtual int			 handle (const value &);
	
protected:
	lock<file>			 faccess; ///< The access log file
	lock<file>			 ferror; ///< The error log file
	bool				 haserrorlog; ///< True if there is an error log.
	string				 accessPath; ///< Path to the access log
	string				 errorPath; ///< Path to the error log
};

// ------------------------------------------------------------------------
// CLASS httpd: The root http daemon. Will only return 404s unless if
//              you add httpdobjects to do something interesting.
//              This class will just do the housekeeping of httpdworker
//              threadds and keep track of the httpdobject chain as
//              well as the httpdeventhandler chain.
// ------------------------------------------------------------------------

/// The root httpd daemon class.
/// This will only return generic 404 errors if no httpdobjects are linked
/// to implement some server behavior.
/// Here's a typical example of a httpd being used together with a small
/// chain of httpdobjects to create a basic static webserver:
/// \verbinclude httpd_ex1.cpp
class httpd : public thread
{
public:
					 /// Constructor, create httpd object
					 //  using a Unix Domain Socket for 
					 /// communication
					 /// \param path listening socket path
					 /// \param mint Minimum number of threads
					 /// \param maxt Maximum number of threads
					 httpd (const string &path, int mint=2, int maxt=4);

					 /// Constructor.
					 /// \param listenport The tcp port to listen to
					 /// \param inmint Minimum number of threads.
					 /// \param inmaxt Maximum number of threads.
					 httpd (int listenport, int inmint=2, int inmaxt=4);

					 /// Delayed initialization constructor.
					 /// For situations where you want to define the
					 /// listenport later.
					 httpd (void);
					 
					 /// Set listen-port post-facto.
					 /// If the httpd object was created through
					 /// the constructor without a listenport, this
					 /// function binds the listening socket to
					 /// a specific port.
					 /// \param port The port number.
	void			 listento (int port);
	
					 /// Set listen-port and address post-facto.
					 /// If the httpd object was created through
					 /// the constructor without a listenport, this
					 /// function binds the listening socket to
					 /// a specific ip address and port.
					 /// \param addr The listening address.
					 /// \param port The listening port.
	void			 listento (ipaddress addr, int port);
					 
					 /// Sey Unix listen socket
					 /// Only usefull if you used the constructor
					 /// without arguments
	void			 listento (const string &unixsock);

					 /// Destructor.					 
					~httpd (void);

					 /// Start the server process. Threads are not
					 /// started on construction, to allow a full chain
					 /// of httpdobjects to be built before serving
					 /// requests.
	void			 start (void) { spawn(); };	
	
					 /// Get current server load.
					 /// \return Number of open connections.
	int				 getload (void);
					 
					 /// Set a default document. Links a file path to
					 /// a HTTP status code.
					 /// \param sti The status code
					 /// \param file The path to the default file.
	void			 setdefaultdocument (int sti, const string &file);

	// ---------------------------------------------------------------------
	// Property methods
	// ---------------------------------------------------------------------
	
					 /// Returns the minimum number of threads.
	int				 minthreads (void) { return minthr; };
					 /// Returns the maximum number of threads.
	int				 maxthreads (void) { return maxthr; };
					 /// Returns the maximum HTTP post size.
	int				 maxpostsize (void) { return _maxpostsize; };
					 /// Returns the system path.
	const string	&systempath (void) { return syspath; };

					 /// Set the minimum number of threads.
	void			 minthreads (int i) { minthr = i; };
					 /// Set the maximum number of threads.
	void			 maxthreads (int i) { maxthr = i; };
					 /// Set the maximum HTTP post size.
	void			 maxpostsize (int i) { _maxpostsize = i; };
					 /// Set the system path.
	void			 systempath (const string &str) { syspath = str; };
	
	
	// ---------------------------------------------------------------------
	// Methods below this line run in another thread context
	// ---------------------------------------------------------------------
	
					 /// The main thread implementation.
	virtual void	 run (void);
	
					 /// Link a httpdobject to the end of the chain.
					 /// Called from the constructor of the httpdobject
					 /// base class.
	void			 addobject (httpdobject *);
	
					 /// Link an event handler.
	void			 addeventhandler (httpdeventhandler *);
	
					 /// Handle a request through the chain.
					 /// \param uri The uri of the request
					 /// \param postbody The posted body data
					 /// \param inhdr The received headers
					 /// \param method The HTTP method
					 /// \param httpver The HTTP version
					 /// \param s The tcpsocket handling the request
					 /// \param keepalive Whether HTTP keepalive should be used.
	void			 handle (string &uri, string &postbody, value &inhdr,
							 const string &method, const string &httpver,
							 tcpsocket &s, bool &keepalive);
	
					 /// Handle an event through the chain of event handlers.
					 /// Some example events:
					 /// \verbinclude httpd_events.xml
	void			 eventhandle (const value &);
	
					 /// Use sendfile symantics to send a disk file.
					 /// \param s The request's tcpsocket.
					 /// \param fn The path of the file to send.
					 /// \return Number of bytes sent.
	unsigned int	 sendfile (tcpsocket &s, const string &fn);
	
					 /// Default document checker.
					 /// \param sti The http status code.
					 /// \return \b true if there is a default document
					 ///         defined for the status.
	bool			 havedefault (int sti);
					 
					 /// Default document path.
					 /// \param sti The http status code.
					 /// \return Path to the default document for provided status.
	const string	&defaultdocument (int sti);
	
					 /// Shut down all threads.
	void			 shutdown (void);
	
	value			 defaultdocuments; ///< Default document database.
	tcplistener		 listener; ///< The listening socket.
	lock<int>		 load; ///< The current connection load.
	lock<int>		 tcplock; ///< Lock for the tcp listener.
	threadgroup		 workers; ///< The httpd worker threads.
	int				 eventmask; ///< Which event classes need handling.
	
protected:
	httpdobject			*first; ///< Linked list of httpdobjects.
	httpdeventhandler	*firsthandler; ///< Linked list of event handlers.
	int					 minthr; ///< Minimum threads.
	int					 maxthr; ///< Maximum threads.
	string				 syspath; ///< System path.
	int					_maxpostsize; ///< Max post size.
	bool				_shutdown; ///< True if shutdown mode is on.
	conditional			 shutdowndone; ///< Shutdown conditional
};

// ------------------------------------------------------------------------
// CLASS httpdworker: Implements a worker thread doing the httpd
//                    gruntwork.
// ------------------------------------------------------------------------

$exception (httpdWorkerException, "Error in worker");

/// Worker thread handling an http connection.
class httpdworker : public groupthread
{
public:
					 /// Construcotr. Spawns the thread.
					 /// \param pop Parent httpd object.
					 httpdworker (httpd *pop);
					 
					 /// Destructor.
					~httpdworker (void);
					 
					 /// Thread implementation.
					 /// Does a non-blocking accept on the parent's
					 /// tcp listener every 5 seconds. Shuts down
					 /// during this loop when it receives an event
					 /// with ev["command"] set to "die".
					 /// If a connection is accepted, it parses the
					 /// HTTP command and headers. Then it sets
					 /// httpd::handle() on the case.
	virtual void	 run (void);

protected:
	httpd			*parent; ///< Link to parent httpd.
};

#endif
