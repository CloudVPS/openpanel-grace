// ========================================================================
// daemon.h: Application subclass that can be used to implement a daemon
//           that can spawn into the background and leave log messages.
//
// Copyright (C) 2005 Pim van Riezen <pi@madscience.nl>
//					  Madscience Labs, Rotterdam.
// ========================================================================

#ifndef _DAEMON_H
#define _DAEMON_H 1

#include <grace/application.h>
#include <grace/thread.h>
#include <grace/lock.h>
#include <grace/system.h>

/// Background log message dispatcher.
/// A thread object that is part of the daemon class, routing log messages
/// to a file or syslog daemon.
class logthread : public thread
{
friend class daemon;
public:
					 /// Constructor.
					 /// \param d The daemon object to attach to
					 logthread (class daemon *d) : thread ("logthread")
					 {
					 	app = d;
					 	padding = 8;
					 }
					 /// Implementation.
	virtual void	 run (void);
					 /// Shuts down the thread.
	void			 shutdown (void);
	
					 /// Set the width of the module part of a log line.
	void			 setmodulewidth (int i) { padding = i; }
	
protected:
	class daemon	*app; ///< Link to the parent daemon object.
	conditional		 shutdownCondition; ///< Shutdown trigger.
	conditional		 startupCondition; ///< Startup trigger.
	int				 padding;
};

/// Namespace for log-related constants.
namespace log
{
	/// Priority levels.
	/// Multiple priorities can be combined with a logical OR.
	enum priority {
		debug = 0x01, ///< debugging messages
		info = 0x02, ///< informational messages
		warning = 0x04, ///< warning messages
		error = 0x08, ///< error messages
		critical = 0x10, ///< critical failure messages
		alert = 0x20, ///< system alert messages 
		emergency = 0x40, ///< imminent emergency messages
		application = 0x80, ///< application-specific messages
		allerror=0xf8, ///< all levels of error and above
		allwarning = 0xfc, ///< all levels of warning and above
		allinfo = 0xfe, ///< all levels of info and above
		all = 0xff ///< all levels
	};
	
	/// Type of logging.
	enum logtype {
		file = 0x1, ///< Log to a system file
		syslog = 0x2 ///< Log to a syslog socket
	};
};

/// Bookkeeping for a generic logging target.
/// Used internally to define a target for log messages.
struct logtarget
{
	logtarget		*next; ///< Linked list pointer.
	unsigned char	 priorities; ///< Priority bitmask.
	log::logtype	 type; ///< Type indicator
	string			 target; ///< Target filename (for log::file)
	file			 f; ///< File object (for log::file)
	unsigned int	 maxsize; ///< Max size of logfile (for log::file)
};

/// Daemon class. Acts like an application except that it will spawn
/// in the background. A log facility is added in that allows the
/// daemon to write log messages to a file or to syslog. The targets
/// are described in the application resource file (rsrc:resources.xml)
/// as children of the ["log"] node. A typical layout:
///
/// \verbinclude daemon_doc_resources.xml
///
/// If you want to define your logging somewhere other than the
/// resources.xml file, you should manually add the target(s)
/// using daemon::addlogtarget() before daemonize() is called.
///
/// Log messages sent before the system is daemonized are sent to
/// the stderr stream and queued up for later logging. The reasoning
/// behind this is that it's not possible to start threads before
/// the fork() calls that relate to daemonization have been executed.
class daemon : public application
{
friend class logthread;
public:
					 /// Constructor.
					 /// \param title The application-id.
					 daemon (const char *title)
					 		: application (title)
					 {
					 	_foreground = false;
					 	_log = NULL;
					 	_logtargets = NULL;
					 	daemonized = false;
					 	tuid = teuid = 0;
					 	tgid = tegid = 0;
					 }
					 
					 /// Destructor.
	virtual			~daemon (void);
	
					 /// Check for another running instance.
					 /// Looks for a pid-file and sends a null-signal to
					 /// the process to see if it is still running.
					 /// \return Result, \b true if there is no other instance.
	bool			 checkpid (void);
	
	void			 writepid (void); ///< Write daemon's pidfile.
	virtual int		 main (void); ///< Virtual implementation method.
	
	void			 daemonize (void); ///< Spawn to background.
	
					 /// Send log message to the logthread.
					 /// \param prio The message priority.
					 /// \param moduleName A short description of the
					 ///                   affected application subsystem.
					 /// \param fmt Printf-style formatted arguments.
	void			 log (log::priority prio, const string &moduleName,
						  const char *fmt, ...);
					 
					 /// Manually add a log target.
					 /// \param type The logtype.
					 /// \param target The target string (for log::file).
					 /// \param priorities Bitmask of which priorities to route.
					 /// \param maxsz Maximum size of the logfile (for log;:file).
	void			 addlogtarget (log::logtype type,
								   const string &target,
								   unsigned char priorities = 0xff,
								   unsigned int maxsz = 0);
					 /// Do not spawn to background.
					 /// This method flags the daemonize() method not to do
					 /// the actual fork()s.
	void			 setforeground (void);
	
	bool			 settargetuser (const string &uname)
					 {
					 	value pw = kernel.userdb.getpwnam (uname);
					 	if (! pw) return false;
					 	tuid = pw["uid"].uval();
					 	tgid = pw["gid"].uval();
					 	return true;
					 }
	
					 /// Set the desired width of the 'module' part of
					 /// a log line in log::file logs.
	void			 setlogmodulewidth (int i)
					 {
					 	if (_log) _log->setmodulewidth (i);
					 }
								   
	logtarget		*_logtargets; ///< Linked list of log targets
	
					 /// Set the real and effective userid that
					 /// will be active when we daemonize.
					 /// \param uid The real and effective uid.
	void			 settargetuid (uid_t uid) { tuid = teuid = uid; }
	
					 /// Set the real and effective userid that
					 /// will be active when we daemonize.
					 /// \param ruid The real uid.
					 /// \param euid The effective uid.
	void			 settargetuid (uid_t ruid, uid_t euid)
					 {
					 	tuid = ruid;
					 	teuid = euid;
					 }
					 
					 /// Set the real and effective groupid that
					 /// will be active when we daemonize.
					 /// \param gid The real and effective gid.
	void			 settargetgid  (gid_t gid) { tgid = tegid = gid; }

					 /// Set the real and effective groupid that
					 /// will be active when we daemonize.
					 /// \param rgid The real gid.
					 /// \param egid The effective gid.
	void			 settargetgid (gid_t rgid, gid_t egid)
					 {
					 	tgid = rgid;
					 	tegid = egid;
					 }

protected:
					 /// Shut down logthread;
	void			 stoplog (void) { if (_log) _log->shutdown(); }
	bool			 _foreground; ///< If true, daemonize will not fork.
	class logthread	*_log; ///< The logthread.
	bool			 daemonized; ///< True if daemonize() was called.
	uid_t			 tuid; ///< Target real userid after daemonize().
	uid_t			 teuid; ///< Target effective userid after daemonize().
	gid_t			 tgid; ///< Target real groupid after daemonize().
	gid_t			 tegid; ///< Target effective groupid after daemonize().
};

#endif
