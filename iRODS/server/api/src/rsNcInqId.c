/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */
/* See dataObjGet.h for a description of this API call.*/

#include "ncInqId.h"
#include "rodsLog.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"
#include "rsApiHandler.h"
#include "objMetaOpr.h"
#include "physPath.h"
#include "specColl.h"
#include "getRemoteZoneResc.h"

int
rsNcInqId (rsComm_t *rsComm, ncInqIdInp_t *ncInqIdInp, int **outId)
{
    int remoteFlag;
    rodsServerHost_t *rodsServerHost = NULL;
    int l1descInx;
    ncInqIdInp_t myNcInqIdInp;
    int status = 0;
    int myoutId = 0;

    if (getValByKey (&ncInqIdInp->condInput, NATIVE_NETCDF_CALL_KW) != NULL) {
	/* just do nc_inq_YYYYid */
	status = _rsNcInqId (ncInqIdInp->paramType, ncInqIdInp->ncid, 
	  ncInqIdInp->name, &myoutId);
        if (status == NC_NOERR) {
            *outId = (int *) malloc (sizeof (int));
            *(*outId) = myoutId;
            return 0;
        } else {
            return status;
        }
    }
    l1descInx = ncInqIdInp->ncid;
    if (l1descInx < 2 || l1descInx >= NUM_L1_DESC) {
        rodsLog (LOG_ERROR,
          "rsNcInqId: l1descInx %d out of range",
          l1descInx);
        return (SYS_FILE_DESC_OUT_OF_RANGE);
    }
    if (L1desc[l1descInx].inuseFlag != FD_INUSE) return BAD_INPUT_DESC_INDEX;
    if (L1desc[l1descInx].remoteZoneHost != NULL) {
	bzero (&myNcInqIdInp, sizeof (myNcInqIdInp));
        myNcInqIdInp.paramType = ncInqIdInp->paramType;
	myNcInqIdInp.ncid = L1desc[l1descInx].remoteL1descInx;
        rstrcpy (myNcInqIdInp.name, ncInqIdInp->name, NAME_LEN);

        /* cross zone operation */
	status = rcNcInqId (L1desc[l1descInx].remoteZoneHost->conn,
	  &myNcInqIdInp, &myoutId);
    } else {
        remoteFlag = resoAndConnHostByDataObjInfo (rsComm,
	  L1desc[l1descInx].dataObjInfo, &rodsServerHost);
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else if (remoteFlag == LOCAL_HOST) {
	    status = _rsNcInqId (ncInqIdInp->paramType, 
	      L1desc[l1descInx].l3descInx, ncInqIdInp->name, &myoutId);
            if (status < 0) {
                return status;
            }
        } else {
	    /* execute it remotely */
	    bzero (&myNcInqIdInp, sizeof (myNcInqIdInp));
	    myNcInqIdInp.paramType = ncInqIdInp->paramType;
	    myNcInqIdInp.ncid = L1desc[l1descInx].l3descInx;
	    rstrcpy (myNcInqIdInp.name, ncInqIdInp->name, NAME_LEN);
	    addKeyVal (&myNcInqIdInp.condInput, NATIVE_NETCDF_CALL_KW, "");
            status = rcNcInqId (rodsServerHost->conn, &myNcInqIdInp, &myoutId);
	    clearKeyVal (&myNcInqIdInp.condInput);
            if (status < 0) {
                rodsLog (LOG_ERROR,
                  "rsNcInqId: rcNcInqId %d for %s error, status = %d",
                  L1desc[l1descInx].l3descInx,
                  L1desc[l1descInx].dataObjInfo->objPath, status);
                return (status);
            }
	}
    }
    *outId = (int *) malloc (sizeof (int));
    *(*outId) = myoutId;
    return status;
}

int
_rsNcInqId (int paramType, int ncid, char *name, int *outId)
{
    int status;

    switch (paramType) {
      case NC_VAR_T:
	status = nc_inq_varid (ncid, name, outId);
	break;
      case NC_DIM_T:
        status = nc_inq_dimid (ncid, name, outId);
      default:
        rodsLog (LOG_ERROR,
          "_rsNcInqId: Unknow paramType %d for %s ", paramType, name);
        return (NETCDF_INVALID_PARAM_TYPE);
    }

    if (status != NC_NOERR) {
        rodsLog (LOG_ERROR,
          "_rsNcInqId: nc_inq error paramType %d for %s. %s ", 
	  paramType, name, nc_strerror(status));
        status = NETCDF_INQ_ID_ERR - status;
    }
    return status;
}
