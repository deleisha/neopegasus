//%/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001 The Open group, BMC Software, Tivoli Systems, IBM
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
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <iostream>
#include <Pegasus/Common/TCPChannel.h>

#define D(X) /* empty */

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

class MyChannelHandler : public ChannelHandler
{
public:

    MyChannelHandler()
    {
	D( cout << "MyChannelHandler::MyChannelHandler()" << endl; )
    }

    virtual ~MyChannelHandler()
    {
	D( cout << "MyChannelHandler::~MyChannelHandler()" << endl; )
    }

    virtual Boolean handleOpen(Channel* channel)
    {
	D( cout << "MyChannelHandler::handleOpen()" << endl; )
	return true;
    }

    virtual Boolean handleInput(Channel* channel)
    {
	D( cout << "MyChannelHandler::handleInput()" << endl; )

	char buffer[1024];

	Sint32 size = channel->read(buffer, sizeof(buffer));

	if (size <= 0)
	    return false;

	D( cout << "size=" << size << endl; )

	channel->write(buffer, size);

	return true;
    }

    virtual Boolean handleOutput(Channel* channel)
    {
	D( cout << "MyChannelHandler::handleOutput()" << endl; )
	return true;
    }

    virtual void handleClose(Channel* channel)
    {
	D( cout << "MyChannelHandler::handleClose()" << endl; ) 
    }
};

int main()
{
    ChannelHandlerFactory* factory 
	= new DefaultChannelHandlerFactory<MyChannelHandler>;

    Selector* selector = new Selector;

    TCPChannelAcceptor acceptor(factory, selector);
    assert(acceptor.bind("7777"));

    while (true)
    {
	D( cout << "Loop..." << endl; )
	selector->select(10000);
    }

    PEGASUS_UNREACHABLE( return 0; )
}
