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
// Author: Chip Vincent (cvincent@us.ibm.com)
//
// Modified By: Roger Kumpf, Hewlett-Packard Company (roger_kumpf@hp.com)
//              Nitin Upasani, Hewlett-Packard Company (Nitin_Upasani@hp.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_ProviderFacade_h
#define Pegasus_ProviderFacade_h

#include <Pegasus/Common/Config.h>

#include <Pegasus/Provider/CIMBaseProvider.h>
#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <Pegasus/Provider/CIMClassProvider.h>
#include <Pegasus/Provider/CIMAssociationProvider.h>
#include <Pegasus/Provider/CIMPropertyProvider.h>
#include <Pegasus/Provider/CIMMethodProvider.h>
#include <Pegasus/Provider/CIMQueryProvider.h>
#include <Pegasus/Provider/CIMIndicationProvider.h>
#include <Pegasus/Provider/CIMIndicationConsumer.h>

PEGASUS_NAMESPACE_BEGIN

// The ProviderModule class wraps a provider pointer extracted from a provider
// module to ensure it is complete and well behaved. So, regardless of the
// method supported by a "real" provider, it can be placed inside a reliable
// facade with a known interface.
class PEGASUS_SERVER_LINKAGE ProviderFacade :
    public CIMInstanceProvider,
    public CIMClassProvider,
    public CIMAssociationProvider,
    public CIMPropertyProvider,
    public CIMMethodProvider,
    public CIMQueryProvider,
    public CIMIndicationProvider,
    public CIMIndicationConsumer
{
public:
    ProviderFacade(CIMBaseProvider * provider);
    virtual ~ProviderFacade(void);

    // CIMBaseProvider interface
    virtual void initialize(CIMOMHandle & cimom);
    virtual void terminate(void);

    // CIMInstanceProvider interface
    virtual void getInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const Uint32 flags,
        const CIMPropertyList & propertyList,
        ResponseHandler<CIMInstance> & handler);

    virtual void enumerateInstances(
        const OperationContext & context,
        const CIMObjectPath & classReference,
        const Uint32 flags,
        const CIMPropertyList & propertyList,
        ResponseHandler<CIMInstance> & handler);

    virtual void enumerateInstanceNames(
        const OperationContext & context,
        const CIMObjectPath & classReference,
        ResponseHandler<CIMObjectPath> & handler);

    virtual void modifyInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const CIMInstance & instanceObject,
        const Uint32 flags,
        const CIMPropertyList & propertyList,
        ResponseHandler<void> & handler);

    virtual void createInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const CIMInstance & instanceObject,
        ResponseHandler<CIMObjectPath> & handler);

    virtual void deleteInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        ResponseHandler<void> & handler);

    // CIMClassProvider interface
    virtual void getClass(
        const OperationContext & context,
        const CIMObjectPath & classReference,
        const Uint32 flags,
        const CIMPropertyList & propertyList,
        ResponseHandler<CIMClass> & handler);

    virtual void enumerateClasses(
        const OperationContext & context,
        const CIMObjectPath & classReference,
        const Uint32 flags,
        ResponseHandler<CIMClass> & handler);

    virtual void enumerateClassNames(
        const OperationContext & context,
        const CIMObjectPath & classReference,
        const Uint32 flags,
        ResponseHandler<CIMObjectPath> & handler);

    virtual void modifyClass(
        const OperationContext & context,
        const CIMObjectPath & classReference,
        const CIMClass & classObject,
        ResponseHandler<void> & handler);

    virtual void createClass(
        const OperationContext & context,
        const CIMObjectPath & classReference,
        const CIMClass & classObject,
        ResponseHandler<void> & handler);

    virtual void deleteClass(
        const OperationContext & context,
        const CIMObjectPath & classReference,
        ResponseHandler<void> & handler);

    // CIMAssociationProvider interface
    virtual void associators(
        const OperationContext & context,
        const CIMObjectPath & objectName,
        const String & associationClass,
        const String & resultClass,
        const String & role,
        const String & resultRole,
        const Uint32 flags,
        const CIMPropertyList & propertyList,
        ResponseHandler<CIMObject> & handler);

    virtual void associatorNames(
        const OperationContext & context,
        const CIMObjectPath & objectName,
        const String & associationClass,
        const String & resultClass,
        const String & role,
        const String & resultRole,
        ResponseHandler<CIMObjectPath> & handler);

    virtual void references(
        const OperationContext & context,
        const CIMObjectPath & objectName,
        const String & resultClass,
        const String & role,
        const Uint32 flags,
        const CIMPropertyList & propertyList,
        ResponseHandler<CIMObject> & handler);

    virtual void referenceNames(
        const OperationContext & context,
        const CIMObjectPath & objectName,
        const String & resultClass,
        const String & role,
        ResponseHandler<CIMObjectPath> & handler);

    // CIMPropertyProvider interface
    virtual void getProperty(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const String & propertyName,
        ResponseHandler<CIMValue> & handler);

    virtual void setProperty(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const String & propertyName,
        const CIMValue & newValue,
        ResponseHandler<CIMValue> & handler);

    // CIMMethodProviderFacade
    virtual void invokeMethod(
        const OperationContext & context,
        const CIMObjectPath & objectReference,
        const String & methodName,
        const Array<CIMParamValue> & inParameters,
        Array<CIMParamValue> & outParameters,
        ResponseHandler<CIMValue> & handler);

    // CIMQueryProvider interface
    virtual void executeQuery(
        const OperationContext & context,
        const String & nameSpace,
        const String & queryLanguage,
        const String & query,
        ResponseHandler<CIMObject> & handler);

    // CIMIndicationProvider interface
    virtual void enableIndications(
        ResponseHandler<CIMIndication> & handler);

    virtual void disableIndications(void);

    virtual void createSubscription(
        const OperationContext & context,
        const CIMObjectPath & subscriptionName,
        const Array<CIMObjectPath> & classNames,
        const CIMPropertyList & propertyList,
        const Uint16 repeatNotificationPolicy);

    virtual void modifySubscription(
        const OperationContext & context,
        const CIMObjectPath & subscriptionName,
        const Array<CIMObjectPath> & classNames,
        const CIMPropertyList & propertyList,
        const Uint16 repeatNotificationPolicy);

    virtual void deleteSubscription(
        const OperationContext & context,
        const CIMObjectPath & subscriptionName,
        const Array<CIMObjectPath> & classNames);

    // CIMIndicationConsumer interface
    virtual void handleIndication(
        const OperationContext & context,
        const CIMInstance & indication,
        ResponseHandler<void> & handler)
    {
    }

    virtual void handleIndication(
        const OperationContext & context,
        const String & url,
        const CIMInstance& indicationInstance)
    {
    }

protected:
    CIMBaseProvider * _provider;

};

PEGASUS_NAMESPACE_END

#endif
