//%////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 BMC Software, Hewlett-Packard Company, IBM,
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
//=============================================================================
//
// Author: Al Stone <ahs3@fc.hp.com>
//         Christopher Neufeld <neufeld@linuxcare.com>
//
// Modified By: David Kennedy       <dkennedy@linuxcare.com>
//              Christopher Neufeld <neufeld@linuxcare.com>
//              Al Stone            <ahs3@fc.hp.com>
//
//%////////////////////////////////////////////////////////////////////////////


#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/Logger.h>
#include "OperatingSystemProvider.h"
#include "OperatingSystem.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

#define DEBUG(X) // cout << "OperatingSystemProvider" <<  X << endl;

OperatingSystemProvider::OperatingSystemProvider(void)
{
}

OperatingSystemProvider::~OperatingSystemProvider(void)
{
}

void
OperatingSystemProvider::getInstance(const OperationContext& context,
				     const CIMReference& ref,
				     const Uint32 flags,
				     const CIMPropertyList& propertyList,
				     ResponseHandler<CIMInstance> &handler)
{
    Array<KeyBinding> keys;
    CIMInstance instance;
    OperatingSystem os;
    String className;
    String csName;
    String name;

    //-- make sure we're working on the right class
    className = ref.getClassName();
    if (!String::equalNoCase(className, STANDARDOPERATINGSYSTEMCLASS) &&
        !String::equalNoCase(className, EXTENDEDOPERATINGSYSTEMCLASS))
        throw CIMException(CIM_ERR_NOT_SUPPORTED);

    if (!os.getCSName(csName))
    {
        throw CIMException(CIM_ERR_FAILED);
    }

    if (!os.getName(name))
    {
        throw CIMException(CIM_ERR_FAILED);
    }

    //-- make sure we're the right instance
    int keyCount;
    String keyName;

    keyCount = 4;
    keys = ref.getKeyBindings();

    if ((unsigned int)keys.size() < (unsigned int)keyCount)
        throw CIMException(CIM_ERR_INVALID_PARAMETER);

    for (unsigned int ii = 0; ii < keys.size(); ii++)
    {
         keyName = keys[ii].getName();

         if (String::equal(keyName, "CSCreationClassName") &&
       	    String::equal(keys[ii].getValue(), "CIM_ComputerSystem"))
         {
              keyCount--;
         }
         else if (String::equal(keyName, "CSName") &&
  	         String::equal(keys[ii].getValue(), csName))
         {
              keyCount--;
         }
         else if (String::equal(keyName, "CreationClassName") &&
 	         String::equal(keys[ii].getValue(), STANDARDOPERATINGSYSTEMCLASS))
         {
              keyCount--;
         }
         else if (String::equal(keyName, "Name") &&
 	         String::equal(keys[ii].getValue(), name))
         {
              keyCount--;
         }
         else
         {
              throw CIMException(CIM_ERR_INVALID_PARAMETER);
         }
     }

     if (keyCount)
     {
        throw CIMException(CIM_ERR_INVALID_PARAMETER);
     }

    DEBUG("losp-> getInstance got the right keys");

    handler.processing();
    
    //-- fill 'er up...
    instance = _build_instance(ref);

    DEBUG("losp-> getInstance built an instance");

    handler.deliver(instance);
    handler.complete();

    DEBUG("losp-> getInstance done");
    return;
}

void
OperatingSystemProvider::enumerateInstances(
      				const OperationContext& context,
			        const CIMReference& ref,
			        const Uint32 flags,
			        const CIMPropertyList& propertyList,
			        ResponseHandler<CIMInstance>& handler)
{
    String className;
    CIMInstance instance;

    className = ref.getClassName();
    
    // only support enumerate on our subclass, CIMOM will call us as 
    // natural part of recursing through subtree on enumerate - if we
    // return on enumerate of our superclass, there would be dups
    if (String::equalNoCase(className, EXTENDEDOPERATINGSYSTEMCLASS))
    {
         handler.processing();
         instance = _build_instance(ref);
         handler.deliver(instance);
         handler.complete();
    }
    else
    {
        throw CIMException(CIM_ERR_NOT_SUPPORTED);
    }
    return;
}

void
OperatingSystemProvider::enumerateInstanceNames(
      				const OperationContext& context,
			  	const CIMReference &ref,
			  	ResponseHandler<CIMReference>& handler )
{
    CIMReference newref;
    String className;

    // only support enumerate on our subclass, CIMOM will call us as 
    // natural part of recursing through subtree on enumerate - if we
    // return on enumerate of our superclass, there would be dups
    className = ref.getClassName();
    if (!String::equalNoCase(className, EXTENDEDOPERATINGSYSTEMCLASS))
    {
        throw CIMException(CIM_ERR_NOT_SUPPORTED);
    }

    // in terms of the class we use, want to set to what was requested 
    newref = _fill_reference(ref.getNameSpace(), className);
// newref = _fill_reference(ref.getNameSpace(), STANDARDOPERATINGSYSTEMCLASS);
    handler.processing();
    handler.deliver(newref);
    handler.complete();

    return;
}

void
OperatingSystemProvider::modifyInstance(
      				const OperationContext& context,
			  	const CIMReference& ref,
			  	const CIMInstance& instanceObject,
			  	const Uint32 flags,
			  	const CIMPropertyList& propertyList,
			  	ResponseHandler<CIMInstance>& handler )
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
}

void
OperatingSystemProvider::createInstance(
      				const OperationContext& context,
			  	const CIMReference& ref,
			  	const CIMInstance& instanceObject,
			  	ResponseHandler<CIMReference>& handler )
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);

}

void
OperatingSystemProvider::deleteInstance(
      				const OperationContext& context,
			  	const CIMReference& ref,
			  	ResponseHandler<CIMInstance>& handler )
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
}

void OperatingSystemProvider::initialize(CIMOMHandle& handle)
{
   _cimomhandle = handle;  // save off for future use

   // call platform-specific routines to get certain values
   
}


void OperatingSystemProvider::terminate(void)
{
}


CIMInstance
OperatingSystemProvider::_build_instance(const CIMReference& objectReference)
{
    CIMInstance instance(objectReference.getClassName());
    OperatingSystem os;
    String className;
    String stringValue;
    Uint16 uint16Value;
    CIMDateTime cimDateTimeValue;
    Sint16 sint16Value;
    Uint32 uint32Value;
    Uint64 uint64Value;
    Boolean booleanValue;

    className = objectReference.getClassName();

    //-- fill in all the blanks
    instance.addProperty(CIMProperty("CSCreationClassName",
 	                 "CIM_ComputerSystem"));

    if (os.getCSName(stringValue))
    {
        instance.addProperty(CIMProperty("CSName", stringValue));
    }

    instance.addProperty(CIMProperty("CreationClassName",
 	                 STANDARDOPERATINGSYSTEMCLASS));

    if (os.getName(stringValue))
    {
        instance.addProperty(CIMProperty("Name", stringValue));
    }

    if (os.getCaption(stringValue))
    {
        instance.addProperty(CIMProperty("Caption", stringValue));
    }

    if (os.getDescription(stringValue))
    {
        instance.addProperty(CIMProperty("Description", stringValue));
    }

    if (os.getInstallDate(cimDateTimeValue))
    {
        instance.addProperty(CIMProperty("InstallDate", cimDateTimeValue));
    }
    
    if (os.getStatus(stringValue))
    {
        instance.addProperty(CIMProperty("Status", stringValue));
    }

    if (os.getOSType(uint16Value))
    {
        instance.addProperty(CIMProperty("OSType", uint16Value));
    }

    if (os.getOtherTypeDescription(stringValue))
    {
        instance.addProperty(CIMProperty("OtherTypeDescription", stringValue));
    }

    if (os.getVersion(stringValue))
    {
        instance.addProperty(CIMProperty("Version", stringValue));
    }

    if (os.getLastBootUpTime(cimDateTimeValue))
    {
        instance.addProperty(CIMProperty("LastBootUpTime", cimDateTimeValue));
    }

    if (os.getLocalDateTime(cimDateTimeValue))
    {
        instance.addProperty(CIMProperty("LocalDateTime", cimDateTimeValue));
    }

    if (os.getCurrentTimeZone(sint16Value))
    {
        instance.addProperty(CIMProperty("CurrentTimeZone", sint16Value));
    }
    
    if (os.getNumberOfLicensedUsers(uint32Value))
    {
        instance.addProperty(CIMProperty("NumberOfLicensedUsers",uint32Value));
    }

    if (os.getNumberOfUsers(uint32Value))
    {
        instance.addProperty(CIMProperty("NumberOfUsers", uint32Value));
    }

    if (os.getNumberOfProcesses(uint32Value))
    {
        instance.addProperty(CIMProperty("NumberOfProcesses", uint32Value));
    }
 
    if (os.getMaxNumberOfProcesses(uint32Value))
    {
        instance.addProperty(CIMProperty("MaxNumberOfProcesses", uint32Value));
    }

    if (os.getTotalSwapSpaceSize(uint64Value))
    {
        instance.addProperty(CIMProperty("TotalSwapSpaceSize", uint64Value));
    }

    if (os.getTotalVirtualMemorySize(uint64Value))
    {
        instance.addProperty(CIMProperty("TotalVirtualMemorySize", uint64Value));
    }

    if (os.getFreeVirtualMemory(uint64Value))
    {
        instance.addProperty(CIMProperty("FreeVirtualMemory", uint64Value));
    }

    if (os.getFreePhysicalMemory(uint64Value))
    {
       instance.addProperty(CIMProperty("FreePhysicalMemory", uint64Value));
    }

    if (os.getTotalVisibleMemorySize(uint64Value))
    {
       instance.addProperty(CIMProperty("TotalVisibleMemorySize", uint64Value));
    }

    if (os.getSizeStoredInPagingFiles(uint64Value))
    {
        instance.addProperty(CIMProperty("SizeStoredInPagingFiles", uint64Value));
    }

    if (os.getFreeSpaceInPagingFiles(uint64Value))
    {
        instance.addProperty(CIMProperty("FreeSpaceInPagingFiles", uint64Value));
    }

    if (os.getMaxProcessMemorySize(uint64Value))
    {
        instance.addProperty(CIMProperty("MaxProcessMemorySize", uint64Value));
    }
    
    if (os.getDistributed(booleanValue))
    {
        instance.addProperty(CIMProperty("Distributed", booleanValue));
    }

    if (os.getMaxProcsPerUser(uint32Value))
    {
        instance.addProperty(CIMProperty("MaxProcessesPerUser", uint32Value));
    }

    if (String::equalNoCase(className, EXTENDEDOPERATINGSYSTEMCLASS))
    {
        if (os.getSystemUpTime(uint64Value))
        {
            instance.addProperty(CIMProperty("SystemUpTime",uint64Value ));
        }

        if (os.getOperatingSystemCapability(stringValue))
        {
            instance.addProperty(CIMProperty("OperatingSystemCapability",
                                              stringValue));
        }
    }
    return instance;
}

CIMReference
OperatingSystemProvider::_fill_reference(const String &nameSpace,
				         const String &className)
{
    Array<KeyBinding> keys;
    OperatingSystem os;
    String csName;
    String name;

    if (!os.getCSName(csName))
    {
// ATTN-SLC-P2-19-Apr-02: workaround for Windows key BZ#42 - remove BZ#46
       csName = "Unknown";  // remove when not needed for Windows provider
    }

    if (!os.getName(name))
    {
// ATTN-SLC-P2-19-Apr-02: workaround for Windows key BZ#42 - remove BZ#46
       name = "Unknown";    // remove when not needed for Window provider
    }

    keys.append(KeyBinding("CSCreationClassName",
 	                   "CIM_ComputerSystem",
			   KeyBinding::STRING));
    keys.append(KeyBinding("CSName", csName, KeyBinding::STRING));
    keys.append(KeyBinding("CreationClassName", STANDARDOPERATINGSYSTEMCLASS, 
        KeyBinding::STRING));
    keys.append(KeyBinding("Name", name, KeyBinding::STRING));

    return CIMReference(csName, nameSpace, className, keys);
}


void OperatingSystemProvider::invokeMethod(
      				const OperationContext& context,
    				const CIMReference& objectReference,
				const String& methodName,
				const Array<CIMParamValue>& inParameters,
				Array<CIMParamValue>& outParameters,
				ResponseHandler<CIMValue>& handler)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED);
}

PEGASUS_NAMESPACE_END

