//%///////-*-c++-*-/////////////////////////////////////////////////////////////
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
//              Nag Boranna, Hewlett-Packard Company (nagaraja_boranna@hp.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_CIMExportResponseDecoder_h
#define Pegasus_CIMExportResponseDecoder_h

#include <fstream>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/MessageQueueService.h>
#include <Pegasus/Common/HTTPMessage.h>
#include <Pegasus/Client/ClientAuthenticator.h>
#include <Pegasus/ExportClient/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class XmlParser;

/** This class receives HTTP messages and decodes them into CIM Operation 
    Responses messages which it places on its output queue.
*/
class PEGASUS_EXPORT_CLIENT_LINKAGE CIMExportResponseDecoder :  public MessageQueue
{
  
   public:
     
      typedef MessageQueue Base;

      /** Constuctor.
	  @param outputQueue queue to receive decoded HTTP messages.
      */
      CIMExportResponseDecoder(
	 MessageQueue* outputQueue,
	 MessageQueue* encoderQueue,
	 ClientAuthenticator* authenticator);

      /** Destructor. */
      ~CIMExportResponseDecoder();

      void setEncoderQueue(MessageQueue* encoderQueue);

      /** This method is called when a message is enqueued on this queue. */
      virtual void handleEnqueue(Message *);
      virtual void handleEnqueue();

   private:

      void _handleHTTPMessage(
	 HTTPMessage* message);

      void _handleMethodResponse(
	 char* content);

      CIMExportIndicationResponseMessage* _decodeExportIndicationResponse(
	 XmlParser& parser, const String& messageId);

      MessageQueue*        _outputQueue;
      MessageQueue*        _encoderQueue;
      ClientAuthenticator* _authenticator;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMExportResponseDecoder_h */
