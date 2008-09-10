#include "integritychecksMS.h"
#include "icutils.h"


/* Utility function for listing file object and an input parameter field */
int msiListFields (msParam_t *mPin1, msParam_t *mPin2, msParam_t *mPout1, msParam_t* mPout2, ruleExecInfo_t *rei) {

	genQueryInp_t gqin;
	genQueryOut_t *gqout = NULL;
	char condStr[MAX_NAME_LEN];
	char tmpstr[MAX_NAME_LEN];
	rsComm_t *rsComm;
	char* collname;
	char* fieldname;
	sqlResult_t *dataName;
	sqlResult_t *dataField;
	bytesBuf_t*	mybuf=NULL;
	int i,j;
	int	fieldid;
	int debug=1;

	RE_TEST_MACRO ("    Calling msiListFields")

	/* Sanity check */
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiListFields: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	rsComm = rei->rsComm;

	/* init gqout, gqin & mybuf structs */
	mybuf = (bytesBuf_t *) malloc(sizeof(bytesBuf_t));
	memset (mybuf, 0, sizeof (bytesBuf_t));
	gqout = (genQueryOut_t *) malloc (sizeof (genQueryOut_t));
	memset (gqout, 0, sizeof (genQueryOut_t));
	memset (&gqin, 0, sizeof (genQueryInp_t));

	gqin.maxRows = MAX_SQL_ROWS;

	/* construct an SQL query from the parameter list */
	collname = (char*) strdup (mPin1->inOutStruct);
	fieldname = (char*) strdup (mPin2->inOutStruct);
	if ((fieldid = getAttrIdFromAttrName(fieldname))==NO_COLUMN_NAME_FOUND) {
		sprintf (tmpstr, "Field: %s not found in database", fieldname);
		appendToByteBuf (mybuf, tmpstr);
		return (-1);
	}

	if (debug) rodsLog (LOG_NOTICE, "fieldname: %s\tfieldid:%d", fieldname,fieldid);

	/* this is the info we want returned from the query */
	addInxIval (&gqin.selectInp, COL_DATA_NAME, 1);
	addInxIval (&gqin.selectInp, fieldid, 1);
	snprintf (condStr, MAX_NAME_LEN, " = '%s'", collname);
	addInxVal (&gqin.sqlCondInp, COL_COLL_NAME, condStr);

	j = rsGenQuery (rsComm, &gqin, &gqout);

	if (j<0) {
		appendToByteBuf (mybuf, "Gen Query Error returned badness");
	} else if (j != CAT_NO_ROWS_FOUND) {

		printGenQueryOut(stderr, NULL, NULL, gqout);

		dataName = getSqlResultByInx (gqout, COL_DATA_NAME);
		dataField = getSqlResultByInx (gqout, fieldid);

		rodsLog (LOG_ERROR, "got here 3 rowCnt=%d",gqout->rowCnt);

		for (i=0; i<gqout->rowCnt; i++) {
			sprintf (tmpstr, "Data object:%s\t%s:%s\n", &dataName->value[dataName->len *i], fieldname, &dataField->value[dataField->len *i]);
		rodsLog (LOG_ERROR, "got here 4");
			appendToByteBuf (mybuf, tmpstr);
		}

	} else appendToByteBuf (mybuf, "No matching rows found");

	fillBufLenInMsParam (mPout1, mybuf->len, mybuf);
	fillIntInMsParam (mPout2, rei->status);
  
	return(rei->status);
	
}


/* Test writePosInt microservice */
int msiTestWritePosInt (msParam_t* mPout1, ruleExecInfo_t *rei) {

	int butter=99;

	RE_TEST_MACRO ("    Calling msiTestWritePosInt")

	fillIntInMsParam (mPout1, butter);

	return(rei->status);
	
}

/* Silly hello world microservice */
int msiHiThere (msParam_t* mPout1, ruleExecInfo_t *rei) {

	char str[]="hi there\n";

	RE_TEST_MACRO ("    Calling msiHiThere")

	fillStrInMsParam (mPout1, str);

	return(rei->status);
	
}

