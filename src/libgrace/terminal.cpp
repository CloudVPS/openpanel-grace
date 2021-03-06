// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

// ========================================================================
// terminal.cpp: VT100 terminal handling
// ========================================================================

//      012345678 TAB-O-METER should measure 4
//      ^	^
#include <grace/terminal.h>
#include <grace/strutil.h>
#include <grace/filesystem.h>

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
	engaged = false;
	utf8expect = 0;
	memset (&oldterm, 0, sizeof(oldterm));
	memset (&newterm, 0, sizeof(newterm));
	
	size = _size;
	
	// Using malloc here because we'll want to realloc eventually.
	buffer = (char *) malloc (size * sizeof (char));
	curview = (char *) malloc (size * sizeof (char));

	// Copy viewport size if explicitely defined.
	wsize = _wsize;
	if (! wsize) // Defaulted to 0?
	{
		// Deduct terminal size.
		if (ioctl (fin.filno, TIOCGWINSZ, (char *) &sz) >= 0)
			wsize = sz.ws_col;
	}
	
	if (! wsize) wsize = 80;

	for (unsigned int i=0; i<wsize; ++i) curview[i] = ' ';
	
	// Initialize all other cursors.
	crsr = ocrsr = wcrsr = owcrsr = 0;
	historycrsr = 0;
	
	idlecb = NULL;
	idlearg = NULL;
	
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

unsigned int _calcutfpos (const char *buf, unsigned int crsr)
{
	unsigned int res = 0;
	for (unsigned int i=0; i<crsr; ++i) if ((buf[i] & 0xc0) != 0x80) res++;
	return res;
}

unsigned int _calcunipos (const char *buf, unsigned int ucrsr)
{
	unsigned int res = 0;
	unsigned int pos = 0;
	
	while (buf[res] && (pos < ucrsr))
	{
		res++;
		while ((buf[res] & 0xc0) == 0x80) res++;
		pos++;
	}
	
	return res;
}

// ==========================================================================
// METHOD termbuffer::on
// ==========================================================================
void termbuffer::on (void)
{
	if (engaged) return;
	
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
	engaged = true;
}

// ==========================================================================
// METHOD termbuffer::off
// ==========================================================================
void termbuffer::off (void)
{
	if (engaged) tcsetattr (fin.filno, TCSAFLUSH, &oldterm);
	engaged = false;
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
	// Nothing to backspace before the prompt.
	if (crsr <= prompt.strlen()) return;
	
	do
	{
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
		
		unsigned int rcrsr = _calcutfpos (buffer, crsr);
		unsigned int rwcrsr = _calcutfpos (buffer, wcrsr);
		
		// Does the viewport need to move along?
		if (rcrsr < rwcrsr)
		{
			if (rcrsr > (wsize-6))
			{
				wcrsr = _calcunipos (buffer, rcrsr - (wsize-6));
			}
			else wcrsr = 0;
		}
	} while ( (buffer[crsr-1] & 0xc0) == 0x80 );
}

// ==========================================================================
// METHOD termbuffer::del
// ==========================================================================
void termbuffer::del (void)
{
	if (crsr < prompt.strlen()) return;
	if (crsr == len) return;
	
	do
	{
		memmove (buffer+crsr, buffer+crsr+1, len-(crsr+1));
		len--;
		buffer[len] = 0;
	} while ((crsr < len) && ((buffer[crsr] & 0xc0) == 0x80));
}

// ==========================================================================
// METHOD termbuffer::eraseword
// ==========================================================================
void termbuffer::eraseword (void)
{
	backspace();
	while ( (crsr>0) && (crsr > prompt.strlen()) && (buffer[crsr-1] != ' ') )
		backspace();
}

// ==========================================================================
// METHOD termbuffer::crleft
// ==========================================================================
void termbuffer::crleft (void)
{
	do
	{
		if (crsr > prompt.strlen()) crsr--;
		else return;
	} while (crsr && ((buffer[crsr-1] & 0xc0) == 0x80));
	
	unsigned int rcrsr = _calcutfpos (buffer, crsr);
	unsigned int rwcrsr = _calcutfpos (buffer, wcrsr);
		
	if (rcrsr < rwcrsr)
	{
		if (rcrsr > (wsize-6))
		{
			wcrsr = _calcunipos (buffer, rcrsr - (wsize-6));
		}
		else wcrsr = 0;
	}
}

// ==========================================================================
// METHOD termbuffer::crright
// ==========================================================================
void termbuffer::crright (void)
{
	do
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
	} while ((buffer[crsr] & 0xc0) == 0x80);
	
	advance();
}

// ==========================================================================
// METHOD termbuffer::crend
// ==========================================================================
void termbuffer::crend (void)
{
	crsr = len;

	unsigned int rcrsr = _calcutfpos (buffer, crsr);
		
	if (rcrsr > (wsize-4))
	{
		wcrsr = _calcunipos (buffer, rcrsr - (wsize - 4));
	}
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
	if (strlen(buffer) > prompt.strlen())
	{
		history.newval() = buffer + prompt.strlen();
		historycrsr = history.count() - 1;
		while (history.count() > 256) history.rmindex (0);
	}
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
	
	if ((c & 0xc0) == 0xc0)
	{
		if ((c & 0xe0) == 0xc0) utf8expect = 2;
		else if ((c & 0xf0) == 0xe0) utf8expect = 3;
		else if ((c & 0xf8) == 0xf0) utf8expect = 4;
	}
	
	if (utf8expect) utf8expect--;
	else
	{
		advance();
	}
}

// ==========================================================================
// METHOD termbuffer::clearleft
// ==========================================================================
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

// ==========================================================================
// METHOD termbuffer::clearright
// ==========================================================================
void termbuffer::clearright (void)
{
	if (crsr == len) return;
	
	for (;len>crsr;--len) buffer[len] = 0;
}

// ==========================================================================
// METHOD termbuffer::wordleft
// ==========================================================================
void termbuffer::wordleft (void)
{
	unsigned int promptsz = prompt.strlen();
	if (crsr <= promptsz) return;
	crsr--;
	if (crsr > promptsz) crsr--;
	
	while ((crsr>promptsz) && (buffer[crsr] != ' ')) crsr--;
	if (crsr>promptsz) crsr++;
	advance();
}

// ==========================================================================
// METHOD termbuffer::wordright
// ==========================================================================
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

	unsigned int rcrsr = _calcutfpos (buffer, crsr);
	unsigned int rwcrsr = _calcutfpos (buffer, wcrsr);

	if (rcrsr < rwcrsr)
	{
		wcrsr = crsr;
		return;
	}
	if ( (rcrsr - rwcrsr) > (wsize-2) )
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
			wcrsr = _calcunipos (buffer, rwcrsr += 16);
		}
	}
}

// ==========================================================================
// METHOD termbuffer::set
// ==========================================================================
void termbuffer::set (const string &to)
{
	memset (curview, 0, len+1);
	memset (buffer+prompt.strlen(), 0, size-prompt.strlen());
	crsr = prompt.strlen();
	len = prompt.strlen();
	buffer[prompt.strlen()] = 0;
	strcat (buffer, to.str());
	len += to.strlen();
	buffer[len] = 0;
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
	unsigned int rocrsr = _calcutfpos (buffer, ocrsr);
	unsigned int rcrsr = _calcutfpos (buffer, crsr);
	unsigned int rwcrsr = _calcutfpos (buffer, wcrsr);
	unsigned int rowcrsr = _calcutfpos (buffer, owcrsr);
	
	unsigned int xc;
	unsigned int cpos = rocrsr - rowcrsr;
	unsigned int rxc = 0;

	for (xc=0; xc<(wsize-1); ++xc)
	{
		if ((rxc+wcrsr) >= len)
		{
			if (curview[xc] != ' ')
			{
				setpos (cpos, xc);
				tputc (' ');
				curview[xc] = ' ';
				cpos = xc+1;
				
			}
		}
		else if (curview[xc] != buffer[rxc+wcrsr])
		{
			curview[xc] = buffer[rxc+wcrsr];

			setpos (cpos, xc);
			do
			{
				tputc (buffer[rxc+wcrsr]);
				rxc++;
			} while ((buffer[rxc+wcrsr] & 0xc0) == 0x80);
			
			cpos = xc+1;
		}
		else do { rxc++; } while ((buffer[rxc+wcrsr] & 0xc0) == 0x80);
	}
	
	setpos (cpos, rcrsr - rwcrsr);

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
	
	(void) write (fout.filno, data.str(), data.strlen());
}

// ==========================================================================
// METHOD termbuffer::tputs
// ==========================================================================
void termbuffer::tputs (const string &str)
{
	(void) write (fout.filno, str.str(), str.strlen());
}

// ==========================================================================
// METHOD termbuffer::tputc
// ==========================================================================
void termbuffer::tputc (char c)
{
	(void) write (fout.filno, &c, sizeof (char));	
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
			if (idlecb) idlecb (idlearg);
			eventlock.lockw();
			if (events.count())
			{
				while (events.count())
				{
					value ev;
					ev = events[0];
					events.rmindex (0);
					eventlock.unlock();
					
					fout.puts ("\n%s\n" %format (ev));
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
	
	into = strutil::splitquoted (src, ' ', true);
	for (i=0; i<into.count(); ++i)
	{
		if (into[i].sval().strlen() == 0)
		{
			into.rmindex (i--);
		}
	}
	
	if (! src.strlen()) return;
	
	for (i=0; tsz < atpos; ++i)
	{
		const string &v = into[i].sval();
		tsz += v.strlen() + 1;
		if (v.strchr (' ') >= 0)
		{
			if ((v.strchr ('\"') < 0) && (v.strchr ('\'') < 0)) tsz += 2;
		}
	}
	
	while (i<into.count()) into.rmindex (i);
	//if ( (atpos>0) && (src[atpos-1] == ' ') ) into.newval() == "";
}

// ==========================================================================
// METHOD cliutil::expandword
// ==========================================================================
void cliutil::expandword (const string &part, const value &options,
						  string &into)
{
	string completion;
	bool hadwildcard = false;

	if (! part.strlen())
	{
		into.crop (0);
		return;
	}
	
	foreach (opt, options)
	{
		string total = opt.id().sval();
		if (! total.strlen()) continue;
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
	if (! options.count())
	{
		tb.tputs ("\n<cr>            Execute command\n");
		tb.redraw ();
		return;
	}
	
	int maxlen = 14;
	foreach (opt, options)
	{
		int w = opt.id().sval().strlen();
		if (w > maxlen) maxlen = w;
	}
	
	tb.tputs ("\n");
	
	foreach (opt, options)
	{
		string nm = opt.id().sval();
		nm.pad (maxlen, ' ');
		tb.tputs ("%s  %s\n" %format (nm, opt));
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
