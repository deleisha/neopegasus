//%2006////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002 BMC Software; Hewlett-Packard Development
// Company, L.P.; IBM Corp.; The Open Group; Tivoli Systems.
// Copyright (c) 2003 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation, The Open Group.
// Copyright (c) 2004 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation; VERITAS Software Corporation; The Open Group.
// Copyright (c) 2005 Hewlett-Packard Development Company, L.P.; IBM Corp.;
// EMC Corporation; VERITAS Software Corporation; The Open Group.
// Copyright (c) 2006 Hewlett-Packard Development Company, L.P.; IBM Corp.;
// EMC Corporation; Symantec Corporation; The Open Group.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// THE ABOVE COPYRIGHT NOTICE AND THIS PERMISSION NOTICE SHALL BE INCLUDED IN
// ALL COPIES OR SUBSTANTIAL PORTIONS OF THE SOFTWARE. THE SOFTWARE IS PROVIDED
// "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
// LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//==============================================================================
//
//%/////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cstdio>

#include <Pegasus/Common/Config.h>
#include "XmlGenerator.h"
#include "Constants.h"
#include "StrLit.h"
#include "CommonUTF.h"
#include "StringConversion.h"
#include "LanguageParser.h"
#include "AutoPtr.h"

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//
// SpecialChar and table.
//
////////////////////////////////////////////////////////////////////////////////

// Note: we cannot use StrLit here since it has a constructor (forbids
// structure initialization).

struct SpecialChar
{
    const char* str;
    Uint32 size;
};

// Defines encodings of special characters. Just use a 7-bit ASCII character
// as an index into this array to retrieve its string encoding and encoding
// length in bytes.
static const SpecialChar _specialChars[] =
{
    {STRLIT_ARGS("&#0;")},
    {STRLIT_ARGS("&#1;")},
    {STRLIT_ARGS("&#2;")},
    {STRLIT_ARGS("&#3;")},
    {STRLIT_ARGS("&#4;")},
    {STRLIT_ARGS("&#5;")},
    {STRLIT_ARGS("&#6;")},
    {STRLIT_ARGS("&#7;")},
    {STRLIT_ARGS("&#8;")},
    {STRLIT_ARGS("&#9;")},
    {STRLIT_ARGS("&#10;")},
    {STRLIT_ARGS("&#11;")},
    {STRLIT_ARGS("&#12;")},
    {STRLIT_ARGS("&#13;")},
    {STRLIT_ARGS("&#14;")},
    {STRLIT_ARGS("&#15;")},
    {STRLIT_ARGS("&#16;")},
    {STRLIT_ARGS("&#17;")},
    {STRLIT_ARGS("&#18;")},
    {STRLIT_ARGS("&#19;")},
    {STRLIT_ARGS("&#20;")},
    {STRLIT_ARGS("&#21;")},
    {STRLIT_ARGS("&#22;")},
    {STRLIT_ARGS("&#23;")},
    {STRLIT_ARGS("&#24;")},
    {STRLIT_ARGS("&#25;")},
    {STRLIT_ARGS("&#26;")},
    {STRLIT_ARGS("&#27;")},
    {STRLIT_ARGS("&#28;")},
    {STRLIT_ARGS("&#29;")},
    {STRLIT_ARGS("&#30;")},
    {STRLIT_ARGS("&#31;")},
    {STRLIT_ARGS(" ")},
    {STRLIT_ARGS("!")},
    {STRLIT_ARGS("&quot;")},
    {STRLIT_ARGS("#")},
    {STRLIT_ARGS("$")},
    {STRLIT_ARGS("%")},
    {STRLIT_ARGS("&amp;")},
    {STRLIT_ARGS("&apos;")},
    {STRLIT_ARGS("(")},
    {STRLIT_ARGS(")")},
    {STRLIT_ARGS("*")},
    {STRLIT_ARGS("+")},
    {STRLIT_ARGS(",")},
    {STRLIT_ARGS("-")},
    {STRLIT_ARGS(".")},
    {STRLIT_ARGS("/")},
    {STRLIT_ARGS("0")},
    {STRLIT_ARGS("1")},
    {STRLIT_ARGS("2")},
    {STRLIT_ARGS("3")},
    {STRLIT_ARGS("4")},
    {STRLIT_ARGS("5")},
    {STRLIT_ARGS("6")},
    {STRLIT_ARGS("7")},
    {STRLIT_ARGS("8")},
    {STRLIT_ARGS("9")},
    {STRLIT_ARGS(":")},
    {STRLIT_ARGS(";")},
    {STRLIT_ARGS("&lt;")},
    {STRLIT_ARGS("=")},
    {STRLIT_ARGS("&gt;")},
    {STRLIT_ARGS("?")},
    {STRLIT_ARGS("@")},
    {STRLIT_ARGS("A")},
    {STRLIT_ARGS("B")},
    {STRLIT_ARGS("C")},
    {STRLIT_ARGS("D")},
    {STRLIT_ARGS("E")},
    {STRLIT_ARGS("F")},
    {STRLIT_ARGS("G")},
    {STRLIT_ARGS("H")},
    {STRLIT_ARGS("I")},
    {STRLIT_ARGS("J")},
    {STRLIT_ARGS("K")},
    {STRLIT_ARGS("L")},
    {STRLIT_ARGS("M")},
    {STRLIT_ARGS("N")},
    {STRLIT_ARGS("O")},
    {STRLIT_ARGS("P")},
    {STRLIT_ARGS("Q")},
    {STRLIT_ARGS("R")},
    {STRLIT_ARGS("S")},
    {STRLIT_ARGS("T")},
    {STRLIT_ARGS("U")},
    {STRLIT_ARGS("V")},
    {STRLIT_ARGS("W")},
    {STRLIT_ARGS("X")},
    {STRLIT_ARGS("Y")},
    {STRLIT_ARGS("Z")},
    {STRLIT_ARGS("[")},
    {STRLIT_ARGS("\\")},
    {STRLIT_ARGS("]")},
    {STRLIT_ARGS("^")},
    {STRLIT_ARGS("_")},
    {STRLIT_ARGS("`")},
    {STRLIT_ARGS("a")},
    {STRLIT_ARGS("b")},
    {STRLIT_ARGS("c")},
    {STRLIT_ARGS("d")},
    {STRLIT_ARGS("e")},
    {STRLIT_ARGS("f")},
    {STRLIT_ARGS("g")},
    {STRLIT_ARGS("h")},
    {STRLIT_ARGS("i")},
    {STRLIT_ARGS("j")},
    {STRLIT_ARGS("k")},
    {STRLIT_ARGS("l")},
    {STRLIT_ARGS("m")},
    {STRLIT_ARGS("n")},
    {STRLIT_ARGS("o")},
    {STRLIT_ARGS("p")},
    {STRLIT_ARGS("q")},
    {STRLIT_ARGS("r")},
    {STRLIT_ARGS("s")},
    {STRLIT_ARGS("t")},
    {STRLIT_ARGS("u")},
    {STRLIT_ARGS("v")},
    {STRLIT_ARGS("w")},
    {STRLIT_ARGS("x")},
    {STRLIT_ARGS("y")},
    {STRLIT_ARGS("z")},
    {STRLIT_ARGS("{")},
    {STRLIT_ARGS("|")},
    {STRLIT_ARGS("}")},
    {STRLIT_ARGS("~")},
    {STRLIT_ARGS("&#127;")},
};

// If _isSpecialChar7[ch] is true, then ch is a special character, which must
// have a special encoding in XML. But only use 7-bit ASCII characters to
// index this array.
static const int _isSpecialChar7[] =
{
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0,0,
    0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
};

////////////////////////////////////////////////////////////////////////////////

Buffer& operator<<(Buffer& out, const Char16& x)
{
    XmlGenerator::append(out, x);
    return out;
}

Buffer& operator<<(Buffer& out, const String& x)
{
    XmlGenerator::append(out, x);
    return out;
}

Buffer& operator<<(Buffer& out, const Buffer& x)
{
    out.append(x.getData(), x.size());
    return out;
}

Buffer& operator<<(Buffer& out, Uint32 x)
{
    XmlGenerator::append(out, x);
    return out;
}

Buffer& operator<<(Buffer& out, const CIMName& name)
{
    XmlGenerator::append(out, name.getString ());
    return out;
}

Buffer& operator<<(Buffer& out, const AcceptLanguageList& al)
{
    XmlGenerator::append(out, LanguageParser::buildAcceptLanguageHeader(al));
    return out;
}

Buffer& operator<<(Buffer& out, const ContentLanguageList& cl)
{
    XmlGenerator::append(out, LanguageParser::buildContentLanguageHeader(cl));
    return out;
}

void XmlGenerator::_appendChar(Buffer& out, const Char16& c)
{
    // We need to convert the Char16 to UTF8 then append the UTF8
    // character into the array.
    // NOTE: The UTF8 character could be several bytes long.
    // WARNING: This function will put in replacement character for
    // all characters that have surogate pairs.
    char str[6];
    memset(str,0x00,sizeof(str));
    Uint8* charIN = (Uint8 *)&c;

    const Uint16 *strsrc = (Uint16 *)charIN;
    Uint16 *endsrc = (Uint16 *)&charIN[1];

    Uint8 *strtgt = (Uint8 *)str;
    Uint8 *endtgt = (Uint8 *)&str[5];

    UTF16toUTF8(
        &strsrc,
        endsrc,
        &strtgt,
        endtgt);

    out.append(str, UTF_8_COUNT_TRAIL_BYTES(str[0]) + 1);
}

void XmlGenerator::_appendSpecialChar7(Buffer& out, char c)
{
    if (_isSpecialChar7[int(c)])
        out.append(_specialChars[int(c)].str, _specialChars[int(c)].size);
    else
        out.append(c);
}

void XmlGenerator::_appendSpecialChar(Buffer& out, const Char16& c)
{
    if (c < 128)
        _appendSpecialChar7(out, char(c));
    else
        _appendChar(out, c);
}

void XmlGenerator::_appendSpecialChar(PEGASUS_STD(ostream)& os, char c)
{
    if ( ((c < 0x20) && (c >= 0)) || (c == 0x7f) )
    {
        char scratchBuffer[22];
        Uint32 outputLength;
        const char * output = Uint8ToString(scratchBuffer, 
                                            static_cast<Uint8>(c), 
                                            outputLength);
        os << "&#" << output << ";";
    }
    else
    {
        switch (c)
        {
            case '&':
                os << "&amp;";
                break;

            case '<':
                os << "&lt;";
                break;

            case '>':
                os << "&gt;";
                break;

            case '"':
                os << "&quot;";
                break;

            case '\'':
                os << "&apos;";
                break;

            default:
                os << c;
        }
    }
}

void XmlGenerator::_appendSurrogatePair(Buffer& out, Uint16 high, Uint16 low)
{
    char str[6];
    Uint8 charIN[5];
    memset(str,0x00,sizeof(str));
    memcpy(&charIN,&high,2);
    memcpy(&charIN[2],&low,2);
    const Uint16 *strsrc = (Uint16 *)charIN;
    Uint16 *endsrc = (Uint16 *)&charIN[3];

    Uint8 *strtgt = (Uint8 *)str;
    Uint8 *endtgt = (Uint8 *)&str[5];

    UTF16toUTF8(
        &strsrc,
        endsrc,
        &strtgt,
        endtgt);

    Uint32 number1 = UTF_8_COUNT_TRAIL_BYTES(str[0]) + 1;
    out.append(str,number1);
}

void XmlGenerator::_appendSpecial(PEGASUS_STD(ostream)& os, const char* str)
{
    while (*str)
        _appendSpecialChar(os, *str++);
}

// On windows sprintf outputs 3 digit precision exponent prepending
// zeros. Make it 2 digit precision if first digit is zero in the exponent.
#ifdef PEGASUS_OS_TYPE_WINDOWS
inline void _normalizeRealValueString(char *str)
{
    // skip initial sign value...
    if (*str == '-' || *str == '+')
    {
        ++str;
    }
    while (*str && *str != '+' && *str != '-')
    {
        ++str;
    }
    if (*str && * ++str == '0')
    {
        *str = *(str+1);
        *(str+1) = *(str+2);
        *(str+2) = 0;
    }
}
#endif

void XmlGenerator::append(Buffer& out, const Char16& x)
{
    _appendChar(out, x);
}

void XmlGenerator::append(Buffer& out, Boolean x)
{
    if (x)
        out.append(STRLIT_ARGS("TRUE"));
    else
        out.append(STRLIT_ARGS("FALSE"));
}

void XmlGenerator::append(Buffer& out, Uint32 x)
{
    Uint32 outputLength=0;
    char buffer[22];
    const char * output = Uint32ToString(buffer, x, outputLength);
    out.append(output, outputLength);
}

void XmlGenerator::append(Buffer& out, Sint32 x)
{
    Uint32 outputLength=0;
    char buffer[22];
    const char * output = Sint32ToString(buffer, x, outputLength);
    out.append(output, outputLength);
}

void XmlGenerator::append(Buffer& out, Uint64 x)
{
    Uint32 outputLength=0;
    char buffer[22];
    const char * output = Uint64ToString(buffer, x, outputLength);
    out.append(output, outputLength);
}

void XmlGenerator::append(Buffer& out, Sint64 x)
{
    Uint32 outputLength=0;
    char buffer[22];
    const char * output = Sint64ToString(buffer, x, outputLength);
    out.append(output, outputLength);
}

void XmlGenerator::append(Buffer& out, Real32 x)
{
    char buffer[128];
    // %.7e gives '[-]m.ddddddde+/-xx', which seems compatible with the format
    // given in the CIM/XML spec, and the precision required by the CIM 2.2 spec
    // (4 byte IEEE floating point)
    sprintf(buffer, "%.7e", x);
#ifdef PEGASUS_OS_TYPE_WINDOWS
    _normalizeRealValueString(buffer);
#endif
    append(out, buffer);
}

void XmlGenerator::append(Buffer& out, Real64 x)
{
    char buffer[128];
    // %.16e gives '[-]m.dddddddddddddddde+/-xx', which seems compatible
    // with the format given in the CIM/XML spec, and the precision required
    // by the CIM 2.2 spec (8 byte IEEE floating point)
    sprintf(buffer, "%.16e", x);
#ifdef PEGASUS_OS_TYPE_WINDOWS
    _normalizeRealValueString(buffer);
#endif
    append(out, buffer);
}

void XmlGenerator::append(Buffer& out, const char* str)
{
    size_t n = strlen(str);
    out.append(str, n);
}

void XmlGenerator::append(Buffer& out, const String& str)
{
    const Uint16* p = (const Uint16*)str.getChar16Data();
    size_t n = str.size();

    // Handle leading ASCII 7 characers in these next two loops (use unrolling).

    while (n >= 8 && ((p[0]|p[1]|p[2]|p[3]|p[4]|p[5]|p[6]|p[7]) & 0xFF80) == 0)
    {
        out.append(p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
        p += 8;
        n -= 8;
    }

    while (n >= 4 && ((p[0]|p[1]|p[2]|p[3]) & 0xFF80) == 0)
    {
        out.append(p[0], p[1], p[2], p[3]);
        p += 4;
        n -= 4;
    }

    while (n--)
    {
        Uint16 c = *p++;

        // Special processing for UTF8 case:

        if (c < 128)
        {
            out.append(c);
            continue;
        }

        // Handle UTF8 case (if reached).

        if (((c >= FIRST_HIGH_SURROGATE) && (c <= LAST_HIGH_SURROGATE)) ||
            ((c >= FIRST_LOW_SURROGATE) && (c <= LAST_LOW_SURROGATE)))
        {
            Char16 highSurrogate = p[-1];
            Char16 lowSurrogate = p[0];
            p++;
            n--;

            _appendSurrogatePair(
                out, Uint16(highSurrogate),Uint16(lowSurrogate));
        }
        else
        {
            _appendChar(out, c);
        }
    }
}

void XmlGenerator::appendSpecial(Buffer& out, const Char16& x)
{
    _appendSpecialChar(out, x);
}

void XmlGenerator::appendSpecial(Buffer& out, char x)
{
    _appendSpecialChar7(out, x);
}

void XmlGenerator::appendSpecial(Buffer& out, const char* str)
{
    while (*str)
        _appendSpecialChar7(out, *str++);
}

void XmlGenerator::appendSpecial(Buffer& out, const String& str)
{
    const Uint16* p = (const Uint16*)str.getChar16Data();
    // prevCharIsSpace is true when the last character written to the Buffer
    // is a space character (not a character reference).
    Boolean prevCharIsSpace = false;

    // If the first character is a space, use a character reference to avoid
    // space compression.
    if (*p == ' ')
    {
        out.append(STRLIT_ARGS("&#32;"));
        p++;
    }

    Uint16 c;
    while ((c = *p++) != 0)
    {
        if (c < 128)
        {
            if (_isSpecialChar7[c])
            {
                // Write the character reference for the special character
                out.append(
                    _specialChars[int(c)].str, _specialChars[int(c)].size);
                prevCharIsSpace = false;
            }
            else if (prevCharIsSpace && (c == ' '))
            {
                // Write the character reference for the space character, to
                // avoid compression
                out.append(STRLIT_ARGS("&#32;"));
                prevCharIsSpace = false;
            }
            else
            {
                out.append(c);
                prevCharIsSpace = (c == ' ');
            }
        }
        else
        {
            // Handle UTF8 case

            if ((((c >= FIRST_HIGH_SURROGATE) && (c <= LAST_HIGH_SURROGATE)) ||
                 ((c >= FIRST_LOW_SURROGATE) && (c <= LAST_LOW_SURROGATE))) &&
                *p)
            {
                _appendSurrogatePair(out, c, *p++);
            }
            else
            {
                _appendChar(out, c);
            }

            prevCharIsSpace = false;
        }
    }

    // If the last character is a space, use a character reference to avoid
    // space compression.
    if (prevCharIsSpace)
    {
        out.remove(out.size() - 1);
        out.append(STRLIT_ARGS("&#32;"));
    }
}

// See http://www.ietf.org/rfc/rfc2396.txt section 2
// Reserved characters = ';' '/' '?' ':' '@' '&' '=' '+' '$' ','
// Excluded characters:
//   Control characters = 0x00-0x1f, 0x7f
//   Space character = 0x20
//   Delimiters = '<' '>' '#' '%' '"'
//   Unwise = '{' '}' '|' '\\' '^' '[' ']' '`'
//

static const char _is_uri[128] =
{
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,
    1,1,0,0,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,
};

// Perform the necessary URI encoding of characters in HTTP header values.
// This is required by the HTTP/1.1 specification and the CIM/HTTP
// Specification (section 3.3.2).
void XmlGenerator::_encodeURIChar(String& outString, Sint8 char8)
{
    Uint8 c = (Uint8)char8;

#ifndef PEGASUS_DO_NOT_IMPLEMENT_URI_ENCODING
    if (c > 127 || _is_uri[int(c)])
    {
        char hexencoding[4];
        int n = sprintf(hexencoding, "%%%X%X", c/16, c%16);
#ifdef PEGASUS_USE_STRING_EXTENSIONS
        outString.append(hexencoding, n);
#else /* PEGASUS_USE_STRING_EXTENSIONS */
        outString.append(hexencoding);
#endif /* PEGASUS_USE_STRING_EXTENSIONS */
    }
    else
#endif
    {
        outString.append((Uint16)c);
    }
}

String XmlGenerator::encodeURICharacters(const Buffer& uriString)
{
    String encodedString;

    for (Uint32 i=0; i<uriString.size(); i++)
    {
        _encodeURIChar(encodedString, uriString[i]);
    }

    return encodedString;
}

String XmlGenerator::encodeURICharacters(const String& uriString)
{
    String encodedString;

    // See the "CIM Operations over HTTP" spec, section 3.3.2 and
    // 3.3.3, for the treatment of non US-ASCII (UTF-8) chars

    // First, convert to UTF-8 (include handling of surrogate pairs)
    Buffer utf8;
    for (Uint32 i = 0; i < uriString.size(); i++)
    {
        Uint16 c = uriString[i];

        if (((c >= FIRST_HIGH_SURROGATE) && (c <= LAST_HIGH_SURROGATE)) ||
            ((c >= FIRST_LOW_SURROGATE) && (c <= LAST_LOW_SURROGATE)))
        {
            Char16 highSurrogate = uriString[i];
            Char16 lowSurrogate = uriString[++i];

            _appendSurrogatePair(
                utf8, Uint16(highSurrogate),Uint16(lowSurrogate));
        }
        else
        {
            _appendChar(utf8, uriString[i]);
        }
    }

    // Second, escape the non HTTP-safe chars
    for (Uint32 i=0; i<utf8.size(); i++)
    {
        _encodeURIChar(encodedString, utf8[i]);
    }

    return encodedString;
}

//------------------------------------------------------------------------------
//
// _printAttributes()
//
//------------------------------------------------------------------------------

void XmlGenerator::_printAttributes(
    PEGASUS_STD(ostream)& os,
    const XmlAttribute* attributes,
    Uint32 attributeCount)
{
    for (Uint32 i = 0; i < attributeCount; i++)
    {
        os << attributes[i].name << "=";

        os << '"';
        _appendSpecial(os, attributes[i].value);
        os << '"';

        if (i + 1 != attributeCount)
            os << ' ';
    }
}

//------------------------------------------------------------------------------
//
// _indent()
//
//------------------------------------------------------------------------------

void XmlGenerator::_indent(
    PEGASUS_STD(ostream)& os,
    Uint32 level,
    Uint32 indentChars)
{
    Uint32 n = level * indentChars;

    for (Uint32 i = 0; i < n; i++)
        os << ' ';
}

//------------------------------------------------------------------------------
//
// indentedPrint()
//
//------------------------------------------------------------------------------

void XmlGenerator::indentedPrint(
    PEGASUS_STD(ostream)& os,
    const char* text,
    Uint32 indentChars)
{
    AutoArrayPtr<char> tmp(strcpy(new char[strlen(text) + 1], text));

    XmlParser parser(tmp.get());
    XmlEntry entry;
    Stack<const char*> stack;

    while (parser.next(entry))
    {
        switch (entry.type)
        {
            case XmlEntry::XML_DECLARATION:
            {
                _indent(os, stack.size(), indentChars);

                os << "<?" << entry.text << " ";
                _printAttributes(
                    os, entry.attributes, entry.attributeCount);
                os << "?>";
                break;
            }

            case XmlEntry::START_TAG:
            {
                _indent(os, stack.size(), indentChars);

                os << "<" << entry.text;

                if (entry.attributeCount)
                    os << ' ';

                _printAttributes(
                    os, entry.attributes, entry.attributeCount);
                os << ">";
                stack.push(entry.text);
                break;
            }

            case XmlEntry::EMPTY_TAG:
            {
                _indent(os, stack.size(), indentChars);

                os << "<" << entry.text << " ";
                _printAttributes(
                    os, entry.attributes, entry.attributeCount);
                os << "/>";
                break;
            }

            case XmlEntry::END_TAG:
            {
                if (!stack.isEmpty() && strcmp(stack.top(), entry.text) == 0)
                    stack.pop();

                _indent(os, stack.size(), indentChars);

                os << "</" << entry.text << ">";
                break;
            }

            case XmlEntry::COMMENT:
            {
                _indent(os, stack.size(), indentChars);
                os << "<!--";
                _appendSpecial(os, entry.text);
                os << "-->";
                break;
            }

            case XmlEntry::CONTENT:
            {
                _indent(os, stack.size(), indentChars);
                _appendSpecial(os, entry.text);
                break;
            }

            case XmlEntry::CDATA:
            {
                _indent(os, stack.size(), indentChars);
                os << "<![CDATA[" << entry.text << "]]>";
                break;
            }

            case XmlEntry::DOCTYPE:
            {
                _indent(os, stack.size(), indentChars);
                os << "<!DOCTYPE...>";
                break;
            }
        }

        os << PEGASUS_STD(endl);
    }
}

PEGASUS_NAMESPACE_END