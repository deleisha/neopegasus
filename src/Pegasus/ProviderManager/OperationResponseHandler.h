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
// Modified By: Carol Ann Krug Graves, Hewlett-Packard Company
//                  (carolann_graves@hp.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_OperationResponseHandler_h
#define Pegasus_OperationResponseHandler_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/MessageQueueService.h>
#include <Pegasus/Common/Constants.h>

#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMIndication.h>
#include <Pegasus/Common/CIMValue.h>

#include <Pegasus/Provider/ResponseHandler.h>
#include <Pegasus/Provider/SimpleResponseHandler.h>

PEGASUS_NAMESPACE_BEGIN

template<class T>
class OperationResponseHandler : public SimpleResponseHandler<T>
{
public:
    OperationResponseHandler(CIMRequestMessage * request, CIMResponseMessage * response);
    virtual ~OperationResponseHandler(void);

    virtual void complete(void);
    virtual void complete(const OperationContext & context);

    CIMRequestMessage * getRequest(void) const;
    CIMResponseMessage * getResponse(void) const;

    virtual void setStatus(const Uint32 code, const String & message = String::EMPTY);

protected:
    CIMRequestMessage * _request;
    CIMResponseMessage * _response;

};

template<class T>
inline OperationResponseHandler<T>::OperationResponseHandler(CIMRequestMessage * request, CIMResponseMessage * response)
    : _request(request), _response(response)
{
}

template<class T>
inline OperationResponseHandler<T>::~OperationResponseHandler(void)
{
}

template<class T>
inline void OperationResponseHandler<T>::complete(void)
{
    OperationContext context;

    complete(context);
}

template<class T>
inline void OperationResponseHandler<T>::complete(const OperationContext & context)
{
}

template<class T>
inline CIMRequestMessage * OperationResponseHandler<T>::getRequest(void) const
{
    return(_request);
}

template<class T>
inline CIMResponseMessage * OperationResponseHandler<T>::getResponse(void) const
{
    return(_response);
}

template<class T>
inline void OperationResponseHandler<T>::setStatus(const Uint32 code, const String & message)
{
    _response->cimException = PEGASUS_CIM_EXCEPTION(CIMStatusCode(code), message);
}

/* ------------------------------------------------------------------------- */
/* operation specific response handlers                                      */
/* ------------------------------------------------------------------------- */

class GetInstanceResponseHandler:  public OperationResponseHandler<CIMInstance>
{
public:
    GetInstanceResponseHandler(
        CIMGetInstanceRequestMessage * request,
        CIMGetInstanceResponseMessage * response)
    : OperationResponseHandler<CIMInstance>(request, response)
    {
    }

    virtual void complete(const OperationContext & context)
    {
        if(getObjects().size() == 0)
        {
            // error? provider claims success,
            // but did not deliver an instance.
            setStatus(CIM_ERR_NOT_FOUND);

            return;
        }

        static_cast<CIMGetInstanceResponseMessage *>(
            getResponse())->cimInstance = getObjects()[0];
    }
};

class EnumerateInstancesResponseHandler : public OperationResponseHandler<CIMInstance>
{
public:
    EnumerateInstancesResponseHandler(
        CIMEnumerateInstancesRequestMessage * request,
        CIMEnumerateInstancesResponseMessage * response)
    : OperationResponseHandler<CIMInstance>(request, response)
    {
    }

    virtual void complete(const OperationContext & context)
    {
        static_cast<CIMEnumerateInstancesResponseMessage *>(
            getResponse())->cimNamedInstances = getObjects();
    }

};

class EnumerateInstanceNamesResponseHandler : public OperationResponseHandler<CIMObjectPath>
{
public:
    EnumerateInstanceNamesResponseHandler(
        CIMEnumerateInstanceNamesRequestMessage * request,
        CIMEnumerateInstanceNamesResponseMessage * response)
    : OperationResponseHandler<CIMObjectPath>(request, response)
    {
    }

    virtual void complete(const OperationContext & context)
    {
        static_cast<CIMEnumerateInstanceNamesResponseMessage *>(
            getResponse())->instanceNames = getObjects();
    }

};

class CreateInstanceResponseHandler : public OperationResponseHandler<CIMObjectPath>
{
public:
    CreateInstanceResponseHandler(
        CIMCreateInstanceRequestMessage * request,
        CIMCreateInstanceResponseMessage * response)
    : OperationResponseHandler<CIMObjectPath>(request, response)
    {
    }

    virtual void complete(const OperationContext & context)
    {
        if(getObjects().size() == 0)
        {
            // ATTN: is it an error to not return instance name?
            return;
        }

        static_cast<CIMCreateInstanceResponseMessage *>(
            getResponse())->instanceName = getObjects()[0];
    }

};

class ModifyInstanceResponseHandler : public ResponseHandler<void>
{
public:
    ModifyInstanceResponseHandler(
        CIMModifyInstanceRequestMessage * request,
        CIMModifyInstanceResponseMessage * response)
    : ResponseHandler<void>(request, response)
    {
    }

};

class DeleteInstanceResponseHandler : public ResponseHandler<void>
{
public:
    DeleteInstanceResponseHandler(
        CIMDeleteInstanceRequestMessage * request,
        CIMDeleteInstanceResponseMessage * response)
    : ResponseHandler<void>(request, response)
    {
    }

};

class GetPropertyResponseHandler : public OperationResponseHandler<CIMValue>
{
public:
    GetPropertyResponseHandler(
        CIMGetPropertyRequestMessage * request,
        CIMGetPropertyResponseMessage * response)
    : OperationResponseHandler<CIMValue>(request, response)
    {
    }

    virtual void complete(const OperationContext & context)
    {
        if(getObjects().size() == 0)
        {
            // error? provider claims success,
            // but did not deliver an instance.
            setStatus(CIM_ERR_NOT_FOUND);

            return;
        }

        static_cast<CIMGetPropertyResponseMessage *>(
            getResponse())->value = getObjects()[0];
    }
};

class SetPropertyResponseHandler : public OperationResponseHandler<CIMValue>
{
public:
    SetPropertyResponseHandler(
        CIMSetPropertyRequestMessage * request,
        CIMSetPropertyResponseMessage * response)
    : OperationResponseHandler<CIMValue>(request, response)
    {
    }

};

class AssociatorNamesResponseHandler : public OperationResponseHandler<CIMObjectPath>
{
public:
    AssociatorNamesResponseHandler(
        CIMAssociatorNamesRequestMessage * request,
        CIMAssociatorNamesResponseMessage * response)
    : OperationResponseHandler<CIMObjectPath>(request, response)
    {
    }

    virtual void complete(const OperationContext & context)
    {
        static_cast<CIMAssociatorNamesResponseMessage *>(
            getResponse())->objectNames.appendArray(getObjects());
    }

};

class ReferencesResponseHandler : public OperationResponseHandler<CIMObject>
{
public:
    ReferencesResponseHandler(
        CIMReferencesRequestMessage * request,
        CIMReferencesResponseMessage * response)
    : OperationResponseHandler<CIMObject>(request, response)
    {
    }

    virtual void complete(const OperationContext & context)
    {
        static_cast<CIMReferencesResponseMessage *>(
            getResponse())->cimObjects = getObjects();
    }

};

class ReferenceNamesResponseHandler : public OperationResponseHandler<CIMObjectPath>
{
public:
    ReferenceNamesResponseHandler(
        CIMReferenceNamesRequestMessage * request,
        CIMReferenceNamesResponseMessage * response)
    : OperationResponseHandler<CIMObjectPath>(request, response)
    {
    }

    virtual void complete(const OperationContext & context)
    {
        static_cast<CIMReferenceNamesResponseMessage *>(
            getResponse())->objectNames.appendArray(getObjects());
    }

};

class InvokeMethodResponseHandler : public OperationResponseHandler<CIMValue>
{
public:
    InvokeMethodResponseHandler(
        CIMInvokeMethodRequestMessage * request,
        CIMInvokeMethodResponseMessage * response)
    : OperationResponseHandler<CIMValue>(request, response)
    {
    }

    virtual void complete(const OperationContext & context)
    {
        if(getObjects().size() == 0)
        {
            // ATTN: error? provider claims success, but did not deliver a CIMValue.
            return;
        }

        static_cast<CIMInvokeMethodResponseMessage *>(
            getResponse())->retValue = getObjects()[0];
    }

};

class EnableIndicationsResponseHandler : public OperationResponseHandler<CIMIndication>
{
public:
    EnableIndicationsResponseHandler(
        CIMEnableIndicationsRequestMessage * request,
        CIMEnableIndicationsResponseMessage * response,
        MessageQueueService * source,
        MessageQueueService * target = 0)
    : OperationResponseHandler<CIMIndication>(request, response),
        _source(source),
        _target(target)
    {
        PEGASUS_ASSERT(_source != 0);

        // get indication service
        if(_target == 0)
        {
            Array<Uint32> serviceIds;

            _source->find_services(PEGASUS_QUEUENAME_INDICATIONSERVICE, 0, 0, &serviceIds);

            PEGASUS_ASSERT(serviceIds.size() != 0);

            _target = dynamic_cast<MessageQueueService *>(MessageQueue::lookup(serviceIds[0]));

            PEGASUS_ASSERT(_target != 0);
        }
    }

    virtual void deliver(const CIMIndication & cimIndication)
    {
        OperationContext context;

        deliver(context, cimIndication);
    }

    virtual void deliver(const OperationContext & context, const CIMIndication & cimIndication)
    {
        // ATTN: temporarily convert indication to instance
        CIMInstance cimInstance(cimIndication);

        // create message
        CIMProcessIndicationRequestMessage * request =
            new CIMProcessIndicationRequestMessage(
            getRequest()->messageId,
            cimInstance.getPath().getNameSpace(),
            cimInstance,
            QueueIdStack(_target->getQueueId(), _source->getQueueId()));

        // send message
        // <<< Wed Apr 10 21:04:00 2002 mdd >>>
        // AsyncOpNode * op = _source->get_op();

        AsyncLegacyOperationStart * asyncRequest =
            new AsyncLegacyOperationStart(
            _source->get_next_xid(),
            0,
            _target->getQueueId(),
            request,
            _target->getQueueId());

        PEGASUS_ASSERT(asyncRequest != 0);

        //AsyncReply * asyncReply = _source->SendWait(asyncRequest);
        // <<< Wed Apr 10 21:04:50 2002 mdd >>>
        _source->SendForget(asyncRequest);
        //PEGASUS_ASSERT(asyncReply != 0);

        //  Chip - receiver of the request should delete it
        //delete asyncRequest;
        // <<< Wed Apr 10 21:05:10 2002 mdd >>>
    }

    virtual void deliver(const Array<CIMIndication> & cimIndications)
    {
        OperationContext context;

        deliver(context, cimIndications);
    }

    virtual void deliver(const OperationContext & context, const Array<CIMIndication> & cimIndications)
    {
        for(Uint32 i = 0, n = cimIndications.size(); i < n; i++)
        {
            deliver(context, cimIndications[i]);
        }
    }

protected:
    MessageQueueService * _source;
    MessageQueueService * _target;
};

PEGASUS_NAMESPACE_END

#endif
