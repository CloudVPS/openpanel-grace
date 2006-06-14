#include <grace/terminal.h>

#define VT100_CRRIGHT "\033[C"
#define VT100_CRLEFT "\033[D"
#define VT100_HOME "\033[1G"
#define VT100_MOVETO(xx) "\033[%iG", (xx)+1
#define XTERM_SETVT100 "\033 F"

termbuffer::termbuffer (file &in, file &out, int _size, int _wsize)
{
	fin = in;
	fout = out;
	struct winsize sz;
	
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

	for (int i=0; i<wsize; ++i) curview[i] = ' ';
	
	// Initialize all other cursors.
	crsr = ocrsr = wcrsr = owcrsr = 0;
	smaxpos = 0;
	historycrsr = 0;
	
	// Default prompt.
	setprompt ("$ ");
}

termbuffer::~termbuffer (void)
{
	free (buffer);
	free (curview);
}

void termbuffer::on (void)
{
	int i;
	string term;
	
	// Get current termios attributes.
	if (tcgetattr (fin.filno, &oldterm) < 0) return;
	
	// Create a copy of the attributes for the new setting.
	memcpy (&newterm, &oldterm, sizeof (newterm));
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

void termbuffer::off (void)
{
	tcsetattr (fin.filno, TCSAFLUSH, &oldterm);
}

void termbuffer::setprompt (const string &p)
{
	// If it doesn't fit, don't try it.
	if ((p.strlen() + len - prompt.strlen()) > size) return;
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
	rdpos = -1;
}

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
		rdpos = crsr;
	}
	else // No, shift everything.
	{
		memmove (buffer + crsr-1, buffer+crsr, len-crsr);
		len--;
		crsr--;
		rdpos = crsr-1;
		buffer[len] = 0;
	}
	
	// Does the viewport need to move along?
	if (crsr < wcrsr)
	{
		wcrsr = crsr - (wsize-6);
		if (wcrsr<0) wcrsr = 0;
		rdpos = wcrsr;
	}
}

void termbuffer::crleft (void)
{
	if (crsr > prompt.strlen()) crsr--;
	else return;
	
	rdpos = -1; // FIXME, shouldn't this just be crsr?
	
	if (crsr < wcrsr)
	{
		wcrsr = crsr - (wsize - 6);
		if (wcrsr<0) wcrsr = 0;
		rdpos = wcrsr;
	}
}

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
	
	rdpos = -1;
	advance();
}

void termbuffer::crend (void)
{
	crsr = len;
	wcrsr = crsr - (wsize - 4);
	if (wcrsr<0)
	{
		wcrsr = 0;
		rdpos = -1;
		return;
	}

	advance();
}

void termbuffer::crhome (void)
{
	if (crsr >= prompt.strlen())
	{
		crsr = prompt.strlen();
		rdpos = 0;
	}
	
	if (wcrsr)
	{
		wcrsr = 0;
		rdpos = 0;
	}
}

void termbuffer::tohistory (void)
{
	history.newval() = buffer + prompt.strlen();
	historycrsr = history.count() - 1;
}

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
		rdpos = crsr;
		crsr++;
	}
	else
	{
		memmove (buffer+crsr+1, buffer+crsr, len-crsr);
		len++;
		rdpos = crsr;
		buffer[crsr++] = c;
	}
	
	advance();
}

void termbuffer::advance (void)
{
	char *ptr;
	if (crsr < wcrsr)
	{
		wcrsr = rdpos = crsr;
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
		rdpos = wcrsr;
	}
}

void termbuffer::set (const string &to)
{
	memset (curview, 0, len);
	crsr = prompt.strlen();
	len = prompt.strlen();
	rdpos = prompt.strlen();
	buffer[prompt.strlen()] = 0;
	strcat (buffer, to.str());
	len += to.strlen();
	crend();
}

void termbuffer::clear (void)
{
	set ("");
}

void termbuffer::setpos (int cpos, int npos)
{
	switch (npos - cpos)
	{
		case 0: break;
		case 1: tprintf (VT100_CRRIGHT); break;
		default: tprintf (VT100_MOVETO(npos)); break;
	}
}

void termbuffer::draw (void)
{
	int xc;
	int cpos = ocrsr - owcrsr;

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

void termbuffer::tprintf (const char *fmt, ...)
{
	va_list ap;
	string data;
	
	va_start (ap, fmt);
	data.printf_va (fmt, &ap);
	va_end (ap);
	
	write (fout.filno, data.str(), data.strlen());
}

void termbuffer::tputc (char c)
{
	write (fout.filno, &c, sizeof (char));	
}

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
