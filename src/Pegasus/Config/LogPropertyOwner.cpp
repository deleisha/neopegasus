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
// Author: Nag Boranna (nagaraja_boranna@hp.com)
//
// Modified By: Yi Zhou (yi_zhou@hp.com)
//
// Modified By: Dave Rosckes (rosckes@us.ibm.com)
//
//%/////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// 
// This file has implementation for the log property owner class.
//
///////////////////////////////////////////////////////////////////////////////

#include "LogPropertyOwner.h"
#include <Pegasus/Common/Logger.h>


PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////
//  LogPropertyOwner
///////////////////////////////////////////////////////////////////////////////

static struct ConfigPropertyRow properties[] =
{
    {"logdir", "./logs", 1, 0, 0, 1},
    {"logLevel", "INFORMATION", 1, 0, 0, 1}
};

const Uint32 NUM_PROPERTIES = sizeof(properties) / sizeof(properties[0]);


/** Constructors  */
LogPropertyOwner::LogPropertyOwner()
{
    _logdir      = new ConfigProperty;
    _logLevel    = new ConfigProperty;
}

/** Destructor  */
LogPropertyOwner::~LogPropertyOwner()
{
    delete _logdir;
    delete _logLevel;
}


/**
Initialize the config properties.
*/
void LogPropertyOwner::initialize()
{
    for (Uint32 i = 0; i < NUM_PROPERTIES; i++)
    {
        //
        // Initialize the properties with default values
        //
        if (String::equalNoCase(properties[i].propertyName, "logdir"))
        {
            _logdir->propertyName = properties[i].propertyName;
            _logdir->defaultValue = properties[i].defaultValue;
            _logdir->currentValue = properties[i].defaultValue;
            _logdir->plannedValue = properties[i].defaultValue;
            _logdir->dynamic = properties[i].dynamic;
            _logdir->domain = properties[i].domain;
            _logdir->domainSize = properties[i].domainSize;
            _logdir->externallyVisible = properties[i].externallyVisible;
        }
        else if (String::equalNoCase(properties[i].propertyName, "logLevel"))
        {
            _logLevel->propertyName = properties[i].propertyName;
            _logLevel->defaultValue = properties[i].defaultValue;
            _logLevel->currentValue = properties[i].defaultValue;
            _logLevel->plannedValue = properties[i].defaultValue;
            _logLevel->dynamic = properties[i].dynamic;
            _logLevel->domain = properties[i].domain;
            _logLevel->domainSize = properties[i].domainSize;
            _logLevel->externallyVisible = properties[i].externallyVisible;

	    Logger::setlogLevelMask(_logLevel->currentValue);
        }
    }
}

struct ConfigProperty* LogPropertyOwner::_lookupConfigProperty(
    const String& name)
{
    if (String::equalNoCase(_logdir->propertyName, name))
    {
        return _logdir;
    }
    else if (String::equalNoCase(_logLevel->propertyName, name))
    {
        return _logLevel;
    }
    else
    {
        throw UnrecognizedConfigProperty(name);
    }
}

/** 
Get information about the specified property.
*/
void LogPropertyOwner::getPropertyInfo(
    const String& name, 
    Array<String>& propertyInfo)
{
    propertyInfo.clear();

    struct ConfigProperty* configProperty = _lookupConfigProperty(name);

    propertyInfo.append(configProperty->propertyName);
    propertyInfo.append(configProperty->defaultValue);
    propertyInfo.append(configProperty->currentValue);
    propertyInfo.append(configProperty->plannedValue);
    if (configProperty->dynamic)
    {
        propertyInfo.append(STRING_TRUE);
    }
    else
    {
        propertyInfo.append(STRING_FALSE);
    }
    if (configProperty->externallyVisible)
    {
        propertyInfo.append(STRING_TRUE);
    }
    else
    {
        propertyInfo.append(STRING_FALSE);
    }
}

/** 
Get default value of the specified property.
*/
const String LogPropertyOwner::getDefaultValue(const String& name)
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);
    return configProperty->defaultValue;
}

/** 
Get current value of the specified property.
*/
const String LogPropertyOwner::getCurrentValue(const String& name)
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);
    return configProperty->currentValue;
}

/** 
Get planned value of the specified property.
*/
const String LogPropertyOwner::getPlannedValue(const String& name)
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);
    return configProperty->plannedValue;
}


/** 
Init current value of the specified property to the specified value.
*/
void LogPropertyOwner::initCurrentValue(
    const String& name, 
    const String& value)
{
    if(String::equalNoCase(_logLevel->propertyName,name))
    {
	_logLevel->currentValue = value;
	Logger::setlogLevelMask(_logLevel->currentValue);
    }
    else
    {
	struct ConfigProperty* configProperty = _lookupConfigProperty(name);
	configProperty->currentValue = value;
    }
}


/** 
Init planned value of the specified property to the specified value.
*/
void LogPropertyOwner::initPlannedValue(
    const String& name, 
    const String& value)
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);
    configProperty->plannedValue = value;
}

/** 
Update current value of the specified property to the specified value.
*/
void LogPropertyOwner::updateCurrentValue(
    const String& name, 
    const String& value) 
{
    //
    // make sure the property is dynamic before updating the value.
    //
    if (!isDynamic(name))
    {
        throw NonDynamicConfigProperty(name); 
    }

    //
    // Since the validations done in initCurrrentValue are sufficient and 
    // no additional validations required for update, we will call 
    // initCurrrentValue.
    //
    initCurrentValue(name, value);
}


/** 
Update planned value of the specified property to the specified value.
*/
void LogPropertyOwner::updatePlannedValue(
    const String& name, 
    const String& value)
{
    //
    // Since the validations done in initPlannedValue are sufficient and 
    // no additional validations required for update, we will call 
    // initPlannedValue.
    //
    initPlannedValue(name, value);
}

/** 
Checks to see if the given value is valid or not.
*/
Boolean LogPropertyOwner::isValid(const String& name, const String& value)
{
    if (String::equalNoCase(_logLevel->propertyName, name))
    {
	//
	// Check if the logLevel is valid
	//
	if (!Logger::isValidlogLevel(value))
	{
	    throw InvalidPropertyValue(name, value);
        }

        return true;
    }

    // ATTN: Add validation code
    return 1;
}

/** 
Checks to see if the specified property is dynamic or not.
*/
Boolean LogPropertyOwner::isDynamic(const String& name)
{
    struct ConfigProperty* configProperty = _lookupConfigProperty(name);
    return configProperty->dynamic;
}


PEGASUS_NAMESPACE_END
