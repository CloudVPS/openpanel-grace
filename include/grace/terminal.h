#ifndef _TERMINAL_H
#define _TERMINAL_H 1

#include <grace/file.h>
#include <grace/str.h>
#include <grace/strutil.h>
#include <grace/filesystem.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <stdarg.h>
#include <sys/ioctl.h>

#define KEYCODE_HOME 1
#define KEYCODE_END 5
#define KEYCODE_BACKSPACE 8
#define KEYCODE_CLEARSCREEN 12
#define KEYCODE_ERASEWORD 23
#define KEYCODE_DELETE 127
#define KEYCODE_CLEARLEFT 21
#define KEYCODE_CLEARRIGHT 25
#define KEYCODE_WORDRIGHT 6
#define KEYCODE_WORDLEFT 2
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
					 /// \param c The character to insert.
	void			 insert (char c);
	
					 /// Insert a string into the buffer.
					 /// \param str The text to insert.
	void			 insert (const string &str)
					 {
						for (unsigned int i=0; i<str.strlen(); ++i)
							insert (str[i]);
					 }
					 /// Perform a backspace.
	void			 backspace (void);
	
					 /// Erase word to left of cursor.
	void			 eraseword (void);
	
					 /// Perform a cursor left movement.
	void			 crleft (void);
	
					 /// Perform a cursor right movement.
	void			 crright (void);
	
					 /// Move one word to the left.
	void			 wordleft (void);
	
					 /// Move one word to the right.
	void			 wordright (void);
	
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
	
					 /// Clear the terminal buffer to the left of the
					 /// cursor.
	void			 clearleft (void);
	
					 /// Clear the terminal buffer to the right of the
					 /// cursor.
	void			 clearright (void);
	
					 /// Get the current contents of the input buffer.
	string			*getline (void)
					 {
					 	string *res = new (memory::retainable::onstack) string
					 							(buffer + prompt.strlen());
						//res->chomp ();
						return res;
					 }
					 
					 /// Send a console message (should be called
					 /// from another thread).
	void			 sendconsole (const string &s)
					 {
					 	eventlock.lockw();
					 	events.newval() = s;
					 	eventlock.unlock();
					 }
					 
					 /// Send a line directly to the console, with no
					 /// regard to the state of the terminal. Safe from
					 /// command callbacks, unsafe from other threads.
	void			 writeconsole (const string &s)
					 {
					 	fout.writeln (s);
					 }
					 
					 /// Report the current cursor position.
	int				 crsrpos (void) { return crsr - prompt.strlen(); }
	
					 /// Report the current input buffer length.
	int				 length (void) { return len - prompt.strlen(); }
	
					 /// Set the cursorposition.
	void			 setcrsr (int i)
					 {
					 	crsr = prompt.strlen() + i;
					 	if (crsr>len) crsr = len;
					 	if (i<0) crsr = 0;
					 }
					 
					 /// Force the buffer to be completely redrawn
					 /// on the next draw() cycle.
	void			 redraw (void)
					 {
					 	for (unsigned int i=0; i<wsize; ++i) curview[i] = 0;
					 }
					 
	void			 savehistory (const string &to)
					 {
					 	history.savexml (to);
					 }
					 
	void			 loadhistory (const string &from)
					 {
					 	history.loadxml (from);
					 	historycrsr = history.count();
					 }

	file			 fin, fout; // Input/output file descriptors.
					 
protected:
	unsigned int	 size; ///< Total size of the buffer.
	unsigned int	 wsize; ///< Width of the display
	unsigned int	 len; ///< Current length of text in buffer.
	unsigned int	 crsr; ///< Current cursor position.
	unsigned int	 ocrsr; ///< Old cursor position.
	unsigned int	 wcrsr; ///< Current offset of the viewport.
	unsigned int	 owcrsr; ///< Old offset of the viewport.
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

	/// A node in the keyboard response list. Contains
	/// a pointer to a method to call.
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
	
	/// Turn terminal mode off.
	void off (void) { termbuf.off(); }
	
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
					
				case KEYCODE_CLEARLEFT:
					termbuf.clearleft();
					break;

				case KEYCODE_CLEARRIGHT:
					termbuf.clearright();
					break;
					
				case KEYCODE_WORDLEFT:
					termbuf.wordleft();
					break;
				
				case KEYCODE_WORDRIGHT:
					termbuf.wordright();
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
	string *readline (const string &prompt, bool basicmode=false)
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
			while ((!basicmode) && r)
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
						
					case KEYCODE_CLEARLEFT:
						termbuf.clearleft();
						break;

					case KEYCODE_CLEARRIGHT:
						termbuf.clearright();
						break;
						
					case KEYCODE_WORDLEFT:
						termbuf.wordleft();
						break;
						
					case KEYCODE_CLEARSCREEN:
						termbuf.tprintf ("\033[2J\033[0;0H");
						termbuf.redraw();
						break;
						
					case KEYCODE_ERASEWORD:
						termbuf.eraseword();
						break;
					
					case KEYCODE_WORDRIGHT:
						termbuf.wordright();
						break;
						
					case KEYCODE_ESCAPE:
						kib = termbuf.getkey();
						if (kib != '[')
						{
							if (kib == 'f') termbuf.wordright();
							else if (kib == 'b') termbuf.wordleft();
							else
							{
								termbuf.insert ("^[");
								termbuf.insert (kib);
							}
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
		if (! basicmode) termbuf.tohistory();
		
		string *res = termbuf.getline();
		res->chomp();
		return res;
	}
		
	termbuffer termbuf; ///< Embedded termbuffer.
				
protected:
	ctlclass *ctl; ///< The parent object.
	keyresponse *first; ///< First node in the keyresponse linked list.
	keyresponse *last; ///< Last node in the keyresponse linked list.
	
};

/// Utility class for the cli template class to save on inlining.
class cliutil
{
public:
	//@{
	/// Internal handling method for the cli class.
	static void splitwords (const string &src, int atpos, value &into);
	static void expandword (const string &part, const value &options,
							string &into);
	static void displayoptions (termbuffer &tb, const value &options);
	static void parsedeclaration (const string &, string &, value &);
	static void sethelp (const string &, const string &, value &);
	//@}
};

/// A vt100 command line interface and command parser module.
/// The parser's main principle is a syntax tree where certain nodes
/// may be of a dynamic type. The end of a tree leads to a callback.
/// Callbacks both for dynamic types and command execution are non-static
/// methods of the ctlclass.
///
/// Here's an example of an application using a cli without dynamic
/// lists:
/// \verbinclude cli_ex1.cpp
/// The two built-in tokens '*' and '#' can be used to substitute generic
/// string input. A word in a syntax rule containing a '*' will accept any
/// non-whitespace input or input enclosed by full quotes. The '#' word can
/// be appended at the end of a rule and indicates that the previous rule
/// should be evaluated for each consecutive word in the command line. Here
/// is an example:
/// \verbinclude cli_ex2.cpp
template <class ctlclass> class cli
{
public:
	/// A callback method for a dynamic list.
	typedef value *(ctlclass::*srcmethod)(const value &, int);
	
	/// A callback method for command execution.
	typedef int (ctlclass::*hmethod)(const value &);

	/// An array node containing a link to a specific dynamic list method.
	class expandsource
	{
	public:
						 expandsource (const statstring &sname, srcmethod m)
						 {
						 	name = sname;
						 	method = m;
						 	next = prev = NULL;
						 }
						~expandsource (void) {}
						
		value			*callsrc (ctlclass *x, const value &ln, int p)
						 {
						 	return (x->*method) (ln, p);
						 }
		
		expandsource	*next, *prev;
		statstring		 name;
		srcmethod		 method;
	};
	
	/// An array node containing a link to a specific command execution method.
	class cmdhandler
	{
	public:
						 /// Constructor.
						 /// \param ppath The path of the command.
						 /// \param m The method to call.
						 cmdhandler (const statstring &ppath, hmethod m)
						 {
						 	path = ppath;
						 	method = m;
						 	next = prev = NULL;
						 }
						 
						 /// Destructor.
						~cmdhandler (void)
						 {
						 }
						 
						 /// Calls the method.
						 /// \param x The class-instance for the callback.
						 /// \param argv Arguments for the call.
		int				 runcmd (ctlclass *x, const value &argv)
						 {
						 	return (x->*method) (argv);
						 }
						 
						 //@{ Linked list pointers.
		cmdhandler		*next, *prev; //@}
		statstring		 path; ///< The 'path' of the command.
		hmethod			 method; ///< The method to call.
	};
	
	/// Constructor.
	/// \param p The parent object to associate with callbacks.
	/// \param in The application input stream.
	/// \param out The application output stream.
	/// \param _size The buffer size (default 4096).
	/// \param _wsize THe window width (default auto).
	cli (ctlclass *p, file &in, file &out, int _size=4096, int _wsize=0)
		: term (this, in, out, (int) _size, (int) _wsize)
	{
		first = last = NULL;
		hfirst = hlast = NULL;
		exitcmd = "exit";
		upcmd = "..";
		owner = p;
		
		term.addkeyresponse (4, &cli<ctlclass>::exithandler);
		term.addkeyresponse (26, &cli<ctlclass>::uphandler);
		term.addkeyresponse (9, &cli<ctlclass>::tabhandler);
		term.addkeyresponse ('?', &cli<ctlclass>::tabhandler);
	}

	/// Destructor.
	/// Remove linked lists.
	~cli (void)
	{
		expandsource *s, *ns;
		s = first;
		while (s)
		{
			ns = s->next;
			delete s;
			s = ns;
		}
		first = last = NULL;
		
		cmdhandler *h, *nh;
		h = hfirst;
		while (h)
		{
			nh = h->next;
			delete h;
			h = nh;
		}
		hfirst = hlast = NULL;
	}
	
	/// Call a dynamic list source with a specific command line context.
	/// \param srcid The source id, including the leading '@'.
	/// \param cmd The command line.
	/// \param pos The focused position in the command list.
	value *callsrc (const statstring &srcid, const value &cmd, int pos)
	{
		expandsource *s;
		s = first;
		while (s)
		{
			if (s->name == srcid)
			{
				return s->callsrc (owner, cmd, pos);
			}
			s = s->next;
		}
		return NULL;
	}
	
	/// Add a link to a dynamic list source.
	/// A source should be a method to the parent class that returns
	/// a pointer to a new value object which contains a context-
	/// sensitive list of expansion options for a given command list
	/// at a given cursor position.
	/// \param srcid The source id, including the leading '@'.
	/// \param m The method to call on the parent object.
	void addsrc (const statstring &srcid, srcmethod m)
	{
		expandsource *s = new expandsource (srcid, m);
		if (last)
		{
			s->prev = last;
			last->next = s;
			last = s;
		}
		else
		{
			last = first = s;
		}
	}
	
	/// Add a syntax rule and help text. A rule is a list of words
	/// separated by a single space. Words can be either:
	/// - A literal string ("kill", "maim", ...)
	/// - A wildcard ("*", "poontang=*")
	/// - A reference to a dynamic list/type ("@files", "@ipaddress", ...)
	/// - A repeat indicator '#'.
	/// \param cmd The command and its parameters.
	/// \param m The method on the parent object to call.
	/// \param help text for the final node of the command list.
	void addsyntax (const string &cmd, hmethod m, const string &help)
	{
		addsyntax (cmd, m);
		addhelp   (cmd, help);
	}
	
	/// Add a syntax rule.
	/// \param cmd The command and its parameters.
	/// \param m The method on the parent object to call.
	void addsyntax (const string &cmd, hmethod m)
	{
		string hackme = cmd;
		cliutil::parsedeclaration (cmd, hackme, cmdtree);
		
		cmdhandler *h = new cmdhandler (cmd, m);
		if (hlast)
		{
			h->prev = hlast;
			hlast->next = h;
			hlast = h;
		}
		else
		{
			hfirst = hlast = h;
		}
	}
	
	/// Add a help text. Dynamic lists/types can generate their own
	/// help data, but any static text words should be explained.
	/// So, if you declare the syntax "show fridge status", you
	/// should add help for "show", "show fridge" as well as
	/// "show fridge status".
	/// \param path The command and its parameters.
	/// \param help The help text for the final node.
	void addhelp (const string &path, const string &help)
	{
		cliutil::sethelp (path, help, cmdtree);
	}
	
	/// Internal expansion method. Given a visitor to the syntax
	/// tree, the array of input words, the current input word
	/// position, this method will gather a list of all possible
	/// completions for the currently entered characters at
	/// the position.
	/// \param probe The syntax cursor.
	/// \param cmd The command line.
	/// \param atpos The word the cursor is in.
	/// \param into The return array. Nodes will have the completed
	///             word as their id, the description as their value
	///             and the actual token represented by the match
	///             in the syntax tree inside the "node" attribute.
	void fullexpand (visitor<value> &probe, value &cmd,
					 int atpos, value &into)
	{
		statstring theid;
		string word;
		
		word = cmd[atpos];
		
		foreach (obj, probe.obj())
		{
			value opts;
			value res;
			string ocmd;
			
			ocmd = obj.id().sval();
			
			if (ocmd[0] == '@')
			{
				opts = callsrc (obj.id(), cmd, atpos);
				foreach (opt, opts)
				{
					string mopt;
					mopt = opt.id().sval();
					if ((mopt.strlen()>1) && (mopt[-1] == '*'))
					{
						if ((! word.strlen()) ||
						    (word.strncmp (mopt, word.strlen()) == 0) ||
						    (word.strncmp (mopt, mopt.strlen()-1) == 0))
						{
							into[opt.id()] = opt;
							into[-1]("node") = ocmd;
						}
					}
					else if (mopt == "*")
					{
						into[word] = opt;
						into[-1]("node") = ocmd;
					}
					else if ((! word.strlen()) || 
					         (word.strncmp (opt.id().sval(), word.strlen()) == 0))
					{
						if (opt.id() == word) into.clear ();
						into[opt.id()] = opt;
						into[-1]("node") = ocmd;
						if (opt.id() == word) return;
					}
				}
			}
			else if (ocmd == "*")
			{
				if (word.strlen())
				{
					into[word] = obj("description");
					into[-1]("node") = "*";
				}
				else
				{
					into["<string>"] = obj("description");
					into[-1]("node") = "*";
				}
			}
			else
			{
				if ((! word.strlen()) || 
				    (word.strncmp (ocmd, word.strlen()) == 0))
				{
					if (word == ocmd)
					{
						into.clear();
					}
					
					into[obj.id()] = obj ("description");
					into[-1]("node") = obj.id().sval();
					
					if (word == ocmd) return;
				}
			}
		}
	}
	
	/// Keyhandler for the ^D command.
	/// \param ki The pressed key.
	/// \param tb The termbuffer.
	int exithandler (int ki, termbuffer &tb)
	{
		string ln = tb.getline();
		if (ln.strlen()) return 0;
		tb.set (exitcmd);
		return 1;
	}
	
	/// Keyhandler for the ^Z command.
	/// \param key The pressed key.
	/// \param tb The termbuffer.
	int uphandler (int key, termbuffer &tb)
	{
		string ln = tb.getline();
		if (ln.strlen()) return 0;
		tb.set (upcmd);
		return 1;
	}
	
	/// Keyhandler for tab expansion and the question mark key.
	/// \param ki The pressed key, either '\\t', '?' or 0, which is
	///           a special trigger to fully expand any abbreviated
	///           words to their full syntax tree equivalent.
	/// \param tb The termbuffer.
	int tabhandler (int ki, termbuffer &tb)
	{
		value split;
		string ln;
		visitor<value> probe (cmdtree);
		value opts;
		int i;
		cmdline.clear();
		curcmd = "";
		
		ln = tb.getline();
		cliutil::splitwords (ln, ki ? tb.crsrpos() : ln.strlen(), split);
		if (! ki) ln = ln.rtrim ();
		if (ki == '?')
		{
			if (ln[-1]=='\"')
			{
				tb.insert (ki);
				return 0;
			}
			// check out if we're in the middle of something quoted.
			if (split.count())
			{
				int qpos = split[-1].sval().strchr('\"');
				if (qpos >= 0)
				{
					// ok there's a quote, is there a second one?
					if (split[-1].sval().strchr ('\"', qpos+1) < 0)
					{
						// thought not, let's treat this as a question
						// mark proper.
						tb.insert (ki);
						return 0;
					}
				}
			}
			split.newval();
		}
		
		
		for (i=0; i< (split.count()-1); ++i)
		{
			//if (! (split[i].sval().strlen())) break;
			opts.clear ();
			fullexpand (probe, split, i, opts);
			
			switch (opts.count())
			{
				case 0:
					tb.tprintf ("%s%% Error at '%s'\n", ki?"\n":"",
								split[i].cval());
					if (ki) tb.redraw ();
					return 0;
				
				case 1:
					if (!ki)
					{
						string obid = opts[0].id().sval();
						if ((obid.strlen() == 1) || (obid[-1] != '*'))
						{
							split[i] = opts[0].id().sval();
						}
					}
					probe.enter (opts[0]("node").sval());
					break;
				
				default:
					tb.tprintf ("%s%% Ambiguous command at '%s'\n",
								ki?"\n":"", split[i].cval());
					if (ki) tb.redraw ();
					return 0;
			}
			
			if (probe.exists ("#"))
			{
				probe.up();
			}
		}
		
		opts.clear ();
		
		if (probe.enter ("#"))
		{
			probe.up();
			probe.up();
		}
		
		fullexpand (probe, split, i, opts);
		
		value dbug;
		dbug["probe"] = probe.obj();
		dbug["split"] = split;
		dbug["opts"] = opts;
		dbug.savexml ("terminus-debug.xml");
		
		if (ki == '?')
		{
			cliutil::displayoptions(tb, opts);
			return 0;
		}
		
		switch (opts.count())
		{
			case 0:
				if (probe.enter ("#"))
				{
					if (ki==9) tb.insert (" ");
					break;
				}
				// if (! split[i]) break;
				tb.tprintf ("%s%% Error at '%s'\n", ki?"\n":"", split[i].cval());
				if (ki) tb.redraw ();
				return 0;
			
			case 1:
				if (!ki)
				{
					string obid = opts[0].id().sval();
					if ((obid.strlen() == 1) || (obid[-1] != '*'))
					{
						split[i] = opts[0].id().sval();
					}
				}
				cliutil::expandword (split[i].sval(), opts, ln);
				if (ki == 9) tb.insert (ln);
				else if (! ki)
				{
					curcmd = "@error";
					if (probe.enter (opts[0]("node").sval()))
					{
						if (probe.obj().attribexists ("cmd"))
						{
							curcmd = probe.obj()("cmd").sval();
						}
					}
				}
				break;
				
			default:
				if (ki)
				{
					if (ki==9) cliutil::expandword (split[i].sval(), opts, ln);
					cliutil::displayoptions (tb, opts);
					if (ki==9) tb.insert (ln);
					tb.redraw ();
				}
				else
				{
					tb.tprintf ("%s%% Ambiguous command at '%s'\n",
								ki?"\n":"", split[i].cval());
				}
				break;
		}
		cmdline = split;
		return 0;
	}
	
	/// Start the command line interpreter.
	/// \param p The prompt.
	void run (const string &p)
	{
		bool done = false;
		setprompt (p);
		
		while (! done)
		{
			string res;
			res = term.readline (prompt);
			if (res.strlen())
			{
				tabhandler (0, term.termbuf);
				if (curcmd == "@error")
				{
					term.termbuf.tprintf ("%% Incomplete command\n");
				}
				else if (cmdline.count())
				{
					cmdhandler *h = hfirst;
					while (h)
					{
						if (h->path == curcmd)
						{
							if (h->runcmd (owner, cmdline)) done = true;
							break;
						}
						
						h = h->next;
					}
				}
			}
		}
	}
	
	/// Change the prompt. Can be used by callbacks.
	void setprompt (const string &p) { prompt = p; }
	
	/// Send data to the console. Can be used by other threads while
	/// the cli is in run() mode.
	void sendconsole (const string &s) { term.sendconsole (s); }
	
	/// Send formatted data to the console.
	void printf (const char *fmt, ...)
	{
		string out;
		va_list ap;
		
		va_start (ap, fmt);
		out.printf_va (fmt, &ap);
		va_end (ap);
		
		term.termbuf.fout.puts (out);
		term.termbuf.redraw ();
	}
	
	/// The embedded terminal object.
	terminal<cli <ctlclass> > term;

protected:
	expandsource *first; ///< First node in the expandsource linked list.
	expandsource *last; ///< Last node in the expandsource linked list.
	cmdhandler *hfirst; ///< First node in the cmdhandler linked list.
	cmdhandler *hlast; ///< Last node in the cmdhandler linked list.
	value cmdtree; ///< The syntax tree.
	string exitcmd; ///< The command that will be used for ^D.
	string upcmd; ///< The command that will be used for ^Z.
	string prompt; ///< The current prompt.
	ctlclass *owner; ///< Pointer to the parent object.
	statstring curcmd; ///< The declaration of a matched command stream. 
	value cmdline; ///< The last parsed command stream.
	
	string linebuffer;
};

#endif
