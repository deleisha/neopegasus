//%/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002 BMC Software, Hewlett-Packard Company, IBM,
// The Open Group, Tivoli Systems
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
// Author: Mike Brasher (mbrasher@bmc.com)
//
// Modified By:
//         Brian Bobryk (Brian.Bobryk@compaq.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_Platform_TRU64_ALPHA_DECCXX_h
#define Pegasus_Platform_TRU64_ALPHA_DECCXX_h

#include <cstdlib>
#include <Pegasus/Common/ConfigUnix.h>

#define PEGASUS_OS_TRU64

#define PEGASUS_ARCHITECTURE_ALPHA

#define PEGASUS_COMPILER_DECCXX

#define PEGASUS_UINT64 unsigned long long

#define PEGASUS_SINT64 long long

#define PEGASUS_LLONG_MAX 9223372036854775807LL
#define PEGASUS_LLONG_MIN (-PEGASUS_LLONG_MAX - 1LL)

#define PEGASUS_ULLONG_MAX 18446744073709551615ULL

#define PEGASUS_HAVE_NAMESPACES

#define PEGASUS_HAVE_NO_STD

#define PEGASUS_HAVE_EXPLICIT

#define PEGASUS_HAVE_MUTABLE

#define PEGASUS_HAVE_FOR_SCOPE

// #define PEGASUS_HAVE_TEMPLATE_SPECIALIZATION

#define PEGASUS_HAVE_BOOLEAN

// #define PEGASUS_EXPLICIT_INSTANTIATION

#define PEGASUS_SUPPRESS_UNREACHABLE_STATEMENTS

#define PEGASUS_SUPPORTS_DYNLIB

#endif /* Pegasus_Platform_TRU64_ALPHA_DECCXX_h */
