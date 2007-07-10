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
//%=============================================================================

#include <utils/mu/Dependency.h>
#include <cstdio>
#include <cstddef>
#include <Pegasus/Common/PegasusAssert.h>

void ErrorExit(const char* programName, const string& message)
{
    fprintf(stderr, "%s: Error: %s\n", programName, message.c_str());
    exit(1);
}

void Warning(const char* programName, string& message)
{
    fprintf(stderr, "%s: Warning: %s\n", programName, message.c_str());
}

void ProcessOptions(
    int& argc,
    char**& argv,
    const char* programName,
    vector<string>& includePath,
    string& objectDir,
    string& prependDir,
    bool& warn)
{
    int i;

    for (i = 0; i < argc; i++)
    {
        const char* p = argv[i];

        if (*p != '-')
        {
            break;
        }

        p++;

        if (*p == 'I')
        {
            if (*++p)
            {
                includePath.push_back(p);
            }
            else
            {
                ErrorExit(programName, "Missing argument for -I option");
            }
        }
        else if (*p == 'O')
        {
            if (*++p)
            {
                objectDir = p;
            }
            else
            {
                ErrorExit(programName, "Missing argument for -O option");
            }
        }
        else if (*p == 'D')
        {
            if(*++p)
            {
                prependDir = p;
            }
            else
            {
                ErrorExit(programName, "Missing argument for -D option");
            }
        }
        else if (*p == 'W' && p[1] == '\0')
        {
            warn = true;
        }
        else
        {
            ErrorExit(programName, string("Unknown option: -") + *p);
        }
    }

    argc -= i;
    argv += i;
}

void PrintVector(const vector<string>& v)
{
    for (size_t i = 0; i < v.size(); i++)
    {
        printf("%s\n", v[i].c_str());
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// GetIncludePath():
//
//     Get the include path from an #include directive.
//
////////////////////////////////////////////////////////////////////////////////

bool GetIncludePath(
    const string& fileName,
    size_t lineNumber,
    const char* line,
    string& path,
    char& openDelim)
{
    if (line[0] == '#')
    {
        const char* p = line + 1;

        // Skip whitespace:

        while (isspace(*p))
        {
            p++;
        }
        // Check for "include" keyword:

        const char INCLUDE[] = "include";

        if (memcmp(p, INCLUDE, sizeof(INCLUDE) - 1) == 0)
        {
            // Advance past "include" keyword:

            p += sizeof(INCLUDE) - 1;

            // Skip whitespace:

            while (isspace(*p))
            {
                p++;
            }

            // Expect opening '"' or '<':

            if (*p != '"' && *p != '<')
            {
                return false;
#if 0
                // ATTN: noticed that "#include /**/ <path>" style not
                // handle so just returning silently when this situration
                // encountered!

                char message[64];
                sprintf(message,
                    "corrupt #include directive at %s(%d)",
                    fileName.c_str(),
                    lineNumber);
                ErrorExit(message);
#endif
            }

            openDelim = *p++;

            // Skip whitespace:

            while (isspace(*p))
            {
                p++;
            }
            // Store pointer to start of path:

            const char* start = p;

            // Look for closing '"' or '>':

            while (*p && *p != '"' && *p != '>')
            {
                p++;
            }

            if (*p != '"' && *p != '>')
            {
                return false;
#if 0
                char message[64];
                sprintf(message,
                    "corrupt #include directive at %s(%d)",
                    fileName.c_str(),
                    lineNumber);
                ErrorExit(message);
#endif
            }

            // Find end of the path (go backwards, skipping whitespace
            // between the closing delimiter and the end of the path:

            if (p == start)
            {
                return false;
#if 0
                char message[64];
                sprintf(message,
                    "empty path in #include directive at %s(%d)",
                    fileName.c_str(),
                    lineNumber);
                ErrorExit(message);
#endif
            }

            p--;

            while (isspace(*p))
            {
                p--;
            }

            if (p == start)
            {
                return false;
#if 0
                char message[64];
                sprintf(message,
                    "empty path in #include directive at %s(%d)",
                    fileName.c_str(),
                    lineNumber);
                ErrorExit(message);
#endif
            }

            path.assign(start, p - start + 1);
            return true;
        }
    }

    return false;
}

FILE* FindFile(
    const vector<string>& includePath,
    const string& prependDir,
    const string& path,
    char openDelim,
    string& fullPath)
{
    // If the opening delimiter was '"', then check the current
    // directory first:

    if (openDelim == '"')
    {
        FILE* fp = fopen(path.c_str(), "rb");

        if (fp)
        {
            if (prependDir.size())
            {
                fullPath = prependDir;
                fullPath += '/';
                fullPath += path;
            }
            else
            {
                fullPath = path;
            }
            return fp;
        }
    }

    // Search the include path for the file:

    vector<string>::const_iterator first = includePath.begin();
    vector<string>::const_iterator last = includePath.end();

    for (; first != last; first++)
    {
        fullPath = *first;
        fullPath += '/';
        fullPath += path;

        FILE* fp = fopen(fullPath.c_str(), "rb");

        if (fp)
        {
            return fp;
        }
    }

    return NULL;
}

void ProcessFile(
    const string& objectFileName,
    const string& fileName,
    const char* programName,
    FILE* fp,
    const vector<string>& includePath,
    string& prependDir,
    size_t nesting,
    set<string, less<string> >& cache,
    PrintFunc printFunc,
    bool& warn)
{
    printFunc(objectFileName, fileName);

    if (nesting == 100)
    {
        ErrorExit(programName,
            "Infinite include file recursion? nesting level reached 100");
    }

    PEGASUS_ASSERT(fp != NULL);

    // For each line in the file:

    char line[4096];
    size_t lineNumber = 1;

    for (; fgets(line, sizeof(line), fp) != NULL; lineNumber++)
    {
        // Check for include directive:

        string path;
        char openDelim;

        if (line[0] == '#' &&
            GetIncludePath(fileName, lineNumber, line, path, openDelim))
        {
            // ATTN: danger! not distinguising between angle brack delimited
            // and quote delimited paths!

            set<string, less<string> >::const_iterator pos
                = cache.find(path);

            if (pos != cache.end())
            {
                continue;
            }

            cache.insert(path);

            string fullPath;
            FILE* fp = FindFile(includePath, prependDir, path, openDelim,
                fullPath);

            if (!fp)
            {
                if (warn)
                {
                    string message = "header file not found: " + path +
                        " included from " + fileName;
                    Warning(programName, message);
                }
            }
            else
            {
                ProcessFile(objectFileName, fullPath,programName, fp,
                    includePath, prependDir, nesting + 1, cache, printFunc,
                    warn);
            }
        }
    }

    fclose(fp);
}
