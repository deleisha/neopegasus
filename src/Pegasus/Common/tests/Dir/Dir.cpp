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
#include <iostream>
#include <Pegasus/Common/Dir.h>
#include <Pegasus/Common/Array.h>

using namespace Pegasus;
using namespace std;

void test01()
{
    Array<String> names;

    for (Dir dir("testdir"); dir.more(); dir.next())
    {
	String name = dir.getName();

	if (String::equal(name, ".") || String::equal(name, "..") 
	    || String::equal(name, "CVS"))
	{
	    continue;
	}

	names.append(name);
    }

    BubbleSort(names);
    assert(names.getSize() == 3);
    assert(String::equal(names[0], "a"));
    assert(String::equal(names[1], "b"));
    assert(String::equal(names[2], "c"));
}

int main()
{
    try
    {
	test01();
    }
    catch (Exception& e)
    {
	cout << e.getMessage() << endl;
	exit(1);
    }

    try 
    {
	Dir dir("noSuchDirectory");
    }
    catch (CannotOpenDirectory&)
    {
	cout << "+++++ passed all tests" << endl;
	exit(0);
    }

    assert(0);
    exit(1);

    return 0;
}
