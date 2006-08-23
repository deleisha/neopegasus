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
//%/////////////////////////////////////////////////////////////////////////////
package Client;

import org.pegasus.jmpi.CIMClient;
import org.pegasus.jmpi.CIMException;
import org.pegasus.jmpi.UnsignedInt32;

public class testUnsignedInt32
{
   private boolean DEBUG     = false;
   private long    MIN_VALUE = 0L;
   private long    MAX_VALUE = 4294967295L;

   /**
    * This returns the group name.
    *
    * @return String "class" testcase belongs in.
    */
   public String getGroup ()
   {
      return "UnsignedInt32";
   }

   public void setDebug (boolean fDebug)
   {
      DEBUG = fDebug;
   }

   public boolean main (String args[], CIMClient cimClient)
   {
      boolean fExecuted = false;
      boolean fRet      = true;

      for (int i = 0; i < args.length; i++)
      {
          if (args[i].equalsIgnoreCase ("debug"))
          {
              setDebug (true);
          }
      }

      if (!fExecuted)
      {
         fRet = testUnsignedInt32 (cimClient);
      }

      return fRet;
   }

   private boolean testUnsignedInt32 (CIMClient client)
   {
      if (client == null)
      {
         System.out.println ("FAILURE: testUnsignedInt32: client == null");

         return false;
      }

      // -----

      UnsignedInt32 uint32 = null;

      uint32 = new UnsignedInt32 (0);

      if (uint32 == null)
      {
         System.out.println ("FAILURE: testUnsignedInt32: uint32 == null (1)");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testUnsignedInt32: uint32 = " + uint32);
      }

      // -----

      try
      {
         uint32 = new UnsignedInt32 (-1);

         System.out.println ("FAILURE: testUnsignedInt32: uint32 != null (2)");

         return false;
      }
      catch (NumberFormatException e)
      {
      }
      catch (Exception e)
      {
         System.out.println ("FAILURE: testUnsignedInt32: caught " + e + " (2)");

         return false;
      }

      // -----

      uint32 = new UnsignedInt32 (MIN_VALUE);

      if (uint32 == null)
      {
         System.out.println ("FAILURE: testUnsignedInt32: uint32 == null (3)");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testUnsignedInt32: uint32 = " + uint32);
      }

      // -----

      try
      {
         uint32 = new UnsignedInt32 ((MIN_VALUE - 1));

         System.out.println ("FAILURE: testUnsignedInt32: uint32 != null (4)");

         return false;
      }
      catch (NumberFormatException e)
      {
      }
      catch (Exception e)
      {
         System.out.println ("FAILURE: testUnsignedInt32: caught " + e + " (4)");

         return false;
      }

      // -----

      uint32 = new UnsignedInt32 (MAX_VALUE);

      if (uint32 == null)
      {
         System.out.println ("FAILURE: testUnsignedInt32: uint32 == null (5)");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testUnsignedInt32: uint32 = " + uint32);
      }

      // -----

      try
      {
         uint32 = new UnsignedInt32 ((MAX_VALUE + 1));

         System.out.println ("FAILURE: testUnsignedInt32: uint32 != null (6)");

         return false;
      }
      catch (NumberFormatException e)
      {
      }
      catch (Exception e)
      {
         System.out.println ("FAILURE: testUnsignedInt32: caught " + e + " (6)");

         return false;
      }

      // -----

      uint32 = new UnsignedInt32 ("0");

      if (uint32 == null)
      {
         System.out.println ("FAILURE: testUnsignedInt32: uint32 == null (7)");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testUnsignedInt32: uint32 = " + uint32);
      }

      // -----

      try
      {
         uint32 = new UnsignedInt32 ("-1");

         System.out.println ("FAILURE: testUnsignedInt32: uint32 != null (8)");

         return false;
      }
      catch (NumberFormatException e)
      {
      }
      catch (Exception e)
      {
         System.out.println ("FAILURE: testUnsignedInt32: caught " + e + " (8)");

         return false;
      }

      // -----

      uint32 = new UnsignedInt32 (new Long (MAX_VALUE).toString ());

      if (uint32 == null)
      {
         System.out.println ("FAILURE: testUnsignedInt32: uint32 == null (9)");

         return false;
      }

      if (DEBUG)
      {
         System.out.println ("testUnsignedInt32: uint32 = " + uint32);
      }

      // -----

      try
      {
         uint32 = new UnsignedInt32 (new Long ((MAX_VALUE + 1)).toString ());

         System.out.println ("FAILURE: testUnsignedInt32: uint32 != null (10)");

         return false;
      }
      catch (NumberFormatException e)
      {
      }
      catch (Exception e)
      {
         System.out.println ("FAILURE: testUnsignedInt32: caught " + e + " (10)");

         return false;
      }

      // -----

      System.out.println ("SUCCESS: testUnsignedInt32");

      return true;
   }
}
