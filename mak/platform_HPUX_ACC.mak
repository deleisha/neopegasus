#//%2006////////////////////////////////////////////////////////////////////////
#//
#// Copyright (c) 2000, 2001, 2002 BMC Software; Hewlett-Packard Development
#// Company, L.P.; IBM Corp.; The Open Group; Tivoli Systems.
#// Copyright (c) 2003 BMC Software; Hewlett-Packard Development Company, L.P.;
#// IBM Corp.; EMC Corporation, The Open Group.
#// Copyright (c) 2004 BMC Software; Hewlett-Packard Development Company, L.P.;
#// IBM Corp.; EMC Corporation; VERITAS Software Corporation; The Open Group.
#// Copyright (c) 2005 Hewlett-Packard Development Company, L.P.; IBM Corp.;
#// EMC Corporation; VERITAS Software Corporation; The Open Group.
#// Copyright (c) 2006 Hewlett-Packard Development Company, L.P.; IBM Corp.;
#// EMC Corporation; Symantec Corporation; The Open Group.
#//
#// Permission is hereby granted, free of charge, to any person obtaining a copy
#// of this software and associated documentation files (the "Software"), to
#// deal in the Software without restriction, including without limitation the
#// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
#// sell copies of the Software, and to permit persons to whom the Software is
#// furnished to do so, subject to the following conditions:
#// 
#// THE ABOVE COPYRIGHT NOTICE AND THIS PERMISSION NOTICE SHALL BE INCLUDED IN
#// ALL COPIES OR SUBSTANTIAL PORTIONS OF THE SOFTWARE. THE SOFTWARE IS PROVIDED
#// "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
#// LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
#// PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
#// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
#// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
#// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#//
#//==============================================================================
MAJOR_VERSION_NUMBER = 1

OS = HPUX

ifdef ACC_COMPILER_COMMAND
   CXX = $(ACC_COMPILER_COMMAND)
else
   CXX = aCC
endif

CC = $(CXX) -Ae

COMPILER = acc

PLATFORM_VERSION_SUPPORTED = yes

SYS_INCLUDES = 

ifdef PEGASUS_CCOVER
 SYS_INCLUDES += -I/opt/ccover11/include
endif

ifdef PEGASUS_PURIFY
 SYS_INCLUDES += -I$(PURIFY_HOME)
endif

#########################################################################
##
## Platform specific compile options controlled by environment variables
## are set here.  
##
#########################################################################

# Enable OOP by default if preference not already set in the environment
#
ifndef PEGASUS_DEFAULT_ENABLE_OOP
PEGASUS_DEFAULT_ENABLE_OOP = true
endif



#########################################################################

DEFINES = -DPEGASUS_PLATFORM_$(PEGASUS_PLATFORM) -DPEGASUS_PLATFORM_HPUX_ACC

DEFINES += -DPEGASUS_USE_SYSLOGS

ifdef PEGASUS_USE_EMANATE
 DEFINES += -DHPUX_EMANATE
endif

ifdef PEGASUS_CCOVER
 DEFINES += -DPEGASUS_CCOVER
endif

ifdef PEGASUS_PURIFY
 DEFINES += -DPEGASUS_PURIFY
endif

##
## The following flags need to be set or unset 
## to compile-in the code required for PAM authentication
## and compile-out the code that uses the password file.
##

ifdef PEGASUS_PAM_AUTHENTICATION
 DEFINES += -DPEGASUS_PAM_AUTHENTICATION -DPEGASUS_NO_PASSWORDFILE
endif

ifdef PEGASUS_HAS_MESSAGES
    ifeq ($(PEGASUS_HAS_ICU),true)
        MSG_COMPILE = ${ICU_INSTALL}/bin/genrb
        MSG_FLAGS =
        MSG_SOURCE_EXT = .txt
        MSG_COMPILE_EXT = .res
        CNV_ROOT_CMD = cnv2rootbundle

        SYS_INCLUDES += -I${ICU_INSTALL}/include
        EXTRA_LIBRARIES += -L${ICU_INSTALL}/lib -licui18n -licuuc
    endif
endif


DEPEND_INCLUDES =


## Flags:
##     +Z - produces position independent code (PIC).
##     +DAportable generates code for any HP9000 architecture
##     -Wl, passes the following option to the linker
##       +s causes the linked image or shared lib to be able to
##          search for any referenced shared libs dynamically in
##          SHLIB_PATH (LD_LIBRARY_PATH on 64-bit HP9000)
##       +b enables dynamic search in the specified directory(ies)
##

FLAGS = 

PEGASUS_SUPPORTS_DYNLIB=yes

ifdef PEGASUS_USE_RELEASE_DIRS
  FLAGS += -Wl,+b$(PEGASUS_DEST_LIB_DIR):/usr/lib
  ifeq ($(PEGASUS_PLATFORM), HPUX_PARISC_ACC)
    FLAGS += -Wl,+cdp,$(PEGASUS_PLATFORM_SDKROOT)/usr/lib:/usr/lib -Wl,+cdp,$(PEGASUS_HOME)/lib:$(PEGASUS_DEST_LIB_DIR)
    ifdef OPENSSL_HOME
      FLAGS += -Wl,+cdp,$(OPENSSL_HOME)/lib:/usr/lib
    endif
    ifdef ICU_INSTALL
      FLAGS += -Wl,+cdp,$(ICU_INSTALL)/lib:$(PEGASUS_DEST_LIB_DIR)
    endif
  endif
else
  ifdef PEGASUS_HAS_MESSAGES
    ifdef ICU_INSTALL
      FLAGS += -Wl,+b$(LIB_DIR):/usr/lib:${ICU_INSTALL}/lib
    endif
  else
    FLAGS += -Wl,+b$(LIB_DIR):/usr/lib
  endif
endif

FLAGS += -Wl,+s

ifdef PEGASUS_USE_DEBUG_BUILD_OPTIONS 
  FLAGS += -g
else
  FLAGS += +O2 -s
endif

ifdef PEGASUS_USE_RELEASE_DIRS
    PEGASUS_DEST_LIB_DIR=/opt/wbem/lib
endif

#
#  For future use on HP-UX
#
ifdef HPUX_LARGE_INTERFACES
        FLAGS += -D_HPUX_API_LEVEL=20040821
endif

SYS_LIBS = -lpthread -lrt

SH = sh

YACC = bison

RM = rm -f

DIFF = diff

SORT = sort

COPY = cp

MOVE = mv

LIB_SUFFIX = .$(MAJOR_VERSION_NUMBER)
