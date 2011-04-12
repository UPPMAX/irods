/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#include "rcPortalOpr.h"
#include "dataObjOpen.h"
#include "dataObjWrite.h"
#include "dataObjRead.h"
#include "dataObjLseek.h"
#include "fileLseek.h"
#include "dataObjOpr.h"
#include "rodsLog.h"
#include "rcGlobalExtern.h"
#ifdef PARA_OPR
#include <pthread.h>
#endif

int
sendTranHeader (int sock, int oprType, int flags, rodsLong_t offset,
rodsLong_t length)
{
    transferHeader_t myHeader;
    int retVal;

    myHeader.oprType = htonl (oprType);
    myHeader.flags = htonl (flags);
    myHtonll (offset, (rodsLong_t *) &myHeader.offset);
    myHtonll (length, (rodsLong_t *) &myHeader.length);

    retVal = myWrite (sock, (void *) &myHeader, sizeof (myHeader), 
      SOCK_TYPE, NULL);

    if (retVal != sizeof (myHeader)) {
        rodsLog (LOG_ERROR,
         "sendTranHeader: toWrite = %d, written = %d",
          sizeof (myHeader), retVal);
        if (retVal < 0)
            return (retVal);
        else
            return (SYS_COPY_LEN_ERR);
    } else {
        return (0);
    }
}

int
rcvTranHeader (int sock, transferHeader_t *myHeader)
{
    int retVal;
    transferHeader_t tmpHeader;

    retVal = myRead (sock, (void *) &tmpHeader, sizeof (tmpHeader),
      SOCK_TYPE, NULL, NULL);

    if (retVal != sizeof (tmpHeader)) {
        rodsLog (LOG_ERROR,
         "rcvTranHeader: toread = %d, read = %d",
          sizeof (tmpHeader), retVal);
        if (retVal < 0)
            return (retVal);
        else
            return (SYS_COPY_LEN_ERR);
    }

    myHeader->oprType = htonl (tmpHeader.oprType);
    myHeader->flags = htonl (tmpHeader.flags);
    myNtohll (tmpHeader.offset, &myHeader->offset);
    myNtohll (tmpHeader.length, &myHeader->length);

    return (0);
}

int
fillBBufWithFile (rcComm_t *conn, bytesBuf_t *myBBuf, char *locFilePath, 
rodsLong_t dataSize)
{
    int in_fd, status;

    if (dataSize > 10 * MAX_SZ_FOR_SINGLE_BUF) {
	rodsLog (LOG_ERROR,
	  "fillBBufWithFile: dataSize %lld too large", dataSize);
	return (USER_FILE_TOO_LARGE);
    } else if (dataSize > MAX_SZ_FOR_SINGLE_BUF) {
        rodsLog (LOG_NOTICE,
          "fillBBufWithFile: dataSize %lld too large", dataSize);
    }

#ifdef windows_platform
	in_fd = iRODSNt_bopen(locFilePath, O_RDONLY,0);
#else
    in_fd = open (locFilePath, O_RDONLY, 0, FILE_DESC_TYPE, NULL);
#endif
    if (in_fd < 0) { /* error */
	status = USER_FILE_DOES_NOT_EXIST - errno;
	rodsLogError (LOG_ERROR, status,
	"cannot open file %s, status = %d", locFilePath, status);
	return (status);
    }
    

    myBBuf->buf = malloc (dataSize);
    myBBuf->len = dataSize;
    conn->transStat.bytesWritten = dataSize;

    status = myRead (in_fd, myBBuf->buf, (int) dataSize, FILE_DESC_TYPE,
      NULL, NULL);

    close (in_fd);

    return (status); 
}

int
putFileToPortal (rcComm_t *conn, portalOprOut_t *portalOprOut, 
char *locFilePath, char *objPath, rodsLong_t dataSize)
{
    portList_t *myPortList;
    int i, sock, in_fd;
    int numThreads; 
    rcPortalTransferInp_t myInput[MAX_NUM_CONFIG_TRAN_THR];
#ifdef PARA_OPR
    pthread_t tid[MAX_NUM_CONFIG_TRAN_THR];
#endif
    int retVal = 0;

    if (portalOprOut == NULL || portalOprOut->numThreads <= 0) {
        rodsLog (LOG_ERROR,
         "putFileToPortal: invalid portalOprOut");
        return (SYS_INVALID_PORTAL_OPR);
    }

    numThreads = portalOprOut->numThreads;

    myPortList = &portalOprOut->portList;

    if (portalOprOut->numThreads > MAX_NUM_CONFIG_TRAN_THR) {
        for (i = 0; i < portalOprOut->numThreads; i++) {
            sock = connectToRhostPortal (myPortList->hostAddr,
              myPortList->portNum, myPortList->cookie, myPortList->windowSize);
            if (sock > 0) {
                close (sock);
            }
        }
       rodsLog (LOG_ERROR,
         "putFileToPortal: numThreads %d too large", 
	 portalOprOut->numThreads);
        return (SYS_INVALID_PORTAL_OPR);
    }

    initFileRestart (conn, locFilePath, objPath, dataSize, 
      portalOprOut->numThreads);
#ifdef PARA_OPR
    memset (tid, 0, sizeof (tid));
#endif
    memset (myInput, 0, sizeof (myInput));

   if (numThreads == 1) {
        sock = connectToRhostPortal (myPortList->hostAddr, 
	  myPortList->portNum, myPortList->cookie, myPortList->windowSize);
        if (sock < 0) {
	    return (sock);
        }
#ifdef windows_platform
		in_fd = iRODSNt_bopen(locFilePath, O_RDONLY,0);
#else
        in_fd = open (locFilePath, O_RDONLY, 0);
#endif
        if (in_fd < 0) { /* error */
            retVal = USER_FILE_DOES_NOT_EXIST - errno;
            rodsLogError (LOG_ERROR, retVal,
             "cannot open file %s, status = %d", locFilePath, retVal);
            return (retVal);
        }
	fillRcPortalTransferInp (conn, &myInput[0], sock, in_fd, 0);
	rcPartialDataPut (&myInput[0]);
	if (myInput[0].status < 0) {
	    return (myInput[0].status);
	} else {
	    if (dataSize <= 0 || myInput[0].bytesWritten == dataSize) {
                if (conn->fileRestart.info.numSeg > 0) {     /* file restart */
                    clearLfRestartFile (&conn->fileRestart);
                }
		return (0);
	    } else {
		rodsLog (LOG_ERROR,
		  "putFileToPortal: bytesWritten %lld dataSize %lld mismatch",
		  myInput[0].bytesWritten, dataSize);
	        return (SYS_COPY_LEN_ERR);
	    }
	}
    } else {
#ifdef PARA_OPR
        rodsLong_t totalWritten = 0;

	for (i = 0; i < numThreads; i++) {
            sock = connectToRhostPortal (myPortList->hostAddr,
              myPortList->portNum, myPortList->cookie, myPortList->windowSize);
            if (sock < 0) {
                return (sock);
            }
            in_fd = open (locFilePath, O_RDONLY, 0);
            if (in_fd < 0) { 	/* error */
                retVal = USER_FILE_DOES_NOT_EXIST - errno;
                rodsLogError (LOG_ERROR, retVal,
                 "cannot open file %s, status = %d", locFilePath, retVal);
		continue;
            }
            fillRcPortalTransferInp (conn, &myInput[i], sock, in_fd, i);
            pthread_create (&tid[i], pthread_attr_default,
             (void *(*)(void *)) rcPartialDataPut, (void *) &myInput[i]);
        }
	if (retVal < 0)
	    return (retVal);

        for ( i = 0; i < numThreads; i++) {
            if (tid[i] != 0) {
                pthread_join (tid[i], NULL);
	    }
	    totalWritten += myInput[i].bytesWritten;
            if (myInput[i].status < 0) {
                retVal = myInput[i].status;
	    }
        }
        if (retVal < 0) {
	    return (retVal);
        } else {
	    if (dataSize <= 0 || totalWritten == dataSize) { 
                if (conn->fileRestart.info.numSeg > 0) {     /* file restart */
                    clearLfRestartFile (&conn->fileRestart);
                }
                if (gGuiProgressCB != NULL) 
		    gGuiProgressCB (&conn->operProgress);
                return (0);
            } else {
                rodsLog (LOG_ERROR,
                  "putFileToPortal: totalWritten %lld dataSize %lld mismatch",
                  totalWritten, dataSize);
                return (SYS_COPY_LEN_ERR);
            }
        }
#else   /* PARA_OPR */
        return (SYS_PARA_OPR_NO_SUPPORT);
#endif  /* PARA_OPR */
    }
}

int
fillRcPortalTransferInp (rcComm_t *conn, rcPortalTransferInp_t *myInput, 
int destFd, int srcFd, int threadNum)
{
    if (myInput == NULL)
        return (SYS_INTERNAL_NULL_INPUT_ERR);

    myInput->conn = conn;
    myInput->destFd = destFd;
    myInput->srcFd = srcFd;
    myInput->threadNum = threadNum;

    return (0);
}

void
rcPartialDataPut (rcPortalTransferInp_t *myInput)
{
    transferHeader_t myHeader;
    int destFd;
    int srcFd;
    void *buf;
    transferStat_t *myTransStat;
    rodsLong_t curOffset = 0;
    rcComm_t *conn;
    fileRestartInfo_t *info;
    int threadNum;

#ifdef PARA_DEBUG
    printf ("rcPartialDataPut: thread %d at start\n", myInput->threadNum);
#endif
    if (myInput == NULL) {
	rodsLog (LOG_ERROR,
	 "rcPartialDataPut: NULL input");
	return;
    }
    conn = myInput->conn;
    info = &conn->fileRestart.info;
    threadNum = myInput->threadNum;

    myTransStat = &myInput->conn->transStat;

    destFd = myInput->destFd;
    srcFd = myInput->srcFd;

    buf = malloc (TRANS_BUF_SZ);

    myInput->bytesWritten = 0;

    if (gGuiProgressCB != NULL) {
        conn->operProgress.flag = 1;
    }

    while (myInput->status >= 0) {
	rodsLong_t toPut;

        myInput->status = rcvTranHeader (destFd, &myHeader);

#ifdef PARA_DEBUG
        printf ("rcPartialDataPut: thread %d after rcvTranHeader\n", 
          myInput->threadNum);
#endif

        if (myInput->status < 0) {
	    break;
        }

	if (myHeader.oprType == DONE_OPR) {
	    break;
        }
	if (myHeader.offset != curOffset) {
	    curOffset = myHeader.offset;
	    if (lseek (srcFd, curOffset, SEEK_SET) < 0) {
		myInput->status = UNIX_FILE_LSEEK_ERR - errno;
		rodsLogError (LOG_ERROR, myInput->status,
		  "rcPartialDataPut: lseek to %lld error, status = %d",
		  curOffset, myInput->status);
		break;
	    }
	    if (info->numSeg > 0)       /* file restart */
                info->dataSeg[threadNum].offset = curOffset;
	}

	toPut = myHeader.length;
	while (toPut > 0) {
	    int toRead, bytesRead, bytesWritten;

	    if (toPut > TRANS_BUF_SZ) {
		toRead = TRANS_BUF_SZ;
	    } else {
		toRead = toPut;
	    } 

	    bytesRead = myRead (srcFd, buf, toRead, FILE_DESC_TYPE, 
	      &bytesRead, NULL);
	    if (bytesRead != toRead) {
		myInput->status = SYS_COPY_LEN_ERR - errno;
		rodsLogError (LOG_ERROR, myInput->status,
		  "rcPartialDataPut: toPut %lld, bytesRead %d",
		  toPut, bytesRead);   
		break;
	    }
	    bytesWritten = myWrite (destFd, buf, bytesRead, SOCK_TYPE,
	      &bytesWritten);

	    if (bytesWritten != bytesRead) {
                myInput->status = SYS_COPY_LEN_ERR - errno;
		rodsLogError (LOG_ERROR, myInput->status,
                  "rcPartialDataPut: toWrite %d, bytesWritten %d, errno = %d",
                  bytesRead, bytesWritten, errno);
                break;
	    }
	    toPut -= bytesWritten;
	    if (info->numSeg > 0) {     /* file restart */
		info->dataSeg[threadNum].len += bytesWritten;
		conn->fileRestart.writtenSinceUpdated += bytesWritten;
                if (threadNum == 0 && conn->fileRestart.writtenSinceUpdated >= 
		  RESTART_FILE_UPDATE_SIZE) {
		    int status;
                    /* time to write to the restart file */
                    status = writeLfRestartFile (conn->fileRestart.infoFile,
                      &conn->fileRestart.info);
                    if (status < 0) {
                        rodsLog (LOG_ERROR,
                         "putFile: writeLfRestartFile for %s, status = %d",
                         conn->fileRestart.info.fileName, status);
                    }
                    conn->fileRestart.writtenSinceUpdated = 0;
                }
	    }
	}
	curOffset += myHeader.length;
	myInput->bytesWritten += myHeader.length;
	/* should lock this. But window browser is the only one using it */ 
	myTransStat->bytesWritten += myHeader.length;
        /* should lock this. but it is info only */
        if (gGuiProgressCB != NULL) {
            conn->operProgress.curFileSizeDone += myHeader.length;
            if (myInput->threadNum == 0) gGuiProgressCB (&conn->operProgress);
        }
    }

    free (buf);
    close (srcFd);
    mySockClose (destFd);
}

int
putFile (rcComm_t *conn, int l1descInx, char *locFilePath, char *objPath,
rodsLong_t dataSize)
{
    int in_fd, status;
    bytesBuf_t dataObjWriteInpBBuf;
    openedDataObjInp_t dataObjWriteInp;
    int bytesWritten;
    rodsLong_t totalWritten = 0;
    int bytesRead;
    int progressCnt = 0;
    fileRestartInfo_t *info = &conn->fileRestart.info;
    rodsLong_t lastUpdateSize = 0;


#ifdef windows_platform
	in_fd = iRODSNt_bopen(locFilePath, O_RDONLY,0);
#else
    in_fd = open (locFilePath, O_RDONLY, 0);
#endif
    if (in_fd < 0) { /* error */
        status = USER_FILE_DOES_NOT_EXIST - errno;
        rodsLogError (LOG_ERROR, status,
        "cannot open file %s, status = %d", locFilePath, status);
        return (status);
    }

    bzero (&dataObjWriteInp, sizeof (dataObjWriteInp));
    dataObjWriteInpBBuf.buf = malloc (TRANS_BUF_SZ);
    dataObjWriteInpBBuf.len = 0;
    dataObjWriteInp.l1descInx = l1descInx;
    initFileRestart (conn, locFilePath, objPath, dataSize, 1);

    if (gGuiProgressCB != NULL) conn->operProgress.flag = 1;

    while ((dataObjWriteInpBBuf.len =
      myRead (in_fd, dataObjWriteInpBBuf.buf, TRANS_BUF_SZ, FILE_DESC_TYPE,
      &bytesRead, NULL)) > 0) {
        /* Write to the data object */

        dataObjWriteInp.len = dataObjWriteInpBBuf.len;
        bytesWritten = rcDataObjWrite (conn, &dataObjWriteInp,
          &dataObjWriteInpBBuf);
        if (bytesWritten < dataObjWriteInp.len) {
           rodsLog (LOG_ERROR,
	    "putFile: Read %d bytes, Wrote %d bytes.\n ",
            dataObjWriteInp.len, bytesWritten);
	    free (dataObjWriteInpBBuf.buf);
	    close (in_fd);
	    return (SYS_COPY_LEN_ERR);
        } else {
            totalWritten += bytesWritten;
	    conn->transStat.bytesWritten = totalWritten;
	    if (info->numSeg > 0) {	/* file restart */
	        info->dataSeg[0].len += bytesWritten;
		if (totalWritten - lastUpdateSize >= RESTART_FILE_UPDATE_SIZE) {
		    /* time to write to the restart file */
		    status = writeLfRestartFile (conn->fileRestart.infoFile,
		      &conn->fileRestart.info);
		    if (status < 0) {
                        rodsLog (LOG_ERROR,
                         "putFile: writeLfRestartFile for %s, status = %d",
                         locFilePath, status);
                         free (dataObjWriteInpBBuf.buf);
                         close (in_fd);
                         return status;
		    }
		    lastUpdateSize = totalWritten;
		}
	    }
            if (gGuiProgressCB != NULL) {
                if (progressCnt >= (MAX_PROGRESS_CNT - 1)) {
                    conn->operProgress.curFileSizeDone +=
                    ((MAX_PROGRESS_CNT - 1) * TRANS_BUF_SZ + bytesWritten);
                    gGuiProgressCB (&conn->operProgress);
                    progressCnt = 0;
                } else {
                    progressCnt ++;
                }
            }
	}
    }

    free (dataObjWriteInpBBuf.buf);
    close (in_fd);

    if (dataSize <= 0 || totalWritten == dataSize) {
        if (info->numSeg > 0) {     /* file restart */
            clearLfRestartFile (&conn->fileRestart);
        }
        if (gGuiProgressCB != NULL) {
            conn->operProgress.curFileSizeDone = conn->operProgress.curFileSize;
            gGuiProgressCB (&conn->operProgress);
        }
        return (0);
    } else {
        rodsLog (LOG_ERROR,
          "putFile: totalWritten %lld dataSize %lld mismatch",
          totalWritten, dataSize);
        return (SYS_COPY_LEN_ERR);
    }
}

int
getIncludeFile (rcComm_t *conn, bytesBuf_t *dataObjOutBBuf, char *locFilePath)
{
    int status, out_fd, bytesWritten;

    if (strcmp (locFilePath, STDOUT_FILE_NAME) == 0) {
	if (dataObjOutBBuf->len <= 0) {
	    return (0);
	}
	bytesWritten = fwrite (dataObjOutBBuf->buf, dataObjOutBBuf->len,
	  1, stdout);
	if (bytesWritten == 1)
	    bytesWritten = dataObjOutBBuf->len;
    } else { 
#ifdef windows_platform
		out_fd = iRODSNt_bopen(locFilePath, O_WRONLY | O_CREAT | O_TRUNC, 0640);
#else
        out_fd = open (locFilePath, O_WRONLY | O_CREAT | O_TRUNC, 0640);
#endif
        if (out_fd < 0) { /* error */
            status = USER_FILE_DOES_NOT_EXIST - errno;
            rodsLogError (LOG_ERROR, status,
            "cannot open file %s, status = %d", locFilePath, status);
            return (status);
        }

        if (dataObjOutBBuf->len <= 0) {
	    close (out_fd);
            return 0;
        } 

        bytesWritten = myWrite (out_fd, dataObjOutBBuf->buf, 
	  dataObjOutBBuf->len, FILE_DESC_TYPE, NULL);

        close (out_fd);
    }
    if (bytesWritten != dataObjOutBBuf->len) {
       rodsLog (LOG_ERROR,
        "getIncludeFile: Read %d bytes, Wrote %d bytes. errno = %d\n ",
        dataObjOutBBuf->len, bytesWritten, errno);
        return (SYS_COPY_LEN_ERR);
    } else {
	conn->transStat.bytesWritten = bytesWritten;
        return (0);
    }
}

int
getFile (rcComm_t *conn, int l1descInx, char *locFilePath,
rodsLong_t dataSize)
{
    int out_fd, status;
    bytesBuf_t dataObjReadInpBBuf;
    openedDataObjInp_t dataObjReadInp;
    int bytesWritten, bytesRead;
    rodsLong_t totalWritten = 0;
    int progressCnt = 0;

    if (strcmp (locFilePath, STDOUT_FILE_NAME) == 0) {
	/* streaming to stdout */
        out_fd =1;
    } else {
#ifdef windows_platform
		out_fd = iRODSNt_bopen(locFilePath, O_WRONLY | O_CREAT | O_TRUNC, 0640);
#else
        out_fd = open (locFilePath, O_WRONLY | O_CREAT | O_TRUNC, 0640);
#endif
    }
 
    if (out_fd < 0) { /* error */
        status = USER_FILE_DOES_NOT_EXIST - errno;
        rodsLogError (LOG_ERROR, status,
        "cannot open file %s, status = %d", locFilePath, status);
        return (status);
    }

    bzero (&dataObjReadInp, sizeof (dataObjReadInp));
    dataObjReadInpBBuf.buf = malloc (TRANS_BUF_SZ);
    dataObjReadInpBBuf.len = dataObjReadInp.len = TRANS_BUF_SZ;
    dataObjReadInp.l1descInx = l1descInx;

    if (gGuiProgressCB != NULL) conn->operProgress.flag = 1;

    while ((bytesRead = rcDataObjRead (conn, &dataObjReadInp, 
      &dataObjReadInpBBuf)) > 0) {

	if (out_fd == 1) {
            bytesWritten = fwrite (dataObjReadInpBBuf.buf, bytesRead,
              1, stdout);
            if (bytesWritten == 1)
		bytesWritten = bytesRead;
	} else {
            bytesWritten = myWrite (out_fd, dataObjReadInpBBuf.buf, 
	      bytesRead, FILE_DESC_TYPE, NULL);
	}

        if (bytesWritten != bytesRead) {
           rodsLog (LOG_ERROR,
            "getFile: Read %d bytes, Wrote %d bytes.\n ",
            bytesRead, bytesWritten);
            free (dataObjReadInpBBuf.buf);
	    if (out_fd != 1) 
                close (out_fd);
            return (SYS_COPY_LEN_ERR);
        } else {
            totalWritten += bytesWritten;
	    conn->transStat.bytesWritten = totalWritten;
	    if (gGuiProgressCB != NULL) {
		if (progressCnt >= (MAX_PROGRESS_CNT - 1)) {
		    conn->operProgress.curFileSizeDone += 
		    ((MAX_PROGRESS_CNT - 1) * TRANS_BUF_SZ + bytesWritten);
		    gGuiProgressCB (&conn->operProgress);
		    progressCnt = 0;
		} else {
		    progressCnt ++;
		}
	    }
        }
    }

    free (dataObjReadInpBBuf.buf);
    if (out_fd != 1)
        close (out_fd);

    if (dataSize <= 0 || totalWritten == dataSize) {
        if (gGuiProgressCB != NULL) {
            conn->operProgress.curFileSizeDone = conn->operProgress.curFileSize;
	    gGuiProgressCB (&conn->operProgress);
	}
        return (0);
    } else {
        rodsLog (LOG_ERROR,
          "getFile: totalWritten %lld dataSize %lld mismatch",
          totalWritten, dataSize);
        return (SYS_COPY_LEN_ERR);
    }
}

int
getFileFromPortal (rcComm_t *conn, portalOprOut_t *portalOprOut, 
char *locFilePath, rodsLong_t dataSize)
{
    portList_t *myPortList;
    int i, sock, out_fd;
    int numThreads;
    rcPortalTransferInp_t myInput[MAX_NUM_CONFIG_TRAN_THR];
#ifdef PARA_OPR
    pthread_t tid[MAX_NUM_CONFIG_TRAN_THR];
#endif
    int retVal = 0;

    if (portalOprOut == NULL || portalOprOut->numThreads <= 0) {
        rodsLog (LOG_ERROR,
         "getFileFromPortal: invalid portalOprOut");
        return (SYS_INVALID_PORTAL_OPR);
    }

    numThreads = portalOprOut->numThreads;

    myPortList = &portalOprOut->portList;

    if (portalOprOut->numThreads > MAX_NUM_CONFIG_TRAN_THR) {
	/* drain the connection or it will be stuck */
	for (i = 0; i < numThreads; i++) {
            sock = connectToRhostPortal (myPortList->hostAddr,
              myPortList->portNum, myPortList->cookie, myPortList->windowSize);
	    if (sock > 0) {
		close (sock);
	    }
	}
        rodsLog (LOG_ERROR,
         "getFileFromPortal: numThreads %d too large", numThreads);
        return (SYS_INVALID_PORTAL_OPR);
    }

#ifdef PARA_OPR
    memset (tid, 0, sizeof (tid));
#endif
    memset (myInput, 0, sizeof (myInput));

   if (numThreads == 1) {
        sock = connectToRhostPortal (myPortList->hostAddr,
          myPortList->portNum, myPortList->cookie, myPortList->windowSize);
        if (sock < 0) {
            return (sock);
        }
#ifdef windows_platform
		out_fd = iRODSNt_bopen(locFilePath, O_WRONLY | O_CREAT | O_TRUNC, 0640);
#else
        out_fd = open (locFilePath, O_WRONLY | O_CREAT | O_TRUNC, 0640);
#endif
        if (out_fd < 0) { /* error */
	    retVal = USER_FILE_DOES_NOT_EXIST - errno;
            rodsLogError (LOG_ERROR, retVal,
            "cannot open file %s, status = %d", locFilePath, retVal);
            return (retVal);
        }
        fillRcPortalTransferInp (conn, &myInput[0], out_fd, sock, 0640);
        rcPartialDataGet (&myInput[0]);
        if (myInput[0].status < 0) {
            return (myInput[0].status);
        } else {
            if (dataSize <= 0 || myInput[0].bytesWritten == dataSize) {
                return (0);
            } else {
                rodsLog (LOG_ERROR,
                  "getFileFromPortal:bytesWritten %lld dataSize %lld mismatch",
                  myInput[0].bytesWritten, dataSize);
                return (SYS_COPY_LEN_ERR);
            }
        }
    } else {
#ifdef PARA_OPR
        rodsLong_t totalWritten = 0;

        for (i = 0; i < numThreads; i++) {
            sock = connectToRhostPortal (myPortList->hostAddr,
              myPortList->portNum, myPortList->cookie, myPortList->windowSize);
            if (sock < 0) {
                return (sock);
            }
	    if (i == 0) { 
                out_fd = open (locFilePath, O_WRONLY | O_CREAT | O_TRUNC,0640);
	    } else {
                out_fd = open (locFilePath, O_WRONLY, 0640);
	    }
            if (out_fd < 0) {    /* error */
                retVal = USER_FILE_DOES_NOT_EXIST - errno;
                rodsLogError (LOG_ERROR, retVal,
                "cannot open file %s, status = %d", locFilePath, retVal);
		CLOSE_SOCK (sock);
		continue;
            }
            fillRcPortalTransferInp (conn, &myInput[i], out_fd, sock, i);
            pthread_create (&tid[i], pthread_attr_default,
             (void *(*)(void *)) rcPartialDataGet, (void *) &myInput[i]);
        }

	if (retVal < 0) {
	    return (retVal);
	}

        for ( i = 0; i < numThreads; i++) {
            if (tid[i] != 0) {
                pthread_join (tid[i], NULL);
            }
            totalWritten += myInput[i].bytesWritten;
            if (myInput[i].status < 0) {
                retVal = myInput[i].status;
            }
        }
        if (retVal < 0) {
            return (retVal);
        } else {
            if (dataSize <= 0 || totalWritten == dataSize) {
                if (gGuiProgressCB != NULL)
                    gGuiProgressCB (&conn->operProgress);
                return (0);
            } else {
                rodsLog (LOG_ERROR,
                  "getFileFromPortal: totalWritten %lld dataSize %lld mismatch",
                  totalWritten, dataSize);
                return (SYS_COPY_LEN_ERR);
            }
        }
#else   /* PARA_OPR */
        return (SYS_PARA_OPR_NO_SUPPORT);
#endif  /* PARA_OPR */
    }
}

void
rcPartialDataGet (rcPortalTransferInp_t *myInput)
{
    transferHeader_t myHeader;
    int destFd;
    int srcFd;
    void *buf;
    transferStat_t *myTransStat;
    rodsLong_t curOffset = 0;
    rcComm_t *conn;

#ifdef PARA_DEBUG
    printf ("rcPartialDataGet: thread %d at start\n", myInput->threadNum);
#endif
    if (myInput == NULL) {
        rodsLog (LOG_ERROR,
         "rcPartialDataGet: NULL input");
        return;
    }

    myTransStat = &myInput->conn->transStat;

    destFd = myInput->destFd;
    srcFd = myInput->srcFd;

    buf = malloc (TRANS_BUF_SZ);

    myInput->bytesWritten = 0;

    if (gGuiProgressCB != NULL) {
	conn = myInput->conn;
	conn->operProgress.flag = 1;
    }

    while (myInput->status >= 0) {
        rodsLong_t toGet;

        myInput->status = rcvTranHeader (srcFd, &myHeader);

#ifdef PARA_DEBUG
        printf ("rcPartialDataGet: thread %d after rcvTranHeader\n",
          myInput->threadNum);
#endif

        if (myInput->status < 0) {
            break;
        }

        if (myHeader.oprType == DONE_OPR) {
            break;
        }
        if (myHeader.offset != curOffset) {
            curOffset = myHeader.offset;
            if (lseek (destFd, curOffset, SEEK_SET) < 0) {
                myInput->status = UNIX_FILE_LSEEK_ERR - errno;
                rodsLogError (LOG_ERROR, myInput->status,
                  "rcPartialDataGet: lseek to %lld error, status = %d",
                  curOffset, myInput->status);
                break;
            }
        }

        toGet = myHeader.length;
        while (toGet > 0) {
            int toRead, bytesRead, bytesWritten;

            if (toGet > TRANS_BUF_SZ) {
                toRead = TRANS_BUF_SZ;
            } else {
                toRead = toGet;
            }

            bytesRead = myRead (srcFd, buf, toRead, SOCK_TYPE, &bytesRead, 
	      NULL);
            if (bytesRead != toRead) {
                myInput->status = SYS_COPY_LEN_ERR - errno;
                rodsLogError (LOG_ERROR, myInput->status,
                  "rcPartialDataGet: toGet %lld, bytesRead %d",
                  toGet, bytesRead);
                break;
            }
            bytesWritten = myWrite (destFd, buf, bytesRead, FILE_DESC_TYPE,
	      &bytesWritten);

            if (bytesWritten != bytesRead) {
                myInput->status = SYS_COPY_LEN_ERR - errno;
                rodsLogError (LOG_ERROR, myInput->status,
                  "rcPartialDataGet: toWrite %d, bytesWritten %d",
                  bytesRead, bytesWritten);
                break;
            }
            toGet -= bytesWritten;
        }
        curOffset += myHeader.length;
        myInput->bytesWritten += myHeader.length;
        /* should lock this. But window browser is the only one using it */
        myTransStat->bytesWritten += myHeader.length;
	/* should lock this. but it is info only */
	if (gGuiProgressCB != NULL) {
	    conn->operProgress.curFileSizeDone += myHeader.length;
	    if (myInput->threadNum == 0) gGuiProgressCB (&conn->operProgress);
	}
    }

    free (buf);
    close (destFd);
    CLOSE_SOCK (srcFd);
}

#ifdef RBUDP_TRANSFER
/* putFileToPortalRbudp - The client side of putting a file using 
 * Rbudp. If locFilePath is NULL, the local file has already been opned
 * and locFd should be used. If sendRate and packetSize are 0, it will 
 * try to set it based on env and default.
 */
int
putFileToPortalRbudp (portalOprOut_t *portalOprOut, char *locFilePath, 
char *objPath, int locFd, rodsLong_t dataSize, int veryVerbose,
int sendRate, int packetSize)
{
    portList_t *myPortList;
    int status;
    rbudpSender_t rbudpSender;
    int mysendRate, mypacketSize;
    char *tmpStr;

    if (portalOprOut == NULL || portalOprOut->numThreads != 1) {
        rodsLog (LOG_ERROR,
         "putFileToPortal: invalid portalOprOut");
        return (SYS_INVALID_PORTAL_OPR);
    }

    myPortList = &portalOprOut->portList;

    bzero (&rbudpSender, sizeof (rbudpSender));
    status = initRbudpClient (&rbudpSender.rbudpBase, myPortList);
    if (status < 0) {
        rodsLog (LOG_ERROR,
         "putFileToPortalRbudp: initRbudpClient error for %s", 
	  myPortList->hostAddr);
        return (status);
    }
    rbudpSender.rbudpBase.verbose = veryVerbose;
    if (sendRate <= 0) {
        if ((tmpStr = getenv (RBUDP_SEND_RATE_KW)) != NULL) {
	    mysendRate = atoi (tmpStr);
        } else {
	    mysendRate = DEF_UDP_SEND_RATE;
	}
    } else {
	mysendRate = sendRate;
    }
    if (packetSize <= 0) {
        if ((tmpStr = getenv (RBUDP_PACK_SIZE_KW)) != NULL) {
	    mypacketSize = atoi (tmpStr);
        } else {
	    mypacketSize = DEF_UDP_PACKET_SIZE;
	}
    } else {
	mypacketSize = packetSize;
    }

    if (locFilePath == NULL) {
        status = sendfileByFd (&rbudpSender, mysendRate, mypacketSize,
          locFd);
    } else {
        status = rbSendfile (&rbudpSender, mysendRate, mypacketSize, 
          locFilePath);
    }

    sendClose (&rbudpSender);
    if (status < 0) {
        rodsLog (LOG_ERROR,
         "putFileToPortalRbudp: sendfile error for %s", 
	  myPortList->hostAddr);
        return (status);
    }
    return (status);
}

/* getFileToPortalRbudp - The client side of getting a file using 
 * Rbudp. If locFilePath is NULL, the local file has already been opned
 * and locFd should be used. If sendRate and packetSize are 0, it will 
 * try to set it based on env and default.
 */
int
getFileToPortalRbudp (portalOprOut_t *portalOprOut, 
char *locFilePath, int locFd, rodsLong_t dataSize, int veryVerbose,
int packetSize)
{
    portList_t *myPortList;
    int status;
    rbudpReceiver_t rbudpReceiver;
    int mypacketSize;
    char *tmpStr;

    if (portalOprOut == NULL || portalOprOut->numThreads != 1) {
        rodsLog (LOG_ERROR,
         "getFileToPortalRbudp: invalid portalOprOut");
        return (SYS_INVALID_PORTAL_OPR);
    }

    myPortList = &portalOprOut->portList;

    bzero (&rbudpReceiver, sizeof (rbudpReceiver));
    status = initRbudpClient (&rbudpReceiver.rbudpBase, myPortList);
    if (status < 0) {
        rodsLog (LOG_ERROR,
         "getFileToPortalRbudp: initRbudpClient error for %s", 
	  myPortList->hostAddr);
        return (status);
    }
    rbudpReceiver.rbudpBase.verbose = veryVerbose;

    if (packetSize <= 0) {
        if ((tmpStr = getenv (RBUDP_PACK_SIZE_KW)) != NULL) {
            mypacketSize = atoi (tmpStr);
        } else {
            mypacketSize = DEF_UDP_PACKET_SIZE;
        }
    } else {
        mypacketSize = packetSize;
    }

    if (locFilePath == NULL) {
        status = getfileByFd (&rbudpReceiver, locFd, mypacketSize);
    } else {
        status = getfile (&rbudpReceiver, NULL, locFilePath, mypacketSize);
    }

    recvClose (&rbudpReceiver);
    if (status < 0) {
        rodsLog (LOG_ERROR,
         "getFileToPortalRbudp: getfile error for %s", 
	  myPortList->hostAddr);
        return (status);
    }
    return (status);
}

int
initRbudpClient (rbudpBase_t *rbudpBase, portList_t *myPortList)
{ 
    int  tcpSock;
    int tcpPort, udpPort;
    int status;
    struct sockaddr_in localUdpAddr;
    int udpLocalPort;

    if ((udpPort = getUdpPortFromPortList (myPortList)) == 0) {
        rodsLog (LOG_ERROR,
         "putFileToPortalRbudp: udpPort == 0");
        return (SYS_INVALID_PORTAL_OPR);
    }
   
    tcpPort = getTcpPortFromPortList (myPortList);

    tcpSock = connectToRhostPortal (myPortList->hostAddr,
      tcpPort, myPortList->cookie, myPortList->windowSize);
    if (tcpSock < 0) {
        return (tcpSock);
    }

    rbudpBase->udpSockBufSize = UDPSOCKBUF;
    rbudpBase->tcpPort = tcpPort;
    rbudpBase->tcpSockfd = tcpSock;
    rbudpBase->hasTcpSock = 0;	/* so it will close properly */
    rbudpBase->udpRemotePort = udpPort;

    /* connect to the server's UDP port */
    status = passiveUDP (rbudpBase, myPortList->hostAddr);

    if (status < 0) {
        rodsLog (LOG_ERROR,
         "initRbudpClient: passiveUDP connect to %s error. status = %d", 
	  myPortList->hostAddr, status);
        return (SYS_UDP_CONNECT_ERR + status);
    }

    /* inform the server of the UDP port */
    rbudpBase->udpLocalPort = 
      setLocalAddr (rbudpBase->udpSockfd, &localUdpAddr);
    if (rbudpBase->udpLocalPort < 0) 
        return rbudpBase->udpLocalPort;
    udpLocalPort = htonl (rbudpBase->udpLocalPort);
    status = writen (rbudpBase->tcpSockfd, (char *) &udpLocalPort, 
      sizeof (udpLocalPort));
    if (status != sizeof (udpLocalPort)) {
        rodsLog (LOG_ERROR,
         "initRbudpClient: writen error. towrite %d, bytes written %d ",
          sizeof (udpLocalPort), status);
        return (SYS_UDP_CONNECT_ERR);
    }

    return 0;
}
#endif  /* RBUDP_TRANSFER */

int
initFileRestart (rcComm_t *conn, char *fileName, char *objPath,
rodsLong_t fileSize, int numThr)
{
    fileRestart_t *fileRestart = &conn->fileRestart;
    fileRestartInfo_t *info = &fileRestart->info;

    if (fileRestart->flags != FILE_RESTART_ON || 
      fileSize < MIN_RESTART_SIZE || numThr <= 0) {
	info->numSeg = 0;	/* indicate no restart */
	return 0;
    }
    if (numThr > MAX_NUM_CONFIG_TRAN_THR) {
        rodsLog (LOG_NOTICE,
         "initFileRestart: input numThr %d larger than max %d ",
          numThr, MAX_NUM_CONFIG_TRAN_THR);
	info->numSeg = 0;	/* indicate no restart */
	return 0;
    }
    info->numSeg = numThr;
    info->fileSize = fileSize;
    rstrcpy (info->fileName, fileName, MAX_NAME_LEN);
    rstrcpy (info->objPath, objPath, MAX_NAME_LEN);
    bzero (info->dataSeg, sizeof (dataSeg_t) * MAX_NUM_CONFIG_TRAN_THR);
    return 0;
}

int
writeLfRestartFile (char *infoFile, fileRestartInfo_t *info)
{
    bytesBuf_t *packedBBuf = NULL;
    int status, fd;

    status =  packStruct ((void *) info, &packedBBuf,
      "FileRestartInfo_PI", RodsPackTable, 0, XML_PROT);
    if (status < 0) {
        rodsLog (LOG_ERROR,
         "writeLfRestartFile: packStruct error for %s, status = %d",
          info->fileName, status);
	return status;
    }
    /* write it to a file */
    fd = open (infoFile, O_CREAT|O_TRUNC|O_WRONLY, 0640);
    if (fd < 0) {
        status = UNIX_FILE_OPEN_ERR - errno;
        rodsLog (LOG_ERROR,
          "writeLfRestartFile: open failed for %s, status = %d",
          infoFile, status);
        return (status);
    }

    status = write (fd, packedBBuf->buf, packedBBuf->len);
    close (fd);

    if (packedBBuf != NULL) {
        clearBBuf (packedBBuf);
        free (packedBBuf);
    }
    if (status < 0) {
        status = UNIX_FILE_WRITE_ERR - errno;
        rodsLog (LOG_ERROR,
          "writeLfRestartFile: write failed for %s, status = %d",
          infoFile, status);
        return (status);
    }
    return status;
}

int
readLfRestartFile (char *infoFile, fileRestartInfo_t **info)
{
    int status, fd;
    struct stat statbuf;
    char *buf;

    *info = NULL;
    status = stat (infoFile, &statbuf);
    if (status < 0) {
        status = UNIX_FILE_STAT_ERR - errno;
        return (status);
    }
    if ( statbuf.st_size == 0) {
        status = UNIX_FILE_STAT_ERR - errno;
        rodsLog (LOG_ERROR,
          "readLfRestartFile restart infoFile size is 0 for %s",
          infoFile);
        return (status);
    }
    /* read the restart infoFile */
    fd = open (infoFile, O_RDONLY, 0640);
    if (fd < 0) {
        status = UNIX_FILE_OPEN_ERR - errno;
        rodsLog (LOG_ERROR,
          "readLfRestartFile open failed for %s, status = %d",
          infoFile, status);
        return (status);
    }

    buf = (char *) calloc (1, 2 * statbuf.st_size);
    if (buf == NULL) {
	close (fd);
        return SYS_MALLOC_ERR;
    }
    status = read (fd, buf, statbuf.st_size);
    if (status != statbuf.st_size) {
        rodsLog (LOG_ERROR,
          "readLfRestartFile error failed for %s, toread %d, read %d",
          infoFile, statbuf.st_size, status);
        status = UNIX_FILE_READ_ERR - errno;
        close (fd);
	free (buf);
        return (status);
    }
    close (fd);

    status =  unpackStruct (buf, (void **) info, "FileRestartInfo_PI", 
      NULL, XML_PROT);
    if (status < 0) {
        rodsLog (LOG_ERROR,
         "readLfRestartFile: unpackStruct error for %s, status = %d",
          infoFile, status);
    }
    close (fd);
    free (buf);
    return (status);
}


int
clearLfRestartFile (fileRestart_t *fileRestart)
{
    unlink (fileRestart->infoFile);
    bzero (&fileRestart->info, sizeof (fileRestartInfo_t));

    return 0;
}

int
lfRestartPutWithInfo (rcComm_t *conn, fileRestartInfo_t *info)
{
    rodsLong_t curOffset = 0;
    bytesBuf_t dataObjWriteInpBBuf;
    int status, i;
    int localFd, irodsFd;
    dataObjInp_t dataObjOpenInp;
    openedDataObjInp_t dataObjWriteInp;
    openedDataObjInp_t dataObjLseekInp;
    openedDataObjInp_t dataObjCloseInp;
    fileLseekOut_t *dataObjLseekOut = NULL;
    int writtenSinceUpdated = 0;
    rodsLong_t gap;

#ifdef windows_platform
    localFd = iRODSNt_bopen(info->fileName, O_RDONLY,0);
#else
    localFd = open (info->fileName, O_RDONLY, 0);
#endif
    if (localFd < 0) { /* error */
        status = USER_FILE_DOES_NOT_EXIST - errno;
        rodsLogError (LOG_ERROR, status,
        "cannot open file %s, status = %d", info->fileName, status);
        return (status);
    }

    bzero (&dataObjOpenInp, sizeof (dataObjOpenInp));
    rstrcpy (dataObjOpenInp.objPath, info->objPath, MAX_NAME_LEN);
    dataObjOpenInp.openFlags = O_WRONLY;
    addKeyVal (&dataObjOpenInp.condInput, FORCE_FLAG_KW, "");

    irodsFd = rcDataObjOpen (conn, &dataObjOpenInp);
    if (irodsFd < 0) { /* error */
        rodsLogError (LOG_ERROR, status,
        "cannot open target file %s, status = %d", info->objPath, status);
	close (localFd);
        return (status);
    }

    bzero (&dataObjWriteInp, sizeof (dataObjWriteInp));
    dataObjWriteInpBBuf.buf = malloc (TRANS_BUF_SZ);
    dataObjWriteInpBBuf.len = 0;
    dataObjWriteInp.l1descInx = irodsFd;

    memset (&dataObjLseekInp, 0, sizeof (dataObjLseekInp));
    dataObjLseekInp.whence = SEEK_SET;
    for (i = 0; i < info->numSeg; i++) {
	gap = info->dataSeg[i].offset - curOffset;
	if (gap < 0) {
	    /* should not be here */
	} else if (gap > 0) {
	    rodsLong_t tmpLen, *lenToUpdate;
	    if (i == 0) {
		/* should not be here */
		tmpLen = 0;
		lenToUpdate = &tmpLen;
	    } else {
		lenToUpdate = &info->dataSeg[i - 1].len;
	    }
	    status = putSeg (conn, gap, localFd, &dataObjWriteInp, 
             &dataObjWriteInpBBuf, TRANS_BUF_SZ, &writtenSinceUpdated,
	     info, lenToUpdate);
	    if (status < 0) break;
	    curOffset += gap;
	}
	if (info->dataSeg[i].len > 0) {
	    curOffset += info->dataSeg[i].len;
            if (lseek (localFd, curOffset, SEEK_SET) < 0) {
                status = UNIX_FILE_LSEEK_ERR - errno;
                rodsLogError (LOG_ERROR, status,
                  "lfRestartWithInfo: lseek to %lld error for %s",
                  curOffset, info->fileName);
                break;
            }
            dataObjLseekInp.l1descInx = irodsFd;
            dataObjLseekInp.offset = curOffset;
            status = rcDataObjLseek (conn, &dataObjLseekInp, &dataObjLseekOut);
            if (status < 0) {
                rodsLogError (LOG_ERROR, status,
                  "lfRestartWithInfo: rcDataObjLseek to %lld error for %s",
                  curOffset, info->objPath);
                break;
	    } else {
		if (dataObjLseekOut != NULL) free (dataObjLseekOut);
	    }
	}
    }
    if (status >= 0) {
        gap = info->fileSize - curOffset;
        if (gap > 0) {
            status = putSeg (conn, gap, localFd, &dataObjWriteInp, 
              &dataObjWriteInpBBuf, TRANS_BUF_SZ,  &writtenSinceUpdated,
             info, &info->dataSeg[i - 1].len);
	}
    }
    free (dataObjWriteInpBBuf.buf);
    close (localFd);
    memset (&dataObjCloseInp, 0, sizeof (dataObjCloseInp));
    dataObjCloseInp.l1descInx = irodsFd;
    rcDataObjClose (conn, &dataObjCloseInp);
    return status;
}

int
putSeg (rcComm_t *conn, rodsLong_t segSize, int localFd, 
openedDataObjInp_t *dataObjWriteInp, bytesBuf_t *dataObjWriteInpBBuf, 
int bufLen, int *writtenSinceUpdated, fileRestartInfo_t *info, 
rodsLong_t *dataSegLen)
{
    rodsLong_t gap = segSize;
    int bytesWritten;
    int status;

    while (gap > 0) {
        int toRead;
        if (gap > bufLen) {
            toRead = bufLen;
        } else {
            toRead = (int) gap;
        }

        dataObjWriteInpBBuf->len = myRead (localFd,
          dataObjWriteInpBBuf->buf, toRead, FILE_DESC_TYPE, NULL, NULL);
        /* Write to the data object */
        dataObjWriteInp->len = dataObjWriteInpBBuf->len;
        bytesWritten = rcDataObjWrite (conn, dataObjWriteInp,
          dataObjWriteInpBBuf);
        if (bytesWritten < dataObjWriteInp->len) {
           rodsLog (LOG_ERROR,
            "putFile: Read %d bytes, Wrote %d bytes.\n ",
            dataObjWriteInp->len, bytesWritten);
            return (SYS_COPY_LEN_ERR);
        } else {
            gap -= toRead;
	    *writtenSinceUpdated += toRead;
	    *dataSegLen += toRead;
	    if (*writtenSinceUpdated >= RESTART_FILE_UPDATE_SIZE) {
		status = writeLfRestartFile (conn->fileRestart.infoFile, info);
                if (status < 0) {
                    rodsLog (LOG_ERROR,
                     "putSeg: writeLfRestartFile for %s, status = %d",
                     info->fileName, status);
                     return status;
                }
		*writtenSinceUpdated = 0;
	    }
	}
    }
    return 0;
}

