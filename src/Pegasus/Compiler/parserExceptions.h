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
// Author: Bob Blair (bblair@bmc.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////


// Exceptions that can occur in processing the cimmof compiler command line
//

#ifndef _PARSEREXCEPTION_H_
#define _PARSEREXCEPTION_H_

// NOTE:  This exception does not use the Exception class from
// Pegasus/Common because it has to do with the base class parser,
// which should be reusable outside Pegasus.

#include <Pegasus/Common/String.h>

namespace ParserExceptions {

class PEGASUS_COMPILER_LINKAGE ParserException {
 private:
  const String _msg;
 public:
  ParserException(const char *msg) : _msg(msg) {} ;
  ParserException(const String &msg) :  _msg(msg) {};
  const String &getMessage() const { return _msg; };
};

class PEGASUS_COMPILER_LINKAGE ParserLexException : public ParserException  {
 public:
  ParserLexException(const char *msg) : ParserException(msg) {};
  ParserLexException(const String &msg) : ParserException(msg) {};
  ~ParserLexException() {};
};

}

#endif
