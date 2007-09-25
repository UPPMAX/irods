#include "ruleExecSubmit.h"
#include "icatHighLevelRoutines.h"

int
rsRuleExecSubmit (rsComm_t *rsComm, ruleExecSubmitInp_t *ruleExecSubmitInp,
char **ruleExecId)
{
    rodsServerHost_t *rodsServerHost;
    int status;

    if (ruleExecSubmitInp == NULL || 
      ruleExecSubmitInp->packedReiAndArgBBuf == NULL ||
      ruleExecSubmitInp->packedReiAndArgBBuf->len <= 0 ||
      ruleExecSubmitInp->packedReiAndArgBBuf->buf == NULL) {
       rodsLog(LOG_NOTICE,
        "rsRuleExecSubmit error. NULL input");
       return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    status = getAndConnRcatHost(rsComm, MASTER_RCAT, NULL,
                                &rodsServerHost);
    if (status < 0) {
       return(status);
    }

    if (rodsServerHost->localFlag == LOCAL_HOST) {
#ifdef RODS_CAT
       status = _rsRuleExecSubmit (rsComm, ruleExecSubmitInp);
#else
       rodsLog(LOG_NOTICE,
               "rsRuleExecSubmit error. ICAT is not configured on this host");
       return (SYS_NO_ICAT_SERVER_ERR);
#endif
    } else {
        status = rcRuleExecSubmit(rodsServerHost->conn, ruleExecSubmitInp,
	  ruleExecId);
	if (status >= 0) {
            *ruleExecId = strdup (ruleExecSubmitInp->ruleExecId);
	}
    }
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "rsRuleExecSubmit: rcRuleExecSubmit failed, status = %d", 
	  status);
    }
    return (status);
}

int
_rsRuleExecSubmit (rsComm_t *rsComm, ruleExecSubmitInp_t *ruleExecSubmitInp)
{
    int reiFd;
    int status;

    /* write the packedReiAndArgBBuf to local file */

    while (1) {
	status = getReiFilePath (ruleExecSubmitInp->reiFilePath, 
         ruleExecSubmitInp->userName);
	if (status < 0) {
            rodsLog (LOG_ERROR,
              "rsRuleExecSubmit: getReiFilePath failed, status = %d", status);
	    return (status);
	}
        reiFd = creat (ruleExecSubmitInp->reiFilePath, 0640);
	if (reiFd < 0) {
	    status = SYS_OPEN_REI_FILE_ERR - errno;
            rodsLog (LOG_ERROR,
              "rsRuleExecSubmit: creat failed for %s, status = %d", 
	      ruleExecSubmitInp->reiFilePath, status);
	    if (errno == EEXIST) {
		continue;
	    } else {
		return (SYS_OPEN_REI_FILE_ERR - errno);
	    }
	} else {
	    break;
	}
    }

    status = write (reiFd, ruleExecSubmitInp->packedReiAndArgBBuf->buf,
      ruleExecSubmitInp->packedReiAndArgBBuf->len);

    close (reiFd);

    if (status != ruleExecSubmitInp->packedReiAndArgBBuf->len) {
	rodsLog (LOG_ERROR,
         "rsRuleExecSubmit: write rei error.toWrite %d, %d written", 
	   ruleExecSubmitInp->packedReiAndArgBBuf->len, status);
	return (SYS_COPY_LEN_ERR - errno);
    }
  
    /* register the request */
#ifdef RODS_CAT
    status = chlRegRuleExec (rsComm, ruleExecSubmitInp);
    if (status < 0) {
        rodsLog(LOG_ERROR,
         "_rsRuleExecSubmit: chlRegRuleExec error. status = %d", status);
    }
    return (status);
#else
       rodsLog(LOG_ERROR,
               "_rsRuleExecSubmit error. ICAT is not configured on this host");
       return (SYS_NO_ICAT_SERVER_ERR);
#endif

}

int 
getReiFilePath (char *reiFilePath, char *userName)
{
    char *myUserName;

    if (reiFilePath == NULL) {
	return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    if (userName == NULL || strlen (userName) == 0) {
	myUserName = DEF_REI_USER_NAME;
    } else {
	myUserName = userName;
    } 

    snprintf (reiFilePath, MAX_NAME_LEN,
     "%-s/%-s/%-s.%-s.%-d", getConfigDir(), PACKED_REI_DIR,
       REI_FILE_NAME, myUserName, (uint) random());

    return (0);
}

