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
//  GeneralRandomAccessFile.java  -  edu.sdsc.grid.io.GeneralRandomAccessFile
//
//  CLASS HIERARCHY
//  java.lang.Object
//      |
//      +-java.io.OuputStream
//          |
//          +-edu.sdsc.grid.io.GeneralFileOutputStream
//              |
//              +-edu.sdsc.grid.io.RemoteFileOutputStream
//
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io;



import java.io.*;


/**
 * A RemoteFileOutputStream writes bytes to a file in a file system.
 * What files are available depends on the host environment.
 *<P>
 * RemoteFileOutputStream is meant for writing streams of raw bytes such
 * as image data.
 *<P>
 * @author  Lucas Gilbert
 * @since   JARGON1.4
 */
public abstract class RemoteFileOutputStream extends GeneralFileOutputStream
{
//----------------------------------------------------------------------
//  Constants
//----------------------------------------------------------------------




//----------------------------------------------------------------------
//  Fields
//----------------------------------------------------------------------



//----------------------------------------------------------------------
//  Constructors and Destructors
//----------------------------------------------------------------------
  /**
   * Creates a <code>FileOuputStream</code> by
   * opening a connection to an actual file,
   * the file named by the path name <code>name</code>
   * in the file system.
   * <p>
   * First, the security is checked to verify the file can be written.
   * <p>
   * If the named file does not exist, is a directory rather than a regular
   * file, or for some other reason cannot be opened for reading then a
   * <code>IOException</code> is thrown.
   *
   * @param  name   the system-dependent file name.
   * @exception  IOException  if the file does not exist,
   *                   is a directory rather than a regular file,
   *                   or for some other reason cannot be opened for
   *                   reading.
   */
  public RemoteFileOutputStream( GeneralFileSystem fileSystem, String name )
    throws IOException
  {
    super(fileSystem, name);
  }

  /**
   * Creates a <code>FileInputStream</code> by
   * opening a connection to an actual file,
   * the file named by the <code>File</code>
   * object <code>file</code> in the file system.
   * A new <code>FileDescriptor</code> object
   * is created to represent this file connection.
   * <p>
   * First, the security is checked to verify the file can be written.
   * <p>
   * If the named file does not exist, is a directory rather than a regular
   * file, or for some other reason cannot be opened for reading then a
   * <code>IOException</code> is thrown.
   *
   * @param  file   the file to be opened for reading.
   * @exception  IOException  if the file does not exist,
   *                   is a directory rather than a regular file,
   *                   or for some other reason cannot be opened for
   *                   reading.
   * @see        java.io.File#getPath()
   */
  public RemoteFileOutputStream( GeneralFile file )
    throws IOException
  {
    super(file);
  }






//----------------------------------------------------------------------
// Methods
//----------------------------------------------------------------------

}

