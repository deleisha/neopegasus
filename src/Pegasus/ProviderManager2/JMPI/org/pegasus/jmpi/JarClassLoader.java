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

import java.net.URL;
import java.net.URLClassLoader;
import java.net.JarURLConnection;
import java.io.IOException;
import java.io.IOException;

public class JarClassLoader extends URLClassLoader {
    private URL url;

    public JarClassLoader(URL url) {
      super(new URL[] { url });
      this.url = url;
      System.err.println("--- url: "+url.toString());
    }

    void connect()
       throws IOException
    {
      URL u = new URL("jar", "", url + "!/");
      JarURLConnection uc = (JarURLConnection)u.openConnection();
      System.err.println("--- u: "+u.toString());
    }

    public Class loadClass(String name)
      throws ClassNotFoundException
    {
      Class c = loadClass(name);
      return c;
    }

    static public Class load(String jar, String cls)
       throws ClassNotFoundException, IOException
    {
       JarClassLoader cl=new JarClassLoader(new URL("File:"+jar));
       cl.connect();
       System.err.println("--- finding ...");
//       Class c=cl.findClass(cls);
//       System.err.println("--- Loading ...");
       Class c=cl.loadClass(cls);
       System.err.println("--- Loading done: "+cls.toString());
       return c;
    }

}
