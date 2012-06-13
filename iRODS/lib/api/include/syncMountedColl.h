/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* syncMountedColl.h
 */

#ifndef SYNC_MOUNTED_COLL_H
#define SYNC_MOUNTED_COLL_H

/* This is a Object File I/O API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "dataObjInpOut.h"

#if defined(RODS_SERVER)
#define RS_SYNC_MOUNTED_COLL rsSyncMountedColl
/* prototype for the server handler */
int
rsSyncMountedColl (rsComm_t *rsComm, dataObjInp_t *syncMountedCollInp);
int
_rsSyncMountedColl (rsComm_t *rsComm, specColl_t *specColl, int oprType);
#else
#define RS_SYNC_MOUNTED_COLL NULL
#endif

/* flag for oprType of dataObjInp_t and structFileOprInp_t. */

#define PURGE_STRUCT_FILE_CACHE	0x1 
#define DELETE_STRUCT_FILE	0x2 
#define NO_REG_COLL_INFO	0x4	/* don't register collInfo even if
					 * struct file is changed */ 
#define LOGICAL_BUNDLE		0x8	/* use the filePath associated with
					 * the logical path instead of
					 * the path in cacheDir */  
#define CREATE_TAR_OPR		0x0	/* create tar file -c */
#define ADD_TO_TAR_OPR          0x10	/* add to a tar file */
#define PRESERVE_COLL_PATH      0x20	/* preserver the last entry of coll */
#define PRESERVE_DIR_CONT	0x40	/* preserve the content of cachrdir */



#ifdef  __cplusplus
extern "C" {
#endif

/* prototype for the client call */
int
rcSyncMountedColl (rcComm_t *conn, dataObjInp_t *syncMountedCollInp);

/* rcSyncMountedColl - Sync the mounted structured file with the cache.
 * Input - 
 *   rcComm_t *conn - The client connection handle.
 *   dataObjInp_t *dataObjInp - generic dataObj input. Relevant items are:
 *	objPath - the path of the Mounted collection.
 *      oprType - PURGE_STRUCT_FILE_CACHE - purge the cache after the sync
 *                DELETE_STRUCT_FILE - delete the structured file and
 *		  the cache  
 *
 * OutPut - 
 *   int status - The status of the operation.   
 */

#ifdef  __cplusplus
}
#endif

#endif	/* SYNC_MOUNTED_COLL_H */
