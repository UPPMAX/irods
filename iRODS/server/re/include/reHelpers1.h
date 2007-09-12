/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* reHelpers1.h - common header file for rods server and agents
 */



#ifndef RE_HELPERS1_H
#define RE_HELPERS1_H

#include "reGlobalsExtern.h"

int
checkRuleCondition(char *action, char *incond, 
char *args[MAX_NUM_OF_ARGS_IN_ACTION], int argc, ruleExecInfo_t *rei, 
int reiSaveFlag );
int
checkRuleConditionNew(char *action, char *incond,  
msParamArray_t *inMsParamArray, ruleExecInfo_t *rei, int reiSaveFlag );
int
computeExpression( char *expr, ruleExecInfo_t *rei, int reiSaveFlag , char *res);
int
replaceVariables(char *action, char *inStr, 
char *args[MAX_NUM_OF_ARGS_IN_ACTION], int argc, ruleExecInfo_t *rei );
int
replaceVariablesNew(char *action, char *inStr, msParamArray_t *inMsParamArray,
ruleExecInfo_t *rei );
int
replaceVariablesAndMsParams(char *action, char *inStr, msParamArray_t *inMsParamArray, ruleExecInfo_t *rei );
int
replaceMsParams(char *inStr, msParamArray_t *inMsParamArray);
int
replaceDollarParam(char *action, char *dPtr, int len,
                   char *args[MAX_NUM_OF_ARGS_IN_ACTION], int argc,
                   ruleExecInfo_t *rei);
int
reREMatch(char *pat, char *str);


#endif	/* RE_HELPERS1_H */
