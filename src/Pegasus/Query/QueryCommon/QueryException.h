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

#ifndef Pegasus_QueryException_h
#define Pegasus_QueryException_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Query/QueryCommon/Linkage.h>
#include <Pegasus/Common/Exception.h>
//#include <Pegasus/Common/MessageLoader.h>

PEGASUS_NAMESPACE_BEGIN

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES

class PEGASUS_QUERYCOMMON_LINKAGE QueryException : public Exception
{
public:
    static const char MSG[];
    static const char KEY[];

    QueryException(const String& msg)
        : Exception(MessageLoaderParms(KEY, MSG, msg))
    {
    }

    QueryException(const ContentLanguageList& langs, const String& msg)
        : Exception(msg)
    {
        setContentLanguages(langs);
    }

    QueryException(
        const ContentLanguageList& langs,
        const MessageLoaderParms& parms)
        : Exception(parms)
    {
        setContentLanguages(langs);
    }

    QueryException(const MessageLoaderParms& parms)
        : Exception(parms)
    {
    }
};

class PEGASUS_QUERYCOMMON_LINKAGE QueryLanguageInvalidException :
    public QueryException
{
public:
    static const char MSG[];
    static const char KEY[];

    QueryLanguageInvalidException(const String& msg)
        : QueryException(MessageLoaderParms(KEY, MSG, msg))
    {
    }

    QueryLanguageInvalidException(const MessageLoaderParms& parms)
        : QueryException(parms)
    {
    }
};

class PEGASUS_QUERYCOMMON_LINKAGE QueryValidationException :
    public QueryException
{
public:
    static const char MSG[];
    static const char KEY[];

    QueryValidationException(const String& msg)
        : QueryException(MessageLoaderParms(KEY, MSG, msg))
    {
    }

    QueryValidationException(const MessageLoaderParms& parms)
        : QueryException(parms)
    {
    }
};

class PEGASUS_QUERYCOMMON_LINKAGE QueryMissingPropertyException :
    public QueryValidationException
{
public:
    static const char MSG[];
    static const char KEY[];

    QueryMissingPropertyException(const String& msg)
        : QueryValidationException(MessageLoaderParms(KEY, MSG, msg))
    {
    }

    QueryMissingPropertyException(const MessageLoaderParms& parms)
        : QueryValidationException(parms)
    {
    }
};

class PEGASUS_QUERYCOMMON_LINKAGE QueryParseException : public QueryException
{
public:
    static const char MSG[];
    static const char KEY[];

    QueryParseException(const String& msg)
        : QueryException(MessageLoaderParms(KEY, MSG, msg))
    {
    }

    QueryParseException(const MessageLoaderParms& parms)
        : QueryException(parms)
    {
    }
};

class PEGASUS_QUERYCOMMON_LINKAGE QueryRuntimeException : public QueryException
{
public:
    static const char MSG[];
    static const char KEY[];

    QueryRuntimeException(const String& msg)
        : QueryException(MessageLoaderParms(KEY, MSG, msg))
    {
    }

    QueryRuntimeException(const MessageLoaderParms& parms)
        : QueryException(parms)
    {
    }
};

class PEGASUS_QUERYCOMMON_LINKAGE QueryRuntimePropertyException :
    public QueryRuntimeException
{
public:
    static const char MSG[];
    static const char KEY[];

    QueryRuntimePropertyException(const String& msg)
        : QueryRuntimeException(MessageLoaderParms(KEY, MSG, msg))
    {
    }

    QueryRuntimePropertyException(const MessageLoaderParms& parms)
        : QueryRuntimeException(parms)
    {
    }
};

class PEGASUS_QUERYCOMMON_LINKAGE CQLSyntaxErrorException :
    public QueryParseException
{
public:
    static const char MSG[];
    static const char KEY[];

    CQLSyntaxErrorException(
        const String& msg,
        Uint32 token,
        Uint32 position,
        const String& rule)
        : QueryParseException(
              MessageLoaderParms(KEY, MSG, msg, token, position, rule))
    {
    }

    CQLSyntaxErrorException(const MessageLoaderParms& parms)
        : QueryParseException(parms)
    {
    }
};

class PEGASUS_QUERYCOMMON_LINKAGE CQLIdentifierParseException :
    public QueryParseException
{
public:
    static const char MSG[];
    static const char KEY[];

    CQLIdentifierParseException(const String& msg)
        : QueryParseException(MessageLoaderParms(KEY, MSG, msg))
    {
    }

    CQLIdentifierParseException(const MessageLoaderParms& parms)
        : QueryParseException(parms)
    {
    }
};

class PEGASUS_QUERYCOMMON_LINKAGE CQLChainedIdParseException :
    public QueryParseException
{
public:
    static const char MSG[];
    static const char KEY[];

    CQLChainedIdParseException(const String& msg)
        : QueryParseException(MessageLoaderParms(KEY, MSG, msg))
    {
    }

    CQLChainedIdParseException(const MessageLoaderParms& parms)
        : QueryParseException(parms)
    {
    }
};

class PEGASUS_QUERYCOMMON_LINKAGE CQLRuntimeException :
    public QueryRuntimeException
{
public:
    static const char MSG[];
    static const char KEY[];

    CQLRuntimeException(const String& msg)
        : QueryRuntimeException(MessageLoaderParms(KEY, MSG, msg))
    {
    }

    CQLRuntimeException(const MessageLoaderParms& parms)
        : QueryRuntimeException(parms)
    {
    }
};

class PEGASUS_QUERYCOMMON_LINKAGE CQLNullContagionException :
    public CQLRuntimeException
{
public:
    static const char MSG[];
    static const char KEY[];

    CQLNullContagionException(const String& msg)
        : CQLRuntimeException(MessageLoaderParms(KEY, MSG, msg))
    {
    }

    CQLNullContagionException(const MessageLoaderParms& parms)
        : CQLRuntimeException(parms)
    {
    }
};

// Macros to convert non QueryExceptions to QueryExceptions

#define PEGASUS_QUERY_EXCEPTION(LANGS, MESSAGE) QueryException(LANGS, MESSAGE)

#define PEGASUS_QUERY_EXCEPTION_LANG(LANGS, PARMS) QueryException(LANGS, PARMS)

#endif

PEGASUS_NAMESPACE_END

#endif
