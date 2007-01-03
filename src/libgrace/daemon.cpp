// ========================================================================
// daemon.cpp: System daemon abstraction class with background forking.
//
// (C) Copyright 2004-2006 Pim van Riezen <pi@madscience.nl>
//                         Madscience Labs, Rotterdam 
// ========================================================================

#include "platform.h"
#include <grace/daemon.h>
#include <grace/system.h>
#include <grace/timestamp.h>
#include <grace/filesystem.h>
#include <grace/defaults.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <time.h>

lock<value> THREADLIST;

// Statically kept statstrings to be used as common keys in log events.
namespace logproperty
{
	statstring priority ("priority");
	statstring module ("module");
	statstring command ("command");
	statstring text ("text");
};

// ========================================================================
// METHOD ::daemonize
// ------------------
// Spawns the current process into the background like a daemon, using
// a double fork().
// ========================================================================
void daemon::daemonize (bool delayedexit)
{
	if (! checkpid ())
	{
		ferr.printf (errortext::daemon::running, creator.str());
		exit (1);
	}
	
	if (_foreground)
	{
		writepid ();
		if (tgid) setregid (tgid, tegid);
		if (tuid) setreuid (tuid, teuid);
		return;
	}
	
	int backpipe[2];
	int i;

	if (delayedexit) pipe (backpipe);
	
	switch (fork())
	{
		case 0:
			for (i=0; i<3; ++i) ::close(i);
			
			if (delayedexit)
			{
				open ("/dev/null", O_WRONLY);
				dup2 (backpipe[1], 1);
				open ("/dev/null", O_RDONLY);
				//for (i=3;i<16;++i) ::close (i);
				fout.openread (1);
			}
			
			switch (fork())
			{
				case 0:
					daemonized = true;
					writepid ();
					if (tgid) setregid (tgid, tegid);
					if (tuid) setreuid (tuid, teuid);
	
					return;
				
				case -1:
					exit (1);
			}
			break;
		
		case -1:
			ferr.printf (errortext::daemon::nofork);
			exit (1);
	}
	
	if (delayedexit)
	{
		string res;
		file freturn;
		
		freturn.openread (backpipe[0]);
		try
		{
			res = freturn.gets();
			if (res.strncmp ("ok") == 0)
			{
				exit (0);
			}
			ferr.writeln (res);
			exit (1);
		}
		catch (...)
		{
			exit (1);
		}
	}
	
	exit (0);
}

void daemon::delayedexiterror (const char *fmtx, ...)
{
	va_list ap;
	string out;
	
	va_start (ap, fmtx);
	out.printf_va (fmtx, &ap);
	va_end (ap);
	
	fout.writeln (fmtx);
	fout.close ();
}

// ========================================================================
// DESTRUCTOR daemon
// ========================================================================
daemon::~daemon (void)
{
	string path;
	string empty;
	path.printf ("run:%s.pid", creator.str());
	path = fs.transr (path);
	fs.save (path, empty);
}

// ========================================================================
// METHOD checkpid
// ---------------
// Investigates the pid-file in run: and sees if another copy is already
// running. 
// ========================================================================
bool daemon::checkpid (void)
{
	string path;
	string tpath;
	string line;
	pid_t  pid;
	char  *p;
	
	path.printf ("run:%s.pid", creator.str());
	tpath = fs.transr (path);
	if (! tpath.strlen()) return true;

	// The pid-file exists?
	if (fs.exists (tpath))
	{
		// Load the pid-file into a string and parse the pid.
		line = fs.load (tpath);
		pid = ::strtoul (line.str(), &p, 10);
		
		// If the pid is not 0, test if the pid exists by sending it
		// a NULL signal
		if (pid && (! kill (pid, 0)))
		{
			// The pid exists, report as such.
			return false;
		}
		
		// The pid no longer exists, empty the pid-file.
		line.crop();
		fs.save (tpath, line);
	}
	return true;
}

// ========================================================================
// METHOD ::writepid
// -----------------
// Writes a pid-file named after the application-id to the run: directory
// with the current pid inside.
// ========================================================================
void daemon::writepid (void)
{
	string path;
	string tpath;
	pid_t  pid;
	file   f;
	
	pid = kernel.proc.self ();
	
	path.printf ("run:%s.pid", creator.str());
	try
	{
		tpath = fs.transr (path);
		if (! tpath) tpath = fs.transw (path);
		if (! tpath.strlen())
		{
			ferr.printf (errortext::daemon::writepid);
			return;
		}
		if (!f.openwrite (tpath))
		{
			return;
		}
		
		f.printf ("%U", (unsigned long long) pid);
		f.close();
		
		if (teuid || tegid) fs.chown (tpath, teuid, tegid);
	}
	catch (...)
	{
	}
}

// ========================================================================
// METHOD main
// -----------
// Duh, this should be overridden in the inherited class.
// ========================================================================
int daemon::main (void)
{
	return 1;
}

// ========================================================================
// METHOD ::log
// ------------
// Sends a log event to the logthread, starts the thread if it was not
// running yet.
//
// Accepts a priority level, an identifying string for the program's
// module/subsystem affected and a formatted C-string with the desired
// log text.
// ========================================================================
void daemon::log (log::priority prio, const string &modulename,
				  const char *fmt, ...)
{
	// If we weren't threaded, we are now.
	__THREADED = true;
	static lock<bool> logmutex;
	static value backlog;
	static bool dq = false;
	
	string logText;
	va_list ap;
	
	va_start (ap, fmt);
	logText.printf_va (fmt, &ap);
	va_end (ap);
	
	if ((! daemonized) || (_foreground && (prio == log::critical)))
	{
		ferr.printf ("%s: %s\n", modulename.str(), logText.str());
		
		if (! daemonized)
		{
			backlog.newval();
			backlog[-1][logproperty::module] = modulename;
			backlog[-1][logproperty::text] = logText;
			backlog[-1][logproperty::priority] = prio;
			dq = true;
			return;
		}
	}
	
	logmutex.lockw();
	
	// Keep a cache of conversion from text-based priorities
	// to log::priority values.
	static value prioNames;
	
	// Fill the cache if it is empty
	if (! prioNames.exists ("debug"))
	{
		prioNames["debug"] = log::debug;
		prioNames["info"]  = log::info;
		prioNames["warning"] = log::warning;
		prioNames["error"] = log::error;
		prioNames["critical"] = log::critical;
		prioNames["alert"] = log::alert;
		prioNames["emergency"] = log::emergency;
		prioNames["application"] = log::application;
		prioNames["allinfo"] = 255 - log::debug;
		prioNames["allerror"] = log::error | log::critical
								 | log::alert | log::emergency;
		prioNames["all"]= 255;
	}

	// If no logtargets were manually defined, get the desired
	// information from the application's resources.xml	
	
	if (! _logtargets)
	{
		foreach (logdef, rsrc["log"])
		{
			log::logtype ltype;
			log::priority prio = (log::priority) 0;
			string target;
			
			target = logdef["target"];
			
			if (logdef["type"] == "syslog")
				ltype = log::syslog;
			else
				ltype = log::file;
			
			unsigned int maxsz = 0;
			if (logdef.exists ("maxsize"))
			{
				maxsz = logdef["maxsize"];
			}
			
			// create a bitmask of all the desired priorities to be
			// handled by this logtarget.
			
			foreach (reqprio, logdef["priorities"])
			{
				int myprio = (prioNames[reqprio.sval()].ival());
				myprio |= (int) prio;
				prio = ((log::priority) myprio);
			}
			
			addlogtarget (ltype, target, prio, maxsz);
		}
	}
	
	if (! _log)
	{
		_log = new logthread (this);
		_log->start();
		_log->startupCondition.wait();
	}
	
	logmutex.unlock();
	
	if (dq)
	{
		foreach (ev, backlog)
		{
			_log->sendevent (ev);
		}
		backlog.clear();
		dq = false;
	}
	
	value  logEv;
	
	logEv[logproperty::priority] = prio;
	logEv[logproperty::module] = modulename;
	logEv[logproperty::text] = logText;
	
	_log->sendevent (logEv);
}

// ========================================================================
// METHOD ::addlogtarget
// ---------------------
// Adds a new target for log messages, with a given type, optional
// target name and a bitmask of priorities to be handled.
// ========================================================================
void daemon::addlogtarget (log::logtype type, const string &target,
						   unsigned char priorities,
						   unsigned int maxsize)
{
	logtarget *mytg = new logtarget;
	logtarget *tail;
	
	mytg->next = NULL;
	mytg->priorities = priorities;
	mytg->type = type;
	mytg->target = target;
	mytg->maxsize = maxsize;
	if (mytg->type == log::file)
	{
		mytg->f.openappend (mytg->target);
	}
	if (_logtargets)
	{
		tail = _logtargets;
		while (tail->next) tail = tail->next;
		tail->next = mytg;
	}
	else _logtargets = mytg;
}

// ========================================================================
// METHOD ::setforeground
// ----------------------
// If invoked, daemon will not spawn into the background when
// daemonize() is invoked.
// ========================================================================
void daemon::setforeground (void)
{
	_foreground = true;
}

// ========================================================================
// METHOD ::shutdown
// -----------------
// Sends a "die" event to the logthread, then waits for it to raise the
// shutdownCondition.
// ========================================================================
void logthread::shutdown (void)
{
	value death;
	death[logproperty::command] = "die";
	sendevent (death);
	shutdownCondition.wait();
}

// ========================================================================
// METHOD ::run
// ------------
// Implements the logthread. Waits for an event, then goes over all
// configured log targets to see if they match the priority mask.
// ========================================================================
void logthread::run (void)
{
	value ev;
	logtarget *tf;
	bool syslogopen = false;
	
	startupCondition.broadcast();

	while (true)
	{
		ev = waitevent();
		tf = app->_logtargets;
		
		if (ev.exists (logproperty::command) &&
			(ev[logproperty::command] == "die"))
		{
			break;
		}
		
		// Loop over all logtargets
		while (tf)
		{
			// Match the priority mask
			if (ev[logproperty::priority].ival() & tf->priorities)
			{
				// Is this a syslog target?
				if (tf->type == log::syslog) // yes
				{
					if (! syslogopen)
					{
						openlog (app->creator.str(), 0, LOG_DAEMON);
						syslogopen = true;
					}
					
					syslog (ev[logproperty::priority].ival(), "%s: %s",
							ev[logproperty::module].cval(),
							ev[logproperty::text].cval());
				}
				else // no, file target
				{
					int prio= ev[logproperty::priority];
					string prioName;
					string tstr;
					string modName;
					timestamp ti = kernel.time.now();
					
					modName = ev[logproperty::module].sval();
					modName.pad (padding, ' ');
					
					#define MPRIO(pname) \
						else if (prio & log::pname) \
							prioName = names::logprio::pname
						
					if (false) {}
					MPRIO (emergency);
					MPRIO (alert);
					MPRIO (critical);
					MPRIO (error);
					MPRIO (warning);
					MPRIO (info);
					MPRIO (application);
					else
					{
						prioName = names::logprio::debug;
					}
					
					#undef MPRIO
					
					tstr = ti.format ("%b %e %H:%M:%S");
					
					tf->f.printf ("%s %s [%s]: %s\n",
								  tstr.str(),
								  modName.str(),
								  prioName.str(),
								  ev[logproperty::text].cval());
					
					// rotate logfiles
					unsigned int msize;
					msize = tf->maxsize ? tf->maxsize : (defaults::sz::logfile);
					if (tf->f.pos() > msize)
					{
						tf->f.close();
						
						int ii = defaults::lim::logrotate - 1;
						
						for (ii ; ii >= 0; --ii)
						{
							string oldfn, newfn;
							
							oldfn = tf->target;
							if (ii) oldfn.printf (".%i", ii-1);
							newfn.printf ("%s.%i", tf->target.str(), ii);
							fs.mv (oldfn, newfn);
						}
						
						tf->f.openappend (tf->target);
					}
				}
			}
			tf = tf->next;
		}
	}
	
	tf = app->_logtargets;
	while (tf)
	{
		if (tf->type == log::syslog)
		{
			if (syslogopen)
			{
				closelog ();
				syslogopen = false;
			}
		}
		else
		{
			tf->f.close();
		}
		
		tf = tf->next;
	}
	
	// Ok we're dying, raise the white flag.
	shutdownCondition.broadcast();
}
