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
// Author: Mike Brasher (mbrasher@bmc.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_CIMClass_h
#define Pegasus_CIMClass_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMClassRep.h>

PEGASUS_NAMESPACE_BEGIN

class CIMConstClass;

/** The CIMClass class is used to represent CIM classes in Pegasus.  In CIM,
    a class object may be a class or an associator.  A CIM class must contain a
    name and may contain methods, properties, and qualifiers.  It is a template
    for creating a CIM instance.  A CIM class represents a collection of CIM
    instances, all of which support a common type (for example, a set of
    properties, methods, and associations).
*/

class PEGASUS_COMMON_LINKAGE CIMClass
{
public:

    /** Constructor - Creates an uninitiated a new CIM object 
	reprenting a CIM class. The class object created by this
	constructor can only be used in an operation such as the
	copy constructor.  It cannot be used to create a class by
	appending names, properties, etc. since it is unitiated.
	
	Use one of the other constructors to create an initiated new CIM class 
	object.
	@exception Throws an exception "unitialized handle" if this 
	unitialized handle is used
	/REF(HPEGASUS_HANDLES)
    */
    CIMClass() : _rep(0)
    {

    }

    /** Constructor - Creates a class from a previous class
    */
    CIMClass(const CIMClass& x)
    {
	Inc(_rep = x._rep);
    }

    /// Operator = Assigns the CIM Class constructor.
    CIMClass& operator=(const CIMClass& x)
    {
	if (x._rep != _rep)
	{
	    Dec(_rep);
	    Inc(_rep = x._rep);
	}
	return *this;
    }

    /**	 Constructor - Creates a Class from inputs of a classname and
	SuperClassName
	@param className - String representing name of the class being created
	@param superClassName - String representing name of the SuperClass
	ATTN: Define what makes up legal name.
	@return Throws IllegalName if className argument illegal CIM identifier.
	<pre>
	    CIMClass NewCass("MyClass", "YourClass"); 
	</pre>

    */
    CIMClass(
	const String& className,
	const String& superClassName = String())
    {
	_rep = new CIMClassRep(className, superClassName);
    }

    /// Destructor
    ~CIMClass()
    {
	Dec(_rep);
    }

    /** CIMMethod isAssociation - Identifies whether or not this CIM class
	is an association. An association is a relationship between two
	(or more) classes or instances of two classes.  The properties of an
	association class include pointers, or references, to the two (or
	more) instances. All CIM classes can be included in one or more
	associations.  ATTN: Move the association definition elsewhere
	@return  Boolean True if this CIM class belongs to an association;
	otherwise, false.
    */
    Boolean isAssociation() const
    {
	_checkRep();
	return _rep->isAssociation();
    }

    ///	 CIMMethod isAbstract
    Boolean isAbstract() const
    {
	_checkRep();
	return _rep->isAbstract();
    }

    /** CIMMethod Gets the name of the class
	ATTN: COMMENT. Why not just get name so we have common method for all.
    */
    const String& getClassName() const
    {
	_checkRep();
	return _rep->getClassName();
    }

    /** CIMMethod getSuperClassName - Gets the name of the Parent
	@return String with parent class name.
    */
    const String& getSuperClassName() const
    {
	_checkRep();
	return _rep->getSuperClassName();
    }

    /**	CIMMethod setSuperClassName - Sets the name of the parent class from
	the input parameter. \REF{CLASSNAME}. ATTN: Define legal classnames
	@param - String defining parent name.
	@return Throws IllegalName if superClassName argument not legal CIM
	identifier.
    */
    void setSuperClassName(const String& superClassName)
    {
	_checkRep();
	_rep->setSuperClassName(superClassName);
    }

    /** CIMMethod addQualifier - Adds the specified qualifier to the class
	and increments the qualifier count. It is illegal to add the same
	qualifier more than one time.
	@param - CIMQualifier object representing the qualifier to be added
	ATTN: Pointer to qualifier object.
	@return Throws AlreadyExists.
    */
    CIMClass& addQualifier(const CIMQualifier& qualifier)
    {
	_checkRep();
	_rep->addQualifier(qualifier);
	return *this;
    }

    /**	CIMMethod findQualifier - Finds a qualifier with the specified input
	name if it exists in the class @param name CIMName of the qualifier
	to be found @return Position of the qualifier in the Class ATTN:
	Clarify the return.  What if not found, etc.
    */
    Uint32 findQualifier(const String& name)
    {
	_checkRep();
	return _rep->findQualifier(name);
    }

    /** CIMMethod FindQualifier - ATTN:
	@param name of the qualifier to be found
	@return ATTN: Define this
    */
    Uint32 findQualifier(const String& name) const
    {
	_checkRep();
	return _rep->findQualifier(name);
    }

    /**	 CIMMethod getQualifier - Gets the CIMQualifier object defined
	by the input parameter
	@param pos defines the position of the qualifier in the class from the
	findQualifier method
	@return CIMQualifier object representing the qualifier found.
	ATTN: what is error return here?
    */
    CIMQualifier getQualifier(Uint32 pos)
    {
	_checkRep();
	return _rep->getQualifier(pos);
    }

    /// CIMMethod getQualifier - ATTN:
    CIMConstQualifier getQualifier(Uint32 pos) const
    {
	_checkRep();
	return _rep->getQualifier(pos);
    }

    /** CIMMethod getQualifierCount - Returns the number of qualifiers
	in the class.
	@return ATTN:
    */
    Uint32 getQualifierCount() const
    {
	_checkRep();
	return _rep->getQualifierCount();
    }

    /**	CIMMethod addProperty - Adds the specified property object to the
	properties in the CIM class
    */
    CIMClass& addProperty(const CIMProperty& x)
    {
	_checkRep();
	_rep->addProperty(x);
	return *this;
    }

    /** CIMMethod removeProperty - Removes the property represented
	by the position input parameter from the class
	@param position parameter for the property to be removed from the
	findPropety method
	@return ATTN:
    */
    void removeProperty(Uint32 pos)
    {
	_checkRep();
	_rep->removeProperty(pos);
    }

    /** CIMMethod findProperty - Finds the property object with the
	name defined by the input parameter in the class.
	@param String parameter with the property name.
	@return position representing the property object found.
	ATTN:   Clarify case of not found
    */
    Uint32 findProperty(const String& name)
    {
	_checkRep();
	return _rep->findProperty(name);
    }

    /// CIMMethod findProperty
    Uint32 findProperty(const String& name) const
    {
	_checkRep();
	return _rep->findProperty(name);
    }

    /** CIMMethod getProperty - Returns a property representing the property
	defined by the input parameter
	@param position for this property
	ATTN: Should we not use something like handle for position???
	@return CIMProperty object
	ATTN: what is error return?
    */
    CIMProperty getProperty(Uint32 pos)
    {
	_checkRep();
	return _rep->getProperty(pos);
    }

    /// CIMMethod getProperty - ATTN
    CIMConstProperty getProperty(Uint32 pos) const
    {
	_checkRep();
	return _rep->getProperty(pos);
    }

    /** CIMMethod getProperty -   Gets the count of the number of properties
	defined in the class.
	@return count of number of proerties in the class
    */
    Uint32 getPropertyCount() const
    {
	_checkRep();
	return _rep->getPropertyCount();
    }

    /** CIMMethod addMethod - Adds the method object defined by the input
	parameter to the class and increments the count of the number of
	methods in the class
	@param - method object representing the method to be added
    */
    CIMClass& addMethod(const CIMMethod& x)
    {
	_checkRep();
	_rep->addMethod(x);
	return *this;
    }

    /** CIMMethod findMethod - Located the method object defined by the
	name input
	@param String representing the name of the method to be found
	@return Position of the method object in the class to be used in
	subsequent getmethod, etc. operations
    */
    Uint32 findMethod(const String& name)
    {
	_checkRep();
	return _rep->findMethod(name);
    }

    /// CIMMethod findMethod - ATTN:
    Uint32 findMethod(const String& name) const
    {
	_checkRep();
	return _rep->findMethod(name);
    }

    /** CIMMethod getMethod - Gets the method object defined by the
	input parameter.
	@param   ATTN:
	@ method object representing the method defined
	ATTN: Error???
    */
    CIMMethod getMethod(Uint32 pos)
    {
	_checkRep();
	return _rep->getMethod(pos);
    }

    /// CIMMethod getMethod - ATTN:
    CIMConstMethod getMethod(Uint32 pos) const
    {
	_checkRep();
	return _rep->getMethod(pos);
    }

    /** CIMMethod getMethodCount - Count of the number of methods in the class
	@return integer representing the number of methods in the class
    */
    Uint32 getMethodCount() const
    {
	_checkRep();
	return _rep->getMethodCount();
    }

    /** CIMMethod Resolve -  Resolve the class: inherit any properties and
	qualifiers. Make sure the superClass really exists and is consistent
	with this class. Also set the propagated flag class-origin for each
	class feature.
	ATTN: explain why this here
    */
    void resolve(
	DeclContext* declContext,
	const String& nameSpace)
    {
	_checkRep();
	_rep->resolve(declContext, nameSpace);
    }

    /// operator - ATTN:
    operator int() const { return _rep != 0; }

    /// CIMMethod toXML
    void toXml(Array<Sint8>& out) const
    {
	_checkRep();
	_rep->toXml(out);
    }

    /// CIMMethod print 
    void print(std::ostream &o=std::cout) const
    {
	_checkRep();
	_rep->print(o);
    }

    /** CIMMethod identical -  Compares with another class
	ATTN: Clarify exactly what identical means
	@parm Class object for the class to be compared
	@return True if the classes are identical
    */
    Boolean identical(const CIMConstClass& x) const;

    /// CIMMethod clone - ATTN:
    CIMClass clone() const
    {
	return CIMClass(_rep->clone());
    }

    void getKeyNames(Array<String>& keyNames) const
    {
	_checkRep();
	_rep->getKeyNames(keyNames);
    }

private:

    CIMClass(CIMClassRep* rep) : _rep(rep)
    {
    }

    void _checkRep() const
    {
	if (!_rep)
	    throw UnitializedHandle();
    }

    CIMClassRep* _rep;
    friend class CIMConstClass;
};

/** CIMConstClass - ATTN: define this.

*/

class PEGASUS_COMMON_LINKAGE CIMConstClass
{
public:

    CIMConstClass() : _rep(0)
    {

    }

    CIMConstClass(const CIMConstClass& x)
    {
	Inc(_rep = x._rep);
    }

    CIMConstClass(const CIMClass& x)
    {
	Inc(_rep = x._rep);
    }

    CIMConstClass& operator=(const CIMConstClass& x)
    {
	if (x._rep != _rep)
	{
	    Dec(_rep);
	    Inc(_rep = x._rep);
	}
	return *this;
    }

    CIMConstClass& operator=(const CIMClass& x)
    {
	if (x._rep != _rep)
	{
	    Dec(_rep);
	    Inc(_rep = x._rep);
	}
	return *this;
    }

    // Throws IllegalName if className argument not legal CIM identifier.

    CIMConstClass(
	const String& className,
	const String& superClassName = String())
    {
	_rep = new CIMClassRep(className, superClassName);
    }

    ~CIMConstClass()
    {
	Dec(_rep);
    }

    Boolean isAssociation() const
    {
	_checkRep();
	return _rep->isAssociation();
    }

    Boolean isAbstract() const
    {
	_checkRep();
	return _rep->isAbstract();
    }

    const String& getClassName() const
    {
	_checkRep();
	return _rep->getClassName();
    }

    const String& getSuperClassName() const
    {
	_checkRep();
	return _rep->getSuperClassName();
    }

    Uint32 findQualifier(const String& name) const
    {
	_checkRep();
	return _rep->findQualifier(name);
    }

    CIMConstQualifier getQualifier(Uint32 pos) const
    {
	_checkRep();
	return _rep->getQualifier(pos);
    }

    Uint32 getQualifierCount() const
    {
	_checkRep();
	return _rep->getQualifierCount();
    }

    Uint32 findProperty(const String& name) const
    {
	_checkRep();
	return _rep->findProperty(name);
    }

    CIMConstProperty getProperty(Uint32 pos) const
    {
	_checkRep();
	return _rep->getProperty(pos);
    }

    Uint32 getPropertyCount() const
    {
	_checkRep();
	return _rep->getPropertyCount();
    }

    Uint32 findMethod(const String& name) const
    {
	_checkRep();
	return _rep->findMethod(name);
    }

    CIMConstMethod getMethod(Uint32 pos) const
    {
	_checkRep();
	return _rep->getMethod(pos);
    }

    Uint32 getMethodCount() const
    {
	_checkRep();
	return _rep->getMethodCount();
    }

    operator int() const { return _rep != 0; }

    void toXml(Array<Sint8>& out) const
    {
	_checkRep();
	_rep->toXml(out);
    }

    void print(std::ostream &o=std::cout) const
    {
	_checkRep();
	_rep->print(o);
    }

    Boolean identical(const CIMConstClass& x) const
    {
	x._checkRep();
	_checkRep();
	return _rep->identical(x._rep);
    }

    CIMClass clone() const
    {
	return CIMClass(_rep->clone());
    }

    void getKeyNames(Array<String>& keyNames) const
    {
	_checkRep();
	_rep->getKeyNames(keyNames);
    }

private:

    void _checkRep() const
    {
	if (!_rep)
	    throw UnitializedHandle();
    }

    CIMClassRep* _rep;

    friend class CIMClassRep;
    friend class CIMClass;
    friend class CIMInstanceRep;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMClass_h */
