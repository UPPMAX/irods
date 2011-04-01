/*** Copyright (c) 2010 Data Intensive Cyberinfrastructure Foundation. All rights reserved.    ***
 *** For full copyright notice please refer to files in the COPYRIGHT directory                ***/
/* Written by Jean-Yves Nief of CCIN2P3 and copyright assigned to Data Intensive Cyberinfrastructure Foundation */

#include "rodsPath.h"
#include "rodsErrorTable.h"
#include "rodsLog.h"
#include "scanUtil.h"
#include "miscUtil.h"

int
scanObj (rcComm_t *conn, rodsArguments_t *myRodsArgs, rodsPathInp_t *rodsPathInp, char hostname[LONG_NAME_LEN])
{
	char inpPath[LONG_NAME_LEN] = "";
	char *inpPathO, *lastChar;
	struct stat sbuf;
	int lenInpPath, status;
	
	if ( rodsPathInp->numSrc == 1 ) {
		inpPathO = rodsPathInp->srcPath[0].outPath;
		if ( rodsPathInp->srcPath[0].objType == LOCAL_FILE_T || \
			rodsPathInp->srcPath[0].objType == LOCAL_DIR_T ) {
			status = lstat(inpPathO, &sbuf);
			if ( status == 0 ) {
				/* remove any trailing "/" from inpPathO */
				lenInpPath = strlen(inpPathO);
				lastChar = strrchr(inpPathO, '/');
				if ( strlen(lastChar) == 1 ) {
					lenInpPath = lenInpPath - 1;
				}
				strncpy(inpPath, inpPathO, lenInpPath);
				if ( S_ISDIR(sbuf.st_mode) == 1 ) {   /* check if it is not included into a mounted collection */
					status = checkIsMount(conn, inpPath);
					if ( status != 0 ) {  /* if it is part of a mounted collection, abort */
						printf("The directory %s or one of its subdirectories to be scanned is declared as being \
used for a mounted collection: abort!\n", inpPath);
						return (status);
					}
				}
				status = scanObjDir(conn, myRodsArgs, inpPath, hostname);
			}
			else {
				status = USER_INPUT_PATH_ERR;
				rodsLog (LOG_ERROR, "scanObj: %s does not exist", inpPathO);
			}
		}
		else if ( rodsPathInp->srcPath[0].objType == UNKNOWN_OBJ_T || 
			rodsPathInp->srcPath[0].objType == COLL_OBJ_T ) {
			status = scanObjCol(conn, myRodsArgs, inpPathO);
		}
	}
	else {
		rodsLog (LOG_ERROR, "scanObj: gave %i input source path, should give one and only one", rodsPathInp->numSrc);
		status = USER_INPUT_PATH_ERR;
	}
	
	return (status);
}

int 
scanObjDir (rcComm_t *conn, rodsArguments_t *myRodsArgs, char *inpPath, char *hostname)
{
	DIR *dirPtr;
	struct dirent *myDirent;
	struct stat sbuf;
	int status;
	char fullPath[LONG_NAME_LEN] = "\0";
	
	dirPtr = opendir (inpPath);
	/* check if it is a directory */
	lstat(inpPath, &sbuf);
	if ( S_ISDIR(sbuf.st_mode) == 1 ) {
		if ( dirPtr == NULL ) {
			return (-1);
		}
	}
	else {
		status = chkObjExist(conn, inpPath, hostname);
		return (status);
	}
	
	while ( ( myDirent = readdir(dirPtr) ) != NULL ) {
        if ( strcmp(myDirent->d_name, ".") == 0 || strcmp(myDirent->d_name, "..") == 0 ) {
			continue;
        }
        strcpy(fullPath, inpPath);
        strcat(fullPath, "/");
        strcat(fullPath, myDirent->d_name);
        lstat(fullPath, &sbuf);
		if ( S_ISDIR(sbuf.st_mode) == 1 ) {
			if ( myRodsArgs->recursive == True ) {
				status = scanObjDir(conn, myRodsArgs, fullPath, hostname);
			}
        }
        else {
			status = chkObjExist(conn, fullPath, hostname);
        }
	}
   
   return (status);
	
}

int 
scanObjCol (rcComm_t *conn, rodsArguments_t *myRodsArgs, char *inpPath)
{
	int isColl, status;
	genQueryInp_t genQueryInp1, genQueryInp2;
    genQueryOut_t *genQueryOut1 = NULL, *genQueryOut2 = NULL;
	char condStr1[MAX_NAME_LEN], condStr2[MAX_NAME_LEN], *lastPart;
	char firstPart[MAX_NAME_LEN] = "";

	/* check if inpPath is a file or a collection */
	lastPart = strrchr(inpPath, '/') + 1;
	strncpy(firstPart, inpPath, strlen(inpPath) - strlen(lastPart) - 1);
	memset (&genQueryInp1, 0, sizeof (genQueryInp1));
	addInxIval(&genQueryInp1.selectInp, COL_COLL_ID, 1);
	genQueryInp1.maxRows = MAX_SQL_ROWS;
	
	snprintf (condStr1, MAX_NAME_LEN, "='%s'", firstPart);
    addInxVal (&genQueryInp1.sqlCondInp, COL_COLL_NAME, condStr1);
	snprintf (condStr1, MAX_NAME_LEN, "='%s'", lastPart);
    addInxVal (&genQueryInp1.sqlCondInp, COL_DATA_NAME, condStr1);
	
	status =  rcGenQuery (conn, &genQueryInp1, &genQueryOut1);
	if (status == CAT_NO_ROWS_FOUND) {
		isColl = 1;
	}
	else {
		isColl = 0;
	}
	
	/* for each files check if the physical file associated to it exists on the 
       physical resource */
	memset (&genQueryInp2, 0, sizeof (genQueryInp2));
	addInxIval(&genQueryInp2.selectInp, COL_D_DATA_PATH, 1);
	addInxIval(&genQueryInp2.selectInp, COL_R_LOC, 1);
	addInxIval(&genQueryInp2.selectInp, COL_R_ZONE_NAME, 1);
	addInxIval(&genQueryInp2.selectInp, COL_DATA_NAME, 1);
	addInxIval(&genQueryInp2.selectInp, COL_COLL_NAME, 1);
	genQueryInp2.maxRows = MAX_SQL_ROWS;
	
	if ( isColl ) {
		if ( myRodsArgs->recursive == True ) {
			snprintf (condStr2, MAX_NAME_LEN, "like '%s%s'", inpPath, "%");
		}
		else {
			snprintf (condStr2, MAX_NAME_LEN, "='%s'", inpPath);
		}
		addInxVal (&genQueryInp2.sqlCondInp, COL_COLL_NAME, condStr2);
	}
	else {
		snprintf (condStr2, MAX_NAME_LEN, "='%s'", firstPart);
		addInxVal (&genQueryInp2.sqlCondInp, COL_COLL_NAME, condStr2);
		snprintf (condStr2, MAX_NAME_LEN, "='%s'", lastPart);
		addInxVal (&genQueryInp2.sqlCondInp, COL_DATA_NAME, condStr2);
	}
	
	/* check if the physical file corresponding to the iRODS object does exist */
	status =  rcGenQuery (conn, &genQueryInp2, &genQueryOut2);
	if (status == 0) {
		statPhysFile(conn, genQueryOut2);
	}
	while ( status == 0 && genQueryOut2->continueInx > 0) {
		genQueryInp2.continueInx=genQueryOut2->continueInx;
		status = rcGenQuery(conn, &genQueryInp2, &genQueryOut2);
		if (status == 0) {
			statPhysFile(conn, genQueryOut2);
		}
	}
	
	freeGenQueryOut(&genQueryOut1);
	freeGenQueryOut(&genQueryOut2);
	
	return(status);
	
}

int
statPhysFile (rcComm_t *conn, genQueryOut_t *genQueryOut2)
{
	int i, rc;
	char *dataPath, *loc, *zone, *dataName, *collName;
	sqlResult_t *dataPathStruct, *locStruct, *zoneStruct,
	 *dataNameStruct, *collNameStruct;
	fileStatInp_t fileStatInp;
	rodsStat_t *fileStatOut;
	
	fileStatInp.fileType = UNIX_FILE_TYPE;
	for (i=0;i<genQueryOut2->rowCnt;i++) {
		dataPathStruct = getSqlResultByInx (genQueryOut2, COL_D_DATA_PATH);
		locStruct = getSqlResultByInx (genQueryOut2, COL_R_LOC);
		zoneStruct = getSqlResultByInx (genQueryOut2, COL_R_ZONE_NAME);
		dataNameStruct = getSqlResultByInx (genQueryOut2, COL_DATA_NAME);
		collNameStruct = getSqlResultByInx (genQueryOut2, COL_COLL_NAME);
		dataPath = &dataPathStruct->value[dataPathStruct->len*i];
		loc = &locStruct->value[locStruct->len*i];
		zone = &zoneStruct->value[zoneStruct->len*i];
		dataName = &dataNameStruct->value[dataNameStruct->len*i];
		collName = &collNameStruct->value[collNameStruct->len*i];
		
		/* check if the physical file does exist on the filesystem */
		rstrcpy (fileStatInp.addr.hostAddr, loc, NAME_LEN);
		rstrcpy (fileStatInp.addr.zoneName, zone, NAME_LEN);
		rstrcpy (fileStatInp.fileName, dataPath, MAX_NAME_LEN);
		rc = rcFileStat (conn, &fileStatInp, &fileStatOut);
		if ( rc != 0 ) {
			printf("Physical file %s on server %s is missing, corresponding to \
iRODS object %s/%s \n", dataPath, loc, collName, dataName);
		}
			
	}
	
	return (rc);

}

int
chkObjExist (rcComm_t *conn, char *inpPath, char *hostname)
{
	int status;
	genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut = NULL;
	char condStr[MAX_NAME_LEN];

	memset (&genQueryInp, 0, sizeof (genQueryInp));
	addInxIval(&genQueryInp.selectInp, COL_D_DATA_ID, 1);
	genQueryInp.maxRows = 0;
/*
 Use the AUTO_CLOSE option to close down the statement after the
 query, avoiding later 'too many concurrent statements' errors (and
 CAT_SQL_ERR: -806000) later.  This could also be done by asking for 2
 rows (maxRows), but the rows are not needed, just the status.
 This may also fix a segfault error which might be related.
 */
	genQueryInp.options = AUTO_CLOSE;
	
	snprintf (condStr, MAX_NAME_LEN, "='%s'", inpPath);
    addInxVal (&genQueryInp.sqlCondInp, COL_D_DATA_PATH, condStr);
    snprintf (condStr, MAX_NAME_LEN, "like '%s%s' || ='%s'", hostname, "%", hostname);
	addInxVal (&genQueryInp.sqlCondInp, COL_R_LOC, condStr);
	
	status =  rcGenQuery (conn, &genQueryInp, &genQueryOut);
	if (status == CAT_NO_ROWS_FOUND) {
	    printf ("%s tagged as orphan file\n", inpPath);
	}
	
	clearGenQueryInp(&genQueryInp);
	freeGenQueryOut(&genQueryOut);
	
	return (status);
	
}

int 
checkIsMount (rcComm_t *conn, char *inpPath)
{
	int i, minLen, status, status1;
	genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut = NULL;
	char condStr[MAX_NAME_LEN], *dirMPath;
	
	memset (&genQueryInp, 0, sizeof (genQueryInp));
	addInxIval(&genQueryInp.selectInp, COL_COLL_INFO1, 1);
	genQueryInp.maxRows = MAX_SQL_ROWS;

	snprintf (condStr, MAX_NAME_LEN, "='%s'", "mountPoint");
    addInxVal (&genQueryInp.sqlCondInp, COL_COLL_TYPE, condStr);
	
	status1 = rcGenQuery (conn, &genQueryInp, &genQueryOut);
	if (status1 == CAT_NO_ROWS_FOUND) {       
		status = 0;  /* there is no mounted collection, so no potential problem */
	}
	else {  /* check if inpPath is part of one of the mounted collections */
		status = 0;
		for (i = 0; i < genQueryOut->rowCnt; i++) {
			dirMPath = genQueryOut->sqlResult[0].value;
			dirMPath += i*genQueryOut->sqlResult[0].len;
			if ( strlen(dirMPath) <= strlen(inpPath) ) {
				minLen = strlen(dirMPath);
			}
			else {
				minLen = strlen(inpPath);
			}
			if (strncmp(dirMPath, inpPath, minLen) == 0) { 
				status = -1;
			}
		}
	}
	
	clearGenQueryInp(&genQueryInp);
	freeGenQueryOut(&genQueryOut);
	
	return (status);
	
}
