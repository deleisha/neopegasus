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
// Author: Nitin Upasani, Hewlett-Packard Company (Nitin_Upasani@hp.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <cassert>
#include <stdlib.h>
#include <stdio.h>
#include <Pegasus/Client/CIMClient.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

const CIMNamespaceName NAMESPACE = CIMNamespaceName ("root/cimv2");

void TestCreateFilterInstances(CIMClient& client)
{
    CIMClass filterClass = client.getClass(NAMESPACE, 
        CIMName ("CIM_IndicationFilter"), false);
    
    for (Uint8 i = 1; i <= 10; i++)
    {
	CIMInstance filterInstance(CIMName ("CIM_IndicationFilter"));
        filterInstance.addProperty(CIMProperty
            (CIMName ("SystemCreationClassName"), 
                                          String("CIM_UnitaryComputerSystem")));
	filterInstance.addProperty(CIMProperty(CIMName ("SystemName"), 
                                               String("bigbasin.hp.com")));
	filterInstance.addProperty(CIMProperty(CIMName ("CreationClassName"), 
                                               String("CIM_IndicationFilter")));
	switch (i)
	{
	    case 1:
		filterInstance.addProperty(CIMProperty(CIMName ("Name"),
                                                       String("Filter1")));
	        filterInstance.addProperty (CIMProperty(CIMName ("Query"), 
                    String("SELECT NewAlertType FROM Sample_HelloWorldIndication WHERE NewAlertType = 5")));
		break;
	    case 2:
		filterInstance.addProperty(CIMProperty(CIMName ("Name"),
                                                       String("Filter2")));
	        filterInstance.addProperty (CIMProperty(CIMName ("Query"), 
                    String("SELECT IndicationTime, ProbableCause, ProbableCauseDescription, EventID, EventTime FROM CIM_ProcessIndication")));
		break;
	    case 3:
		filterInstance.addProperty(CIMProperty(CIMName ("Name"),
                                                       String("Filter3")));
	        filterInstance.addProperty (CIMProperty(CIMName ("Query"), 
                    String("SELECT ProbableCause FROM CIM_ProcessIndication")));
		break;
	    case 4:
		filterInstance.addProperty(CIMProperty(CIMName ("Name"),
                                                       String("Filter4")));
	        filterInstance.addProperty (CIMProperty(CIMName ("Query"), 
                    String("SELECT TestProperty, ProbableCause, ProbableCauseDescription FROM CIM_ProcessIndication")));
		break;
	    case 5:
		filterInstance.addProperty(CIMProperty(CIMName ("Name"),
                                                       String("Filter5")));
	        filterInstance.addProperty (CIMProperty (CIMName ("Query"), 
                    String("SELECT ProbableCause, ProbableCauseDescription FROM CIM_ProcessIndication WHERE EventID IS NOT NULL")));
		break;
	    case 6:
		filterInstance.addProperty(CIMProperty(CIMName ("Name"),
                                                       String("Filter6")));
	        filterInstance.addProperty (CIMProperty (CIMName ("Query"), 
                    String("SELECT * FROM CIM_ProcessIndication WHERE EventID IS NOT NULL")));
		break;
	    case 7:
		filterInstance.addProperty(CIMProperty(CIMName ("Name"),
                                                       String("Filter7")));
	        filterInstance.addProperty (CIMProperty (CIMName ("Query"), 
                    String("SELECT * FROM CIM_AlertIndication")));
		break;
	    case 8:
		filterInstance.addProperty(CIMProperty(CIMName ("Name"),
                                                       String("Filter8")));
	        filterInstance.addProperty (CIMProperty (CIMName ("Query"), 
                    String("SELECT * FROM CIM_AlertIndication")));
		break;
	    case 9:
		filterInstance.addProperty(CIMProperty(CIMName ("Name"),
                                                       String("Filter9")));
	        filterInstance.addProperty (CIMProperty (CIMName ("Query"), 
                    String("SELECT * FROM CIM_AlertIndication")));
		break;
	    case 10:
		filterInstance.addProperty(CIMProperty(CIMName ("Name"),
                                                       String("Filter10")));
	        filterInstance.addProperty (CIMProperty (CIMName ("Query"), 
                    String("SELECT NewAlertType FROM Sample_HelloWorldIndication WHERE NewAlertType = 5")));
		break;
	}

	filterInstance.addProperty(CIMProperty(CIMName ("QueryLanguage"), 
            String("WQL1")));
	CIMObjectPath filterRef = filterInstance.buildPath(filterClass);
	client.createInstance(NAMESPACE, filterInstance);
    }
}

int main(int argc, char** argv)
{
    CIMClient client;
    client.connect("localhost", 5988, String::EMPTY, String::EMPTY);

    try 
    {
        TestCreateFilterInstances(client);
    }
    catch (Exception e)
    {
        PEGASUS_STD (cout) << "Exception: " << e.getMessage () 
                           << PEGASUS_STD (endl);
        PEGASUS_STD (cout) << "+++++ create filter instances failed" 
                           << PEGASUS_STD (endl);
        exit (-1);
    }

    PEGASUS_STD (cout) << "+++++ filter instances created" 
                       << PEGASUS_STD (endl);
    exit (0);
}
