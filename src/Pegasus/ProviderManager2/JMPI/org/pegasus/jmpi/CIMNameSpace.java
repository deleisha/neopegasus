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
// Author:      Adrian Schuur, schuur@de.ibm.com 
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

package org.pegasus.jmpi;


public class CIMNameSpace {
   int cInst;
    
   private native int _new();
   private native int _newHn(String hn);
   private native int _newHnNs(String hn, String ns);
   private native String _getNameSpace(int inst);
   private native String _getHost(int inst);
   private native void   _setNameSpace(int inst, String ns);
   private native void   _setHost(int inst, String h);
   private native void _finalize(int cns);
    
   public CIMNameSpace() {
      cInst=_new();
   }

   protected void finalize() {
      //   _finalize(cInst);
   }

   public CIMNameSpace(String host) {
      cInst=_newHn(host);
   }
   
   public CIMNameSpace(String host,String ns) {
      cInst=_newHnNs(host,ns);
   }
   
   public String getNameSpace() {
      return _getNameSpace(cInst);
   }
   
   public String getHost(){
      return _getHost(cInst);
   }
   
   public void setNameSpace(String ns) {
      _setNameSpace(cInst,ns);
   }
   
   public void setHost(String host) {
      _setHost(cInst,host);
   }
    
   static {
      System.loadLibrary("JMPIProviderManager");
   }
}
