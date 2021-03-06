// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#ifndef _FILE_H
#define _FILE_H 1

#include <grace/exception.h>
#include <grace/str.h>
#include <grace/exception.h>
#include <grace/ringbuffer.h>

/// Abstract utility class for encoding/decoding data streams
/// that pass a file object.
/// The file class can be told to use objects of a class derived
/// from iocodec to process input/output data that it streams. The
/// primary application for this is the implementation of openssl
/// hooks in grace-ssl, but there may be other possibilities. For
/// an example of a working implementation, look at either the
/// grace-ssl code or the iocodec test scenario.
class iocodec
{
public:
					 iocodec (void);
	virtual			~iocodec (void);
	
					 /// Perform setup on newly opened link/file.
	virtual bool	 setup (void);
	
					 /// Reset a previously configured codec link.
	virtual void	 reset (void);
	
					 /// Add new input data to be processed.
					 /// \param dt Data block.
					 /// \param sz Data size.
	virtual bool	 addinput (const char *dt, size_t sz);

					 /// Add new output data to be processed.
					 /// \param dt Data block.
					 /// \param sz Data size.
	virtual bool	 addoutput (const char *db, size_t sz);

					 /// Add codec-specific data to handle
					 /// a 'link will close' handshake, if needed.
	virtual void	 addclose (void);
	
					 /// Read translated input data.
					 /// \param into The ringbuffer.
	virtual bool	 fetchinput (ringbuffer &into);
	
					 /// Get a copy of the entire output buffer.
					 /// \param into The string object to dump the
					 ///             buffer data into.
	virtual void	 peekoutput (string &into);
	
					 /// Advance the cursor into the output buffer.
					 /// \param sz The number of bytes to skip.
	virtual void	 doneoutput (unsigned int sz);
	
					 /// Determine if there is enough room in the
					 /// output buffer.
					 /// \param sz The number of bytes to reserve.
	virtual bool	 canoutput (unsigned int sz);
	
					 /// Return error string.
	const string	&error (void);
	
					 /// Reference count.
	unsigned int	 refcnt;
	
					 /// Disable certificate checking.
	virtual void	 nocertcheck (void);
	
protected:
	string			 err; ///< Last generated error text.
};

#define FERR_OK			0
#define FERR_EOF		0x38a49aae
#define FERR_NOTOPEN	0x02dcdf87
#define FERR_CODEC		0x3a9a408e
#define FERR_BUFFER		0x6f9ed1c1
#define FERR_IO			0x37cb955b
#define FERR_TIMEOUT	0x7624460d
#define FERR_CONNECT2	0x2fb1760b
#define FERR_NOCONNECT	0x395e2cbb
#define FERR_NORSRC		0x2babc890

$exception (endOfFileException, "End of file");
$exception (fileNotOpenException, "File not open");
$exception (fileReadException, "Read error");
$exception (fileWriteException, "Write error");
$exception (fileCodecException, "Codec exception");

/// An input/output channel.
class file
{
public:
				 file (void);
				 
				 /// Constructor with explicit path. Opens a file
				 /// for reading (or writing)
				 /// \param inpath The disk file to open. If this
				 ///               starts with a '<', the file is
				 ///               opened for reading. If it starts
				 ///               with a '>', it is opened for
				 ///               writing (in both cases the first
				 ///               character is stripped). In other
				 ///               cases the file will be opened for
				 ///               reading.
				 file (const string &inpath)
				 {
					filno = -1;
					feof = true;
					nonblocking = false;
					codec = NULL;
					errcode = FERR_OK;
					if (inpath[0] == '<') openread (inpath.mid(1));
					else if (inpath[0] == '>') openwrite (inpath.mid(1));
					else openread (inpath);
				 }
				 
				 /// Destructor.
	virtual		~file (void);
				
				
				 /// Open a disk file for reading.
				 /// \param fn Name (and path) of the file.
				 /// \return Result, \b true if open succeeded.
	bool		 openread (const string &fn);
	
				 /// Open a unix filedecriptor for reading.
				 /// \param fd The filedescriptor.
				 /// \return Result, \b true if open succeeded.
	bool		 openread (int fd);
	
				 /// Open a disk file for writing.
				 /// \param fn Name (and path) of the file.
				 /// \param mode Unix permissio bits.
				 /// \return Result, \b true if open succeeded.
	bool		 openwrite (const string &fn, int mode=0644);

				 /// Open a unix filedescriptor for writing.
				 /// \param fd The filedescriptor.
				 /// \return Result, \b true if open succeeded.
	bool		 openwrite (int fd);
	
				 /// Open a disk file for appending.
				 /// \param fn Name (and path) of the file.
				 /// \param mode Unix permissio bits.
				 /// \return Result, \b true if open succeeded.
	bool		 openappend (const string &fn, int mode=0644);

				 /// Close a currently opened file.	
	void		 close (void);
	
				 /// Read a string terminated by a newline.
				 /// \param maxlinesize Set maximum size of a line.
				 /// \return New string object with data.
				 /// \throw endOfFileException End of file reached.
				 /// \throw fileNotOpenException File is not open.
				 /// \throw fileCodecException Codec failure.
				 /// \throw fileReadException File read error.
				 /// \throw EX_SSL_BUFFER_SNAFU Error in sslcodec buffer.
				 /// \throw EX_SSL_PROTOCOL_ERROR Error in sslcodec protocol.
				 /// \throw EX_SSL_CLIENT_ALERT Unhandled sslcodec client alert.
	string 		*gets (int maxlinesize=1024);
	
				 /// Wait for a string terminated by a newline.
				 /// \param into String to append the line to.
				 /// \param timeout_ms Timeout in milliseconds.
				 /// \param maxlinesize Maximum size of a line.
				 /// \return Status, \b false in case of a timeout condition.
				 /// \throw endOfFileException End of file reached.
				 /// \throw fileNotOpenException File not open.
				 /// \throw fileReadException File read error.
				 /// \throw EX_SSL_BUFFER_SNAFU Error in sslcodec buffer.
				 /// \throw EX_SSL_PROTOCOL_ERROR Error in sslcodec protocol.
				 /// \throw EX_SSL_CLIENT_ALERT Unhandled sslcodec client alert.
	bool		 waitforline (string &into, int timeout_ms,
							  int maxlinesize=1024);
							  
				 /// Keep reading into a string until a specific
				 /// byte sequence occurs.
				 /// \param into String to append data to.
				 /// \param watchfor Pointer to byte sequence.
				 /// \param size Size of byte sequence.
				 /// \param tmout Timeout in milliseconds.
				 /// \return Status, \b false in case of a timeout condition.
	bool		 readuntil (string &into, const string &watchfor,
							unsigned int size, int tmout);
	
				 /// Read data into the internal buffer.
				 /// \param sz Number of bytes to read.
				 /// \param tmout Timeout in milliseconds.
				 /// \return Number of bytes read.
				 /// \throw endOfFileException End of file reached.
				 /// \throw fileNotOpenException File not open.
				 /// \throw fileReadException File read error.
				 /// \throw EX_SSL_BUFFER_SNAFU Error in sslcodec buffer.
				 /// \throw EX_SSL_PROTOCOL_ERROR Error in sslcodec protocol.
				 /// \throw EX_SSL_CLIENT_ALERT Unhandled sslcodec client alert.
	int			 readbuffer (size_t, unsigned int tmout=0);
	
				 /// Blocking read. Returns a string object with the data.
				 /// \param sz Number of bytes to read.
				 /// \return New string object.
				 /// \throw endOfFileException End of file reached.
				 /// \throw fileNotOpenException File not open.
				 /// \throw fileReadException File read error.
				 /// \throw EX_SSL_BUFFER_SNAFU Error in sslcodec buffer.
				 /// \throw EX_SSL_PROTOCOL_ERROR Error in sslcodec protocol.
				 /// \throw EX_SSL_CLIENT_ALERT Unhandled sslcodec client alert.
	string		*read (size_t sz);
	
				 /// Nonblocking read. Returns a string object with the data.
				 /// \param sz Number of bytes to read.
				 /// \param timeout_ms Timeout in milliseconds.
				 /// \return New string object.
				 /// \throw endOfFileException End of file reached.
				 /// \throw fileNotOpenException File not open.
				 /// \throw fileReadException File read error.
				 /// \throw EX_SSL_BUFFER_SNAFU Error in sslcodec buffer.
				 /// \throw EX_SSL_PROTOCOL_ERROR Error in sslcodec protocol.
				 /// \throw EX_SSL_CLIENT_ALERT Unhandled sslcodec client alert.
	string		*read (size_t sz, int timeout_ms);
	
				 /// Write a string followed by a newline.
				 /// \param str The string to write.
				 /// \return Status, \b true for success.
	bool		 writeln (const string &str);
	
				 /// Write a data block.
				 /// \param s The data to write.
				 /// \return Status, \b true for success.
	bool		 puts (const string &s);
	
				 /// Write a data block.
				 /// \param data Pointer to the data.
				 /// \param sz Size of teh data block.
				 /// \return Status, \b true for success.
				 /// \throw EX_SSL_NO_HANDSHAKE No sslcodec handshake done.
				 /// \throw EX_SSL_BUFFER_SNAFU Error in sslcodec buffer.
	bool		 puts (const char *data, size_t sz);
	
				 /// Non-blocking puts.
				 /// \param str Pointer to the data.
				 /// \param sz Size of the data to be written.
				 /// \param tmout Timeout in milliseconds.
				 /// \return Number of bytes written.
				 /// \throw EX_SSL_NO_HANDSHAKE No sslcodec handshake done.
				 /// \throw EX_SSL_BUFFER_SNAFU Error in sslcodec buffer.
	int			 tryputs (const char *str, size_t sz, unsigned int tmout=0);
	
				 /// A libc-style printf.
				 /// \return Status, \b true for success.
				 /// \throw EX_SSL_NO_HANDSHAKE No sslcodec handshake done.
				 /// \throw EX_SSL_BUFFER_SNAFU Error in sslcodec buffer.
	bool		 printf (const char *, ...);
	
				 /// Write out what's left in the buffer.
	void		 flush (void);
	
				 /// Write out any data that's waiting to get out of the codec.
	void		 flushcodec (void);
	
				 /// End-of-file.
				 /// \return EOF status, \b true if the end of the file has 
				 ///         been reached.
	bool		 eof (void);
	
				 /// Position.
				 /// \return The current position in a disk file that is open
				 ///         for writing/reading.
	off_t		 pos (void);
	
				 /// Bool cast.
				 /// Casts to true if the file is open.
				 operator bool (void) { return (filno>=0); };

				 /// Last generated error string.
	const string &error (void) { return err; };
	
				 /// Last generated error code.
	unsigned int errorcode (void) { return errcode; };
	
				 /// Iterator for foreach.
	string		*visitchild (int index)
				 {
				 	string *s;
				 	if (index<0) return NULL;
				 	if (eof()) return NULL;
				 	s = gets();
				 	if (! s) return NULL;
				 	iter = s;
				 	return &iter;
				 }

	ringbuffer	 buffer; ///< The internal ringbuffer.
	iocodec		*codec; ///< If set, will be used to encode/decode data.
	
	int			 filno; ///< The unix filedescriptor.

protected:
	bool		 feof; ///< End-of-file
	bool		 nonblocking; ///< True if the fd is in non-blocking mode.
	unsigned int errcode; ///< Last generated error code/
	string 		 err; ///< Last generated error text.
	string		 iter; ///< Buffer for iteration
};

#endif
