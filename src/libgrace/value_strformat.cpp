#include <grace/str.h>
#include <grace/value.h>
#include <grace/strutil.h>

const char __HEXTAB[] = "0123456789abcdef";

string *operator% (const char *args, const value &arglist)
{
	returnclass (string) res retain;
	
	char *fmt = (char *) args;
	char copy[20];
	char sprintf_out[256];
	char *copy_p;
	int argptr = 0;
	string copy_s;
	int sz, asz;
	int szoffset = 1;
	statstring key;
	bool useskey;
	unsigned int ip;
	
	#define KEYORARG (useskey ? arglist[0][key] : arglist[argptr++])
	
	while (*fmt)
	{
		if (*fmt != '%')
		{
			res.strcat (char (*fmt++));
			continue;
		}
		
		++fmt;
		copy[0] = '%';
		useskey = false;
		
		for (copy_p = copy+1; copy_p < copy+19;)
		{
			switch ((*copy_p++ = *fmt++))
			{
				case 0:
					fmt--; goto CONTINUE;
					
				case '%':
					res.strcat ('%'); goto CONTINUE;
				
				case 'c':
					res.strcat ((char) KEYORARG.ival());
					goto CONTINUE;
					
				case '$':
					printcurrency (res, KEYORARG.getcurrency());
					goto CONTINUE;
					
				case 'L':
					--copy_p;
					*(copy_p++) = 'l';
					*(copy_p++) = 'l';
					*(copy_p++) = 'i';
					*copy_p = 0;
					
					sprintf (sprintf_out, copy, KEYORARG.lval());
					copy_p = sprintf_out;
					goto DUP;
				
				case 'X':
					--copy_p;
					*(copy_p++) = 'l';
					*(copy_p++) = 'l';
					*(copy_p++) = 'x';
					*copy_p = 0;
					
					sprintf (sprintf_out, copy, KEYORARG.ulval());
					copy_p = sprintf_out;
					goto DUP;
					
				case 'U':
					--copy_p;
					*(copy_p++) = 'l';
					*(copy_p++) = 'l';
					*(copy_p++) = 'u';
					*copy_p = 0;
					sprintf (sprintf_out, copy, KEYORARG.lval());
					copy_p = sprintf_out;
					goto DUP;
				
				case 'd':
				case 'i':
				case 'o':
				case 'u':
				case 'x':
					*copy_p = 0;
					sprintf (sprintf_out, copy, KEYORARG.ival());
					copy_p = sprintf_out;
					goto DUP;
				
				case 'e':
				case 'E':
				case 'f':
				case 'g':
					*copy_p = 0;
					sprintf (sprintf_out, copy, KEYORARG.dval());
					copy_p = sprintf_out;
					goto DUP;
				
				case 'S':
					copy_p = (char *) KEYORARG.cval();
					while (*copy_p)
					{
						char c = *copy_p;
						if ((c=='%')||(c=='\\')||(c=='\'')||(c=='\"'))
						{
							res.strcat ('\\');
							res.strcat (*copy_p++);
						}
						else if (*copy_p < 32)
						{
							res.strcat ('%');
							res.strcat (__HEXTAB [(*copy_p >> 4) & 15]);
							res.strcat (__HEXTAB [(*copy_p++) & 15]);
						}
						else res.strcat (*copy_p++);
					}
					goto CONTINUE;
					
				// mysql escape
				case 'M':
					{
						const string &kstr = KEYORARG.sval();
						char quot;
						
						if (kstr.strchr ('\'') >= 0)
						{
							quot = '\"';
						}
						else
						{
							quot = '\'';
						}
						
						res.strcat (quot);
						
						for (int ii=0; ii<kstr.strlen(); ++ii)
						{
							char c = kstr[ii];
							switch (c)
							{
								case 0 :
									res.strcat ("\\0");
									break;
								
								case '\\' :
									res.strcat ("\\\\");
									break;
								
								case '\"' :
									if (quot == '\"')
									{
										res.strcat ("\"\"");
									}
									else
									{
										res.strcat (c);
									}
									break;
								
								default:
									res.strcat (c);
									break;
							}
						}
						
						res.strcat (quot);
					}
					*copy_p = 0;
					goto CONTINUE;
					
				// ansi sql escape
				case 'Q':
					{
						const string &kstr = KEYORARG.sval();
						char quot;
						
						if (kstr.strchr ('\'') >= 0)
						{
							quot = '\"';
						}
						else
						{
							quot = '\'';
						}
						
						res.strcat (quot);
						
						for (int ii=0; ii<kstr.strlen(); ++ii)
						{
							char c = kstr[ii];
							switch (c)
							{
								case '\"' :
									if (quot == '\"')
									{
										res.strcat ("\"\"");
									}
									else
									{
										res.strcat (c);
									}
									break;
								
								default:
									res.strcat (c);
									break;
							}
						}
						
						res.strcat (quot);
					}
					*copy_p = 0;
					goto CONTINUE;
					
				case 'P':
					ip = KEYORARG.ipval();
					sprintf (sprintf_out, "%i.%i.%i.%i",
							   (ip & 0xff000000) >> 24,
							   (ip & 0xff0000) >> 16,
							   (ip & 0xff00) >> 8,
							   ip & 0xff);
					res.strcat (sprintf_out);
					goto CONTINUE;
				
				case 'Z':
					copy_p = (char *) KEYORARG.cval();
					while (*copy_p)
					{
						if ( (*copy_p == '&') )
						{
							res.strcat ("&amp;");
							++copy_p;
						}
						else if ( (*copy_p == '<') )
						{
							res.strcat ("&lt;");
							++copy_p;
						}
						else if ( (*copy_p == '>') )
						{
							res.strcat ("&gt;");
							++copy_p;
						}
						else if (*copy_p < 32)
						{
							res.strcat ("&#");
							::sprintf (sprintf_out,
									   "%i", (int) *copy_p++);
							res.strcat (sprintf_out);
							res.strcat (';');
						}
						else res.strcat (*copy_p++);
					}
					goto CONTINUE;
				
				case '!':
					copy_s = KEYORARG.toxml (value::compact);
					res.strcat (copy_s);
					goto CONTINUE;
				
				case 'J':
					copy_s = KEYORARG.tojson ();
					res.strcat (copy_s);
					goto CONTINUE;
					
				case '{':
					if (*fmt)
					{
						argptr = ::atoi (fmt);
						fmt++;
						if (*fmt) fmt++;
						copy_p--;
					}
					break;
					
				case '[':
					copy_s.crop ();
					while ((*fmt) && (*fmt != ']'))
					{
						copy_s.strcat (*fmt++);
					}
					if (*fmt) fmt++;
					key = copy_s;
					useskey=true;
					*copy_p = 0;
					copy_p--;
					break;
				
				case '~':
					copy_s = strutil::urlencode (KEYORARG.sval());
					res.strcat (copy_s);
					goto CONTINUE;
				
				case 's':
					*copy_p = 0;
					sz = atoi ((const char *)copy+1);
					copy_s = KEYORARG.sval();
					if (sz != 0)
					{
						copy_s.pad (sz, ' ');
					}
					res.strcat (copy_s);
					goto CONTINUE;
					
DUP:
					res.strcat ((char *) copy_p);
					goto CONTINUE;
			}
		}
CONTINUE:
	;
	}
	return &res;
}

value format (const value &v)
{
	value res;
	res.newval() = v;
	return res;
}

value format (const value &v1, const value &v2)
{
	value res;
	res.newval() = v1;
	res.newval() = v2;
	return res;
}

value format (const value &v1, const value &v2, const value &v3,
			  const value &v4)
{
	value res;
	res.newval() = v1;
	res.newval() = v2;
	res.newval() = v3;
	if (! v4.isempty()) res.newval() = v4;
	return res;
}

value format (const value &v1, const value &v2, const value &v3,
			  const value &v4, const value &v5, const value &v6,
			  const value &v7, const value &v8, const value &v9,
			  const value &v10, const value &v11, const value &v12)
{
	#define ADDROW(var) if (var.isempty()) return res; res.newval() = var
	value res;
	res.newval() = v1;
	res.newval() = v2;
	res.newval() = v3;
	res.newval() = v4;
	res.newval() = v5;
	ADDROW(v6);
	ADDROW(v7);
	ADDROW(v8);
	ADDROW(v9);
	ADDROW(v10);
	ADDROW(v11);
	ADDROW(v12);
	
	#undef ADDROW
	return res;
}

