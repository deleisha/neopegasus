//BEGIN_LICENSE
//
// Copyright (c) 2000 The Open Group, BMC Software, Tivoli Systems, IBM
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//END_LICENSE
//BEGIN_HISTORY
//
// Author: Michael E. Brasher
//
// $Log: Selector.h,v $
// Revision 1.5  2001/04/11 04:53:10  mike
// Porting
//
// Revision 1.4  2001/04/11 04:20:39  mike
// new
//
// Revision 1.3  2001/04/08 08:28:20  mike
// Added more windows channel implementation code.
//
// Revision 1.2  2001/04/08 05:06:06  mike
// New Files for Channel Implementation
//
// Revision 1.1  2001/04/08 04:46:11  mike
// Added new selector class for windows
//
//
//END_HISTORY

#ifndef Pegasus_Selector_h 
#define Pegasus_Selector_h 

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Array.h>

PEGASUS_NAMESPACE_BEGIN

class SelectorHandler;
struct SelectorRep;

class PEGASUS_COMMON_LINKAGE Selector
{
public:

    enum Reason { READ = 1, WRITE = 2, EXCEPTION = 4 };

    Selector();

    ~Selector();

    Boolean select(Uint32 milliseconds);

    Boolean addHandler(
	Sint32 desc, 
	Uint32 reasons,
	SelectorHandler* handler);

    Boolean removeHandler(SelectorHandler* handler);

private:

    Uint32 _findEntry(Sint32 desc) const;

    struct Entry
    {
	Sint32 desc;
	SelectorHandler* handler;
    };

    Array<Entry> _entries;
    SelectorRep* _rep;
};

class PEGASUS_COMMON_LINKAGE SelectorHandler
{
public:

    virtual ~SelectorHandler();

    virtual Boolean handle(Sint32 desc, Uint32 reasons) = 0;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Selector_h */
