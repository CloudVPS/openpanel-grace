// ========================================================================
// file.cpp: GRACE file class
//
// (C) Copyright 2003-2004 Pim van Riezen <pi@madscience.nl>
//                         Madscience Labs, Rotterdam 
// ========================================================================

#include <grace/file.h>
#include <grace/filesystem.h>
#include <grace/defaults.h>

// Tainted UNIX includes ;)
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>

const char __HEXTAB[] = "0123456789abcdef";

// ========================================================================
// CONSTRUCTOR
// -----------
// Creates an unbound file object
// ========================================================================

file::file (void)
	: buffer (2*65536)
{
	filno = -1;
	feof = true;
	nonblocking = false;
	codec = NULL;
	errcode = FERR_OK;
}

// ========================================================================
// DESTRUCTOR
// ----------
// Closes the file socket if there was one open
// ========================================================================

file::~file (void)
{
	if (filno>=0) ::close (filno);
	if (codec)
	{
		codec->refcnt--;
		if (! codec->refcnt) delete codec;
	}
}

// ========================================================================
// METHOD ::eof
// ------------
// Returns true if the end-of-file marker has been reached
// ========================================================================

bool file::eof (void)
{
	return (feof);
}

// ========================================================================
// METHOD ::openread
// -----------------
// Opens a filesystem object for reading. The path argument understands
// GRACE paths.
// ========================================================================

bool file::openread (const string &path)
{
	buffer.flush();
	string res;
	
	res = fs.transr (path);
	
	if (! res.strlen()) return false;
	
	int fid = open (res.str(), O_RDONLY);
	if (fid<0) return false;
	filno = fid;
	feof = false;
	
	if (codec)
	{
		if (!codec->setup())
			return false;
	}
	
	return true;
}

// ========================================================================
// METHOD ::openread
// -----------------
// Binds the object to an operating system filedescriptor. This is used
// to initialize the fin object in the application class.
// ========================================================================

bool file::openread (int fid)
{
	buffer.flush();
	filno = fid;
	feof = false;
	
	if (codec)
	{
		if (!codec->setup())
		{
			return false;
		}
	}
	return true;
}

// ========================================================================
// METHOD ::openwrite
// ------------------
// Opens a filesystem object for writing. The path argument understands
// GRACE paths.
// ========================================================================

bool file::openwrite (const string &path, int mode)
{
	buffer.flush();
	string res;
	
	res = fs.transw (path);
	if (! res.strlen()) return false;
	
	int fid = open (res.str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t) mode);
	if (fid<0) return false;
	filno = fid;
	feof = false;
	
	if (codec)
	{
		if (! codec->setup())
		{
			::close (filno);
			feof = true;
			return false;
		}
		string outbuf;
		int szdone;
		codec->peekoutput (outbuf);
		if (outbuf.strlen())
		{
			szdone = write (filno, outbuf.str(), outbuf.strlen());
			if ( (szdone<=0) && (errno != EAGAIN) )
			{
				::close (filno);
				feof = true;
				return false;
			}
		}
	}
	
	return true;
}

bool file::openappend (const string &path, int mode)
{
	buffer.flush();
	string res;
	
	res = fs.transw (path);
	if (! res.strlen()) return false;
	
	int fid = open (res.str(), O_RDWR | O_APPEND | O_CREAT, (mode_t) mode);
	if (fid<0) return false;
	filno = fid;
	feof = false;
	return true;
}

off_t file::pos (void)
{
	if (filno<0) return 0;
	off_t res = lseek (filno, 0, SEEK_CUR);
	return res;
}

// ========================================================================
// METHOD ::openwrite
// ------------------
// Binds an operating system filedescriptor for writing. Used for the fout
// and ferr objects.
// ========================================================================

bool file::openwrite (int fid)
{
	buffer.flush();
	filno = fid;
	feof = false;
	if (codec)
	{
		if (! codec->setup())
		{
			::close (filno);
			feof = true;
			return false;
		}
		string outbuf;
		int szdone;
		codec->peekoutput (outbuf);
		szdone = write (filno, outbuf.str(), outbuf.strlen());
		if ( (szdone<=0) && (errno != EAGAIN) )
		{
			::close (filno);
			feof = true;
			return false;
		}
	}
	return true;
}

// ========================================================================
// METHOD ::close
// --------------
// Releases access to any open operating system filesystem objects.
// ========================================================================

void file::close (void)
{
	if (codec)
	{
		string dat;
		codec->addclose();
		codec->peekoutput (dat);
		write (filno, dat.str(), dat.strlen());
	}
	if (filno>=0) ::close (filno);
	filno = -1;
	feof = true;
}

bool file::writeln (const string &str)
{
	if (puts (str.str(), str.strlen()))
	{
		return puts ("\n");
	}
	return false;
}

// ========================================================================
// METHOD ::puts
// -------------
// Writes a literal string to the file.
// ========================================================================

bool file::puts (const string &str)
{
	return puts (str.str(), str.strlen());
}

int file::tryputs (const char *str, size_t sz, unsigned int timeout_ms)
{
	if (feof) return -1;
	if (filno<0) return -1;

	int szdone = 0;
	
	fd_set fds;
	struct timeval tv;
			
	tv.tv_sec = timeout_ms/1000;
	tv.tv_usec = (timeout_ms % 1000) * 1000;
	
	FD_ZERO(&fds);
	FD_SET(filno,&fds);

	if (! nonblocking)
	{
		int opts;
		opts = fcntl(filno,F_GETFL);
		fcntl(filno,F_SETFL, opts | (O_NONBLOCK));
		nonblocking = true;
	}

	if (codec)
	{
		string outbuf;
		codec->peekoutput (outbuf);
		szdone = write (filno, outbuf.str(), outbuf.strlen());
		if ( (szdone<=0) && (errno == EAGAIN) )
		{
			if (select (filno+1, NULL, &fds, NULL, &tv) > 0)
			{
				szdone = write (filno, outbuf.str(), outbuf.strlen());
			}
		}
		if ((szdone < 0) && (errno == EAGAIN)) szdone = 0;
		if (szdone>0) codec->doneoutput ((unsigned int) szdone);
		
		if (szdone < 0) return szdone;
		
		if (codec->canoutput (sz))
		{
			if (codec->addoutput (str, sz))
			{
				szdone = sz;
			}
			else szdone = 0;
		}
		else szdone = 0;

		if (szdone>0)
		{
			codec->peekoutput (outbuf);
			szdone = write (filno, outbuf.str(), outbuf.strlen());
			if ( (szdone<=0) && (errno == EAGAIN) ) szdone = 0;
		}
	}
	else
	{
		szdone = write (filno, str, sz);
		
		if ( (szdone<=0) && (errno == EAGAIN) )
		{
			if (select (filno+1, NULL, &fds, NULL, &tv) > 0)
			{
				szdone = write (filno, str, sz);
			}
		}
		if ((szdone < 0) && (errno == EAGAIN)) szdone = 0;
	}

	return szdone;
}

bool file::puts (const char *str, size_t sz)
{
	if (feof) return false;
	if (filno<0) return false;
	
	size_t szleft = sz;
	size_t szdone = 0;

	if (nonblocking)
	{
		int opts;
		opts = fcntl(filno,F_GETFL);
		if (opts>=0)
		{
			fcntl(filno,F_SETFL, opts & (!O_NONBLOCK));
			nonblocking = false;
		}
	}
	
	if (codec)
	{
		//::printf ("codec called\n");
		if (codec->addoutput (str, sz))
		{
			string dat;
			codec->peekoutput(dat);
			szleft = dat.strlen();
			//::printf ("2.szleft=%i\n", szleft);
			while (szleft>0)
			{
				sz = write (filno, dat.str() + szdone, szleft);
				if (sz<=0)
				{
					feof = true;
					return false;
				}
				szdone += sz;
				szleft -= sz;
			}
			codec->doneoutput (szdone);
			return true;
		}
		//::printf ("addoutput failed %s\n", codec->error().str());
		return false;
	}

	while (szleft)
	{
		int sz;
		sz = write (filno, str + szdone, szleft);
		if (sz<=0)
		{
			feof = true;
			return false;
		}
		szdone += sz;
		szleft -= sz;
	}

	return true;
}

// ========================================================================
// METHOD ::printf
// ---------------
// Writes a formatted string to the file.
// ========================================================================

bool file::printf (const char *fmtx, ...)
{
	va_list ap;
	string out;
	int sz;
	
	char copy[20]; // Temporary storage for a format argument
	char sprintf_out[100]; // Temporary storage to build a string 
	char *copy_p; // Iterator
	const char *fmt = fmtx; // Iterator
	string copy_s;
	
	va_start (ap, fmtx);
	
	while (*fmt)
	{
		// If there is no special format character, copy the current
		// character literally into the string
	
		if (*fmt != '%') out.strcat ((char) *fmt++);
		else
		{
			// A format character, oh joy
		
			++fmt;
			copy[0] = '%';
			for (copy_p = copy+1; copy_p < copy+19;)
			{
				switch ((*copy_p++ = *fmt++))
				{
					case 0:
						fmt--; goto CONTINUE;
					case '%':
						out.strcat ("%"); goto CONTINUE;
					
					case 'c':
						out.strcat ((char) va_arg(ap, int));
						goto CONTINUE;
					
					case 'L':
						*copy_p = 0;
						sprintf ((char *) sprintf_out,
								 "%lli",
								 va_arg(ap, long long));
						copy_p = sprintf_out;
						goto DUP;
					case 'U':
						*copy_p = 0;
						sprintf ((char *) sprintf_out,
								 "%llu",
								 va_arg(ap, unsigned long long));
						copy_p = sprintf_out;
						goto DUP;
					case 'd':
					case 'i':
					case 'o':
					case 'u':
					case 'x':
					case 'X':
						*copy_p = 0;
						sprintf (sprintf_out, copy, va_arg(ap, int));
						copy_p = sprintf_out;
						goto DUP;
					case 'e':
					case 'E':
					case 'f':
					case 'g':
						*copy_p = 0;
						sprintf (sprintf_out, copy, va_arg(ap, double));
						copy_p = sprintf_out;
						goto DUP;
					case 'p':
						*copy_p = 0;
						sprintf (sprintf_out, copy, va_arg(ap, void*));
						copy_p = sprintf_out;
						goto DUP;
					
					// --------------------------------------------------------
					// %S for escaping strings that can safely appear within
					// a single or double quoted expression.
					// --------------------------------------------------------
					
					case 'S':
						copy_p = va_arg(ap, char *);
						if (!copy_p) copy_p = "(null)";
						
						while (*copy_p)
						{
							if ( (*copy_p == '%') || (*copy_p == '\\') ||
								 (*copy_p == '\'') || (*copy_p == '\"') )
							{
								out.strcat ('\\');
								out.strcat (*copy_p++);
							}
							else if (*copy_p < 32)
							{
								out.strcat ('%');
								out.strcat (__HEXTAB [(*copy_p >> 4) & 15]);
								out.strcat (__HEXTAB [(*copy_p++) & 15]);
							}
							else out.strcat (*copy_p++);
						}
						goto CONTINUE;
					
					case 's':
					    sz = atoi (copy+1);
						copy_p = va_arg(ap, char *);
						if (!copy_p) copy_p = "(null)";
						if (sz != 0)
						{
							int asz = sz;
							if (asz < 0) asz = -sz;
							
							copy_s = copy_p;
							
							if (copy_s.strlen() < asz)
							{
								string spc;
								spc = "                                                                                               ";
								spc.crop (asz - copy_s.strlen());
								if (sz < 0)
								{
									copy_s = spc;
									copy_s += copy_p;
								}
								else copy_s += spc;
							}
							else
								copy_s.crop (sz);
							
							out.strcat (copy_s);
							copy_p = "";
						}
DUP:
						out.strcat (copy_p);
						goto CONTINUE;
				}
			}
		}
CONTINUE:;
	}
	
	va_end (ap);
	return puts(out);
}

// ========================================================================
// METHOD ::gets
// -------------
// Reads a string from the file up to a newline.
// ========================================================================

void __file_breakme (void)
{
}

string *file::gets (int maxlinesize)
{
	if (feof)
	{
		errcode = FERR_EOF;
		err = "Called file::gets() while at end-of-file";
		throw (EX_FILE_EOF);
	}
	if (filno<0)
	{
		errcode = FERR_NOTOPEN;
		err = "Called file::gets() while file was not open";
		throw (EX_FILE_NOTOPEN);
	}

	int nlpos;
	
	while (buffer.room() && (! buffer.hasline()))
	{
		char buf[DEFAULT_SZ_FILE_READBUF];
		size_t sz;
		size_t wanted;
		
		wanted = DEFAULT_SZ_FILE_READBUF;
		if (wanted > buffer.room()) wanted = buffer.room();

		int opts;
		opts = fcntl(filno,F_GETFL);
		if (opts & O_NONBLOCK)
		{
			fcntl(filno,F_SETFL, opts & (!O_NONBLOCK));
			nonblocking = false;
		}
		
		sz = ::read (filno, buf, wanted-1);
		
		if (sz>0)
		{
			if (codec)
			{
				if (! codec->addinput (buf, sz))
				{
					errcode = FERR_CODEC;
					err.crop(0);
					err.printf ("Codec error: %s", codec->error().str());
					throw (EX_FILE_CODEC);
				}
				try
				{
					codec->fetchinput (buffer);
				}
				catch (...)
				{
					errcode = FERR_CODEC;
					err.crop(0);
					err.printf ("Codec exception while fetching data");
					throw (EX_FILE_CODEC);
				}
			}
			else
			{
				try
				{
					buffer.add (buf, sz);
				}
				catch (...)
				{
					__file_breakme();
					err.crop(0);
					err.printf ("Exception in ringbuffer (sz=%i am=%i)",
								  sz, wanted);
					throw (EX_FILE_ERR_READ);
				}
				if (sz == 365) __file_breakme();
			}
		}
		else
		{
			feof = true;
			err = "Broken pipe";
			unsigned int p;
			if (buffer.hasline (p)) err.printf (" hasline %i", p);
			else err.printf (" noline");
			__file_breakme ();
			return NULL;
		}
	}
	
	unsigned int eolpos;
	if (buffer.hasline(eolpos))
	{
		if (eolpos >= maxlinesize)
		{
			return buffer.read (maxlinesize);
		}
		return buffer.readline();
	}
	return buffer.read (maxlinesize);
}

// ========================================================================
// METHOD ::waitforline
// --------------------
// Waits for a designated times expecting a line of text terminated by
// a newline. Returns true if at least one complete line has been
// found in the buffer (which will also be copied into the string
// argument).
// ========================================================================

bool file::waitforline (string &into, int timeout_ms, int maxlinesize)
{
	if (feof)
	{
		errcode = FERR_EOF;
		err= "Called file::waitforline() while at end-of-file";
		throw (EX_FILE_EOF);
	}
	if (filno<0)
	{
		errcode = FERR_NOTOPEN;
		err = "Called file::waitforline() while file not open";
		throw (EX_FILE_NOTOPEN);
	}
	
	unsigned int room = buffer.room();

	int nlpos;
	
	if (room && (! buffer.hasline()))
	{
		if (! nonblocking)
		{
			int opts;
			opts = fcntl(filno,F_GETFL);
				fcntl(filno,F_SETFL, opts | (O_NONBLOCK));
				nonblocking = true;
		}
		
		fd_set fds;
		struct timeval tv;
		char buf[8192];
		int ssz;
		int opt;
		
		tv.tv_sec = timeout_ms/1000;
		tv.tv_usec = (timeout_ms % 1000) * 1000;
		
		FD_ZERO(&fds);
		FD_SET(filno,&fds);
		
		ssz = ::read (filno, buf, (room<8192) ? room : 8192);
		if (ssz > 0)
		{
			try
			{
				if (codec)
				{
					if (codec->addinput (buf, ssz))
					{
						codec->fetchinput (buffer);
					}
				}
				else
				{
					buffer.add (buf, ssz);
				}
			}
			catch (...)
			{
				errcode = FERR_BUFFER;
				err.crop (0);
				err.printf ("Buffer exception (ssz=%i "
							"am=%i)", ssz, (room<8192) ? room : 8192);
				throw (EX_FILE_ERR_READ);
			}
		}
		else
		{
			if (select (filno+1, &fds, NULL, NULL, &tv) > 0)
			{
				ssz = ::read (filno, buf, (room<8192) ? room : 8192);
				if (ssz > 0)
				{
					try
					{
						if (codec)
						{
							if (codec->addinput (buf, ssz))
							{
								codec->fetchinput (buffer);
							}
						}
						else
						{
							buffer.add (buf, ssz);
						}
					}
					catch (...)
					{
						errcode = FERR_BUFFER;
						err.crop (0);
						err.printf ("Buffer exception (ssz=%i "
									"am=%i)", ssz, (room<8192) ? room : 8192);
						throw (EX_FILE_ERR_READ);
					}
				}
			}
			else return false;
		}
	}

	unsigned int eolpos;

	if (buffer.hasline(eolpos))
	{
		if (! eolpos)
		{
			into.crop(0);
			return true;
		}
		if (eolpos >= maxlinesize)
		{
			into = buffer.read (maxlinesize);
		}
		else
		{
			into = buffer.readline();
		}
		return true;
	}
	
	return false;
}

bool file::readuntil (string &into, const char *watchfor, unsigned int sz, int timeout_ms)
{
	unsigned int endposition;
	if (buffer.findforward (watchfor, sz, endposition))
	{
		into.strcat (buffer.read (endposition));
		return true;
	}
	if (buffer.backlog() < 65536)
	{
		readbuffer (65536, timeout_ms);
	}

	if (buffer.findforward (watchfor, sz, endposition))
	{
		into.strcat (buffer.read (endposition));
		return true;
	}
	
	if (buffer.backlog() > (sz+2))
		into.strcat (buffer.read (buffer.backlog() - (sz+3)));
	return false;
}

int file::readbuffer (size_t sz, unsigned int timeout_ms)
{
	if (feof)
	{
		errcode = FERR_EOF;
		err = "Called file::readbuffer() while at end-of-file";
		throw (EX_FILE_EOF);
	}
	if (filno<0)
	{
		errcode = FERR_NOTOPEN;
		err = "Called file::readbuffer() while file not open";
		throw (EX_FILE_NOTOPEN);
	}
	
	if (buffer.room() < 8) return 0;


	unsigned int rsz = sz;
	if (rsz > buffer.room()) rsz = buffer.room();
	
	if (! nonblocking)
	{
		int opts;
		opts = fcntl(filno,F_GETFL);
			fcntl(filno,F_SETFL, opts | (O_NONBLOCK));
			nonblocking = true;
	}
	
	fd_set fds;
	struct timeval tv;
	char buf[65536];
	
	tv.tv_sec = timeout_ms/1000;
	tv.tv_usec = (timeout_ms % 1000) * 1000;
	
	FD_ZERO(&fds);
	FD_SET(filno,&fds);
	
	int ssz;
	ssz = ::read (filno, buf, (rsz<65536) ? rsz : 65536);
	
	if ((ssz==0) && (errno==EAGAIN) && ((!timeout_ms) || 
	                 (select (filno+1, &fds, NULL, NULL, &tv) > 0)))
	{
		ssz = ::read (filno, buf, (rsz<65536) ? rsz : 65536);
	}
	
	if (ssz > 0)
	{
		try
		{
			if (codec)
			{
				if (codec->addinput (buf, ssz))
				{
					codec->fetchinput (buffer);
				}
			}
			else
			{
				buffer.add (buf, ssz);
			}
		}
		catch (...)
		{
			errcode = FERR_BUFFER;
			err.crop (0);
			err.printf ("Buffer exception (ssz=%i "
						"am=%i)", ssz, (rsz<65536) ? rsz : 65536);
			throw (EX_FILE_ERR_READ);
		}
		return ssz;
	}
	else if (ssz < 0)
	{
		if (errno != EAGAIN) feof = true;
	}
	return 0;
}

// ========================================================================
// METHOD ::read
// --------------
// Read an indicated amount of bytes from the file.
// ========================================================================

string *file::read (size_t sz)
{
	if (feof && (! buffer.backlog()))
	{
		errcode = FERR_EOF;
		err = "Called file::read() while at end-of-file with "
			  "no backlog";
		throw (EX_FILE_EOF);
	}
	if (filno<0)
	{
		errcode = FERR_NOTOPEN;
		err = "Called file::read() while file not open";
		throw (EX_FILE_NOTOPEN);
	}
	
	int ssz;

	if (nonblocking)
	{
		int opts;
		opts = fcntl(filno,F_GETFL);
		if (opts>=0)
		{
			fcntl(filno,F_SETFL, opts & (!O_NONBLOCK));
			nonblocking = false;
		}
	}

	char buf[DEFAULT_SZ_FILE_READBUF];
	while ( (! feof) && (buffer.backlog() < sz) )
	{
		unsigned int am = DEFAULT_SZ_FILE_READBUF;
		if (am > buffer.room()) am = buffer.room();

		ssz = ::read (filno, buf, am);
		if (ssz > 0)
		{
			try
			{
				if (codec)
				{
					if (codec->addinput (buf, ssz))
					{
						codec->fetchinput (buffer);
					}
				}
				else
				{
					buffer.add (buf, ssz);
				}
			}
			catch (...)
			{
				errcode = FERR_IO;
				err.crop(0);
				err.printf ("Buffer exception (ssz=%i am=%i)",sz,am);
				throw (EX_FILE_ERR_READ);
			}
		}
		else
		{
			feof = true;
			break;
		}
	}
	
	return buffer.read(sz);
}

string *file::read (size_t sz, int timeout_ms)
{
	if (feof && (! buffer.backlog()))
	{
		errcode = FERR_EOF;
		err = "Called file::read() while at end-of-file with no backlog";
		throw (EX_FILE_EOF);
	}

	if (buffer.backlog() >= sz)
	{
		return buffer.read (sz);
	}

	if (! nonblocking)
	{
		int opts;
		opts = fcntl(filno,F_GETFL);
		fcntl(filno,F_SETFL, opts | O_NONBLOCK);
		nonblocking = true;
	}
	
	
	fd_set fds;
	struct timeval tv;
	char buf[DEFAULT_SZ_FILE_READBUF];
	int ssz;
	unsigned int am = DEFAULT_SZ_FILE_READBUF;
	if (am > buffer.room()) am = buffer.room();
	
	tv.tv_sec = timeout_ms/1000;
	tv.tv_usec = (timeout_ms % 1000) * 1000;
	
	FD_ZERO(&fds);
	FD_SET(filno,&fds);
	
	if (codec)
	{
		string obuf;
		int szdone;
		
		codec->peekoutput (obuf);
		if (obuf.strlen())
		{
			szdone = write (filno, obuf.str(), obuf.strlen());
			if (szdone>0) codec->doneoutput ((unsigned int) szdone);
			else if (szdone < 0)
			{
				errcode = FERR_IO;
				err.crop(0);
				err.printf ("Error flushing codec data",sz,am);
				throw (EX_FILE_ERR_READ);
			}
		}
	}
	
	ssz = ::read (filno, buf, am);
	if ( (ssz<=0) && (errno == EAGAIN) ) 
	{
		if (nonblocking)
		{
			int opts;
			opts = fcntl(filno,F_GETFL);
			if (opts>=0)
			{
				fcntl(filno,F_SETFL, opts & (!O_NONBLOCK));
				nonblocking = false;
			}
		}
				  
		if ( (timeout_ms) &&
	         (select (filno+1, &fds, NULL, NULL, &tv) <= 0) )
	    {
	    	errcode = FERR_TIMEOUT;
	    	err = "Read timeout (select)";
	    	return NULL;
	    }

		if (! nonblocking)
		{
			int opts;
			opts = fcntl(filno,F_GETFL);
			fcntl(filno,F_SETFL, opts | O_NONBLOCK);
			nonblocking = true;
		}

		ssz = ::read (filno, buf, am);
		if (ssz <= 0)
		{
			errcode = FERR_TIMEOUT;
			err = "Read timeout (read)";
			return NULL;
		}
		
	}
	
	if (ssz > 0)
	{
		try
		{
			if (codec)
			{
				if (codec->addinput (buf, ssz))
				{
					codec->fetchinput (buffer);
				}
			}
			else
			{
				buffer.add (buf, ssz);
			}
		}
		catch (...)
		{
			errcode = FERR_BUFFER;
			err.crop(0);
			err.printf ("Buffer exception (ssz=%i am=%i)",sz,am);
			throw (EX_FILE_ERR_READ);
		}
		if (! buffer.backlog())
		{
			return read (sz, timeout_ms);
		}
	}
	
	return buffer.read (sz);
}

iocodec::iocodec (void)
{
	refcnt = 1;
}

iocodec::~iocodec (void)
{
}

bool iocodec::setup (void)
{
	return false;
}

void iocodec::reset (void)
{
}

bool iocodec::addinput (const char *dt, size_t sz)
{
	return false;
	
}

bool iocodec::addoutput (const char *dt, size_t sz)
{
	return false;
}

void iocodec::addclose (void)
{
}

void iocodec::fetchinput (ringbuffer &into)
{
}

void iocodec::peekoutput (string &into)
{
}

void iocodec::doneoutput (unsigned int sz)
{
}

bool iocodec::canoutput (unsigned int sz)
{
}

const string &iocodec::error (void)
{
	return err;
}
