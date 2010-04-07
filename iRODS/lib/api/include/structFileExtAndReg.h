/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* structFileExtAndReg.h - This file may be generated by a program or 
 * script
 */

#ifndef STRUCT_FILE_EXT_AND_REG_H
#define STRUCT_FILE_EXT_AND_REG_H

/* This is a Object File I/O call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"

typedef struct StructFileExtAndRegInp {
    char objPath[MAX_NAME_LEN];		/* the obj path of the struct file */
    char collection[MAX_NAME_LEN];	/* the collection under which the
					 * extracted files are registered.
					 */
    int oprType;			/* not used */
    int flags;				/* not used */
    keyValPair_t condInput;   /* include chksum flag and value */
} structFileExtAndRegInp_t;

#define MAX_NUM_BULK_OPR_FILES	50

typedef struct RenamedPhyFiles {
    int count;
    char objPath[MAX_NUM_BULK_OPR_FILES][MAX_NAME_LEN];
    char origFilePath[MAX_NUM_BULK_OPR_FILES][MAX_NAME_LEN];
    char newFilePath[MAX_NUM_BULK_OPR_FILES][MAX_NAME_LEN];
} renamedPhyFiles_t;

#define StructFileExtAndRegInp_PI "str objPath[MAX_NAME_LEN]; str collection[MAX_NAME_LEN]; int oprType; int flags; struct KeyValPair_PI;"

#if defined(RODS_SERVER)
#define RS_STRUCT_FILE_EXT_AND_REG rsStructFileExtAndReg
/* prototype for the server handler */
int
rsStructFileExtAndReg (rsComm_t *rsComm, 
structFileExtAndRegInp_t *structFileExtAndRegInp);
int 
chkCollForExtAndReg (rsComm_t *rsComm, char *collection);
int
regUnbunSubfiles (rsComm_t *rsComm, rescInfo_t *rescInfo, char *collection,
char *phyBunDir, int flags);
int
regSubfile (rsComm_t *rsComm, rescInfo_t *rescInfo, char *subObjPath,
char *subfilePath, rodsLong_t dataSize, int flags);
int
bulkAddSubfile (rsComm_t *rsComm, rescInfo_t *rescInfo, char *subObjPath,
char *subfilePath, rodsLong_t dataSize, int dataMode,
genQueryOut_t *bulkDataObjRegInp, renamedPhyFiles_t *renamedPhyFiles);
int
bulkRegSubfile (rsComm_t *rsComm, char *rescName, char *subObjPath,
char *subfilePath, rodsLong_t dataSize, int dataMode, int modFlag,
genQueryOut_t *bulkDataObjRegInp, renamedPhyFiles_t *renamedPhyFiles);
int
addRenamedPhyFile (char *subObjPath, char *oldFileName, char *newFileName, 
renamedPhyFiles_t *renamedPhyFiles);
#else
#define RS_STRUCT_FILE_EXT_AND_REG NULL
#endif

/* prototype for the client call */
int
rcStructFileExtAndReg (rcComm_t *conn, 
structFileExtAndRegInp_t *structFileExtAndRegInp);

#endif	/* STRUCT_FILE_EXT_AND_REG_H */
