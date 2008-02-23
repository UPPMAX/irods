
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdf.ncsa.uiuc.edu/HDF5/doc/Copyright.html.  If you do not have     *
 * access to either file, you may request a copy from hdfhelp@ncsa.uiuc.edu. *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef _H5DATASPACE_H
#define _H5DATASPACE_H

/* H5Spublic.h:#define H5S_MAX_RANK    32 */
#define H5DATASPACE_MAX_RANK 32

/* Define basic structure for Dataspace object */

typedef struct H5Dataspace
{
    int rank;
    unsigned int  dims[H5DATASPACE_MAX_RANK]; 
    unsigned int npoints;

    /* fields for hyperslab selection */
    unsigned int  start[H5DATASPACE_MAX_RANK];
    unsigned int  stride[H5DATASPACE_MAX_RANK];
    unsigned int  count[H5DATASPACE_MAX_RANK];
} H5Dataspace;

#endif

