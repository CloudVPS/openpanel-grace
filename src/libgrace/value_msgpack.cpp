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
	// TODO: implement write support
}

