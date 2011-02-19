// This file is part of the Grace library (libgrace).
// The Grace library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, using version 3 of the License.
// You should have received a copy of the GNU Lesser General Public License 
// along with Grace library. If not, see <http://www.gnu.org/licenses/>.

#include <grace/value.h>
#include <grace/file.h>
#include <grace/stack.h>
#include <grace/strutil.h>

#include <stdint.h>

// ==========================================================================
// METHOD value::frommsgpack
// ==========================================================================
bool value::frommsgpack (const string &m, size_t& offset)
{
    clear();

    unsigned char opcode;
    offset = m.binget8u(offset,opcode);
    if (!offset) return false;

    if ((opcode & 0x80) == 0x00) // an integer within the range [0, 127] in 1 bytes
    {
        (*this) = (int)opcode;
    }
    else if ((opcode & 0xE0) == 0xE0) // signed 8-bit 111XXXXX
    {
        (*this) = (int)(char)opcode;
    }
    else if ((opcode & 0xE0) == 0xA0) // raw bytes up to 31 bytes.
    {
        type (t_string);
        int len = 0x5F & opcode;
        *this = m.mid (offset,len);
        offset += len;
        if (offset > m.strlen()) return false;
    }
    else if ((opcode & 0xF0) == 0x90) // an array up to 15 elements.
    {
        type (t_array);
        int len = 0x0F & opcode;
        while (len--)
        {
            bool success = newval().frommsgpack (m, offset);
            if (! success) return false;
        }
    }
    else if ((opcode & 0xF0) == 0x80) // a map up to 15 elements.
    {
        type (t_dict);
        // an integer within the range [-32, -1] in 1 bytes.
        int len = 0x0F & opcode;
        value v; // temporary element used for keys
        while (len--)
        {
            bool success = v.frommsgpack (m, offset);
            if (! success) return false;

            success = (*this)[statstring(v)].frommsgpack (m, offset);
            if (! success) return false;
        }
    }
    else switch (opcode)
    {
        case 0xc0: type (t_unset); break;

        case 0xc3: *this = true; break;
        case 0xc2: *this = false; break;

        case 0xcc: { uint8_t s;  offset = m.binget8u (offset,s);  *this=s;} break;
        case 0xcd: { uint16_t s; offset = m.binget16u (offset,s); *this=s;} break;
        case 0xce: { uint32_t s; offset = m.binget32u (offset,s); *this=s;} break;
        case 0xcf: { unsigned long long s; offset = m.binget64u (offset,s);
        			 *this=s;}  break;

        case 0xd0: { int8_t s;  offset = m.binget8 (offset,s);  *this=s;}  break;
        case 0xd1: { int16_t s; offset = m.binget16 (offset,s); *this=s;}  break;
        case 0xd2: { int32_t s; offset = m.binget32 (offset,s); *this=s;}  break;
        case 0xd3: { signed long long s; offset = m.binget64 (offset,s);
        			 *this=s;}  break;
                    
        // no single precision float support in grace. piggyback on int instead
        case 0xca: { union { float s; uint32_t i;};
        			 offset = m.binget32u (offset,i); *this=s;}  break;
        case 0xcb: { double s; offset = m.bingetieee (offset,s);
        			 *this=s;}  break;

        case 0xda: { 
                uint16_t s; offset = m.binget16u (offset,s); 
                *this = m.mid (offset,s); 
                offset+=s; 
            }  break;
        case 0xdb: { 
                uint32_t s; offset = m.binget32u (offset,s); 
                *this = m.mid (offset,s); 
                offset+=s; 
            }  break;
            
		// An array up to (2^16)-1 elements. Number of elements is stored 
		// in an unsigned 16-bit big-endian integer.
        case 0xdc: { 
                type (t_array);
                uint16_t len; offset = m.binget16u (offset,len);
                while (len--)
                {
                    bool success = newval().frommsgpack (m, offset);
                    if (! success) return false;
                }
            } break;
        
        // An array up to (2^32)-1 elements. Number of elements is stored in
        // an unsigned 32-bit big-endian integer.
        case 0xdd:  { 
                type (t_array);
                uint32_t len; offset = m.binget32u (offset,len);
                while (len--)
                {
                    bool success = newval().frommsgpack (m, offset);
                    if (! success) return false;
                }
            } break;
        
        // A map of up to (2^16)-1 elements. Number of elements is stored in
        // an unsigned 16-bit big-endian integer.
        case 0xde: {
                type (t_dict);
                
                uint16_t len; offset = m.binget16u (offset,len);
                value v; // temporary element used for keys
                while (len--)
                {
                    bool success = v.frommsgpack (m, offset);
                    if (! success) return false;

                    success = (*this)[statstring(v)].frommsgpack (m, offset);
                    if (! success) return false;
                }
            } break;
            
		// A map of up to (2^32)-1 elements. Number of elements is stored in
		// an unsigned 32-bit big-endian integer.
        case 0xdf: {
                type (t_dict);
                uint32_t len; offset = m.binget32u (offset,len);
                value v; // temporary element used for keys
                while (len--)
                {
                    bool success = v.frommsgpack (m, offset);
                    if (! success) return false;

                    success = (*this)[statstring(v)].frommsgpack (m, offset);
                    if (! success) return false;
                }
            } break;   
        default:
            break;
    }

    return offset > 0 && offset <= m.strlen();
}

// ==========================================================================
// METHOD value::tomsgpack
// ==========================================================================
string *value::tomsgpack (void) const
{
	returnclass (string) res retain;
	tomsgpack (res);
	return &res;
}


void value::tomsgpack (string& out) const
{
	if (count())
	{
		if (count() == ucount) // array
		{
			if (count() < 16) // fix array
			{
				out.binput8u (out.strlen(), 0x90 + count());
			}
			else if (count() < 65536) // array 16
			{
				out.binput8u (out.strlen(), 0xdc);
				out.binput16u (out.strlen(), count());
			}
			else // array 32
			{
				out.binput8u (out.strlen(), 0xdd);
				out.binput32u (out.strlen(), count());
			}
			
			// store individual array nodes
			for (int i=0; i<count(); ++i)
			{
				(*this)[i].tomsgpack (out);
			}
		}
		else if (count()) // map
		{
			if (count() < 16) // fix map
			{
				out.binput8u (out.strlen(), 0x80 + count());
			}
			else if (count() < 65536) // map 16
			{
				out.binput8u (out.strlen(), 0xde);
				out.binput16u (out.strlen(), count());
			}
			else
			{
				out.binput8u (out.strlen(), 0xdf);
				out.binput32u (out.strlen(), count());
			}
			
			foreach (v, (*this))
			{
				string id = v.id();
				int l = id.strlen();
				if (l < 32)
				{
					out.binput8u (out.strlen(), 0xa0 + l);
				}
				else if (l < 65536)
				{
					out.binput8u (out.strlen(), 0xda);
					out.binput16u (out.strlen(), l);
				}
				else
				{
					out.binput8u (out.strlen(), 0xdb);
					out.binput32u (out.strlen(), l);
				}
				
				out.strcat (id);
				v.tomsgpack (out);
			}
		}
	}
	else
	{
		switch (_itype)
		{
			case i_unset:
				out.binput8u (out.strlen(), 0xc0);
				break;
			
			case i_int:
				if ((ival() >= 0) && (ival() < 128))
				{
					out.binput8u (out.strlen(), ival());
				}
				else if ((ival() < 0) && (ival() >= -32))
				{
					out.binput8u (out.strlen(), 0xe0 - ival());
				}
				else out.binput32 (out.strlen(), ival());
				break;
			
			case i_unsigned:
				if (uval() < 128)
				{
					out.binput8u (out.strlen(), uval());
				}
				else if (uval() < 256) // uint 8
				{
					out.binput8u (out.strlen(), 0xcc);
					out.binput8u (out.strlen(), uval());
				}
				else if (uval() < 65536) // uint 16
				{
					out.binput8u (out.strlen(), 0xcd);
					out.binput16u (out.strlen(), uval());
				}
				else
				{
					out.binput8u (out.strlen(), 0xce);
					out.binput32u (out.strlen(), uval());
				}
				break;
			
			case i_double:
				// Always double
				out.binput8u (out.strlen(), 0xcb);
				out.binputieee (out.strlen(), dval());
				break;
				
			case i_bool:
				out.binput8u (out.strlen(), bval() ? 0xc3 : 0xc2);
				break;
			
			case i_long:
				out.binput8u (out.strlen(), 0xd3);
				out.binput64 (out.strlen(), lval());
				break;
			
			case i_ulong:
				out.binput8u (out.strlen(), 0xcf);
				out.binput64u (out.strlen(), ulval());
				break;
			
			case i_ipaddr:
			case i_date:
			case i_currency:
			case i_string:
				if (sval().strlen() < 32)
				{
					out.binput8u (out.strlen(), 0xa0 + sval().strlen());
				}
				else if (sval().strlen() < 65536)
				{
					out.binput8u (out.strlen(), 0xda);
					out.binput16u (out.strlen(), sval().strlen());
				}
				else
				{
					out.binput8u (out.strlen(), 0xdb);
					out.binput32u (out.strlen(), sval().strlen());
				}
				out.strcat (sval());
				break;
			
			default:
				out.binput8u (out.strlen(), 0xc0);
		}
	}
}

