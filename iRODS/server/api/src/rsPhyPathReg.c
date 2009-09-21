/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */ 
/* See phyPathReg.h for a description of this API call.*/

#include "phyPathReg.h"
#include "rodsLog.h"
#include "icatDefines.h"
#include "objMetaOpr.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"
#include "reGlobalsExtern.h"
#include "miscServerFunct.h"
#include "apiHeaderAll.h"

/* phyPathRegNoChkPerm - Wrapper internal function to allow phyPathReg with 
 * no checking for path Perm.
 */
int
phyPathRegNoChkPerm (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp)
{
    int status;

    addKeyVal (&phyPathRegInp->condInput, NO_CHK_FILE_PERM_KW, "");

    status = irsPhyPathReg (rsComm, phyPathRegInp);
    return (status);
}

int
rsPhyPathReg (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp)
{
    int status;

    if (getValByKey (&phyPathRegInp->condInput, NO_CHK_FILE_PERM_KW) != NULL &&
      rsComm->proxyUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
	return SYS_NO_API_PRIV;
    }

    status = irsPhyPathReg (rsComm, phyPathRegInp);
    return (status);
}

int
irsPhyPathReg (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp)
{
    int status;
    rescGrpInfo_t *rescGrpInfo = NULL;
    rodsServerHost_t *rodsServerHost = NULL;
    int remoteFlag;
    int rescCnt;
    rodsHostAddr_t addr;
    char *tmpStr = NULL;

    if ((tmpStr = getValByKey (&phyPathRegInp->condInput,
      COLLECTION_TYPE_KW)) != NULL && strcmp (tmpStr, UNMOUNT_STR) == 0) {
        status = unmountFileDir (rsComm, phyPathRegInp);
        return (status);
    } else if (tmpStr != NULL && (strcmp (tmpStr, HAAW_STRUCT_FILE_STR) == 0 ||
      strcmp (tmpStr, TAR_STRUCT_FILE_STR) == 0)) {
	status = structFileReg (rsComm, phyPathRegInp);
        return (status);
    }

    status = getRescInfo (rsComm, NULL, &phyPathRegInp->condInput,
      &rescGrpInfo);

    if (status < 0) {
	rodsLog (LOG_ERROR,
	  "rsPhyPathReg: getRescInfo error for %s, status = %d",
	  phyPathRegInp->objPath, status);
	return (status);
    }

    rescCnt = getRescCnt (rescGrpInfo);

    if (rescCnt != 1) {
        rodsLog (LOG_ERROR,
          "rsPhyPathReg: The input resource is not unique for %s",
          phyPathRegInp->objPath);
        return (SYS_INVALID_RESC_TYPE);
    }

    memset (&addr, 0, sizeof (addr));
    
    rstrcpy (addr.hostAddr, rescGrpInfo->rescInfo->rescLoc, LONG_NAME_LEN);
    remoteFlag = resolveHost (&addr, &rodsServerHost);

    if (remoteFlag == LOCAL_HOST) {
        status = _rsPhyPathReg (rsComm, phyPathRegInp, rescGrpInfo, 
	  rodsServerHost);
    } else if (remoteFlag == REMOTE_HOST) {
        status = remotePhyPathReg (rsComm, phyPathRegInp, rodsServerHost);
    } else {
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else {
            rodsLog (LOG_ERROR,
              "rsPhyPathReg: resolveHost returned unrecognized value %d",
               remoteFlag);
            return (SYS_UNRECOGNIZED_REMOTE_FLAG);
        }
    }

    return (status);
}

int
remotePhyPathReg (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp, 
rodsServerHost_t *rodsServerHost)
{
   int status;

   if (rodsServerHost == NULL) {
        rodsLog (LOG_ERROR,
          "remotePhyPathReg: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    status = rcPhyPathReg (rodsServerHost->conn, phyPathRegInp);

    if (status < 0) {
        rodsLog (LOG_ERROR,
         "remotePhyPathReg: rcPhyPathReg failed for %s",
          phyPathRegInp->objPath);
    }

    return (status);
}

int
_rsPhyPathReg (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp, 
rescGrpInfo_t *rescGrpInfo, rodsServerHost_t *rodsServerHost)
{
    int status;
    fileOpenInp_t chkNVPathPermInp;
    int rescTypeInx;
    char *tmpFilePath;
    char filePath[MAX_NAME_LEN];
    dataObjInfo_t dataObjInfo;
    char *tmpStr = NULL;

    if ((tmpFilePath = getValByKey (&phyPathRegInp->condInput, FILE_PATH_KW))
      == NULL) {
        rodsLog (LOG_ERROR,
	  "_rsPhyPathReg: No filePath input for %s",
	  phyPathRegInp->objPath);
	return (SYS_INVALID_FILE_PATH);
    } else {
	/* have to do this since it will be over written later */
	rstrcpy (filePath, tmpFilePath, MAX_NAME_LEN);
    }

    /* check if we need to chk permission */

    memset (&dataObjInfo, 0, sizeof (dataObjInfo));
    rstrcpy (dataObjInfo.objPath, phyPathRegInp->objPath, MAX_NAME_LEN);
    rstrcpy (dataObjInfo.filePath, filePath, MAX_NAME_LEN);
    dataObjInfo.rescInfo = rescGrpInfo->rescInfo;
    rstrcpy (dataObjInfo.rescName, rescGrpInfo->rescInfo->rescName, 
      LONG_NAME_LEN);

 
    if (getValByKey (&phyPathRegInp->condInput, NO_CHK_FILE_PERM_KW) == NULL &&
      getchkPathPerm (rsComm, phyPathRegInp, &dataObjInfo)) { 
        memset (&chkNVPathPermInp, 0, sizeof (chkNVPathPermInp));

        rescTypeInx = rescGrpInfo->rescInfo->rescTypeInx;
        rstrcpy (chkNVPathPermInp.fileName, filePath, MAX_NAME_LEN);
        chkNVPathPermInp.fileType = RescTypeDef[rescTypeInx].driverType;
        rstrcpy (chkNVPathPermInp.addr.hostAddr,  
	  rescGrpInfo->rescInfo->rescLoc, NAME_LEN);

        status = chkFilePathPerm (rsComm, &chkNVPathPermInp, rodsServerHost);
    
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "_rsPhyPathReg: chkFilePathPerm error for %s",
              phyPathRegInp->objPath);
            return (SYS_INVALID_FILE_PATH);
        }
    } else {
	status = 0;
    }

    if (getValByKey (&phyPathRegInp->condInput, COLLECTION_KW) != NULL) {
	status = dirPathReg (rsComm, phyPathRegInp, filePath, 
	  rescGrpInfo->rescInfo); 
    } else if ((tmpStr = getValByKey (&phyPathRegInp->condInput, 
      COLLECTION_TYPE_KW)) != NULL && strcmp (tmpStr, MOUNT_POINT_STR) == 0) {
        status = mountFileDir (rsComm, phyPathRegInp, filePath,
          rescGrpInfo->rescInfo);
    } else {
	status = filePathReg (rsComm, phyPathRegInp, filePath,
	  rescGrpInfo->rescInfo); 
    }

    return (status);
}

int
filePathReg (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp, char *filePath, 
rescInfo_t *rescInfo)
{
    dataObjInfo_t dataObjInfo;
    int status;

    initDataObjInfoWithInp (&dataObjInfo, phyPathRegInp);
    dataObjInfo.replStatus = NEWLY_CREATED_COPY;
    dataObjInfo.rescInfo = rescInfo;
    rstrcpy (dataObjInfo.rescName, rescInfo->rescName, NAME_LEN);

    if (dataObjInfo.dataSize <= 0 && 
      (dataObjInfo.dataSize = getSizeInVault (rsComm, &dataObjInfo)) < 0) {
	status = (int) dataObjInfo.dataSize;
        rodsLog (LOG_ERROR,
         "filePathReg: getSizeInVault for %s failed, status = %d",
          dataObjInfo.objPath, status);
	return (status);
    }

    status = svrRegDataObj (rsComm, &dataObjInfo);
    if (status < 0) {
        rodsLog (LOG_ERROR,
         "filePathReg: rsRegDataObj for %s failed, status = %d",
          dataObjInfo.objPath, status);
    } else {
        ruleExecInfo_t rei;
        initReiWithDataObjInp (&rei, rsComm, phyPathRegInp);
	rei.doi = &dataObjInfo;
	rei.status = status;
        rei.status = applyRule ("acPostProcForFilePathReg", NULL, &rei,
          NO_SAVE_REI);
    }

    return (status);
}

int
dirPathReg (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp, char *filePath,
rescInfo_t *rescInfo)
{
    collInp_t collCreateInp;
    fileOpendirInp_t fileOpendirInp;
    fileClosedirInp_t fileClosedirInp;
    int rescTypeInx;
    int status;
    int dirFd;
    dataObjInp_t subPhyPathRegInp;
    fileReaddirInp_t fileReaddirInp;
    rodsDirent_t *rodsDirent = NULL;
    rodsObjStat_t *rodsObjStatOut = NULL;

    status = collStat (rsComm, phyPathRegInp, &rodsObjStatOut);
    if (status < 0) {
        memset (&collCreateInp, 0, sizeof (collCreateInp));
        rstrcpy (collCreateInp.collName, phyPathRegInp->objPath, 
	  MAX_NAME_LEN);
        /* create the coll just in case it does not exist */
        status = rsCollCreate (rsComm, &collCreateInp);
	if (status < 0) return status;
    } else if (rodsObjStatOut->specColl != NULL) {
        free (rodsObjStatOut);
        rodsLog (LOG_ERROR,
          "mountFileDir: %s already mounted", phyPathRegInp->objPath);
        return (SYS_MOUNT_MOUNTED_COLL_ERR);
    }
    free (rodsObjStatOut);

    memset (&fileOpendirInp, 0, sizeof (fileOpendirInp));

    rescTypeInx = rescInfo->rescTypeInx;
    rstrcpy (fileOpendirInp.dirName, filePath, MAX_NAME_LEN);
    fileOpendirInp.fileType = RescTypeDef[rescTypeInx].driverType;
    rstrcpy (fileOpendirInp.addr.hostAddr,  rescInfo->rescLoc, NAME_LEN);

    dirFd = rsFileOpendir (rsComm, &fileOpendirInp);
    if (dirFd < 0) {
       rodsLog (LOG_ERROR,
          "dirPathReg: rsFileOpendir for %s error, status = %d",
          filePath, dirFd);
        return (dirFd);
    }

    fileReaddirInp.fileInx = dirFd;

    while ((status = rsFileReaddir (rsComm, &fileReaddirInp, &rodsDirent))
      >= 0) {
        fileStatInp_t fileStatInp;
	rodsStat_t *myStat = NULL;

        if (strcmp (rodsDirent->d_name, ".") == 0 ||
          strcmp (rodsDirent->d_name, "..") == 0) {
            continue;
        }

	memset (&fileStatInp, 0, sizeof (fileStatInp));

        snprintf (fileStatInp.fileName, MAX_NAME_LEN, "%s/%s",
          filePath, rodsDirent->d_name);

        fileStatInp.fileType = fileOpendirInp.fileType; 
	fileStatInp.addr = fileOpendirInp.addr;
        status = rsFileStat (rsComm, &fileStatInp, &myStat);

	if (status != 0) {
            rodsLog (LOG_ERROR,
	      "dirPathReg: rsFileStat failed for %s, status = %d",
	      fileStatInp.fileName, status);
	    return (status);
	}

	subPhyPathRegInp = *phyPathRegInp;
	snprintf (subPhyPathRegInp.objPath, MAX_NAME_LEN, "%s/%s",
	  phyPathRegInp->objPath, rodsDirent->d_name);

	if ((myStat->st_mode & S_IFREG) != 0) {     /* a file */
            addKeyVal (&subPhyPathRegInp.condInput, FILE_PATH_KW, 
	      fileStatInp.fileName);
	    subPhyPathRegInp.dataSize = myStat->st_size;
	    status = filePathReg (rsComm, &subPhyPathRegInp, 
	      fileStatInp.fileName, rescInfo);
        } else if ((myStat->st_mode & S_IFDIR) != 0) {      /* a directory */
            status = dirPathReg (rsComm, &subPhyPathRegInp,
              fileStatInp.fileName, rescInfo);
	}
	free (myStat);
    }
    if (status == -1) {         /* just EOF */
        status = 0;
    }

    fileClosedirInp.fileInx = dirFd;
    rsFileClosedir (rsComm, &fileClosedirInp);

    return (status);
}

int
mountFileDir (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp, char *filePath,
rescInfo_t *rescInfo)
{
    collInp_t collCreateInp;
    int rescTypeInx;
    int status;
    fileStatInp_t fileStatInp;
    rodsStat_t *myStat = NULL;
    rodsObjStat_t *rodsObjStatOut = NULL;

    status = collStat (rsComm, phyPathRegInp, &rodsObjStatOut);
    if (status < 0) return status;

    if (rodsObjStatOut->specColl != NULL) {
        free (rodsObjStatOut);
        rodsLog (LOG_ERROR,
          "mountFileDir: %s already mounted", phyPathRegInp->objPath);
        return (SYS_MOUNT_MOUNTED_COLL_ERR);
    }
    free (rodsObjStatOut);

    if (isCollEmpty (rsComm, phyPathRegInp->objPath) == False) {
        rodsLog (LOG_ERROR,
          "mountFileDir: collection %s not empty", phyPathRegInp->objPath);
        return (SYS_COLLECTION_NOT_EMPTY);
    }

    memset (&fileStatInp, 0, sizeof (fileStatInp));

    rstrcpy (fileStatInp.fileName, filePath, MAX_NAME_LEN);

    rescTypeInx = rescInfo->rescTypeInx;
    fileStatInp.fileType = RescTypeDef[rescTypeInx].driverType;
    rstrcpy (fileStatInp.addr.hostAddr,  rescInfo->rescLoc, NAME_LEN);
    status = rsFileStat (rsComm, &fileStatInp, &myStat);

    if (status < 0) {
	fileMkdirInp_t fileMkdirInp;

        rodsLog (LOG_NOTICE,
          "mountFileDir: rsFileStat failed for %s, status = %d, create it",
          fileStatInp.fileName, status);
	memset (&fileMkdirInp, 0, sizeof (fileMkdirInp));
	rstrcpy (fileMkdirInp.dirName, filePath, MAX_NAME_LEN);
        fileMkdirInp.fileType = RescTypeDef[rescTypeInx].driverType;
	fileMkdirInp.mode = DEFAULT_DIR_MODE;
        rstrcpy (fileMkdirInp.addr.hostAddr,  rescInfo->rescLoc, NAME_LEN);
	status = rsFileMkdir (rsComm, &fileMkdirInp);
	if (status < 0) {
            return (status);
	}
    } else if ((myStat->st_mode & S_IFDIR) == 0) {
        rodsLog (LOG_ERROR,
          "mountFileDir: phyPath %s is not a directory",
          fileStatInp.fileName);
	free (myStat);
        return (USER_FILE_DOES_NOT_EXIST);
    }

    free (myStat);
    /* mk the collection */

    memset (&collCreateInp, 0, sizeof (collCreateInp));
    rstrcpy (collCreateInp.collName, phyPathRegInp->objPath, MAX_NAME_LEN);
    addKeyVal (&collCreateInp.condInput, COLLECTION_TYPE_KW, MOUNT_POINT_STR);

    addKeyVal (&collCreateInp.condInput, COLLECTION_INFO1_KW, filePath);
    addKeyVal (&collCreateInp.condInput, COLLECTION_INFO2_KW, 
      rescInfo->rescName);

    /* try to mod the coll first */
    status = rsModColl (rsComm, &collCreateInp);

    if (status < 0) {	/* try to create it */
       status = rsRegColl (rsComm, &collCreateInp);
    }

    return (status);
}

int
unmountFileDir (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp)
{
    int status;
    collInp_t modCollInp;
    rodsObjStat_t *rodsObjStatOut = NULL;

    status = collStat (rsComm, phyPathRegInp, &rodsObjStatOut);
    if (status < 0) {
        return status;
    } else if (rodsObjStatOut->specColl == NULL) {
        free (rodsObjStatOut);
        rodsLog (LOG_ERROR,
          "unmountFileDir: %s not mounted", phyPathRegInp->objPath);
        return (SYS_COLL_NOT_MOUNTED_ERR);
    }

    if (getStructFileType (rodsObjStatOut->specColl) >= 0) {    
	/* a struct file */
        status = _rsSyncMountedColl (rsComm, rodsObjStatOut->specColl,
          PURGE_STRUCT_FILE_CACHE);
#if 0
	if (status < 0) {
	    free (rodsObjStatOut);
	    return (status);
	}
#endif
    }

    free (rodsObjStatOut);

    memset (&modCollInp, 0, sizeof (modCollInp));
    rstrcpy (modCollInp.collName, phyPathRegInp->objPath, MAX_NAME_LEN);
    addKeyVal (&modCollInp.condInput, COLLECTION_TYPE_KW, 
      "NULL_SPECIAL_VALUE");
    addKeyVal (&modCollInp.condInput, COLLECTION_INFO1_KW, "NULL_SPECIAL_VALUE");
    addKeyVal (&modCollInp.condInput, COLLECTION_INFO2_KW, "NULL_SPECIAL_VALUE");

    status = rsModColl (rsComm, &modCollInp);

    return (status);
}

int
structFileReg (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp)
{
    collInp_t collCreateInp;
    int status;
    dataObjInfo_t *dataObjInfo = NULL;
    char *structFilePath = NULL;
    dataObjInp_t dataObjInp;
    char *collType;
    int len;
    rodsObjStat_t *rodsObjStatOut = NULL;
    specCollCache_t *specCollCache = NULL;
    rescInfo_t *rescInfo = NULL;

    if ((structFilePath = getValByKey (&phyPathRegInp->condInput, FILE_PATH_KW))
      == NULL) {
        rodsLog (LOG_ERROR,
          "structFileReg: No structFilePath input for %s",
          phyPathRegInp->objPath);
        return (SYS_INVALID_FILE_PATH);
    }

    collType = getValByKey (&phyPathRegInp->condInput, COLLECTION_TYPE_KW);
    if (collType == NULL) {
        rodsLog (LOG_ERROR,
          "structFileReg: Bad COLLECTION_TYPE_KW for structFilePath %s",
              dataObjInp.objPath);
            return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    len = strlen (phyPathRegInp->objPath);
    if (strncmp (structFilePath, phyPathRegInp->objPath, len) == 0 &&
     (structFilePath[len] == '\0' || structFilePath[len] == '/')) {
        rodsLog (LOG_ERROR,
          "structFileReg: structFilePath %s inside collection %s",
          structFilePath, phyPathRegInp->objPath);
        return (SYS_STRUCT_FILE_INMOUNTED_COLL);
    }

    /* see if the struct file is in spec coll */

    if (getSpecCollCache (rsComm, structFilePath, 0,  &specCollCache) >= 0) {
        rodsLog (LOG_ERROR,
          "structFileReg: structFilePath %s is in a mounted path",
          structFilePath);
        return (SYS_STRUCT_FILE_INMOUNTED_COLL);
    }

    status = collStat (rsComm, phyPathRegInp, &rodsObjStatOut);
    if (status < 0) return status;
 
    if (rodsObjStatOut->specColl != NULL) {
	free (rodsObjStatOut);
        rodsLog (LOG_ERROR,
          "structFileReg: %s already mounted", phyPathRegInp->objPath);
	return (SYS_MOUNT_MOUNTED_COLL_ERR);
    }

    free (rodsObjStatOut);

    if (isCollEmpty (rsComm, phyPathRegInp->objPath) == False) {
        rodsLog (LOG_ERROR,
          "structFileReg: collection %s not empty", phyPathRegInp->objPath);
        return (SYS_COLLECTION_NOT_EMPTY);
    }

    memset (&dataObjInp, 0, sizeof (dataObjInp));
    rstrcpy (dataObjInp.objPath, structFilePath, sizeof (dataObjInp));
    /* user need to have write permission */
    dataObjInp.openFlags = O_WRONLY;
    status = getDataObjInfoIncSpecColl (rsComm, &dataObjInp, &dataObjInfo);
    if (status < 0) {
	int myStatus;
	/* try to make one */
	dataObjInp.condInput = phyPathRegInp->condInput;
	/* have to remove FILE_PATH_KW because getFullPathName will use it */
	rmKeyVal (&dataObjInp.condInput, FILE_PATH_KW);
	myStatus = rsDataObjCreate (rsComm, &dataObjInp);
	if (myStatus < 0) {
            rodsLog (LOG_ERROR,
              "structFileReg: Problem with open/create structFilePath %s, status = %d",
              dataObjInp.objPath, status);
            return (status);
	} else {
	    openedDataObjInp_t dataObjCloseInp;
	    bzero (&dataObjCloseInp, sizeof (dataObjCloseInp));
	    rescInfo = L1desc[myStatus].dataObjInfo->rescInfo;
	    dataObjCloseInp.l1descInx = myStatus;
	    rsDataObjClose (rsComm, &dataObjCloseInp);
	}
    } else {
	rescInfo = dataObjInfo->rescInfo;
    }

    if (!structFileSupport (rsComm, phyPathRegInp->objPath, 
      collType, rescInfo)) {
        rodsLog (LOG_ERROR,
          "structFileReg: structFileDriver type %s does not exist for %s",
          collType, dataObjInp.objPath);
        return (SYS_NOT_SUPPORTED);
    }

    /* mk the collection */

    memset (&collCreateInp, 0, sizeof (collCreateInp));
    rstrcpy (collCreateInp.collName, phyPathRegInp->objPath, MAX_NAME_LEN);
    addKeyVal (&collCreateInp.condInput, COLLECTION_TYPE_KW, collType);

    /* have to use dataObjInp.objPath because structFile path was removed */ 
    addKeyVal (&collCreateInp.condInput, COLLECTION_INFO1_KW, 
     dataObjInp.objPath);

    /* try to mod the coll first */
    status = rsModColl (rsComm, &collCreateInp);

    if (status < 0) {	/* try to create it */
       status = rsRegColl (rsComm, &collCreateInp);
    }

    return (status);
}

int
structFileSupport (rsComm_t *rsComm, char *collection, char *collType, 
rescInfo_t *rescInfo)
{
    rodsStat_t *myStat = NULL;
    int status;
    subFile_t subFile;
    specColl_t specColl;

    if (rsComm == NULL || collection == NULL || collType == NULL || 
      rescInfo == NULL) return 0;

    memset (&subFile, 0, sizeof (subFile));
    memset (&specColl, 0, sizeof (specColl));
    /* put in some fake path */
    subFile.specColl = &specColl;
    rstrcpy (specColl.collection, collection, MAX_NAME_LEN);
    specColl.collClass = STRUCT_FILE_COLL;
    if (strcmp (collType, HAAW_STRUCT_FILE_STR) == 0) { 
        specColl.type = HAAW_STRUCT_FILE_T;
    } else if (strcmp (collType, TAR_STRUCT_FILE_STR) == 0) {
	specColl.type = TAR_STRUCT_FILE_T;
    } else {
	return (0);
    }
    snprintf (specColl.objPath, MAX_NAME_LEN, "%s/myFakeFile",
      collection);
    rstrcpy (specColl.resource, rescInfo->rescName, NAME_LEN);
    rstrcpy (specColl.phyPath, "/fakeDir1/fakeDir2/myFakeStructFile",
      MAX_NAME_LEN);
    rstrcpy (subFile.subFilePath, "/fakeDir1/fakeDir2/myFakeFile",
      MAX_NAME_LEN);
    rstrcpy (subFile.addr.hostAddr, rescInfo->rescLoc, NAME_LEN);

    status = rsSubStructFileStat (rsComm, &subFile, &myStat);

    if (status == SYS_NOT_SUPPORTED)
	return (0);
    else
	return (1);
}
