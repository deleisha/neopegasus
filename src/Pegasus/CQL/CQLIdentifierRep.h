//%2005////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002 BMC Software; Hewlett-Packard Development
// Company, L.P.; IBM Corp.; The Open Group; Tivoli Systems.
// Copyright (c) 2003 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation, The Open Group.
// Copyright (c) 2004 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation; VERITAS Software Corporation; The Open Group.
// Copyright (c) 2005 Hewlett-Packard Development Company, L.P.; IBM Corp.;
// EMC Corporation; VERITAS Software Corporation; The Open Group.
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
// Authors: David Rosckes (rosckes@us.ibm.com)
//          Bert Rivero (hurivero@us.ibm.com)
//          Chuck Carmack (carmack@us.ibm.com)
//          Brian Lucier (lucier@us.ibm.com)
//
// Modified By: 
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_CQLIdentifierRep_h
#define Pegasus_CQLIdentifierRep_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/CQL/Linkage.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Query/QueryCommon/QueryIdentifierRep.h>

PEGASUS_NAMESPACE_BEGIN


/** 
  The CQLIdentifier class encapsulates
  the different formats of the CQL property portion
  of a CQLChainedIdentifier. 

  For example, a CQLChainedIdentifier can have these parts:
 
    Class.EmbeddedObject.Property
    Class.Property


The "Property" portion of the CQLChainedIdentifier can be
in any of these 3 formats:
   (a)  property name
   (b)  property[3]     e.g. an array index
   (c)  property#'OK'    e.g. a symbolic constant
   (d)  *   (wildcard)

In the future, there may be support added for a set of indices (ranges).
  */
class PEGASUS_CQL_LINKAGE CQLIdentifierRep: public QueryIdentifierRep
{
  public:
    CQLIdentifierRep();
    /**  The constructor for a CQLIdentifier object
          takes a string as input.  The string should contain the
          property portion of a CQLChainedIdentifier.
    
         The constructor parses the input string into the components of 
         the property identifier. 
    
         Throws parsing errors.
      */
    CQLIdentifierRep(String identifier);

    CQLIdentifierRep(const CQLIdentifierRep* rep);

    ~CQLIdentifierRep();

    CQLIdentifierRep& operator=(const CQLIdentifierRep& rhs);

  private:
    void parse(String indentifier);

    static Char16 STAR;
    static Char16 HASH;
    static Char16 RBRKT;
    static Char16 LBRKT;
    static const char SCOPE[];
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CQLIdentifier_h */
