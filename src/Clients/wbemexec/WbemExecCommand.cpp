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
// Author: Carol Ann Krug Graves, Hewlett-Packard Company 
//         (carolann_graves@hp.com)
//
// Modified By:
//         Warren Otsuka (warren_otsuka@hp.com)
//
//%/////////////////////////////////////////////////////////////////////////////

// define asprintf used to implement ultostr on Linux
#if defined(PEGASUS_PLATFORM_LINUX_IX86_GNU)
#define _GNU_SOURCE
#include <features.h>
#include <stdio.h>
#endif

#ifdef PEGASUS_PLATFORM_WIN32_IX86_MSVC
# include <winsock.h>
#else
# include <cctype>
# include <unistd.h>
# include <cstdlib>
# include <errno.h>
# include <fcntl.h>
# include <netdb.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/socket.h>
#endif

#include <iostream>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/getoopt/getoopt.h>
#include <Clients/cliutils/CommandException.h>
#include "HttpConstants.h"
#include "TCPChannel.h"
#include "XMLProcess.h"
#include "WbemExecClientHandler.h"
#include "WbemExecClientHandlerFactory.h"
#include "WbemExecCommand.h"

PEGASUS_NAMESPACE_BEGIN

/**
    The command name.
 */
const char   WbemExecCommand::COMMAND_NAME []      = "wbemexec";

/**
    Label for the usage string for this command.
 */
const char   WbemExecCommand::_USAGE []            = "usage: ";

/**
    The option character used to specify the hostname.
 */
const char   WbemExecCommand::_OPTION_HOSTNAME     = 'h';

/**
    The option character used to specify the port number.
 */
const char   WbemExecCommand::_OPTION_PORTNUMBER   = 'p';

/**
    The option character used to specify the HTTP version for the request.
 */
const char   WbemExecCommand::_OPTION_HTTPVERSION  = 'v';

/**
    The option character used to specify the HTTP method for the request.
 */
const char   WbemExecCommand::_OPTION_HTTPMETHOD   = 'm';

/**
    The option character used to specify the timeout value.
 */
const char   WbemExecCommand::_OPTION_TIMEOUT      = 't';

/**
    The option character used to specify that debug output is requested.
 */
const char   WbemExecCommand::_OPTION_DEBUG        = 'd';

/**
    The option character used to specify the username.
 */
const char   WbemExecCommand::_OPTION_USERNAME     = 'u';

/**
    The option character used to specify the password.
 */
const char   WbemExecCommand::_OPTION_PASSWORD     = 'w';

/**
    The minimum valid portnumber.
 */
const Uint32 WbemExecCommand::_MIN_PORTNUMBER      = 0;

/**
    The maximum valid portnumber.
 */
const Uint32 WbemExecCommand::_MAX_PORTNUMBER      = 65535;

/**
    The debug option argument value used to specify that the HTTP
    encapsulation of the original XML request be included in the output.
*/
const char   WbemExecCommand::_DEBUG_OPTION1       = '1';

/**
    The debug option argument value used to specify that the HTTP
    encapsulation of the XML response be included in the output.
*/
const char   WbemExecCommand::_DEBUG_OPTION2       = '2';

/**
  
    Constructs a WbemExecCommand and initializes instance variables.
  
 */
WbemExecCommand::WbemExecCommand ()
{

    _hostName            = String ();
    _hostNameSet         = false;
    _portNumber          = WBEM_DEFAULT_PORT;
    _portNumberSet       = false;

    char buffer[32];
    sprintf(buffer, "%lu", (unsigned long) _portNumber);
    _portNumberStr       = buffer;

    _useHTTP11           = true;   
    _useMPost            = true;   
    _timeout             = CIMClient::DEFAULT_TIMEOUT_MILLISECONDS;
    _timeout             = 200000;
    _debugOutput1        = false;
    _debugOutput2        = false;
    _userName            = String ();
    _password            = String ();
    _inputFilePath       = String ();
    _inputFilePathSet    = false;

    //
    //  Note: debug option is not shown in usage string.
    //  The debug option is not included in end-user documentation.
    //
    String usage = String (_USAGE);
    usage.append (COMMAND_NAME);
    usage.append (" [ -");
    usage.append (_OPTION_HOSTNAME);
    usage.append (" hostname ] [ -");
    usage.append (_OPTION_PORTNUMBER);
    usage.append (" portnumber ] [ -");
    usage.append (_OPTION_HTTPVERSION);
    usage.append (" httpversion ] [ -");
    usage.append (_OPTION_HTTPMETHOD);
    usage.append (" httpmethod ] [ -");
    usage.append (_OPTION_TIMEOUT);
    usage.append (" timeout ] [ -");
    usage.append (_OPTION_USERNAME);
    usage.append (" username ] [ -");
    usage.append (_OPTION_PASSWORD);
    usage.append (" password ] [ inputfilepath ] ");

    setUsage (usage);
}

/**
  
    Creates a channel for an HTTP connection.
  
    @param   outPrintWriter     the ostream to which error output should be
                                written
  
    @return  the Channel created
  
    @exception  WbemExecException  if an error is encountered in creating
                                   the connection
  
 */
Channel* WbemExecCommand::_getHTTPChannel (ostream& outPrintWriter) 
    throw (WbemExecException)
{
    Selector*              selector;
    ChannelHandlerFactory* factory;
    TCPChannelConnector*   connector;
    Channel*               channel               = NULL;
    String                 addressStr            = String ();
    char*                  address               = NULL;


    //
    //  Create HTTP channel
    //
    selector = new Selector ();
    factory = new WbemExecClientHandlerFactory (selector, outPrintWriter,
        _debugOutput2);
    connector = new TCPChannelConnector (factory, selector);

#ifdef PEGASUS_LOCAL_DOMAIN_SOCKET
    if ((!_hostNameSet) && (!_portNumberSet))
    {
#endif
    addressStr.append (_hostName);
    addressStr.append (":");
    addressStr.append (_portNumberStr);
#ifdef PEGASUS_LOCAL_DOMAIN_SOCKET
    }
#endif

    address = addressStr.allocateCString ();

    // ATTN-A: need connection timeout here:

    channel = connector->connect (address);

    delete [] address;

    if (!channel) 
    {
        WbemExecException e (WbemExecException::CONNECT_FAIL);
        throw e;
    }


    return (channel);
}
/**
  
    Check the HTTP response message for errors.
  
    @param   startLine             First header line from an HTTP response

    @return  true = successful response

 */
Boolean WbemExecCommand::_isHTTPOk( String startLine )
{
  String httpReturnCode;
  String httpVersion;

  // Response-Line = HTTP-Version SP HTTP-Return Code SP HTTP-Message Text

  // Extract HTTP version

  Uint32 space1 = startLine.find(' ');

  if (space1 == PEGASUS_NOT_FOUND)
    return false;

  httpVersion = startLine.subString(0, space1);
  
  // Extract the HTTP response code
  
  Uint32 space2 = startLine.find(space1 + 1, ' ');
  
  if (space2 == PEGASUS_NOT_FOUND)
    return false;
  
  Uint32 rcPos = space1 + 1;
  httpReturnCode = startLine.subString( rcPos, space2 - rcPos );

  if( String::equal( httpReturnCode, "200" ) )
    {
      return true;
    }
            
  // This is an error response

  return false;
}

/**
  
    Check the HTTP response message for authentication challenge or data.
  
    @param   channel             Connection to cimserver

    @param   handler             Communications processor for channel
 
    @param   content             Array <Sint8> containing XML request

    @param   httpHeaders         Array <Sint8> returning the HTTP headers

    @param   clientAuthenticator Authenticator object used to generate
                                 authentication headers

    @param   useAuthentication   Boolean indicating use of client authenticaion

    @param   ostream             the ostream to which output should be written

    @return  true  = wait for data from challenge response
    @return  false = client response has been received
  
 */
Boolean WbemExecCommand::_handleResponse( Channel*               channel,
                                          WbemExecClientHandler* handler,
                                          Array <Sint8>          content,
                                          Array <Sint8>          httpHeaders,
                                          ClientAuthenticator*   clientAuthenticator,
                                          Boolean                useAuthentication,
                                          ostream&               oStream
                                       )
{
    Array <Sint8>                responseMessage;
    String                       startLine;
    Array<HTTPHeader>            headers;
    Uint32                       contentLength;
    HTTPMessage*                 httpMessage;
    Boolean                      needsAuthentication = false;

    responseMessage = handler->getMessage();
    httpMessage = new HTTPMessage( responseMessage, 0 );
    httpMessage->parse( startLine, headers, contentLength );
    if( useAuthentication )
      {
	try
	  {
	    needsAuthentication = 
	      clientAuthenticator->checkResponseHeaderForChallenge( headers );
	  }
	catch(InvalidAuthHeader& e)
	  {
	    // This is an invalid authentication challenge
	    
	    oStream << startLine << endl;
	    oStream.flush();
	    exit( 1 );
	  }

	if( needsAuthentication )
	  {
	    //
	    // An authentication header was found.
	    // Get the original request, send with authentication challenge
	    // response. 
	    //
	    Array <Sint8>                challengeResponse;
	    
	    challengeResponse << httpHeaders;
	    
	    String authHeader = clientAuthenticator->buildRequestAuthHeader();
	    if (authHeader.size())
	      {
		challengeResponse <<  authHeader << HTTP_CRLF;
	      }
	    challengeResponse << HTTP_CRLF;
	    challengeResponse << content;
	    channel->writeN( challengeResponse.getData(), challengeResponse.size() );
	    return true;
	  }
      }
        if( !_isHTTPOk( startLine ) )
	  {
            // Received an HTTP error response
            // Output the HTTP error message and exit
	    
	    oStream << startLine << endl;
	    for (Uint32 i = 0, n = headers.size(); i < n; i++)
	      {
		oStream << headers[i].first << ": " << headers[i].second << endl;
	      }
	    oStream.flush();
	    exit( 1 );
	  }
	
        //
        // Received a valid response or an error response from the server.
        //
        return false;
}

/**
  
    Executes the command using HTTP.  A CIM request encoded in XML is read
    from the input, and encapsulated in an HTTP request message.  A channel
    is obtained for an HTTP connection, and the message is written to the
    channel.  The response is written to the specified outPrintWriter, and
    consists of the CIM response encoded in XML.
  
    @param   outPrintWriter     the ostream to which output should be
                                written
    @param   errPrintWriter     the ostream to which error output should be
                                written
  
    @exception  WbemExecException  if an error is encountered in executing
                                   the command
  
 */
void WbemExecCommand::_executeHttp (ostream& outPrintWriter, 
                                    ostream& errPrintWriter) 
    throw (WbemExecException)
{
    Channel*                     channel                        = NULL;
    WbemExecClientHandler*       handler                        = NULL;
    Uint32                       size;
    Array <Sint8>                content;
    Array <Sint8>                contentCopy;
    Array <Sint8>                message;
    Array <Sint8>                httpHeaders;
    ClientAuthenticator*         clientAuthenticator            = NULL;
    Boolean                      useAuthentication              = false;
    struct sockaddr_in           saddress;


    //
    //  Check for invalid combination of options
    //  The M-POST method may not be used with HTTP/1.0
    //
    if ((!_useHTTP11) && (_useMPost))
    {
        WbemExecException e 
            (WbemExecException::MPOST_HTTP10_INVALID);
        throw e;
    }

    //
    //  If no hostName specified 
    //  Default to local host
    //
    if (!_hostNameSet)
    {
        memset( &saddress, 0, sizeof(saddress) );

        saddress.sin_addr.s_addr = INADDR_LOOPBACK;
        _hostName = inet_ntoa( saddress.sin_addr );
    }
    if( !_portNumberSet )
    {
        _portNumber = System::lookupPort( WBEM_SERVICE_NAME,
    				          WBEM_DEFAULT_PORT );
        char buffer[32];
        sprintf( buffer, "%lu", (unsigned long) _portNumber );
        _portNumberStr = buffer;
    }

    //
    //  Get XML request from input file
    //
    if (_inputFilePathSet)
    {
        //
        //  Check that input file exists
        //
        if (!FileSystem::exists (_inputFilePath))

        {
            WbemExecException e 
                (WbemExecException::INPUT_FILE_NONEXISTENT);
            throw e;
        } 

        //
        //  Check that input file is readable
        //
        if (!FileSystem::canRead (_inputFilePath))
        {
            WbemExecException e 
                (WbemExecException::INPUT_FILE_NOT_READABLE);
            throw e;
        }

        //
        //  Check that file is not empty
        //
        FileSystem::getFileSize (_inputFilePath, size);
        if (size <= 0)
        {
            WbemExecException e (WbemExecException::NO_INPUT);
            throw e;
        }

        //
        //  Read from input file
        //
        try
        {
            FileSystem::loadFileToMemory (content, _inputFilePath);
            content.append ('\0');
        }
        catch (CannotOpenFile)
        {
            WbemExecException e 
                (WbemExecException::INPUT_FILE_CANNOT_OPEN);
            throw e;
        }
    } 
    else
    {
        //
        //  Read from cin
        //
        //  (GetLine is defined in Pegasus/Common/String.[h,cpp], but is
        //  not a class member.)
        //
        String line;

        while (GetLine (cin, line))
        {
            content << line << '\n';
        }

        if (content.size () <= 0)
        {
            //
            //  No input
            //
            WbemExecException e (WbemExecException::NO_INPUT);
            throw e;
        }
    }

    //
    //  Make a copy of the content because the XmlParser constructor
    //  modifies the text
    //
    contentCopy << content;

    XmlParser parser ((char*) contentCopy.getData ());

    //
    // Create authenticator for authentication header generation.
    //
    clientAuthenticator = new ClientAuthenticator();
    clientAuthenticator->clearRequest();
    useAuthentication = true;
    
    if( _userName.size() )
      {
	clientAuthenticator->setUserName( _userName );
      }
    if( _hostNameSet )
      {
	
        if( _password.size() )
	  {
            clientAuthenticator->setPassword( _password );
	  }
      }
    else 
    {
      // No hostname specified; use local authentication
      // We don't set password because cimserver will do a
      // local authentication challenge sequence.

        clientAuthenticator->setAuthType( ClientAuthenticator::LOCAL );
    }

    //
    //  Encapsulate XML request in an HTTP request
    //
    try
    {
        message = XMLProcess::encapsulate( parser, _hostName, 
                                           _useMPost, _useHTTP11,
                                           clientAuthenticator,
                                           useAuthentication,
                                           content, httpHeaders );
        if (_debugOutput1)
        {
          outPrintWriter << message.getData () << endl;
        }
    }
    catch (XmlValidationError xve)
    {
        WbemExecException e 
            (WbemExecException::INVALID_XML, xve.getMessage ());
        throw e;
    }
    catch (XmlSemanticError xse)
    {
        WbemExecException e 
            (WbemExecException::INVALID_XML, xse.getMessage ());
        throw e;
    }

    //
    //  Get channel and write message to channel
    //
    channel = _getHTTPChannel (outPrintWriter);
    channel->writeN (message.getData (), message.size ());

    //
    //  Get handler and wait for response
    //
    handler = (WbemExecClientHandler*) channel->getChannelHandler ();

    if (!handler->waitForResponse (_timeout))
    {
        WbemExecException e (WbemExecException::TIMED_OUT);
        throw e;
    }

    //
    // Process the response message
    //
    if( _handleResponse( channel, handler, content, httpHeaders,
                         clientAuthenticator, useAuthentication,
                         outPrintWriter ) == true )
    {
      // Wait for final response
      
      handler->clear();
      if (!handler->waitForResponse (_timeout))
        {
	  WbemExecException e (WbemExecException::TIMED_OUT);
	  throw e;
        }
      _handleResponse( channel, handler, content, httpHeaders,
		       clientAuthenticator, false,
		       outPrintWriter );
    }
    handler->printContent();
}

/**
  
    Parses the command line, validates the options, and sets instance
    variables based on the option arguments.
  
    @param   argc  the number of command line arguments
    @param   argv  the string vector of command line arguments
  
    @exception  CommandFormatException  if an error is encountered in parsing
                                        the command line
  
 */
void WbemExecCommand::setCommand (Uint32 argc, char* argv []) 
    throw (CommandFormatException)
{
    Uint32         i              = 0;
    Uint32         c              = 0;
    String         httpVersion    = String ();
    String         httpMethod     = String ();
    String         timeoutStr     = String ();
    String         GetOptString   = String ();
    getoopt        getOpts;

    //
    //  Construct GetOptString
    //
    GetOptString.append (_OPTION_HOSTNAME);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_PORTNUMBER);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_HTTPVERSION);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_HTTPMETHOD);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_TIMEOUT);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_USERNAME);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_PASSWORD);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);
    GetOptString.append (_OPTION_DEBUG);
    GetOptString.append (getoopt::GETOPT_ARGUMENT_DESIGNATOR);

    //
    //  Initialize and parse getOpts
    //
    getOpts = getoopt ();
    getOpts.addFlagspec (GetOptString);
    getOpts.parse (argc, argv);

    if (getOpts.hasErrors ())
    {
        CommandFormatException e (getOpts.getErrorStrings () [0]);
        throw e;
    }
    
    //
    //  Get options and arguments from the command line
    //
    for (i =  getOpts.first (); i <  getOpts.last (); i++)
    {
        if (getOpts [i].getType () == Optarg::LONGFLAG)
        {
            //
            //  This path should not be hit
            //  The wbemexec command has no LONGFLAG options
            //
        } 
        else if (getOpts [i].getType () == Optarg::REGULAR)
        {
            //
            // _inputFilePath is the only non-option argument
            //
            if (_inputFilePathSet)
            {
                //
                // more than one _inputFilePath argument was found
                //
                UnexpectedArgumentException e (getOpts [i].Value ()); 
                throw e;
            }
            _inputFilePath = getOpts [i].Value ();
            _inputFilePathSet = true;
        } 
        else /* getOpts [i].getType () == FLAG */
        {
            c = getOpts [i].getopt () [0];
    
            switch (c) 
            {
                case _OPTION_HOSTNAME: 
                {
                    if (getOpts.isSet (_OPTION_HOSTNAME) > 1)
                    {
                        //
                        // More than one hostname option was found
                        //
                        DuplicateOptionException e (_OPTION_HOSTNAME); 
                        throw e;
                    }
                    _hostName = getOpts [i].Value ();
                    _hostNameSet = true;
                    break;
                }
    
                case _OPTION_PORTNUMBER: 
                {
                    if (getOpts.isSet (_OPTION_PORTNUMBER) > 1)
                    {
                        //
                        // More than one portNumber option was found
                        //
                        DuplicateOptionException e (_OPTION_PORTNUMBER); 
                        throw e;
                    }
    
                    _portNumberStr = getOpts [i].Value ();
    
                    try
                    {
                        getOpts [i].Value (_portNumber);
                    }
                    catch (IncompatibleTypes it)
                    {
                        InvalidOptionArgumentException e (_portNumberStr,
                            _OPTION_PORTNUMBER);
                        throw e;
                    }
		    _portNumberSet = true;
                    break;
                }
    
                case _OPTION_HTTPVERSION: 
                {
                    if (getOpts.isSet (_OPTION_HTTPVERSION) > 1)
                    {
                        //
                        // More than one httpVersion option was found
                        //
                        DuplicateOptionException e (_OPTION_HTTPVERSION); 
                        throw e;
                    }
                    httpVersion = getOpts [i].Value ();
                    break;
                }
    
                case _OPTION_HTTPMETHOD: 
                {
                    if (getOpts.isSet (_OPTION_HTTPMETHOD) > 1)
                    {
                        //
                        // More than one httpMethod option was found
                        //
                        DuplicateOptionException e (_OPTION_HTTPMETHOD); 
                        throw e;
                    }
                    httpMethod = getOpts [i].Value ();
                    break;
                }
    
                case _OPTION_TIMEOUT: 
                {
                    if (getOpts.isSet (_OPTION_TIMEOUT) > 1)
                    {
                        //
                        // More than one timeout option was found
                        //
                        DuplicateOptionException e (_OPTION_TIMEOUT); 
                        throw e;
                    }
    
                    timeoutStr = getOpts [i].Value ();
    
                    try
                    {
                        getOpts [i].Value (_timeout);
                    }
                    catch (IncompatibleTypes it)
                    {
                        InvalidOptionArgumentException e (timeoutStr,
                            _OPTION_TIMEOUT);
                        throw e;
                    }
                    break;
                }
    
                case _OPTION_USERNAME: 
                {
                    if (getOpts.isSet (_OPTION_USERNAME) > 1)
                    {
                        //
                        // More than one username option was found
                        //
                        DuplicateOptionException e (_OPTION_USERNAME); 
                        throw e;
                    }
                    _userName = getOpts [i].Value ();
                    break;
                }
    
                case _OPTION_PASSWORD: 
                {
                    if (getOpts.isSet (_OPTION_PASSWORD) > 1)
                    {
                        //
                        // More than one password option was found
                        //
                        DuplicateOptionException e (_OPTION_PASSWORD); 
                        throw e;
                    }
                    _password = getOpts [i].Value ();
                    break;
                }
    
                case _OPTION_DEBUG: 
                {
                    //
                    //
                    String debugOptionStr;

                    debugOptionStr = getOpts [i].Value ();

                    if (debugOptionStr.size () != 1)
                    {
                        //
                        //  Invalid debug option
                        //
                        InvalidOptionArgumentException e (debugOptionStr,
                            _OPTION_DEBUG);
                        throw e;
                    }

                    if (debugOptionStr [0] == _DEBUG_OPTION1)
                    {
                        _debugOutput1 = true;
                    }
                    else if (debugOptionStr [0] == _DEBUG_OPTION2)
                    {
                        _debugOutput2 = true;
                    }
                    else
                    {
                        //
                        //  Invalid debug option
                        //
                        InvalidOptionArgumentException e (debugOptionStr,
                            _OPTION_DEBUG);
                        throw e;
                    }
                    break;
                }
    
                default:
                    //
                    //  This path should not be hit
                    //
                    break;
            }
        }
    }

    if (getOpts.isSet (_OPTION_USERNAME) < 1)
    {
        //
        //  No username specified
        //  TODO: Default to current username
        //
    }

    if (getOpts.isSet (_OPTION_PASSWORD) < 1)
    {
        //
        //  No password specified
        //  TODO: Prompt for password
        //
    }

    if (getOpts.isSet (_OPTION_PORTNUMBER) < 1)
    {
        //
        //  No portNumber specified
        //  Default to WBEM_DEFAULT_PORT
        //  Already done in constructor
        //
    } 
    else 
    {
        if (_portNumber > _MAX_PORTNUMBER)
        {
            //
            //  Portnumber out of valid range
            //
            InvalidOptionArgumentException e (_portNumberStr,
                _OPTION_PORTNUMBER);
            throw e;
        }
    }

    if (getOpts.isSet (_OPTION_HTTPVERSION) < 1)
    {
        //
        //  No httpVersion specified
        //  Default is to use HTTP/1.1
        //
        _useHTTP11 = true;
    } 
    else 
    {
        if (httpVersion == HTTP_VERSION_10)
        {
            _useHTTP11 = false;
        }

        //
        //  If version specified is "1.1", use HTTP/1.1
        //
        else if (httpVersion == HTTP_VERSION_11)
        {
            _useHTTP11 = true;
        }

        //
        //  Invalid (unsupported) HTTP version specified 
        //
        else
        {
            InvalidOptionArgumentException e (httpVersion,
                _OPTION_HTTPVERSION);
            throw e;
        }
    }

    if (getOpts.isSet (_OPTION_HTTPMETHOD) < 1)
    {
        //
        //  No httpMethod specified
        //  Default is to use M-POST
        //  unless HTTP/1.0 was specified
        //
        if (_useHTTP11)
        {
            _useMPost = true;
        } else 
        {
            _useMPost = false;
        }
    } 
    else 
    {
        //
        //  Use HTTP POST method
        //
        if (httpMethod == HTTP_METHOD_POST)
        {
            _useMPost = false;
        }
        //
        //  Use HTTP M-POST method
        //
        else if (httpMethod == HTTP_METHOD_MPOST)
        {
            _useMPost = true;
        }

        //
        //  Invalid HTTP method specified 
        //
        else
        {
            InvalidOptionArgumentException e (httpMethod,
                _OPTION_HTTPMETHOD);
            throw e;
        }
    }

    if (getOpts.isSet (_OPTION_TIMEOUT) < 1)
    {
        //
        //  No timeout specified
        //  Default to CIMClient::DEFAULT_TIMEOUT_MILLISECONDS
        //  Already done in constructor
        //
    } 
    else 
    {
        if (_timeout <= 0) 
        {
            //
            //  Timeout out of valid range
            //
            InvalidOptionArgumentException e (timeoutStr,
                _OPTION_TIMEOUT);
            throw e;
        }
    }
}


/**
  
    Executes the command and writes the results to the PrintWriters.
  
    @param   outPrintWriter     the ostream to which output should be
                                written
    @param   errPrintWriter     the ostream to which error output should be
                                written
  
    @return  0                  if the command is successful
             1                  if an error occurs in executing the command
  
 */
Uint32 WbemExecCommand::execute (ostream& outPrintWriter, 
                                 ostream& errPrintWriter) 
{
    try
    {
        _executeHttp (outPrintWriter, errPrintWriter);
    }
    catch (WbemExecException e)
    {
        errPrintWriter << e.getMessage () << endl;
        return (RC_ERROR);
    }
    return (RC_SUCCESS);
}

/**
    
    Parses the command line, and executes the command.
  
    @param   argc  the number of command line arguments
    @param   argv  the string vector of command line arguments
  
    @return  0                  if the command is successful
             1                  if an error occurs in executing the command
  
 */
PEGASUS_NAMESPACE_END

// exclude main from the Pegasus Namespace
PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

int main (int argc, char* argv []) 
{
    WbemExecCommand    command = WbemExecCommand ();
    int                rc;

    try 
    {
        command.setCommand (argc, argv);
    } 
    catch (CommandFormatException cfe) 
    {
        cerr << WbemExecCommand::COMMAND_NAME << ": " << cfe.getMessage () 
             << endl;
        cerr << command.getUsage () << endl;
        exit (Command::RC_ERROR);
    }

    rc = command.execute (cout, cerr);
    exit (rc);
    return 0;
}
