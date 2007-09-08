#include <grace/str.h>
#include <grace/value.h>

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

value format (const value &v1, const value &v2, const value &v3)
{
	value res;
	res.newval() = v1;
	res.newval() = v2;
	res.newval() = v3;
	return res;
}

value format (const value &v1, const value &v2, const value &v3,
			  const value &v4)
{
	value res;
	res.newval() = v1;
	res.newval() = v2;
	res.newval() = v3;
	res.newval() = v4;
	return res;
}
value format (const value &v1, const value &v2, const value &v3,
			  const value &v4, const value &v5)
{
	value res;
	res.newval() = v1;
	res.newval() = v2;
	res.newval() = v3;
	res.newval() = v4;
	res.newval() = v5;
	return res;
}

value format (const value &v1, const value &v2, const value &v3,
			  const value &v4, const value &v5, const value &v6)
{
	value res;
	res.newval() = v1;
	res.newval() = v2;
	res.newval() = v3;
	res.newval() = v4;
	res.newval() = v5;
	res.newval() = v6;
	return res;
}

value format (const value &v1, const value &v2, const value &v3,
			  const value &v4, const value &v5, const value &v6,
			  const value &v7)
{
	value res;
	res.newval() = v1;
	res.newval() = v2;
	res.newval() = v3;
	res.newval() = v4;
	res.newval() = v5;
	res.newval() = v6;
	res.newval() = v6;
	res.newval() = v7;
	return res;
}

value format (const value &v1, const value &v2, const value &v3,
			  const value &v4, const value &v5, const value &v6,
			  const value &v7, const value &v8)
{
	value res;
	res.newval() = v1;
	res.newval() = v2;
	res.newval() = v3;
	res.newval() = v4;
	res.newval() = v5;
	res.newval() = v6;
	res.newval() = v6;
	res.newval() = v7;
	res.newval() = v8;
	return res;
}

value format (const value &v1, const value &v2, const value &v3,
			  const value &v4, const value &v5, const value &v6,
			  const value &v7, const value &v8, const value &v9)
{
	value res;
	res.newval() = v1;
	res.newval() = v2;
	res.newval() = v3;
	res.newval() = v4;
	res.newval() = v5;
	res.newval() = v6;
	res.newval() = v6;
	res.newval() = v7;
	res.newval() = v8;
	res.newval() = v9;
	return res;
}

value format (const value &v1, const value &v2, const value &v3,
			  const value &v4, const value &v5, const value &v6,
			  const value &v7, const value &v8, const value &v9,
			  const value &v10)
{
	value res;
	res.newval() = v1;
	res.newval() = v2;
	res.newval() = v3;
	res.newval() = v4;
	res.newval() = v5;
	res.newval() = v6;
	res.newval() = v6;
	res.newval() = v7;
	res.newval() = v8;
	res.newval() = v9;
	res.newval() = v10;
	return res;
}

value format (const value &v1, const value &v2, const value &v3,
			  const value &v4, const value &v5, const value &v6,
			  const value &v7, const value &v8, const value &v9,
			  const value &v10, const value &v11)
{
	value res;
	res.newval() = v1;
	res.newval() = v2;
	res.newval() = v3;
	res.newval() = v4;
	res.newval() = v5;
	res.newval() = v6;
	res.newval() = v6;
	res.newval() = v7;
	res.newval() = v8;
	res.newval() = v9;
	res.newval() = v10;
	res.newval() = v11;
	return res;
}

value format (const value &v1, const value &v2, const value &v3,
			  const value &v4, const value &v5, const value &v6,
			  const value &v7, const value &v8, const value &v9,
			  const value &v10, const value &v11, const value &v12)
{
	value res;
	res.newval() = v1;
	res.newval() = v2;
	res.newval() = v3;
	res.newval() = v4;
	res.newval() = v5;
	res.newval() = v6;
	res.newval() = v6;
	res.newval() = v7;
	res.newval() = v8;
	res.newval() = v9;
	res.newval() = v10;
	res.newval() = v11;
	res.newval() = v12;
	return res;
}

value format (const value &v1, const value &v2, const value &v3,
			  const value &v4, const value &v5, const value &v6,
			  const value &v7, const value &v8, const value &v9,
			  const value &v10, const value &v11, const value &v12,
			  const value &v13)
{
	value res;
	res.newval() = v1;
	res.newval() = v2;
	res.newval() = v3;
	res.newval() = v4;
	res.newval() = v5;
	res.newval() = v6;
	res.newval() = v6;
	res.newval() = v7;
	res.newval() = v8;
	res.newval() = v9;
	res.newval() = v10;
	res.newval() = v11;
	res.newval() = v12;
	res.newval() = v13;
	return res;
}

value format (const value &v1, const value &v2, const value &v3,
			  const value &v4, const value &v5, const value &v6,
			  const value &v7, const value &v8, const value &v9,
			  const value &v10, const value &v11, const value &v12,
			  const value &v13, const value &v14)
{
	value res;
	res.newval() = v1;
	res.newval() = v2;
	res.newval() = v3;
	res.newval() = v4;
	res.newval() = v5;
	res.newval() = v6;
	res.newval() = v6;
	res.newval() = v7;
	res.newval() = v8;
	res.newval() = v9;
	res.newval() = v10;
	res.newval() = v11;
	res.newval() = v12;
	res.newval() = v13;
	res.newval() = v14;
	return res;
}

value format (const value &v1, const value &v2, const value &v3,
			  const value &v4, const value &v5, const value &v6,
			  const value &v7, const value &v8, const value &v9,
			  const value &v10, const value &v11, const value &v12,
			  const value &v13, const value &v14, const value &v15)
{
	value res;
	res.newval() = v1;
	res.newval() = v2;
	res.newval() = v3;
	res.newval() = v4;
	res.newval() = v5;
	res.newval() = v6;
	res.newval() = v6;
	res.newval() = v7;
	res.newval() = v8;
	res.newval() = v9;
	res.newval() = v10;
	res.newval() = v11;
	res.newval() = v12;
	res.newval() = v13;
	res.newval() = v14;
	res.newval() = v15;
	return res;
}

value format (const value &v1, const value &v2, const value &v3,
			  const value &v4, const value &v5, const value &v6,
			  const value &v7, const value &v8, const value &v9,
			  const value &v10, const value &v11, const value &v12,
			  const value &v13, const value &v14, const value &v15,
			  const value &v16)
{
	value res;
	res.newval() = v1;
	res.newval() = v2;
	res.newval() = v3;
	res.newval() = v4;
	res.newval() = v5;
	res.newval() = v6;
	res.newval() = v6;
	res.newval() = v7;
	res.newval() = v8;
	res.newval() = v9;
	res.newval() = v10;
	res.newval() = v11;
	res.newval() = v12;
	res.newval() = v13;
	res.newval() = v14;
	res.newval() = v15;
	res.newval() = v16;
	return res;
}
