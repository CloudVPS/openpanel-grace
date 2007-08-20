#ifndef _PROCESS_H
#define _PROCESS_H 1

#include <grace/str.h>
#include <grace/strutil.h>
#include <grace/value.h>
#include <grace/exception.h>
#include <grace/filesystem.h>
#include <grace/system.h>

#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define EX_NOEXEC 142

#define PROC_RUNMAIN if (_running && (! _pid)) exit (main())

/// Abstraction for a child process.
/// Classes derived from this class can fork() off a process and
/// communicate with its input and output channels.

class process
{
public:
					 /// Constructor. Will spawn the actual process,
					 /// so object creation should be followed by a
					 /// call to process::run().
					 /// \param name Process title.
					 /// \param withStdErr If true, stderr will be intercepted.
					 process (const string &name = "noname",
					 		  bool withStdErr = false)
					 {
					 	init (name, withStdErr);
					 }
					 
					 /// Constructor. Will spawn the actual process,
					 /// so object creation should be followed by a
					 /// call to process::run().
					 /// \param name Process title.
					 /// \param v Process arguments.
					 /// \param withStdErr If true, stderr will be intercepted.
					 process (const string &name, const value &v,
					 		  bool withStdErr)
					 {
					 	data = v;
					 	init (name, withStdErr);
					 }
					 
					 /// Constructor for an 'empty' process.
					 /// You need to call process::init() manually
					 /// before you can call process::run().
					 process (bool nostart)
					 {
					 }
	
					 /// Init the process structure and perform the
					 /// fork().
					 /// \param name The process title.
					 /// \param withStdErr The stderr flag.
	void			 init (const string &name, bool withStdErr)
					 {
						 _pid = -1;
						 _running = false;
						 _name = name;
						 
						 int inpipe[2], outpipe[2];
						 
						 pipe (inpipe);
						 pipe (outpipe);
						 
						 _pid = fork();
						 
						 if (_pid == 0)
						 {
						 	::close (0);
						 	::close (1);
						 	::close (2);
						 	dup2 (outpipe[0], 0);
						 	dup2 (inpipe[1], 1);
						 	if (! withStdErr) open ("/dev/null",O_WRONLY);
						 	else dup2 (inpipe[1], 2);
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
					 
					 /// Destructor.
					 /// Will kill the child process.
	virtual			~process (void)
					 {
						 if (running())
						 {
							 kill (SIGKILL);
						 }
					 }
	
					 /// Wait for the child process to finish.
	void			 serialize (void);
	
					 /// Find out process running status.
					 /// \return Status, \b true if the child process is running
	bool			 running (void);
	
					 /// Send a signal to the child process.
					 /// \param sig The signal to be sent.
	void			 kill (int sig);
	
					 /// This method must be called after creating your
					 /// process to 
	void			 run (void)
					 {
					 	if (!_pid)
					 	{
					 		exit (main());
					 	}
					 }

					 /// Get a line of text from the process' output stream.
					 /// \return New string object with the text line, minus
					 ///         its end-of-line markings.
	string			*gets (void)
					 {
				 		return _inf.gets();
					 }
					 
					 /// Send data to the process input stream.
					 /// \param s The data to be sent.
	bool			 puts (const string &s)
					 {
					 	return _outf.puts (s);
					 }
					 
					 /// Close output channel.
	void			 close (void)
					 {
					 	_outf.close();
					 }
					
					 
					 /// Send formatted data to the process input stream.
					 /// \return Status, \b true if operation succeeded.
	bool			 printf (const char *, ...);
	
					 /// Read a number of bytes from the process output.
					 /// \param sz The number of bytes to read.
					 /// \return New string object with read data.
	string			*read (size_t sz)
					 {
				 		return _inf.read (sz);
					 }
					 
					 /// End-of-file. 
					 /// \return Status, \b true if there's an EOF condition.
	bool			 eof (void)
					 {
					 	return _inf.eof();
					 }
	
					 /// Kill child. Will send a SIGTERM to the child process
					 /// if it is still running.
	inline void		 terminate (void)
					 {
						 if (running())
							 kill (SIGTERM);
					 }
					 
					 /// Virtual main method. Implements the background
					 /// process.
					 /// \return Process return-value.
	virtual int		 main (void);
	
					 /// Get child process-id.
					 /// \return The pid, or \b 0 if the process isn't running.
	inline pid_t	 pid (void)
					 {
						 return running() ? _pid : 0;
					 }
					 
					 /// Return value. The process must be finished, use
					 /// serialize() if necessary.
	inline int		 retval (void)
					 {
						 return _retval;
					 }

protected:
	pid_t			 _pid; ///< Process-id of the child.
	bool			 _running; ///< True if the child runs.
	int				 _retval; ///< Return value of the child process.
	string			 _name; ///< Process title.
	file			 _inf; ///< Input from process (pipe to child's stdout).
	file			 _outf; ///< Output to process (pipe to child's stdin).
	file			 fin; ///< Input channel for child process.
	file			 fout; ///< Output channel for child process.
	file			 ferr; ///< Error channel for child process.
	value			 data; ///< Argument data.
};

typedef char *argptr;

/// Run external commands.
/// Accepts a unix path/command with command line arguments and executes
/// the command as a child process.
class systemprocess : public process
{
public:
					 /// Constructor.
					 /// \param mcommand The command string
					 /// \param withStdErr True if the stderr channel should be caught.
					 systemprocess (const string &mcommand,
					 				bool withStdErr = false)
					 				: process (false)
					 {
					 	tuid = teuid = tgid = tegid = 0;
					 	_argv = strutil::splitquoted (mcommand, ' ');
					 	init (mcommand, withStdErr);
					 }
					 
					 /// Constructor.
					 /// \param args The command + arguments array.
					 /// \param withStdErr True if the stderr channel should be caught.
					 systemprocess (const value &args,
					 				bool withStdErr = false)
					 				: process (false)
					 {
					 	tuid = teuid = tgid = tegid = 0;
					 	_argv = args;
					 	init (args[0].sval(), withStdErr);
					 }	

					 /// Constructor.
					 /// \param args The command + arguments array.
					 /// \param env The environment variables.
					 /// \param withStdErr True if the stderr channel should be caught.
					 systemprocess (const value &args, const value &env,
					 				bool withStdErr = false)
					 				: process (false)
					 {
					 	tuid = teuid = tgid = tegid = 0;
					 	_argv = args;
					 	_env = env;
					 	init (args[0].sval(), withStdErr);
					 }				
					 
					 /// Destructor.
	virtual			~systemprocess (void)
					 {
					 }
					 
	void			 settargetuid (uid_t ruid)
					 {
					 	tuid=teuid = ruid;
					 }
	
	void			 settargetgid (gid_t rgid)
					 {
					 	tgid = tegid = rgid;
					 }
					 
	bool			 settargetuser (const string &uname)
					 {
					 	value pw = kernel.userdb.getpwnam (uname);
					 	if (! pw)
					 	{
					 		if (! _pid) exit (0);
					 		return false;
					 	}
					 	
					 	settargetuid (pw["uid"].uval());
					 	settargetgid (pw["gid"].uval());
					 }
					 
					 /// Child process.
					 /// Creates an argument list, resolves the path to the
					 /// file and uses plain old execv() to do the dirty
					 /// work.
					 /// \return Process return-value.
	virtual int		 main (void)
					 {
					 	argptr *myargv;
					 	string cpath;
					 	int i;
					 	
					 	myargv = new argptr[_argv.count()+1];
					 	for (i=0; i<_argv.count(); ++i)
					 	{
					 		myargv[i] = (char *) _argv[i].cval();
					 	}
					 	myargv[i] = NULL;
					 	
					 	foreach (var, _env)
					 	{
					 		::setenv (var.id().str(), var.cval(), 1);
					 	}
					 	
					 	cpath = fs.transr (myargv[0]);
					 	
					 	if (tgid) setregid (tgid, tegid);
					 	if (tuid) setreuid (tuid, teuid);
					 	
					 	execv (cpath.str(), myargv);
					 	fout.printf ("could not run\n");
					 	return EX_NOEXEC;
					 }

protected:
	value			 _argv; ///< Command line arguments used
	value			 _env; ///< The environment variables.
	uid_t			 tuid; ///< Target real userid for the child.
	uid_t			 teuid; ///< Target effective userid for the child.
	gid_t			 tgid; ///< Target real groupid for the child.
	gid_t			 tegid; ///< Target effective groupid for the child.
};

#endif
