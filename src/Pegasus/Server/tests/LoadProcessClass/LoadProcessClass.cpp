//%/////////////////////////////////////////////////////////////////////////////
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
//==============================================================================
//
// Author: Mike Brasher (mbrasher@bmc.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <Pegasus/Server/ProviderTable.h>
#include <Pegasus/Repository/CIMRepository.h>

using namespace Pegasus;
using namespace std;

const char NAMESPACE[] = "root/cimv20";
const char ROOTNAMESPACE[] = "root";

int main(int argc, char** argv)
{
    if (argc != 2)
    {
	cerr << "Usage: " << argv[0] << " repository-root" << endl;
	exit(1);
    }

    try
    {
	CIMRepository r(argv[1]);

	try
	{
	    r.deleteClass(NAMESPACE, "Process");
	    r.deleteClass(NAMESPACE, "__Namespace");
	    r.deleteClass(NAMESPACE, "PEG_Instance");
	}
	catch (Exception&)
	{
	    // Ignore no such class error and namespace already exists:
	}

	CIMClass c("Process");

	c.addQualifier(CIMQualifier("provider", "MyProvider"));

	c.addProperty(CIMProperty("pid", Uint32(0))
	    .addQualifier(CIMQualifier("key", true)));

	c.addProperty(CIMProperty("name", Uint32(0)));
	c.addProperty(CIMProperty("age", Uint32(0)));

	r.createClass(NAMESPACE, c);


	CIMClass d("PEG_Instance");


	d.addProperty(CIMProperty("pid", Uint32(0))
	    .addQualifier(CIMQualifier("key", true)));

	d.addProperty(CIMProperty("name", Uint32(0)));
	d.addProperty(CIMProperty("age", Uint32(0)));

	r.createClass(NAMESPACE, d);


	// cout << "Created Process class with MyProvider Qualifier" <<endl;

	// Create the __NameSpace Class in /root/CIMv20 namespace
	//delete the class if it exists
	try
	{
	    r.deleteClass(NAMESPACE, "__Namespace");
	}
	catch (Exception&)
	{
	    // Ignore no such class error
	}

	// Now create the __Namespace class
	// with name property key.
	// Note that this requires qualifier definition in Root
	
	CIMClass cn("__Namespace");

	cn.addQualifier(CIMQualifier("provider", "__NamespaceProvider"));
	cn.addQualifier(CIMQualifier("Description", 
				     "Namespace manipulation"));

	cn.addProperty(CIMProperty("name", "")
	    .addQualifier(CIMQualifier("key", true))
	     .addQualifier(CIMQualifier("Description", 
					"Namespace Name")));

	// Put it into the normal namespace
	r.createClass(NAMESPACE, cn);
        // cout << "Created __Namespace class with Provider Qualfier" << endl;



	// Create the __NameSpace Class in /root namespace
	//delete the class if it exists
	try
	{
	    r.deleteClass(ROOTNAMESPACE, "__Namespace");
	}
	catch (Exception&)
	{
	    // Ignore no such class error
	}

	// Now create the __Namespace class
	// with name property key.
	// Note that this requires qualifier definition in Root

	r.createClass(ROOTNAMESPACE, cn);
	// cout << "Created __Namespace class with Provider Qualfier" << endl;
    }
    catch(Exception& e)
    {
	std::cerr << "Error: " << e.getMessage() << std::endl;
	exit(1);
    }

    cout << "+++++ passed all tests" << endl;

    return 0;
}
