/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* specColl.h - header file for specColl.c
 */



#ifndef SPEC_COLL_H
#define SPEC_COLL_H

#include "rods.h"
#include "initServer.h"
#include "objInfo.h"
#include "dataObjInpOut.h"
#include "ruleExecSubmit.h"
#include "rcGlobalExtern.h"
#include "rsGlobalExtern.h"
#include "reIn2p3SysRule.h"

#ifdef  __cplusplus
extern "C" {
#endif

int
modCollInfo2 (rsComm_t *rsComm, specColl_t *specColl, int clearFlag);
int
querySpecColl (rsComm_t *rsComm, char *objPath, genQueryOut_t **genQueryOut);
int
queueSpecCollCache (rsComm_t *rsComm, genQueryOut_t *genQueryOut, char *objPath);
int
queueSpecCollCacheWithObjStat (rodsObjStat_t *rodsObjStatOut);
specCollCache_t *
matchSpecCollCache (char *objPath);
int
getSpecCollCache (rsComm_t *rsComm, char *objPath, int inCachOnly,
specCollCache_t **specCollCache);
int
statPathInSpecColl (rsComm_t *rsComm, char *objPath,
int inCachOnly, rodsObjStat_t **rodsObjStatOut);
int
specCollSubStat (rsComm_t *rsComm, specColl_t *specColl,
char *subPath, specCollPerm_t specCollPerm, dataObjInfo_t **dataObjInfo);
int
resolvePathInSpecColl (rsComm_t *rsComm, char *objPath,
specCollPerm_t specCollPerm, int inCachOnly, dataObjInfo_t **dataObjInfo);
int
resolveLinkedPath (rsComm_t *rsComm, char *objPath,
specCollCache_t **specCollCache, keyValPair_t *condInput);

#ifdef  __cplusplus
}
#endif

#endif	/* SPEC_COLL_H */
