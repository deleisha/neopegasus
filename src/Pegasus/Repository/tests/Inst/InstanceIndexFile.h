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
// Author: Mike Brasher
//
// $Log: InstanceIndexFile.h,v $
// Revision 1.1  2001/02/13 01:28:35  mike
// new
//
//
//END_HISTORY

#ifndef Pegasus_InstanceIndexFile_h
#define Pegasus_InstanceIndexFile_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Exception.h>

PEGASUS_NAMESPACE_BEGIN

class CorruptInstanceIndexFile : public Exception 
{ 
public:
    CorruptInstanceIndexFile() : Exception("CorruptInstanceIndexFile") { }
};

class InstanceAlreadyExists : public Exception 
{ 
public:
    InstanceAlreadyExists() : Exception("InstanceAlreadyExists") { }
};

/** This class manages access to an "instance index file" which maps
    instance names to indexes. Lines of this file have the following form:

    <pre>
	ClassName.key1=value1,...,keyN=valueN Index
    </pre>

    Where the instance name is a CIM style object path and the index is a 
    positive integer (non-zero). Here's an example:

    <pre>
	Employee.ssn=444332222 1
	Employee.ssn=555667777 3
    </pre>


    Each instance in Pegasus is represented as a disk file containing the
    CIM/XML encoding of the instance. The name of the file has this form:

    <pre>
	ClassName.Index
    </pre>

    Here's an example:

    <pre>
	Employee.3
    </pre>

    An instance can be obtained from an instance name by using the index file
    to map the instance name to an index and then forming the file name from 
    the class name and index.

    Methods are provided for managing the instance index file: adding,
    removing, and modifying instance names.
*/
class InstanceIndexFile
{
public:
    
    static Uint32 lookupIndex(
	const String& path, 
	const String& instanceName);

    /** Inserts a new entry into the instance file. The index is returned.
	are in sorted order so that they can be compared trivially. This 
	method assumes that the keys in the instance name are in sorted order.
	This must be done prior to calling the rouine.

	throws InstanceAlreadyExists if the instance name already appears
	in the file. 

	throws CorruptInstanceIndexFile if instance file is corrupt.
    */
    static Uint32 insertEntry(
	const String& path, 
	const String& instanceName);
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_InstanceIndexFile_h */
