//%2006////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002 BMC Software; Hewlett-Packard Development
// Company, L.P.; IBM Corp.; The Open Group; Tivoli Systems.
// Copyright (c) 2003 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation, The Open Group.
// Copyright (c) 2004 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation; VERITAS Software Corporation; The Open Group.
// Copyright (c) 2005 Hewlett-Packard Development Company, L.P.; IBM Corp.;
// EMC Corporation; VERITAS Software Corporation; The Open Group.
// Copyright (c) 2006 Hewlett-Packard Development Company, L.P.; IBM Corp.;
// EMC Corporation; Symantec Corporation; The Open Group.
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
//%/////////////////////////////////////////////////////////////////////////////

/*
    This program enables the creation of namespaces exploiting Shared Schema 
    facilities and namespaces designated to be used as remote namespaces using
    Remote CMPI facilities. This uses the CIMRepository interface and namespace
    can be created independent of repository store.

    Unlike other Remote CMPI libraries/programs , this program uses  pegasus
    libraries to make use of repository interface.
*/

#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/Common/OptionManager.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

void getOptions(
    OptionManager &om,
    int &argc,
    char **argv)
{
    static struct OptionRow optionTable[] =
    {
        {"repositoryPath", "", true, 
            Option::STRING, 0, 0, "r", "Repository path"},
        {"parent", "", false, 
            Option::STRING, 0, 0, "p", "Parent namespace (optional)."
                "Parent namespace will be modified to be shareable if"
                " it was not already shareable"},
        {"namespaceName", "", true, 
            Option::STRING, 0, 0, "n", "Namespace name"},
        {"location", "", true, 
            Option::STRING, 0, 0, "l", "Remote location"},
        {"shareable", "true", false, 
            Option::STRING, 0, 0, "s", "Specifies whether the"
                " namespace can shareable or not"},
        {"updatesAllowed", "false", false, 
            Option::STRING, 0, 0, "u", "Specifies whether the "
                "updates to the namespace are allowed or not"}
    };
    const Uint32 NUM_OPTIONS = sizeof(optionTable)/ sizeof(optionTable[0]);
    om.registerOptions(optionTable, NUM_OPTIONS);
    om.mergeCommandLine(argc, argv);
    om.checkRequiredOptions();
}

int main(int argc, char* argv[])
{
    try
    {
        OptionManager om;
        try
        {
            getOptions(om, argc, argv);
        }
        catch(Exception &e)
        {
            cerr << argv[0] << ":" << e.getMessage() << endl;
            String header = "Usage "; 
            String trailer ="";
            om.printOptionsHelpTxt(header, trailer);
            exit(1);
        } 
        String parent;
        String repositoryPath;
        String namespaceName;
        String location;
        String shareable;
        String updatesAllowed;

        om.lookupValue("parent", parent);
        om.lookupValue("repositoryPath", repositoryPath);
        om.lookupValue("namespaceName", namespaceName);
        om.lookupValue("location", location);
        om.lookupValue("shareable", shareable);
        om.lookupValue("updatesAllowed", updatesAllowed);
        
        CIMRepository repository(repositoryPath);
        CIMRepository::NameSpaceAttributes attributes;
        if (parent.size())
        {
            // Check if parent namespace is shareable else modify to be
            // shareable.
            repository.getNameSpaceAttributes(parent, attributes);
            for (CIMRepository::NameSpaceAttributes::Iterator 
                i = attributes.start(); i; i++)
            {
                String key = i.key();
                String value = i.value();
                if (String::equalNoCase(key, "shareable"))
                {
                    if (String::equalNoCase(value, "false"))
                    {
                        CIMRepository::NameSpaceAttributes attr;
                        attr.insert("shareable", "true");
                        repository.modifyNameSpace(
                            parent,
                            attr);
                        break;
                    }
                }
            }
            attributes.clear();
            attributes.insert("parent", parent);
        }
        attributes.insert("remoteInfo", location);
        attributes.insert("shareable", shareable);
        attributes.insert("updatesAllowed", updatesAllowed);
        repository.createNameSpace(namespaceName, attributes);
    }
    catch(Exception &e)
    {
        cerr << argv[0] << ":" << e.getMessage() << endl;
        exit(1);
    }
    catch(...)
    {
        cerr << argv[0] << ":" << "Unknown error" << endl;
        exit(1);
    }
    cout << "Namespace created successfully." << endl;

    return 0;
}
