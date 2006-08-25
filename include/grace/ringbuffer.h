#ifndef _RINGBUFFER_H
#define _RINGBUFFER_H 1

#include <grace/exception.h>

THROWS_EXCEPTION (bufferOverflowException, 0x826ec972, "Ring buffer overflow");

/// Circular buffer for byte streams.
/// Implements a cyclical buffer with a maximum size.
class ringbuffer
{
public:
					 /// Constructor.
					 /// \param sz Size of the buffer.
					 ringbuffer (unsigned int sz = 8192);
					~ringbuffer (void);
	
					 /// Returns the buffer size.
	unsigned int	 size (void) { return count; }

					 /// Unread byte count.	
	unsigned int	 backlog (void);
	
					 /// Reports space left in the buffer.
	unsigned int	 room (void);
	
					 /// Initialize the buffer with a new size.
					 /// \param sz Requested buffer size.
	void			 init (unsigned int sz);
	
					 /// Add data to the buffer.
					 /// \param data Pointer to the data block.
					 /// \param sz Number of bytes to copy.
	void			 add (const char *data, unsigned int sz);
	
					 /// Read a number of bytes from the buffer.
	class string	*read (unsigned int);
	
					 /// Read a number of bytes without advancing
					 /// the cursor.
	class string	*peek (unsigned int);
	
					 /// Manually advance the cursor.
	void			 advance (unsigned int);
	
					 /// Read a line from the buffer.
	class string	*readline (void);
	
					 /// Returns true if there is a terminated line in
					 /// the buffer.
	bool			 hasline (void);
	
					 /// Figure out if there's a newline in the buffer.
					 /// \param maxsz Maximum number of bytes to look
					 ///              ahead.
					 /// \return Newline status, \b true if there's a
					 /// newline within the limited range.
	bool			 hasline (unsigned int &maxsz);
	
					 /// Look for a sequence.
					 /// Scans the buffer for a character sequence.
					 /// \param seq Pointer to the search data.
					 /// \param sz Size of the search sequence.
					 /// \param pos Reported position (altered by method).
					 /// \return Status, \b true if the sequence was found.
	bool			 findforward (const char *seq, unsigned int sz,
								  unsigned int &pos);
					
					 /// Empty the buffer.
	void			 flush (void) { readcursor = writecursor = 0; }
	
					 /// Copy data from another buffer.
	void			 copy (ringbuffer &);
	
					 /// Get read cursor.
					 /// \return Position of the read cursor.
	unsigned int	 rc (void) { return readcursor; }
	
					 /// Get write cursor.
					 /// \return Position of the write cursor.
	unsigned int	 wc (void) { return writecursor; }
	char			*buffer; ///< The memory buffer.
	
protected:
	unsigned int	 count; ///< Allocated buffer size.
	unsigned int	 readcursor; ///< Read cursor position.
	unsigned int	 writecursor; ///< Write cursor position.
};

#endif
