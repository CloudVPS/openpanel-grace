// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

// ========================================================================
// daemon.cpp: System daemon abstraction class with background forking.
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
logthread *LOGTHREAD = NULL;
logtarget *LOGTARGETS = NULL;
class daemon *MAINDAEMON = NULL;

// Statically kept statstrings to be used as common keys in log events.
namespace logproperty
{
	statstring priority ("priority");
	statstring module ("module");
	statstring command ("command");
	statstring text ("text");
};

void log::write (log::priority prio, const string &mod, const string &text)
{
	MAINDAEMON->log (prio, mod, text);
}

// ========================================================================
// CONSTRUCTOR daemon
// ========================================================================
daemon::daemon (const string &title) : application (title)
{
	for (int i=3; i<255; i++) ::close (i);
	MAINDAEMON = this;
	_foreground = false;
	LOGTHREAD = NULL;
	LOGTARGETS = NULL;
	daemonized = false;
	tuid = teuid = 0;
	tgid = tegid = 0;
	pidcheck = true;
	tgroupcount = 0;
	tgroups = NULL;
}

// ========================================================================
// DESTRUCTOR daemon
// ========================================================================
daemon::~daemon (void)
{
	string path;
	string empty;
	
	path = "run:%s.pid" %format (creator);
	path = fs.transr (path);
	fs.save (path, empty);
}

// ========================================================================
// METHOD ::daemonize
// ------------------
// Spawns the current process into the background like a daemon, using
// a double fork().
// ========================================================================
void daemon::daemonize (bool delayedexit)
{
	if (pidcheck && (! checkpid ()))
	{
		ferr.printf (errortext::daemon::running, creator.str());
		exit (1);
	}
	
	signal (SIGTERM, daemon::termhandler);
	signal (SIGHUP, daemon::huphandler);
	
	// If the foreground flag is set, we travel a simpler path.
	if (_foreground)
	{
		daemonized = true;
		
		// Write the pid-file
		if (pidcheck) writepid ();
		
		// Set user-/groupids.
		if (tgroupcount) setgroups (tgroupcount, tgroups);
		if (tgid) setregid (tgid, tegid);
		if (tuid) setreuid (tuid, teuid);
		return;
	}
	
	int backpipe[2]; // Comms channel for delayedexit.
	int i; // Counter

	if (delayedexit)
	{
		int result = pipe (backpipe);
		
		if (result == -1)
		{
			ferr.printf ("Daemonize: Couldn't create pipe");
			exit (1);
		}
	}
	
	switch (fork())
	{
		case 0:
			// Detach from stdin/-out/-err.
			for (i=0; i<3; ++i) ::close(i);
			
			if (delayedexit)
			{
				// Attach the comms pipe to stdout.
				open ("/dev/null", O_RDONLY);
				dup2 (backpipe[1], 1);
				open ("/dev/null", O_WRONLY);
				//for (i=3;i<16;++i) ::close (i);
				fout.openread (1);
			}
			else
			{
				// Just attach to /dev/null.
				open ("/dev/null", O_RDONLY);
				open ("/dev/null", O_WRONLY);
				open ("/dev/null", O_WRONLY);
			}
			
			// Extra fork round, to detach from parent.
			switch (fork())
			{
				case 0:
					// We're set. Do the business.
					daemonized = true;
					if (pidcheck) writepid ();
					if (tgroupcount) setgroups (tgroupcount, tgroups);
					if (tgid) setregid (tgid, tegid);
					if (tuid) setreuid (tuid, teuid);
					return;
				
				case -1:
					exit (1);
			}
			exit (0); // Exit the parent.
			break;
		
		case -1:
			ferr.printf (errortext::daemon::nofork);
			exit (1);
	}
	
	// We're in parent country now, if delayedexit was set we will be
	// waiting for data on the comms channel triggered by either
	// delayedexitok() or delayedexiterror().
	if (delayedexit)
	{
		string res;
		file freturn;
		
		freturn.openread (backpipe[0]);
		try
		{
			res = freturn.gets();
			if (res.strcasecmp ("ok") == 0)
			{
				// Everything fine, exit silently.
				exit (0);
			}
			
			// Write the error as it was passed.
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

// ========================================================================
// METHOD ::delayedexitok
// ========================================================================
void daemon::delayedexitok (void)
{
	fout.writeln ("OK");
	fout.close ();
}

// ========================================================================
// METHOD ::delayedexiterror
// ========================================================================
void daemon::delayedexiterror (const char *fmtx, ...)
{
	va_list ap;
	string out;
	
	va_start (ap, fmtx);
	out.printf_va (fmtx, &ap);
	va_end (ap);
	
	fout.writeln (out);
	fout.close ();
}

void daemon::delayedexiterror (const string &text)
{
	fout.writeln (text);
	fout.close ();
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
	
	path = "run:%s.pid" %format (creator);
	tpath = fs.transr (path);
	if (! tpath.strlen()) return true;

	// The pid-file exists?
	if (fs.exists (tpath))
	{
		// Load the pid-file into a string and parse the pid.
		line = fs.load (tpath, filesystem::optional);
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
	
	pid = core.proc.self ();
	path = "run:%s.pid" %format (creator);

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
	string logText;
	va_list ap;
	
	va_start (ap, fmt);
	logText.printf_va (fmt, &ap);
	va_end (ap);
	
	log (prio, modulename, logText);
}

void daemon::log (log::priority prio, const string &modulename,
				  const string &logText)
{
	// If we weren't threaded, we are now.
	__THREADED = true;
	static lock<bool> logmutex;
	static value backlog;
	static bool dq = false;
	
	if ((! daemonized) || (_foreground && (prio == log::critical)))
	{
		ferr.writeln ("%s: %s" %format (modulename, logText));
		
		if (! daemonized)
		{
			backlog.newval() = $(logproperty::module, modulename) ->
							   $(logproperty::text, logText) ->
							   $(logproperty::priority, prio);
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
		prioNames = $("debug", log::debug) ->
					$("info", log::info) ->
					$("warning", log::warning) ->
					$("error", log::error) ->
					$("critical", log::critical) ->
					$("alert", log::alert) ->
					$("emergency", log::emergency) ->
					$("appliation", log::application) ->
					$("allinfo", 255 - log::debug) ->
					$("allerror", log::error | log::critical |
								  log::alert | log::emergency) ->
					$("all", 255);
	}

	// If no logtargets were manually defined, get the desired
	// information from the application's resources.xml	
	
	if (! LOGTARGETS)
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
	
	if (! LOGTHREAD)
	{
		LOGTHREAD = new logthread (this);
		LOGTHREAD->start();
		LOGTHREAD->startupCondition.wait();
	}
	
	logmutex.unlock();
	
	if (dq)
	{
		foreach (ev, backlog)
		{
			LOGTHREAD->sendevent ("logmessage", ev);
		}
		backlog.clear();
		dq = false;
	}
	
	LOGTHREAD->sendevent ("logmessage", $(logproperty::priority, prio) ->
				   						$(logproperty::module, modulename) ->
				   						$(logproperty::text, logText));
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
	if (LOGTARGETS)
	{
		tail = LOGTARGETS;
		while (tail->next) tail = tail->next;
		tail->next = mytg;
	}
	else LOGTARGETS = mytg;
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
// METHOD ::settargetuser
// ========================================================================
bool daemon::settargetuser (const string &uname)
{
	value pw = core.userdb.getpwnam (uname);
	if (! pw) return false;
	
	settargetuid (pw["uid"].uval());
	settargetgid (pw["gid"].uval());
	return true;
}

// ==========================================================================
// METHOD daemon::settargetgroups
// ==========================================================================
bool daemon::settargetgroups (const value &list)
{
	tgroupcount = list.count();
	tgroups = new gid_t[tgroupcount];
	int i=0;
	
	foreach (g, list)
	{
		value dt = core.userdb.getgrnam (g.sval());
		if (dt.isempty())
		{
			tgroupcount = 0;
			delete[] tgroups;
			tgroups = NULL;
			return false;
		}
		gid_t gid = dt["gid"].uval();
		tgroups[i++] = gid;
	}
	return true;
}

// ========================================================================
// METHOD ::sendevent
// ========================================================================
void daemon::sendevent (const string &type)
{
	statstring tp = type;
	events.send (tp);
}

void daemon::sendevent (const statstring &type, const value &data)
{
	events.send (type, data);
}

// ========================================================================
// METHOD ::waitevent
// ========================================================================
value *daemon::waitevent (void)
{
	return events.waitevent ();
}

value *daemon::waitevent (int timeout_ms)
{
	return events.waitevent (timeout_ms);
}

// ========================================================================
// STATIC METHOD ::termhandler
// ========================================================================
void daemon::termhandler (int sig)
{
	MAINDAEMON->sendevent ("shutdown");
}

// ========================================================================
// STATIC METHOD ::huphandler
// ========================================================================
void daemon::huphandler (int sig)
{
	MAINDAEMON->sendevent ("reconfigure");
}

// ========================================================================
// METHOD ::shutdown
// -----------------
// Sends a "die" event to the logthread, then waits for it to raise the
// shutdownCondition.
// ========================================================================
void logthread::shutdown (void)
{
	sendevent ("die");
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
		tf = LOGTARGETS;
		
		if (ev.type() == "die")
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
					int prio = ev[logproperty::priority];
					string prioName;
					string tstr;
					string modName;
					timestamp ti = core.time.now();
					
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
					tf->f.writeln ("%s %s [%s]: %s" %format (tstr, modName,
										prioName, ev[logproperty::text]));
					
					// rotate logfiles
					unsigned int msize;
					msize = tf->maxsize ? tf->maxsize : (defaults::sz::logfile);
					if (tf->f.pos() > msize)
					{
						tf->f.close();
						
						int ii = defaults::lim::logrotate - 1;
						
						for (; ii >= 0; --ii)
						{
							string oldfn, newfn;
							
							oldfn = tf->target;
							if (ii) oldfn.printf (".%i", ii-1);
							newfn = "%s.%i" %format (tf->target, ii);
							fs.mv (oldfn, newfn);
						}
						
						tf->f.openappend (tf->target);
					}
				}
			}
			tf = tf->next;
		}
	}
	
	tf = LOGTARGETS;
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

