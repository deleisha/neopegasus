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
// Author: Nag Boranna (nagaraja_boranna@hp.com)
//
// Modified By:  Amit K Arora, IBM (amita@in.ibm.com)
//
//%/////////////////////////////////////////////////////////////////////////////


#include <cctype>
#include <fstream>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/Common/Tracer.h>

#include "ConfigExceptions.h"
#include "ConfigFile.h"
#if  defined(PEGASUS_OS_OS400)
#include "OS400ConvertChar.h"
#endif


PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN


////////////////////////////////////////////////////////////////////////////////
//
//  ConfigFile Class
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// ConfigTable 
////////////////////////////////////////////////////////////////////////////////

typedef HashTable<String, String, EqualFunc<String>, HashFunc<String> > Table;

struct ConfigTable
{
    Table table;
};


/*
    Config file header information
*/
static const char* ConfigHeader [] = {
    "########################################################################",
    "##                                                                    ##",
    "##                  CIM Server configuration file                     ##",
    "##                                                                    ##",
    "########################################################################",
    " ",    
    "########################################################################",
    "#                                                                      #",
    "# This is the configuration file for the CIMOM. The configuration      #",
    "# properties in this file are loaded in to CIMOM at startup.           #",
    "# CIMOM updates this file with the changes in the configurations.      #",
    "#                                                                      #",
    "# It is recommended that user do not edit this file, instead use       #",
    "# CIMConfigCommand to make any changes to the CIMOM configurations.    #",
    "#                                                                      #",
    "########################################################################",
    " "
};

static const int HEADER_SIZE = sizeof(ConfigHeader) / sizeof(ConfigHeader[0]);


/** 
    Constructor. 
*/
ConfigFile::ConfigFile (const String& fileName)
{
    _configFile = fileName;

    _configBackupFile = fileName + ".bak";

}

/** 
    Destructor. 
*/
ConfigFile::~ConfigFile ()
{

}


/** 
    Get the name of the configuration file.
*/
String ConfigFile::getFileName ()
{
    return ( _configFile );
}


/** 
    Load the properties from the config file.
*/
void ConfigFile::load (ConfigTable* confTable)
{
    String line;

    //
    // Delete the backup configuration file
    //
    if (FileSystem::exists(_configBackupFile))
    {
        FileSystem::removeFile(_configBackupFile);
    }

    //
    // Open the config file
    //
#if defined(PEGASUS_OS_OS400)
    ifstream ifs(_configFile.getCString(), PEGASUS_STD(_CCSID_T(1208)));
#else
    ifstream ifs(_configFile.getCString());
#endif
    if (!ifs)
    {
        return;
    }

    //
    // Read each line of the file
    //
    for (Uint32 lineNumber = 1; GetLine(ifs, line); lineNumber++)
    {
        // Get the property name and value

        //
        // Skip leading whitespace
        //
        const Char16* p = line.getChar16Data();

        while (*p && isspace(*p))
        {
            p++;
        }

        if (!*p)
        {
            continue;
        }

        //
        // Skip comment lines
        //
        if (*p == '#')
        {
            continue;
        }

        //
        // Get the property name
        //
        String name = String::EMPTY;

        if (!(isalpha(*p) || *p == '_'))
        {
            ifs.close();
            throw ConfigFileSyntaxError(_configFile, lineNumber);
        }

        name.append(*p++);

        while (isalnum(*p) || *p == '_')
        {
            name.append(*p++);
        }

        //
        // Skip whitespace after property name
        //
        while (*p && isspace(*p))
        {
            p++;
        }

        //
        // Expect an equal sign
        //
        if (*p != '=')
        {
            ifs.close();
            throw ConfigFileSyntaxError(_configFile, lineNumber);
        }

        p++;

        //
        // Skip whitespace after equal sign
        //
        while (*p && isspace(*p))
        {
            p++;
        }

        //
        // Get the value
        //
        String value = String::EMPTY;

        while (*p)
        {
            value.append(*p++);
        }

        //
        // Store the property name and value in the table
        //
        if (!confTable->table.insert(name, value))
        {
            //
            // Duplicate property, ignore the new property value.
            // FUTURE: Log this message in a log file.
            //
            PEG_TRACE_STRING(TRC_CONFIG, Tracer::LEVEL3,
                "Duplicate property '" + name + "', value '" + value + "' is ignored.");
        }
    }

    ifs.close();
}


/** 
    Save the properties to the config file.
*/
void ConfigFile::save (ConfigTable* confTable)
{
    //
    // Delete the backup configuration file
    //
    if (FileSystem::exists(_configBackupFile))
    {
        FileSystem::removeFile(_configBackupFile);
    }

    //
    // Rename the configuration file as a backup file
    //
    if (FileSystem::exists(_configFile))
    {
        if (!FileSystem::renameFile(_configFile, _configBackupFile))
        {
            throw CannotRenameFile(_configFile);
        }
    }

    //
    // Open the config file for writing
    //
#if defined(PEGASUS_OS_OS400)
    ofstream ofs(_configFile.getCString(), PEGASUS_STD(_CCSID_T(1208)));
#else
    ofstream ofs(_configFile.getCString());
#endif
    ofs.clear();

#if !defined(PEGASUS_OS_TYPE_WINDOWS)
    //
    // Set permissions on the config file to 0644
    //
    if ( !FileSystem::changeFilePermissions(_configFile,
        (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) )    // set 0644 
    {
        throw CannotOpenFile(_configFile);
    }
#endif

    //
    // Write config file header information
    //
    for (int index = 0; index < HEADER_SIZE; index++)
    {
        ofs << ConfigHeader[index] << endl;
    }

    ofs << endl;

    //
    // Save config properties and values to the file
    //
    for (Table::Iterator i = confTable->table.start(); i; i++)
    {
        ofs << i.key() << "=" << i.value() << endl;
    }

    ofs.close();
}


/** 
    Replace the properties in the config file with the properties from
    the given file.
*/
void ConfigFile::replace (const String& fileName)
{
    String line;

    //
    // Open the given config file for reading
    //
#if defined(PEGASUS_OS_OS400)
    ifstream ifs(fileName.getCString(), PEGASUS_STD(_CCSID_T(1208)));
#else
    ifstream ifs(fileName.getCString());
#endif

    //
    // Delete the backup configuration file
    //
    if (FileSystem::exists(_configBackupFile))
    {
        FileSystem::removeFile(_configBackupFile);
    }

    //
    // Rename the existing config file as a backup file
    //
    if (FileSystem::exists(_configFile))
    {
        if (!FileSystem::renameFile(_configFile, _configBackupFile))
        {
            ifs.close();
            throw CannotRenameFile(_configFile);
        }
    }

    //
    // Open the existing config file for writing
    //
#if defined(PEGASUS_OS_OS400)
    ofstream ofs(_configFile.getCString(), PEGASUS_STD(_CCSID_T(1208)));
#else
    ofstream ofs(_configFile.getCString());
#endif
    ofs.clear();

#if !defined(PEGASUS_OS_TYPE_WINDOWS)
    //
    // Set permissions on the config file to 0644
    //
    if ( !FileSystem::changeFilePermissions(_configFile,
        (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) )    // set 0644 
    {
        throw CannotOpenFile(_configFile);
    }
#endif

    //
    // Read each line of the new file and write to the config file.
    //
    for (Uint32 lineNumber = 1; GetLine(ifs, line); lineNumber++)
    {
        ofs << line << endl;
    }

    // 
    // Close the file handles
    //
    ifs.close();
    ofs.close();
}


PEGASUS_NAMESPACE_END

