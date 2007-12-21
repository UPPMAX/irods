/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to subStructFiles in the COPYRIGHT directory ***/
/* subStructFileRmdir.h  
 */

#ifndef SUB_STRUCT_FILE_RMDIR_H
#define SUB_STRUCT_FILE_RMDIR_H

/* This is Object File I/O type API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "structFileDriver.h"

#if defined(RODS_SERVER)
#define RS_SUB_STRUCT_FILE_RMDIR rsSubStructFileRmdir
/* prototype for the server handler */
int
rsSubStructFileRmdir (rsComm_t *rsComm, subFile_t *subFile);
int
_rsSubStructFileRmdir (rsComm_t *rsComm, subFile_t *subFile);
int
remoteSubStructFileRmdir (rsComm_t *rsComm, subFile_t *subFile,
rodsServerHost_t *rodsServerHost);
#else
#define RS_SUB_STRUCT_FILE_RMDIR NULL
#endif

/* prototype for the client call */
int
rcSubStructFileRmdir (rcComm_t *conn, subFile_t *subFile);

#endif	/* SUB_STRUCT_FILE_RMDIR_H */
