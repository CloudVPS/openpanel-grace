#ifndef _SYSTEM_H
#define _SYSTEM_H 1

#include <sys/types.h>
#include <sys/errno.h>
#include <signal.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

//#include <crypt.h>
#include <grace/value.h>
#include <grace/timestamp.h>

//#include <linux/unistd.h>

#ifdef __SUPPORT_JAIL
#ifndef __NR_jail
#define __NR_jail	254
#endif

_syscall2(long,jail,int,value,int,data);

#define JAIL_CPU	1
#define JAIL_IPV4	2
#define JAIL_VM		3

#endif

extern const char *SALTSRC;
	
char *__grace_internal_crypt (const char *, const char *);

/// Some OS functions.
/// This class is accessed through a global instance named 'kernel'.
class systemclass
{
public:
	systemclass (void)
	{
		pwcrypt.randseeded = false;
	}

	enum systemException {
		exNoSuchUser
	};

	/// Process-related functions.	
	class
	{
	public:
		/// Check if a pid is running.
		/// \param p The pid.
		bool exists (pid_t p)
		{
			if (::kill (p,0)) return false;
			return true;
		}
		
		/// Send a signal.
		bool kill (pid_t p, int sig)
		{
			if (::kill (p, sig)) return false;
			return true;
		}
		
		/// Get our own pid.
		pid_t self (void)
		{
			return ::getpid();
		}
	} proc;
	
	/// User-related functionality.
	class
	{
	public:
		/// Get current userid.
		uid_t getuid (void)
		{
			return ::getuid();
		}
		
		/// Get current group id.
		gid_t getgid (void)
		{
			return ::getgid();
		}
		
		/// Look up a user account by its name.
		/// The return data contains the following keys:
		///   - username: The user name.
		///   - passwd: The crypted password (if available).
		///   - uid: The native user id.
		///   - gid: The native group id.
		///   - gecos: The real name/GECOS data.
		///   - home: The user's home directory.
		///   - shell: The user's login shell.
		value *getpwnam (const string &s)
		{
			struct passwd *p;
			
			p = ::getpwnam (s.str());
			if (! p) throw (exNoSuchUser);
			
			return pwval (p);
		}
		
		/// Look up a user account by its uid.
		/// The return data contains the following keys:
		///   - username: The user name.
		///   - passwd: The crypted password (if available).
		///   - uid: The native user id.
		///   - gid: The native group id.
		///   - gecos: The real name/GECOS data.
		///   - home: The user's home directory.
		///   - shell: The user's login shell.
		value *getpwuid (uid_t uid)
		{
			struct passwd *p;
			
			p = ::getpwuid (uid);
			if (! p) throw (exNoSuchUser);
			
			return pwval (p);
		}
		
	protected:
		/// Convert passwd structure to a value object.
		value *pwval (struct passwd *p)
		{
			value *r = new value;
			(*r)["username"] = p->pw_name;
			(*r)["passwd"] = p->pw_passwd;
			(*r)["uid"] = (int) p->pw_uid;
			(*r)["gid"] = (int) p->pw_gid;
			(*r)["gecos"] = p->pw_gecos;
			(*r)["home"] = p->pw_dir;
			(*r)["shell"] = p->pw_shell;
			
			return r;
		}
	} userdb;
	
	/// Password crypting functionality.
	class
	{
	public:
		/// Create a MD5 hash string for a password.
		string *md5 (const string &pw)
		{
			char mysalt[16];
			int i;
			string *res;
			
			mkseed ();
			
			res = new string;
			mysalt[0] = '$';
			mysalt[1] = '1';
			mysalt[2] = '$';
			
			for (i=0;i<8;++i)
			{
				mysalt[i+3] = SALTSRC[rand() & 63];
			}
			mysalt[i+3] = 0;
			
			(*res) = __grace_internal_crypt (pw.str(), mysalt);
			return res;
		}
		
		/// Create a DES hash string for a password.
		string *des (const string &pw)
		{
			char mysalt[16];
			string *res;
			int i;
			
			mkseed ();
			
			res = new string;
			
			for (i=0;i<2;++i)
			{
				mysalt[i] = SALTSRC[rand() & 63];
			}
			mysalt[i] = 0;
			
			(*res) = __grace_internal_crypt (pw.str(), mysalt);
			return res;
		}
		
		/// Verify a DES/MD5 hash against a plaintext password.
		bool verify (const string &pw, const string &hash)
		{
			string ts;
			
			ts = __grace_internal_crypt (pw.str(), hash.str());
			if (ts == hash) return true;
			return false;
		}
		
		bool randseeded; ///< True if the RNG was seeded.
		
	protected:
		void mkseed (void)
		{
			if (! randseeded)
			{
				srand (::time(NULL) ^ ::getpid());
				randseeded = true;
			}
		}
	} pwcrypt;
	
	/// Self-limiting functionality.
	class
	{
	public:
		/// Perform a system chroot.
		bool chroot (const string &pat)
		{
			if (::chroot (pat.str())) return false;
			return true;
		}
#ifdef __SUPPORT_JAIL
		bool cpu (int perc)
		{
			if (::jail (JAIL_CPU, perc)) return false;
			return true;
		}
		bool vm (int mb)
		{
			if (::jail (JAIL_VM, mb * (1024 * 1024))) return false;
			return true;
		}
		bool ipv4 (const string &addr)
		{
			if (::jail (JAIL_IPV4, inet_addr (addr.str()))) return false;
			return true;
		}
#endif
	} limits;

	class
	{
	public:
		/// Get current system time.
		time_t now (void)
		{
			return ::time (NULL);
		}
		/// get the current time in 
		/// micro seconds
		timeval unow (void)
		{
			struct timeval 	Tval;  ///< Return time
			
			::gettimeofday( &Tval, NULL );			
			
			return Tval;
		}
		/// Sleep until the given time
		/// \param timestamp
		void sleepuntil (const timestamp &dest)
		{
				// Remaining time
				timestamp	TRemain;
				timestamp	Tnow;
					
				// remaining usecs
				unsigned 
				long long 	usecRemain;
							
				while( true )
				{
					Tnow = unow();
					
					if( Tnow >= dest )
						return;
						
					TRemain 	= dest - Tnow;
					usecRemain 	= TRemain.getusec();
					
					// Sleep 10s or The last miliseconds
					if( usecRemain > 10000000LL ) {
						__musleep( 10000LL ); 
					} else {
						// sleep remaining time..
						// this is always less then 10 seconds
						__musleep( TRemain.getusec() / 1000LL );
						return;
					};
				}
		}
	} time;
	
	/// Network related functions.
	class
	{
	public:
		/// Get our local hostname.
		const string &hostname (void)
		{
			static string hn;
			
			if (hn.strlen() == 0)
			{
				char buf[256];
				buf[0] = 0;
				buf[255] = 0;
				::gethostname (buf, 255);
				hn = buf;
			}
			
			return hn;
		}
	} net;
	
	/// Execute a shell command.
	int sh (const string &cmd)
	{
		return ::system (cmd.str());
	}
	
};

extern systemclass kernel;

#endif
