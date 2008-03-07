/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* 

These are the Catalog Low Level (cll) routines for talking to Oracle.

For each of the supported database systems there is .c file like this
one with a set of routines by the same names.

Internal functions are those that do not begin with cll.  The external
functions used are those that begin with OCI (Oracle Call Interface).

*/

#include "icatLowLevelOracle.h"
int _cllFreeStatementColumns(icatSessionStruct *icss, int statementNumber);

int cllBindVarCount=0;
char *cllBindVars[MAX_BIND_VARS];
int cllBindVarCountPrev=0; /* cclBindVarCount earlier in processing */
char *bindName[MAX_BIND_VARS]={
    ":1",":2", ":3", ":4", ":5", ":6", ":7", ":8", ":9", 
    ":10", ":11",":12", ":13", ":14", ":15", ":16", ":17", ":18", ":19",
    ":20", ":21",":22", ":23", ":24", ":25", ":26", ":27", ":28", ":29",
    ":30", ":31",":32", ":33", ":34", ":35", ":36", ":37", ":38", ":39",
};

char testName[]=":1";
char testBindVar[]="a";

/* for now: */
#define MAX_TOKEN 256

#define TMP_STR_LEN 1040

#include <stdio.h>
#include <pwd.h>

static OCIError         *p_err;

OCIBind  *p_bind[20];

char            errbuf[100];
int             errcode;


/*
  call SQLError to get error information and log it
 */
int
logOraError(int level, OCIError *errhp, sword status)
{
   text errbuf[512];
   sb4 errcode;
   int errorVal=-1;
   if (status == OCI_SUCCESS) return(0);
   switch (status)  {
   case OCI_SUCCESS_WITH_INFO:
      rodsLog(level,"OCI_SUCCESS_WITH_INFO");
      OCIErrorGet ((dvoid *) errhp, (ub4) 1, (text *) NULL, &errcode,
		   errbuf, (ub4) sizeof(errbuf), (ub4) OCI_HTYPE_ERROR);
      rodsLog(level, "Error - %s\n", errbuf);
      errorVal=0;
      break;
   case OCI_NEED_DATA:
      rodsLog(level, "OCI_NEED_DATA");
      break;
   case OCI_NO_DATA:
      rodsLog(level, "OCI_NO_DATA");
      errorVal=0;
      break;
   case OCI_ERROR:
      OCIErrorGet ((dvoid *) errhp, (ub4) 1, (text *) NULL, &errcode,
		   errbuf, (ub4) sizeof(errbuf), (ub4) OCI_HTYPE_ERROR);
      rodsLog(level, "OCI_Error: %s", errbuf);
      if (strstr((char *)errbuf, "unique constraint") != 0) {
	 errorVal =  CATALOG_ALREADY_HAS_ITEM_BY_THAT_NAME;
      }
      break;
   case OCI_INVALID_HANDLE:
      rodsLog(level, "OCI_INVALID_HANDLE\n");
      break;
   case OCI_STILL_EXECUTING:
      rodsLog(level, "OCI_STILL_EXECUTING\n");
      break;
   case OCI_CONTINUE:
      rodsLog(level, "OCI_CONTINUE\n");
      break;
   default:
      rodsLog(level, "Unknown OCI status - %d", status);
      break;
   }
   return(errorVal);
}

/* 
 Allocate the environment structure for use by the SQL routines.
 */
int
cllOpenEnv(icatSessionStruct *icss) {
   int             stat;

   OCIEnv           *p_env;
   OCISvcCtx        *p_svc;

   stat = OCIEnvCreate((OCIEnv **)&p_env,
		       (ub4)OCI_DEFAULT,
		       (dvoid *)0, (dvoid * (*)(dvoid *, size_t))0,
		       (dvoid * (*)(dvoid *, dvoid *, size_t))0,
		       (void (*)(dvoid *, dvoid *))0,
		       (size_t)0, (dvoid **)0);
   if (stat != OCI_SUCCESS) {
      rodsLog(LOG_ERROR, "cllOpenEnv: OCIEnvInit failed");
      return(CAT_ENV_ERR);
   }

   /* Initialize handles */
   stat = OCIHandleAlloc( (dvoid *) p_env, (dvoid **) &p_err, OCI_HTYPE_ERROR,
			(size_t) 0, (dvoid **) 0);

   if (stat != OCI_SUCCESS) {
      rodsLog(LOG_ERROR, "cllOpenEnv: OCIHandleAlloc failed");
      return(CAT_ENV_ERR);
   }

   stat = OCIHandleAlloc( (dvoid *) p_env, (dvoid **) &p_svc, OCI_HTYPE_SVCCTX,
			(size_t) 0, (dvoid **) 0);

   if (stat != OCI_SUCCESS) {
      rodsLog(LOG_ERROR, "cllOpenEnv: OCIHandleAlloc failed");
      return(CAT_ENV_ERR);
   }

   icss->connectPtr=p_svc;
   icss->environPtr=p_env;
   return(0);
}

/* 
 Deallocate the environment structure.
 */
int
cllCloseEnv(icatSessionStruct *icss) {
   sword stat;
   stat = OCITerminate(OCI_DEFAULT);

   if (stat != OCI_SUCCESS) {
      rodsLog(LOG_ERROR, "cllCloseEnv: OCITerminate failed");
      return(CAT_ENV_ERR);
   }
   return(0);
}

/*
 Connect to the DBMS.
 */ 
int 
cllConnect(icatSessionStruct *icss) {
   int stat;
   OCIEnv           *p_env;
   OCISvcCtx        *p_svc;

   p_svc = icss->connectPtr;
   p_env = icss->environPtr;
   char userName[110];
   char databaseName[110];
   char *cp1, *cp2;
   int i, atFound;
   
   atFound=0;
   userName[0]='\0';
   databaseName[0]='\0';
   cp1=userName;
   cp2=icss->databaseUsername;
   for(i=0;i<100;i++) {
      if (*cp2=='@') {
	 atFound=1;
	 *cp1='\0';
	 cp1=databaseName;
	 cp2++;
      }
      else {
	 if (*cp2=='\0') {
	    *cp1='\0';
	    break;
	 }
	 else {
	    *cp1++=*cp2++;
	 }
      }
   }
   if (atFound==0) {
      rodsLog(LOG_ERROR, "no @ in the database user name");
      return(CAT_INVALID_ARGUMENT);
   }

#if 0
   stat = OCILogon(p_env, p_err, &p_svc, icss->databaseUsername,
		   strlen(icss->databaseUsername),
		   icss->databasePassword,
		   strlen(icss->databasePassword),
		   ORACLE_DATABASE_NAME, strlen(ORACLE_DATABASE_NAME));
#endif
   stat = OCILogon(p_env, p_err, &p_svc, userName,
		   strlen(userName),
		   icss->databasePassword,
		   strlen(icss->databasePassword),
		   databaseName, strlen(databaseName));

   if (stat != OCI_SUCCESS) {
      rodsLog(LOG_ERROR, "cllConnect: OCILogon failed: %d", stat);
      logOraError(LOG_ERROR, p_err, stat);
      return(CAT_CONNECT_ERR);
   }

   icss->connectPtr=p_svc;
   return(0);
}

/*
 Disconnect from the DBMS.
*/
int
cllDisconnect(icatSessionStruct *icss) {
   sword stat;
   OCISvcCtx *p_svc;

   p_svc = icss->connectPtr;

   stat = OCILogoff(p_svc, p_err);                           /* Disconnect */
   if (stat != OCI_SUCCESS) {
      rodsLog(LOG_ERROR, "cllDisconnect: OCILogoff failed: %d", stat);
      return(CAT_DISCONNECT_ERR);
   }

   stat = OCIHandleFree((dvoid *) p_svc, OCI_HTYPE_SVCCTX);
   stat = OCIHandleFree((dvoid *) p_err, OCI_HTYPE_ERROR);

   icss->connectPtr=0;
   return(0);
}

/* Convert postgres style Bind Variable sql statements into
   Oracle, ? becomes :1, :2, etc. */
int
convertSqlToOra(char *sql, char *sqlOut) {
   char *cp1, *cp2, *cpEnd;
   int i;
   cp1=sql;
   cp2=sqlOut;
   cpEnd=cp2+MAX_SQL_SIZE-2;
   i=1;
   while (*cp1!='\0') {
      if (*cp1 != '?') *cp2++=*cp1++;
      else {
	 *cp2++=':';
	 /* handle cases with up to 29 bind variables */
	 if (i>9) {
	    if (i>19) {
	       *cp2++='2';
	       *cp2++=i-20+'0';
	    }
	    else {
	       *cp2++='1';
	       *cp2++=i-10+'0';
	    }
	 }
	 else {
	    *cp2++=i+'0';
	 }
	 cp1++;
	 i++;
      }
      if (cp2 > cpEnd) return(-1);
   }
   *cp2 = '\0';
   return(0);
}

/*
 Log the bind variables from the global array (after an error)
*/
void
logTheBindVariables(int level)
{
   int i;
   char tmpStr[TMP_STR_LEN+2];
   for (i=0;i<cllBindVarCountPrev;i++) {
      snprintf(tmpStr, TMP_STR_LEN, "bindVar[%d]=:%s:", i+1, cllBindVars[i]);
      if (level == LOG_ERROR) {
	 rodsLog(level, tmpStr);
      } 
      else {
	 rodsLogSql(tmpStr);
      }
   }
}

/*
 Bind variables from the global array.
 */
int
bindTheVariables(OCIStmt *p_statement, char *sql) {
   int myBindVarCount;
   int stat;
   int i;

   for (i=0;i<20;i++) {
      p_bind[i]=NULL;
   }
   myBindVarCount = cllBindVarCount;
   cllBindVarCountPrev=cllBindVarCount; /* save in case we need to log error */
   cllBindVarCount = 0; /* reset for next call */

   if (myBindVarCount > 0) {
      for (i=0;i<myBindVarCount;i++) {
	 int len, len2;
	 len = strlen(bindName[i]);
	 len2 = strlen(cllBindVars[i])+1;
	 stat =  OCIBindByName(p_statement, &p_bind[i], p_err, 
			       (dvoid *)bindName[i],
			       len,
			       (dvoid *)cllBindVars[i],
			       len2,
			      SQLT_STR, 
			      0,0,0,0,0,
			      OCI_DEFAULT);

	 if (stat != OCI_SUCCESS) {
	    rodsLog(LOG_ERROR, "cllExecNoResult: OCIBindByName failed: %d",
		    stat);
	    rodsLog(LOG_ERROR, "sql:%s", sql);
	    logOraError(LOG_ERROR, p_err, stat);
	    return(CAT_OCI_ERROR);
	 }
      }
   }
   return(0);
}

int
logExecuteStatus(int stat, char *sql, char *funcName) {
   char * status;
   int stat2;
   status = "UNKNOWN";
   if (stat == OCI_SUCCESS) status= "SUCCESS";
   if (stat == OCI_SUCCESS_WITH_INFO) status="SUCCESS_WITH_INFO";
   if (stat == OCI_NO_DATA) status="NO_DATA";
   if (stat == OCI_ERROR) status="SQL_ERROR";
   if (stat == OCI_INVALID_HANDLE) status="HANDLE_ERROR";
   rodsLogSqlResult(status);

   if (stat == OCI_SUCCESS ||
       stat == OCI_SUCCESS_WITH_INFO || 
       stat == OCI_NO_DATA) {
      return(0);
   }
   else {
      rodsLog(LOG_NOTICE, 
	      "%s OCIStmtExecute error: %d, sql:%s",
	      funcName, stat, sql);
      stat2 = logOraError(LOG_NOTICE, p_err, stat);
      return(stat2);
   }
}


/*
 Execute a SQL command which has no resulting table.  Examples include
 insert, delete, update.
 */
int
cllExecSqlNoResult(icatSessionStruct *icss, char *sqlInput)
{

   int stat, stat2, stat3;
   OCIEnv           *p_env;
   OCISvcCtx        *p_svc;
   static OCIStmt          *p_statement;
   char sql[MAX_SQL_SIZE];
   ub4 rows_affected;
   ub4 *pUb4;

   stat = convertSqlToOra(sqlInput, sql);
   if (stat!=0) {
      rodsLog(LOG_ERROR, "cllExecSqlNoResult: SQL too long");
      return(CAT_OCI_ERROR);
   }

   p_svc = icss->connectPtr;
   p_env = icss->environPtr;

   if (strcmp(sql, "commit") == 0) {
      rodsLogSql(sql);
      stat = OCITransCommit(p_svc, p_err, (ub4) OCI_DEFAULT);
      stat2 = logExecuteStatus(stat, sql, "cllExecSqlNoResult");
      if (stat != OCI_SUCCESS) {
	 rodsLog(LOG_ERROR, "cllExecSqlNoResult: OCITransCommit failed: %d",
		 stat);
	 logOraError(LOG_ERROR, p_err, stat);
	 return(CAT_OCI_ERROR);
      }
      return(0);
   }

   if (strcmp(sql, "rollback") == 0) {
      rodsLogSql(sql);
      stat = OCITransRollback(p_svc, p_err, (ub4) OCI_DEFAULT);
      stat2 = logExecuteStatus(stat, sql, "cllExecSqlNoResult");
      if (stat != OCI_SUCCESS) {
	 rodsLog(LOG_ERROR, "cllExecSqlNoResult: OCITransRollback failed: %d",
		 stat);
	 logOraError(LOG_ERROR, p_err, stat);
	 return(CAT_OCI_ERROR);
      }
      return(0);
   }


   /* Allocate SQL statement */
   stat = OCIHandleAlloc( (dvoid *) p_env, (dvoid **) &p_statement,
			OCI_HTYPE_STMT, (size_t) 0, (dvoid **) 0);
   if (stat != OCI_SUCCESS) {
      rodsLog(LOG_ERROR, "cllExecSqlNoResult: OCIHandleAlloc failed: %d",stat);
      logOraError(LOG_ERROR, p_err, stat);
      return(CAT_OCI_ERROR);
   }

   /* Prepare SQL statement */
   stat = OCIStmtPrepare(p_statement, p_err, sql,
			 (ub4) strlen(sql),
			 (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
   if (stat != OCI_SUCCESS) {
      rodsLog(LOG_ERROR, "cllExecSqlNoResult: OCIStmtPrepare failed: %d",stat);
      rodsLog(LOG_ERROR, sql);
      logOraError(LOG_ERROR, p_err, stat);
      return(CAT_OCI_ERROR);
   }

   if (bindTheVariables(p_statement, sql) != 0) {
      logTheBindVariables(LOG_ERROR);
      return(CAT_OCI_ERROR);
   }
   logTheBindVariables(0);
   rodsLogSql(sql);

   /* Execute statement */
   stat = OCIStmtExecute(p_svc, p_statement, p_err, (ub4) 1, (ub4) 0,
		       (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
			 OCI_DEFAULT);
   stat2 = logExecuteStatus(stat, sql, "cllExecSqlNoResult");
   if (stat == OCI_NO_DATA) {   /* Don't think this ever happens, but... */
      return(CAT_SUCCESS_BUT_WITH_NO_INFO);
   }

   /* malloc it so that it's aligned properly, else can get
      bus errors when doing 64-bit addressing */
   pUb4 = malloc(sizeof(rows_affected));
   *pUb4=0;
   rows_affected = 0;

   if (stat == OCI_ERROR) {
      rodsLog(LOG_ERROR, "cllExecSqlNoResult: OCIStmtExecute failed: %d",stat);
      logOraError(LOG_ERROR, p_err, stat);
      return(CAT_OCI_ERROR);
   }

   stat3 = OCIAttrGet((dvoid *)p_statement, OCI_HTYPE_STMT, pUb4, 0, 
			    OCI_ATTR_ROW_COUNT, (OCIError *) p_err  );
   rows_affected=*pUb4;
   free(pUb4);

   stat = OCIHandleFree( (dvoid *)p_statement, OCI_HTYPE_STMT); /* free the
								   statement */

   if ( stat3 != OCI_SUCCESS ) {
      rodsLog(LOG_ERROR, "cllExecSqlNoResult: OCIAttrGet failed: %d",stat);
      return(stat3);
   }
   /* rodsLog(LOG_NOTICE, "cllExecSqlNoResult: OCIAttrGet, rows_affected: %d",
              rows_affected); */
   if (rows_affected == 0) {
      return(CAT_SUCCESS_BUT_WITH_NO_INFO);
   }

   return(stat2);

}


/*
  Return a row from a previous cllExecSqlWithResult call.
 */
int
cllGetRow(icatSessionStruct *icss, int statementNumber) {
   static OCIStmt          *p_statement;
   int nCols, stat;

   icatStmtStrct *myStatement;

   myStatement=icss->stmtPtr[statementNumber];
   nCols = myStatement->numOfCols;
   p_statement = myStatement->stmtPtr;

   stat = OCIStmtFetch(p_statement, p_err, (ub4)1, (ub2)0,
		       (ub4) OCI_DEFAULT);
   if (stat != OCI_SUCCESS && stat != OCI_NO_DATA) {
      logOraError(LOG_ERROR, p_err, stat);
      _cllFreeStatementColumns(icss,statementNumber);
      myStatement->numOfCols=0;
      rodsLog(LOG_ERROR, "cllGetRow: Fetch failed: %d", stat);
      return(-1);
   }
   if (stat == OCI_SUCCESS) {
      return(0);
   }
   myStatement->numOfCols=0;
   return(0);
}


/* 
  Execute a SQL command that returns a result table, and and bind the
  default row.  Also check and bind the global array of bind variables
  (if any).
*/
int
cllExecSqlWithResult(icatSessionStruct *icss, int *stmtNum, char *sql) {
   OCIEnv           *p_env;
   OCISvcCtx        *p_svc;
   static OCIStmt          *p_statement;
   static OCIDefine        *p_dfn    = (OCIDefine *) 0;
   int stat, stat2, i, j;
   char *cptr;
   char sqlConverted[MAX_SQL_SIZE];

   icatStmtStrct *myStatement;
   int statementNumber;

   int counter;
   OCIParam     *mypard = (OCIParam *) 0;
   ub2 dtype;
   ub2 col_width;
   ub4 char_semantics;
   OraText  *colName;

   static int columnLength[MAX_TOKEN]; 
   static sb2 indicator[MAX_TOKEN]; 

   p_svc = icss->connectPtr;
   p_env = icss->environPtr;

   i = convertSqlToOra(sql, sqlConverted);
   if (i !=0) {
      rodsLog(LOG_ERROR, "cllExecSqlWithResult: SQL too long");
      return(CAT_OCI_ERROR);
   }

   /* Allocate SQL statement */
   stat = OCIHandleAlloc( (dvoid *) p_env, (dvoid **) &p_statement,
			OCI_HTYPE_STMT, (size_t) 0, (dvoid **) 0);
   if (stat != OCI_SUCCESS) {
      rodsLog(LOG_ERROR, "cllExecSqlWithResult: OCIHandleAlloc failed: %d",
	      stat);
      logOraError(LOG_ERROR, p_err, stat);
      return(CAT_OCI_ERROR);
   }

   /* set up our statement */
   statementNumber=-1;
   for (i=0;i<MAX_NUM_OF_CONCURRENT_STMTS && statementNumber<0;i++) {
      if (icss->stmtPtr[i]==0) {
	 statementNumber=i;
      }
   }
   if (statementNumber<0) {
      rodsLog(LOG_ERROR, 
	      "cllExecSqlWithResult: too many concurrent statements");
      return(-2);
   }

   myStatement = (icatStmtStrct *)malloc(sizeof(icatStmtStrct));
   icss->stmtPtr[statementNumber]=myStatement;
   myStatement->numOfCols=0;

   myStatement->stmtPtr = p_statement;

   /* Prepare SQL statement */
   stat = OCIStmtPrepare(p_statement, p_err, sqlConverted,
			 (ub4) strlen(sqlConverted),
			 (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
   if (stat != OCI_SUCCESS) {
      rodsLog(LOG_ERROR, "cllExecSqlWithResult: OCIStmtPrepare failed: %d",stat);
      rodsLog(LOG_ERROR, sqlConverted);
      logOraError(LOG_ERROR, p_err, stat);
      return(CAT_OCI_ERROR);
   }

   if (bindTheVariables(p_statement, sqlConverted) != 0) {
      logTheBindVariables(LOG_ERROR);
      return(CAT_OCI_ERROR);
   }

   logTheBindVariables(0);
   rodsLogSql(sqlConverted);

   /* Execute statement */
   stat = OCIStmtExecute(p_svc, p_statement, p_err, (ub4) 0, (ub4) 0,
		       (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
			 OCI_DEFAULT);

   stat2 = logExecuteStatus(stat, sql, "cllExecSqlWithResult");

   if (stat2) {
      return(stat2);
   }

   *stmtNum = statementNumber; /* return index to statement handle */

   /* get the number of columns and width of the columns */

   /* Request a parameter descriptor for position 1 in the select-list */
   counter = 1;
   stat = OCIParamGet((dvoid *)p_statement, OCI_HTYPE_STMT, p_err,
			     (dvoid **)&mypard, (ub4) counter);

   /* Loop only if a descriptor was successfully retrieved for
      current position, starting at 1 */

   while (stat == OCI_SUCCESS) {
      /* Retrieve the datatype attribute */
      stat = OCIAttrGet((dvoid*) mypard, (ub4) OCI_DTYPE_PARAM,
			(dvoid*) &dtype,(ub4 *) 0, (ub4) OCI_ATTR_DATA_TYPE,
			(OCIError *) p_err  );
      if (stat != OCI_SUCCESS) {
	 rodsLog(LOG_ERROR, "cllExecSqlWithResult: OCIAttrGet failed: %d",
		 stat);
	 logOraError(LOG_ERROR, p_err, stat);
	 return(CAT_OCI_ERROR);
      }

      /* Retrieve the length semantics for the column */
      char_semantics = 0;
      stat = OCIAttrGet((dvoid*) mypard, (ub4) OCI_DTYPE_PARAM,
			(dvoid*) &char_semantics,(ub4 *) 0, 
			(ub4) OCI_ATTR_CHAR_USED,
			(OCIError *) p_err  );
      if (stat != OCI_SUCCESS) {
	 rodsLog(LOG_ERROR, "cllExecSqlWithResult: OCIAttrGet failed: %d",
		 stat);
	 logOraError(LOG_ERROR, p_err, stat);
	 return(CAT_OCI_ERROR);
      }

      /* Retrieve the column width in characters */
      col_width = 0;
      if (char_semantics) {
	 stat = OCIAttrGet((dvoid*) mypard, (ub4) OCI_DTYPE_PARAM,
			(dvoid*) &col_width, (ub4 *) 0, 
			(ub4) OCI_ATTR_CHAR_SIZE,
			(OCIError *) p_err  );
      }
      else {
	 stat = OCIAttrGet((dvoid*) mypard, (ub4) OCI_DTYPE_PARAM,
			   (dvoid*) &col_width,(ub4 *) 0, 
			   (ub4) OCI_ATTR_DATA_SIZE,
			   (OCIError *) p_err);
      }
      if (stat != OCI_SUCCESS) {
	 rodsLog(LOG_ERROR, "cllExecSqlWithResult: OCIAttrGet failed: %d",
		 stat);
	 logOraError(LOG_ERROR, p_err, stat);
	 return(CAT_OCI_ERROR);
      }

      /* get the col name */
      stat = OCIAttrGet((dvoid*) mypard, (ub4) OCI_DTYPE_PARAM,
			&colName,(ub4 *) 0, 
			(ub4) OCI_ATTR_NAME,
			(OCIError *) p_err  );
      if (stat != OCI_SUCCESS) {
	 rodsLog(LOG_ERROR, "cllExecSqlWithResult: OCIAttrGet failed: %d",
		 stat);
	 logOraError(LOG_ERROR, p_err, stat);
	 return(CAT_OCI_ERROR);
      }

      columnLength[counter]=col_width;

      i = counter-1;
      columnLength[i]=col_width;

      if (strlen(colName)>col_width) columnLength[i]=strlen(colName);

      myStatement->resultColName[i] = malloc((int)columnLength[i]+2);
      strncpy(myStatement->resultColName[i], 
	      (char *)colName, columnLength[i]);

      /* Big cludge/hack, but it looks like an OCI bug when the colName
         is exactly 8 bytes (or something).  Anyway, when the column name
         is "RESC_NETRESC_DEF_PATH" it's actually the running together of
         two names, so the code below corrects it. */
      if (strcmp (myStatement->resultColName[i], "RESC_NETRESC_DEF_PATH")==0) {
	 strncpy(myStatement->resultColName[i], 
		 "RESC_NET", columnLength[i]);
      }

      /* convert the column name to lower case to match postgres */
      cptr = (char*)myStatement->resultColName[i];
      for (j=0;j<columnLength[i];j++) {
	 if (*cptr=='\0') break;
	 if (*cptr==':') break;
	 if (*cptr >= 'A' && *cptr<='Z') *cptr+=((int)'a'-(int)'A');
	 cptr++;
      }

      myStatement->resultValue[i] = malloc((int)columnLength[i]+2);
      strcpy((char *)myStatement->resultValue[i],"");

      stat = OCIDefineByPos(p_statement, &p_dfn, p_err, counter, 
			    (dvoid *) myStatement->resultValue[i],
			    (sword) columnLength[i], SQLT_STR, 
			    (dvoid *)&indicator[i], 
			    (ub2 *)0, (ub2 *)0, OCI_DEFAULT);
      if (stat != OCI_SUCCESS) {
	 rodsLog(LOG_ERROR, "cllExecSqlWithResult: OCIDefineByPos failed: %d",stat);
	 logOraError(LOG_ERROR, p_err, stat);
	 return(CAT_OCI_ERROR);
      }


      /* increment counter and get next descriptor, if there is one */
      counter++;
      stat = OCIParamGet((dvoid *)p_statement, OCI_HTYPE_STMT, p_err,
			     (dvoid **)&mypard, (ub4) counter);
   }


   if (counter==1) {
      rodsLog(LOG_ERROR, "cllExecSqlWithResult: SQLNumResultCols failed: %d",
	      stat);
      return(-2);
   }
   myStatement->numOfCols=counter-1;

   return(0);
}

/* 
  Execute a SQL command that returns a result table, and
  and bind the default row; and allow optional bind variables.
*/
int
cllExecSqlWithResultBV(icatSessionStruct *icss, int *stmtNum, char *sql,
 			 char *bindVar1, char *bindVar2, char *bindVar3,
			 char *bindVar4, char *bindVar5, char *bindVar6) {
   int done;
   done=0;
   if (bindVar1 != NULL && strlen(bindVar1) > 0) {
      cllBindVars[cllBindVarCount++]=bindVar1;
   } else {
      done=1;
   }
   if (!done && bindVar2 != NULL && strlen(bindVar2) > 0) {
      cllBindVars[cllBindVarCount++]=bindVar2;
   } else {
      done=1;
   }
   if (!done && bindVar3 != NULL && strlen(bindVar3) > 0) {
      cllBindVars[cllBindVarCount++]=bindVar3;
   } else {
      done=1;
   }
   if (!done && bindVar4 != NULL && strlen(bindVar4) > 0) {
      cllBindVars[cllBindVarCount++]=bindVar4;
   } else {
      done=1;
   }
   if (!done && bindVar5 != NULL && strlen(bindVar5) > 0) {
      cllBindVars[cllBindVarCount++]=bindVar5;
   } else {
      done=1;
   }
   if (!done && bindVar6 != NULL && strlen(bindVar6) > 0) {
      cllBindVars[cllBindVarCount++]=bindVar6;
   } else {
      done=1;
   }
   return(cllExecSqlWithResult(icss, stmtNum, sql));
}

/* 
  Return the string needed to get the next value in a sequence item.
  The syntax varies between RDBMSes, so it is here, in the DBMS-specific code.
 */
int
cllNextValueString(char *itemName, char *outString, int maxSize) {
   snprintf(outString, maxSize, "%s.nextval", itemName);
   return 0;
}

int
cllCurrentValueString(char *itemName, char *outString, int maxSize) {
   snprintf(outString, maxSize, "%s.currval", itemName);
   return 0;
}

int
cllGetRowCount(icatSessionStruct *icss, int statementNumber) {
   int i, stat;

   icatStmtStrct *myStatement;
   OCIStmt  *p_statement;
   OCIParam *p_param = (OCIParam *) 0;
   ub4 rowCount;
   ub4 *pUb4;

   void * vptr;
   vptr = alignDouble((void *)&rowCount);

   /* malloc it so that it's aligned properly, else can get
      bus errors when doing 64-bit addressing */
   pUb4 = malloc(sizeof(rowCount));

   myStatement=icss->stmtPtr[statementNumber];
   p_statement = myStatement->stmtPtr;
   stat = OCIParamGet((dvoid *)p_statement, OCI_HTYPE_STMT, p_err,
			     (dvoid **)&p_param, (ub4) 1);
   if (stat == OCI_SUCCESS) {
      stat = OCIAttrGet((dvoid*) p_param, (ub4) OCI_DTYPE_PARAM,
			 pUb4,(ub4 *) 0, (ub4) OCI_ATTR_ROW_COUNT,
			(OCIError *) p_err  );
      if (stat != OCI_SUCCESS) {
	 rodsLog(LOG_ERROR, "cllGetRowCount: OCIAttrGet failed: %d",stat);
	 logOraError(LOG_ERROR, p_err, stat);
	 return(CAT_OCI_ERROR);
      }
   }
   rowCount = *pUb4;
   i=rowCount;
   free(pUb4);
   return(rowCount);
}

/* 
  Free a statement (from a previous cllExecSqlWithResult call) and the
  corresponding resultValue array.
*/
int
cllFreeStatement(icatSessionStruct *icss, int statementNumber) {
   int stat;
   int i;
   OCIEnv           *p_env;

   icatStmtStrct *myStatement;
   OCIStmt  *p_statement;

   p_env = icss->environPtr;

   myStatement=icss->stmtPtr[statementNumber];
   if (myStatement==NULL) { /* already freed */
      return(0);
   }
   p_statement = myStatement->stmtPtr;

   for (i=0;i<myStatement->numOfCols;i++) {
      free(myStatement->resultValue[i]);
      free(myStatement->resultColName[i]);
   }

   if (p_statement != NULL) {
      stat = OCIHandleFree( 
			    (dvoid *)p_statement,
			    OCI_HTYPE_STMT);
      
      if (stat != OCI_SUCCESS) {
	 rodsLog(LOG_ERROR, "cllFreeStatement: OCIHandleFree failed: %d",
		 stat);
	 logOraError(LOG_ERROR, p_err, stat);
	 return(CAT_OCI_ERROR);
      }
   }

   free(myStatement);
   icss->stmtPtr[statementNumber]=0;  /* indicate that the statement is free */

   return (0);
}

/* 
  Free the statement columns (from a previous cllExecSqlWithResult call),
  but not the whole statement.
*/
int
_cllFreeStatementColumns(icatSessionStruct *icss, int statementNumber) {
   int i;

   icatStmtStrct *myStatement;

   myStatement=icss->stmtPtr[statementNumber];

   for (i=0;i<myStatement->numOfCols;i++) {
      free(myStatement->resultValue[i]);
      free(myStatement->resultColName[i]);
   }
   return (0);
}

/*
 A few tests to verify basic functionality (including talking with
 the database via ODBC). 
 */
int cllTest(char *userArg, char *pwArg) {
   int i;
   int j, k;
   int OK;
   int stmt;
   int numOfCols;
   char userName[500];
   int numRows;

   struct passwd *ppasswd;
   icatSessionStruct icss;

   icss.stmtPtr[0]=0;
   rodsLogSqlReq(1);
   OK=1;
   i = cllOpenEnv(&icss);
   if (i != 0) OK=0;

   if (userArg==0 || *userArg=='\0') {
      ppasswd = getpwuid(getuid());    /* get user passwd entry             */
      strcpy(userName,ppasswd->pw_name);  /* get user name                  */
   }
   else {
      strncpy(userName, userArg, 500);
   }
   printf("userName=%s\n",userName);
   printf("password=%s\n",pwArg);

   icss.databaseUsername=userName;
   if (pwArg==0 || *pwArg=='\0') {
      icss.databasePassword="";
   }
   else {
      icss.databasePassword=pwArg;
   }

   i = cllConnect(&icss);
   if (i != 0) exit(-1);

   i = cllExecSqlNoResult(&icss,"drop table test");

   i = cllExecSqlNoResult(&icss,"create table test (i integer, a2345678901234567890123456789j integer, a varchar(50) )" );
   if (i != 0 && i !=CAT_SUCCESS_BUT_WITH_NO_INFO) OK=0;

   i = cllExecSqlNoResult(&icss,
			  "insert into test values ('1', '2', 'asdfas')");
   if (i != 0) OK=0;

   i = cllExecSqlNoResult(&icss, "commit");
   if (i != 0) OK=0;

   i = cllExecSqlNoResult(&icss, "insert into test values (2, 3, 'a')");
   if (i != 0) OK=0;

   i = cllExecSqlNoResult(&icss, "commit");
   if (i != 0) OK=0;

   i = cllExecSqlNoResult(&icss, "bad sql");
   if (i == 0) OK=0;   /* should fail, if not it's not OK */

   i = cllExecSqlNoResult(&icss, "delete from test where i = '1'");
   if (i != 0 && i != CAT_SUCCESS_BUT_WITH_NO_INFO) OK=0;

   i = cllExecSqlNoResult(&icss, "commit");
   if (i != 0) OK=0;

   i = cllExecSqlWithResult(&icss, &stmt, "select * from test where a = 'a'");
   if (i != 0) OK=0;

   if (i == 0) {
      numOfCols = 1;
      for (j=0;j<10 && numOfCols>0;j++) {
	 i = cllGetRow(&icss, stmt);
	 if (i != 0) {
	    OK=0;
	    break;
	 }
	 else {
	    numOfCols = icss.stmtPtr[stmt]->numOfCols;
	    if (numOfCols == 0) {
	       printf("No more rows returned\n");
	       i = cllFreeStatement(&icss,stmt);
	    }
	    else {
	       for (k=0; k<numOfCols || k < icss.stmtPtr[stmt]->numOfCols; k++){
		  printf("resultValue[%d]=%s\n",k,
			 icss.stmtPtr[stmt]->resultValue[k]);
	       }
	    }
	 }
      }
   }

   cllBindVars[cllBindVarCount++]="a";
   i = cllExecSqlWithResult(&icss, &stmt, 
			    "select * from test where a = ?");
   if (i != 0) OK=0;

   numRows=0;
   if (i == 0) {
      numOfCols = 1;
      for (j=0;j<10 && numOfCols>0;j++) {
	 i = cllGetRow(&icss, stmt);
	 if (i != 0) {
	    OK=0;
	 }
	 else {
	    numOfCols = icss.stmtPtr[stmt]->numOfCols;
	    if (numOfCols == 0) {
	       printf("No more rows returned\n");
	       i = cllFreeStatement(&icss,stmt);
	    }
	    else {
	       numRows++;
	       for (k=0; k<numOfCols || k<icss.stmtPtr[stmt]->numOfCols; k++){
		  printf("resultValue[%d]=%s\n",k,
			 icss.stmtPtr[stmt]->resultValue[k]);
	       }
	    }
	 }
      }
   }

   if (numRows != 1) {
      printf("Error: Did not return 1 row, %d instead\n", numRows);
      OK=0;
   }

   i = cllExecSqlNoResult(&icss,"drop table test");
   if (i != 0 && i != CAT_SUCCESS_BUT_WITH_NO_INFO) OK=0;

   i = cllExecSqlNoResult(&icss, "commit");
   if (i != 0) OK=0;

   i = cllDisconnect(&icss);
   if (i != 0) OK=0;

   i = cllCloseEnv(&icss);
   if (i != 0) OK=0;

   if (OK) {
      printf("The tests all completed normally\n");
      return(0);
   }
   else {
      printf("One or more tests DID NOT complete normally\n");
      return(-1);
   }
}
