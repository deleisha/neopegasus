//BEGIN_LICENSE
//
// Copyright (c) 2000 The Open Group, BMC Software
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
//END_LICENSE
//BEGIN_HISTORY
//
// Author: Mike Brasher
//
// $Log: CIMOperations.h,v $
// Revision 1.1  2001/02/18 18:39:06  mike
// new
//
// Revision 1.2  2001/02/18 02:49:00  mike
// Removed ugly workarounds for MSVC++ 5.0 (using SP3 now)
//
// Revision 1.1  2001/02/16 02:07:06  mike
// Renamed many classes and headers (using new CIM prefixes).
//
// Revision 1.11  2001/02/13 07:00:18  mike
// Added partial createInstance() method to repository.
//
// Revision 1.10  2001/02/11 05:42:33  mike
// new
//
// Revision 1.9  2001/02/08 00:31:51  mike
// Clean up on CIMOperations class.
// Reformmatted documentation.
//
// Revision 1.8  2001/02/06 17:04:03  karl
// add documentation
//
// Revision 1.7  2001/02/02 21:59:45  karl
// fix enuminstances and some DOC++
//
// Revision 1.6  2001/01/31 08:20:51  mike
// Added dispatcher framework.
// Added enumerateInstanceNames.
//
// Revision 1.5  2001/01/29 08:03:12  karl
// DOC++ Documentation
//
// Revision 1.4  2001/01/28 10:48:22  karl
// Doc++ Documentation
//
// Revision 1.3  2001/01/15 04:31:44  mike
// worked on resolve scheme
//
// Revision 1.2  2001/01/15 04:19:08  karl
// Add Class Documentation
//
// Revision 1.1.1.1  2001/01/14 19:53:00  mike
// Pegasus import
//
//END_HISTORY

#ifndef Pegasus_Operations_h
#define Pegasus_Operations_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMQualifierDecl.h>


PEGASUS_NAMESPACE_BEGIN

/** @name The CIMOperations Class.
    This class defines the external client operations for Pegasus These
    operations are based on the WBEM operations defined in the DMTF CIM
    CIMOperations over HTTP operations Specification. They duplicate the names,
    functions, and parameters of those CIMOperations. Each WBEM operation is a
    method in this class.

    This library (operations.h) defines the external APIs for the Pegasus MSB.
    These are synchronous calls and match the operations defined by the DMTF in
    the document "Specification for CIM CIMOperations over HTTP" Version 1.0 with
    errata

    The CIMOperations class is used by other classes such as the client, provider,
    and repository classes to instantiate the operations interfaces.
*/
class PEGASUS_COMMON_LINKAGE CIMOperations
{
public:

    typedef Array<String> StringArray;

    virtual ~CIMOperations();

    /** The <TT>GetClass</TT> method returns a single CIM Class from the
	target Namespace where the ClassName input parameter defines the name of
	the class to be retrieved.

	@param NameSpace The NameSpace parameter is a string that defines the
	target namespace \Ref{NAMESPACE}

	@param ClassName The ClassName input parameter defines the name of the
	Class to be retrieved.

	@param LocalOnly If the <TT>LocalOnly</TT> input parameter is true, this
	specifies that only CIM Elements (properties, methods and qualifiers)
	overriden within the definition of the Class are returned.  If false,
	all elements are returned.  This parameter therefore effects a CIM
	Server-side mechanism to filter certain elements of the returned object
	based on whether or not they have been propagated from the parent Class
	(as defined by the PROPAGATED attribute).

	@param IncludeQualifiers If the <TT>IncludeQualifiers</TT> input
	parameter is true, this specifies that all Qualifiers for that Class
	(including Qualifiers on the Class and on any returned Properties,
	Methods or CIMMethod Parameters) MUST be included as <QUALIFIER> elements
	in the response.  If false no <QUALIFIER> elements are present in the
	returned Class.

	@param IncludeClassOrigin If the <TT>IncludeClassOrigin</TT> input
	parameter is true, this specifies that the CLASSORIGIN attribute MUST be
	present on all appropriate elements in the returned Class. If false, no
	CLASSORIGIN attributes are present in the returned Class.

	@param PropertyList If the <TT>PropertyList</TT> input parameter is not
	NULL, the members of the array define one or more CIMProperty names.  The
	returned Class MUST NOT include elements for any Properties missing from
	this list. Note that if LocalOnly is specified as true this acts as an
	additional filter on the set of Properties returned (for example, if
	CIMProperty A is included in the PropertyList but LocalOnly is set to true
	and A is not local to the requested Class, then it will not be included
	in the response). If the PropertyList input parameter is an empty array
	this signifies that no Properties are included in the response. If the
	PropertyList input parameter is NULL this specifies that all Properties
	(subject to the conditions expressed by the other parameters) are
	included in the response.

	If the <TT>PropertyList</TT> contains duplicate elements, the Server
	MUST ignore the duplicates but otherwise process the request normally.
	If the PropertyList contains elements which are invalid CIMProperty names
	for the target Class, the Server MUST ignore such entries but otherwise
	process the request normally.

	@return If successful, the return value is a single CIM Class.

	If unsuccessful, one of the following status codes MUST be returned by
	this method, where the first applicable error in the list (starting with
	the first element of the list, and working down) is the error returned.
	Any additional method-specific interpretation of the error in is given
	in parentheses.
	<UL>
		<LI>CIM_ERR_ACCESS_DENIED
		<LI>CIM_ERR_INVALID_NAMESPACE
		<LI>CIM_ERR_INVALID_PARAMETER (including missing,
		duplicate,unrecognized or otherwise
			incorrect parameters)
		<LI>CIM_ERR_NOT_FOUND (the request CIM Class does not exist in
		the
		specified
			namespace)
		<LI>CIM_ERR_FAILED (some other unspecified error occurred)</LI>
	</UL>
    */
    virtual CIMClass getClass(
	    const String& nameSpace,
	    const String& className,
	    Boolean localOnly = true,
	    Boolean includeQualifiers = true,
	    Boolean includeClassOrigin = false,
	    const Array<String>& propertyList = StringArray()) = 0;

    /** The <<TT>GetInstance</TT> method returns a single CIM Instance from
	the target Namespace based on the InstanceName parameter provided.

	@param NameSpace The NameSpace parameter is a string that defines the
	target namespace \Ref{NAMESPACE}

	@param InstanceName The InstanceName input parameter defines the name of
	the Instance to be retrieved.

	@param LocalOnly If the <TT>LocalOnly</TT> input parameter is true, this
	specifies that only elements (properties and qualifiers) overriden
	within the definition of the Instance are returned.  If false, all
	elements are returned.  This parameter therefore effects a CIM
	Server-side mechanism to filter certain elements of the returned object
	based on whether or not they have been propagated from the parent Class
	(as defined by the PROPAGATED attribute).

	@param IncludeQualifiersIf the <TT>IncludeQualifiers</TT> input
	parameter is true, this specifies that all Qualifiers for that Instance
	(including Qualifiers on the Instance and on any returned Properties)
	MUST be included as <QUALIFIER> elements in the response.  If false no
	<QUALIFIER> elements are present in the returned Instance.

	@param IncludeClassOrigin If the <TT>IncludeClassOrigin</TT> input
	parameter is true, this specifies that the CLASSORIGIN attribute MUST
	be present on all appropriate elements in the returned Instance. If
	false, no CLASSORIGIN attributes are present in the returned instance.

	@param PropertyList If the <TT>PropertyList</TT> input parameter is not
	NULL, the members of the array define one or more CIMProperty names.  The
	returned Instance MUST NOT include elements for any Properties missing
	from this list. Note that if LocalOnly is specified as true this acts
	as an additional filter on the set of Properties returned (for example,
	if CIMProperty A is included in the PropertyList but LocalOnly is set to
	true and A is not local to the requested Instance, then it will not be
	included in the response). If the PropertyList input parameter is an
	empty array this signifies that no Properties are included in the
	response. If the PropertyList input parameter is NULL this specifies
	that all Properties (subject to the conditions expressed by the other
	parameters) are included in the response.

	If the <TT>PropertyList</TT> contains duplicate elements, the Server
	MUST ignore the duplicates but otherwise process the request normally.
	If the PropertyList contains elements which are invalid CIMProperty names
	for the target Instance, the Server MUST ignore such entries but
	otherwise process the request normally.

	@return If successful, the return value is a single CIM Instance.

	If unsuccessful, one of the following status codes MUST be returned by
	this method, where the first applicable error in the list (starting
	with the first element of the list, and working down) is the error
	returned. Any additional method-specific interpretation of the error in
	is given in parentheses.

	<UL>
	    <LI>CIM_ERR_ACCESS_DENIED
	    <LI>CIM_ERR_INVALID_NAMESPACE
	    <LI>CIM_ERR_INVALID_PARAMETER (including missing, duplicate,
		unrecognized or otherwise incorrect parameters)
	    <LI>CIM_ERR_INVALID_CLASS (the CIM Class does not exist in the
		specified namespace)
	    <LI>CIM_ERR_NOT_FOUND (the CIM Class does exist, but the requested 
		CIM Instance does not exist in the specified namespace)
	    <LI>CIM_ERR_FAILED (some other unspecified error occurred)
	    </LI>
	</UL>
    */
    virtual CIMInstance getInstance(
	    const String& nameSpace,
	    const CIMReference& instanceName,
	    Boolean localOnly = true,
	    Boolean includeQualifiers = false,
	    Boolean includeClassOrigin = false,
	    const Array<String>& propertyList = StringArray()) = 0;


    /** The <TT>DeleteClass</TT> method deletes a single CIM Class from the
	target Namespace.

	@param NameSpace The NameSpace parameter is a string that defines the
	target namespace \Ref{NAMESPACE}

	@param ClassName The ClassName input parameter defines the name of the
	Class to be deleted.

	@return If successful, the specified Class (including any subclasses 
	and any instances) MUST have been removed by the CIM Server.  The 
	operation MUST fail if any one of these objects cannot be deleted.

	If unsuccessful, one of the following status codes MUST be returned by 
	this method, where the first applicable error in the list (starting 
	with the first element of the list, and working down) is the error 
	returned. Any additional method-specific interpretation of the error 
	in is given in parentheses.

	<UL>
	    <LI>CIM_ERR_ACCESS_DENIED
	    <LI>CIM_ERR_NOT_SUPPORTED
	    <LI>CIM_ERR_INVALID_NAMESPACE
	    <LI>CIM_ERR_INVALID_PARAMETER (including missing, duplicate,
		unrecognized or otherwise incorrect parameters)
	    <LI>CIM_ERR_NOT_FOUND (the CIM Class to be deleted does not exist)
	    <LI>CIM_ERR_CLASS_HAS_CHILDREN (the CIM Class has one or more
		subclasses which cannot be deleted)
	    <LI>CIM_ERR_CLASS_HAS_INSTANCES (the CIM Class has one or more
		instances which cannot be deleted)
	    <LI>CIM_ERR_FAILED (some other unspecified error occurred)
	</UL>
    */
    virtual void deleteClass(
        const String& nameSpace,
        const String& className) = 0;

    /** The <TT>DeleteInstance</TT> operation deletes a single CIM Instance 
	from the target Namespace.

	@param NameSpace The NameSpace parameter is a string that defines the 
	target namespace \Ref{NAMESPACE}

	@param InstanceName The InstanceName input parameter defines the name 
	(model path) of the Instance to be deleted.

	@return If successful, the specified Instance MUST have been removed 
	by the CIM Server.

	If unsuccessful, one of the following status codes MUST be returned by 
	this method, where the first applicable error in the list (starting 
	with the first element of the list, and working down) is the error 
	returned. Any additional method-specific interpretation of the error in
	is given in parentheses.

	<UL>
	    <LI>CIM_ERR_ACCESS_DENIED
	    <LI>CIM_ERR_NOT_SUPPORTED
	    <LI>CIM_ERR_INVALID_NAMESPACE
	    <LI>CIM_ERR_INVALID_PARAMETER (including missing, duplicate,
		unrecognized or otherwise incorrect parameters)
	    <LI>CIM_ERR_INVALID_CLASS (the CIM Class does not exist in the
		specified namespace)
	    <LI>CIM_ERR_NOT_FOUND (the CIM Class does exist, but the requested 
		CIM Instance does not exist in the specified namespace)
	    <LI>CIM_ERR_FAILED (some other unspecified error occurred)</LI>
	</UL>
    */
    virtual void deleteInstance(
        const String& nameSpace,
        const CIMReference& instanceName) = 0;

    //BOOKMARK

    /** The <TT>createClass</TT> method creates a single CIM Class in
    the target Namespace. The Class MUST NOT already exist. The NewClass input
    parameter defines the new Class.  The proposed definition MUST be a correct
    Class definition according to the CIM specification.

    In processing the creation of the new Class, the following rules MUST be
    conformed to by the CIM Server:

    Any CLASSORIGIN and PROPAGATED attributes in the NewClass MUST be ignored by
    the Server. If the new Class has no Superclass, the NewClass parameter
    defines a new base Class. The Server MUST ensure that all Properties and
    Methods of the new Class have a CLASSORIGIN attribute whose value is the
    name of the new Class. If the new Class has a Superclass, the NewClass
    parameter defines a new Subclass of that Superclass. The Superclass MUST
    exist. The Server MUST ensure that:

    <UL>
        <LI>Any Properties, Methods or Qualifiers in the Subclass not defined in
        the Superclass are created as new elements of the Subclass. In
        particular the Server MUST set the CLASSORIGIN attribute on the new
        Properties and Methods to the name of the Subclass, and ensure that all
        other Properties and Methods preserve their CLASSORIGIN attribute value
        from that defined in the Superclass

        If a CIMProperty is defined in the Superclass and in the Subclass, the
        value assigned to that property in the Subclass (including NULL) becomes
        the default value of the property for the Subclass. If a CIMProperty or
        CIMMethod of the Superclass is not specified in the Subclass, then that
        CIMProperty or CIMMethod is inherited without modification by the Subclass

        <LI>Any Qualifiers defined in the Superclass with a TOSUBCLASS attribute
        value of true MUST appear in the resulting Subclass. Qualifiers in the
        Superclass with a TOSUBCLASS attribute value of false MUST NOT be
        propagated to the Subclass . Any CIMQualifier propagated from the
        Superclass cannot be modified in the Subclass if the OVERRIDABLE
        attribute of that CIMQualifier was set to false in the Superclass. It is a
        Client error to specify such a CIMQualifier in the NewClass with a
        different definition to that in the Superclass (where definition
        encompasses the name, type and flavor attribute settings of the
        <QUALIFIER> element, and the value of the CIMQualifier).
	</LI>
    </UL>

    @param NameSpace The NameSpace parameter is a string that defines the target
    namespace \Ref{NAMESPACE}

    @parm NewClass The NewClass input parameter defines the new Class.

    @return If successful, the specified Class MUST have been created by the CIM
    Server.

    If unsuccessful, one of the following status codes MUST be returned by this
    method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.
    <UL>
         <LI>CIM_ERR_ACCESS_DENIED
         <LI>CIM_ERR_NOT_SUPPORTED
         <LI>CIM_ERR_INVALID_NAMESPACE
         <LI>CIM_ERR_INVALID_PARAMETER (including missing, duplicate,
         unrecognized or otherwise incorrect parameters)
         <LI>CIM_ERR_ALREADY_EXISTS (the CIM Class already exists)
         <LI>CIM_ERR_INVALID_SUPERCLASS (the putative CIM Class declares a
         non-existent superclass)
         <LI>CIM_ERR_FAILED (some other unspecified error occurred)</LI>
    </UL>
    */
    virtual void createClass(

		const String& nameSpace,
		CIMClass& newClass) = 0;

    /** The <TT>createInstance</TT> method creates a single CIM
    Instance in the target Namespace. The Instance MUST NOT already exist.

    In processing the creation of the new Instance, the following rules MUST be
    conformed to by the CIM Server:

    Any CLASSORIGIN and PROPAGATED attributes in the NewInstance MUST be ignored
    by the Server.

    The Server MUST ensure that:

    <UL>
        <LI>Any Qualifiers in the Instance not defined in the Class are created
        as new elements of the Instance.
        <LI>All Properties of the Instance preserve their CLASSORIGIN attribute
        value from that defined in the Class.
        <LI>If a CIMProperty is specified in the ModifiedInstance parameter, the
        value assigned to that property in the Instance (including NULL) becomes
        the value of the property for the Instance. Note that it is a Client
        error to specify a CIMProperty that does not belong to the Class.
        <LI>If a CIMProperty of the Class is not specified in the Instance, then
        that CIMProperty is inherited without modification by the Instance.
        <LI>Any Qualifiers defined in the Class with a TOINSTANCE attribute
        value of true appear in the Instance. Qualifiers in the
        Class with a TOINSTANCE attribute value of false MUST NOT be propagated
        to the Instance.
        <LI>Any CIMQualifier propagated from the Class cannot be modified in the
        Instance if the OVERRIDABLE attribute of that CIMQualifier was set to false
        in the Class. It is a Client error to specify such a CIMQualifier in the
        NewInstance with a different definition to that in the Class (where
        definition encompasses the name, type and flavor attribute settings of
        the <QUALIFIER> element, and the value of the CIMQualifier).
    </UL>

    @param NameSpace The NameSpace parameter is a string that defines the target
    namespace \Ref{NAMESPACE}

    @param newInstance The <TT>newInstance</TT> input parameter defines the new
    Instance. The proposed definition MUST be a correct Instance definition for
    the underlying CIM Class according to the CIM specification.

    @return If successful, the return value defines the object path of the new
    CIM Instance relative to the target Namespace (i.e. the Model Path as
    defined by the CIM specification), created by the CIM Server.  It is
    returned in case one or more of the new keys of the Instance are allocated
    dynamically during the creation process rather than specified in the
    request.

    If unsuccessful, one of the following status codes MUST be returned by this
    method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.

    <UL>
        <LI>CIM_ERR_ACCESS_DENIED
        <LI>CIM_ERR_NOT_SUPPORTED
        <LI>CIM_ERR_INVALID_NAMESPACE
        <LI>CIM_ERR_INVALID_PARAMETER (including missing, duplicate,
        	unrecognized or otherwise incorrect parameters)
        <LI>CIM_ERR_INVALID_CLASS (the CIM Class of which this is to be a new
        	Instance does not exist)
        <LI>CIM_ERR_ALREADY_EXISTS (the CIM Instance already exists)
        <LI>CIM_ERR_FAILED (some other unspecified error occurred)
    </UL>
    */
    virtual void createInstance(
	const String& nameSpace,
	CIMInstance& newInstance) = 0;


    /** The <TT>modifyClass</TT> method modifies an existing CIM Class in the
    target Namespace.  The Class MUST already exist. The <TT>ModifiedClass</TT>
    input parameter defines the set of changes (which MUST be  correct
    amendments to the CIM Class as defined by the CIM Specification) to be made
    to the current class definition.

    In processing the  modifcation of the Class, the following rules MUST be
    conformed to by the CIM Server.

    <UL>
      <LI>Any <TT>CLASSORIGIN</TT> and <TT>PROPAGATED</TT> attributes in the
      <TT>ModifiedClass</TT> MUST be ignored by the Server.
      <LI>If the  modified Class has no Superclass,
      the<TT>ModifiedClass</TT> parameter defines modifications to a base Class.
      The Server MUST ensure that:
      <UL>
        <LI>All Properties and Methods of the modified Class have a
        <TT>CLASSORIGIN</TT> attribute whose value is the name of this Class.
        <LI>Any Properties, Methods or Qualifiers in the existing Class
        definition which do not appear in the   <FONT face="Courier
        New">ModifiedClass</TT> parameter are removed from the resulting
        modified Class.</LI>
      </UL>
      <LI>If the  modified Class has a
      Superclass,the <TT>ModifiedClass</TT> parameter
      defines modifications to a Subclass of that Superclass. The
      Superclass MUST exist, and the Client MUST NOT change the name of the
      Superclass in the modified Subclass. The Server MUST ensure that:
      <UL>
        <LI>Any Properties, Methods or Qualifiers in the Subclass not
        defined in the Superclass are created as elements of the Subclass. In
        particular the Server MUST set the <TT>CLASSORIGIN</TT> attribute on the
        new Properties and Methods to the name of the Subclass, and MUST ensure
        that all other Properties and Methods preserve their
        <TT>CLASSORIGIN</TT> attribute value from that defined in the
        Superclass.
        <LI>Any CIMProperty, CIMMethod or CIMQualifier previously defined in the Subclass
        but not defined in the Superclass, and which is not present in the
        <TT>ModifiedClass</TT> parameter, is removed from the Subclass.
        <LI>If a CIMProperty is specified in the <TT>ModifiedClass</TT>
        parameter, the value assigned to that property therein (including
        NULL) becomes the default value of the property for the Subclass.
        <LI>If a CIMProperty or CIMMethod of the Superclass is not specified in the
        Subclass, then that CIMProperty or CIMMethod is inherited
        without modification by the Subclass (so that any previous changes to
        such an Element in the Subclass are lost).
        <LI>If a CIMQualifier in the Superclass is not specified in the
        Subclass, and the CIMQualifier is defined in the Superclass with a
        <TT>TOSUBCLASS</TT> attribute value of <TT>true</TT>, then the CIMQualifier
        MUST still be present in the resulting modified Subclass (it is not
        possible to remove a propagated CIMQualifier from a Subclass).
        <LI>Any CIMQualifier propagated from the Superclass cannot be
        modified in the Subclass if the <TT>OVERRIDABLE</TT> attribute of
        that CIMQualifier was set to <TT>false</TT> in the Superclass. It is a
        Client error to specify such a CIMQualifier in the <TT>ModifiedClass</TT>
        with a different definition to that in the Superclass (where definition
        encompasses the name, type and flavor attribute settings of the
        <TT>&lt;QUALIFIER&gt;</TT> element, and the value of the CIMQualifier).
        <LI>Any Qualifiers defined in the Superclass with a <TT>TOSUBCLASS</TT>
        attribute value of  <TT>false</TT> MUST NOT be propagated to the
        Subclass.</LI> </UL>
       </LI></UL>

    @param NameSpace The NameSpace parameter is a string that defines the target
    namespace \Ref{NAMESPACE}

    @param ModifiedClass The <TT>ModifiedClass</TT>
    input parameter defines the set of changes (which MUST be correct
    amendments to the CIM Class as defined by the CIM Specification) to be made
    to the current class definition.


    @return If successful, the specified Class MUST have been updated by
    the CIM Server.

    The request to modify the Class MUST fail if the Server cannot update any
    existing Subclasses or Instances of that Class in a consistent manner.

    @return If unsuccessful, one of the following status codes MUST be
    returned by this method, where the first applicable error in the list
    (starting with the first element of the list, and working down) is the
    error returned. Any additional method-specific interpretation of the error
    in is given in parentheses.

    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE
      <LI>CIM_ERR_INVALID_PARAMETER (including missing,
        duplicate, unrecognized or otherwise incorrect parameters)
      <LI>CIM_ERR_NOT_FOUND (the CIM Class does not
        exist)
      <LI>CIM_ERR_INVALID_SUPERCLASS (the putative CIM Class
        declares a non-existent or incorrect superclass)
      <LI>CIM_ERR_CLASS_HAS_CHILDREN (the modification could
        not be performed because it was not possible to update the subclasses of
        the
        Class in a consistent fashion)
      <LI>CIM_ERR_CLASS_HAS_INSTANCES (the modification could
        not be performed because it was not possible to update
        the instances of
        the Class in a consistent fashion)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)
     </LI></UL>
    */
    virtual void modifyClass(
        const String& nameSpace,
        CIMClass& modifiedClass) = 0;

    /** The <TT>modifyInstance</TT> method is used to modify an existing CIM
    Instance in the target Namespace.

    @param NameSpace The NameSpace parameter is a string that defines the target
    namespace \Ref{NAMESPACE}

    @param ModifiedInstance The <TT>ModifiedInstance</TT> input parameter
    identifies the name of the Instance to be modified, and defines the set of
    changes (which MUST be correct amendments to the Instance as
    defined by the CIM Specification) to be made to the current Instance
    definition.

    In processing the modifcation of the Instance, the following rules MUST
    be conformed to by the CIM Server:

    <UL>
      <LI>Any <TT>CLASSORIGIN</TT> and <TT>PROPAGATED</TT> attributes in the
      <TT>ModifiedInstance</TT> MUST be ignored by the Server.
      <LI>The Class MUST exist, and the Client MUST NOT change
      the name of the Class in the modified Instance. The Server MUST ensure
      that:
      <UL>
        <LI>Any Qualifiers in the Instance not defined in
        the Class are created as new elements of the Instance.
        <LI>All Properties of the Instance preserve their
        <TT>CLASSORIGIN</TT> attribute value from that defined in the Class.
        <LI>Any CIMQualifier previously defined in the Instance but not
        defined in the Class, and which is not present in the
        <TT>ModifiedInstance</TT> parameter, is removed from the Instance.

        <LI>If a CIMProperty is specified in the <TT>ModifiedInstance</TT>
        parameter, the value assigned to that property therein
        (including NULL) becomes the value of the property for the Instance.
        Note that it is a Client error to specify a CIMProperty that does not
        belong to the Class.

        <LI>If a CIMProperty of the Class is not specified in the Instance,
        then that CIMProperty is inherited without modification by the Instance (so
        that any previous changes to that CIMProperty in the Instance are
        lost).
        <LI>Any Qualifiers defined in the Class with a <TT>TOINSTANCE</TT>
        attribute value of <TT>true</TT> appear in the Instance (it is not
        possible remove a propagated CIMQualifier from an Instance. Qualifiers in
        the Class with a <TT>TOINSTANCE</TT> attribute value of <TT>false</TT>
        MUST NOT be propagated to the Instance.
        <LI>Any CIMQualifier propagated from the Class cannot be modified by the
        Server if the <TT>OVERRIDABLE</TT> attribute of that CIMQualifier was set
        to <TT>false</TT> in the Class. It is a Client error to specify such a
        CIMQualifier in the <TT>ModifiedInstance</TT> with a different definition
        to that in the Class (where definition encompasses the name,
        type and flavor attribute settings of the
        <TT>&lt;QUALIFIER&gt;</TT> element, and the value of the CIMQualifier).
        <LI>Any CIMQualifier propagated from the Class cannot be modified in
        the Instance if the <TT>OVERRIDABLE</TT> attribute of that CIMQualifier was
        set to <TT>false</TT> in the Class. It is a Client error to specify such
        a CIMQualifier in the <TT>ModifiedInstance</TT> with a different definition
        to that in the Class (where definition encompasses the name, type and
        flavor attribute settings of the <TT>&lt;QUALIFIER&gt;</TT>
        element, and the value of the CIMQualifier).</LI>
        </UL>
      </LI></UL>

    @return If successful, the specified Instance MUST have been updated by the
    CIM Server.

    If unsuccessful, one of the following status codes MUST be returned by
    this method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses

    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE
      <LI>CIM_ERR_INVALID_PARAMETER (including missing,
        duplicate, unrecognized or otherwise incorrect parameters)
      <LI>CIM_ERR_INVALID_CLASS (the CIM Class of which this is
        to be a new Instance does not exist)
      <LI>CIM_ERR_NOT_FOUND (the CIM Instance does not exist)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)</LI></UL>
    */
    virtual void modifyInstance(

		const String& nameSpace,
		const CIMInstance& modifiedInstance) = 0;


    /** The <TT>enumerateClasses</TT> method is used to enumerate subclasses of
    a CIM Class in the target Namespace.

    @param NameSpace The NameSpace parameter is a string that defines the target
    namespace \Ref{NAMESPACE}

    @param className The <TT>ClassName</TT> input parameter defines the Class
    that is the basis for the enumeration.

    @param DeepInheritance If the <TT>DeepInheritance</TT> input
    parameter is <TT>true</TT>, this specifies that all subclasses of
    the specified Class should be returned (if the <TT>ClassName</TT> input
    parameter is absent, this implies that all Classes in the target Namespace
    should be returned).  If <TT>false</TT>, only immediate child
    subclasses are returned (if the <TT>ClassName</TT> input parameter is
    NULL, this implies that all base Classes in the target Namespace should be
    returned).

    @param LocalOnly If the <TT>LocalOnly</TT> input parameter is
    <TT>true</TT>, it specifies that, for each returned Class, only elements
    (properties, methods and qualifiers) overriden within the definition of
    that Class are included.  If <TT>false</TT>, all elements are
    returned.  This parameter therefore effects a CIM Server-side mechanism
    to filter certain elements of the returned object based on whether or not
    they have been propagated from the parent Class (as defined by the
    <TT>PROPAGATED</TT> attribute).

    @param IncludeQualifiers If the <TT>IncludeQualifiers</TT> input parameter
    is <TT>true</TT>, this specifies that all Qualifiers for each Class
    (including Qualifiers on the Class and on any returned Properties, Methods
    or CIMMethod Parameters) MUST be included as <TT>&lt;QUALIFIER&gt;</TT>
    elements in the response.  If false no
    <TT>&lt;QUALIFIER&gt;</TT> elements are present in each returned
    Class.

    @param IncludeClassOrigin If the <TT>IncludeClassOrigin</TT> input
    parameter is <TT>true</TT>, this specifies that the <TT>CLASSORIGIN</TT>
    attribute MUST be present on all appropriate elements in each returned
    Class. If false, no <TT>CLASSORIGIN</TT> attributes are present in each
    returned Class.

    @return If successful, the method returns zero or more Classes that meet the
    required criteria.

    If unsuccessful, one of the following status codes MUST be returned by
    this method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.

    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE
      <LI>CIM_ERR_INVALID_PARAMETER (including missing,
        duplicate, unrecognized or otherwise incorrect parameters)
      <LI>CIM_ERR_INVALID_CLASS (the CIM Class that is the
        basis for this enumeration does not exist)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)
      </LI>
    </UL>
    */
    virtual Array<CIMClass> enumerateClasses(
		const String& nameSpace,
		const String& className = String::EMPTY,
		Boolean deepInheritance = false,
		Boolean localOnly = true,
		Boolean includeQualifiers  = true,
		Boolean includeClassOrigin = false) = 0;


    /** The <TT>enumerateClassNames</TT> operation is used to enumerate the
    names of subclasses of a CIM Class in the target Namespace.

    @param NameSpace The NameSpace parameter is a string that defines the
    target namespace \Ref{NAMESPACE}

    @param className The <TT>ClassName</TT> input parameter defines the Class
    that is the basis for the enumeration.

    @param DeepInheritance If the DeepInheritance input parameter is true, this
    specifies that the names of all subclasses of the specified Class should be
    returned (if the ClassName input parameter is absent, this implies that the
    names of all Classes in the target Namespace should be returned).  If false,
    only the names of immediate child subclasses are returned (if the ClassName
    input parameter is NULL, this implies that the names of all base Classes in
    the target Namespace should be returned).

    @return If successful, the method returns zero or more names of Classes that
    meet the requested criteria.

    If unsuccessful, one of the following status codes MUST be returned by this
    method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.

    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE
      <LI>CIM_ERR_INVALID_PARAMETER (including missing,
        duplicate, unrecognized or otherwise incorrect parameters)
      <LI>CIM_ERR_INVALID_CLASS (the CIM Class that is the
        basis for this enumeration does not exist)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)</LI>
    </UL>
    */
    virtual Array<String> enumerateClassNames(
    const String& nameSpace,
    const String& className = String::EMPTY,
    Boolean deepInheritance = false) = 0;


    /** The <TT>enumerateInstances</TT> method enumerates instances of a CIM
    Class in the target Namespace.

    @param NameSpace The NameSpace parameter is a string that defines the
    target namespace \Ref{NAMESPACE}

    @param ClassName The <TT>ClassName</TT> input parameter defines the
    Class that is the basis for the enumeration.    </P>

    @param LocalOnly If the <TT>LocalOnly</TT> input parameter is
    <TT>true</TT>, this specifies that, for each returned Instance,
    only elements (properties and qualifiers) overriden within the
    definition of that Instance are included.  If <TT>false</TT>,
    all elements are returned.  This parameter therefore effects a CIM
    Server-side mechanism to filter certain elements of the returned object
    based on whether or not they have been propagated from the parent
    Class (as defined by the <TT>PROPAGATED</TT> attribute).

    @param DeepInheritance If the <TT>DeepInheritance</TT> input
    parameter is <TT>true</TT>, this specifies that, for each
    returned Instance of the Class, all properties of the Instance MUST
    be present (subject to constraints imposed by the other
    parameters), including any which were added by subclassing the specified
    Class. If <TT>false</TT>, each returned Instance includes only
    properties defined for the specified Class.

    @param IncludeQualifiersIf the <TT>IncludeQualifiers</TT> input
    parameter is <TT>true</TT>, this specifies that all Qualifiers
    for each Instance (including Qualifiers on the Instance
    and on any returned Properties) MUST be included as
    <TT>&lt;QUALIFIER&gt;</TT> elements in the response.  If false no
    <TT>&lt;QUALIFIER&gt;</TT> elements are present in each
    returned Instance.

    @param IncludeClassOrigin If the <TT>IncludeClassOrigin</TT> input
    parameter is <TT>true</TT>, this specifies that the
    <TT>CLASSORIGIN</TT> attribute MUST be present on all appropriate
    elements in each returned Instance. If false, no
    <TT>CLASSORIGIN</TT> attributes are present in each returned
    Instance.

    @param PropertyList If the <TT>PropertyList</TT> input parameter is not
    <TT>NULL</TT>, the members of the array define one or more CIMProperty
    names.  Each returned Instance MUST NOT include elements
    for any Properties missing from this list.  Note that if
    <TT>LocalOnly</TT> is specified as <TT>true</TT> (or
    <TT>DeepInheritance</TT> is specified as <TT>false</TT>) this acts as an
    additional filter on the set of Properties returned (for example,
    if CIMProperty <TT>A</TT> is included in the
    <TT>PropertyList</TT> but <TT>LocalOnly</TT> is set to true and
    <TT>A</TT> is not local to a returned Instance, then it will not be
    included in that Instance). If the <TT>PropertyList</TT> input parameter
    is an empty array this signifies that no Properties are included in each
    returned Instance. If the <TT>PropertyList</TT> input parameter is
    NULL this specifies that all Properties (subject to the conditions
    expressed by the other parameters) are included in each returned
    Instance.

    If the <TT>PropertyList</TT> contains duplicate elements,
    the Server MUST ignore the duplicates but otherwise process the request
    normally.  If the <TT>PropertyList</TT> contains elements which are
    invalid CIMProperty names for any target Instance, the Server MUST
    ignore such entries but otherwise process the request normally.

    @return If successful, the method returns zero or more named
    Instances that meet the required criteria.

    If unsuccessful, one of the following status codes MUST be returned
    by this method, where the first applicable error in the list (starting
    with the first element of the list, and working down) is the error
    returned. Any additional method-specific interpretation of the error in
    is given in parentheses.

	<UL>
	  <LI>CIM_ERR_ACCESS_DENIED
	  <LI>CIM_ERR_NOT_SUPPORTED
	  <LI>CIM_ERR_INVALID_NAMESPACE
	  <LI>CIM_ERR_INVALID_PARAMETER (including missing,
		duplicate, unrecognized or otherwise incorrect parameters)
	  <LI>CIM_ERR_INVALID_CLASS (the CIM Class that is the
		basis for this enumeration does not exist)
	  <LI>CIM_ERR_FAILED (some other unspecified erroroccurred)</LI>
	</UL>
	*/

    virtual Array<CIMInstance> enumerateInstances(
        const String& nameSpace,
        const String& className,
        Boolean deepInheritance = true,
        Boolean localOnly = true,
        Boolean includeQualifiers = false,
        Boolean includeClassOrigin = false,
		const Array<String>& propertyList = StringArray()) = 0;

    /** The <TT>enumerateInstanceNames</TT> operation enumerates the
    names (model paths) of the instances of a CIM Class in the target Namespace.

    @param NameSpace The NameSpace parameter is a string that defines the target
    namespace \Ref{NAMESPACE}

    @param ClassName The ClassName input parameter defines the Class that is the
    basis for the enumeration.

    @return If successful, the method returns zero or more names of Instances
    (model paths) that meet the requsted criteria.

    If unsuccessful, one of the following status codes MUST be returned by this
    method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.

    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE
      <LI>CIM_ERR_INVALID_PARAMETER (including missing,
      duplicate, unrecognized or otherwise incorrect parameters)
      <LI>CIM_ERR_INVALID_CLASS (the CIM Class that is the
      basis for this enumeration does not exist)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)</LI>
     </UL>
    */

    virtual Array<CIMReference> enumerateInstanceNames(
        const String& nameSpace,
        const String& className) = 0;

    /** The <TT>execQuery</TT> is used to execute a query against the target
    Namespace.

    @param NameSpace The NameSpace parameter is a string that defines the target
    namespace \Ref{NAMESPACE}

    @param QueryLanguage The QueryLanguage input parameter defines the query
    language in which the Query parameter is expressed.

    @param The Query input parameter defines the query to be executed.

    @return If successful, the method returns zero or more CIM Classes or
    Instances that correspond to the results set of the query.

    If unsuccessful, one of the following status codes MUST be returned by this
    method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.

    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE
      <LI>CIM_ERR_INVALID_PARAMETER (including missing,
      duplicate, unrecognized or otherwise incorrect parameters)
      <LI>CIM_ERR_QUERY_LANGUAGE_NOT_SUPPORTED (the requested query language is
      not recognized)
      <LI>CIM_ERR_INVALID_QUERY (the query is not a valid query in the
      specified query language)
      <LI>CIM_ERR_FAILED (some other unspecified error ccurred)</LI>
     </UL>
    */

    virtual Array<CIMInstance> execQuery(
		const String& queryLanguage,
		const String& query) = 0;

    /** The <TT>Associators</TT> method enumerates CIM Objects
    (Classes or Instances) that are associated to a particular source CIM
    Object.

    @param NameSpace The NameSpace parameter is a string that defines the target
    namespace \Ref{NAMESPACE}

    @ObjectName The ObjectName input parameter defines the source CIM Object
    whose associated Objects are to be returned.  This may be either a Class
    name or Instance name (model path).

    @param AssocClass The AssocClass input parameter, if not NULL, MUST be a
    valid CIM Association Class name. It acts as a filter on the returned set of
    Objects by mandating that each returned Object MUST be associated to the
    source Object via an Instance of this Class or one of its subclasses.

    @param ResutlClass The ResultClass input parameter, if not NULL, MUST be a
    valid CIM Class name. It acts as a filter on the returned set of Objects by
    mandating that each returned Object MUST be either an Instance of this Class
    (or one of its subclasses) or be this Class (or one of its subclasses).

    @param Role The Role input parameter, if not NULL, MUST be a valid CIMProperty
    name. It acts as a filter on the returned set of Objects by mandating that
    each returned Object MUST be associated to the source Object via an
    Association in which the source Object plays the specified role (i.e. the
    name of the CIMProperty in the Association Class that refers to the source
    Object MUST match the value of this parameter).

    @param ResultRole The ResultRole input parameter, if not NULL, MUST be a
    valid CIMProperty name. It acts as a filter on the returned set of Objects by
    mandating that each returned Object MUST be associated to the source Object
    via an Association in which the returned Object plays the specified role
    (i.e. the name of the CIMProperty in the Association Class that refers to the
    returned Object MUST match the value of this parameter).

    @param IncludeQualifiers If the IncludeQualifiers input parameter is true,
    this specifies that all Qualifiers for each Object (including Qualifiers on
    the Object and on any returned Properties) MUST be included as <QUALIFIER>
    elements in the response.  If false no <QUALIFIER> elements are present in
    each returned Object.

    @param IncludeClassOrigin If the IncludeClassOrigin input parameter is true,
    this specifies that the CLASSORIGIN attribute MUST be present on all
    appropriate elements in each returned Object. If false, no CLASSORIGIN
    attributes are present in each returned Object.

    @parm PropertyList If the PropertyList input parameter is not NULL, the
    members of the array define one or more CIMProperty names.  Each returned
    Object MUST NOT include elements for any Properties missing from this list.
    Note that if LocalOnly is specified as true (or DeepInheritance is specified
    as false) this acts as an additional filter on the set of Properties
    returned (for example, if CIMProperty A is included in the PropertyList but
    LocalOnly is set to true and A is not local to a returned Instance, then it
    will not be included in that Instance). If the PropertyList input parameter
    is an empty array this signifies that no Properties are included in each
    returned Object. If the PropertyList input parameter is NULL this specifies
    that all Properties (subject to the conditions expressed by the other
    parameters) are included in each returned Object.

    If the PropertyList contains duplicate elements, the Server MUST ignore the
    duplicates but otherwise process the request normally.  If the PropertyList
    contains elements which are invalid CIMProperty names for any target Object,
    the Server MUST ignore such entries but otherwise process the request
    normally.

    Clients SHOULD NOT explicitly specify properties in the PropertyList
    parameter unless they have specified a non-NULL value for the ResultClass
    parameter.

    @return If successful, the method returns zero or more CIM Classes or
    Instances
    meeting the requested criteria.  Since it is possible for CIM Objects from
    different hosts or namespaces to be associated, each returned Object
    includes location information.

    If unsuccessful, one of the following status codes MUST be returned by this
    method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.

    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE
      <LI>CIM_ERR_INVALID_PARAMETER (including
      missing,duplicate, unrecognized or
        otherwise incorrect parameters)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)</LI>
    </UL>
    */

    virtual Array<CIMInstance> associators(
        const String& nameSpace,
        const CIMReference& objectName,
        const String& assocClass = String::EMPTY,
        const String& resultClass = String::EMPTY,
        const String& role = String::EMPTY,
        const String& resultRole = String::EMPTY,
        Boolean includeQualifiers = false,
        Boolean includeClassOrigin = false,
        const Array<String>& propertyList = StringArray()) = 0;

    /** The <TT>associatorNames</TT> operation is used to enumerate the names of
    CIM Objects (Classes or Instances) that are associated to a particular
    source CIM Object.

    @param NameSpace The NameSpace parameter is a string that defines the target
	namespace \Ref{NAMESPACE}

    The ObjectName input parameter defines the source CIM Object whose
    associated names are to be returned. This is either a Class name or Instance
    name (model path).

    The AssocClass input parameter, if not NULL, MUST be a valid CIM Association
    Class name. It acts as a filter on the returned set of names by mandating
    that each returned name identifies an Object that MUST be associated to the
    source Object via an Instance of this Class or one of its subclasses.

    The ResultClass input parameter, if not NULL, MUST be a valid CIM Class
    name. It acts as a filter on the returned set of names by mandating that
    each returned name identifies an Object that MUST be either an Instance of
    this Class (or one of its subclasses) or be this Class (or one of its
    subclasses).

    The Role input parameter, if not NULL, MUST be a valid CIMProperty name. It
    acts as a filter on the returned set of names by mandating that each
    returned name identifies an Object that MUST be associated to the source
    Object via an Association in which the source Object plays the specified
    role (i.e. the name of the CIMProperty in the Association Class that refers to
    the source Object MUST match the value of this parameter).

    The ResultRole input parameter, if not NULL, MUST be a valid CIMProperty name.
    It acts as a filter on the returned set of names by mandating that each
    returned name identifies an Object that MUST be associated to the source
    Object via an Association in which the named returned Object plays the
    specified role (i.e. the name of the CIMProperty in the Association Class that
    refers to the returned Object MUST match the value of this parameter).

    @return If successful, the method returns zero or more full CIM Class paths
    or Instance paths of Objects meeting the requested criteria. Since it is
    possible for CIM Objects from different hosts or namespaces to be
    associated, each returned path is an absolute path that includes host and
    namespace information.

    If unsuccessful, one of the following status codes MUST be returned by this
    method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.

    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE;
      <LI>CIM_ERR_INVALID_PARAMETER (including missing,
      duplicate, unrecognized or otherwise incorrect parameters)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)</LI>
    </UL>
    */
    virtual Array<CIMReference> associatorNames(

        const String& nameSpace,
        const CIMReference& objectName,
        const String& assocClass = String::EMPTY,
        const String& resultClass = String::EMPTY,
        const String& role = String::EMPTY,
        const String& resultRole = String::EMPTY) = 0;

    /** The <TT>references</TT> operation enumerates the association
    objects that refer to a particular target CIM Object (Class or Instance).

    @param NameSpace The NameSpace parameter is a string that defines the target
    namespace \Ref{NAMESPACE}

    @param ObjectName The ObjectName input parameter defines the target CIM
    Object whose referring Objects are to be returned. This is either a Class
    name or Instance name (model path).

    @param ResultClass The ResultClass input parameter, if not NULL, MUST be a
    valid CIM Class name. It acts as a filter on the returned set of Objects by
    mandating that each returned Object MUST be an Instance of this Class (or
    one of its subclasses), or this Class (or one of its subclasses).

    @param role The Role input parameter, if not NULL, MUST be a valid CIMProperty
    name. It acts as a filter on the returned set of Objects by mandating that
    each returned Objects MUST refer to the target Object via a CIMProperty whose
    name matches the value of this parameter.

    @param Include Qualifiers If the IncludeQualifiers input parameter is true,
    this specifies that all Qualifiers for each Object (including Qualifiers on
    the Object and on any returned Properties) MUST be included as <QUALIFIER>
    elements in the response.  If false no <QUALIFIER> elements are present in
    each returned Object.

    @param IncludeClassOrigin If the IncludeClassOrigin input parameter is true,
    this specifies that the CLASSORIGIN attribute MUST be present on all
    appropriate elements in each returned Object. If false, no CLASSORIGIN
    attributes are present in each returned Object.

    @param PropertyList If the PropertyList input parameter is not NULL, the
    members of the array define one or more CIMProperty names.  Each returned
    Object MUST NOT include elements for any Properties missing from this list.
    Note that if LocalOnly is specified as true (or DeepInheritance is specified
    as false) this acts as an additional filter on the set of Properties
    returned (for example, if CIMProperty A is included in the PropertyList but
    LocalOnly is set to true and A is not local to a returned Instance, then it
    will not be included in that Instance). If the PropertyList input parameter
    is an empty array this signifies that no Properties are included in each
    returned Object. If the PropertyList input parameter is NULL this specifies
    that all Properties (subject to the conditions expressed by the other
    parameters) are included in each returned Object.

    If the PropertyList contains duplicate elements, the Server MUST ignore the
    duplicates but otherwise process the request normally.  If the PropertyList
    contains elements which are invalid CIMProperty names for any target Object,
    the Server MUST ignore such entries but otherwise process the request
    normally.

    Clients SHOULD NOT explicitly specify properties in the PropertyList
    parameter unless they have specified a non-NULL value for the ResultClass
    parameter.

    @return If successful, the method returns zero or more CIM Classes or
    Instances meeting the requested criteria.  Since it is possible for CIM
    Objects from different hosts or namespaces to be associated, each returned
    Object includes location information.

    If unsuccessful, one of the following status codes MUST be returned by this
    method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.

    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE
      <LI>CIM_ERR_INVALID_PARAMETER (including missing,
      	duplicate, unrecognized or otherwise incorrect parameters)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)</LI>
     </UL>
    */
     virtual Array<CIMInstance> references(
 	const String& nameSpace,
 	const CIMReference& objectName,
 	const String& resultClass = String::EMPTY,
 	const String& role = String::EMPTY,
 	Boolean includeQualifiers = false,
 	Boolean includeClassOrigin = false,
	const Array<String>& propertyList= StringArray()) = 0;
    /**
    The <TT>referenceNames</TT> operation is used to enumerate the association
    objects that refer to a particular target CIM Object (Class or Instance).

    @param NameSpace The NameSpace parameter is a string that defines the target
    namespace \Ref{NAMESPACE}

    @param The ObjectName input parameter defines the target CIM Object whose
    referring object names are to be returned. It may be either a Class name or
    an Instance name (model path).

    @param The ResultClass input parameter, if not NULL, MUST be a valid CIM
    Class name. It acts as a filter on the returned set of Object Names by
    mandating that each returned Object CIMName MUST identify an Instance of this
    Class (or one of its subclasses), or this Class (or one of its subclasses).

    @param The role input parameter, if not NULL, MUST be a valid CIMProperty name.
    It acts as a filter on the returned set of Object Names by mandating that
    each returned Object CIMName MUST identify an Object that refers to the target
    Instance via a CIMProperty whose name matches the value of this parameter.

    @return If successful, the method returns the names of zero or more full CIM
    Class paths or Instance paths of Objects meeting the requested criteria.
    Since it is possible for CIM Objects from different hosts or namespaces to
    be associated, each returned path is an absolute path that includes host and
    namespace information.

    If unsuccessful, one of the following status codes MUST be returned by this
    method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.
    <UL>
     	<LI>CIM_ERR_ACCESS_DENIED
     	<LI>CIM_ERR_NOT_SUPPORTED
     	<LI>CIM_ERR_INVALID_NAMESPACE
     	<LI>CIM_ERR_INVALID_PARAMETER (including missing, duplicate,
     	unrecognized or otherwise incorrect parameters)
     	<LI>CIM_ERR_FAILED (some other unspecified error occurred)
     </UL>
    */
    virtual Array<CIMReference> referenceNames(
    const String& nameSpace,
    const CIMReference& objectName,
    const String& resultClass = String::EMPTY,
    const String& role = String::EMPTY) = 0;
    /**
    This operation is used to retrieve a single property value from a CIM
    Instance in the target Namespace.

    @param NameSpace The NameSpace parameter is a string that defines the target
    namespace \Ref{NAMESPACE}

    @param The InstanceName input parameter specifies the name of the Instance
    (model path) from which the CIMProperty value is requested.\\Ref{INSTANCENAME}

    @param The PropertyName input parameter specifies the name of the CIMProperty
    whose value is to be returned.

    @return If successful, the return value specifies the value of the requested
    CIMProperty. If the value is NULL then no element is returned.

    If unsuccessful, one of the following status codes MUST be returned by this
    method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.
    <UL>
	<LI>CIM_ERR_ACCESS_DENIED
	<LI>CIM_ERR_INVALID_NAMESPACE
	<LI>CIM_ERR_INVALID_PARAMETER (including missing, duplicate,
	unrecognized or otherwise incorrect parameters)
	<LI>CIM_ERR_INVALID_CLASS (the CIM Class does not exist in the specified
	namespace)
	<LI>CIM_ERR_NOT_FOUND (the CIM Class does exist, but the requested CIM
	Instance does not exist in the specified namespace)
	<LI><LI>CIM_ERR_NO_SUCH_PROPERTY (the CIM Instance does exist, but the
	requested CIMProperty does not)
	<LI>CIM_ERR_FAILED (some other unspecified error occurred)
    </UL>
    */
    virtual CIMValue getProperty(
    const String& nameSpace,
    const CIMReference& instanceName,
    const String& propertyName) = 0;

    /** The <TT>setProperty</TT> operation sets a single property value in a CIM
    Instance in the target Namespace.

    @param NameSpace The NameSpace parameter is a string that defines the target
    namespace \Ref{NAMESPACE}

    @param InstanceName The InstanceName input parameter specifies the name of
    the Instance (model
    path) for which the CIMProperty value is to be updated.

    @param PropertyName The PropertyName input parameter specifies the name of
    the CIMProperty whose
    value is to be updated.

    @param Newvalue The NewValue input parameter specifies the new value for the
    CIMProperty (which
    may be NULL).

    @return If unsuccessful, one of the following status codes MUST be returned
    by this method, where the first applicable error in the list (starting with
    the first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.
    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_INVALID_NAMESPACE

      <LI>CIM_ERR_INVALID_PARAMETER (including
      missing,duplicate, unrecognized or otherwise incorrect parameters)

      <LI>CIM_ERR_INVALID_CLASS (the CIM Class does not exist in the specified
      namespace)
      <LI>CIM_ERR_NOT_FOUND (the CIM Class does exist, but the requested
      CIM Instance does not exist in the specified namespace)
      <LI>CIM_ERR_NO_SUCH_PROPERTY (the CIM Instance does exist, but the
      requested CIMProperty does not)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)</LI>
    </UL>
    */
    virtual void setProperty(
        const String& nameSpace,
		const CIMReference& instanceName,
		const String& propertyName,
		const CIMValue& newValue = CIMValue()) = 0;


    /** The <TT>getQualifier</TT> operation retrieves a single CIMQualifier
    declaration from the target Namespace.

    @param NameSpace The NameSpace parameter is a string that defines the target
    namespace \Ref{NAMESPACE}

    @param QualifierName The QualifierName input parameter identifies the
    CIMQualifier whose declaration to be retrieved.

    @return If successful, the method returns the CIMQualifier declaration for the
    named CIMQualifier.

    If unsuccessful, one of the following status codes MUST be returned by this
    method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.

    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE
      <LI>CIM_ERR_INVALID_PARAMETER (including missing,
      	duplicate, unrecognized or otherwise incorrect parameters)
      <LI>CIM_ERR_INVALID_CLASS (the CIM Class does not exist in the specified
      	namespace)
      <LI>CIM_ERR_NOT_FOUND (the CIM Class does exist, but the requested
      	CIM Instance does not exist in the specified namespace)
      <LI>CIM_ERR_NO_SUCH_PROPERTY (the CIM Instance does exist, but the
      	requested CIMProperty does not)
      <LI>CIM_ERR_TYPE_MISMATCH (the supplied value is incompatible with the
      	type of the CIMProperty)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)</LI>
    </UL>
    */
    virtual CIMQualifierDecl getQualifier(
    const String& nameSpace,
    const String& qualifierName) = 0;


    /** The <TT>setQualifier</TT> creates or update a single CIMQualifier
    declaration in the target Namespace.  If the CIMQualifier declaration already
    exists it is overwritten.

    @param NameSpace The NameSpace parameter is a string that defines the target
    namespace \Ref{NAMESPACE}

    @param CIMQualifier Declaration The QualifierDeclaration input parameter
    defines the CIMQualifier Declaration to be added to the Namespace. @return If
    successful, the CIMQualifier declaration MUST have been added to the target
    Namespace.  If a CIMQualifier declaration with the same CIMQualifier name already
    existed, then it MUST have been replaced by the new declaration.

    @return If unsuccessful, one of the following status codes MUST be returned
    by this method, where the first applicable error in the list (starting with
    the first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.

    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE
      <LI>CIM_ERR_INVALID_PARAMETER (including missing,
      	duplicate, unrecognized or otherwise incorrect parameters)
      <LI>CIM_ERR_NOT_FOUND (the requested CIMQualifier declaration did not exist)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)
      </LI>
    </UL>
    */
    virtual void setQualifier(

        const String& nameSpace,
        const CIMQualifierDecl& qualifierDecl) = 0;

    /** The <TT>deleteQualifier</TT> operation deletes a single CIMQualifier
    declaration from the target Namespace.

    @param NameSpace The NameSpace parameter is a string that defines the target
	namespace \Ref{NAMESPACE}

    @param QualifierName The QualifierName input parameter identifies the
    CIMQualifier whose declaration to be deleted. @return If successful, the
    specified CIMQualifier declaration MUST have been deleted from the Namespace.

    @return If unsuccessful, one of the following status codes MUST be returned
    by this method, where the first applicable error in the list (starting with
    the first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.

    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE
      <LI>CIM_ERR_INVALID_PARAMETER (including missing,
      duplicate, unrecognized or otherwise incorrect parameters)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)</LI>
  </UL>

    */
    virtual void deleteQualifier(

        const String& nameSpace,
        const String& qualifierName) = 0;


    /** The <TT>enumerateQualifiers</TT> operation is used to enumerate
    CIMQualifier declarations from the target Namespace.

    @param NameSpace The NameSpace parameter is a string that defines the target
	namespace \Ref{NAMESPACE}

    @return If successful, the method returns zero or more CIMQualifier
    declarations.

    If unsuccessful, one of the following status codes MUST be returned by this
    method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.

    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE
      <LI>CIM_ERR_INVALID_PARAMETER (including missing,
      	duplicate, unrecognized or otherwise incorrect parameters)
      <LI>CIM_ERR_NOT_FOUND (the requested CIMQualifier declaration did not exist)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)
      </LI>
     </UL>
    */
    virtual Array<CIMQualifierDecl> enumerateQualifiers(
		const String& nameSpace) = 0;


    /**
    TBD Documentation on the Invoke method

    */

    virtual CIMValue invokeMethod(
	const String& nameSpace,
	const CIMReference& instanceName,
	const String& methodName,
	const Array<CIMValue>& inParameters,
	Array<CIMValue>& outParameters) = 0;
};

PEGASUS_NAMESPACE_END


#endif /* Pegasus_Operations_h */
























