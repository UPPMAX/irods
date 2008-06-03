/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* collRepl.h - recursively replicate a collection
 */

#ifndef COLL_REPL_H
#define COLL_REPL_H

/* This is a Object File I/O API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "dataObjWrite.h"
#include "dataObjClose.h"
#include "dataCopy.h"

#if defined(RODS_SERVER)
#define RS_COLL_REPL rsCollRepl
/* prototype for the server handler */
int
rsCollRepl (rsComm_t *rsComm, dataObjInp_t *collReplInp, 
transStat_t **transStat);
#else
#define RS_COLL_REPL NULL
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/* prototype for the client call */
int
rcCollRepl (rcComm_t *conn, dataObjInp_t *collReplInp);

#ifdef  __cplusplus
}
#endif

#endif	/* COLL_REPL_H */
