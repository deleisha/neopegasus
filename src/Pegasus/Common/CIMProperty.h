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
// Author: Mike Brasher (mbrasher@bmc.com)
//
// Modified By: Roger Kumpf (roger_kumpf@hp.com)
//              Carol Ann Krug Graves, Hewlett-Packard Company
//                (carolann_graves@hp.com)
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_Property_h
#define Pegasus_Property_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/CIMQualifier.h>

PEGASUS_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//
// CIMProperty
//
////////////////////////////////////////////////////////////////////////////////

class CIMConstProperty;
class CIMPropertyRep;
#ifdef PEGASUS_INTERNALONLY
class CIMClassRep;
class CIMInstanceRep;
class Resolver;
#endif

// ATTN : P3 KS 03/02/02Documentation This is a very poor definition for property class.
// ATTN: Define the property concept in more detail and ref property.

/** CIMProperty Class - This C++ class implements a CIM Property 
object. It defines a single CIM Property and allows the manipulation of that 
property.
*/
class PEGASUS_COMMON_LINKAGE CIMProperty
{
public:

    /** CIMProperty constructor. */
    CIMProperty();

    /** CIMProperty constructor. Constructs this property form another 
		CIMProperty object
	*/
    CIMProperty(const CIMProperty& x);

    /** CIMProperty Constructor for CIMProperty that adds a number of 
        parameters to the constructed CIMProperty object.
        @param name CIMName Name for the property 
		@param value CIMValue defines the value for the property
		@param arraySize (optional) - Size of array if fixed array size.
		@param referenceClassName (optional) CIMName parameter that defines the 
        reference class name for the property. This parameter is required if
		the property is type CIMObjectPath.
        @param classOrigin (optional) CIMName parameter to define the class 
		origin of the property /Ref{ClassOrigin} 
        @param propagated (optional) If true defines the property as 
        propagated  /Ref{propagated properties}
        @return	The constructed property object
    */
    CIMProperty(
	const CIMName& name,
	const CIMValue& value,
	Uint32 arraySize = 0,
	const CIMName& referenceClassName = CIMName(),
	const CIMName& classOrigin = CIMName(),
	Boolean propagated = false);

    /** ~CIMProperty(). */
    ~CIMProperty();

    /// Operator =
    CIMProperty& operator=(const CIMProperty& x);

    /** getName - Gets the name of the property.
        @return CIMName containing the property name.
        <pre>
        CIMProperty p1("count", Uint32(231));
    	assert(p1.getName() == Uint32(231));
		</pre>
    */
    const CIMName& getName() const;

    /** setName - Set the property name.
		@param name Name to set as parameter name
    */
    void setName(const CIMName& name);

    /** Get the value of the property. */
    const CIMValue& getValue() const;

    /** Get the type of the property. */
    CIMType getType() const;

    /** Check if the property is an array type. */
    Boolean isArray() const;

    /** setValue Sets the Value in the Property object from the input 
        parameter.
		@param value CIMValue containing the value to be put into the 
        property. /Ref{CIMValue}
    */
    void setValue(const CIMValue& value);

    /** getArraySize gets the arraysize parameter from the property
	@return Uint32 with the arraysize.
	*/
    Uint32 getArraySize() const;

    /** getReferenceClassName - gets the referenceClassName.
	@return CIMName contianing the referenceClassName if this is a
	reference property or empty if another CIMType.
    */
    const CIMName& getReferenceClassName() const;

    /** getClassOrigin - Gets the classOrigin field from the property
		object. This will be a string with the name of the originating
		class for the property or empty if this is the originating class
		@return CIMName with classOrigin name.
	*/
    const CIMName& getClassOrigin() const;

    /**setClassOrigin - Sets the Class Origin attribute
	@param classOrigin CIMName containing the classOrigin
    */
    void setClassOrigin(const CIMName& classOrigin);

    /** getPropagated - Tests if this property is propagated.
	@return - Returns true if the class is propagated.
    */
    Boolean getPropagated() const;

    /** setProgagated - Sets the propagated attribute true or fals.
		@param Boolean true or false representing propagated state to be set.
	*/
    void setPropagated(Boolean propagated);

    /** addQualifier adds a qualifier object to the property and
		increases the qualifier count
		@param x CIMQualifier object to be added
		@return Returns this properpty as a convience in adding multiple entities
		@exception AlreadyExistsException.
    */
    CIMProperty& addQualifier(const CIMQualifier& x);

    /**findQualifier - Finds the qualifier object defined
	by the name parameter if it is attached to this 
	CIMProperty
	@param name CIMName parameter defining name of Qualifier
	object.
	@return Position of the qualifier object or -1 if not 
	found
    */
    Uint32 findQualifier(const CIMName& name) const;

    /** getQualifier - gets the Qualifier object specified by the
	index parameter.
	@param index Index parameter for the Qualifier object to be
	retrieved
	@return returns a CIMQualifier object.
	@exception IndexOutOfBoundsException if index is outside range
	of Qualifiers in this property object.
    */
    CIMQualifier getQualifier(Uint32 index);

    /** getQualifier Returns the qualifier at the specified index
        @param index Index of the qualifier. Can be obtained from the
        findQualifier method.
        @return Returns the qualifier object.
        @exception IndexOutOfBoundsException if index is outside the range
        of qualifiers that exist for the property.
    */
    CIMConstQualifier getQualifier(Uint32 index) const;

    /** removeQualifier - Removes the CIMQualifier defined by the 
	index input as a parameter.
	@param Position of the qualifier requested.
	@return CIMQualifier object or exception
	@exception IndexOutOfBoundsException if the index is outside the
        range of parameters available from the CIMMethod.
    */
    void removeQualifier(Uint32 index);
    
    /** getQualifierCount - Returns count of the number
    	of qualifiers attached to the CIMProperty object.
	@return Count of the number of CIMQualifier objects attached
	to the CIMProperty object.
    */
    Uint32 getQualifierCount() const;

    /**identical - compares the CIMProperty object with
       another CIMProperty object defined by the input parameter.
       @param x CIMPropery object for comparison
       @return Returns true if the objects are identical
    */
    Boolean identical(const CIMConstProperty& x) const;

    // clone - ATTN: P3 Documentation
    CIMProperty clone() const;

    /**
        Determines if the object has not been initialized.

        @return  True if the object has not been initialized,
                 False otherwise
     */
    Boolean isUninitialized() const;

private:

    CIMProperty(CIMPropertyRep* rep);

    // This constructor allows the CIMClassRep friend class to cast
    // away constness.

    PEGASUS_EXPLICIT CIMProperty(const CIMConstProperty& x);

    void _checkRep() const;

    friend class CIMConstProperty;
    friend class CIMClassRep;
    friend class CIMInstanceRep;
    friend class Resolver;
    friend class XmlWriter;
    friend class MofWriter;

    CIMPropertyRep* _rep;
};

////////////////////////////////////////////////////////////////////////////////
//
// CIMConstProperty
//
////////////////////////////////////////////////////////////////////////////////

class PEGASUS_COMMON_LINKAGE CIMConstProperty
{
public:

    CIMConstProperty();

    CIMConstProperty(const CIMConstProperty& x);

    CIMConstProperty(const CIMProperty& x);

    CIMConstProperty(
	const CIMName& name,
	const CIMValue& value,
	Uint32 arraySize = 0,
	const CIMName& referenceClassName = CIMName(),
	const CIMName& classOrigin = CIMName(),
	Boolean propagated = false);

    ~CIMConstProperty();

    CIMConstProperty& operator=(const CIMConstProperty& x);

    CIMConstProperty& operator=(const CIMProperty& x);

    const CIMName& getName() const;

    const CIMValue& getValue() const;

    CIMType getType() const;

    Boolean isArray() const;

    Uint32 getArraySize() const;

    const CIMName& getReferenceClassName() const;

    const CIMName& getClassOrigin() const;

    Boolean getPropagated() const;

    Uint32 findQualifier(const CIMName& name) const;

    CIMConstQualifier getQualifier(Uint32 index) const;

    Uint32 getQualifierCount() const;

    Boolean identical(const CIMConstProperty& x) const;

    CIMProperty clone() const;

    Boolean isUninitialized() const;

private:

    void _checkRep() const;

    CIMPropertyRep* _rep;

    friend class CIMProperty;
    friend class CIMPropertyRep;
    friend class XmlWriter;
    friend class MofWriter;
};

#define PEGASUS_ARRAY_T CIMProperty
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Property_h */
