//%/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001 BMC Software, Hewlett-Packard Company, IBM,
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
// Author:  Nag Boranna, Hewlett-Packard Company (nagaraja_boranna@hp.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <Pegasus/Security/UserManager/UserManager.h>
#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/Common/Destroyer.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/Tracer.h>
#include "CIMOperationRequestAuthorizer.h"

PEGASUS_NAMESPACE_BEGIN

PEGASUS_USING_STD;


CIMOperationRequestAuthorizer::CIMOperationRequestAuthorizer(
    MessageQueue* outputQueue)
    :
    _outputQueue(outputQueue),
    _serverTerminating(false)
{
    const char METHOD_NAME[] = 
        "CIMOperationRequestAuthorizer::CIMOperationRequestAuthorizer()";

    PEG_FUNC_ENTER(TRC_SERVER, METHOD_NAME);

    PEG_FUNC_EXIT(TRC_SERVER, METHOD_NAME);
}

CIMOperationRequestAuthorizer::~CIMOperationRequestAuthorizer()
{
    const char METHOD_NAME[] = 
        "CIMOperationRequestAuthorizer::~CIMOperationRequestAuthorizer()";

    PEG_FUNC_ENTER(TRC_SERVER, METHOD_NAME);

    PEG_FUNC_EXIT(TRC_SERVER, METHOD_NAME);
}

void CIMOperationRequestAuthorizer::sendResponse(
    Uint32 queueId,
    Array<Sint8>& message)
{
    const char METHOD_NAME[] = 
        "CIMOperationRequestAuthorizer::sendResponse()";

    PEG_FUNC_ENTER(TRC_SERVER, METHOD_NAME);

    MessageQueue* queue = MessageQueue::lookup(queueId);

    if (queue)
    {
        HTTPMessage* httpMessage = new HTTPMessage(message);
        queue->enqueue(httpMessage);
    }
    PEG_FUNC_EXIT(TRC_SERVER, METHOD_NAME);
}

void CIMOperationRequestAuthorizer::sendError(
    Uint32 queueId,
    const String& messageId,
    const String& cimMethodName,
    CIMStatusCode code,
    const String& description)
{
    const char METHOD_NAME[] = 
        "CIMOperationRequestAuthorizer::sendError()";

    PEG_FUNC_ENTER(TRC_SERVER, METHOD_NAME);

    ArrayDestroyer<char> tmp1(cimMethodName.allocateCString());
    ArrayDestroyer<char> tmp2(description.allocateCString());

    Array<Sint8> message = XmlWriter::formatMethodResponseHeader(
        XmlWriter::formatMessageElement(
            messageId,
            XmlWriter::formatSimpleRspElement(
                XmlWriter::formatIMethodResponseElement(
                    tmp1.getPointer(),
                    XmlWriter::formatErrorElement(code, tmp2.getPointer())))));

    sendResponse(queueId, message);

    PEG_FUNC_EXIT(TRC_SERVER, METHOD_NAME);
}

////////////////////////////////////////////////////////////////////////////////

void CIMOperationRequestAuthorizer::handleEnqueue()
{
    const char METHOD_NAME[] = 
        "CIMOperationRequestAuthorizer::handleEnqueue()";

    PEG_FUNC_ENTER(TRC_SERVER, METHOD_NAME);

    Message* request = dequeue();

    if (!request)
    {
        PEG_FUNC_EXIT(TRC_SERVER, METHOD_NAME);
	return;
    }

    //
    // Get the HTTPConnection queue id
    //
    QueueIdStack qis = ((CIMRequestMessage*)request)->queueIds.copyAndPop();

    Uint32 queueId = qis.top();


    String userName = String::EMPTY;

    String nameSpace = String::EMPTY;

    String cimMethodName = String::EMPTY;

    switch (request->getType())
    {
        case CIM_GET_CLASS_REQUEST_MESSAGE:
            userName = ((CIMGetClassRequestMessage*)request)->userName;
            nameSpace = ((CIMGetClassRequestMessage*)request)->nameSpace;
            cimMethodName = "GetClass";
            break;

        case CIM_GET_INSTANCE_REQUEST_MESSAGE:
            userName = ((CIMGetInstanceRequestMessage*)request)->userName;
            nameSpace = ((CIMGetInstanceRequestMessage*)request)->nameSpace;
            cimMethodName = "GetInstance";
            break;

        case CIM_DELETE_CLASS_REQUEST_MESSAGE:
            userName = ((CIMDeleteClassRequestMessage*)request)->userName;
            nameSpace = ((CIMDeleteClassRequestMessage*)request)->nameSpace;
            cimMethodName = "DeleteClass";
            break;

        case CIM_DELETE_INSTANCE_REQUEST_MESSAGE:
            userName = ((CIMDeleteInstanceRequestMessage*)request)->userName;
            nameSpace = ((CIMDeleteInstanceRequestMessage*)request)->nameSpace;
            cimMethodName = "DeleteInstance";
            break;

        case CIM_CREATE_CLASS_REQUEST_MESSAGE:
            userName = ((CIMCreateClassRequestMessage*)request)->userName;
            nameSpace = ((CIMCreateClassRequestMessage*)request)->nameSpace;
            cimMethodName = "CreateClass";
            break;

        case CIM_CREATE_INSTANCE_REQUEST_MESSAGE:
            userName = ((CIMCreateInstanceRequestMessage*)request)->userName;
            nameSpace = ((CIMCreateInstanceRequestMessage*)request)->nameSpace;
            cimMethodName = "CreateInstance";
            break;

        case CIM_MODIFY_CLASS_REQUEST_MESSAGE:
            userName = ((CIMModifyClassRequestMessage*)request)->userName;
            nameSpace = ((CIMModifyClassRequestMessage*)request)->nameSpace;
            cimMethodName = "ModifyClass";
            break;

        case CIM_MODIFY_INSTANCE_REQUEST_MESSAGE:
            userName = ((CIMModifyInstanceRequestMessage*)request)->userName;
            nameSpace = ((CIMModifyInstanceRequestMessage*)request)->nameSpace;
            cimMethodName = "ModifyInstance";
            break;

        case CIM_ENUMERATE_CLASSES_REQUEST_MESSAGE:
            userName = ((CIMEnumerateClassesRequestMessage*)request)->userName;
            nameSpace = ((CIMEnumerateClassesRequestMessage*)request)->nameSpace;
            cimMethodName = "EnumerateClasses";
            break;

        case CIM_ENUMERATE_CLASS_NAMES_REQUEST_MESSAGE:
            userName = ((CIMEnumerateClassNamesRequestMessage*)request)->userName;
            nameSpace = ((CIMEnumerateClassNamesRequestMessage*)request)->nameSpace;
            cimMethodName = "EnumerateClassNames";
            break;

        case CIM_ENUMERATE_INSTANCES_REQUEST_MESSAGE:
            userName = ((CIMEnumerateInstancesRequestMessage*)request)->userName;
            nameSpace = ((CIMEnumerateInstancesRequestMessage*)request)->nameSpace;
            cimMethodName = "EnumerateInstances";
            break;

        case CIM_ENUMERATE_INSTANCE_NAMES_REQUEST_MESSAGE:
            userName = ((CIMEnumerateInstanceNamesRequestMessage*)request)->userName;
            nameSpace = ((CIMEnumerateInstanceNamesRequestMessage*)request)->nameSpace;
            cimMethodName = "EnumerateInstanceNames";
            break;

        //ATTN: Implement this when ExecQuery is implemented in the decoder
        case CIM_EXEC_QUERY_REQUEST_MESSAGE:
            //userName = ((CIMExecQueryRequestMessage*)request)->userName;
            //nameSpace = ((CIMExecQueryRequestMessage*)request)->nameSpace;
            //cimMethodName = "ExecQuery";
            break;

        case CIM_ASSOCIATORS_REQUEST_MESSAGE:
            userName = ((CIMAssociatorsRequestMessage*)request)->userName;
            nameSpace = ((CIMAssociatorsRequestMessage*)request)->nameSpace;
            cimMethodName = "Associators";
            break;

        case CIM_ASSOCIATOR_NAMES_REQUEST_MESSAGE:
            userName = ((CIMAssociatorNamesRequestMessage*)request)->userName;
            nameSpace = ((CIMAssociatorNamesRequestMessage*)request)->nameSpace;
            cimMethodName = "AssociatorNames";
            break;

        case CIM_REFERENCES_REQUEST_MESSAGE:
            userName = ((CIMReferencesRequestMessage*)request)->userName;
            nameSpace = ((CIMReferencesRequestMessage*)request)->nameSpace;
            cimMethodName = "References";
            break;

        case CIM_REFERENCE_NAMES_REQUEST_MESSAGE:
            userName = ((CIMReferenceNamesRequestMessage*)request)->userName;
            nameSpace = ((CIMReferenceNamesRequestMessage*)request)->nameSpace;
            cimMethodName = "ReferenceNames";
            break;

        case CIM_GET_PROPERTY_REQUEST_MESSAGE:
            userName = ((CIMGetPropertyRequestMessage*)request)->userName;
            nameSpace = ((CIMGetPropertyRequestMessage*)request)->nameSpace;
            cimMethodName = "GetProperty";
            break;

        case CIM_SET_PROPERTY_REQUEST_MESSAGE:
            userName = ((CIMSetPropertyRequestMessage*)request)->userName;
            nameSpace = ((CIMSetPropertyRequestMessage*)request)->nameSpace;
            cimMethodName = "SetProperty";
            break;

        case CIM_GET_QUALIFIER_REQUEST_MESSAGE:
            userName = ((CIMGetQualifierRequestMessage*)request)->userName;
            nameSpace = ((CIMGetQualifierRequestMessage*)request)->nameSpace;
            cimMethodName = "GetQualifier";
            break;

        case CIM_SET_QUALIFIER_REQUEST_MESSAGE:
            userName = ((CIMSetQualifierRequestMessage*)request)->userName;
            nameSpace = ((CIMSetQualifierRequestMessage*)request)->nameSpace;
            cimMethodName = "SetQualifier";
            break;

        case CIM_DELETE_QUALIFIER_REQUEST_MESSAGE:
            userName = ((CIMDeleteQualifierRequestMessage*)request)->userName;
            nameSpace = ((CIMDeleteQualifierRequestMessage*)request)->nameSpace;
            cimMethodName = "DeleteQualifier";
            break;

        case CIM_ENUMERATE_QUALIFIERS_REQUEST_MESSAGE:
            userName = ((CIMEnumerateQualifiersRequestMessage*)request)->userName;
            nameSpace = ((CIMEnumerateQualifiersRequestMessage*)request)->nameSpace;
            cimMethodName = "EnumerateQualifiers";
            break;

        case CIM_INVOKE_METHOD_REQUEST_MESSAGE:
            userName = ((CIMInvokeMethodRequestMessage*)request)->userName;
            nameSpace = ((CIMInvokeMethodRequestMessage*)request)->nameSpace;
            cimMethodName = "InvokeMethod";
            break;

        case CIM_ENABLE_INDICATION_SUBSCRIPTION_REQUEST_MESSAGE:
            userName = 
                ((CIMEnableIndicationSubscriptionRequestMessage*)request)->userName;
            nameSpace = 
                ((CIMEnableIndicationSubscriptionRequestMessage*)request)->nameSpace;
            cimMethodName = "EnableIndicationSubscription";
            break;

        case CIM_MODIFY_INDICATION_SUBSCRIPTION_REQUEST_MESSAGE:
            userName = 
                ((CIMModifyIndicationSubscriptionRequestMessage*)request)->userName;
            nameSpace = 
                ((CIMModifyIndicationSubscriptionRequestMessage*)request)->nameSpace;
            cimMethodName = "ModifyIndicationSubscription";
            break;

        case CIM_DISABLE_INDICATION_SUBSCRIPTION_REQUEST_MESSAGE:
            userName = 
                ((CIMDisableIndicationSubscriptionRequestMessage*)request)->userName;
            nameSpace = 
                ((CIMDisableIndicationSubscriptionRequestMessage*)request)->nameSpace;
            cimMethodName = "DisableIndicationSubscription";
            break;

        default:
            break;
    }

    //
    // if CIMOM is shutting down, return error response
    //
    // ATTN:  Need to define a new CIM Error.
    //
    if (_serverTerminating)
    {
        String description = "CIMServer is shutting down.  ";
        description.append("Request cannot be processed: ");

        sendError(
            queueId,
            ((CIMRequestMessage*)request)->messageId,
            cimMethodName,
            CIM_ERR_FAILED,
            description);

        PEG_FUNC_EXIT(TRC_SERVER, METHOD_NAME);

        return;
    }

    //
    // Do Authorization verification
    //
    UserManager* userManager = UserManager::getInstance();

    if ( !userManager || !userManager->verifyAuthorization(
        userName, nameSpace, cimMethodName) )
    {
        String description = "Not authorized to run ";
            description.append(cimMethodName);
        description.append(" in the namespace ");
        description.append(nameSpace);

        sendError(
            queueId,
            ((CIMRequestMessage*)request)->messageId,
            cimMethodName,
            CIM_ERR_FAILED,
            description);

        PEG_FUNC_EXIT(TRC_SERVER, METHOD_NAME);

        return;
    }

    //
    // Enqueue the request
    //
    _outputQueue->enqueue(request);

    PEG_FUNC_EXIT(TRC_SERVER, METHOD_NAME);
}

const char* CIMOperationRequestAuthorizer::getQueueName() const
{
    const char METHOD_NAME[] = 
        "CIMOperationRequestAuthorizer::getQueueName()";

    PEG_FUNC_ENTER(TRC_SERVER, METHOD_NAME);

    PEG_FUNC_EXIT(TRC_SERVER, METHOD_NAME);

    return "CIMOperationRequestAuthorizer";
}

void CIMOperationRequestAuthorizer::setServerTerminating(Boolean flag)
{
    const char METHOD_NAME[] = 
        "CIMOperationRequestAuthorizer::setServerTerminating()";

    PEG_FUNC_ENTER(TRC_SERVER, METHOD_NAME);

    _serverTerminating = flag;

    PEG_FUNC_EXIT(TRC_SERVER, METHOD_NAME);
}

PEGASUS_NAMESPACE_END
