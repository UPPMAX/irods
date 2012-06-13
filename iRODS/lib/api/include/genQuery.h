/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* genQuery.h
   General Query
 */

#ifndef GENERAL_QUERY_H
#define GENERAL_QUERY_H

/* This is a high level file type API call */

#include "rods.h"
#if 0
#include "rcMisc.h"
#endif
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "icatDefines.h"

#include "rodsGenQuery.h"  /* for input/output structs, etc */

#if defined(RODS_SERVER)
#define RS_GEN_QUERY rsGenQuery
/* prototype for the server handler */
int
rsGenQuery (rsComm_t *rsComm, genQueryInp_t *genQueryInp, 
genQueryOut_t **genQueryOut);
int
_rsGenQuery (rsComm_t *rsComm, genQueryInp_t *genQueryInp,
genQueryOut_t **genQueryOut);
#else
#define RS_GEN_QUERY NULL
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/* prototype for the client call */
int
rcGenQuery (rcComm_t *conn, genQueryInp_t *genQueryInp, 
genQueryOut_t **genQueryOut);

#ifdef  __cplusplus
}
#endif

#endif	/* GEN_QUERY_H */
