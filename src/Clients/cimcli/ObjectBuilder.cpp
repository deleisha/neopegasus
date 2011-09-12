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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/XmlWriter.h>

#include "CIMCLIClient.h"
#include "ObjectBuilder.h"
#include "CIMCLICommon.h"
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/ArrayIterator.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

// Define a Pegasus array implementation for the tokenItem type
#define PEGASUS_ARRAY_T tokenItem
# include <Pegasus/Common/ArrayImpl.h>
#undef PEGASUS_ARRAY_T

// Diagnostic dislays activated if verboseTest set to true below.  This is
// compile time used for developer tests only.  Tjis should be disabled
// before code is released or automated tests run.
bool verboseTest = false;
#define VCOUT if (verboseTest) cout << "LINE=" << __LINE__ << " TST "

class csvStringParse;

// Cleans an input array by removing the { } tokens that surround
// the array parameters.  Does nothing if they do not exist.  If there is
// an unmatched set, an error is generated.
// Allows the user to input array definitions surrounded by { }
// as an option.
void _cleanArray(String& x)
{
    if (x[0] == '{' && x[x.size()] == '}')
    {
        x.remove(0,1);
        x.remove(x.size(),1);
    }
    else if (x[0] == '{' || x[x.size()] == '}')
    {
        cimcliMsg::exit(CIMCLI_INPUT_ERR,
            "Unmatched { } in array $0", x);
    }
}

// FUTURE - Expand this for other date time literals suchs as TODAY
//
CIMDateTime _getDateTime(const String& str)
{
    if (String::equalNoCase(str,"NOW"))
    {
        return CIMDateTime::getCurrentDateTime();
    }
    return CIMDateTime(str);
}

Boolean _StringToBoolean(const String& x)
{
    if (String::equalNoCase(x,"true"))
    {
        return true;
    }
    else if (String::equalNoCase(x,"false"))
    {
        return false;
    }

    // NOTE: please code with both the error code and the MessageLoaderParms
    // on the same line.  Allows us to grep for messages that have been
    // internationalized easily.  Thanks. KS
    cimcliMsg::exit(CIMCLI_INPUT_ERR, MessageLoaderParms(
        "Clients.cimcli.CIMCLIClient.BOOLEAN_VALUE_ERR",
        "Invalid keyword for boolean value =  $0. "
        "Must be \"true\" or \"false\" upper or lower case",x));
    return false;
}

Uint32 _includesType(const String& name)
{
    // FUTURE  KS add code. For now this is null because we do not
    // support the data type option on input.
    return 0;
}

/******************************************************************************
**
**   Implementation of tokenItem
**
******************************************************************************/
// return String for the tokenType. This is a diagnostic tool only.
String tokenTypeToString(TokenType tokenType)
{
    String rtn;
    static const char * tokenTypeString[] =
    {
        "UNKNOWN","ILLEGAL", "VALUE", "NO_VALUE", "EXCLAM", "NAME_ONLY",
             "EMBED", "END_EMBED", "EMBED_NEXT_ARRAY_ITEM"
    };
    static const Uint32 _NUM_TYPES =
        sizeof(tokenTypeString) / sizeof(tokenTypeString[0]);

    assert((Uint32)tokenType <= _NUM_TYPES);
    return tokenTypeString[tokenType];
}

tokenItem::tokenItem (const String& inputParam, String& name, String& value,
    TokenType& type)
    :
    tokenInput(inputParam),tokenValue(value),
    tokenType(type)
{
    if (name.size() != 0)
    {
        tokenName = CIMName(name);
    }
}

void tokenItem::print()
{
    cout << toString() << endl;
}

// Convert info in single token to string for display. Diagnostic
// tool
String tokenItem::toString()
{
    char tmp[22];
    Uint32 cvtSize;
    String x = "TOKEN: Name=";
    x.append(tokenName.getString());
    x.append(", value=");
    x.append(tokenValue);
    x.append(", type=");
    x.append(tokenTypeToString(tokenType));
    x.append(", input=");
    x.append(tokenInput);
    x.append(" Instance count=");
    x.append(Uint32ToString(tmp, _instances.size(), cvtSize));
    for (Uint32 i = 0 ; i < _instances.size(); i++)
    {
        x.append(Uint32ToString(tmp, i, cvtSize));
        x.append("\n    Instance # ");
        x.append(Uint32ToString(tmp, i, cvtSize));
        x.append(_instances[i].isUninitialized()? " EMPTY" : " EXISTS");
        if (!_instances[i].isUninitialized())
        {
            XmlWriter::printInstanceElement(_instances[i], cout);
        }
    }
    x.append(" \n}\n");
    return x;
}

/*****************************************************************************
**
**  local support functions for Object Builder
**
******************************************************************************/

/**
   Parse the input string to separate out the  name and value
   components. The name component must be a CIM_Name and contain
   only alphanumerics and _ characters. The value is optional.
   The possible separators are today "=", "{" and "!". and no
   value component is allowed after the ! separator Includes the
   possibility of Embedded Instances where the  property name is
   terminated by { and the following pairs are 1. classname,
   propertyname/value pairs *
   @param input String containing the input parameter
   @param name String name parsed from input parameter
   @param value String representing value component parsed from
                parameter

   @return TokenType Token type that resulted from this parse
 */
tokenItem parseInputParam(const String& input)
{
    TokenType tokenType = UNKNOWN;
    String value;
    String name;
    Uint32 pos;

    // Scan the input string for a terminator character.
    Uint32 index = 0;
    for ( ; index < input.size() ; index++)
    {
        switch(input[index])
        {
            case '=':
                if (index < input.size())
                {
                    if (input[index+1] == '{')
                    {
                        tokenType = EMBED;
                        index++;
                    }
                    else
                        tokenType = VALUE;
                }
                else
                    tokenType = VALUE;
                break;

            case '!':
                tokenType = EXCLAM;
                break;

            case '{':
                tokenType = EMBED;
                break;

            case '}':
                if (input.subString(index,3) ==  "}.{")
                {
                    tokenType = EMBED_NEXT_ARRAY_ITEM;
                    index +=2;
                }
                else
                {
                    tokenType = END_EMBED;
                }
                break;

            default:
                name.append(input[index]);
        }  // end case statement

        // break the loop if separator found
        if (tokenType != UNKNOWN)
        {
            break;
        }
    }

    // separator must be found
    if (tokenType == UNKNOWN)
    {
        cimcliMsg::exit(CIMCLI_INPUT_ERR,
            "No name/value separator in $0."
            "Parameters must have separator such as "
            "= { ={ } !", input);
    }

    // There must be a name entity except for END_EMBED
    else if (name.size() == 0 && tokenType != END_EMBED
             && tokenType != EMBED_NEXT_ARRAY_ITEM)
    {

        cimcliMsg::exit(CIMCLI_INPUT_ERR,
            "No name on input parameter $0 ",input);
    }

    // get the value component from the remaining characters of input
    else
    {
        value = input.subString(index + 1, PEG_NOT_FOUND);
        if (value.size() == 0)
        {
            if (tokenType == VALUE)
            {
                tokenType = NO_VALUE;
            }
        }
    }

    // disgnostic output when verbose set.
    VCOUT << "Input Parse of " << input
          << " into tokenType=" << tokenTypeToString(tokenType)
          << " name=" << name
          << " value=" << value << endl;

    tokenItem ti(input, name, value, tokenType);
    return ti;
}

/******************************************************************************
   Parser for comma-separated-strings (csv). This parser takes into
   account quoted strings the " character and returns everything
   within a quoted block in the string in one batch.  It also
   considers the backslash "\" escape character to escape single
   double quotes.
   Example:
     csvStringParse x (inputstring, ",");
     while (x.more())
        rtnString = x.next();
******************************************************************************/
class csvStringParse
{
public:
    /* Define a string to parse for comma separated values and the
       separation character
    */
    csvStringParse(const String& csvString, const int separator)
    {
        _inputString = csvString;
        _separator = separator;
        _idx = 0;
        _end = csvString.size();
    }

    /* determine if there is more to parse
       @return true if there is more to parse
    */
    Boolean more()
    {
        return (_idx < _end)? true : false;
    }

    /* get next string from input. Note that this will continue to
       return empty strings if you parse past the point where more()
       returns false.
       @return String
    */
    String next()
    {
        String rtnValue;
        parsestate state = NOTINQUOTE;

        while ((_idx <= _end) && (_inputString[_idx]))
        {
            char idxchar = _inputString[_idx];
            switch (state)
            {
                case NOTINQUOTE:
                    switch (idxchar)
                    {
                        case '\\':
                            state = INSQUOTE;
                            break;

                        case '"':
                            state = INDQUOTE;
                            break;

                        default:
                            if (idxchar == _separator)
                            {
                                _idx++;
                                return rtnValue;
                            }
                            else
                            {
                                rtnValue.append(idxchar);
                            }
                            break;
                    }
                    break;

                // add next character and set NOTINQUOTE State
                case INSQUOTE:
                    rtnValue.append(idxchar);
                    state = NOTINQUOTE;
                    break;

                // append all but quote character
                case INDQUOTE:
                    switch (idxchar)
                    {
                        case '"':
                            state = NOTINQUOTE;
                            break;
                        default:
                            rtnValue.append(idxchar);
                            break;
                    }
            }
            _idx++;
        }   // end while

        return rtnValue;
    }

private:
    enum parsestate {INDQUOTE, INSQUOTE, NOTINQUOTE};
    Uint32 _idx;
    int _separator;
    Uint32 _end;
    String _inputString;
};


/* Convert a single string provided as input into a new CIM variable
   and place it in a CIMValue.  This function does not process
   Embedded instances/objects since they are not represented as a single
   string.
   @param str char* representing string to be parsed.
   @param type CIMType expected.
   @return CIMValue with the new value. If the parse fails, this function
   terminates with an exit CIMCLI_INPUT_ERR.
*/
static CIMValue _stringToScalarValue(const char* str, CIMType type)
{
    switch (type)
    {
        case CIMTYPE_BOOLEAN:
            return CIMValue(_StringToBoolean(str));

        case CIMTYPE_UINT8:
            return CIMValue(Uint8(strToUint(str, type)));

        case CIMTYPE_SINT8:
            return CIMValue(Sint8(strToSint(str, type)));

        case CIMTYPE_UINT16:
            return CIMValue(Uint16(strToUint(str, type)));

        case CIMTYPE_SINT16:
            return CIMValue(Sint16(strToSint(str, type)));

        case CIMTYPE_UINT32:
            return CIMValue(Uint32(strToUint(str, type)));

        case CIMTYPE_SINT32:
            return CIMValue(Sint32(strToSint(str, type)));

        case CIMTYPE_UINT64:
            return CIMValue(Uint64(strToUint(str, type)));

        case CIMTYPE_SINT64:
            return CIMValue(Sint64(strToSint(str, type)));

        case CIMTYPE_REAL32:
            return CIMValue(Real32(strToReal(str, type)));

        case CIMTYPE_REAL64:
            return CIMValue(Real64(strToReal(str, type)));

        case CIMTYPE_STRING:
            return CIMValue(String(str));

        case CIMTYPE_DATETIME:
            return CIMValue(_getDateTime(str));

        case CIMTYPE_REFERENCE:
            return CIMValue(CIMObjectPath(str));

        default:
            cimcliMsg::exit(CIMCLI_INTERNAL_ERR,
                "Data type $0 build from string not allowed",
                cimTypeToString(type));
    }
    return CIMValue();
}

/*
    Parse out comma-separated values from input string and build the
    CIMValue array representing the input for array type input entities.
    @param str const char * containing the comma-separated values
    to be parsed.  All value elements in the CSV string must be
    parsable to the same type.  This function does not handle the
    CIMInstance or CIMObject types since they are not a single string on
    input.

    @param v CIMValue into which the resulting values are appended to
    any existing values. This allows building array CIMValues from repeated
    input of single property definitions rather than having to input
    the entire array at one time.
    @return Returns complete CIMValue.
*/
Boolean _stringToArrayValue(
    const char * str,
    CIMValue& val)
{
    CIMType type = val.getType();
    Uint32 arrayDimension = val.getArraySize();
    String parseStr(str);

    csvStringParse strl(parseStr, ',');

    switch (type)
    {
        case CIMTYPE_BOOLEAN:
        {
            Array<Boolean> a;
            val.get(a);
            while(strl.more())
            {
                 a.append(_StringToBoolean(strl.next()));
            }
            val.set(a);
            break;
        }

        case CIMTYPE_UINT8:
        {
            Array<Uint8> a;
            val.get(a);
            while(strl.more())
            {
                a.append(strToUint(strl.next().getCString(), type));
            }
            val.set(a);
            break;
        }

        case CIMTYPE_SINT8:
        {
            Array<Sint8> a;
            val.get(a);
            while(strl.more())
            {
                a.append(strToSint(strl.next().getCString(), type));
            }
            val.set(a);
            break;
        }

        case CIMTYPE_UINT16:
        {
            Array<Uint16> a;
            val.get(a);
            while(strl.more())
            {
                a.append(strToUint(strl.next().getCString(), type));
            }
            val.set(a);
            break;
        }

        case CIMTYPE_SINT16:
        {
            Array<Sint16> a;
            val.get(a);
            while(strl.more())
            {
                a.append(strToSint(strl.next().getCString(), type));
            }
            val.set(a);
            break;
        }

        case CIMTYPE_UINT32:
        {
            Array<Uint32> a;
            val.get(a);
            while(strl.more())
            {
                a.append(strToUint(strl.next().getCString(), type));
            }
            val.set(a);
            break;
        }

        case CIMTYPE_SINT32:
        {
            Array<Sint32> a;
            val.get(a);
            while(strl.more())
            {
                a.append(strToSint(strl.next().getCString(), type));
            }
            val.set(a);
            break;
        }

        case CIMTYPE_UINT64:
        {
            Array<Uint64> a;
            val.get(a);
            while(strl.more())
            {
                a.append(strToUint(strl.next().getCString(), type));
            }
            val.set(a);
            break;
        }

        case CIMTYPE_SINT64:
        {
            Array<Sint64> a;
            val.get(a);
            while(strl.more())
            {
                a.append(strToSint(strl.next().getCString(), type));
            }
            val.set(a);
            break;
        }

        case CIMTYPE_REAL32:
        {
            Array<Real32> a;
            val.get(a);
            while(strl.more())
            {
                a.append(strToSint(strl.next().getCString(), type));
            }
            val.set(a);
            break;
        }

        case CIMTYPE_REAL64:
        {
            Array<Real64> a;
            val.get(a);
            while(strl.more())
            {
                a.append(strToSint(strl.next().getCString(), type));
            }
            val.set(a);
            break;
        }

        case CIMTYPE_STRING:
        {
            Array<String> a;
            val.get(a);
            while(strl.more())
            {
                a.append(strl.next());
            }
            val.set(a);
            break;
        }

        case CIMTYPE_DATETIME:
        {
            Array<CIMDateTime> a;
            val.get(a);
            while(strl.more())
            {
                a.append(_getDateTime(strl.next()));
            }
            val.set(a);
            break;
        }

        case CIMTYPE_REFERENCE:
        {
            Array<CIMObjectPath> a;
            val.get(a);
            while(strl.more())
            {
                 a.append(CIMObjectPath(strl.next()));
            }
            val.set(a);
            break;
        }

        default:
            cimcliMsg::exit(CIMCLI_INTERNAL_ERR,
                "Data type $0 build from string not allowed",
                cimTypeToString(type));
    }
    return true;
}

// Complete the defined value item (iv) by modifying or setting the value.
// The input is expected to have the correct CIMType and array information
// Common builder for parameters and properties. This handles all
// CIM data types but uses the _stringTo.. functions for all but the
// CIMInstance and CIMObject types

void _buildValueFromToken(tokenItem& token, CIMValue& iv, CIMType cimType)
{
    VCOUT <<"_buildValueFromToken cimType=" << cimTypeToString(cimType)
          << " token=" << token.toString() << endl;

    switch (token.tokenType)
    {
        case EXCLAM:
        {
            if (cimType == CIMTYPE_STRING)
            {
                // Set property to empty string
                iv.set(String());
            }
            else
            {
                cimcliMsg::exit(CIMCLI_INPUT_ERR,
                    "\"!\" terminator allowed only on String type. "
                     " Parameter= $0",token.tokenInput);
            }
            break;
        }

        case VALUE:
        {
            if (iv.isArray())
            {
                _cleanArray(token.tokenValue);
                // Append array to existing value
                if (!_stringToArrayValue(token.tokenValue.getCString(), iv))
                {
                    cimcliMsg::exit(CIMCLI_INPUT_ERR,
                        "Error parsing Array parameter. $0",
                           token.tokenInput);
                }
            }
            else  // scalar
            {
                // Replace property value in property.
                iv = _stringToScalarValue(token.tokenValue.getCString(),
                          cimType);
            }
            break;
        }

        // Embedded token. should contain embedded instance built earlier
        case EMBED:
        {
            // Array of Instances should be in place here to put into property.
            if (token._instances.size() != 0)
            {
                // If array type, append to current property value
                if (iv.isArray())
                {
                    if (iv.getType() == CIMTYPE_INSTANCE)
                    {
                        Array<CIMInstance> insts;
                        iv.get(insts);
                        insts.append(token._instances[0]);
                        iv.set(insts);
                    }
                    else if (iv.getType() == CIMTYPE_OBJECT)
                    {
                        Array<CIMObject> objects;
                        iv.get(objects);
                        objects.append((CIMObject)token._instances[0]);
                        iv.set(objects);
                    }
                    else
                    {
                        cimcliMsg::exit(CIMCLI_INTERNAL_ERR,
                            "Invalid CIM datatype on embedded object. Token=$0",
                            token.toString());
                    }
                }

                else // Scalar. set token to property value
                {
                    CIMInstance inst = token._instances[0];

                    VCOUT << "buildInstance EMBED Instance found"
                         << " classname=" << inst.getClassName().getString()
                         <<  " iv type=" << cimTypeToString(cimType)
                         << endl;

                    // set the new instance into the value
                    if (cimType == CIMTYPE_INSTANCE)
                    {
                        iv.set(inst);
                    }
                    else if (cimType == CIMTYPE_OBJECT)
                    {
                        iv.set((CIMObject)inst);
                    }
                    else
                    {
                        cimcliMsg::exit(CIMCLI_INTERNAL_ERR,
                            "Invalid CIM datatype on embedded object. Token=$0",
                            token.toString());
                    }
                }
            }

            else   // Must be instances available if parse worked
            {
                cimcliMsg::exit(CIMCLI_INTERNAL_ERR,
                    "Embedded Instance(s) not built. Token=$0",
                    token.toString());
            }
            break;
        }

        // In case of no value, just pass value existing in input CIMValue
        case NO_VALUE:
        {
            break;
        }

        default:
        {
            cimcliMsg::exit(CIMCLI_INTERNAL_ERR,
                "Invalid token on building CIMValue. Token=$0",
                token.toString());
        }
    }
}

/*
    Build the CIMValue for a single property from the input
    itemToken and set it into the input property.  Errors
    generate exit.

    @param token itemToken defines value, type, etc. for this
            property
    @param ip CIMProperty in instance being built. New value from
            token replaces existing value in property.
 */
void _buildPropertyValue(tokenItem& token, CIMProperty& ip)
{
    CIMType cimType = ip.getType();
    CIMValue iv = ip.getValue();

    _buildValueFromToken(token, iv, cimType);

    // Set new value back into property.
    ip.setValue(iv);
}

/*
     Create a method parameter from the tokenized information for the
     method name, value, type, etc.
*/
CIMParamValue _buildMethodParamValue(
    tokenItem ti,
    const CIMParameter& thisParameter,
    CIMValue& curValue,
    Boolean verbose)
{
    VCOUT << "_createMethodParamValue "
         << " tokenItem= " << ti.toString()
         << " Param type=" <<  cimTypeToString(thisParameter.getType())
         <<  " Param isArray=" << (thisParameter.isArray()? "true" : "false")
         << " curValue type= " << cimTypeToString(curValue.getType())
         << " paramName="  << thisParameter.getName().getString()
         << endl;

    CIMType cimType = curValue.getType();
    String paramName = thisParameter.getName().getString();

    CIMValue vp = curValue;

    _buildValueFromToken(ti, vp, cimType);

    // build the Parameter Value with isTyped == true
    return CIMParamValue(paramName,vp,true);
}

/****************************************************************************

  Defines an iterator class for String arrays. This class is used to
  make the input parameter list parsing more understandable than a
  for loop.

****************************************************************************/
class iterateArray
{
public:
    // construct an iterator for an array
    iterateArray(Array<String> input)
    : _array(input), _pos(0), _prev(String()), _start(0)
    {
    }

    // Create new iterator starting from nonzero position
    // in input array
    iterateArray(Array<String> input, Uint32 pos)
    : _array(input), _pos(pos), _prev(String()), _start(pos)
    {
    }

    // return true if more available
    Boolean more()
    {
        return (_pos < _array.size()? true : false);
    }

    // mget next item.  This is item currently pointed
    // to. And advance pointer to next item. next MUST
    // BE proceeded by more() since there is no check here
    String next()
    {
        if (!more())
        {
            cerr << "Internal Error in iterateArray" << endl;
            cimcliExit(CIMCLI_INTERNAL_ERR);
        }
        _prev = _array[_pos];

        VCOUT << "next returns " << _prev << " at pos " << _pos << endl;
        return _array[_pos++];
    }

    // get previous entry
    String prev()
    {
        if (_pos == 0)
        {
            cerr << "Internal Error in iterateArray" << endl;
            cimcliExit(CIMCLI_INTERNAL_ERR);
        }
        return _prev;
    }

    // get position
    Uint32 pos()
    {
        return _pos;
    }

    // insert  x at current position in the array
    void insert(String x)
    {
        _array.insert(_pos, x);
    }

    void printList()
    {
        VCOUT << "Input is " << _array.size() << " value parameters" << endl;
        for (Uint32 i = 0 ; i < _array.size() ; i++)
        {
            VCOUT << i << " value " << _array[i] << endl;
        }
    }

private:
    Array<String> _array;
    Uint32 _pos;
    String _prev;
    Uint32 _start;
};

/******************************************************************
*
*   ObjectBuilder Class. Creates instances and parameters from
*     name/value pairs received and the corresponding class
*     definitions.
*
*******************************************************************/

// Common Scan loop to Scan input Value Parameter list.  This loop is used
// both by the outer constructor (recurselevel == 0) to control scan of
// all value parameters and by the embedded Object constructor which
// scans a part of the value parameter list for a single embedded object

void ObjectBuilder::scanInputList(CIMClient& client,
    const CIMNamespaceName& nameSpace,
    const CIMPropertyList& cimPropertyList,
    iterateArray& ia,
    Uint32& recurseLevel,
    const CIMName& featureName)
{
    // Set true when end of embedded object "}" or "}.{" found in
    // the scan for a single embedded object.
    Boolean embEndTokenFound = false;

    while (ia.more())
    {
        // parse each input parameter and eliminate any illegal returns
        // Each successful creates new tokenItem
        tokenItem ti = parseInputParam(ia.next());

        // process based on the returned tokenItem type.
        switch(ti.tokenType)
        {
            case EMBED:
            {
                // If embedded indicator, get next token as embedded
                // classname unless value returned
                if (ti.tokenValue.size() == 0)
                {
                    // get class name. If no inputParamList, error
                    if (!ia.more())
                    {
                        cimcliMsg::exit(CIMCLI_INPUT_ERR,
                            "No embedded instance classname at ",
                            ti.tokenInput);
                    }
                    else
                    {
                        // Set next input param to Class to use
                        ti.tokenValue = ia.next();
                    }
                }

                // Parse and buildInstance for the embedded object.
                // Returns built instance and inputParamList pos at next
                // input parameter after end marker of this embedded
                // instance

                ObjectBuilder obj(ia, client, nameSpace,
                    ti,
                    cimPropertyList,
                    _verbose,
                    _instance,
                    ++recurseLevel);

                // Review this for alternate place to do the
                // append. Since ti is part of the recursive call, why not
                // do the insert there instead of returning the objects
                // to this level.
                // Append new instance into any existing array of instances
                // for this token and append the token to the token list
                ti._instances.append(_instance);
                _tokens.append(ti);
                break;
            }

            case EMBED_NEXT_ARRAY_ITEM:
            case END_EMBED:
            {
                // Should not find end markers at the zero recurse level
                if (recurseLevel == 0)
                {
                    cimcliMsg::exit(CIMCLI_INPUT_ERR,
                        "Unmatched Embedded instance"
                        " end marker(s) at parameter=\"$0\"",
                        ti.tokenInput);
                }

                else
                {
                // End when the END_EMBED found and build instance
                // with current tokens to be returned. Return it in
                // CIMInstance parameter. includeQualifiers and
                // includeClassOrigin are false.
                // buildInstance(...) insures that an instance is built
                _instance = buildInstance(false, false,
                    CIMPropertyList());

                // If this is the NEXT_ARRAY item continue scan
                // by reinserting <featureName>{<classname> into the
                // input parameter list which fully defines the next item
                // as an array item of the same featureName.
                if (ti.tokenType == EMBED_NEXT_ARRAY_ITEM)
                {
                    String x = featureName.getString();
                    x.append("{");
                    x.append(_className.getString());
                    ia.insert(x);

                    // Diagnostic
                    ia.printList();
                }
                // set flag to stop since end embed tag found
                embEndTokenFound = true;                }
                break;
            }

            // This may be completely redundant and maybe we
            // should solve in the param parser
            case ILLEGAL:
            {
                cimcliMsg::exit(CIMCLI_INPUT_ERR,
                    "Parse Failed. Input Parameter=\"$0\" $1",
                    ti.tokenInput,
                    ((ti.tokenName.getString().size() == 0)?
                          "No Name Component" : "") );
            }

            default:
            {
                _tokens.append(ti);
            }
        }  // end case

        // exit the input processing loop early if flag set.
        if (embEndTokenFound)
        {
            break;
        }
    }    // end while(ia.more())

    // If we do not find an end_embed marker ( "}" ) report error and in
    // inner constructor, error
    if (!embEndTokenFound && (recurseLevel != 0))
    {
        cimcliMsg::exit(CIMCLI_INPUT_ERR,
            "Missing Embedded object Terminator i.e. \"}\"");
    }

    // If verbose flag set, display all tokens created in this object
    // builder object.
    if (_verbose)
    {
        // Display all the names and
        // property values of the instance with their token type
        // FUTURE Clean up this display to be more readable since in
        // involves an internal structure that we display.
        cout << "Display all tokens created for input parameter analysis"
                << endl;
        printTokens();
    }
}

/*
    Constructor, sets up a single object with the input parameter list
    and the definition to get the class required for the
    parsing and entity building.
    A parameter on this list is the definition of one input parameter
    representing the definition of a property or parameter.  It is in the form
    of a name/value pair with a token separating the name and value that
    definesthe action to be taken with this pair.  The possible actions are
    defined by the TokenType enumerate including:
        - Use value string as input to define the value of a cim data type
        - Treat this as an empty string
        - no value - create a null value
        - Treat this as the start of an embedded object definition
        - Treat as end of an embedded object definition
*/
ObjectBuilder::ObjectBuilder(
    Array<String>& inputParamList,
    CIMClient& client,
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    const CIMPropertyList& cimPropertyList,
    Boolean verbose) :
        _className(className), _verbose(verbose)
{
    // Get the class. Exceptions including class_not_found are automatic
    // not localOnly (Need all properties), get qualifiers and classOrigin
    // since we can filter them out later

    _thisClass = client.getClass(nameSpace, className,
                        false,true,true,cimPropertyList);

    // Loop starts from 1, since the Class Name is first parameter
    //   and we want only the property name and value here
    //   Parse all of the name/value pairs from the input.
    iterateArray ia(inputParamList, 1);

    // Diagnostic to print input param list when verbose set.
    ia.printList();
    Uint32 recurseLevel = 0;

    scanInputList(client, nameSpace, cimPropertyList, ia,
        recurseLevel, CIMName());
}

ObjectBuilder::ObjectBuilder(iterateArray& ia,
    CIMClient& client,
    const CIMNamespaceName& nameSpace,
    const tokenItem& tiInput,
    const CIMPropertyList& cimPropertyList,         // used to getClass
    Boolean verbose,
    CIMInstance& rtnInstance,
    Uint32& recurseLevel) :
        _className(tiInput.tokenValue), _verbose(verbose)
{
    VCOUT << "ObjectBuider Embedded Constructor level=" << recurseLevel
        << " class=" <<_className.getString()
        << " FeatureName=" << tiInput.tokenName.getString()
          << " pos=" << ia.pos() << endl;

    // Get the feature name from the token (features are properties or
    // methods)
    CIMName featureName = tiInput.tokenName;

    // get the class. Exceptions including class_not_found are automatic
    // not localOnly (Need all properties), get qualifiers and classOrigin
    // since we can filter them out later

    _thisClass = client.getClass(nameSpace, _className,
                        false,true,true,cimPropertyList);

    // Scan the input parameters to build an embedded object.
    scanInputList(client, nameSpace, cimPropertyList, ia,
        recurseLevel, featureName);

    rtnInstance = _instance;

    recurseLevel--;
}


ObjectBuilder::~ObjectBuilder()
{
}

void ObjectBuilder::printTokens(String message)
{
    if (message.size() != 0)
    {
        VCOUT << message << endl;
    }
    VCOUT << _tokens.size() << " Tokens exist for class="
          << _className.getString() << endl;
    if (verboseTest)
    {
        for (Uint32 i = 0; i < _tokens.size(); i++)
        {
                _tokens[i].print();
        }
    }
}

Array<CIMName> ObjectBuilder::getPropertyList()
{
    Array<CIMName> n;
    for (Uint32 i = 0; i < _tokens.size(); i++)
    {
        n.append(_tokens[i].tokenName);
    }
    return  n;
}

const CIMClass ObjectBuilder::getTargetClass()
{
    return _thisClass;
}

CIMObjectPath ObjectBuilder::buildCIMObjectPath()
{
    CIMInstance testInstance = buildInstance(
        true, true,
        CIMPropertyList());

    CIMObjectPath thisPath = testInstance.buildPath(_thisClass);

    return thisPath;
}

/*
    Build an instance from the information in the ObjectBuilder
    object.
*/
CIMInstance ObjectBuilder::buildInstance(
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    // Create the instance skeleton from class downloaded in ObjectBuilder
    // constructor with all properties
    CIMInstance newInstance = _thisClass.buildInstance(
        includeQualifiers,
        includeClassOrigin,
        CIMPropertyList());

    if (newInstance.isUninitialized())
    {
        cimcliMsg::exit(CIMCLI_INPUT_ERR,
            "Failed to build Instance for class $0",
            _thisClass.getClassName().getString());
    }
    Array<CIMName> myPropertyList;
    Uint32 propertyPos;

    // Set all the input property values that have tokenItem definitions
    // into an instance of the defined class
    for (Uint32 index = 0; index < _tokens.size(); index++)
    {
        // test if property in class.  Note that this is not a fatal error
        // today but just outputs a message. We just ignore that property
        // input value and continue to the next property.
        if ((propertyPos = _thisClass.findProperty(
            _tokens[index].tokenName)) == PEG_NOT_FOUND)
        {
            cerr << "Warning: property \""
                 << _tokens[index].tokenName.getString() << "\" not in class: "
                 << _thisClass.getClassName().getString() << ". Skipping."
                 << endl;
            continue;
        }

        // get value for this property from built instance

        CIMProperty ip = newInstance.getProperty(propertyPos);

        // If input not NULL type (ends with =) set the value component
        // into property.  NULL input sets whatever is in the class into
        // the instance (done by buildInstance)

        _buildPropertyValue(_tokens[index], ip);

        // Append this property name to list of properties processed
        myPropertyList.append(CIMName(_tokens[index].tokenName));
    }

    // Delete any properties not on the list of properties processed.
    newInstance.instanceFilter(includeQualifiers,
        includeClassOrigin,
        myPropertyList);

    // Display the Instance if verbose
    if (_verbose)
    {
        cout << "Instance Built" << endl;
        XmlWriter::printInstanceElement(newInstance, cout);
    }
    return newInstance;
}

// test if an array of CIMParamValue contains a parameter with the
// defined name.
Uint32 _find(const Array<CIMParamValue>& params, const CIMName& name)
{
    for(Uint32 i = 0 ; i < params.size(); i++)
    {
        if (params[i].getParameterName() == name)
        {
            return i;
        }
    }
    return PEG_NOT_FOUND;
}

Array<CIMParamValue> ObjectBuilder::buildMethodParameters(CIMName& name)
{
    Array<CIMParamValue> params;

    // Find Method
    Uint32 methodPos;
    CIMMethod thisClassMethod;

    // test for method in class
    if ((methodPos = _thisClass.findMethod(name)) == PEG_NOT_FOUND)
    {
        // Allow execution even if method name not found but bypass
        // parameter analysis.
        cerr << "Warning: method " << name.getString()
            << " Not method in the class " << _className.getString()
            <<  endl;
        return params;
    }
    else
    {
        thisClassMethod = _thisClass.getMethod(methodPos);
    }

    // Build parameter for each token defined
    for (Uint32 index = 0; index < _tokens.size(); index++)
    {
        Uint32 parameterPos;

        // Warning if parameter not defined in class.  The problem is that we
        // do not know what typing to apply so define as string with
        // any value supplied.
        if ((parameterPos = thisClassMethod.findParameter(
            CIMName(_tokens[index].tokenName))) == PEG_NOT_FOUND)
        {
            cimcliMsg::msg("Warning: "
                "Parameter $0 not valid method parameter in class $1",
                _tokens[index].tokenName.getString(),
                _className.getString());

            CIMValue v(_tokens[index].tokenValue);
            CIMParamValue rtnParamValue(
                _tokens[index].tokenName.getString(), v);
            params.append(rtnParamValue);
            continue;
        }

        CIMParameter thisParameter =
            thisClassMethod.getParameter(parameterPos);

        // Initialize CIMValue for this parameter
        // Determine if this is embedded type from class qualifiers.
        // If so, modify type to the correct embedded type.
        CIMType cimType = thisParameter.getType();

        for (Uint32 i = 0; i < thisParameter.getQualifierCount() ; i++)
        {
            CIMConstQualifier q = thisParameter.getQualifier(i);
            if (q.getName() == "EmbeddedInstance")
            {
                cimType = CIMTYPE_INSTANCE;
            }
            else if(q.getName() == "EmbeddedObject")
            {
                cimType = CIMTYPE_OBJECT;
            }
        }
        CIMValue curValue(cimType, thisParameter.isArray());

        // Already processed this parameter name? Allowed if this
        // is an array since we may build arrays of values by repeating the
        // parameter input with (ex. to build arrays of instances).
        Uint32 pos;
        if((pos = _find(params, thisParameter.getName())) != PEG_NOT_FOUND)
        {
            // if Array type, we will want to append to value. Get the
            // current value from the /params array if it already exists.
            // remove existing parameter and replace it.
            if (thisParameter.isArray())
            {
                CIMValue v = params[pos].getValue();
                curValue = v;
                params.remove(pos);
            }
            else    // If scalar, multiples are illegal.
            {
                cimcliMsg::exit(CIMCLI_INPUT_ERR,
                    "Multiple definition of scalar parameter name=$0",
                    thisParameter.getName().getString());
            }
        }

        // Build a ParamValue from the token, Class Parameter and
        // current CIMValue elements
        if (verboseTest) {_tokens[index].print();}

        CIMParamValue rtnParamValue = _buildMethodParamValue(
            _tokens[index],
            thisParameter,
            curValue,
            _verbose);

        // failed to build the parameter.  Fatal error
        if (rtnParamValue.isUninitialized())
        {
            cimcliMsg::exit(CIMCLI_INTERNAL_ERR,
            "Parse Error adding parameter $0",
                _tokens[index].tokenInput);
        }

        // Append parameter to return list.
        params.append(rtnParamValue);
    }

    return params;
}

// NOTE: Used by setProperty only. Need to modify whole setProperty logic
// to scan input list, etc. before we modify this.
// Current limitation is that setProperty does NOT handle embedded instances
// or embedded Objects.
CIMValue ObjectBuilder::buildPropertyValue(
    const CIMName& name,
    const String& value)
{
    // Find Property
    Uint32 propertyPos;
    CIMProperty thisClassProperty;

    if ((propertyPos = _thisClass.findProperty(name)) == PEG_NOT_FOUND)
    {
        cimcliMsg::exit(CIMCLI_INPUT_ERR,
            "Property $0 not property in class $1",
                name.getString(), _className.getString());
    }
    else
    {
        thisClassProperty = _thisClass.getProperty(propertyPos);
    }

    CIMType type = thisClassProperty.getType();
    CIMValue vp(type,thisClassProperty.isArray());

    if ((type == CIMTYPE_INSTANCE) || (type == CIMTYPE_OBJECT))
    {
        cimcliMsg::exit(CIMCLI_INPUT_ERR,
            "Input of Instance or object types not allowed");
    }

    if (vp.isArray())
    {
        _stringToArrayValue(value.getCString(), vp );
        return vp;
    }
    else
    {   // scalar
        return _stringToScalarValue(value.getCString(), vp.getType());
    }
}

void  ObjectBuilder::print()
{
    cout << "ObjectBuilder instance\n"
         << "ClassName=" << _className.getString();
    if (!_instance.isUninitialized())
    {
        cout << "Instance exists" << endl;
    }
    else
    {
        cout << "No instance" << endl;
    }
    printTokens();
}

PEGASUS_NAMESPACE_END

// END_OF_FILE
