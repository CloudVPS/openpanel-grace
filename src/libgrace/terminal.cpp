// ========================================================================
// terminal.cpp: VT100 terminal handling
//
// (C) Copyright 2006 Pim van Riezen <pi@madscience.nl>
//                    Madscience Labs, Rotterdam 
// ========================================================================

//      012345678 TAB-O-METER should measure 4
//      ^	^
#include <grace/terminal.h>
#include <grace/strutil.h>

#define VT100_CRRIGHT "\033[C"
#define VT100_CRLEFT "\033[D"
#define VT100_HOME "\033[1G"
#define VT100_MOVETO(xx) "\033[%iG", (xx)+1
#define XTERM_SETVT100 "\033 F"

// ==========================================================================
// CONSTRUCTOR
// ==========================================================================
termbuffer::termbuffer (file &in, file &out, int _size, int _wsize)
{
	fin = in;
	fout = out;
	struct winsize sz;
	len = 0;
	
	size = _size;
	
	// Using malloc here because we'll want to realloc eventually.
	buffer = (char *) malloc (size * sizeof (char *));
	curview = (char *) malloc (size * sizeof (char *));

	// Copy viewport size if explicitely defined.
	wsize = _wsize;
	if (! wsize) // Defaulted to 0?
	{
		// Deduct terminal size.
		ioctl (fin.filno, TIOCGWINSZ, (char *) &sz);
		wsize = sz.ws_col;
	}
	
	if (! wsize) wsize = 80;

	for (unsigned int i=0; i<wsize; ++i) curview[i] = ' ';
	
	// Initialize all other cursors.
	crsr = ocrsr = wcrsr = owcrsr = 0;
	historycrsr = 0;
	
	// Default prompt.
	setprompt ("$ ");
}

// ==========================================================================
// DESTRUCTOR
// ==========================================================================
termbuffer::~termbuffer (void)
{
	free (buffer);
	free (curview);
}

// ==========================================================================
// METHOD termbuffer::on
// ==========================================================================
void termbuffer::on (void)
{
	int i;
	string term;
	
	// Get current termios attributes.
	if (tcgetattr (fin.filno, &oldterm) < 0) return;
	
	// Create a copy of the attributes for the new setting.
	memmove (&newterm, &oldterm, sizeof (newterm));
	newterm.c_lflag = oldterm.c_lflag & ~ (ECHO | ICANON | ISIG);
	newterm.c_cc[VMIN] = 1;
	newterm.c_cc[VTIME] = 0;
	
	// Enact the new terminal discipline.
	tcsetattr (fin.filno, TCSAFLUSH, &newterm);
	
	// Technically speaking this makes no sense, the termios attributes
	// are never applied, yet this is how terminus inside cish/2 does it
	// now so I'll keep it around for this port.
	cfmakeraw (&newterm);
	
	term = ::getenv("TERM");
	if (term == "xterm")
	{
		// The xterm standard defines this sequence for 'vt100
		// compatibility mode', whatever the hell that means (it hasn't
		// yet unbroken any broken behaviour of xterm clones I've seen
		// in the wild).
		tprintf (XTERM_SETVT100);
	}
}

// ==========================================================================
// METHOD termbuffer::off
// ==========================================================================
void termbuffer::off (void)
{
	tcsetattr (fin.filno, TCSAFLUSH, &oldterm);
}

// ==========================================================================
// METHOD termbuffer::setprompt
// ==========================================================================
void termbuffer::setprompt (const string &p)
{
	// If it doesn't fit, don't try it.
	if ((p.strlen() + len - prompt.strlen()) > (unsigned int) size) return;
	int diff;
	
	diff = (p.strlen() - prompt.strlen());
	string tmp;
	
	// Is there any data?
	if (len)
	{
		tmp = buffer + prompt.strlen();
		::strcpy (buffer, p.str());
		::strcat (buffer, tmp.str());
		prompt = p;
		len = p.strlen() + tmp.strlen();
	}
	else // No, this will be easy.
	{
		prompt = p;
		::strcpy (buffer, p.str());
		len = p.strlen();
	}
	
	if (diff)
	{
		crsr += diff;
		if (crsr > len) crsr = len;
		else if (crsr < prompt.strlen()) crsr = prompt.strlen();
		advance();
	}
	
	historycrsr = history.count();
}

// ==========================================================================
// METHOD termbuffer::backspace
// ==========================================================================
void termbuffer::backspace (void)
{
	char *ptr;
	
	// Nothing to backspace before the prompt.
	if (crsr <= prompt.strlen()) return;
	
	// We're at the end?
	if (crsr == len)
	{
		buffer[crsr-1] = 0;
		len--;
		crsr--;
	}
	else // No, shift everything.
	{
		memmove (buffer + crsr-1, buffer+crsr, len-crsr);
		len--;
		crsr--;
		buffer[len] = 0;
	}
	
	// Does the viewport need to move along?
	if (crsr < wcrsr)
	{
		if (crsr > (wsize-6))
			wcrsr = crsr - (wsize-6);
		else wcrsr = 0;
	}
}

// ==========================================================================
// METHOD termbuffer::crleft
// ==========================================================================
void termbuffer::crleft (void)
{
	if (crsr > prompt.strlen()) crsr--;
	else return;
	
	if (crsr < wcrsr)
	{
		if (crsr > (wsize-6))
			wcrsr = crsr - (wsize-6);
		else wcrsr = 0;
	}
}

// ==========================================================================
// METHOD termbuffer::crright
// ==========================================================================
void termbuffer::crright (void)
{
	if (crsr < len)
	{
		crsr++;
	}
	else
	{
		tputc (7); // beep
		return;
	}
	
	advance();
}

// ==========================================================================
// METHOD termbuffer::crend
// ==========================================================================
void termbuffer::crend (void)
{
	crsr = len;
	if (crsr > (wsize-4))
		wcrsr = crsr - (wsize - 4);
	else
	{
		wcrsr = 0;
		return;
	}

	advance();
}

// ==========================================================================
// METHOD termbuffer::crhome
// ==========================================================================
void termbuffer::crhome (void)
{
	if (crsr >= prompt.strlen())
	{
		crsr = prompt.strlen();
	}
	
	if (wcrsr)
	{
		wcrsr = 0;
	}
}

// ==========================================================================
// METHOD termbuffer::tohistory
// ==========================================================================
void termbuffer::tohistory (void)
{
	history.newval() = buffer + prompt.strlen();
	historycrsr = history.count() - 1;
}

// ==========================================================================
// METHOD termbuffer::crup
// ==========================================================================
void termbuffer::crup (void)
{
	if (historycrsr == history.count())
	{
		if (len > prompt.strlen())
		{
			history.newval() = (char *) (buffer + prompt.strlen());
		}
	}
	if (! historycrsr) // top of history, beep.
	{
		tputc (7);
		return;
	}
	historycrsr--;
	
	set (history[historycrsr].sval());
}

// ==========================================================================
// METHOD termbuffer::crdown
// ==========================================================================
void termbuffer::crdown (void)
{
	if (historycrsr >= history.count())
	{
		tputc (7);
		return;
	}
	
	historycrsr++;
	set (history[historycrsr].sval());
}

// ==========================================================================
// METHOD termbuffer::insert
// ==========================================================================
void termbuffer::insert (char c)
{
	char *ptr;
	
	if (len >= size) // buffer full
	{
		tputc (7); // beep
		return;
	}
	
	if (crsr == len) // end of the line?
	{
		buffer[len++] = c;
		buffer[len] = 0;
		crsr++;
	}
	else
	{
		memmove (buffer+crsr+1, buffer+crsr, len-crsr);
		len++;
		buffer[crsr++] = c;
	}
	
	advance();
}

void termbuffer::clearleft (void)
{
	if (crsr == len)
	{
		set ("");
		return;
	}
	
	int promptsz = prompt.strlen();
	
	memmove (buffer+promptsz, buffer+crsr, len-crsr);
	len -= (crsr-promptsz);
	crsr = promptsz;
	wcrsr = 0;
	advance ();
}

void termbuffer::clearright (void)
{
	if (crsr == len) return;
	
	for (;len>crsr;--len) buffer[len] = 0;
}

void termbuffer::wordleft (void)
{
	int promptsz = prompt.strlen();
	if (crsr <= promptsz) return;
	crsr--;
	if (crsr > promptsz) crsr--;
	
	while ((crsr>promptsz) && (buffer[crsr] != ' ')) crsr--;
	if (crsr>promptsz) crsr++;
	advance();
}

void termbuffer::wordright (void)
{
	if (crsr == len) return;
	
	while ((crsr<len) && (buffer[crsr] != ' ')) crsr++;
	if (crsr<len) crsr++;
	advance();
}

// ==========================================================================
// METHOD termbuffer::advance
// ==========================================================================
void termbuffer::advance (void)
{
	char *ptr;
	if (crsr < wcrsr)
	{
		wcrsr = crsr;
		return;
	}
	if ( (crsr - wcrsr) > (wsize-2) )
	{
		// try to jump a word if this scrolls less than 32
		// positions.
		ptr = ::strchr (buffer+wcrsr+12, ' ');
		if (ptr && ( (ptr - (buffer+wcrsr)) < 32))
		{
			wcrsr = (ptr - buffer) + 1;
		}
		else // Mo suitable word boundary, just scroll 16 chars.
		{
			wcrsr += 16;
		}
	}
}

// ==========================================================================
// METHOD termbuffer::set
// ==========================================================================
void termbuffer::set (const string &to)
{
	memset (curview, 0, len);
	crsr = prompt.strlen();
	len = prompt.strlen();
	buffer[prompt.strlen()] = 0;
	strcat (buffer, to.str());
	len += to.strlen();
	crend();
}

// ==========================================================================
// METHOD termbuffer::clear
// ==========================================================================
void termbuffer::clear (void)
{
	set ("");
}

// ==========================================================================
// METHOD termbuffer::setpos
// ==========================================================================
void termbuffer::setpos (int cpos, int npos)
{
	switch (npos - cpos)
	{
		case 0: break;
		case 1: tprintf (VT100_CRRIGHT); break;
		default: tprintf (VT100_MOVETO(npos)); break;
	}
}

// ==========================================================================
// METHOD termbuffer::draw
// ==========================================================================
void termbuffer::draw (void)
{
	unsigned int xc;
	unsigned int cpos = ocrsr - owcrsr;

	for (xc=0; xc<(wsize-1); ++xc)
	{
		if ((xc+wcrsr) >= len)
		{
			if (curview[xc] != ' ')
			{
				setpos (cpos, xc);
				tputc (' ');
				curview[xc] = ' ';
				cpos = xc+1;
				
			}
		}
		else if (curview[xc] != buffer[xc+wcrsr])
		{
			setpos (cpos, xc);
			tputc (buffer[xc+wcrsr]);
			curview[xc] = buffer[xc+wcrsr];
			cpos = xc+1;
		}
	}
	
	setpos (cpos, crsr - wcrsr);

	ocrsr = crsr;
	owcrsr = wcrsr;
}

// ==========================================================================
// METHOD termbuffer::tprintf
// ==========================================================================
void termbuffer::tprintf (const char *fmt, ...)
{
	va_list ap;
	string data;
	
	va_start (ap, fmt);
	data.printf_va (fmt, &ap);
	va_end (ap);
	
	write (fout.filno, data.str(), data.strlen());
}

// ==========================================================================
// METHOD termbuffer::tputc
// ==========================================================================
void termbuffer::tputc (char c)
{
	write (fout.filno, &c, sizeof (char));	
}

// ==========================================================================
// METHOD termbuffer::getkey
// ==========================================================================
int termbuffer::getkey (void)
{
	string res;
	
	while (! res.strlen())
	{
		res = fin.read (1, 1000);
		if (! res.strlen())
		{
			eventlock.lockw();
			if (events.count())
			{
				while (events.count())
				{
					value ev;
					ev = events[0];
					events.rmindex (0);
					eventlock.unlock();

					fout.printf ("\n%s\n", ev.cval());
					memset (curview, 0, len);

					eventlock.lockw();
				}
				eventlock.unlock();
				draw();
			}
			else eventlock.unlock();
		}
	}
	
	return res[0];
}

// ==========================================================================
// METHOD cliutil::splitwords
// ==========================================================================
void cliutil::splitwords (const string &src, int atpos, value &into)
{
	string tmpres;
	int i;
	int tsz = 0;
	
	into = strutil::splitquoted (src, ' ');
	if (! src.strlen()) return;
	
	for (i=0; tsz < atpos; ++i)
	{
		const string &v = into[i].sval();
		tsz += v.strlen() + 1;
		if (v.strchr (' ') >= 0)
		{
			if (v.strchr ('\"') < 0) tsz += 2;
		}
	}
	
	while (i<into.count()) into.rmindex (i);
}

// ==========================================================================
// METHOD cliutil::expandword
// ==========================================================================
void cliutil::expandword (const string &part, const value &options,
						  string &into)
{
	string completion;
	bool hadwildcard = false;
	
	foreach (opt, options)
	{
		string total = opt.id().sval();
		if (total[-1] == '*')
		{
			if (total.strlen()>1)
			{
				total = total.copyuntil ('*');
				if (total.strncmp (part, total.strlen()) == 0)
				{
					total = part;
					total.strcat (" ");
				}
			}
			else
			{
				hadwildcard = true;
			}
		}
		else total.strcat (' ');
		
		//::printf ("<opt '%s'>", total.str());
		
		if (part.strncmp (total, part.strlen()) == 0)
		{
			if (! completion.strlen())
			{
				completion = total;
			}
			else
			{
				for (unsigned int i=0; i<completion.strlen(); ++i)
				{
					if (completion[i] != total[i])
					{
						completion.crop (i);
						break;
					}
				}
			}
		}
	}
	
	if (completion == "*") return;
	if (completion.strlen()) into = completion.mid (part.strlen());
	else if (part.strlen() && hadwildcard) into = " ";
}

// ==========================================================================
// METHOD cliutil::displayoptions
// ==========================================================================
void cliutil::displayoptions (termbuffer &tb, const value &options)
{
	int maxlen = 14;
	foreach (opt, options)
	{
		int w = opt.id().sval().strlen();
		if (w > maxlen) maxlen = w;
	}
	
	tb.tprintf ("\n");
	
	foreach (opt, options)
	{
		string nm = opt.id().sval();
		nm.pad (maxlen, ' ');
		tb.tprintf ("%s  %s\n", nm.str(), opt.cval());
	}
	tb.redraw ();
}

// ==========================================================================
// METHOD cliutil::parsedeclaration
// ==========================================================================
void cliutil::parsedeclaration (const string &cmd, string &declr, value &tree)
{
	value splt = strutil::split (declr, ' ');
	visitor<value> probe (tree);
	
	foreach (word, splt)
	{
		probe.obj()[word.sval()]("description") = "Unknown";
		probe.enter (word.sval());
	}
	probe.obj()("cmd") = cmd;
}

// ==========================================================================
// METHOD cliutil::sethelp
// ==========================================================================
void cliutil::sethelp (const string &path, const string &info, value &tree)
{
	value split;
	visitor<value> probe (tree);
	split = strutil::split (path, ' ');
	foreach (word, split)
	{
		if (! probe.enter (word.sval())) return;
	}
	probe.obj()("description") = info;
}
