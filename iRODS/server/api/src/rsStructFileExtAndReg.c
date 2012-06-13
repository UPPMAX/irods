/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* rsStructFileExtAndReg.c. See structFileExtAndReg.h for a description of 
 * this API call.*/

#include "structFileExtAndReg.h"
#include "apiHeaderAll.h"
#include "objMetaOpr.h"
#include "collection.h"
#include "dataObjOpr.h"
#include "resource.h"
#include "specColl.h"
#include "physPath.h"
#include "objStat.h"
#include "miscServerFunct.h"
#include "fileOpr.h"
#include "rcGlobalExtern.h"
#include "reGlobalsExtern.h"

int
rsStructFileExtAndReg (rsComm_t *rsComm,
structFileExtAndRegInp_t *structFileExtAndRegInp)
{
    int status;
    dataObjInp_t dataObjInp;
    openedDataObjInp_t dataObjCloseInp;
    dataObjInfo_t *dataObjInfo;
    int l1descInx;
    rescInfo_t *rescInfo;
    char *rescGroupName;
    int remoteFlag;
    rodsServerHost_t *rodsServerHost;
    char phyBunDir[MAX_NAME_LEN], *tmpStr;
    int flags = 0;
#if 0
    dataObjInp_t dirRegInp;
    structFileOprInp_t structFileOprInp;
#endif
    specCollCache_t *specCollCache = NULL;

    resolveLinkedPath (rsComm, structFileExtAndRegInp->objPath, &specCollCache,
      &structFileExtAndRegInp->condInput);

    resolveLinkedPath (rsComm, structFileExtAndRegInp->collection, 
      &specCollCache, NULL);

    if (!isSameZone (structFileExtAndRegInp->objPath, 
      structFileExtAndRegInp->collection))
        return SYS_CROSS_ZONE_MV_NOT_SUPPORTED;

    memset (&dataObjInp, 0, sizeof (dataObjInp));
    rstrcpy (dataObjInp.objPath, structFileExtAndRegInp->objPath,
      MAX_NAME_LEN);

    /* replicate the condInput. may have resource input */
    replKeyVal (&structFileExtAndRegInp->condInput, &dataObjInp.condInput);
    dataObjInp.openFlags = O_RDONLY;

    remoteFlag = getAndConnRemoteZone (rsComm, &dataObjInp, &rodsServerHost,
      REMOTE_OPEN);

    if (remoteFlag < 0) {
        return (remoteFlag);
    } else if (remoteFlag == REMOTE_HOST) {
        status = rcStructFileExtAndReg (rodsServerHost->conn, 
          structFileExtAndRegInp);
        return status;
    }

    /* open the structured file */
    addKeyVal (&dataObjInp.condInput, NO_OPEN_FLAG_KW, "");
    l1descInx = _rsDataObjOpen (rsComm, &dataObjInp);

    if (l1descInx < 0) {
        rodsLog (LOG_ERROR,
          "rsStructFileExtAndReg: _rsDataObjOpen of %s error. status = %d",
          dataObjInp.objPath, l1descInx);
        return (l1descInx);
    }

    rescInfo = L1desc[l1descInx].dataObjInfo->rescInfo;
    rescGroupName = L1desc[l1descInx].dataObjInfo->rescGroupName;
    remoteFlag = resolveHostByRescInfo (rescInfo, &rodsServerHost);
    bzero (&dataObjCloseInp, sizeof (dataObjCloseInp));
    dataObjCloseInp.l1descInx = l1descInx;

    if (remoteFlag == REMOTE_HOST) {
        addKeyVal (&structFileExtAndRegInp->condInput, RESC_NAME_KW,
          rescInfo->rescName);

        if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
            return status;
        }
        status = rcStructFileExtAndReg (rodsServerHost->conn,
          structFileExtAndRegInp);

        rsDataObjClose (rsComm, &dataObjCloseInp);


        return status;
    }

    status = chkCollForExtAndReg (rsComm, structFileExtAndRegInp->collection, 
      NULL);
    if (status < 0) return status;


    dataObjInfo = L1desc[l1descInx].dataObjInfo;

    createPhyBundleDir (rsComm, dataObjInfo->filePath, phyBunDir);

    status = unbunPhyBunFile (rsComm, dataObjInp.objPath, rescInfo, 
      dataObjInfo->filePath, phyBunDir, dataObjInfo->dataType, 0);

    if (status == SYS_DIR_IN_VAULT_NOT_EMPTY) {
	/* rename the phyBunDir */
    tmpStr = strdup(phyBunDir); // cppcheck - Undefined behavior: same parameter and destination in snprintf().
	snprintf (phyBunDir, MAX_NAME_LEN, "%s.%-d", tmpStr, (int) random ());
	free(tmpStr);
        status = unbunPhyBunFile (rsComm, dataObjInp.objPath, rescInfo,
          dataObjInfo->filePath, phyBunDir,  dataObjInfo->dataType, 0);
    }
    if (status < 0) {
        rodsLog (LOG_ERROR,
        "rsStructFileExtAndReg:unbunPhyBunFile err for %s to dir %s.stat=%d",
          dataObjInfo->filePath, phyBunDir, status);
        rsDataObjClose (rsComm, &dataObjCloseInp);
        return status;
    }

    if (getValByKey (&structFileExtAndRegInp->condInput, FORCE_FLAG_KW) 
      != NULL) {
	flags = flags | FORCE_FLAG_FLAG;
    }
    if (getValByKey (&structFileExtAndRegInp->condInput, BULK_OPR_KW)
      != NULL) {
        status = bulkRegUnbunSubfiles (rsComm, rescInfo, rescGroupName,
          structFileExtAndRegInp->collection, phyBunDir, flags, NULL);
    } else {
        status = regUnbunSubfiles (rsComm, rescInfo, rescGroupName,
          structFileExtAndRegInp->collection, phyBunDir, flags, NULL);
    }

    if (status == CAT_NO_ROWS_FOUND) {
        /* some subfiles have been deleted. harmless */
        status = 0;
    } else if (status < 0) {
        rodsLog (LOG_ERROR,
          "_rsUnbunAndRegPhyBunfile: rsStructFileExtAndReg for dir %s.stat=%d",
          phyBunDir, status);
    }
    rsDataObjClose (rsComm, &dataObjCloseInp);

    return status;
}

int 
chkCollForExtAndReg (rsComm_t *rsComm, char *collection, 
rodsObjStat_t **rodsObjStatOut)
{
    dataObjInp_t dataObjInp;
    int status;
    rodsObjStat_t *myRodsObjStat = NULL;

    bzero (&dataObjInp, sizeof (dataObjInp));
    rstrcpy (dataObjInp.objPath, collection, MAX_NAME_LEN);
#if 0	/* allow mounted coll */
    status = collStat (rsComm, &dataObjInp, &myRodsObjStat);
#endif
    status = collStatAllKinds (rsComm, &dataObjInp, &myRodsObjStat);
#if 0
    if (status == CAT_NO_ROWS_FOUND || status == OBJ_PATH_DOES_NOT_EXIST ||
      status == USER_FILE_DOES_NOT_EXIST) {
#endif
    if (status < 0) { 
	status = rsMkCollR (rsComm, "/", collection);
	if (status < 0) {
            rodsLog (LOG_ERROR,
              "chkCollForExtAndReg: rsMkCollR of %s error. status = %d",
              collection, status);
            return (status);
	} else {
#if 0	/* allow mounted coll */
	    status = collStat (rsComm, &dataObjInp, &myRodsObjStat);
#endif
	    status = collStatAllKinds (rsComm, &dataObjInp, &myRodsObjStat);
	}
    }

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "chkCollForExtAndReg: collStat of %s error. status = %d",
          dataObjInp.objPath, status);
        return (status);
    } else if (myRodsObjStat->specColl != NULL && 
      myRodsObjStat->specColl->collClass != MOUNTED_COLL) {
	/* only do mounted coll */
        freeRodsObjStat (myRodsObjStat);
        rodsLog (LOG_ERROR,
          "chkCollForExtAndReg: %s is a struct file collection",
          dataObjInp.objPath);
        return (SYS_STRUCT_FILE_INMOUNTED_COLL);
    }

    if (myRodsObjStat->specColl == NULL) {
        status = checkCollAccessPerm (rsComm, collection, ACCESS_DELETE_OBJECT);
    } else {
	status = checkCollAccessPerm (rsComm, 
	  myRodsObjStat->specColl->collection, ACCESS_DELETE_OBJECT);
    }

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "chkCollForExtAndReg: no permission to write %s, status = %d",
          collection, status);
        freeRodsObjStat (myRodsObjStat);
    } else {
	if (rodsObjStatOut != NULL) {
	    *rodsObjStatOut = myRodsObjStat;
	} else {
            freeRodsObjStat (myRodsObjStat);
	}
    }
    return (status);
}

/* regUnbunSubfiles - non bulk version of registering all files in phyBunDir 
 * to the collection. Valid values for flags are: 
 *	FORCE_FLAG_FLAG.
 */

int
regUnbunSubfiles (rsComm_t *rsComm, rescInfo_t *rescInfo, char *rescGroupName,
char *collection, char *phyBunDir, int flags, genQueryOut_t *attriArray)
{
#ifndef USE_BOOST_FS
    DIR *dirPtr;
    struct dirent *myDirent;
    struct stat statbuf;
#endif
    char subfilePath[MAX_NAME_LEN];
    char subObjPath[MAX_NAME_LEN];
    dataObjInp_t dataObjInp;
    int status;
    int savedStatus = 0;
    rodsLong_t st_size;

#ifdef USE_BOOST_FS
    path srcDirPath (phyBunDir);
    if (!exists(srcDirPath) || !is_directory(srcDirPath)) {
#else
    dirPtr = opendir (phyBunDir);
    if (dirPtr == NULL) {
#endif
        rodsLog (LOG_ERROR,
        "regUnbunphySubfiles: opendir error for %s, errno = %d",
         phyBunDir, errno);
        return (UNIX_FILE_OPENDIR_ERR - errno);
    }
    bzero (&dataObjInp, sizeof (dataObjInp));
#ifdef USE_BOOST_FS
    directory_iterator end_itr; // default construction yields past-the-end
    for (directory_iterator itr(srcDirPath); itr != end_itr;++itr) {
        path p = itr->path();
        snprintf (subfilePath, MAX_NAME_LEN, "%s",
          p.c_str ());
#else
    while ((myDirent = readdir (dirPtr)) != NULL) {
        if (strcmp (myDirent->d_name, ".") == 0 ||
          strcmp (myDirent->d_name, "..") == 0) {
            continue;
        }
        snprintf (subfilePath, MAX_NAME_LEN, "%s/%s",
          phyBunDir, myDirent->d_name);
#endif

#ifdef USE_BOOST_FS
        if (!exists (p)) {
#else
        status = lstat (subfilePath, &statbuf);

        if (status != 0) {
#endif
            rodsLog (LOG_ERROR,
              "regUnbunphySubfiles: stat error for %s, errno = %d",
              subfilePath, errno);
            savedStatus = UNIX_FILE_STAT_ERR - errno;
	    unlink (subfilePath);
	    continue;
        }

#ifdef USE_BOOST_FS
	if (is_symlink (p)) {
#else
	if ((statbuf.st_mode & S_IFLNK) == S_IFLNK) {
#endif
            rodsLogError (LOG_ERROR, SYMLINKED_BUNFILE_NOT_ALLOWED,
              "regUnbunSubfiles: %s is a symlink",
              subfilePath);
            savedStatus = SYMLINKED_BUNFILE_NOT_ALLOWED;
            continue;
        }
#ifdef USE_BOOST_FS
        path childPath = p.filename();
        snprintf (subObjPath, MAX_NAME_LEN, "%s/%s",
          collection, childPath.c_str());

	if (is_directory (p)) {
#else
        snprintf (subObjPath, MAX_NAME_LEN, "%s/%s",
          collection, myDirent->d_name);

        if ((statbuf.st_mode & S_IFDIR) != 0) {
#endif
            status = rsMkCollR (rsComm, "/", subObjPath);
            if (status < 0) {
                rodsLog (LOG_ERROR,
                  "regUnbunSubfiles: rsMkCollR of %s error. status = %d",
                  subObjPath, status);
                savedStatus = status;
		continue;
	    }
	    status = regUnbunSubfiles (rsComm, rescInfo, rescGroupName,
	      subObjPath, subfilePath, flags, attriArray);
            if (status < 0) {
                rodsLog (LOG_ERROR,
                  "regUnbunSubfiles: regUnbunSubfiles of %s error. status=%d",
                  subObjPath, status);
                savedStatus = status;
                continue;
            }
#ifdef USE_BOOST_FS
        } else if (is_regular_file (p)) {
	    st_size = file_size (p);
#else
        } else if ((statbuf.st_mode & S_IFREG) != 0) {
	    st_size = statbuf.st_size;
#endif
	    status = regSubfile (rsComm, rescInfo, rescGroupName,
		subObjPath, subfilePath, st_size, flags);
	    unlink (subfilePath);
            if (status < 0) {
                rodsLog (LOG_ERROR,
                  "regUnbunSubfiles: regSubfile of %s error. status=%d",
                  subObjPath, status);
                savedStatus = status;
                continue;
	    }
	}
    }
#ifndef USE_BOOST_FS
    closedir (dirPtr);
#endif
    rmdir (phyBunDir);
    return savedStatus;
}

int
regSubfile (rsComm_t *rsComm, rescInfo_t *rescInfo, char *rescGroupName,
char *subObjPath, char *subfilePath, rodsLong_t dataSize, int flags)
{
    dataObjInfo_t dataObjInfo;
    dataObjInp_t dataObjInp;
#ifndef USE_BOOST_FS
    struct stat statbuf;
#endif
    int status;
    int modFlag = 0;

    bzero (&dataObjInp, sizeof (dataObjInp));
    bzero (&dataObjInfo, sizeof (dataObjInfo));
    rstrcpy (dataObjInp.objPath, subObjPath, MAX_NAME_LEN);
    rstrcpy (dataObjInfo.objPath, subObjPath, MAX_NAME_LEN);
    rstrcpy (dataObjInfo.rescName, rescInfo->rescName, NAME_LEN);
    rstrcpy (dataObjInfo.dataType, "generic", NAME_LEN);
    dataObjInfo.rescInfo = rescInfo;
    rstrcpy (dataObjInfo.rescGroupName, rescGroupName, NAME_LEN);
    dataObjInfo.dataSize = dataSize;

    status = getFilePathName (rsComm, &dataObjInfo, &dataObjInp);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "regSubFile: getFilePathName err for %s. status = %d",
          dataObjInp.objPath, status);
        return (status);
    }

#ifdef USE_BOOST_FS
    path p (dataObjInfo.filePath);
    if (exists (p)) {
	if (is_directory (p)) {
#else
    status = stat (dataObjInfo.filePath, &statbuf);
    if (status == 0 || errno != ENOENT) {
        if ((statbuf.st_mode & S_IFDIR) != 0) {
#endif
	    return SYS_PATH_IS_NOT_A_FILE;
	}

        if (chkOrphanFile (rsComm, dataObjInfo.filePath, rescInfo->rescName, 
	  &dataObjInfo) > 0) {
	    /* an orphan file. just rename it */
	    fileRenameInp_t fileRenameInp;
	    bzero (&fileRenameInp, sizeof (fileRenameInp));
            rstrcpy (fileRenameInp.oldFileName, dataObjInfo.filePath, 
	      MAX_NAME_LEN);
            status = renameFilePathToNewDir (rsComm, ORPHAN_DIR, 
	      &fileRenameInp, rescInfo, 1);
            if (status < 0) {
                rodsLog (LOG_ERROR,
                  "regSubFile: renameFilePathToNewDir err for %s. status = %d",
                  fileRenameInp.oldFileName, status);
                return (status);
	    }
	} else {
	    /* not an orphan file */
	    if ((flags & FORCE_FLAG_FLAG) != 0 && dataObjInfo.dataId > 0 && 
	      strcmp (dataObjInfo.objPath, subObjPath) == 0) {
		/* overwrite the current file */
		modFlag = 1;
		unlink (dataObjInfo.filePath);
	    } else {
		status = SYS_COPY_ALREADY_IN_RESC;
                rodsLog (LOG_ERROR,
                  "regSubFile: phypath %s is already in use. status = %d",
                  dataObjInfo.filePath, status);
                return (status);
	    }
        }
    }
    /* make the necessary dir */
    mkDirForFilePath (UNIX_FILE_TYPE, rsComm, "/", dataObjInfo.filePath,
      getDefDirMode ());
    /* add a link */

#ifndef windows_platform   /* Windows does not support link */
    status = link (subfilePath, dataObjInfo.filePath);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "regSubFile: link error %s to %s. errno = %d",
          subfilePath, dataObjInfo.filePath, errno);
        return (UNIX_FILE_LINK_ERR - errno);
    }
#endif

    if (modFlag == 0) {
        status = svrRegDataObj (rsComm, &dataObjInfo);
    } else {
        char tmpStr[MAX_NAME_LEN];
        modDataObjMeta_t modDataObjMetaInp;
	keyValPair_t regParam;

	bzero (&modDataObjMetaInp, sizeof (modDataObjMetaInp));
	bzero (&regParam, sizeof (regParam));
        snprintf (tmpStr, MAX_NAME_LEN, "%lld", dataSize);
        addKeyVal (&regParam, DATA_SIZE_KW, tmpStr);
        addKeyVal (&regParam, ALL_REPL_STATUS_KW, tmpStr);
        snprintf (tmpStr, MAX_NAME_LEN, "%d", (int) time (NULL));
        addKeyVal (&regParam, DATA_MODIFY_KW, tmpStr);

        modDataObjMetaInp.dataObjInfo = &dataObjInfo;
        modDataObjMetaInp.regParam = &regParam;

        status = rsModDataObjMeta (rsComm, &modDataObjMetaInp);

        clearKeyVal (&regParam);
    }

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "regSubFile: svrRegDataObj of %s. errno = %d",
          dataObjInfo.objPath, errno);
	unlink (dataObjInfo.filePath);
    } else {
	ruleExecInfo_t rei;
	dataObjInp_t dataObjInp;
	bzero (&dataObjInp, sizeof (dataObjInp));
	rstrcpy (dataObjInp.objPath, dataObjInfo.objPath, MAX_NAME_LEN);
	initReiWithDataObjInp (&rei, rsComm, &dataObjInp);
	rei.doi = &dataObjInfo;
	rei.status = applyRule ("acPostProcForTarFileReg", NULL, &rei,
                    NO_SAVE_REI);
	if (rei.status < 0) {
            rodsLogError (LOG_ERROR, rei.status,
              "regSubFile: acPostProcForTarFileReg error for %s. status = %d",
              dataObjInfo.objPath);
	}
    }
    return status;
}

