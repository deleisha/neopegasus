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
// Author: Mike Brasher (mbrasher@bmc.com)
//
// Modified By: Nitin Upasani, Hewlett-Packard Company (Nitin_Upasani@hp.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <cctype>
#include <cstdio>
#include <Pegasus/Common/XmlParser.h>
#include <Pegasus/Common/XmlReader.h>
#include <Pegasus/Common/Destroyer.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/Common/Logger.h>
#include "CIMExportResponseEncoder.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

CIMExportResponseEncoder::CIMExportResponseEncoder()
   : Base("CIMExportResponseEncoder", MessageQueue::getNextQueueId())
{

}

CIMExportResponseEncoder::~CIMExportResponseEncoder()
{

}

void CIMExportResponseEncoder::sendResponse(
   Uint32 queueId, 
   Array<Sint8>& message)
{
   MessageQueue* queue = MessageQueue::lookup(queueId);

   if (queue)
   {
      HTTPMessage* httpMessage = new HTTPMessage(message);
      queue->enqueue(httpMessage);
   }
}

// Code is duplicated in CIMExportRequestDecoder
void CIMExportResponseEncoder::sendEMethodError(
   Uint32 queueId, 
   const String& messageId,
   const String& cimMethodName,
   CIMStatusCode code,
   const String& description) 
{
    ArrayDestroyer<char> tmp1(cimMethodName.allocateCString());
    ArrayDestroyer<char> tmp2(description.allocateCString());
    Array<Sint8> message;
    Array<Sint8> tmp;

    XmlWriter::appendMessageElementBegin(message, messageId);
    XmlWriter::appendSimpleExportRspElementBegin(message);
    XmlWriter::appendEMethodResponseElementBegin(message, tmp1.getPointer());
    XmlWriter::appendErrorElement(message, code, tmp2.getPointer());
    XmlWriter::appendEMethodResponseElementEnd(message);
    XmlWriter::appendSimpleExportRspElementEnd(message);
    XmlWriter::appendMessageElementEnd(message);

    XmlWriter::appendEMethodResponseHeader(tmp, message.size());
    tmp << message;
    sendResponse(queueId, tmp);
}

void CIMExportResponseEncoder::sendEMethodError(
   CIMResponseMessage* response,
   const String& cimMethodName)
{
   Uint32 queueId = response->queueIds.top();
   response->queueIds.pop();

   sendEMethodError(
      queueId,
      response->messageId, 
      cimMethodName, 
      response->errorCode, 
      response->errorDescription);
}

void CIMExportResponseEncoder::handleEnqueue(Message *message)
{
   if (!message)
      return;

   switch (message->getType())
   {
      case CIM_EXPORT_INDICATION_RESPONSE_MESSAGE:
	 encodeExportIndicationResponse(
	    (CIMExportIndicationResponseMessage*)message);
	 break;
   }
   
   delete message;
}


void CIMExportResponseEncoder::handleEnqueue()
{
   Message* message = dequeue();
   if(message)
      handleEnqueue(message);
}

const char* CIMExportResponseEncoder::getQueueName() const
{
   return "CIMExportResponseEncoder";
}

void CIMExportResponseEncoder::encodeExportIndicationResponse(
   CIMExportIndicationResponseMessage* response)
{
   if (response->errorCode != CIM_ERR_SUCCESS)
   {
      sendEMethodError(response, "ExportIndication");
      return;
   }

   Array<Sint8> body;
    
   Array<Sint8> message = XmlWriter::formatSimpleEMethodRspMessage(
      "ExportIndication", response->messageId, body);

   sendResponse(response->queueIds.top(), message);
}

PEGASUS_NAMESPACE_END
