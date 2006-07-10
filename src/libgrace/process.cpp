#include <grace/process.h>
#include <grace/system.h>

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
	::printf ("eek, unoverloaded process::main()\n");
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
		kernel.proc.kill (_pid, sig);
		running();
	}
}
