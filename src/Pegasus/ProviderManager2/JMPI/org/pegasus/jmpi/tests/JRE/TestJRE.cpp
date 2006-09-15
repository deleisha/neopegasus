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

#include "Convert.h"
#include "JMPIImpl.h"
#include "JMPIProvider.h"

#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Common/CIMParamValue.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Common/FileSystem.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

int trace = 0;

#ifdef PEGASUS_DEBUG
#define DDD(x) if (trace) x;
#else
#define DDD(x)
#endif

void
printEnvironmentVariables ()
{
   const char *apszEnvVariables[] = {
      "PEGASUS_HOME",
      "PEGASUS_ROOT",
      "PEGASUS_PLATFORM",
      "PEGASUS_DEBUG",
      "PEGASUS_JMPI_TRACE",
      "PATH",
      "LD_LIBRARY_PATH",
      "CLASSPATH",
      "JAVA_SDK",
      "JAVA_SDKINC"
   };

   for (size_t i = 0; i < sizeof (apszEnvVariables)/sizeof (apszEnvVariables[0]); i++)
   {
      const char *pszValue = 0;

      pszValue = getenv (apszEnvVariables[i]);

      if (pszValue)
      {
         printf ("%s = \"%s\"\n", apszEnvVariables[i], pszValue);
      }
      else
      {
         printf ("%s = NULL\n", apszEnvVariables[i]);
      }
   }
}

int testJVM ()
{
   JvmVector  *jv   = NULL;
   JNIEnv     *jEnv = NULL;

   jEnv = JMPIjvm::attachThread (&jv);

   if (!jEnv)
   {
      PEGASUS_STD(cerr)<<"testJVM: FAILURE: Could not attach a thread!"<<PEGASUS_STD(endl);
      return 1;
   }

   jobject jSI8 = jEnv->NewObject (JMPIjvm::jv.ByteClassRef,
                                   JMPIjvm::jv.ByteNewB,
                                   (jbyte)-42);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Byte" << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jSI8)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Byte" << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create Byte" << PEGASUS_STD (endl);
   }
   jobject jUI8 = jEnv->NewObject (JMPIjvm::jv.UnsignedInt8ClassRef,
                                   JMPIjvm::jv.UnsignedInt8NewS,
                                   (jshort)16);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create UnsignedInt8" << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jUI8)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create UnsignedInt8" << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create UnsignedInt8" << PEGASUS_STD (endl);
   }
   jobject jSI16 = jEnv->NewObject (JMPIjvm::jv.ShortClassRef,
                                    JMPIjvm::jv.ShortNewS,
                                    (jshort)-1578);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Short" << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jSI16)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Short" << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create Short" << PEGASUS_STD (endl);
   }
   jobject jUI16 = jEnv->NewObject (JMPIjvm::jv.UnsignedInt16ClassRef,
                                    JMPIjvm::jv.UnsignedInt16NewI,
                                    (jint)9831);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create UnsignedInt16" << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jUI16)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create UnsignedInt16" << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create UnsignedInt16" << PEGASUS_STD (endl);
   }
   jobject jSI32 = jEnv->NewObject (JMPIjvm::jv.IntegerClassRef,
                                    JMPIjvm::jv.IntegerNewI,
                                    (jint)-45000);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Integer" << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jSI32)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Integer" << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create Integer" << PEGASUS_STD (endl);
   }
   jobject jUI32 = jEnv->NewObject (JMPIjvm::jv.UnsignedInt32ClassRef,
                                    JMPIjvm::jv.UnsignedInt32NewJ,
                                    (jlong)33000);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create UnsignedInt32" << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jUI32)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create UnsignedInt32" << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create UnsignedInt32" << PEGASUS_STD (endl);
   }
   jobject jSI64 = jEnv->NewObject (JMPIjvm::jv.LongClassRef,
                                    JMPIjvm::jv.LongNewJ,
                                    (jlong)-4500000);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Long" << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jSI64)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Long" << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create Long" << PEGASUS_STD (endl);
   }
   // UnsignedInt64NewBi
   jobject jBiStr = jEnv->NewStringUTF ("1234567890");
   jobject jUI64  = jEnv->NewObject (JMPIjvm::jv.UnsignedInt64ClassRef,
                                     JMPIjvm::jv.UnsignedInt64NewStr,
                                     jBiStr);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create UnsignedInt64" << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jUI64)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create UnsignedInt64" << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create UnsignedInt64" << PEGASUS_STD (endl);
   }
   jobject jStr = jEnv->NewStringUTF ("Hello world");
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create String" << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jStr)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create String" << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create String" << PEGASUS_STD (endl);
   }
   CIMDateTime *cdt = new CIMDateTime (String ("20060227183400.000000-360"));
   jobject jDateTime = jEnv->NewObject (JMPIjvm::jv.CIMDateTimeClassRef,
                                        JMPIjvm::jv.CIMDateTimeNewI,
                                        DEBUG_ConvertJavaToC (jint, CIMDateTime *, cdt));
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMDateTime" << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jDateTime)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMDateTime" << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create CIMDateTime" << PEGASUS_STD (endl);
   }
   jobject jVector = jEnv->NewObject (JMPIjvm::jv.VectorClassRef,
                                      JMPIjvm::jv.VectorNew);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Vector" << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jVector)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Vector" << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create Vector" << PEGASUS_STD (endl);
   }
   jobject jBoolean = jEnv->NewObject (JMPIjvm::jv.BooleanClassRef,
                                       JMPIjvm::jv.BooleanNewZ,
                                       (jboolean)true);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Boolean" << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jBoolean)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Boolean" << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create Boolean" << PEGASUS_STD (endl);
   }
   jobject jCharacter = jEnv->NewObject (JMPIjvm::jv.CharacterClassRef,
                                         JMPIjvm::jv.CharacterNewC,
                                         (jchar)'A');
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Character" << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jCharacter)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Character" << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create Character" << PEGASUS_STD (endl);
   }
   jobject jFloat = jEnv->NewObject (JMPIjvm::jv.FloatClassRef,
                                     JMPIjvm::jv.FloatNewF,
                                     (jfloat)3.1415279);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Float" << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jFloat)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Float" << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create Float" << PEGASUS_STD (endl);
   }
   jobject jDouble = jEnv->NewObject (JMPIjvm::jv.DoubleClassRef,
                                      JMPIjvm::jv.DoubleNewD,
                                      (jdouble)3.1415279);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Double" << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jDouble)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Double" << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create Double" << PEGASUS_STD (endl);
   }
   Sint8 si8CVInit = -8;
   CIMParamValue *cpv = new CIMParamValue (String ("bob"),
                                           CIMValue (si8CVInit));
   jobject jCIMArgument = jEnv->NewObject (JMPIjvm::jv.CIMArgumentClassRef,
                                           JMPIjvm::jv.CIMArgumentNewI,
                                           DEBUG_ConvertJavaToC (jint, CIMParamValue *, cpv));
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMArgument" << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jCIMArgument)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMArgument" << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create CIMArgument" << PEGASUS_STD (endl);
   }
   CIMClass *cc = new CIMClass (CIMName ("bob"));
   jobject jCIMClass = jEnv->NewObject (JMPIjvm::jv.CIMClassClassRef,
                                        JMPIjvm::jv.CIMClassNewI,
                                        DEBUG_ConvertJavaToC (jint, CIMClass *, cc));
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMClass" << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jCIMClass)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMClass" << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create CIMClass" << PEGASUS_STD (endl);
   }
   CIMFlavor *cf = new CIMFlavor ();
   jobject jCIMFlavor = jEnv->NewObject (JMPIjvm::jv.CIMFlavorClassRef,
                                         JMPIjvm::jv.CIMFlavorNewI,
                                         DEBUG_ConvertJavaToC (jint, CIMFlavor *, cf));
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMFlavor" << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jCIMFlavor)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMFlavor" << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create CIMFlavor" << PEGASUS_STD (endl);
   }
   CIMInstance *ci = new CIMInstance (CIMName ("bob"));
   jobject jCIMInstance = jEnv->NewObject (JMPIjvm::jv.CIMInstanceClassRef,
                                           JMPIjvm::jv.CIMInstanceNewI,
                                           DEBUG_ConvertJavaToC (jint, CIMInstance *, ci));
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMInstance" << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jCIMInstance)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMInstance" << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create CIMInstance" << PEGASUS_STD (endl);
   }
   CIMClass *cc2 = new CIMClass (CIMName ("bob"));
   CIMObject *co = new CIMObject (*cc2);
   jobject jCIMObject = jEnv->NewObject (JMPIjvm::jv.CIMObjectClassRef,
                                         JMPIjvm::jv.CIMObjectNewIZ,
                                         DEBUG_ConvertJavaToC (jint, CIMObject *, co));
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMObject" << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jCIMObject)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMObject" << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create CIMObject" << PEGASUS_STD (endl);
   }
   CIMObjectPath *cop = new CIMObjectPath (String ("bob"));
   jobject jCIMObjectPath = jEnv->NewObject (JMPIjvm::jv.CIMObjectPathClassRef,
                                             JMPIjvm::jv.CIMObjectPathNewI,
                                             DEBUG_ConvertJavaToC (jint, CIMObjectPath *, cop));
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMObjectPath" << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jCIMObjectPath)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMObjectPath" << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create CIMObjectPath" << PEGASUS_STD (endl);
   }
   Uint32 ui32CVInit = 77;
   CIMProperty *cp = new CIMProperty (CIMName ("bobprop"),
                                      CIMValue (ui32CVInit));
   jobject jCIMProperty = jEnv->NewObject (JMPIjvm::jv.CIMPropertyClassRef,
                                           JMPIjvm::jv.CIMPropertyNewI,
                                           DEBUG_ConvertJavaToC (jint, CIMProperty *, cp));
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMProperty" << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jCIMProperty)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMProperty" << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create CIMProperty" << PEGASUS_STD (endl);
   }
   Uint64 ui64CVInit = 42;
   CIMQualifier *cq = new CIMQualifier (CIMName ("bob"),
                                        CIMValue (ui64CVInit));
   jobject jCIMQualifier = jEnv->NewObject (JMPIjvm::jv.CIMQualifierClassRef,
                                            JMPIjvm::jv.CIMQualifierNewI,
                                            DEBUG_ConvertJavaToC (jint, CIMQualifier *, cq));
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMQualifier" << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jCIMQualifier)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMQualifier" << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create CIMQualifier" << PEGASUS_STD (endl);
   }
   Uint8 ui8CVInit = 0;
   CIMValue *cv = new CIMValue (ui8CVInit);
   jobject jCIMValue = jEnv->NewObject (JMPIjvm::jv.CIMValueClassRef,
                                        JMPIjvm::jv.CIMValueNewI,
                                        DEBUG_ConvertJavaToC (jint, CIMValue *, cv));
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMValue" << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jCIMValue)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMValue" << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create CIMValue" << PEGASUS_STD (endl);
   }
   // OperationContextNewI
   // SelectExpNewI

   // ---------------------------------------------------------------------

   // ---------------------------------------------------------------------

   {
      ConfigManager *manager      = ConfigManager::getInstance ();
      String         pegasusHome  = String::EMPTY;

// BEGIN: Copy from src/Server/cimserver.cpp
//        It would have been nice if this were part of ConfigManager.
#ifdef PEGASUS_OS_OS400

      VFYPTRS_INCDCL;               // VFYPTRS local variables

      // verify pointers
#pragma exception_handler (qsyvp_excp_hndlr,qsyvp_excp_comm_area,0,_C2_MH_ESCAPE)
      for( int arg_index = 1; arg_index < argc; arg_index++ ){
         VFYPTRS(VERIFY_SPP_NULL(argv[arg_index]));
      }
#pragma disable_handler

      // Convert the args to ASCII
      for(Uint32 i = 0;i< argc;++i)
      {
         EtoA(argv[i]);
      }

      // Initialize Pegasus home to the shipped OS/400 directory.
      pegasusHome = OS400_DEFAULT_PEGASUS_HOME;
#endif

#ifndef PEGASUS_OS_TYPE_WINDOWS
      //
      // Get environment variables:
      //
#ifdef PEGASUS_OS_OS400
#pragma convert(37)
      const char* tmp = getenv("PEGASUS_HOME");
#pragma convert(0)
      char home[256] = {0};
      if (tmp && strlen(tmp) < 256)
      {
        strcpy(home, tmp);
        EtoA(home);
        pegasusHome = home;
      }
#else
#if defined(PEGASUS_OS_AIX) && defined(PEGASUS_USE_RELEASE_DIRS)
      pegasusHome = AIX_RELEASE_PEGASUS_HOME;
#elif !defined(PEGASUS_USE_RELEASE_DIRS) || defined(PEGASUS_PLATFORM_ZOS_ZSERIES_IBM)
      const char* tmp = getenv("PEGASUS_HOME");

      if (tmp)
      {
         pegasusHome = tmp;
      }
#endif
#endif

      FileSystem::translateSlashes(pegasusHome);
#else

      // windows only
      //setHome(pegasusHome);
      pegasusHome = _cimServerProcess->getHome();
#endif

      //
      // Set the value for pegasusHome property
      //
      ConfigManager::setPegasusHome(pegasusHome);
// END

      String         path         = ConfigManager::getHomedPath (manager->getCurrentValue ("providerDir"));
      ProviderVector pv           = { 0, 0 };
      String         fileName;
      String         className;
      String         providerName;

      fileName  = path + "/JMPIExpAssociationProvider.jar";
      className = "Associations/JMPIExpAssociationProvider";

      FileSystem::translateSlashes (fileName);

      providerName = fileName + ":" + className;

      if (!FileSystem::exists (fileName))
      {
         PEGASUS_STD(cerr)<<"testJVM: FAILURE: \"" << fileName <<"\" does not exist!"<<PEGASUS_STD(endl);

         return 1;
      }

      pv.jProvider = JMPIjvm::getProvider (jEnv,
                                           fileName,
                                           className,
                                           providerName.getCString (),
                                           &pv.jProviderClass);

      if (  !pv.jProvider
         || !pv.jProviderClass
         )
      {
         PEGASUS_STD (cout) << "testJVM: FAILURE: Create Provider" << PEGASUS_STD (endl);

         return 1;
      }
      else
      {
         PEGASUS_STD (cout) << "testJVM: SUCCESS: Create Provider" << PEGASUS_STD (endl);
      }
   }

   // ---------------------------------------------------------------------

   JMPIjvm::detachThread ();

   PEGASUS_STD(cerr)<<"testJVM: SUCCESS"<<PEGASUS_STD(endl);

   return 0;
}

int
main (int argc, char *argv[])
{
#ifdef PEGASUS_DEBUG
   if (getenv ("PEGASUS_JMPI_TRACE"))
      trace = 1;
   else
      trace = 0;
#else
   trace = 0;
#endif

   printEnvironmentVariables ();

   return testJVM ();
}