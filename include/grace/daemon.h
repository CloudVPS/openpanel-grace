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
	
	/// Send a log-event to the logthread.
	/// \param prio The log priority.
	/// \param mod The module for this log entry.
	/// \param text The log text to send.
	void write (priority prio, const string &mod, const string &text);
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

extern logthread *LOGTHREAD;
extern logtarget *LOGTARGETS;
extern class daemon *MAINDAEMON;

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
public:
					 /// Constructor.
					 /// \param title The application-id.
					 daemon (const string &title);

					 /// Destructor.
	virtual			~daemon (void);
	
					 /// Check for another running instance.
					 /// Looks for a pid-file and sends a null-signal to
					 /// the process to see if it is still running.
					 /// \return Result, \b true if there is no other instance.
	bool			 checkpid (void);
	
					 /// Write the daemon's pidfile to
					 /// /var/run/application-id.pid. This call is
					 /// normally handdled by daemonize(). For daemons
					 /// with a target uid, the pidfile is opened
					 /// and written to by the invoking user (usually
					 /// root at that point).
	void			 writepid (void);
	virtual int		 main (void); ///< Virtual implementation method.
	
					 /// Spawn to the background.
					 /// Unless if setforeground() was called, this will
					 /// fork the process to the background, detaching
					 /// from the terminal. The exit of the parent process
					 /// can optionally be delayed until a later point,
					 /// when the spawned process is done with its
					 /// initialization and validation (if it needs to
					 /// run threads to do this, it has to fork early or
					 /// the threads will break). In such delayed cases,
					 /// the parent process will keep a pipe open on the
					 /// standard output channel of the child and wait
					 /// for a line of input. If this input does not
					 /// contain the literal string "OK", the parent
					 /// process assumes initialization went awry and will
					 /// exit itself with a status of 1. Otherwise it
					 /// will use exit(0).
					 /// \param delayedexit Flag for the delayed exit.
	void			 daemonize (bool delayedexit=false);

					 /// Background initialization success report.
					 /// If the process was daemonized with delayedexit
					 /// set to true, this method should be called once
					 /// all background initialization has proceeded
					 /// succesfully.
	void			 delayedexitok (void);
					 
					 /// Background initialization failure report.
					 /// If the process was daemonized with delayedexit
					 /// set to true, this method should be called if
					 /// initialization failed for some reason. Expects
					 /// printf-formatted arguments or a single string.
	void			 delayedexiterror (const char *parm, ...);
	void			 delayedexiterror (const string &text);
	
					 /// Send log message to the logthread. Deprecated
					 /// in favor of global log::write.
					 /// \param prio The message priority.
					 /// \param moduleName A short description of the
					 ///                   affected application subsystem.
					 /// \param fmt Printf-style formatted arguments.
	void			 log (log::priority prio, const string &moduleName,
						  const char *fmt, ...);
						  
					 /// Send log message to the logthread.
					 /// Variation that accepts one string to accommodate
					 /// %format. Deprecated in favor of using
					 /// global log::write.
					 /// \param prio The message priority.
					 /// \param moduleName A short description of the
					 ///                   affected application subsystem.
					 /// \param text The log text.
	void			 log (log::priority prio, const string &moduleName,
						  const string &text);
					 
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
	
					 /// Call this to disable checking of the pid-file
					 /// during daemonize().
	void			 disablepidcheck (void) { pidcheck = false; }
	
					 /// Set a target user/group for the spawned process,
					 /// as specified by a unix account. Will use the
					 /// specified uid and primary gid for the account.
					 /// \param uname The unix username.
	bool			 settargetuser (const string &uname);
	
					 /// Set the desired width of the 'module' part of
					 /// a log line in log::file logs.
	void			 setlogmodulewidth (int i)
					 {
					 	if (LOGTHREAD) LOGTHREAD->setmodulewidth (i);
					 }
								   
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
					 
					 /// Set the additional target groups.
					 /// \param grouplist An array of group names.
	void			 settargetgroups (const value &grouplist);

					 /// Send an empty event to the main thread, with
					 /// only the type() set.
					 /// \param type The event type.
	void			 sendevent (const string &type);
	
					 /// Send an event to the main thread.
					 /// \param type The event type.
					 /// \param v Event parameter data.
	void			 sendevent (const statstring &type, const value &v);
	
					 /// Indefinitely wait for a new event.
					 /// \return A new event object.
	value			*waitevent (void);
	
					 /// Wait a given amount of time for an event.
					 /// \param msec Timeout in milliseconds.
					 /// \return A new event object or NULL.
	value			*waitevent (int msec);
	
					 /// Static handler for SIGTERM. Sends a
					 /// 'shutdown' event.
	static void		 termhandler (int sig);
	
					 /// Static handler for SIGHUP. Sends a
					 /// 'reconfigure' event.
	static void		 huphandler (int sig);

protected:
					 /// Shut down logthread;
	void			 stoplog (void) { if (LOGTHREAD) LOGTHREAD->shutdown(); }
	bool			 _foreground; ///< If true, daemonize will not fork.
	bool			 daemonized; ///< True if daemonize() was called.
	uid_t			 tuid; ///< Target real userid after daemonize().
	uid_t			 teuid; ///< Target effective userid after daemonize().
	gid_t			 tgid; ///< Target real groupid after daemonize().
	gid_t			 tegid; ///< Target effective groupid after daemonize().
	int				 tgroupcount; ///< Number of extra groups.
	gid_t			*tgroups; ///< Array of extra groups.
	bool			 pidcheck; ///< False if no pidcheck should be performed.
	eventq			 events; ///< Inbound event socket.
};

#endif
