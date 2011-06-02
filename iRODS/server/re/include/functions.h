/* For copyright information please refer to files in the COPYRIGHT directory
 */

#ifndef FUNCTIONS_H
#define	FUNCTIONS_H

#include "hashtable.h"
#include "region.h"
#include "parser.h"




FunctionDesc *newFunctionDesc(char* type, SmsiFuncPtrType func, Region *r);
FunctionDesc *newConstructorDesc(char* type, Region *r);
FunctionDesc *newConstructorDesc2(Node* type, Region *r);
FunctionDesc *newDeconstructorDesc(char *type, int proj, Region *r);
/*Hashtable *getSystemFunctionTypes(Region *r); */
void getSystemFunctions(Hashtable *ft, Region *r);

Res* eval(char *expr, Env *env, ruleExecInfo_t *rei, int saveREI, rError_t *errmsg, Region *r);

int getParamIOType(char *iotypes, int index);

FunctionDesc *getFuncDescFromChain(int n, FunctionDesc *fDesc);
Node *construct(char *fn, Node **args, int argc, Node* constype, Region *r);
Node *deconstruct(char *fn, Node **args, int argc, int proj, rError_t*errmsg, Region *r);
#endif	/* FUNCTIONS_H */

