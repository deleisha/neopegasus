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
// Author:      Adrian Schuur, schuur@de.ibm.com
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>

#include "JMPIProviderManager.h"

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

extern "C" PEGASUS_EXPORT ProviderManager * PegasusCreateProviderManager(
   const String & providerManagerName)
{
#ifdef PEGASUS_DEBUG
    PEGASUS_STD(cerr)<<"--- PegasusCreateProviderManager ("
                     <<providerManagerName<<")"<<PEGASUS_STD(endl);
#endif

    if (  String::equalNoCase(providerManagerName, "JMPI")
       || String::equalNoCase(providerManagerName, "JMPIExperimental")
       )
    {
#ifdef PEGASUS_DEBUG
        PEGASUS_STD(cerr)<<"--- JMPI Provider Manager activated"
                         <<PEGASUS_STD(endl);
#endif

        return new JMPIProviderManager ();
    }

    return 0;
}


const char *ifcNames[] = {"JMPI", "JMPIExperimental", NULL};
const char *ifcVersionsJMPI[] = {"1.0.0", "2.0.0", "2.2.0", NULL};
const char *ifcVersionsJMPIExp[] = {"0.0.1", NULL};
const char *ifcVersionsNone[] = {NULL};

///////////////////////////////////////////////////////////////////////////////
extern "C" PEGASUS_EXPORT const char ** getProviderManagerInterfaceNames()
{
    return ifcNames;
}


///////////////////////////////////////////////////////////////////////////////
extern "C" PEGASUS_EXPORT const char ** getProviderManagerInterfaceVersions(
    const char *providerManagerName)
{
    if (Pegasus::String::equalNoCase(String(providerManagerName), "JMPI"))
    {
        return ifcVersionsJMPI;
    }
    else if (Pegasus::String::equalNoCase(String(providerManagerName), 
        "JMPIExperimental"))
    {
        return ifcVersionsJMPIExp;
    }
    return ifcVersionsNone;
}


///////////////////////////////////////////////////////////////////////////////
PEGASUS_GET_VERSION_FUNC; 


