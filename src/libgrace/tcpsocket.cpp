// ========================================================================
// tcpsocket.cpp: File class that can handle TCP socket connections.
//
// (C) Copyright 2003-2006 Pim van Riezen <pi@madscience.nl>
//                         Madscience Labs, Rotterdam 
// ========================================================================

//      012345678 TAB-O-METER should measure 4
//      ^	^

#include <grace/tcpsocket.h>
#include <grace/value.h>
#include <grace/system.h>
#include <grace/defaults.h>
#include <grace/filesystem.h>
#include "platform.h"

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/errno.h>
#ifdef HAVE_PASSCRED
  #include <linux/socket.h>
  #include <sys/uio.h>
#endif
#ifdef HAVE_SENDFILE
  #include <sys/sendfile.h>
#endif
#include <unistd.h>
#include <netdb.h>

// ========================================================================
// FUNCTION __grace_internal_gethostbyname
// ---------------------------------------
// Threadsafe implementation of gethostbyname(). Some platforms export
// a gethostbyname_r() but it is generally not worth the trouble; Name
// resolution is generally associated with the establishment of a
// connection and is, overall, an unpredictable and time consuming process
// that should not be invoked at time-critical moments. For this reason
// the function uses the easier and more portable method of using the
// non-reentrant gethostbyname() function guarded by a spinlock.
// ========================================================================
struct hostent *__grace_internal_gethostbyname (const char *name)
{
	static lock<bool> lck;
	struct hostent *reply;
	struct hostent *result = NULL;
	
	exclusivesection (lck)
	{
		reply = ::gethostbyname (name);
		if (reply)
		{
			int aliascount;
			int addr_count;
			int i;
			
			// How do I hate thee, let me count the ways...
			result = new struct hostent;
			result->h_name = ::strdup (reply->h_name);
			for (aliascount=0; reply->h_aliases[aliascount]; ++aliascount);
			result->h_aliases = (char **) malloc ((aliascount+1) * sizeof(char *));
			for (i=0; i<aliascount; ++i)
				result->h_aliases[i] = ::strdup (reply->h_aliases[i]);
			result->h_aliases[i] = NULL;
			result->h_addrtype = reply->h_addrtype;
			result->h_length = reply->h_length;
			for (addr_count=0; reply->h_addr_list[addr_count]; ++addr_count);
			result->h_addr_list = (char **) malloc ((addr_count+1) * sizeof(char *));
			for (i=0; i<addr_count; ++i)
			{
				result->h_addr_list[i] = (char *) malloc ((size_t) reply->h_length);
				::memcpy (result->h_addr_list[i], reply->h_addr_list[i], reply->h_length);
			}
			result->h_addr_list[i] = NULL;
		}
	}
	return result;
}

// ========================================================================
// FUNCTION __grace_internal_freehostent
// -------------------------------------
// Deallocate memory of structure returned by the internal
// gethostbyname() function.
// ========================================================================
void __grace_internal_freehostent (struct hostent *he)
{
	int i;
	free (he->h_name);
	for (i=0; he->h_aliases[i]; ++i) free (he->h_aliases[i]);
	free (he->h_aliases);
	for (i=0; he->h_addr_list[i]; ++i) free (he->h_addr_list[i]);
	free (he->h_addr_list);
	delete he;
}

// ========================================================================
// METHOD ::connect
// ----------------
// Attempt to connect to a specified host and port. This operation
// can block on name resolution and tcp negociations. It returns
// false if the connection failed. In the rare case no socket
// could be created, it throws an exception.
// ========================================================================
bool tcpsocket::connect (
  const string &host, int port)
{
	struct sockaddr_in	 remote;
	struct sockaddr_in	 local;
	struct in_addr		 bindaddr;
	struct hostent 		*myhostent;
	int					 pram = 1;
	
	buffer.flush();
	
	if (filno >= 0)
	{
		errcode = FERR_CONNECT2;
		err = "Connecting twice on open socket";
		return false;
	}
	
	filno = socket (PF_INET, SOCK_STREAM, 0);
	if (filno >= 0)
	{
		setsockopt (filno, SOL_SOCKET, SO_KEEPALIVE,
					(char *) &pram, sizeof (int));
					
		remote.sin_family = AF_INET;
		remote.sin_port   = htons (port);
		
		myhostent = __grace_internal_gethostbyname (host);
		if (myhostent)
		{
			peer_addr = ntohl (*(myhostent->h_addr));
			peer_name = host;
			peer_port = port;
			
			memcpy (&bindaddr, myhostent->h_addr,
					sizeof (struct in_addr));
			
			__grace_internal_freehostent (myhostent);
			
			remote.sin_addr = bindaddr;

			// If an address to bind is set.. first bind 
			if( localbindaddr )
			{
				local.sin_family 		  = AF_INET;
				local.sin_port			  = 0;
				local.sin_addr.s_addr = inet_addr( localbindaddr.cval() );
					  
				::bind( filno, (struct sockaddr *)&local, sizeof(local) );
			}
			
			if (::connect (filno, (const sockaddr *) &remote,
						   sizeof (struct sockaddr_in)) != 0)
			{
				::close (filno);
				filno = -1;
				errcode = FERR_NOCONNECT;
				err.crop (0);
				err.printf ("TCP connection failed: %s", strerror (errno));
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		errcode = FERR_NORSRC;
		err = "Could not create a socket";
		throw (EX_SOCK_CREATE);
	}
	
	feof = false;
	ti_established = kernel.time.now ();
	if (codec)
	{
		size_t szleft;
		size_t szdone;
		size_t sz;
		
		if (! codec->setup())
		{
			errcode = FERR_CODEC;
			err.crop();
			err.printf ("Codec error: %s", codec->error().str());
			::close (filno);
			filno = -1;
			feof = true;
			return false;
		}
		
handshakes:
		string dt;
		codec->peekoutput(dt);
		szleft = dt.strlen();
		szdone = 0;
		
		if (! szleft) return true;
		
		while (szleft > 0)
		{
			sz = ::write (filno, dt.str() + szdone, szleft);
			if (sz<=0)
			{
				::close (filno);
				filno = -1;
				feof = true;
				errcode = FERR_CODEC;
				err = "Codec handshake i/o error";
				return false;
			}
			szdone += sz;
			szleft -= sz;
		}
		if (szdone)
		{
			char buf[4096];
			codec->doneoutput (szdone);
			sz = ::read (filno, buf, 4095);
			if (sz>0)
			{
				if (! codec->addinput (buf, sz))
				{
					::close (filno);
					filno = -1;
					feof = true;
					errcode = FERR_CODEC;
					err = "Codec handshake return error";
					codec->reset();
					return false;
				}
				try
				{
					codec->fetchinput (buffer);
				}
				catch (...)
				{
					errcode = FERR_CODEC;
					err.crop(0);
					err.printf ("Codec error: %s", codec->error().str());
					codec->reset();
					return false;
				}
			}
			goto handshakes;
		}
	}
	return true;
}

// ========================================================================
// METHOD bindtoaddr
// ========================================================================
bool tcpsocket::bindtoaddr (const string &address)
{
	localbindaddr = address; 
	return true;
}

// ========================================================================
// METHOD uconnect
// ---------------
// Connect to a specific unix domain socket.
// ========================================================================
bool tcpsocket::uconnect (const string &path)
{
	struct sockaddr_un	 remote;
	int					 pram = 1;
	string 				 realpath;
	
	filno = socket (PF_UNIX, SOCK_STREAM, 0);
	if (filno<0)
	{
		errcode = FERR_NORSRC;
		err = "Could not create a socket";
		throw (EX_SOCK_CREATE);
	}
	
	realpath = fs.transr (path);
	
	remote.sun_family = AF_UNIX;
	::strncpy (remote.sun_path, realpath.str(), 107);
	remote.sun_path[107] = 0;
	
	buffer.flush ();
	
	if (::connect (filno, (const sockaddr *) &remote,
				   sizeof (struct sockaddr_un)) != 0)
	{
		::close (filno);
		filno = -1;
		return false;
	}
	
	feof = false;
	peer_addr = 0;
	peer_name = path;
	peer_port = 0;

	#ifdef HAVE_PASSCRED	
		setsockopt (filno, SOL_SOCKET, SO_PASSCRED, (char *) &pram,
					sizeof (int));
	#endif
	ti_established = kernel.time.now ();
	if (codec)
	{
		if (! codec->setup())
		{
			errcode = FERR_CODEC;
			err.crop();
			err.printf ("Codec error: %s", codec->error().str());
			::close (filno);
			filno = -1;
			feof = true;
			return false;
		}
	}
	
	return true;
}


// ========================================================================
// METHOD getcredentials
// ---------------------
// Receive remote user credentials for the other end of a unix
// domain socket.
// ========================================================================
void tcpsocket::getcredentials (void)
{
#ifdef HAVE_PASSCRED
	struct ucred credp;
	char buf[16];
	unsigned bufsiz = 8;
	void *addr;
	socklen_t *alen;
	
	int z;
	struct msghdr msgh;
	struct iovec iov[1];
	struct cmsghdr *cmsgp = NULL;
	char mbuf[CMSG_SPACE(sizeof (credp))];
	int pram = 1;

	setsockopt (filno, SOL_SOCKET, SO_PASSCRED, (char *) &pram,
				sizeof (int));
	
	memset (&msgh,0,sizeof (msgh));
	memset (mbuf,0,sizeof (mbuf));
	msgh.msg_name = NULL;
	msgh.msg_namelen = 0;
	
	msgh.msg_iov = iov;
	msgh.msg_iovlen = 1;
	
	iov[0].iov_base = buf;
	iov[0].iov_len = bufsiz;
	
	msgh.msg_control = mbuf;
	msgh.msg_controllen = sizeof (mbuf);
	
	do {
		z = recvmsg (filno, &msgh, 0);
	} while ( (z==-1)&&(errno==EINTR) );
	
	if (z != -1)
	{
		for (cmsgp=CMSG_FIRSTHDR(&msgh);cmsgp;cmsgp=CMSG_NXTHDR(&msgh,cmsgp))
		{
			if ((cmsgp->cmsg_level == SOL_SOCKET) &&
			    (cmsgp->cmsg_type == SCM_CREDENTIALS))
			{
				credp = *(struct ucred *) CMSG_DATA(cmsgp);
				peer_pid = credp.pid;
				peer_uid = credp.uid;
				peer_gid = credp.gid;
				return;
			}
		}
	}
#endif
}

// ========================================================================
// METHOD sendcredentials
// ========================================================================
void tcpsocket::sendcredentials (void)
{
#ifdef HAVE_PASSCRED
	int foo;
	
	write (filno, &foo, sizeof (foo));
#endif
}

// ========================================================================
// METHOD ::sendfd
// ---------------
// Transmit a file descriptor over a unix socket.
// ========================================================================
void tcpsocket::sendfd (file &fil)
{
#ifdef HAVE_PASSCRED
	int fd = fil.filno;
	struct msghdr msgh;
	struct iovec iov[1];
	struct cmsghdr *cmsgp = NULL;
	char buf[CMSG_SPACE(sizeof(fd))];
	int er=0;
	int z;
	
	memset (&msgh,0,sizeof (msgh));
	memset (buf,0,sizeof (buf));
	
	msgh.msg_name = NULL;
	msgh.msg_namelen = 0;
	
	msgh.msg_iov = iov;
	msgh.msg_iovlen = 1;
	
	iov[0].iov_base = &er;
	iov[0].iov_len = sizeof (er);
	
	msgh.msg_control = buf;
	msgh.msg_controllen = sizeof (buf);
	
	cmsgp = CMSG_FIRSTHDR (&msgh);
	cmsgp->cmsg_level = SOL_SOCKET;
	cmsgp->cmsg_type = SCM_RIGHTS;
	cmsgp->cmsg_len = CMSG_LEN(sizeof (fd));
	
	*((int *)CMSG_DATA(cmsgp)) = fd;
	msgh.msg_controllen = cmsgp->cmsg_len;
	
	do {
		z = sendmsg (filno, &msgh, 0);
	} while ( (z==-1) && (errno==EINTR) );
#endif
}

// ========================================================================
// METHOD ::getfd
// -----------------
// Receive a file descriptor over a unix socket.
// ========================================================================
file *tcpsocket::getfd (void)
{
#ifdef HAVE_PASSCRED
	int fd;
	char buf[16];
	unsigned bufsiz = 8;
	void *addr;
	socklen_t *alen;
	file *res = new file;
	
	int z;
	struct msghdr msgh;
	struct iovec iov[1];
	struct cmsghdr *cmsgp = NULL;
	char mbuf[CMSG_SPACE(sizeof (int))];
	
	memset (&msgh,0,sizeof (msgh));
	memset (mbuf,0,sizeof (mbuf));
	msgh.msg_name = NULL;
	msgh.msg_namelen = 0;
	
	msgh.msg_iov = iov;
	msgh.msg_iovlen = 1;
	
	iov[0].iov_base = buf;
	iov[0].iov_len = bufsiz;
	
	msgh.msg_control = mbuf;
	msgh.msg_controllen = sizeof (mbuf);
	
	do {
		z = recvmsg (filno, &msgh, 0);
	} while ( (z==-1)&&(errno==EINTR) );
	
	if (z != -1)
	{
		for (cmsgp=CMSG_FIRSTHDR(&msgh);cmsgp;cmsgp=CMSG_NXTHDR(&msgh,cmsgp))
		{
			if ((cmsgp->cmsg_level == SOL_SOCKET) &&
			    (cmsgp->cmsg_type == SCM_RIGHTS))
			{
				fd = *(int *) CMSG_DATA(cmsgp);
				res->openread (fd);
				return res;
			}
		}
	}
	return res;
#else
	return NULL;
#endif
}

// ========================================================================
// METHOD operator=
// ----------------
// Derive a socket from another object
// ========================================================================
tcpsocket &tcpsocket::operator= (tcpsocket &s)
{
	derive (s);
	return (*this);
}

tcpsocket &tcpsocket::operator= (tcpsocket *s)
{
	if (! s) return *this;
	derive (s);
	return (*this);
}

// ========================================================================
// CONSTRUCTOR
// -----------
// Creator for a listener object for a listening tcp socket
// on the specified tcp port.
// ========================================================================
tcplistener::tcplistener (int port)
{
	listento (port);
}

tcplistener::tcplistener (void)
{
	unprotected (sock) { sock = 0; }
	listening = false;
	tcpdomain = true;
	tcpdomainport = 0;
}

// ========================================================================
// METHOD ::listento
// -----------------
// Set up listening on a tcp port.
// ========================================================================
void tcplistener::listento (int port)
{
	exclusivesection (sock)
	{
		if (listening)
		{
			close (sock);
			listening = false;
		}
		
		tcpdomain = true;
		tcpdomainport = port;
		
		struct sockaddr_in	 remote;
		struct in_addr		 bindaddr;
		struct hostent 		*myhostent;
		int					 pram = 1;
	
		bzero ((char *) &remote, sizeof (remote));
		remote.sin_family = AF_INET;
		remote.sin_addr.s_addr = htonl (INADDR_ANY);
		remote.sin_port = htons (port);
		
		sock = socket (AF_INET, SOCK_STREAM, 0);
		if (sock < 0)
		{
			breaksection throw (EX_SOCK_CREATE);
		}
		
		setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, (char *) &pram,
					sizeof (int));
					
	#ifdef SO_REUSEPORT
		setsockopt (sock, SOL_SOCKET, SO_REUSEPORT, (char *) &pram,
					sizeof (int));
	#endif
	
		if (bind (sock, (struct sockaddr *) &remote, sizeof (remote)) < 0)
		{
			close (sock);
			breaksection throw (EX_SOCK_CREATE);
		}
		
		listen (sock, tune::tcplistener::backlog);
		listening = true;
	}
}

// ========================================================================
// CONSTRUCTOR
// -----------
// Constructor variant for unix domain sockets.
// ========================================================================
tcplistener::tcplistener (const string &path)
{
	listento (path);
}

// ========================================================================
// METHOD ::listento
// -----------------
// Set up listening on a unix path.
// ========================================================================
void tcplistener::listento (const string &path)
{
	exclusivesection (sock)
	{
		if (listening)
		{
			close (sock);
			listening = false;
		}
		
		string realpath;
		realpath = fs.transw (path);
		
		tcpdomain = false;
		unixdomainpath = path;
		
		struct sockaddr_un remote;
		int    pram = 1;
		
		bzero ((char *) &remote, sizeof (remote));
		remote.sun_family = AF_UNIX;
		strncpy (remote.sun_path, realpath.str(), 107);
		remote.sun_path[107] = 0;
		
		sock = socket (AF_UNIX, SOCK_STREAM, 0);
		if (sock < 0)
		{
			breaksection throw (EX_SOCK_CREATE);
		}
		
		setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, (char *) &pram,
					sizeof (int));
		
		if (bind (sock, (struct sockaddr *) &remote, sizeof (remote)) < 0)
		{
			close (sock);
			breaksection throw (EX_SOCK_CREATE);
		}
		
		listen (sock, tune::tcplistener::backlog);
		listening = true;
	}
}

// ========================================================================
// DESTRUCTOR
// ----------
// Close the open socket
// ========================================================================
tcplistener::~tcplistener (void)
{
	unprotected (sock) { ::close (sock); }
}

// ========================================================================
// METHOD accept
// -------------
// Waits for an incoming connection and creates a tcpsocket object
// for it.
// ========================================================================
tcpsocket *tcplistener::accept (void)
{
	struct sockaddr_in	remote;
	struct sockaddr_in	peer;
	socklen_t anint=1;
	int pram=1;
	unsigned int raddr;
	
	int s;
	unprotected (sock)
	{
		s = ::accept (sock, (struct sockaddr *) &remote, &anint);
	}
	
	if (s<0)
	{
		throw (EX_SOCK_CREATE);
	}
	
	if (tcpdomain)
	{
		anint = sizeof (peer);
		getpeername (s, (struct sockaddr *) &peer, &anint);
		raddr = ntohl (peer.sin_addr.s_addr);
	}

	setsockopt (s, SOL_SOCKET, SO_KEEPALIVE, (char *) &pram,
				sizeof (int));	
	
	tcpsocket *myfil = new tcpsocket;
	(*myfil).openread (s);
	if (tcpdomain)
	{
		(*myfil).peer_addr = raddr;
		(*myfil).peer_port = ntohs (peer.sin_port);
		(*myfil).peer_name = ip2str (raddr);
	}
	(*myfil).ti_established = kernel.time.now();
	return myfil;
}

// ========================================================================
// METHOD ::tryaccept
// ------------------
// Same as the accept method, but with an optional timeout. An
// unopened file evaluates to the boolean 'false', so this works
// out pretty transparently, like:
//
// tcpsocket foo;
// if (foo = mylistener.tryaccept (2.0)) { ... }
// 
// The actual return data when a timeout occurs or the accept fails
// is a NULL-pointer.
// ========================================================================
tcpsocket *tcplistener::tryaccept (double timeout)
{
	struct sockaddr_in	remote;
	struct sockaddr_in	peer;
	socklen_t anint=1;
	int pram=1;
	unsigned int raddr;
	fd_set fds;
	int s = -1;
	struct timeval tv;
	
	if (timeout < 0.0) return NULL;
	
	tv.tv_sec = (int) timeout;
	tv.tv_usec = (int) (10000.0 * ( timeout - ((double) tv.tv_sec)));

	int selresult;
	unprotected (sock)
	{
		FD_ZERO (&fds);
		FD_SET (sock, &fds);
		selresult = select (sock+1, &fds, NULL, NULL, &tv);
	}
	
	if (selresult > 0)
	{
		exclusivesection (sock)
		{
			FD_ZERO (&fds);
			FD_SET (sock, &fds);
			tv.tv_sec = tv.tv_usec = 0;
			if (select (sock+1, &fds, NULL, NULL, &tv) > 0)
			{
				s = ::accept (sock, (struct sockaddr *) &remote, &anint);
			}
		}
	}
	
	if (s<0)
	{
		return NULL;
	}
	
	anint = sizeof (peer);
	getpeername (s, (struct sockaddr *) &peer, &anint);
	raddr = ntohl (peer.sin_addr.s_addr);

	setsockopt (s, SOL_SOCKET, SO_KEEPALIVE, (char *) &pram,
				sizeof (int));	
	
	tcpsocket *myfil = new tcpsocket;
	(*myfil).openread (s);
	(*myfil).peer_addr = raddr;
	(*myfil).peer_port = ntohs (peer.sin_port);
	(*myfil).peer_name = ip2str (raddr);
	(*myfil).ti_established = kernel.time.now();
	return myfil;
}

// ========================================================================
// METHOD ::sendfile
// -----------------
// Uses the sendfile syscall, if available, to effectively stream
// a file from disk to a tcpsocket.
// ========================================================================
void tcpsocket::sendfile (const string &path, unsigned int amount)
{
#ifdef HAVE_SENDFILE
	file fi;
	off_t off = 0;
	
	fi.openread (path);
	::sendfile (filno, fi.filno, &off, amount);
	fi.close();
#else
	file fi;
	string buf;
	
	fi.openread (path);
	
	try {
		while (! fi.eof())
		{
			buf = fi.read (4096);
			if (buf.strlen()) puts (buf);
		}
	}
	catch (...)
	{
	}
#endif
}
