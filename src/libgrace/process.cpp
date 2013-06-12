// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#include <grace/process.h>
#include <grace/system.h>
#include <grace/defaults.h>

// ========================================================================
// METHOD ::init
// ========================================================================
void process::init (const string &name, bool withStdErr)
{
	static lock<bool> forklock;
	int inpipe[2], outpipe[2];
	
	 _pid = -1;
	 _running = false;
	 _name = name;
	 
	 pipe (inpipe);
	 pipe (outpipe);
	 
	 try {STRINGREF().treelock.lockw();} catch (...) {}
	 _pid = fork();
	 if (_pid) try {STRINGREF().treelock.unlock();} catch (...) {}
	
	 if (_pid == 0)
	 {
		::close (0);
		::close (1);
		::close (2);
		__THREADED = false;
		
		(void) dup2 (outpipe[0], 0);
		(void) dup2 (inpipe[1], 1);
		if (! withStdErr) (void) open ("/dev/null",O_WRONLY);
		else (void) dup2 (inpipe[1], 2);
		
		fin.openread (0);
		fout.openwrite (1);
		ferr.openwrite (2);
		for (int fd=3; fd<256; ++fd)
			::close (fd);
		
		_running = true;
	 }
	 else if (_pid < 0)
	 {
		 _pid = -1;
	 }
	 else
	 {
		_inf.openread (inpipe[0]);
		_outf.openwrite (outpipe[1]);
		::close (inpipe[1]);
		::close (outpipe[0]);
		_running = true;
	 }
}

// ========================================================================
// METHOD ::running
// ----------------
// Returns true if the process is running
// ========================================================================
bool process::running (void)
{
	if (! _running) return false;
	
	int istat = 0;
	
	if (waitpid (_pid, &istat, WNOHANG))
	{
		if (WIFEXITED(istat))
			_retval = WEXITSTATUS(istat);
		else
			_retval = -1;
		
		_running = false;
		_pid = 0;
	}
	
	return _running;
}

// ========================================================================
// METHOD ::serialize
// ------------------
// Wait for the process to exit
// ========================================================================
void process::serialize (void)
{
	if (_running)
	{
		_retval = -1;
		int istat = 0;
		
		if (waitpid (_pid, &istat, 0))
		{
			_retval = WIFEXITED(istat) ? WEXITSTATUS(istat) : -1;
			_running = false;
			_pid = 0;
		}
	}
}

// ========================================================================
// METHOD ::main
// -------------
// Base method, should never be overloaded
// ========================================================================
int process::main (void)
{
	::printf (errortext::process::nomain);
	return 1;
}

// ========================================================================
// METHOD ::printf
// ---------------
// Send formatted text to the process' stdin
// ========================================================================
bool process::printf (const char *fmt, ...)
{
	string s;
	va_list ap;
	
	va_start (ap, fmt);
	s.printf_va (fmt, &ap);
	va_end (ap);
	
	return puts (s);
}

// ========================================================================
// METHOD ::kill
// -------------
// Terminate the process
// ========================================================================
void process::kill (int sig)
{
	if (running())
	{
		core.proc.kill (_pid, sig);
		running();
	}
}
