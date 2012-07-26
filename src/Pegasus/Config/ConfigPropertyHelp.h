//%LICENSE////////////////////////////////////////////////////////////////
//
// Licensed to The Open Group (TOG) under one or more contributor license
// agreements.  Refer to the OpenPegasusNOTICE.txt file distributed with
// this work for additional information regarding copyright ownership.
// Each contributor licenses this file to you under the OpenPegasus Open
// Source License; you may not use this file except in compliance with the
// License.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//////////////////////////////////////////////////////////////////////////
//
//%/////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
// This file defines the table of help descriptions.  It is maintained as
// a separate file so that the help info can be modified without touching
// any of the code of Config
//
///////////////////////////////////////////////////////////////////////////////


#ifndef Pegasus_ConfigPropertyHelp_h
#define Pegasus_ConfigPropertyHelp_h

#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Config/ConfigExceptions.h>
#include <Pegasus/Config/Linkage.h>


PEGASUS_NAMESPACE_BEGIN

/**
 * structure defining property names / description string
 */
struct configPropertyDescription
{
    const char* name;
    const char* Description;
};

// Define table of struct configPropertyDescription which
// provides short descriptions descriptions
PEGASUS_CONFIG_LINKAGE
    extern struct configPropertyDescription configPropertyDescriptionList[];

PEGASUS_CONFIG_LINKAGE extern Uint32 configPropertyDescriptionListSize;

PEGASUS_NAMESPACE_END

#endif /* Pegasus_ConfigPropertyHelp_h */
