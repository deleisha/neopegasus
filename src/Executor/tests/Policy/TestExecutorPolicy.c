/*
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
*/

#include <Executor/Policy.h>
#include <Executor/Macro.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#define TEST_DUMP_FILE "dumpfile"
#define MAX_DUMP_SIZE 4096

static struct Policy _testPolicyTable[] =
{
    {
        EXECUTOR_PING_MESSAGE,
        NULL,
        NULL
    },
    {
        EXECUTOR_RENAME_FILE_MESSAGE,
        "${file1}",
        "${file2}"
    },
    {
        EXECUTOR_RENAME_FILE_MESSAGE,
        "file1",
        "${file2}"
    },
    {
        EXECUTOR_RENAME_FILE_MESSAGE,
        "file1",
        "file2"
    }
};

static const size_t _testPolicyTableSize =
    sizeof(_testPolicyTable) / sizeof(_testPolicyTable[0]);

void testCheckPolicy(void)
{
    /* Test non-existent policy */
    assert(CheckPolicy(
        _testPolicyTable,
        _testPolicyTableSize,
        EXECUTOR_REAP_PROVIDER_AGENT_MESSAGE,
        NULL,
        NULL) != 0);

    /* Test policy with no arguments */
    assert(CheckPolicy(
        _testPolicyTable,
        _testPolicyTableSize,
        EXECUTOR_PING_MESSAGE,
        NULL,
        NULL) == 0);

    /* Test policies with invalid macro expansion in first argument and
     * non-match in first argument
     */
    assert(CheckPolicy(
        _testPolicyTable,
        _testPolicyTableSize,
        EXECUTOR_RENAME_FILE_MESSAGE,
        "MyFile",
        "file2") != 0);

    /* Test policies with invalid macro expansion in second argument and
     * non-match in second argument
     */
    assert(CheckPolicy(
        _testPolicyTable,
        _testPolicyTableSize,
        EXECUTOR_RENAME_FILE_MESSAGE,
        "file1",
        "MyFile") != 0);

    /* Test policy with successful match in both arguments */
    assert(CheckPolicy(
        _testPolicyTable,
        _testPolicyTableSize,
        EXECUTOR_RENAME_FILE_MESSAGE,
        "file1",
        "file2") == 0);
}

void testFilePolicies(void)
{
    const char* currentConfigFile = "MyConfigFile";
    const char* currentConfigFileBak = "MyConfigFile.bak";
    const char* noAccessFile = "NoAccessFile";

    /* Define a macro used in the static policy table */
    DefineMacro("currentConfigFilePath", currentConfigFile);

    assert(CheckOpenFilePolicy(currentConfigFile, 'w') == 0);
    assert(CheckOpenFilePolicy(noAccessFile, 'w') != 0);

    assert(CheckRemoveFilePolicy(currentConfigFile) == 0);
    assert(CheckRemoveFilePolicy(noAccessFile) != 0);

    assert(CheckRenameFilePolicy(currentConfigFile, currentConfigFileBak) == 0);
    assert(CheckRenameFilePolicy(currentConfigFile, noAccessFile) != 0);
}

void testDumpPolicy(void)
{
    FILE* dumpFile;
    char dumpFileBuffer[MAX_DUMP_SIZE];

    unlink(TEST_DUMP_FILE);

    /* Test DumpPolicy() with expandMacros=false */
    {
        const char* expectedDumpResult =
            "===== Policy:\n"
            "OpenFile(\"${currentConfigFilePath}\", \"w\")\n"
            "RenameFile(\"${currentConfigFilePath}\", "
                "\"${currentConfigFilePath}.bak\")\n"
            "RemoveFile(\"${currentConfigFilePath}\")\n"
            "RemoveFile(\"${currentConfigFilePath}.bak\")\n"
            "OpenFile(\"${plannedConfigFilePath}\", \"w\")\n"
            "RenameFile(\"${plannedConfigFilePath}\", "
                "\"${plannedConfigFilePath}.bak\")\n"
            "RemoveFile(\"${plannedConfigFilePath}\")\n"
            "RemoveFile(\"${plannedConfigFilePath}.bak\")\n"
            "OpenFile(\"${passwordFilePath}\", \"w\")\n"
            "RenameFile(\"${passwordFilePath}.bak\", \"${passwordFilePath}\")\n"
            "RenameFile(\"${passwordFilePath}\", \"${passwordFilePath}.bak\")\n"
            "RemoveFile(\"${passwordFilePath}.bak\")\n"
            "RemoveFile(\"${passwordFilePath}\")\n"
            "OpenFile(\"${sslKeyFilePath}\", \"r\")\n"
            "OpenFile(\"${sslTrustStore}/*\", \"w\")\n"
            "RemoveFile(\"${sslTrustStore}/*\")\n"
            "OpenFile(\"${crlStore}/*\", \"w\")\n"
            "RemoveFile(\"${crlStore}/*\")\n"
            "\n";

        dumpFile = fopen(TEST_DUMP_FILE, "a");
        assert(dumpFile != 0);
        DumpPolicy(dumpFile, 0);
        fclose(dumpFile);

        dumpFile = fopen(TEST_DUMP_FILE, "rb");
        memset(dumpFileBuffer, 0, MAX_DUMP_SIZE);
        fread(dumpFileBuffer, sizeof(char), MAX_DUMP_SIZE - 1, dumpFile);
        fclose(dumpFile);

        assert(strcmp(dumpFileBuffer, expectedDumpResult) == 0);

        unlink(TEST_DUMP_FILE);
    }

    /* Test DumpPolicyHelper() with expandMacros=false */
    {
        const char* expectedDumpResult =
            "Ping()\n"
            "RenameFile(\"${file1}\", \"${file2}\")\n"
            "RenameFile(\"file1\", \"${file2}\")\n"
            "RenameFile(\"file1\", \"file2\")\n";

        dumpFile = fopen(TEST_DUMP_FILE, "a");
        assert(dumpFile != 0);
        DumpPolicyHelper(dumpFile, _testPolicyTable, _testPolicyTableSize, 0);
        fclose(dumpFile);

        dumpFile = fopen(TEST_DUMP_FILE, "rb");
        memset(dumpFileBuffer, 0, MAX_DUMP_SIZE);
        fread(dumpFileBuffer, sizeof(char), MAX_DUMP_SIZE - 1, dumpFile);
        fclose(dumpFile);

        assert(strcmp(dumpFileBuffer, expectedDumpResult) == 0);

        unlink(TEST_DUMP_FILE);
    }

    /* Test DumpPolicyHelper() with expandMacros=true */
    {
        const char* expectedDumpResult =
            "Ping()\n"
            "RenameFile(\"MyFile1\", \"MyFile2\")\n"
            "RenameFile(\"file1\", \"MyFile2\")\n"
            "RenameFile(\"file1\", \"file2\")\n";

        DefineMacro("file1", "MyFile1");
        DefineMacro("file2", "MyFile2");

        dumpFile = fopen(TEST_DUMP_FILE, "a");
        assert(dumpFile != 0);
        DumpPolicyHelper(dumpFile, _testPolicyTable, _testPolicyTableSize, 1);
        fclose(dumpFile);

        UndefineMacro("file1");
        UndefineMacro("file2");

        dumpFile = fopen(TEST_DUMP_FILE, "rb");
        memset(dumpFileBuffer, 0, MAX_DUMP_SIZE);
        fread(dumpFileBuffer, sizeof(char), MAX_DUMP_SIZE - 1, dumpFile);
        fclose(dumpFile);

        assert(strcmp(dumpFileBuffer, expectedDumpResult) == 0);

        unlink(TEST_DUMP_FILE);
    }
}

int main()
{
    testCheckPolicy();
    testFilePolicies();
    testDumpPolicy();

    printf("+++++ passed all tests\n");

    return 0;
}