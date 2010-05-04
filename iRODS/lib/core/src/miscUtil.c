/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

#ifndef windows_platform
#include <sys/time.h>
#endif
#include "rodsClient.h"
#include "rodsLog.h"
#include "miscUtil.h"

static uint Myumask = INIT_UMASK_VAL;

int
mkColl (rcComm_t *conn, char *collection)
{
    int status;
    collInp_t collCreateInp;

    memset (&collCreateInp, 0, sizeof (collCreateInp));

    rstrcpy (collCreateInp.collName, collection, MAX_NAME_LEN);
   
    status = rcCollCreate (conn, &collCreateInp);

    if (status == CATALOG_ALREADY_HAS_ITEM_BY_THAT_NAME)
	status = 0;

    return (status);
}

/* mk the directory resursively */

int
mkCollR (rcComm_t *conn, char *startColl, char *destColl)
{
    int status;
    int startLen;
    int pathLen, tmpLen;
    char tmpPath[MAX_NAME_LEN];
    rodsPath_t rodsPath;

    startLen = strlen (startColl);
    pathLen = strlen (destColl);

    rstrcpy (tmpPath, destColl, MAX_NAME_LEN);

    tmpLen = pathLen;

    memset (&rodsPath, 0, sizeof (rodsPath));
    while (tmpLen > startLen) {
        rodsPath.objType = COLL_OBJ_T;
	rodsPath.objState = UNKNOWN_ST;
	rstrcpy (rodsPath.outPath, tmpPath, MAX_NAME_LEN);
	status =getRodsObjType (conn, &rodsPath);
        if (status >= 0 && rodsPath.objState == EXIST_ST) {
	    clearRodsPath (&rodsPath);
	    break;
        } else {
	    clearRodsPath (&rodsPath);
	}

        /* Go backward */

        while (tmpLen && tmpPath[tmpLen] != '/')
            tmpLen --;
        tmpPath[tmpLen] = '\0';
    }

    /* Now we go forward and make the required coll */
    while (tmpLen < pathLen) {
        /* Put back the '/' */
        tmpPath[tmpLen] = '/';
       status = mkColl (conn, tmpPath);
        if (status < 0) {
            rodsLog (LOG_NOTICE,
             "mkCollR: mkdir failed for %s, status =%d",
              tmpPath, status);
            return status;
        }
        while (tmpLen && tmpPath[tmpLen] != '\0')
            tmpLen ++;
    }
    return 0;
}

int
mkdirR (char *startDir, char *destDir, int mode)
{
    int status;
    int startLen;
    int pathLen, tmpLen;
    char tmpPath[MAX_NAME_LEN];
#ifndef windows_platform
    struct stat statbuf;
#else
	struct irodsntstat statbuf;
#endif

    startLen = strlen (startDir);
    pathLen = strlen (destDir);

    rstrcpy (tmpPath, destDir, MAX_NAME_LEN);

    tmpLen = pathLen;

    while (tmpLen > startLen) {
#ifndef windows_platform
	if ((status = stat (tmpPath, &statbuf)) >= 0) break;
#else
	if ((status = iRODSNt_stat(tmpPath, &statbuf)) >= 0) break;
#endif
        if (status >= 0) {
	    break;
        }

        /* Go backward */

        while (tmpLen && tmpPath[tmpLen] != '/')
            tmpLen --;
        tmpPath[tmpLen] = '\0';
    }

    /* Now we go forward and make the required coll */
    while (tmpLen < pathLen) {
        /* Put back the '/' */
        tmpPath[tmpLen] = '/';
#ifdef _WIN32
        status = iRODSNt_mkdir (tmpPath, mode);
#else
        status = mkdir (tmpPath, mode);
#endif
        if (status < 0) {
            rodsLog (LOG_NOTICE,
             "mkdirR: mkdir failed for %s, errno =%d",
              tmpPath, errno);
            return UNIX_FILE_MKDIR_ERR - errno;
        }
        while (tmpLen && tmpPath[tmpLen] != '\0')
            tmpLen ++;
    }
    return 0;
}

int
mkdirForFilePath (char* filePath)
{
    char child[MAX_NAME_LEN], parent[MAX_NAME_LEN];
    int status;

    if ((status = splitPathByKey (filePath, parent, child, '/')) < 0) {
        rodsLogError (LOG_ERROR, status,
          "mkdirForFilePath:: splitPathByKey for %s error, status = %d",
          filePath, status);
        return (status);
    }
    status =  mkdirR ("/", parent, DEFAULT_DIR_MODE);

    return status;
}

int
rmdirR (char *startDir, char *destDir)
{
    int startLen;
    int pathLen, tmpLen;
    char tmpPath[MAX_NAME_LEN];

    startLen = strlen (startDir);
    pathLen = strlen (destDir);

    rstrcpy (tmpPath, destDir, MAX_NAME_LEN);

    tmpLen = pathLen;

    while (tmpLen > startLen) {
	rmdir (tmpPath);

        /* Go backward */

        while (tmpLen && tmpPath[tmpLen] != '/')
            tmpLen --;
        tmpPath[tmpLen] = '\0';
    }
    return (0);
}

int 
getRodsObjType (rcComm_t *conn, rodsPath_t *rodsPath)
{
    int status;
    dataObjInp_t dataObjInp;
    rodsObjStat_t *rodsObjStatOut = NULL;

    if (rodsPath == NULL) {
	return (USER__NULL_INPUT_ERR);
    }

    memset (&dataObjInp, 0, sizeof (dataObjInp));
    rstrcpy (dataObjInp.objPath, rodsPath->outPath, MAX_NAME_LEN);
    status = rcObjStat (conn, &dataObjInp, &rodsObjStatOut);

    if (status < 0) {
        rodsPath->objState = NOT_EXIST_ST;
	if (status == USER_FILE_DOES_NOT_EXIST) {
            return (NOT_EXIST_ST);
	} else {
	    rodsLogError (LOG_ERROR, status, 
	     "rcObjStat of %s failed", rodsPath->outPath);
	    return status;
	}
    } else if (rodsPath->objType == COLL_OBJ_T && 
      rodsObjStatOut->objType != COLL_OBJ_T) {
        rodsPath->objState = NOT_EXIST_ST;
    } else if (rodsPath->objType == DATA_OBJ_T && 
      rodsObjStatOut->objType != DATA_OBJ_T) {
        rodsPath->objState = NOT_EXIST_ST;
    } else {
	if (rodsObjStatOut->objType == UNKNOWN_OBJ_T) {
	    rodsPath->objState = NOT_EXIST_ST;
	} else {
	    rodsPath->objState = EXIST_ST;
	}
	rodsPath->objType = rodsObjStatOut->objType;
	if (rodsPath->objType == DATA_OBJ_T) {
	    rodsPath->objMode = rodsObjStatOut->dataMode;
            rstrcpy (rodsPath->dataId, rodsObjStatOut->dataId, NAME_LEN);
            rodsPath->size = rodsObjStatOut->objSize;
            rstrcpy (rodsPath->chksum, rodsObjStatOut->chksum, NAME_LEN);
	}
    }
    rodsPath->rodsObjStat = rodsObjStatOut;

    return (rodsPath->objState);
}

int
extractRodsObjType (rodsPath_t *rodsPath, sqlResult_t *dataId, 
sqlResult_t *replStatus, sqlResult_t *chksum, sqlResult_t *dataSize, 
int inx, int rowCnt)
{
    int i;
    char *prevdataId = NULL;
    int gotCopy = 0;

    if (dataId == NULL) {
	/* special coll */
        rodsPath->size =
          strtoll (&dataSize->value[dataSize->len * inx], 0, 0);
        rodsPath->objState = EXIST_ST;
	return (inx);
    }

    i = inx;

    for (i = inx;i < rowCnt; i++) {
	if (prevdataId != NULL) { 
	    if (strcmp (prevdataId, &dataId->value[dataId->len * i]) != 0) {
	        break;
	    }
	} else {
	    prevdataId = &dataId->value[dataId->len * i];
	}

        if (gotCopy == 0 && 
	  atoi (&replStatus->value[replStatus->len * i]) > 0) {
            rstrcpy (rodsPath->dataId, &dataId->value[dataId->len * i],
              NAME_LEN);
            rodsPath->size =
              strtoll (&dataSize->value[dataSize->len * i], 0, 0);
            rstrcpy (rodsPath->chksum,
              &chksum->value[chksum->len * i], NAME_LEN);
	    gotCopy = 1;
        }
    }

    if (strlen (rodsPath->dataId) == 0) {
        /* just use the first one */
        rstrcpy (rodsPath->dataId, &dataId->value[dataId->len * inx], NAME_LEN);
        rodsPath->size = strtoll (&dataSize->value[dataSize->len * inx], 0, 0);
            rstrcpy (rodsPath->chksum, 
	      &chksum->value[chksum->len * inx], NAME_LEN);
    }

    rodsPath->objState = EXIST_ST;
    return (i - 1);
}

/* genAllInCollQCond - Generate a sqlCondInp for querying every thing under  
 * a collection. 
 * The server will handle special char such as "-" and "%".
 */

int
genAllInCollQCond (char *collection, char *collQCond)
{
    if (collection == NULL) {
        return (USER__NULL_INPUT_ERR);
    }

    if (strcmp (collection, "/") == 0) {
        snprintf (collQCond, MAX_NAME_LEN, " like '/%%' ");
    } else {

        snprintf (collQCond, MAX_NAME_LEN*2, " = '%s' || like '%s/%%' ",
          collection, collection);
    }
    return (0);
}

/* queryCollInColl - query the subCollections in a collection.
 */

int
queryCollInColl (queryHandle_t *queryHandle, char *collection,
int flags, genQueryInp_t *genQueryInp,
genQueryOut_t **genQueryOut)
{
    char collQCond[MAX_NAME_LEN];
    int status;

    if (collection == NULL || genQueryOut == NULL) {
        return (USER__NULL_INPUT_ERR);
    }

    memset (genQueryInp, 0, sizeof (genQueryInp_t));

    if ((flags & RECUR_QUERY_FG) != 0) {
        genAllInCollQCond (collection, collQCond);
        addInxVal (&genQueryInp->sqlCondInp, COL_COLL_NAME, collQCond);
    } else {
        snprintf (collQCond, MAX_NAME_LEN, "='%s'", collection);
        addInxVal (&genQueryInp->sqlCondInp, COL_COLL_PARENT_NAME, collQCond);
    }
    addInxIval (&genQueryInp->selectInp, COL_COLL_NAME, 1);
    addInxIval (&genQueryInp->selectInp, COL_COLL_OWNER_NAME, 1);
    addInxIval (&genQueryInp->selectInp, COL_COLL_CREATE_TIME, 1);
    addInxIval (&genQueryInp->selectInp, COL_COLL_MODIFY_TIME, 1);
    addInxIval (&genQueryInp->selectInp, COL_COLL_TYPE, 1);
    addInxIval (&genQueryInp->selectInp, COL_COLL_INFO1, 1);
    addInxIval (&genQueryInp->selectInp, COL_COLL_INFO2, 1);

    genQueryInp->maxRows = MAX_SQL_ROWS;

    status = (*queryHandle->genQuery) (
      (rcComm_t *) queryHandle->conn, genQueryInp, genQueryOut);

    return (status);
}

/* queryDataObjInColl - query the DataObj in a collection.
 */
int
queryDataObjInColl (queryHandle_t *queryHandle, char *collection, 
int flags, genQueryInp_t *genQueryInp,
genQueryOut_t **genQueryOut, keyValPair_t *condInput)
{
    char collQCond[MAX_NAME_LEN];
    int status;
    char *rescName = NULL;

    if (collection == NULL || genQueryOut == NULL) {
        return (USER__NULL_INPUT_ERR);
    }

    memset (genQueryInp, 0, sizeof (genQueryInp_t));

    if ((flags & RECUR_QUERY_FG) != 0) {
        genAllInCollQCond (collection, collQCond);
        addInxVal (&genQueryInp->sqlCondInp, COL_COLL_NAME, collQCond);
    } else {
        snprintf (collQCond, MAX_NAME_LEN, " = '%s'", collection);
        addInxVal (&genQueryInp->sqlCondInp, COL_COLL_NAME, collQCond);
    }

    if ((flags & INCLUDE_CONDINPUT_IN_QUERY) != 0 && 
      condInput != NULL &&
      (rescName = getValByKey (condInput, RESC_NAME_KW)) != NULL) {
        snprintf (collQCond, MAX_NAME_LEN, " = '%s'", rescName);
        addInxVal (&genQueryInp->sqlCondInp, COL_D_RESC_NAME, collQCond);
    }

    setQueryInpForData (flags, genQueryInp);

    genQueryInp->maxRows = MAX_SQL_ROWS;
    genQueryInp->options = RETURN_TOTAL_ROW_COUNT;

    status = (*queryHandle->genQuery) (
      (rcComm_t *) queryHandle->conn, genQueryInp, genQueryOut);

    return (status);

}

int
setQueryInpForData (int flags, genQueryInp_t *genQueryInp)
{

    if (genQueryInp == NULL) { 
        return (USER__NULL_INPUT_ERR);
    }

    addInxIval (&genQueryInp->selectInp, COL_COLL_NAME, 1);
    addInxIval (&genQueryInp->selectInp, COL_DATA_NAME, 1);
    addInxIval (&genQueryInp->selectInp, COL_D_DATA_ID, 1);
    addInxIval (&genQueryInp->selectInp, COL_DATA_MODE, 1);
    addInxIval (&genQueryInp->selectInp, COL_DATA_SIZE, 1);
    addInxIval (&genQueryInp->selectInp, COL_D_MODIFY_TIME, 1);
    addInxIval (&genQueryInp->selectInp, COL_D_CREATE_TIME, 1);
    if ((flags & LONG_METADATA_FG) != 0 || 
      (flags & VERY_LONG_METADATA_FG) != 0) {
        addInxIval (&genQueryInp->selectInp, COL_D_RESC_NAME, 1);
        addInxIval (&genQueryInp->selectInp, COL_D_OWNER_NAME, 1);
        addInxIval (&genQueryInp->selectInp, COL_DATA_REPL_NUM, 1);
        addInxIval (&genQueryInp->selectInp, COL_D_REPL_STATUS, 1);

        if ((flags & VERY_LONG_METADATA_FG) != 0) {
             addInxIval (&genQueryInp->selectInp, COL_D_DATA_PATH, 1);
             addInxIval (&genQueryInp->selectInp, COL_D_DATA_CHECKSUM, 1);
             addInxIval (&genQueryInp->selectInp, COL_D_RESC_GROUP_NAME, 1);
	}
    }

    return (0);
}

int
printTiming (rcComm_t *conn, char *objPath, rodsLong_t fileSize, 
char *localFile, struct timeval *startTime, struct timeval *endTime)
{
    struct timeval diffTime;
    char myDir[MAX_NAME_LEN], myFile[MAX_NAME_LEN];
    float transRate, sizeInMb, timeInSec;
    int status;
    

    if ((status = splitPathByKey (objPath, myDir, myFile, '/')) < 0) {
        rodsLogError (LOG_NOTICE, status,
          "printTiming: splitPathByKey for %s error, status = %d",
          objPath, status);
        return (status);
    }

    diffTime.tv_sec = endTime->tv_sec - startTime->tv_sec;
    diffTime.tv_usec = endTime->tv_usec - startTime->tv_usec;

    if (diffTime.tv_usec < 0) {
	diffTime.tv_sec --;
	diffTime.tv_usec += 1000000;
    }

    timeInSec = (float) diffTime.tv_sec + ((float) diffTime.tv_usec /
     1000000.0);

    if (fileSize < 0) {
	/* may be we can find it from the local file */

	if (localFile != NULL) {
	    fileSize = getFileSize (localFile);
	}
    }

    
    if (fileSize <= 0) {
	transRate = 0.0;
	sizeInMb = 0.0;
    } else {
	sizeInMb = (float) fileSize / 1048600.0;
        if (timeInSec == 0.0) {
	    transRate = 0.0;
        } else {
	    transRate = sizeInMb / timeInSec;
	}
    }

    if (fileSize < 0) {
	fprintf (stdout,
	  "   %-25.25s  %.3f sec\n",    
	    myFile, timeInSec);
    } else {
        fprintf (stdout,
          "   %-25.25s  %10.3f MB | %.3f sec | %d thr | %6.3f MB/s\n",
            myFile, sizeInMb, timeInSec, conn->transStat.numThreads, transRate);
    }

    return (0);
}

#ifdef SYS_TIMING
int
initSysTiming (char *procName, char *action, int envVarFlag)
{
    char *tmpStr;

    (void) gettimeofday(&SysTimingVal, (struct timezone *)0);

    if (envVarFlag > 0) {
	tmpStr = malloc (NAME_LEN);
	snprintf(tmpStr, NAME_LEN, "%s=%u", 
	  SYS_TIMING_SEC, (unsigned int) SysTimingVal.tv_sec);
	putenv(tmpStr);
	tmpStr = malloc (NAME_LEN);
	snprintf(tmpStr, NAME_LEN, "%s=%u", 
	  SYS_TIMING_USEC, (unsigned int) SysTimingVal.tv_usec);
	putenv(tmpStr);
    }
    rodsLog (LOG_NOTICE,
      "initSysTiming: %s at %s", procName, action);

    return 0;
}

int 
printSysTiming (char *procName, char *action, int envVarFlag)
{
    struct timeval curTime, diffTime;
    float timeInSec;
    char *tmpStr;

    if (envVarFlag > 0) {
	if ((tmpStr = getenv (SYS_TIMING_SEC)) == NULL) {
            rodsLog (LOG_ERROR,
              "printSysTiming: env var SYS_TIMING_SEC  not set");
            return -1;
        }
	SysTimingVal.tv_sec = atoi (tmpStr);
        if ((tmpStr = getenv (SYS_TIMING_USEC)) == NULL) {
            rodsLog (LOG_ERROR,
              "printSysTiming: env var SYS_TIMING_USEC  not set");
            return -1;
        }
        SysTimingVal.tv_usec = atoi (tmpStr);
    } else if (SysTimingVal.tv_sec == 0) {
        rodsLog (LOG_NOTICE,
          "printSysTiming: SysTimingVal has not been initialized");
	return -1;
    }
    (void) gettimeofday(&curTime, (struct timezone *)0);

    diffTime.tv_sec = curTime.tv_sec - SysTimingVal.tv_sec;
    diffTime.tv_usec = curTime.tv_usec - SysTimingVal.tv_usec;

    if (diffTime.tv_usec < 0) {
        diffTime.tv_sec --;
        diffTime.tv_usec += 1000000;
    }

    timeInSec = (float) diffTime.tv_sec + ((float) diffTime.tv_usec /
     1000000.0);

    rodsLog (LOG_NOTICE,
      "Time for %s to %s = %8.5f sec", procName, action, timeInSec);

    SysTimingVal = curTime;

    return 0;
}
#endif

int
printNoSync (char *objPath, rodsLong_t fileSize)
{
    char myDir[MAX_NAME_LEN], myFile[MAX_NAME_LEN];
    float sizeInMb;
    int status;
   

    if ((status = splitPathByKey (objPath, myDir, myFile, '/')) < 0) {
        rodsLogError (LOG_NOTICE, status,
          "printNoSync: splitPathByKey for %s error, status = %d",
          objPath, status);
        return (status);
    }

    if (fileSize <= 0) {
        sizeInMb = 0.0;
    } else {
        sizeInMb = (float) fileSize / 1048600.0;
    }

    fprintf (stdout,
      "   %-25.25s  %10.3f MB --- no sync required \n", myFile, sizeInMb);

    return (0);
}

int
queryDataObjAcl (rcComm_t *conn, char *dataId, genQueryOut_t **genQueryOut)
{
    genQueryInp_t genQueryInp;
    int status;
    char tmpStr[MAX_NAME_LEN];

    if (dataId == NULL || genQueryOut == NULL) {
        return (USER__NULL_INPUT_ERR);
    }

    memset (&genQueryInp, 0, sizeof (genQueryInp_t));

    addInxIval (&genQueryInp.selectInp, COL_USER_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_USER_ZONE, 1);
    addInxIval (&genQueryInp.selectInp, COL_DATA_ACCESS_NAME, 1);

    snprintf (tmpStr, MAX_NAME_LEN, " = '%s'", dataId);

    addInxVal (&genQueryInp.sqlCondInp, COL_DATA_ACCESS_DATA_ID, tmpStr);

    snprintf (tmpStr, MAX_NAME_LEN, "='%s'", "access_type");

    /* Currently necessary since other namespaces exist in the token table */
    addInxVal (&genQueryInp.sqlCondInp, COL_DATA_TOKEN_NAMESPACE, tmpStr);

    genQueryInp.maxRows = MAX_SQL_ROWS;

    status =  rcGenQuery (conn, &genQueryInp, genQueryOut);

    return (status);

}


int
queryCollAcl (rcComm_t *conn, char *collName, genQueryOut_t **genQueryOut)
{
    genQueryInp_t genQueryInp;
    genQueryOut_t *myGenQueryOut;
    int status;
    char tmpStr[MAX_NAME_LEN];

    if (collName == NULL || genQueryOut == NULL) {
        return (USER__NULL_INPUT_ERR);
    }

    memset (&genQueryInp, 0, sizeof (genQueryInp_t));

    myGenQueryOut = *genQueryOut = 
      (genQueryOut_t *) malloc (sizeof (genQueryOut_t));
    memset (myGenQueryOut, 0, sizeof (genQueryOut_t));

    clearGenQueryInp (&genQueryInp);

    addInxIval (&genQueryInp.selectInp, COL_COLL_USER_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_COLL_USER_ZONE, 1);

    addInxIval (&genQueryInp.selectInp, COL_COLL_ACCESS_NAME, 1);

    snprintf (tmpStr, MAX_NAME_LEN, "='%s'", "access_type");

    /* Currently necessary since other namespaces exist in the token table */
    addInxVal (&genQueryInp.sqlCondInp, COL_COLL_TOKEN_NAMESPACE, tmpStr);
 
    snprintf (tmpStr, MAX_NAME_LEN, " = '%s'", collName);
    addInxVal (&genQueryInp.sqlCondInp, COL_COLL_NAME, tmpStr);

    genQueryInp.maxRows = MAX_SQL_ROWS;

    status =  rcGenQuery (conn, &genQueryInp, genQueryOut);

    return (status);

}

int
queryCollInheritance (rcComm_t *conn, char *collName, 
		      genQueryOut_t **genQueryOut)
{
    genQueryInp_t genQueryInp;
    genQueryOut_t *myGenQueryOut;
    int status;
    char tmpStr[MAX_NAME_LEN];

    if (collName == NULL || genQueryOut == NULL) {
        return (USER__NULL_INPUT_ERR);
    }

    memset (&genQueryInp, 0, sizeof (genQueryInp_t));

    myGenQueryOut = *genQueryOut = 
      (genQueryOut_t *) malloc (sizeof (genQueryOut_t));
    memset (myGenQueryOut, 0, sizeof (genQueryOut_t));

    clearGenQueryInp (&genQueryInp);

    addInxIval (&genQueryInp.selectInp, COL_COLL_INHERITANCE, 1);

    snprintf (tmpStr, MAX_NAME_LEN, " = '%s'", collName);
    addInxVal (&genQueryInp.sqlCondInp, COL_COLL_NAME, tmpStr);

    genQueryInp.maxRows = MAX_SQL_ROWS;

    status =  rcGenQuery (conn, &genQueryInp, genQueryOut);

    return (status);

}

int
genQueryOutToCollRes (genQueryOut_t **genQueryOut, 
collSqlResult_t *collSqlResult)
{
    genQueryOut_t *myGenQueryOut;
    sqlResult_t *collName, *collType, *collInfo1, *collInfo2, *collOwner,
      *collCreateTime, *collModifyTime;

    if (genQueryOut == NULL || (myGenQueryOut = *genQueryOut) == NULL ||
      collSqlResult == NULL) 
        return (USER__NULL_INPUT_ERR);

    collSqlResult->rowCnt = myGenQueryOut->rowCnt;
    collSqlResult->attriCnt = myGenQueryOut->attriCnt;
    collSqlResult->continueInx = myGenQueryOut->continueInx;
    collSqlResult->totalRowCount = myGenQueryOut->totalRowCount;
    
    if ((collName = getSqlResultByInx (myGenQueryOut, COL_COLL_NAME)) == NULL) {
        rodsLog (LOG_ERROR,
          "genQueryOutToCollRes: getSqlResultByInx for COL_COLL_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    } else {
	collSqlResult->collName = *collName;
    }

    if ((collType = getSqlResultByInx (myGenQueryOut, COL_COLL_TYPE)) == NULL) {
	/* should inherit parent's specColl */
	setSqlResultValue (&collSqlResult->collType, COL_COLL_NAME, 
	  INHERIT_PAR_SPEC_COLL_STR, myGenQueryOut->rowCnt); 
        setSqlResultValue (&collSqlResult->collInfo1, COL_COLL_INFO1, "",
        myGenQueryOut->rowCnt);
        setSqlResultValue (&collSqlResult->collInfo2, COL_COLL_INFO2, "",
        myGenQueryOut->rowCnt);
        setSqlResultValue (&collSqlResult->collOwner, COL_COLL_OWNER_NAME, "",
        myGenQueryOut->rowCnt);
        setSqlResultValue (&collSqlResult->collCreateTime, COL_COLL_CREATE_TIME,
	  "", myGenQueryOut->rowCnt);
        setSqlResultValue (&collSqlResult->collModifyTime, COL_COLL_MODIFY_TIME,
	  "", myGenQueryOut->rowCnt);
    } else {
	collSqlResult->collType = *collType;
        if ((collInfo1 = getSqlResultByInx (myGenQueryOut, COL_COLL_INFO1)) == 
	  NULL) {
            rodsLog (LOG_ERROR,
              "genQueryOutToCollRes: getSqlResultByInx COL_COLL_INFO1 failed");
            return (UNMATCHED_KEY_OR_INDEX);
        } else {
	    collSqlResult->collInfo1 = *collInfo1;
	}
        if ((collInfo2 = getSqlResultByInx (myGenQueryOut, COL_COLL_INFO2)) == 
          NULL) {
            rodsLog (LOG_ERROR,
              "genQueryOutToCollRes: getSqlResultByInx COL_COLL_INFO2 failed");
            free (collSqlResult);
            return (UNMATCHED_KEY_OR_INDEX);
        } else {
            collSqlResult->collInfo2 = *collInfo2;
        }
        if ((collOwner = getSqlResultByInx (myGenQueryOut, 
	  COL_COLL_OWNER_NAME)) == NULL) {
            rodsLog (LOG_ERROR,
              "genQueryOutToCollRes: getSqlResultByInx COL_COLL_OWNER_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        } else {
            collSqlResult->collOwner = *collOwner;
        }
        if ((collCreateTime = getSqlResultByInx (myGenQueryOut,
          COL_COLL_CREATE_TIME)) == NULL) {
            rodsLog (LOG_ERROR,
              "genQueryOutToCollRes: getSqlResultByInx COL_COLL_CREATE_TIME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        } else {
            collSqlResult->collCreateTime = *collCreateTime;
        }
        if ((collModifyTime = getSqlResultByInx (myGenQueryOut,
          COL_COLL_MODIFY_TIME)) == NULL) {
            rodsLog (LOG_ERROR,
              "genQueryOutToCollRes: getSqlResultByInx COL_COLL_MODIFY_TIME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        } else {
            collSqlResult->collModifyTime = *collModifyTime;
        }
    }

    free (*genQueryOut);
    *genQueryOut = NULL;
    return (0);
}
 
int
setSqlResultValue (sqlResult_t *sqlResult, int attriInx, char *valueStr,
int rowCnt)
{
    if (sqlResult == NULL || rowCnt <= 0) return (0);
	
    sqlResult->attriInx = attriInx;
    if (valueStr == NULL) {
	sqlResult->len = 1;
    } else {
        sqlResult->len = strlen (valueStr) + 1;
    }
    if (sqlResult->len == 1) {
	sqlResult->value = (char *)malloc (rowCnt);
	memset (sqlResult->value, 0, rowCnt);
    } else {
        int i;
	char *tmpPtr;
	tmpPtr = sqlResult->value = (char *)malloc (rowCnt * sqlResult->len);
	for (i = 0; i < rowCnt; i++) {
	    rstrcpy (tmpPtr, valueStr, sqlResult->len);
	    tmpPtr += sqlResult->len;
	}
    }
    return (0); 
}

int
clearCollSqlResult (collSqlResult_t *collSqlResult)
{
    if (collSqlResult == NULL) return (USER__NULL_INPUT_ERR);

    if (collSqlResult->collName.value != NULL) 
      free (collSqlResult->collName.value);
    if (collSqlResult->collType.value != NULL) 
      free (collSqlResult->collType.value);
    if (collSqlResult->collInfo1.value != NULL) 
      free (collSqlResult->collInfo1.value);
    if (collSqlResult->collInfo2.value != NULL) 
      free (collSqlResult->collInfo2.value);
    if (collSqlResult->collOwner.value != NULL) 
      free (collSqlResult->collOwner.value);
    if (collSqlResult->collCreateTime.value != NULL) 
      free (collSqlResult->collCreateTime.value);
    if (collSqlResult->collModifyTime.value != NULL) 
      free (collSqlResult->collModifyTime.value);

    memset (collSqlResult, 0, sizeof (collSqlResult_t));

    return 0;
}

int
clearDataObjSqlResult (dataObjSqlResult_t *dataObjSqlResult)
{
    if (dataObjSqlResult == NULL) return (USER__NULL_INPUT_ERR);

    if (dataObjSqlResult->collName.value != NULL)
      free (dataObjSqlResult->collName.value);
    if (dataObjSqlResult->dataName.value != NULL)
      free (dataObjSqlResult->dataName.value);
    if (dataObjSqlResult->dataMode.value != NULL)
      free (dataObjSqlResult->dataMode.value);
    if (dataObjSqlResult->dataSize.value != NULL)
      free (dataObjSqlResult->dataSize.value);
    if (dataObjSqlResult->createTime.value != NULL)
      free (dataObjSqlResult->createTime.value);
    if (dataObjSqlResult->modifyTime.value != NULL)
      free (dataObjSqlResult->modifyTime.value);
    if (dataObjSqlResult->chksum.value != NULL)
      free (dataObjSqlResult->chksum.value);
    if (dataObjSqlResult->replStatus.value != NULL)
      free (dataObjSqlResult->replStatus.value);
    if (dataObjSqlResult->dataId.value != NULL)
      free (dataObjSqlResult->dataId.value);
    if (dataObjSqlResult->resource.value != NULL)
      free (dataObjSqlResult->resource.value);
    if (dataObjSqlResult->phyPath.value != NULL)
      free (dataObjSqlResult->phyPath.value);
    if (dataObjSqlResult->ownerName.value != NULL)
      free (dataObjSqlResult->ownerName.value);
    if (dataObjSqlResult->replNum.value != NULL)
      free (dataObjSqlResult->replNum.value);
    if (dataObjSqlResult->rescGrp.value != NULL)
      free (dataObjSqlResult->rescGrp.value);

    memset (dataObjSqlResult, 0, sizeof (dataObjSqlResult_t));

    return 0;
}

int
genQueryOutToDataObjRes (genQueryOut_t **genQueryOut,
dataObjSqlResult_t *dataObjSqlResult)
{
    genQueryOut_t *myGenQueryOut;
    sqlResult_t *collName, *dataName, *dataSize, *dataMode, *createTime, 
      *modifyTime, *chksum, *replStatus, *dataId, *resource, *phyPath, 
      *ownerName, *replNum, *rescGrp;

    if (genQueryOut == NULL || (myGenQueryOut = *genQueryOut) == NULL ||
      dataObjSqlResult == NULL)
        return (USER__NULL_INPUT_ERR);

    dataObjSqlResult->rowCnt = myGenQueryOut->rowCnt;
    dataObjSqlResult->attriCnt = myGenQueryOut->attriCnt;
    dataObjSqlResult->continueInx = myGenQueryOut->continueInx;
    dataObjSqlResult->totalRowCount = myGenQueryOut->totalRowCount;

    if ((collName = getSqlResultByInx (myGenQueryOut, COL_COLL_NAME)) == NULL) {
        rodsLog (LOG_ERROR,
         "genQueryOutToDataObjRes: getSqlResultByInx for COL_COLL_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    } else {
        dataObjSqlResult->collName = *collName;
    }

    if ((dataName = getSqlResultByInx (myGenQueryOut, COL_DATA_NAME)) == NULL) {
        rodsLog (LOG_ERROR,
         "genQueryOutToDataObjRes: getSqlResultByInx for COL_DATA_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    } else {
        dataObjSqlResult->dataName = *dataName;
    }

    if ((dataMode = getSqlResultByInx (myGenQueryOut, COL_DATA_MODE)) == NULL) {
        setSqlResultValue (&dataObjSqlResult->dataMode, COL_DATA_MODE, "",
        myGenQueryOut->rowCnt);
    } else {
        dataObjSqlResult->dataMode = *dataMode;
    }

    if ((dataSize = getSqlResultByInx (myGenQueryOut, COL_DATA_SIZE)) == NULL) {
	setSqlResultValue (&dataObjSqlResult->dataSize, COL_DATA_SIZE, "-1",
        myGenQueryOut->rowCnt);
    } else {
        dataObjSqlResult->dataSize = *dataSize;
    }

    if ((createTime = getSqlResultByInx (myGenQueryOut, COL_D_CREATE_TIME)) 
      == NULL) {
        setSqlResultValue (&dataObjSqlResult->createTime, COL_D_CREATE_TIME, 
	  "", myGenQueryOut->rowCnt);
    } else {
        dataObjSqlResult->createTime = *createTime;
    }

    if ((modifyTime = getSqlResultByInx (myGenQueryOut, COL_D_MODIFY_TIME)) 
      == NULL) {
        setSqlResultValue (&dataObjSqlResult->modifyTime, COL_D_MODIFY_TIME, 
          "", myGenQueryOut->rowCnt);
    } else {
        dataObjSqlResult->modifyTime = *modifyTime;
    }

    if ((dataId = getSqlResultByInx (myGenQueryOut, COL_D_DATA_ID))
      == NULL) {
        setSqlResultValue (&dataObjSqlResult->dataId, COL_D_DATA_ID,
          "", myGenQueryOut->rowCnt);
    } else {
        dataObjSqlResult->dataId = *dataId;
    }

    if ((chksum = getSqlResultByInx (myGenQueryOut, COL_D_DATA_CHECKSUM)) 
      == NULL) {
        setSqlResultValue (&dataObjSqlResult->chksum, COL_D_DATA_CHECKSUM,
          "", myGenQueryOut->rowCnt);
    } else {
        dataObjSqlResult->chksum = *chksum;
    }

    if ((replStatus = getSqlResultByInx (myGenQueryOut, COL_D_REPL_STATUS)) 
      == NULL) {
        setSqlResultValue (&dataObjSqlResult->replStatus, COL_D_REPL_STATUS, 
	  "", myGenQueryOut->rowCnt);
    } else {
        dataObjSqlResult->replStatus = *replStatus;
    }

    if ((resource = getSqlResultByInx (myGenQueryOut, COL_D_RESC_NAME))
      == NULL) {
        setSqlResultValue (&dataObjSqlResult->resource, COL_D_RESC_NAME,
          "", myGenQueryOut->rowCnt);
    } else {
        dataObjSqlResult->resource = *resource;
    }

    if ((rescGrp = getSqlResultByInx (myGenQueryOut, COL_D_RESC_GROUP_NAME))
      == NULL) {
        setSqlResultValue (&dataObjSqlResult->rescGrp, COL_D_RESC_GROUP_NAME,
          "", myGenQueryOut->rowCnt);
    } else {
        dataObjSqlResult->rescGrp = *rescGrp;
    }

    if ((phyPath = getSqlResultByInx (myGenQueryOut, COL_D_DATA_PATH))
      == NULL) {
        setSqlResultValue (&dataObjSqlResult->phyPath, COL_D_DATA_PATH,
          "", myGenQueryOut->rowCnt);
    } else {
        dataObjSqlResult->phyPath = *phyPath;
    }

    if ((ownerName = getSqlResultByInx (myGenQueryOut, COL_D_OWNER_NAME))
      == NULL) {
        setSqlResultValue (&dataObjSqlResult->ownerName, COL_D_OWNER_NAME,
          "", myGenQueryOut->rowCnt);
    } else {
        dataObjSqlResult->ownerName = *ownerName;
    }

    if ((replNum = getSqlResultByInx (myGenQueryOut, COL_DATA_REPL_NUM))
      == NULL) {
        setSqlResultValue (&dataObjSqlResult->replNum, COL_DATA_REPL_NUM,
          "", myGenQueryOut->rowCnt);
    } else {
        dataObjSqlResult->replNum = *replNum;
    }

    free (*genQueryOut);
    *genQueryOut = NULL;
    
    return (0);
}

int
rclOpenCollection (rcComm_t *conn, char *collection, int flags,
collHandle_t *collHandle)
{
    rodsObjStat_t *rodsObjStatOut = NULL;
    int status;

    if (conn == NULL || collection == NULL || collHandle == NULL) {
	rodsLog (LOG_ERROR,
          "rclOpenCollection: NULL conn, collection or collHandle input");
	return USER__NULL_INPUT_ERR;
    }

    status = rclInitQueryHandle (&collHandle->queryHandle, conn);
    
    if (status < 0) return status;

    memset (collHandle, 0, sizeof (collHandle_t));

    rstrcpy (collHandle->dataObjInp.objPath, collection, MAX_NAME_LEN);
    status = rcObjStat (conn, &collHandle->dataObjInp, &rodsObjStatOut);

    
    if (status < 0) return status;
	
    if (rodsObjStatOut->objType != COLL_OBJ_T) {
	free (rodsObjStatOut);
	return CAT_UNKNOWN_COLLECTION;
    }

    collHandle->dataObjInp.specColl = rodsObjStatOut->specColl;
    if (rodsObjStatOut->specColl != NULL &&
#if 0
      rodsObjStatOut->specColl->collClass == LINKED_COLL) {
#else
      rodsObjStatOut->specColl->collClass != STRUCT_FILE_COLL &&
      strlen (rodsObjStatOut->specColl->objPath) > 0) {
#endif
        /* save the linked path */
        rstrcpy (collHandle->linkedObjPath, rodsObjStatOut->specColl->objPath,
          MAX_NAME_LEN);
    };

    collHandle->rodsObjStat = rodsObjStatOut;

    collHandle->state = COLL_OPENED;
    collHandle->flags = flags;
    /* the collection exist. now query the data in it */
    status = rclInitQueryHandle (&collHandle->queryHandle, conn);

    if (status < 0) return status;

    return (0);
}

int
rclReadCollection (rcComm_t *conn, collHandle_t *collHandle,
collEnt_t *collEnt)
{
    int status;

    collHandle->queryHandle.conn = conn;	/* in case it changed */
    status = readCollection (collHandle, collEnt);

    return (status);
}

int
readCollection (collHandle_t *collHandle, collEnt_t *collEnt)
{
    int status = 0;
    collMetaInfo_t collMetaInfo;
    dataObjMetaInfo_t dataObjMetaInfo;
    queryHandle_t *queryHandle = &collHandle->queryHandle;

    if (queryHandle == NULL || collHandle == NULL || collEnt == NULL) {
        rodsLog (LOG_ERROR,
          "rclReadCollection: NULL queryHandle or collHandle input");
        return USER__NULL_INPUT_ERR;
    }

    memset (collEnt, 0, sizeof (collEnt_t));

    if (collHandle->state == COLL_CLOSED) return (CAT_NO_ROWS_FOUND);

    if ((collHandle->flags & DATA_QUERY_FIRST_FG) == 0) {
	/* recursive - coll first, dataObj second */
        if (collHandle->state == COLL_OPENED) {
	    status = genCollResInColl (queryHandle, collHandle);
	}

        if (collHandle->state == COLL_COLL_OBJ_QUERIED) {
            memset (&collMetaInfo, 0, sizeof (collMetaInfo));
            status = getNextCollMetaInfo (collHandle, collEnt);
            if (status >= 0) {
                return status;
            } else {
                if (status != CAT_NO_ROWS_FOUND) {
                    rodsLog (LOG_ERROR,
                      "rclReadCollection: getNextCollMetaInfo error for %s. status = %d",
                      collHandle->dataObjInp.objPath, status);
                }
                if (collHandle->dataObjInp.specColl == NULL) {
                    clearGenQueryInp (&collHandle->genQueryInp);
                }
            }
	    status = genDataResInColl (queryHandle, collHandle);
        }
        if (collHandle->state == COLL_DATA_OBJ_QUERIED) {
            memset (&dataObjMetaInfo, 0, sizeof (dataObjMetaInfo));
            status = getNextDataObjMetaInfo (collHandle, collEnt);

            if (status >= 0) {
                return status;
            } else {
                if (status != CAT_NO_ROWS_FOUND) {
                    rodsLog (LOG_ERROR,
                      "rclReadCollection: getNextDataObjMetaInfo error for %s. status = %d",
                      collHandle->dataObjInp.objPath, status);
                }
                /* cleanup */
                if (collHandle->dataObjInp.specColl == NULL) {
                    clearGenQueryInp (&collHandle->genQueryInp);
                }
                /* Nothing else to do. cleanup */
                collHandle->state = COLL_CLOSED;
            }
            return status;
        }
    } else {
        if (collHandle->state == COLL_OPENED) {
	    status = genDataResInColl (queryHandle, collHandle);
        }

        if (collHandle->state == COLL_DATA_OBJ_QUERIED) {
            memset (&dataObjMetaInfo, 0, sizeof (dataObjMetaInfo));
            status = getNextDataObjMetaInfo (collHandle, collEnt);

            if (status >= 0) {
                return status;
            } else {
                if (status != CAT_NO_ROWS_FOUND) {
                    rodsLog (LOG_ERROR,
                      "rclReadCollection: getNextDataObjMetaInfo error for %s. status = %d",
                      collHandle->dataObjInp.objPath, status);
                }
                /* cleanup */
                if (collHandle->dataObjInp.specColl == NULL) {
                    clearGenQueryInp (&collHandle->genQueryInp);
                }
            }

	    status = genCollResInColl (queryHandle, collHandle);
        }

        if (collHandle->state == COLL_COLL_OBJ_QUERIED) {
            memset (&collMetaInfo, 0, sizeof (collMetaInfo));
            status = getNextCollMetaInfo (collHandle, collEnt);
	    if (status < 0) {
                if (status != CAT_NO_ROWS_FOUND) {
                    rodsLog (LOG_ERROR,
                      "rclReadCollection: getNextCollMetaInfo error for %s. status = %d",
                      collHandle->dataObjInp.objPath, status);
                }
                /* cleanup */
                if (collHandle->dataObjInp.specColl == NULL) {
                    clearGenQueryInp (&collHandle->genQueryInp);
                }
                /* Nothing else to do. cleanup */
                collHandle->state = COLL_CLOSED;
            }
	    return status;
        }
    }
    return (CAT_NO_ROWS_FOUND);
}


int
genCollResInColl (queryHandle_t *queryHandle, collHandle_t *collHandle)
{
    genQueryOut_t *genQueryOut = NULL;
    int status = 0;

    /* query for sub-collections */
    if (collHandle->dataObjInp.specColl != NULL) {
        if (collHandle->dataObjInp.specColl->collClass == LINKED_COLL) {
            memset (&collHandle->genQueryInp, 0, sizeof (genQueryInp_t));
            status = queryCollInColl (queryHandle,
              collHandle->linkedObjPath, collHandle->flags & (~RECUR_QUERY_FG),
              &collHandle->genQueryInp, &genQueryOut);
        } else {
            if (strlen (collHandle->linkedObjPath) > 0) {
                rstrcpy (collHandle->dataObjInp.objPath, 
                  collHandle->linkedObjPath, MAX_NAME_LEN);
            }
            addKeyVal (&collHandle->dataObjInp.condInput,
              SEL_OBJ_TYPE_KW, "collection");
	    collHandle->dataObjInp.openFlags = 0;    /* start over */
            status = (*queryHandle->querySpecColl) (
	      (rcComm_t *) queryHandle->conn, &collHandle->dataObjInp, 
	      &genQueryOut);
	}
    } else {
        memset (&collHandle->genQueryInp, 0, sizeof (genQueryInp_t));
        status = queryCollInColl (queryHandle,
          collHandle->dataObjInp.objPath, collHandle->flags,
          &collHandle->genQueryInp, &genQueryOut);
    }

    collHandle->rowInx = 0;
    collHandle->state = COLL_COLL_OBJ_QUERIED;
    if (status >= 0) {
        status = genQueryOutToCollRes (&genQueryOut,
          &collHandle->collSqlResult);
    } else if (status != CAT_NO_ROWS_FOUND) {
        rodsLog (LOG_ERROR,
          "genCollResInColl: query collection error for %s. status = %d",
          collHandle->dataObjInp.objPath, status);
    }
    return status;
}

int
genDataResInColl (queryHandle_t *queryHandle, collHandle_t *collHandle)
{
    genQueryOut_t *genQueryOut = NULL;
    int status = 0;

    if (collHandle->dataObjInp.specColl != NULL) {
	if (collHandle->dataObjInp.specColl->collClass == LINKED_COLL) {
            memset (&collHandle->genQueryInp, 0, sizeof (genQueryInp_t));
            status = queryDataObjInColl (queryHandle,
              collHandle->linkedObjPath, collHandle->flags & (~RECUR_QUERY_FG),
              &collHandle->genQueryInp, &genQueryOut,
              &collHandle->dataObjInp.condInput);
	} else {
	    if (strlen (collHandle->linkedObjPath) > 0) {
		rstrcpy (collHandle->dataObjInp.objPath, 
		  collHandle->linkedObjPath, MAX_NAME_LEN);
	    }
            addKeyVal (&collHandle->dataObjInp.condInput,
              SEL_OBJ_TYPE_KW, "dataObj");
            status = (*queryHandle->querySpecColl) 
	      ((rcComm_t *) queryHandle->conn,
	      &collHandle->dataObjInp, &genQueryOut);
	}
    } else {
        memset (&collHandle->genQueryInp, 0, sizeof (genQueryInp_t));
        status = queryDataObjInColl (queryHandle,
          collHandle->dataObjInp.objPath, collHandle->flags,
          &collHandle->genQueryInp, &genQueryOut, 
	  &collHandle->dataObjInp.condInput);
    }

    collHandle->rowInx = 0;
    collHandle->state = COLL_DATA_OBJ_QUERIED;
    if (status >= 0) {
        status = genQueryOutToDataObjRes (&genQueryOut,
          &collHandle->dataObjSqlResult);
    } else if (status != CAT_NO_ROWS_FOUND) {
        rodsLog (LOG_ERROR,
          "genDataResInColl: query dataObj error for %s. status = %d",
          collHandle->dataObjInp.objPath, status);
    }
    return status;
}

int
rclCloseCollection (collHandle_t *collHandle)
{
    return (clearCollHandle (collHandle, 1));
}

int
clearCollHandle (collHandle_t *collHandle, int freeSpecColl)
{
    if (collHandle == NULL) return 0;
    if (collHandle->dataObjInp.specColl == NULL) {
        clearGenQueryInp (&collHandle->genQueryInp);
    }
    if (freeSpecColl != 0 && collHandle->dataObjInp.specColl != NULL) {
        free (collHandle->dataObjInp.specColl);
    }
    if (collHandle->rodsObjStat != NULL) {
	free (collHandle->rodsObjStat);
	collHandle->rodsObjStat = NULL;
    }
    clearKeyVal (&collHandle->dataObjInp.condInput);
    memset (&collHandle->dataObjInp, 0,  sizeof (dataObjInp_t));

    clearDataObjSqlResult (&collHandle->dataObjSqlResult);
    clearCollSqlResult (&collHandle->collSqlResult);

    collHandle->state = COLL_CLOSED;
    collHandle->rowInx = 0;

    return (0);
}

int
getNextCollMetaInfo (collHandle_t *collHandle, collEnt_t *outCollEnt)
{
    char *value;
    int len;
    char *collType, *collInfo1, *collInfo2;
    int status = 0;
    queryHandle_t *queryHandle = &collHandle->queryHandle;
    dataObjInp_t *dataObjInp =  &collHandle->dataObjInp;
    genQueryInp_t *genQueryInp = &collHandle->genQueryInp;
    collSqlResult_t *collSqlResult = &collHandle->collSqlResult;

    if (outCollEnt == NULL)
        return (USER__NULL_INPUT_ERR);

    memset (outCollEnt, 0, sizeof (collEnt_t));

    outCollEnt->objType = COLL_OBJ_T;
    if (collHandle->rowInx >= collSqlResult->rowCnt) {
        genQueryOut_t *genQueryOut = NULL;
        int continueInx = collSqlResult->continueInx;
        clearCollSqlResult (collSqlResult);

        if (continueInx > 0) {
            /* More to come */

            if (dataObjInp->specColl != NULL) {
                dataObjInp->openFlags = continueInx;
                status = (*queryHandle->querySpecColl) (
		  (rcComm_t *) queryHandle->conn, dataObjInp, &genQueryOut);
            } else {
                genQueryInp->continueInx = continueInx;
                status = (*queryHandle->genQuery) (
		  (rcComm_t *) queryHandle->conn, genQueryInp, &genQueryOut);
            }
            if (status < 0) {
                return (status);
            } else {
                status = genQueryOutToCollRes (&genQueryOut, collSqlResult);
                collHandle->rowInx = 0;
                free (genQueryOut);
            }
        } else {
            return (CAT_NO_ROWS_FOUND);
        }
    }
    value = collSqlResult->collName.value;
    len = collSqlResult->collName.len;
    outCollEnt->collName = &value[len * (collHandle->rowInx)];

    value = collSqlResult->collOwner.value;
    len = collSqlResult->collOwner.len;
    outCollEnt->ownerName = &value[len * (collHandle->rowInx)];

    value = collSqlResult->collCreateTime.value;
    len = collSqlResult->collCreateTime.len;
    outCollEnt->createTime = &value[len * (collHandle->rowInx)];

    value = collSqlResult->collModifyTime.value;
    len = collSqlResult->collModifyTime.len;
    outCollEnt->modifyTime = &value[len * (collHandle->rowInx)];

    value = collSqlResult->collType.value;
    len = collSqlResult->collType.len;
    collType = &value[len * (collHandle->rowInx)];

    if (*collType != '\0') {
        value = collSqlResult->collInfo1.value;
        len = collSqlResult->collInfo1.len;
        collInfo1 = &value[len * (collHandle->rowInx)];

        value = collSqlResult->collInfo2.value;
        len = collSqlResult->collInfo2.len;
        collInfo2 = &value[len * (collHandle->rowInx)];

        if (strcmp (collType, INHERIT_PAR_SPEC_COLL_STR) == 0) {
            if (dataObjInp->specColl == NULL) {
                rodsLog (LOG_ERROR,
                  "getNextCollMetaInfo: parent specColl is NULL for %s",
                  outCollEnt->collName);
                outCollEnt->specColl.collClass = NO_SPEC_COLL;
            } else {
                outCollEnt->specColl = *dataObjInp->specColl;
            }
            status = 0;
        } else {
            status = resolveSpecCollType (collType, outCollEnt->collName,
              collInfo1, collInfo2, &outCollEnt->specColl);
        }
    } else {
        outCollEnt->specColl.collClass = NO_SPEC_COLL;
        status = 0;
    }
    (collHandle->rowInx) ++;
    return (status);
}

int
getNextDataObjMetaInfo (collHandle_t *collHandle, collEnt_t *outCollEnt)
{
    int status;
    char *value;
    int len;
    int nextInx;
    char *replStatus, *dataId;
    int dataIdLen, replStatusLen;
    queryHandle_t *queryHandle = &collHandle->queryHandle;
    dataObjInp_t *dataObjInp =  &collHandle->dataObjInp;
    genQueryInp_t *genQueryInp = &collHandle->genQueryInp;
    dataObjSqlResult_t *dataObjSqlResult = &collHandle->dataObjSqlResult;
    rodsObjStat_t *rodsObjStat = collHandle->rodsObjStat;

    if (outCollEnt == NULL)
        return (USER__NULL_INPUT_ERR);

    memset (outCollEnt, 0, sizeof (collEnt_t));
    outCollEnt->objType = DATA_OBJ_T;
    if (collHandle->rowInx >= dataObjSqlResult->rowCnt) {
        genQueryOut_t *genQueryOut = NULL;
        int continueInx = dataObjSqlResult->continueInx;
        clearDataObjSqlResult (dataObjSqlResult);

        if (continueInx > 0) {
            /* More to come */

            if (dataObjInp->specColl != NULL) {
                dataObjInp->openFlags = continueInx;
                status = (*queryHandle->querySpecColl) (
		  (rcComm_t *) queryHandle->conn, dataObjInp, &genQueryOut);
            } else {
                genQueryInp->continueInx = continueInx;
                status = (*queryHandle->genQuery) (
		  (rcComm_t *) queryHandle->conn, genQueryInp, &genQueryOut);
            }
            if (status < 0) {
                return (status);
            } else {
                status = genQueryOutToDataObjRes (&genQueryOut,
                  dataObjSqlResult);
                collHandle->rowInx = 0;
                free (genQueryOut);
            }
        } else {
            return (CAT_NO_ROWS_FOUND);
        }
    }

    dataId = dataObjSqlResult->dataId.value;
    dataIdLen = dataObjSqlResult->dataId.len;
    replStatus = dataObjSqlResult->replStatus.value;
    replStatusLen = dataObjSqlResult->replStatus.len;

    if (strlen (dataId) > 0 && (collHandle->flags & NO_TRIM_REPL_FG) == 0) {
        int i;
        char *prevdataId = NULL;
        int gotCopy = 0;

        /* rsync type query ask for dataId. Others don't. Need to
         * screen out dup copies */

        for (i = collHandle->rowInx; i < dataObjSqlResult->rowCnt; i++) {
            if (prevdataId != NULL) {
                if (strcmp (prevdataId, &dataId[dataIdLen * i]) != 0) {
                    break;
                }
            } else {
                prevdataId = &dataId[dataIdLen * i];
                collHandle->rowInx = i;
            }

            if (gotCopy == 0 &&
              atoi (&replStatus[replStatusLen * i]) > 0) {
                collHandle->rowInx = i;
                gotCopy = 1;
            }
        }
        nextInx = i;
    } else {
        nextInx = (collHandle->rowInx) + 1;
    }

    value = dataObjSqlResult->collName.value;
    len = dataObjSqlResult->collName.len;
    outCollEnt->collName = &value[len * (collHandle->rowInx)];

    value = dataObjSqlResult->dataName.value;
    len = dataObjSqlResult->dataName.len;
    outCollEnt->dataName = &value[len * (collHandle->rowInx)];

    value = dataObjSqlResult->dataMode.value;
    len = dataObjSqlResult->dataMode.len;
    outCollEnt->dataMode = atoi (&value[len * (collHandle->rowInx)]);

    value = dataObjSqlResult->dataSize.value;
    len = dataObjSqlResult->dataSize.len;
    outCollEnt->dataSize = strtoll (&value[len * (collHandle->rowInx)], 0, 0);

    value = dataObjSqlResult->createTime.value;
    len = dataObjSqlResult->createTime.len;
    outCollEnt->createTime = &value[len * (collHandle->rowInx)];

    value = dataObjSqlResult->modifyTime.value;
    len = dataObjSqlResult->modifyTime.len;
    outCollEnt->modifyTime = &value[len * (collHandle->rowInx)];

    outCollEnt->dataId = &dataId[dataIdLen * (collHandle->rowInx)];

    outCollEnt->replStatus = atoi (&replStatus[replStatusLen * 
     (collHandle->rowInx)]);

    value = dataObjSqlResult->replNum.value;
    len = dataObjSqlResult->replNum.len;
    outCollEnt->replNum = atoi (&value[len * (collHandle->rowInx)]);

    value = dataObjSqlResult->chksum.value;
    len = dataObjSqlResult->chksum.len;
    outCollEnt->chksum = &value[len * (collHandle->rowInx)];

    if (rodsObjStat->specColl != NULL) {
	outCollEnt->specColl = *rodsObjStat->specColl;
    }
    if (rodsObjStat->specColl != NULL && 
      rodsObjStat->specColl->collClass != LINKED_COLL) {
	outCollEnt->resource = rodsObjStat->specColl->resource;
	outCollEnt->ownerName = rodsObjStat->ownerName;
	outCollEnt->replStatus = NEWLY_CREATED_COPY;
    } else {
        value = dataObjSqlResult->resource.value;
        len = dataObjSqlResult->resource.len;
        outCollEnt->resource = &value[len * (collHandle->rowInx)];
        value = dataObjSqlResult->ownerName.value;
        len = dataObjSqlResult->ownerName.len;
        outCollEnt->ownerName = &value[len * (collHandle->rowInx)];
    }

    value = dataObjSqlResult->phyPath.value;
    len = dataObjSqlResult->phyPath.len;
    outCollEnt->phyPath = &value[len * (collHandle->rowInx)];

    value = dataObjSqlResult->rescGrp.value;
    len = dataObjSqlResult->rescGrp.len;
    outCollEnt->rescGrp = &value[len * (collHandle->rowInx)];

    collHandle->rowInx = nextInx;
    return (0);
}

int
setQueryFlag (rodsArguments_t *rodsArgs)
{
    int queryFlags;

    if (rodsArgs->veryLongOption == True) {
        queryFlags = VERY_LONG_METADATA_FG;
    } else if (rodsArgs->longOption == True) {
        queryFlags = LONG_METADATA_FG;
    } else {
        queryFlags = 0;
    }

    return queryFlags;
}

int
rclInitQueryHandle (queryHandle_t *queryHandle, rcComm_t *conn)
{
    if (queryHandle == NULL || conn == NULL) {
        return (USER__NULL_INPUT_ERR);
    }

    queryHandle->conn = conn;
    queryHandle->connType = RC_COMM;
    queryHandle->querySpecColl = (funcPtr) rcQuerySpecColl;
    queryHandle->genQuery = (funcPtr) rcGenQuery;

    return (0);
}

int 
freeCollEnt (collEnt_t *collEnt)
{
    if (collEnt == NULL) return (0);

    clearCollEnt (collEnt);

    free (collEnt);

    return (0);
}

int
clearCollEnt (collEnt_t *collEnt)
{
    if (collEnt == NULL) return (0);

    if (collEnt->collName != NULL) free (collEnt->collName);
    if (collEnt->dataName != NULL) free (collEnt->dataName);
    if (collEnt->dataId != NULL) free (collEnt->dataId);
    if (collEnt->createTime != NULL) free (collEnt->createTime);
    if (collEnt->modifyTime != NULL) free (collEnt->modifyTime);
    if (collEnt->chksum != NULL) free (collEnt->chksum);
    if (collEnt->resource != NULL) free (collEnt->resource);
    if (collEnt->phyPath != NULL) free (collEnt->phyPath);
    if (collEnt->ownerName != NULL) free (collEnt->ownerName);
 
    return (0);
}


int
myChmod (char *inPath, uint dataMode)
{
    if (dataMode < 0100) return (0);

    if (Myumask == INIT_UMASK_VAL) {
        Myumask = umask (0022);
	umask (Myumask);
    }

    chmod (inPath, dataMode & 0777 & ~(Myumask));

    return (0);
}

char *
getZoneHintForGenQuery (genQueryInp_t *genQueryInp)
{
    char *zoneHint;
    int i;

    if (genQueryInp == NULL) return NULL;

    if ((zoneHint = getValByKey (&genQueryInp->condInput, ZONE_KW)) != NULL) 
	return (zoneHint);

    for (i = 0; i < genQueryInp->sqlCondInp.len; i++) {
	int inx = genQueryInp->sqlCondInp.inx[i];
#if 0	/* COL_DATA_NAME does not contain zone info */
	if (inx == COL_DATA_NAME ||
#endif
	if (inx == COL_COLL_NAME ||
	  inx == COL_COLL_PARENT_NAME ||
	  inx == COL_ZONE_NAME) {
	    char *tmpPtr;
	    zoneHint = genQueryInp->sqlCondInp.value[i];
	    if ((tmpPtr = strchr (zoneHint, '/')) != NULL) zoneHint = tmpPtr;
	    return (zoneHint);
	}
    }
    return NULL; 
}

/* getZoneType - get the ZoneType of inZoneName. icatZone is the icat
 * to query.
 */ 
int
getZoneType (rcComm_t *conn, char *icatZone, char *inZoneName,
char *outZoneType)
{
    genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut = NULL;
    int status;
    sqlResult_t *zoneType;
    char tmpStr[MAX_NAME_LEN];

    if (inZoneName == NULL || outZoneType == NULL)
        return (USER__NULL_INPUT_ERR);

    memset (&genQueryInp, 0, sizeof (genQueryInp));
    if (icatZone != NULL && strlen (icatZone) > 0) {
        addKeyVal (&genQueryInp.condInput, ZONE_KW, icatZone);
    }
    addInxIval (&genQueryInp.selectInp, COL_ZONE_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_ZONE_TYPE, 1);

    snprintf (tmpStr, MAX_NAME_LEN, " = '%s'", inZoneName);

    addInxVal (&genQueryInp.sqlCondInp, COL_ZONE_NAME, tmpStr);

    genQueryInp.maxRows = MAX_SQL_ROWS;

    status =  rcGenQuery (conn, &genQueryInp, &genQueryOut);

    if (status < 0)
        return status;

    if ((zoneType = getSqlResultByInx (genQueryOut, COL_ZONE_TYPE)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getZoneInfo: getSqlResultByInx for COL_ZONE_TYPE failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    rstrcpy (outZoneType, zoneType->value, MAX_NAME_LEN);

    freeGenQueryOut (&genQueryOut);

    return 0;
}

/* getCollSizeForProgStat - Calculate the totalNumFiles and totalFileSize and
 * put it in the operProgress struct.
 * Note that operProgress->totalNumFiles and operProgress->totalFileSize  
 * needs to be initialized since it can be a recursive operation and
 * cannot be initialized in this routine.
 */
int
getCollSizeForProgStat (rcComm_t *conn, char *srcColl, 
operProgress_t *operProgress)
{
    int status = 0;
    collHandle_t collHandle;
    collEnt_t collEnt;

    status = rclOpenCollection (conn, srcColl, RECUR_QUERY_FG,
      &collHandle);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "getCollSizeForProgStat: rclOpenCollection of %s error. status = %d",
          srcColl, status);
        return status;
    }

    while ((status = rclReadCollection (conn, &collHandle, &collEnt)) >= 0) {
        if (collEnt.objType == DATA_OBJ_T) {
	    operProgress->totalNumFiles++;
	    operProgress->totalFileSize += collEnt.dataSize;
        } else if (collEnt.objType == COLL_OBJ_T) {
            if (collEnt.specColl.collClass != NO_SPEC_COLL) {
                /* the child is a spec coll. need to drill down */
                status = getCollSizeForProgStat (conn, collEnt.collName, 
		  operProgress);
                if (status < 0 && status != CAT_NO_ROWS_FOUND) return (status);
            }
	}
    }
    if (status == CAT_NO_ROWS_FOUND) 
        return 0;
    else
        return status;
}

/* getDirSizeForProgStat - Calculate the totalNumFiles and totalFileSize and
 * put it in the operProgress struct.
 * Note that operProgress->totalNumFiles and operProgress->totalFileSize
 * needs to be initialized since it can be a recursive operation and
 * cannot be initialized in this routine.
 */
int
getDirSizeForProgStat (char *srcDir, operProgress_t *operProgress)
{
    int status = 0;
    DIR *dirPtr;
    struct dirent *myDirent;
#ifndef windows_platform
    struct stat statbuf;
#else
    struct irodsntstat statbuf;
#endif
    char srcChildPath[MAX_NAME_LEN];

    dirPtr = opendir (srcDir);
    if (dirPtr == NULL) {
        rodsLog (LOG_ERROR,
        "getDirSizeForProgStat: opendir local dir error for %s, errno = %d\n",
         srcDir, errno);
        return (USER_INPUT_PATH_ERR);
    }

    while ((myDirent = readdir (dirPtr)) != NULL) {
        if (strcmp (myDirent->d_name, ".") == 0 ||
          strcmp (myDirent->d_name, "..") == 0) {
            continue;
        }
        snprintf (srcChildPath, MAX_NAME_LEN, "%s/%s",
          srcDir, myDirent->d_name);

#ifndef windows_platform
        status = stat (srcChildPath, &statbuf);
#else
        status = iRODSNt_stat(srcChildPath, &statbuf);
#endif

        if (status != 0) {
            rodsLog (LOG_ERROR,
              "getDirSizeForProgStat: stat error for %s, errno = %d\n",
              srcChildPath, errno);
            closedir (dirPtr);
            return (USER_INPUT_PATH_ERR);
        }

        if (statbuf.st_mode & S_IFREG) {
            operProgress->totalNumFiles++;
            operProgress->totalFileSize += statbuf.st_size;
        } else if (statbuf.st_mode & S_IFDIR) {
            status = getDirSizeForProgStat (srcChildPath, operProgress);
            if (status < 0) return (status);

	}
    }
    return status;
}

/* iCommandProgStat - the irodsGuiProgressCallbak for icommand
 */
irodsGuiProgressCallbak 
iCommandProgStat (operProgress_t *operProgress)
{
    char myDir[MAX_NAME_LEN], myFile[MAX_NAME_LEN];
    int status;

    if (strchr (operProgress->curFileName, '/') == NULL) {
	/* relative path */
	rstrcpy (myFile, operProgress->curFileName, MAX_NAME_LEN);
    } else if ((status = splitPathByKey (operProgress->curFileName, 
      myDir, myFile, '/')) < 0) {
        rodsLogError (LOG_NOTICE, status,
          "iCommandProgStat: splitPathByKey for %s error, status = %d",
          operProgress->curFileName, status);
        return NULL;
    }

    if (operProgress->flag == 0) {
	printf (
          "%-lld/%-lld - %5.2f%% of files done   ", 
	  operProgress->totalNumFilesDone, operProgress->totalNumFiles, 
	  (float) operProgress->totalNumFilesDone/operProgress->totalNumFiles *
	  100.0);
	printf ("%-.3f/%-.3f MB - %5.2f%% of file sizes done\n",
	  (float) operProgress->totalFileSizeDone / 1048600.0,
	  (float) operProgress->totalFileSize / 1048600.0,
	  (float) operProgress->totalFileSizeDone/operProgress->totalFileSize *
	  100.0);
        printf ("Processing %s - %-.3f MB\n", myFile,
         (float) operProgress->curFileSize / 1048600.0);
    } else if (operProgress->flag == 1) {
        printf ("%s - %-.3f/%-.3f MB - %5.2f%% done\n", myFile,
         (float) operProgress->curFileSizeDone / 1048600.0,
         (float) operProgress->curFileSize / 1048600.0,
         (float) operProgress->curFileSizeDone/operProgress->curFileSize *
         100.0);
	/* done. don't print again */
	if (operProgress->curFileSizeDone == operProgress->curFileSize) {
	    operProgress->flag = 2;
	}
    }
    return NULL;
}

int
getOpenedCollLen (collHandle_t *collHandle)
{
    int len;

    if (collHandle->rodsObjStat->specColl != NULL &&
#if 0
      collHandle->rodsObjStat->specColl->collClass == LINKED_COLL) {
	len = strlen (collHandle->linkedObjPath);
#else
	(len = strlen (collHandle->linkedObjPath)) > 0) {
#endif
    } else {
	len = strlen (collHandle->dataObjInp.objPath);
    }
    return (len);
}

int
rmSubDir (char *mydir) 
{
    int status = 0;
    int savedStatus = 0;
    DIR *dirPtr;
    struct dirent *myDirent;
#ifndef windows_platform
    struct stat statbuf;
#else
    struct irodsntstat statbuf;
#endif
    char childPath[MAX_NAME_LEN];

    dirPtr = opendir (mydir);
    if (dirPtr == NULL) {
	status = USER_INPUT_PATH_ERR - errno;
        rodsLogError (LOG_ERROR, status,
        "rmSubDir: opendir local dir error for %s", mydir);
        return status;
    }
    while ((myDirent = readdir (dirPtr)) != NULL) {
        if (strcmp (myDirent->d_name, ".") == 0 ||
          strcmp (myDirent->d_name, "..") == 0) {
            continue;
	}
        snprintf (childPath, MAX_NAME_LEN, "%s/%s", mydir, myDirent->d_name);
#ifndef windows_platform
        status = stat (childPath, &statbuf);
#else
        status = iRODSNt_stat(childPath, &statbuf);
#endif
        if (status != 0) {
	    savedStatus = USER_INPUT_PATH_ERR - errno;
            rodsLogError (LOG_ERROR, status,
              "rmSubDir: stat error for %s", childPath);
	    continue;
        }
        if (statbuf.st_mode & S_IFDIR) {
	    status = rmSubDir (childPath);
	    if (status < 0) {
                savedStatus = USER_INPUT_PATH_ERR - errno;
                rodsLogError (LOG_ERROR, status,
                  "rmSubDir: rmSubDir error for %s ", childPath);
            }
	    if (rmdir (childPath) != 0) {
                savedStatus = USER_INPUT_PATH_ERR - errno;
                rodsLogError (LOG_ERROR, status,
                  "rmSubDir: rmdir error for %s ", childPath);
	    }
            continue;
        } else {
	    savedStatus = USER_INPUT_PATH_ERR - errno;
            rodsLogError (LOG_ERROR, status,
              "rmSubDir: %s is not a dir", childPath);
            continue;
	}
    }
    return savedStatus;
}


