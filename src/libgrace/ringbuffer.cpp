// ========================================================================
// ringbuffer.cpp: Keyed generic data storage class
//
// (C) Copyright 2005-2006 Pim van Riezen <pi@madscience.nl>
//                         Madscience Labs, Rotterdam 
// ========================================================================

//      012345678 TAB-O-METER should measure 4
//      ^	^

#include <grace/ringbuffer.h>
#include <grace/str.h>
#include <string.h>

// ========================================================================
// CONSTRUCTOR
// -----------
// Initializes the buffersize and cursors, sets the buffer allocation
// to a specific size, but does not yet actually allocate the buffer.
// ========================================================================
ringbuffer::ringbuffer (unsigned int sz)
{
	count = sz;
	buffer = NULL;
	readcursor = writecursor = 0;
	::printf ("+ringbuffer %08x\n", this);
}

// ========================================================================
// DESTRUCTOR
// ----------
// De-allocates the buffer if it was ever allocated.
// ========================================================================
ringbuffer::~ringbuffer (void)
{
	::printf ("-ringbuffer %08x buffer=%08x\n", this, buffer);
	if (buffer) delete[] buffer;
}

// ========================================================================
// METHOD ::init
// -------------
// Resizes the buffer to a new size, clearing all the old buffer data.
// ========================================================================
void ringbuffer::init (unsigned int sz)
{
	if (buffer != NULL)
	{
		::printf ("*ringbuffer::init %08x oldbuffer=%08x\n", this, buffer);
		delete[] buffer;
	}
	buffer = new char[sz];
	::printf ("*ringbuffer::init %08x newbuffer=%08x\n", this, buffer);
	count = sz;
	readcursor = writecursor = 0;
}

// ========================================================================
// METHOD ::backlog
// ----------------
// Returns the number of bytes between the read- and write-cursors,
// indicating the amount of unprocessed bytes in the stream.
// ========================================================================
unsigned int ringbuffer::backlog (void)
{
	if (! buffer)
	{
		init (count);
	}
	return ( ((writecursor + count) - readcursor) % count );
}

// ========================================================================
// METHOD ::room
// -------------
// Returns the number of bytes that can be written until the write cursor
// will leap over the readcursor, minus 256 (or 0 if this would yield a
// negative number). Indicates the room left in the buffer for writing
// with a safety margin.
// ========================================================================
unsigned int ringbuffer::room (void)
{
	unsigned int res;
	if (! buffer)
	{
		init (count);
	}
	if (readcursor == writecursor) return count-256;
	res = ( ((readcursor + count) - writecursor) % count );
	if (res<256) return 0;
	return ((res-256)%count);
}

// ========================================================================
// METHOD ::add
// ------------
// Add data to the buffer. Check room() for space.
// ========================================================================
void ringbuffer::add (const char *data, unsigned int sz)
{
	if (! buffer)
	{
		init (count);
	}

	unsigned int toend;
	if (sz > count) throw (bufferOverflowException());
	toend = count - writecursor;
	if (sz > toend)
	{
		memmove (buffer+writecursor, data, toend);
		memmove (buffer, data + toend, sz - toend);
	}
	else
	{
		memmove (buffer+writecursor, data, sz);
	}
	
	writecursor = (writecursor + sz) % count;
	
}

// ========================================================================
// METHOD ::read
// -------------
// Read data from the buffer, shift read cursor.
// ========================================================================
string *ringbuffer::read (unsigned int sz)
{
	if (! buffer)
	{
		init (count);
	}

	unsigned int rsz = (sz > count) ? count : sz;
	unsigned int toend = count - readcursor;
	
	returnclass (string) res retain;
	
	if (rsz > backlog()) rsz = backlog();
	
	if (rsz > toend)
	{
		res.strcat (buffer + readcursor, (size_t) toend);
		res.strcat (buffer, (size_t) (rsz - toend));
	}
	else if (rsz)
	{
		res.strcat (buffer + readcursor, (size_t) rsz);
	}
	
	readcursor = (readcursor + rsz) % count;
	return &res;
}

// ========================================================================
// METHOD ::peek
// -------------
// Get data out of the buffer without updating the read cursor.
// ========================================================================
string *ringbuffer::peek (unsigned int sz)
{
	if (! buffer)
	{
		init (count);
	}

	unsigned int rsz = (sz > count) ? count : sz;
	unsigned int toend = count - readcursor;
	
	returnclass (string) res retain;
	
	if (rsz > backlog()) rsz = backlog();
	
	if (rsz > toend)
	{
		res.strcat (buffer + readcursor, (size_t) toend);
		res.strcat (buffer, (size_t) (rsz - toend));
	}
	else if (rsz)
	{
		res.strcat (buffer + readcursor, (size_t) rsz);
	}
	
	return &res;
}

void ringbuffer::advance (unsigned int sz)
{
	unsigned int rsz = sz;
	if (rsz > backlog()) rsz = backlog();
	readcursor = (readcursor + rsz) % count;
}

// ========================================================================
// METHOD ::hasline
// ----------------
// Returns true if there is a newline in the buffer.
// ========================================================================
bool ringbuffer::hasline (void)
{
	if (! buffer)
	{
		init (count);
	}

	if (readcursor == writecursor)
	{
		return false;
	}

	unsigned int pcursor = readcursor;
	while (pcursor != writecursor)
	{
		if (buffer[pcursor] == '\n') return true;
		pcursor = (pcursor+1) % count;
	}
	return false;
}

// ========================================================================
// METHOD ::hasline
// ----------------
// Returns position of a newline, if one is in the buffer.
// ========================================================================
bool ringbuffer::hasline (unsigned int &atposition)
{
	if (! buffer)
	{
		init (count);
	}

	if (readcursor == writecursor) return false;
	atposition = 0;

	unsigned int pcursor = readcursor;
	while (pcursor != writecursor)
	{
		if (buffer[pcursor] == '\n')
		{
			if (atposition && (buffer[(pcursor-1)%count] == '\r'))
				--atposition;
			return true;
		}
		pcursor = (pcursor+1) % count;
		++atposition;
	}
	return false;
}

// ========================================================================
// METHOD ::readline
// -----------------
// Reads a newline-terminated line of text out of the buffer, if there
// is one.
// ========================================================================
string *ringbuffer::readline (void)
{
	if (! buffer)
	{
		init (count);
	}
	string *result;

	unsigned int eolpos = 0;
	bool found = false;
	bool previousWasCR = false;
	if (readcursor == writecursor) return NULL;
	unsigned int pcursor = readcursor;
	
	while (pcursor != writecursor)
	{
		eolpos++;
		if (buffer[pcursor] == '\r')
		{
			previousWasCR = true;
			pcursor = (pcursor+1) % count;
		}
		else if (buffer[pcursor] == '\n')
		{
			found = true;
			eolpos--;
			if (previousWasCR) eolpos--;
			pcursor = writecursor;
		}
		else
		{
			previousWasCR = false;
			pcursor = (pcursor+1) % count;
		}
	}
	
	if (eolpos) result = read (eolpos);
	else result = new (memory::retainable::onstack) string;
	
	if (found)
	{
		if (readcursor!=writecursor)
		{
			if (buffer[readcursor] == '\r') 
				readcursor = (readcursor+1) % count;
			if (readcursor!=writecursor)
			{
				if (buffer[readcursor] == '\n')
					readcursor = (readcursor+1) % count;
			}
		}
	}
	
	return result;
}

// ========================================================================
// METHOD ::copy
// -------------
// Fish the buffer data out of another ringbuffer.
// ========================================================================
void ringbuffer::copy (ringbuffer &orig)
{
	if (! buffer) init (count);

	memmove (buffer, orig.buffer, count);
	readcursor = orig.readcursor;
	writecursor = orig.writecursor;
}

// ========================================================================
// METHOD ::findforward
// --------------------
// Look for a sequence of characters in the buffer data.
// ========================================================================
bool ringbuffer::findforward (const char *what, unsigned int sz,
							  unsigned int &setposition)
{
	unsigned int cursor = readcursor;
	unsigned int counter;
	unsigned int icounter;
	unsigned int tbacklog = backlog();
	if (readcursor == writecursor) return false;
	if (tbacklog < sz) return false;
	
	for (counter=0; counter<=(tbacklog-sz); ++counter)
	{
		if (buffer[cursor] == what[0])
		{
			for (icounter=1; icounter<sz; ++icounter)
			{
				if (buffer[(cursor+icounter)%count] != what[icounter])
					break;
			}
			if (icounter==sz)
			{
				setposition = counter;
				return true;
			}
		}
		cursor = (cursor+1) % count;
	}
	return false;
}
