//  Copyright (c) 2005, Regents of the University of California
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//    * Redistributions of source code must retain the above copyright notice,
//  this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//  notice, this list of conditions and the following disclaimer in the
//  documentation and/or other materials provided with the distribution.
//    * Neither the name of the University of California, San Diego (UCSD) nor
//  the names of its contributors may be used to endorse or promote products
//  derived from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//
//  FILE
//  ProtocolCatalog.java  -  edu.sdsc.grid.io.ProtocolCatalog
//
//  CLASS HIERARCHY
//  java.lang.Object
//      |
//      +-.ProtocolCatalog
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io;

import edu.sdsc.grid.io.srb.*;

import java.util.Vector;


/**
 * The ProtocolCatalog stores all the metadata query protocols available
 * for querying.
 *
 * @author  Lucas Gilbert, San Diego Supercomputer Center
 */
public class ProtocolCatalog
{
//----------------------------------------------------------------------
//  Constants
//----------------------------------------------------------------------


//----------------------------------------------------------------------
//  Fields
//----------------------------------------------------------------------
  private static Vector protocols = new Vector();

//TODO yeah, not much here. It only exists because some tricky stuff
//was needed to get around certain 'static' problems.
  ProtocolCatalog()
  {
    super();
  }


  public static void add( Protocol protocol ) {
    protocols.add( protocol );
  }

  /**
   * Check if the protocol was already loaded
   */
  public static boolean has( Protocol protocol ) {
    for (int i=protocols.size()-1; i>=0; i--) {
      if (protocols.get(i).equals( protocol )) {
        return true;
      }
    }
    return false;
  }
}
