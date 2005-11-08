//%2005////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002 BMC Software; Hewlett-Packard Development
// Company, L.P.; IBM Corp.; The Open Group; Tivoli Systems.
// Copyright (c) 2003 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation, The Open Group.
// Copyright (c) 2004 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation; VERITAS Software Corporation; The Open Group.
// Copyright (c) 2005 Hewlett-Packard Development Company, L.P.; IBM Corp.;
// EMC Corporation; VERITAS Software Corporation; The Open Group.
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

#if !defined(Pegasus_ValueInline_cxx)
# if !defined(PEGASUS_INTERNALONLY) || defined(PEGASUS_DISABLE_INTERNAL_INLINES)
#   define Pegasus_ValueInline_h
# endif
#endif

#ifndef Pegasus_ValueInline_h
#define Pegasus_ValueInline_h

#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/CIMValueRep.h>

#ifdef Pegasus_ValueInline_cxx
# define PEGASUS_CIM_VALUE_INLINE /* empty */
#else
# define PEGASUS_CIM_VALUE_INLINE inline
#endif

PEGASUS_NAMESPACE_BEGIN

PEGASUS_CIM_VALUE_INLINE
CIMValue::CIMValue() : _rep(&CIMValueRep::_emptyRep)
{
}

PEGASUS_CIM_VALUE_INLINE
CIMValue::CIMValue(const CIMValue& x)
{
    CIMValueRep::ref(_rep = x._rep);
}

PEGASUS_CIM_VALUE_INLINE
CIMValue::~CIMValue()
{
    CIMValueRep::unref(_rep);
}

PEGASUS_CIM_VALUE_INLINE
Boolean CIMValue::isArray() const
{
    return _rep->isArray;
}

PEGASUS_CIM_VALUE_INLINE
Boolean CIMValue::isNull() const
{
    return _rep->isNull;
}

PEGASUS_CIM_VALUE_INLINE
CIMType CIMValue::getType() const
{
    return CIMType(_rep->type);
}

PEGASUS_NAMESPACE_END

#endif /* Pegasus_ValueInline_h */
