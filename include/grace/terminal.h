#ifndef _TERMINAL_H
#define _TERMINAL_H 1

#include <grace/file.h>
#include <grace/str.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>

#define KEYCODE_HOME 1
#define KEYCODE_END 5
#define KEYCODE_BACKSPACE 8
#define KEYCODE_DELETE 127
#define KEYCODE_CLEAR 21
#define KEYCODE_ESCAPE 27
#define KEYCODE_RETURN 10

/// A class implementing a VT100-compatible terminal suitable for
/// command line interaction models. The class models an input line that
/// automatically scrolls if the cursor reaches the end and the user
/// still has text to enter. The input line is prefixed by a configurable
/// prompt indicator.
class termbuffer
{
public:
					 /// Constructor.
					 /// \param in The terminal-attached input stream.
					 /// \param out The terminal-attached output stream.
					 /// \param _size The maximum buffer size.
					 /// \param _wsize The window width (0 for automatic).
					 termbuffer (file &in, file &out, int _size=4096,
					 			 int _wsize=0);
					 			 
					 /// Destructor.
					~termbuffer (void);
	
					 /// Turns terminal handling on. Should not be called
					 /// twice in a row, or previous state will be
					 /// nuked.
	void			 on (void);
	
					 /// Turns terminal handling back off. Should not be
					 /// called for a terminal that has not first been
					 /// called with termbuffer:::on().
	void			 off (void);
	
					 /// Set the prompt string. The new prompt will be
					 /// active at the next draw().
	void			 setprompt (const string &prmpt);
	
					 /// Insert a character into the buffer.
	void			 insert (char);
	
	void			 insert (const string &str)
					 {
						for (int i=0; i<str.strlen(); ++i) insert (str[i]);
					 }
									 /// Perform a backspace.
	void			 backspace (void);
	
					 /// Perform a cursor left movement.
	void			 crleft (void);
	
					 /// Perform a cursor right movement.
	void			 crright (void);
	
					 /// Move cursor to the end of the buffer.
	void			 crend (void);
	
					 /// Move cursor to the home position.
	void			 crhome (void);
	
					 /// Move up in the history.
	void			 crup (void);
	
					 /// Move down in the history.
	void			 crdown (void);
	
					 /// Add current buffer to history.
	void			 tohistory (void);

					 /// Read a key from the terminal. This process
					 /// may be interrupted by console output sent
					 /// by other threads through the termbuffer::sendconsole
					 /// method if the user has not pressed a key for longer
					 /// than 2 seconds.
	int				 getkey (void);
	
					 /// Update the terminal display. Will selectively
					 /// update the visible characters on the screen that
					 /// have changed since the last time.
	void			 draw (void);
	
					 /// Print formatted text to the terminal display.
	void			 tprintf (const char *, ...);
	
					 /// Write a character to the display.
	void			 tputc (char);
	
					 /// Explicitly set the contents of the buffer.
					 /// Cursor will be moved to the end position.
	void			 set (const string &);
	
					 /// Clear the terminal buffer.
	void			 clear (void);
	
					 /// Get the current contents of the input buffer.
	string			*getline (void)
					 {
					 	return new (memory::retainable::onstack) string
					 							(buffer + prompt.strlen());
					 }
					 
					 /// Send a console message (should be called
					 /// from another thread).
	void			 sendconsole (const string &s)
					 {
					 	eventlock.lockw();
					 	events.newval() = s;
					 	eventlock.unlock();
					 }
					 
					 /// Report the current cursor position.
	int				 crsrpos (void) { return crsr - prompt.strlen(); }
	
					 /// Report the current input buffer length.
	int				 length (void) { return len - prompt.strlen(); }
	
					 /// Set the cursorposition.
	int				 setcrsr (int i)
					 {
					 	crsr = prompt.strlen() + i;
					 	if (crsr>len) crsr = len;
					 	if (crsr<0) crsr = 0;
					 }
					 
					 /// Force the buffer to be completely redrawn
					 /// on the next draw() cycle.
	void			 redraw (void)
					 {
					 	for (int i=0; i<wsize; ++i) curview[i] = 0;
					 }
					 
protected:
	file			 fin, fout; // Input/output file descriptors.
	int				 size; ///< Total size of the buffer.
	int				 wsize; ///< Width of the display
	int				 len; ///< Current length of text in buffer.
	int				 crsr; ///< Current cursor position.
	int				 ocrsr; ///< Old cursor position.
	int				 wcrsr; ///< Current offset of the viewport.
	int				 owcrsr; ///< Old offset of the viewport.
	char 			*buffer; ///< The input buffer.
	char			*curview; ///< The screen buffer.
	string			 prompt; ///< The prompt string.
	
	value			 events; ///< Queue of console events.
	lock<int>		 eventlock; ///< Lock on the event queue.
	
	value			 history; ///< Command history.
	int				 historycrsr; ///< Current position in command history.

	struct termios 	 oldterm; ///< Stored termios settings.
	struct termios	 newterm; ///< Active termios settings.
	
					 /// Automatically advance viewport cursor to updated
					 /// situation.
	void			 advance (void);
	
					 /// Move the cursor from one position to the other
					 /// in a somewhat efficient way using vt100 escapes.
					 /// Used by draw.
	void			 setpos (int, int);
};

/// A command line interface template built on top of the termbuffer
/// class. It implements a 'readline' call, that offers standard emacs
/// keybindings for controlling the cursor, with the following keys
/// handled by default:
/// - ^A beginning-of-line.
/// - ^E end-of-line.
/// - ^U erase line.
/// - left and right cursor keys for cursor movement.
/// - up and down cursor keys for history navigation.
///
/// This is implemented as a template class to allow callbacks to be
/// performed against non-static methods of your class of choice.
template <class ctlclass> class terminal
{
public:
	/// A callback method.
	typedef int (ctlclass::*kmethod)(int, termbuffer &);

	class keyresponse
	{
	public:
					 keyresponse (int pkey, kmethod pm)
					 {
					 	key = pkey;
					 	method = pm;
					 }
					~keyresponse (void)
					 {
					 }
				 
		int			 call (ctlclass *pctl, termbuffer &tb)
					 {
					 	return (pctl->*method) (key, tb);
					 }
		
		keyresponse	*next;
		keyresponse	*prev;
		int			 key;
		kmethod		 method;
	};

	/// Constructor.
	terminal (ctlclass *p, file &in, file &out, int _size=4096, int _wsize=0)
		: termbuf (in, out, _size, _wsize)
	{
		ctl = p;
		first = last = NULL;
	}
	
	/// Destructor. Remove linked list.
	~terminal (void)
	{
		keyresponse *c, *nc;
		c = first;
		while (c)
		{
			nc = c->next;
			delete c;
			c = nc;
		}
	}
	
	/// Explicitly set the input buffer.
	void set (const string &s) { termbuf.set(s); }

	/// Send text to the console from another thread. Console messages
	/// are only shown if the keyboard has been idle for at least a
	/// second.
	/// \param s The message to send.
	void sendconsole (const string &s) { termbuf.sendconsole(s); }
	
	/// Regsiter a new keyresponse.
	/// \param key The key code.
	/// \param pm The method to call.
	void addkeyresponse (int key, kmethod pm)
	{
		keyresponse *r = new keyresponse (key, pm);
		r->next = NULL;
		if (last)
		{
			r->prev = last;
			last->next = r;
			last = r;
		}
		else
		{
			r->prev = NULL;
			first = last = r;
		}
	}
	
	/// Read a password. Extended keyresponders are not called,
	/// only home, end, delte/backspace, clear and return
	/// are mapped.
	/// \param prompt The password prompt.
	string *readpass (const string &prompt)
	{
		termbuf.setprompt (prompt);
		termbuf.on ();
		termbuf.clear ();
		termbuf.draw ();
		
		bool done = false;
		int ki;
		
		while (! done)
		{
			ki = termbuf.getkey();
			switch (ki)
			{
				case KEYCODE_HOME:
					termbuf.crhome();
					break;
				
				case KEYCODE_END:
					termbuf.crend();
					break;
				
				case KEYCODE_DELETE:
				case KEYCODE_BACKSPACE:
					termbuf.backspace();
					break;
					
				case KEYCODE_CLEAR:
					termbuf.clear();
					break;

				case KEYCODE_RETURN:
					done = true;
					break;
				
				default:
					termbuf.insert (ki);
					break;
			}
		}
		
		termbuf.tprintf ("\n");
		termbuf.off ();
		return termbuf.getline ();
	}
	
	/// Read a line of input.
	/// \param prompt The input prompt.
	string *readline (const string &prompt)
	{
		termbuf.setprompt (prompt);
		termbuf.on ();
		termbuf.clear ();
		termbuf.draw ();
		
		bool done = false;
		int ki, kib, kic;
		bool handled;
		
		while (! done)
		{
			ki = termbuf.getkey();
			handled = false;
			
			keyresponse *r = first;
			while (r)
			{
				if (r->key == ki)
				{
					if (r->call (ctl, termbuf)) done = true;
					r = NULL;
					handled = true;
				}
				else
				{
					r = r->next;
				}
			}
			
			if (! handled)
			{
				switch (ki)
				{
					case KEYCODE_HOME:
						termbuf.crhome();
						break;
					
					case KEYCODE_END:
						termbuf.crend();
						break;
					
					case KEYCODE_DELETE:
					case KEYCODE_BACKSPACE:
						termbuf.backspace();
						break;
						
					case KEYCODE_CLEAR:
						termbuf.clear();
						break;
						
					case KEYCODE_ESCAPE:
						kib = termbuf.getkey();
						if (kib != '[')
						{
							termbuf.insert ("^[");
							termbuf.insert (kib);
						}
						else
						{
							kic = termbuf.getkey();
							if (kic == 'C') termbuf.crright();
							else if (kic == 'D') termbuf.crleft();
							else if (kic == 'A') termbuf.crup();
							else if (kic == 'B') termbuf.crdown();
							else
							{
								termbuf.insert ("^[");
								termbuf.insert (kib);
								termbuf.insert (kic);
							}
						}
						break;
					
					case KEYCODE_RETURN:
						done = true;
						break;
					
					default:
						termbuf.insert (ki);
						break;
				}
			}
			
			termbuf.draw();
		}
		
		termbuf.tprintf ("\n");
		termbuf.off();
		termbuf.tohistory();
		
		return termbuf.getline();
	}
	
				
protected:
	ctlclass *ctl;
	keyresponse *first;
	keyresponse *last;
	termbuffer termbuf;
	
};

#endif
