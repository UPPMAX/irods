/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* ncInq.h
 */

#ifndef NC_INQ_H
#define NC_INQ_H

/* This is a NETCDF API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "dataObjInpOut.h"
#include "ncOpen.h"
#include "ncInqId.h"
#include "ncGetVarsByType.h"

typedef struct {
    rodsLong_t arrayLen;
    int id;
    int myint;	/* not used */
    char name[LONG_NAME_LEN];
} ncGenDimOut_t;

#define NcGenDimOut_PI "double arrayLen; int id; int myint; str name[LONG_NAME_LEN];"

typedef struct {
    int dataType;
    int id;
    int length;
    int myint;  /* not used */
    char name[LONG_NAME_LEN];
    ncGetVarOut_t value;
} ncGenAttOut_t;

#define NcGenAttOut_PI "int dataType; int id; int length;  int myint; str name[LONG_NAME_LEN]; struct NcGetVarOut_PI;"

typedef struct {
    int natts;
    int dataType;
    int id;
    int nvdims; 
    char name[LONG_NAME_LEN];
    ncGenAttOut_t *att;		/* array of natts length */
    int *dimId;			/* arrays of dim id */
} ncGenVarOut_t;

#define NcGenVarOut_PI "int natts; int dataType; int id; int nvdims; str name[LONG_NAME_LEN]; struct *NcGenAttOut_PI(natts); int *dimId(nvdims);"

typedef struct {
    int ndims;
    int nvars;
    int ngatts;	/* number of global gttr */
    int unlimdimid;
    int format;		/* one of NC_FORMAT_CLASSIC, NC_FORMAT_64BIT, 
                         * NC_FORMAT_NETCDF4, NC_FORMAT_NETCDF4_CLASSIC */
    int myint;  /* not used */
    ncGenDimOut_t *dim;		/* array of ndims length */
    ncGenVarOut_t *var;		/* array of ngvars length */
    ncGenAttOut_t *gatt;	/* array of ngatts length */
} ncInqOut_t;

#define NcInqOut_PI "int ndims; int nvars; int ngatts; int unlimdimid; int format; int myint; struct *NcGenDimOut_PI(ndims); struct *NcGenVarOut_PI(nvars); struct *NcGenAttOut_PI(ngatts);"

#if defined(RODS_SERVER)
#define RS_NC_INQ rsNcInq
/* prototype for the server handler */
int
rsNcInq (rsComm_t *rsComm, ncInqIdInp_t *ncInqInp, ncInqOut_t **ncInqOut);
int
_rsNcInq (rsComm_t *rsComm, int ncid, ncInqOut_t **ncInqOut);
int
inqAtt (int ncid, int varid, int natt, ncGenAttOut_t **attOut);
int
getAttValue (int ncid, int varid, char *name, int dataType, int length,
ncGetVarOut_t *value);
#else
#define RS_NC_INQ NULL
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/* rcNcInq - general netcdf inq for id (equivalent to nc_inq + nc_inq_format
 * Input - 
 *   rcComm_t *conn - The client connection handle.
 *   ncInqIdInp_t struct:
 *     ncid - the the ncid.   
 * OutPut - ncInqOut_t.
 */
/* prototype for the client call */
int
rcNcInq (rcComm_t *conn, ncInqIdInp_t *ncInqInp, ncInqOut_t **ncInqOut);

int
initNcInqOut (int ndims, int nvars, int ngatts, int unlimdimid, int format,
ncInqOut_t **ncInqOut);
int
freeNcInqOut (ncInqOut_t **ncInqOut);
int
dumpNcInqOut (rcComm_t *conn, char *fileName, int ncid, int dumpVarFlag,
ncInqOut_t *ncInqOut);
int
getNcTypeStr (int dataType, char *outString);
int
ncValueToStr (int dataType, void **value, char *outString);
#ifdef  __cplusplus
}
#endif

#endif	/* NC_INQ_H */
