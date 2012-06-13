/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code.  */ 
/* See filePut.h for a description of this API call.*/

#include "filePut.h"

int
rcFilePut (rcComm_t *conn, fileOpenInp_t *filePutInp, 
bytesBuf_t *filePutInpBBuf)
{
    int status;

#if defined(osx_platform)
    if (filePutInp->flags & O_TRUNC) {
        filePutInp->flags = filePutInp->flags ^ O_TRUNC;
        filePutInp->flags = filePutInp->flags | 0x200;
    }
#endif
    status = procApiRequest (conn, FILE_PUT_AN, filePutInp, filePutInpBBuf, 
        (void **) NULL, NULL);

    return (status);
}
