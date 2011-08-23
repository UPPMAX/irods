/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* 
 * iput - The irods put utility
*/

#include "rodsClient.h"
#include "parseCommandLine.h"
#include "rodsPath.h"
#include "putUtil.h"
void usage ();

int
main(int argc, char **argv) {
    int status;
    rodsEnv myEnv;
    rErrMsg_t errMsg;
    rcComm_t *conn;
    rodsArguments_t myRodsArgs;
    char *optStr;
    rodsPathInp_t rodsPathInp;
    int reconnFlag;
    

    optStr = "abD:fhIkKn:N:p:PrR:QTvVX:Z";
   
    status = parseCmdLineOpt (argc, argv, optStr, 1, &myRodsArgs);

    if (status < 0) {
	printf("use -h for help.\n");
        exit (1);
    }

    if (myRodsArgs.help==True) {
       usage();
       exit(0);
    }

    status = getRodsEnv (&myEnv);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status, "main: getRodsEnv error. ");
        exit (1);
    }

    status = parseCmdLinePath (argc, argv, optind, &myEnv,
      UNKNOWN_FILE_T, UNKNOWN_OBJ_T, 0, &rodsPathInp);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status, "main: parseCmdLinePath error. "); 
	printf("use -h for help.\n");
        exit (1);
    }

    if (myRodsArgs.reconnect == True) {
        reconnFlag = RECONN_TIMEOUT;
    } else {
        reconnFlag = NO_RECONN;
    }

    conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
      myEnv.rodsZone, reconnFlag, &errMsg);

    if (conn == NULL) {
        exit (2);
    }
   
    status = clientLogin(conn);
    if (status != 0) {
       rcDisconnect(conn);
        exit (7);
    }

    if (myRodsArgs.progressFlag == True) {
        gGuiProgressCB = (irodsGuiProgressCallbak) iCommandProgStat;
    }

    status = putUtil (&conn, &myEnv, &myRodsArgs, &rodsPathInp);

    rcDisconnect(conn);

    if (status < 0) {
	exit (3);
    } else {
        exit(0);
    }

}

void 
usage ()
{
   char *msgs[]={
"Usage : iput [-abfIkKPQrTUvV] [-D dataType] [-N numThreads] [-n replNum]",
"             [-p physicalPath] [-R resource] [-X restartFile] [--link]", 
"             [--lfrestart lfRestartFile] [--retries count]",
"		localSrcFile|localSrcDir ...  destDataObj|destColl",
"Usage : iput [-abfIkKPQTUvV] [-D dataType] [-N numThreads] [-n replNum] ",
"             [-p physicalPath] [-R resource] [-X restartFile] [--link]",
"             [--lfrestart lfRestartFile] [--retries count]",
"               localSrcFile",
" ",
"Store a file into iRODS.  If the destination data-object or collection are",
"not provided, the current irods directory and the input file name are used.",
" ",
"The -X option specifies that the restart option is on and the operation",
"is restartable. The restartFile input specifies a local file that contains",
"the restart info. If the restartFile does not exist, it will be created", 
"and used for recording subsequent restart info. If it exists and is not", 
"empty, the restart info contained in this file will be used to restart", 
"the operation. Note that the operation is not restarted automatically", 
"when it failed. Another iput -X run must be made to continue from where", 
"it left off using the restart info. But the -X option can be used in", 
"conjunction with the --retries option to automatically restart the operation",
"in case of failure. Also note that the restart operation only works for",
"uploading directories and the path input must be identical to the one",
"that generated the restart file",
" ",
"The --lfrestart option specifies that the large file restart option is on",
"and the lfRestartFile input specifies a local file that contains the restart",
"info. Currently, only files larger than 32 Mbytes will be restarted.",
"The --lfrestart option can be used together with the -X option to do large",
"file transfer restart as part of the overall directory upload restart.",
" ",
"If the options -f is used to overwrite an existing data-object, the copy",
"in the resource specified by the -R option will be picked if it exists.",
"Otherwise, one of the copy in the other resources will be picked for the",
"overwrite. Note that a copy will not be made in the specified resource",
"if a copy in the specified resource does not already exist. The irepl",
"command should be used to make a replica of an existing copy.", 
" ",
"The -I option specifies the redirection of the connection so that it can",
"be connected directly to the resource server. This option can improve",
"the performance of uploading a large number of small (<32 Mbytes) files.", 
"This option is only effective if the source is a directory and the -f ",
"option is not used", 
" ",
"The -Q option specifies the use of the RBUDP transfer mechanism which uses",
"the UDP protocol for data transfer. The UDP protocol is very efficient",
"if the network is very robust with few packet losses. Two environment",
"variables - rbudpSendRate and rbudpPackSize are used to tune the RBUDP",
"data transfer. rbudpSendRate is used to throttle the send rate in ",
"kbits/sec. The default rbudpSendRate is 600,000. rbudpPackSize is used",
"to set the packet size. The dafault rbudpPackSize is 8192. The -V option", 
"can be used to show the loss rate of the transfer. If the lost rate is", 
"more than a few %, the sendrate should be reduced.",
" ",
"The -T option will renew the socket connection between the client and ",
"server after 10 minutes of connection. This gets around the problem of",
"sockets getting timed out by the firewall as reported by some users.",
" ",
"The -b option specifies bulk upload operation which can do up to 50 uploads",
"at a time to reduce overhead. If the -b is specified with the -f option ",
"to overwrite existing files, the operation will work only if there is no",
"existing copy at all or if there is an existing copy in the target resource.",
"The operation will fail if there are existing copies but not in the",
"target resource because this type of operation requires a replication",
"operation and bulk replication has not been implemented yet.",
"The bulk option does work for mounted collections which may represent the",
"quickest way to upload a large number of small files.",
" ",
"Options are:",
" -a  all - update all existing copy",
" -b  bulk upload to reduce overhead",
" -D  dataType - the data type string",
" -f  force - write data-object even it exists already; overwrite it",
" -I  redirect connection - redirect the connection to connect directly",
"       to the resource server.",
" -k  checksum - calculate a checksum on the data",
" -K  verify checksum - calculate and verify the checksum on the data",
" --link - ignore symlink.",
" -N  numThreads - the number of thread to use for the transfer. A value of",
"       0 means no threading. By default (-N option not used) the server ",
"       decides the number of threads to use.",
" -p physicalPath - the physical path of the uploaded file on the sever ",
" -P  output the progress of the upload.",
" -Q  use RBUDP (datagram) protocol for the data transfer",
" -R  resource - specifies the resource to store to. This can also be specified",
"     in your environment or via a rule set up by the administrator.",
" -r  recursive - store the whole subdirectory",
" -T  renew socket connection after 10 minutes",
" -v  verbose",
" -V  Very verbose",
" -X  restartFile - specifies that the restart option is on and the",
"     restartFile input specifies a local file that contains the restart info.",
"--retries count - Retry the iput in case of error. The 'count' input",
"     specifies the number of times to retry. It must be used with the", 
"     -X option",
" --lfrestart lfRestartFile - specifies that the large file restart option is",
"      on and the lfRestartFile input specifies a local file that contains",
"      the restart info.",
" -h  this help",
""};
   int i;
   for (i=0;;i++) {
      if (strlen(msgs[i])==0) break;
      printf("%s\n",msgs[i]);
   }
   printReleaseInfo("iput");
}
