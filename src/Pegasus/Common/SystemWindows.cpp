//%2003////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002  BMC Software, Hewlett-Packard Development
// Company, L. P., IBM Corp., The Open Group, Tivoli Systems.
// Copyright (c) 2003 BMC Software; Hewlett-Packard Development Company, L. P.;
// IBM Corp.; EMC Corporation, The Open Group.
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
// Modified By: Sushma Fernandes (sushma_fernandes@hp.com)
//              Nag Boranna, Hewlett-Packard Company (nagaraja_boranna@hp.com)
//              Bapu Patil (bapu_patil@hp.com)
//
// Modified By: Dave Rosckes (rosckes@us.ibm.com)
//              Terry Martin, Hewlett-Packard Company (terry.martin@hp.com)
//              Amit K Arora, IBM (amita@in.ibm.com) for Bug#1428
//				Seema Gupta (gseema@in.ibm.com) for Bug#1617
//
//%/////////////////////////////////////////////////////////////////////////////

#include "System.h"

#include <windows.h>
#ifndef _WINSOCKAPI_
#include <winsock2.h>
#endif
#include <fcntl.h>
#include <sys/types.h>
#include <time.h>
#include <sys/timeb.h>
#include <io.h>
#include <conio.h>
#include <direct.h>
#include <sys/types.h>
#include <windows.h>
#include <process.h>
#include <lm.h>

PEGASUS_NAMESPACE_BEGIN

#define PEGASUS_ACCESS_EXISTS 0
#define PEGASUS_ACCESS_WRITE 2
#define PEGASUS_ACCESS_READ 4
#define PEGASUS_ACCESS_READ_AND_WRITE 6

#define PW_BUFF_LEN 65

void System::getCurrentTime(Uint32& seconds, Uint32& milliseconds)
{
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER largeInt = { ft.dwLowDateTime, ft.dwHighDateTime };
    largeInt.QuadPart -= 0x19db1ded53e8000;
    seconds = long(largeInt.QuadPart / (10000 * 1000));
    milliseconds = long((largeInt.QuadPart % (10000 * 1000)) / 10);
    // This is a real hack. Added the following line after timevalue was
    // corrected and this apparently wrong. ks 7 apri 2002
    milliseconds = milliseconds / 1000;
}

String System::getCurrentASCIITime()
{
    char tmpbuf[128];
    _strdate( tmpbuf );
    String date = tmpbuf;
    _strtime( tmpbuf );
    date.append("-");
    date.append(tmpbuf);
    return date;
}

void System::sleep(Uint32 seconds)
{
    Sleep(seconds * 1000);
}

Boolean System::exists(const char* path)
{
    return _access(path, PEGASUS_ACCESS_EXISTS) == 0;
}

Boolean System::canRead(const char* path)
{
    return _access(path, PEGASUS_ACCESS_READ) == 0;
}

Boolean System::canWrite(const char* path)
{
    return _access(path, PEGASUS_ACCESS_WRITE) == 0;
}

Boolean System::getCurrentDirectory(char* path, Uint32 size)
{
    return GetCurrentDirectory(size, path) != 0;
}

Boolean System::isDirectory(const char* path)
{
    struct stat st;

    if (stat(path, &st) != 0)
	return false;

    return (st.st_mode & _S_IFDIR) != 0;
}

Boolean System::changeDirectory(const char* path)
{
    return chdir(path) == 0;
}

Boolean System::makeDirectory(const char* path)
{
    return _mkdir(path) == 0;
}

Boolean System::getFileSize(const char* path, Uint32& size)
{
    struct stat st;

    if (stat(path, &st) != 0)
	return false;

    size = st.st_size;
    return true;
}

Boolean System::removeDirectory(const char* path)
{
    return rmdir(path) == 0;	
}

Boolean System::removeFile(const char* path)
{
    return unlink(path) == 0;	
}

Boolean System::renameFile(const char* oldPath, const char* newPath)
{
    return rename(oldPath, newPath) == 0;
}

DynamicLibraryHandle System::loadDynamicLibrary(const char* fileName)
{
    return DynamicLibraryHandle(LoadLibrary(fileName));
}

void System::unloadDynamicLibrary(DynamicLibraryHandle libraryHandle)
{
	FreeLibrary(HINSTANCE(libraryHandle));
}

String System::dynamicLoadError(void) {
return String();
}

DynamicSymbolHandle System::loadDynamicSymbol(
    DynamicLibraryHandle libraryHandle,
    const char* symbolName)
{
    return DynamicSymbolHandle(GetProcAddress(
	(HINSTANCE)libraryHandle, symbolName));
}

String System::getHostName()
{
    static char hostname[PEGASUS_MAXHOSTNAMELEN];

    if (!*hostname)
        gethostname(hostname, sizeof(hostname));

    return hostname;
}

String System::getFullyQualifiedHostName ()
{
    static char FQHostName[PEGASUS_MAXHOSTNAMELEN];

    if (!*FQHostName)
    {
        String hostname = getHostName();
        struct hostent* hostEnt;

        hostEnt = gethostbyname((const char *)hostname.getCString());
        if (hostEnt == NULL)
        {
            return String::EMPTY;
        }
        strcpy(FQHostName, hostEnt->h_name);
    }
   
    return FQHostName;
}

String System::getSystemCreationClassName ()
{
    return "CIM_ComputerSystem";
}

Uint32 System::lookupPort(
    const char * serviceName,
    Uint32 defaultPort)
{
    Uint32 localPort;

    struct servent *serv;

    //
    // Get wbem-local port from /etc/services
    //
    if (  (serv = getservbyname(serviceName, TCP)) != NULL )
    {
        localPort = serv->s_port;
    }
    else
    {
        localPort = defaultPort;
    }

    return localPort;
}

String System::getPassword(const char* prompt)
{
  char password[PW_BUFF_LEN] = {0};
  int num_chars = 0;
  int ch;

  fputs(prompt, stderr);

  while ((ch = _getch()) != '\r' &&
         num_chars < PW_BUFF_LEN)
    {
      // EOF
      if (ch == EOF)
        {
           fputs("[EOF]\n", stderr);
           return String::EMPTY;
        }
      // Backspace or Delete
      else if ((ch == '\b' || ch == 127) &&
               num_chars > 0) 
        {
          password[--num_chars] = '\0';
          fputs("\b \b", stderr);
        }
      // CTRL+C
      else if (ch == 3)
        {
          // _getch() does not catch CTRL+C
          fputs("^C\n", stderr);
          exit(-1);
        }
      // CTRL+Z
      else if (ch == 26)
        {
          fputs("^Z\n", stderr);
          return String::EMPTY;
        }
      // Esc
      else if (ch == 27)
        {
          fputc('\n', stderr);
          fputs(prompt, stderr);
          num_chars = 0;
        }
      // Function keys (0 or E0) are a guards for a Function key codes
      else if (ch == 0 || ch == 0xE0)
        {
          ch = (ch << 4) | _getch();
          // Handle DELETE, left arrow, keypad DEL, and keypad left arrow
          if ((ch == 0xE53 || ch == 0xE4B || ch == 0x053 || ch == 0x04b) &&
              num_chars > 0)
            {
              password[--num_chars] = '\0';
              fputs("\b \b", stderr);
            }
          else
            {
              fputc('\a', stderr);
            }
        }
      else if ((num_chars < sizeof(password) - 1) &&
               !iscntrl(((unsigned char)(ch))))
        {
          password[num_chars++] = ch;
          fputc('*', stderr);
        }
      else
        {
          fputc('\a', stderr);
        }
    }

  fputc('\n', stderr);
  password[num_chars] = '\0';

  return String(password);
}

String System::getEffectiveUserName()
{
  int retcode = 0;

  // UNLEN (256) is the limit, not including null
  char pUserName[256+1] = {0};
  DWORD nSize = sizeof(pUserName);

  retcode = GetUserName(pUserName, &nSize);
  if (retcode == 0)
    {
      // zero is failure
      return String();
    }
  
  return String(pUserName);
}

String System::encryptPassword(const char* password, const char* salt)
{
  BYTE pbBuffer[PW_BUFF_LEN] = {0};
  DWORD dwByteCount;
  char pcSalt[3] = {0};

  strncpy(pcSalt, salt, 2);
  dwByteCount = strlen(password);
  memcpy(pbBuffer, password, dwByteCount);
  for (DWORD i=0; (i<dwByteCount) || (i>=PW_BUFF_LEN); i++)
    (i%2 == 0) ? pbBuffer[i] ^= pcSalt[1] : pbBuffer[i] ^= pcSalt[0];
  
  return String(pcSalt) + String((char *)pbBuffer);
}

Boolean System::isSystemUser(const char* userName)
{
    //ATTN: Implement this method to verify if user is vaild on the local system
    //      This is used in User Manager
    return true;
}

Boolean System::isPrivilegedUser(const String userName)
{
    // ATTN: Implement this method to verify if user executing the current
    //       command is a priviliged user, when user name is not passed as
    //       as argument. If user name is passed the function checks 
    //       whether the given user is a priviliged user.
    //       This is used in cimuser CLI and CIMOperationRequestAuthorizer
    return true;
}

String System::getPrivilegedUserName()
{
    // ATTN-NB-03-20000304: Implement better way to get the privileged
    // user on the system.

    return (String("Administrator"));
}

Boolean System::isGroupMember(const char* userName, const char* groupName)
{
   Boolean retVal = false;

   LPLOCALGROUP_USERS_INFO_0 pBuf = NULL;
   DWORD dwLevel = 0;
   DWORD dwFlags = LG_INCLUDE_INDIRECT ;
   DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
   DWORD dwEntriesRead = 0;
   DWORD dwTotalEntries = 0;
   NET_API_STATUS nStatus;


   //
   // Call the NetUserGetLocalGroups function 
   // specifying information level 0.
   //
   // The LG_INCLUDE_INDIRECT flag specifies that the 
   // function should also return the names of the local 
   // groups in which the user is indirectly a member.
   //
   nStatus = NetUserGetLocalGroups(NULL,
                                   (LPCWSTR)userName,
                                   dwLevel,
                                   dwFlags,
                                   (LPBYTE *) &pBuf,
                                   dwPrefMaxLen,
                                   &dwEntriesRead,
                                   &dwTotalEntries);

   //
   // If the call succeeds,
   //
   if (nStatus == NERR_Success)
   {
      LPLOCALGROUP_USERS_INFO_0 pTmpBuf;
      DWORD i;
      DWORD dwTotalCount = 0;

      if ((pTmpBuf = pBuf) != NULL)
      {
         //
         // Loop through the local groups that the user belongs
         // and find the matching group name.
         //
         for (i = 0; i < dwEntriesRead; i++)
         {
            //
            // Compare the user's group name to groupName.
            //
            if ( strcmp ((char *)pTmpBuf->lgrui0_name, groupName) == 0 )
            {
                 // User is a member of the group.
                 retVal = true;
                 break;
            }

            pTmpBuf++;
            dwTotalCount++;
         }
      }
   }

   //
   // Free the allocated memory.
   //
   if (pBuf != NULL)
      NetApiBufferFree(pBuf);

   //
   // If the given user and group are not found in the local group
   // then try on the global groups.
   //
   if (!retVal)
   {
       LPGROUP_USERS_INFO_0 pBuf = NULL;
       dwLevel = 0;
       dwPrefMaxLen = MAX_PREFERRED_LENGTH;
       dwEntriesRead = 0;
       dwTotalEntries = 0;

       //
       // Call the NetUserGetGroups function, specifying level 0.
       //
       nStatus = NetUserGetGroups(NULL,
                                  (LPCWSTR)userName,
                                  dwLevel,
                                  (LPBYTE*)&pBuf,
                                  dwPrefMaxLen,
                                  &dwEntriesRead,
                                  &dwTotalEntries);
       //
       // If the call succeeds,
       //
       if (nStatus == NERR_Success)
       {
          LPGROUP_USERS_INFO_0 pTmpBuf;
          DWORD i;
          DWORD dwTotalCount = 0;
    
          if ((pTmpBuf = pBuf) != NULL)
          {
             //
             // Loop through the global groups to which the user belongs
             // and find the matching group name.
             //
             for (i = 0; i < dwEntriesRead; i++)
             {
                //
                // Compare the user's group name to groupName.
                //
                if ( strcmp ((char *)pTmpBuf->grui0_name, groupName) == 0 )
                {
                     // User is a member of the group.
                     retVal = true;
                     break;
                }

                pTmpBuf++;
                dwTotalCount++;
             }
          }
       }

       //
       // Free the allocated buffer.
       //
       if (pBuf != NULL)
          NetApiBufferFree(pBuf);
   }

   return retVal;
}
    
Uint32 System::getPID()
{
    return _getpid();
}

Boolean System::truncateFile(
    const char* path, 
    size_t newSize)
{
    int fd = open(path, O_RDWR);

    if (fd == -1)
        return false;

    if (chsize(fd, newSize) != 0)
	return false;

    close(fd);
    return true;
}

// Is absolute path?
Boolean System::is_absolute_path(const char *path)
{
  char full[_MAX_PATH];
  char path_slash[_MAX_PATH];
  char *p;

  strncpy(path_slash, path, _MAX_PATH);
  path_slash[_MAX_PATH-1] = '\0';

  for(p = path_slash; p < path_slash + strlen(path_slash); p++)
    if (*p == '/') *p = '\\';
  
  return (strcasecmp(_fullpath( full, path_slash, _MAX_PATH ), path_slash) == 0) ? true : false;
}

// Changes file permissions on the given file.
Boolean System::changeFilePermissions(const char* path, mode_t mode)
{
    // ATTN: File permissions are not currently defined in Windows
    return true;
}

void System::openlog(const String ident)
{
    return;
}

void System::syslog(Uint32 severity, const char *data)
{
    return;
}

void System::closelog()
{
    return;
}

// System ID constants for Logger::put and Logger::trace
const String System::CIMSERVER = "cimserver";  // Server system ID

PEGASUS_NAMESPACE_END
