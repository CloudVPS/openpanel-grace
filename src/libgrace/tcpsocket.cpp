// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

// ========================================================================
// tcpsocket.cpp: File class that can handle TCP socket connections.
// ========================================================================

//      012345678 TAB-O-METER should measure 4
//      ^	^

#include <grace/tcpsocket.h>
#include <grace/value.h>
#include <grace/system.h>
#include <grace/defaults.h>
#include <grace/filesystem.h>
#include <grace/netdb.h>
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
	
	exclusiveaccess (lck)
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
				::memmove (result->h_addr_list[i], reply->h_addr_list[i], reply->h_length);
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
// CONSTRUCTOR tcpsocket
// ========================================================================
tcpsocket::tcpsocket (void)
	: file ()
{
	peer_pid = 0;
	peer_uid = 65534;
	peer_gid = 65534;
	peer_addr = ipaddress();
	peer_port = 0;
	ti_established = core.time.now();
	
	localbindaddr = "";
}

// ========================================================================
// DESTRUCTOR tcpsocket
// ========================================================================
tcpsocket::~tcpsocket (void)
{
}

// ========================================================================
// METHOD ::connect
// ----------------
// Attempt to connect to a specified host and port. This operation
// can block on name resolution and tcp negociations. It returns
// false if the connection failed. In the rare case no socket
// could be created, it throws an exception.
// ========================================================================
bool tcpsocket::connect (const string &host, int port)
{
	return connect ((ipaddress) netdb::resolve (host), port);
}

bool tcpsocket::connect (ipaddress addr, int port)
{
	sockaddr_in6 remotev6;
	sockaddr_in remotev4;
	sockaddr_in6 localv6;
	sockaddr_in localv4;
	
	int				 pram = 1;
	
	if (! addr) return false;
	
	buffer.flush();
	
	if (filno >= 0)
	{
		errcode = FERR_CONNECT2;
		err = errortext::sock::isconn;
		return false;
	}
	
	memset (&remotev6, 0, sizeof (remotev6));
	memset (&remotev4, 0, sizeof (remotev4));
	memset (&localv6, 0, sizeof (localv6));
	memset (&localv4, 0, sizeof (localv4));
	bool usev4 = false;
		
	if (addr.isv4())
	{
		filno = socket (PF_INET6, SOCK_STREAM, 0);	
		
		if (filno < 0 && errno == EAFNOSUPPORT)
		{
			filno = socket (PF_INET, SOCK_STREAM, 0);	
			remotev4.sin_family = AF_INET;
			remotev4.sin_addr = addr;
			remotev4.sin_port = htons (port);
			usev4 = true;
		}		
		else
		{
			remotev6.sin6_family = AF_INET6;
			remotev6.sin6_addr = addr;
			remotev6.sin6_port = htons (port);			
		}		
	}
	else 
	{
		remotev6.sin6_family = AF_INET6;
		remotev6.sin6_addr = addr;
		remotev6.sin6_port = htons (port);
		filno = socket (PF_INET6, SOCK_STREAM, 0);
	}
	
	if (filno >= 0)
	{
		setsockopt (filno, SOL_SOCKET, SO_KEEPALIVE,
					(char *) &pram, sizeof (int));
		
		peer_addr = addr;
		peer_port = port;

		// If an address to bind is set.. first bind 
		if (localbindaddr)
		{
			if (localbindaddr.isv4())
			{
				localv4.sin_family = AF_INET;
				localv4.sin_port = 0;
				localv4.sin_addr = localbindaddr;
				::bind (filno, (struct sockaddr *)&localv4, sizeof(localv4));
			}
			else
			{
				localv6.sin6_family = AF_INET6;
				localv6.sin6_port = 0;
				localv6.sin6_addr = localbindaddr;
				::bind (filno, (struct sockaddr *)&localv6, sizeof(localv6));
			}
		}

		int r;
		
		if (usev4)
		{
			r = ::connect (filno, (const sockaddr *) &remotev4, sizeof(remotev4));
		}
		else
		{
			r = ::connect (filno, (const sockaddr *) &remotev6, sizeof(remotev6));
		}
			
		if (r != 0)
		{
			::close (filno);
			filno = -1;
			errcode = FERR_NOCONNECT;
			err.crop (0);
			err.printf (errortext::sock::connfail, strerror (errno));
			return false;
		}
	}
	else
	{
		errcode = FERR_NORSRC;
		err = errortext::sock::create;
		throw socketCreateException();
	}
	
	feof = false;
	ti_established = core.time.now ();
	if (codec)
	{
		size_t szleft;
		size_t szdone;
		size_t sz;
		
		if (! codec->setup())
		{
			errcode = FERR_CODEC;
			err.crop();
			err.printf (errortext::sock::codec, codec->error().str());
			::close (filno);
			filno = -1;
			feof = true;
			return false;
		}
		
		bool mayskip = false;
		
handshakes:
		string dt;
		codec->peekoutput (dt);
		szleft = dt.strlen();
		szdone = 0;
		
		//::printf ("handshakes szleft=%i mayskip=%i\n", szleft, mayskip?1:0);
		
		if ((! szleft) && (! mayskip)) return true;
		
		while (szleft > 0)
		{
			sz = ::write (filno, dt.str() + szdone, szleft);
			if (sz<=0)
			{
				::close (filno);
				filno = -1;
				feof = true;
				errcode = FERR_CODEC;
				err = errortext::sock::chandshake;
				return false;
			}
			szdone += sz;
			szleft -= sz;
		}
		if (mayskip || szdone)
		{
			mayskip = false;
			char buf[4096];
			if (szdone) codec->doneoutput (szdone);

			sz = ::read (filno, buf, 4095);
			if (sz>0)
			{
				if (! codec->addinput (buf, sz))
				{
					::close (filno);
					filno = -1;
					feof = true;
					errcode = FERR_CODEC;
					err = errortext::sock::chandshake;
					codec->reset();
					return false;
				}
				try
				{
					if (codec->fetchinput (buffer))
					{
						mayskip = true;
						goto handshakes;
					}
				}
				catch (...)
				{
					errcode = FERR_CODEC;
					err.crop (0);
					err.printf (errortext::sock::codec, codec->error().str());
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
bool tcpsocket::bindtoaddr (ipaddress address)
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
		err = errortext::sock::create;
		throw socketCreateException();
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
	peer_addr = ipaddress();
	peer_name = path;
	peer_port = 0;

	#ifdef HAVE_PASSCRED	
		setsockopt (filno, SOL_SOCKET, SO_PASSCRED, (char *) &pram,
					sizeof (int));
	#endif
	ti_established = core.time.now ();
	if (codec)
	{
		if (! codec->setup())
		{
			errcode = FERR_CODEC;
			err.crop();
			err.printf (errortext::sock::codec, codec->error().str());
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
	socklen_t len = sizeof (credp);
	
	if (getsockopt (filno, SOL_SOCKET, SO_PEERCRED, (char *) &credp, &len) == 0)
	{		
		peer_pid = credp.pid;
		peer_uid = credp.uid;
		peer_gid = credp.gid;
	}
	else
	{
		peer_pid = 0;
		peer_uid = 65534;
		peer_gid = 65534;
	}
#endif
}

// ========================================================================
// METHOD sendcredentials
// ========================================================================
void tcpsocket::sendcredentials (void)
{
#ifdef HAVE_PASSCRED
	// int foo;
	// 
	// write (filno, &foo, sizeof (foo));
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
	} while ((z==-1) && (errno==EINTR));
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
	} while ((z==-1)&&(errno==EINTR));
	
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
// METHOD tcpsocket::derive
// ========================================================================
void tcpsocket::derive (tcpsocket *s)
{
	buffer.flush();
	filno = s->filno;
	feof = s->feof;
	codec = s->codec;
	peer_pid = s->peer_pid;
	peer_uid = s->peer_uid;
	peer_gid = s->peer_gid;
	peer_addr = s->peer_addr;
	peer_name = s->peer_name;
	peer_port = s->peer_port;
	err = s->err;
	errcode = s->errcode;
	ti_established = s->ti_established;
	s->filno = -1;
	s->feof = true;
	s->codec = 0;
		
	buffer.copy (s->buffer);
	delete s;
}

void tcpsocket::derive (tcpsocket &s)
{
	buffer.flush();
	filno = s.filno;
	feof = s.feof;
	codec = s.codec;
	peer_pid = s.peer_pid;
	peer_uid = s.peer_uid;
	peer_gid = s.peer_gid;
	peer_addr = s.peer_addr;
	peer_name = s.peer_name;
	peer_port = s.peer_port;
	err = s.err;
	errcode = s.errcode;
	ti_established = s.ti_established;
	buffer.copy (s.buffer);
	s.codec = 0;
}

// ========================================================================
// BASE CONSTRUCTOR listenerbase
// ========================================================================
listenerbase::listenerbase (void)
{
}

// ========================================================================
// BASE DESTRUCTOR listenerbase
// ========================================================================
listenerbase::~listenerbase (void)
{
}

// ========================================================================
// METHOD listenerbase::accept
// ========================================================================
tcpsocket *listenerbase::accept (void)
{
	return NULL;
}

// ========================================================================
// METHOD listenerbase::tryaccept
// ========================================================================
tcpsocket *listenerbase::tryaccept (double t)
{
	return NULL;
}

// ========================================================================
// CONSTRUCTOR
// -----------
// Creator for a listener object for a listening tcp socket
// on the specified tcp port.
// ========================================================================
tcplistener::tcplistener (int port)
{
	listening = false;
	listento (port);
}

tcplistener::tcplistener (void)
{
	unprotected (sock) { sock = 0; }
	listening = false;
	tcpdomain = true;
	tcpdomainport = 0;
}

void tcplistener::listento (int port)
{
	listento (ipaddress(), port);
}

// ========================================================================
// METHOD ::listento
// -----------------
// Set up listening on a tcp port.
// ========================================================================
void tcplistener::listento (ipaddress addr, int port)
{
	exclusivesection (sock)
	{
		if (listening)
		{
			close (sock);
			listening = false;
		}
		
		bindaddress = addr;
		
		tcpdomain = true;
		tcpdomainport = port;
		
		struct sockaddr_in6 remotev6;
		struct sockaddr_in  remotev4;
		int					 pram = 1;
		bool usev4 = false;
	
		memset (&remotev4, 0, sizeof (remotev4));
		memset (&remotev6, 0, sizeof (remotev6));
		
		if (!addr)
		{
			remotev6.sin6_family = AF_INET6;
			remotev6.sin6_addr = in6addr_any;
			remotev6.sin6_port = htons (port);
				
			sock = socket (PF_INET6, SOCK_STREAM, 0);
			
			if (sock < 0 && errno == EAFNOSUPPORT)
			{
				remotev4.sin_family = AF_INET;
				remotev4.sin_addr.s_addr = INADDR_ANY;
				remotev4.sin_port = htons (port);
				
				sock = socket (PF_INET, SOCK_STREAM, 0);
				usev4 = true;
			}
		}
		else if (addr.isv4())
		{
			remotev4.sin_family = AF_INET;
			remotev4.sin_addr = addr;
			remotev4.sin_port = htons (port);
				
			sock = socket (PF_INET, SOCK_STREAM, 0);	
			usev4 = true;
		}
		else 
		{
			remotev6.sin6_family = AF_INET6;
			remotev6.sin6_addr = addr;
			remotev6.sin6_port = htons (port);
			sock = socket (PF_INET6, SOCK_STREAM, 0);	
		}
		
		if (sock < 0)
		{
			::printf ("sock <0\n");
			throw socketCreateException();
		}
		
		setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, (char *) &pram,
					sizeof (int));
					
	#ifdef SO_REUSEPORT
		setsockopt (sock, SOL_SOCKET, SO_REUSEPORT, (char *) &pram,
					sizeof (int));
	#endif
	
		
		if (usev4)
		{
			if (bind (sock, (struct sockaddr *) &remotev4, sizeof (remotev4)) < 0)
			{
				close (sock);
				throw socketCreateException();
			}
		}
		else
		{
			if (bind (sock, (struct sockaddr *) &remotev6, sizeof (remotev6)) < 0)
			{
				close (sock);
				throw socketCreateException();
			}
		}
		
		if (listen (sock, tune::tcplistener::backlog) < 0)
		{
			close (sock);
			throw socketCreateException();
		}

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
	listening = false;
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
		
		sock = socket (PF_UNIX, SOCK_STREAM, 0);
		if (sock < 0)
		{
			throw socketCreateException();
		}
		
		setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, (char *) &pram,
					sizeof (int));
		
		if (bind (sock, (struct sockaddr *) &remote, sizeof (remote)) < 0)
		{
			close (sock);
			throw socketCreateException();
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
	struct sockaddr_in6	remote;
	struct sockaddr_in6	peer;
	socklen_t anint=1;
	int pram=1;
	
	int s;
	unprotected (sock)
	{
		s = ::accept (sock, (struct sockaddr *) &remote, &anint);
	}
	
	if (s<0)
	{
		throw socketCreateException();
	}
	
	if (tcpdomain)
	{
		anint = sizeof (peer);
		getpeername (s, (struct sockaddr *) &peer, &anint);
	}

	setsockopt (s, SOL_SOCKET, SO_KEEPALIVE, (char *) &pram,
				sizeof (int));	
	
	tcpsocket *myfil = new tcpsocket;
	(*myfil).openread (s);
	if (tcpdomain)
	{
		(*myfil).peer_addr = ipaddress(peer.sin6_addr);
		(*myfil).peer_port = ntohs (peer.sin6_port);
		(*myfil).peer_name = ipaddress::ip2str ((*myfil).peer_addr);
	}
	(*myfil).ti_established = core.time.now();
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
	int pram=1;
	fd_set fds;
	int s = -1;
	struct timeval tv;
	
	if (timeout < 0.0) return NULL;
	
	tv.tv_sec = (int) timeout;
	tv.tv_usec = (int) (10000.0 * (timeout - ((double) tv.tv_sec)));

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
				struct sockaddr_in6	remote;
				socklen_t remote_len = sizeof(remote);
				s = ::accept (sock, (struct sockaddr *) &remote, &remote_len);
			}
		}
	}
	
	if (s<0)
	{
		return NULL;
	}
	
	struct sockaddr_storage	peer;
	
	socklen_t anint = sizeof (peer);
	getpeername (s, (struct sockaddr *) &peer, &anint);

	setsockopt (s, SOL_SOCKET, SO_KEEPALIVE, (char *) &pram, sizeof (int));
	
	tcpsocket *myfil = new tcpsocket;
	myfil->openread (s);
	
	if (peer.ss_family == AF_INET)
	{
		sockaddr_in* peer_in = (sockaddr_in*)&peer;
		myfil->peer_addr = ipaddress (peer_in->sin_addr);
		myfil->peer_port = ntohs (peer_in->sin_port);
	}
	else if (peer.ss_family == AF_INET6)
	{
		sockaddr_in6* peer_in6 = (sockaddr_in6*)&peer;
		myfil->peer_addr = ipaddress (peer_in6->sin6_addr);
		myfil->peer_port = ntohs (peer_in6->sin6_port);
	}
	
	(*myfil).peer_name = ipaddress::ip2str ((*myfil).peer_addr);
	(*myfil).ti_established = core.time.now();
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
	if (!codec)
	{
		file fi;
		off_t off = 0;
	
		fi.openread (path);
		::sendfile (filno, fi.filno, &off, amount);
		fi.close();
	}
#endif
	
	file fi;
	string buf;
	
	fi.openread (path);
	
	try {
		
		
		while (! fi.eof() && amount > 0)
		{
			unsigned int sz = amount > 4096 ? 4096 : amount;
			buf = fi.read (sz);
			if (buf.strlen()) puts (buf);
			
			amount -= buf.strlen();
		}
	}
	catch (...)
	{
	}
	fi.close ();
}
