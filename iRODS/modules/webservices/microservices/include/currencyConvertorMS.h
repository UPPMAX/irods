/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/**
 * @file	stockQuoteMS.h
 *
 * @brief	Declarations for the msiStockuote* microservices.
 */



#ifndef CURRENCYCONVERTORMS_H
#define CURRENCYCONVERTORMS_H

#include "rods.h"
#include "reGlobalsExtern.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"


int msiConvertCurrency(msParam_t* inConvertFromlParam, msParam_t* inConvertToParam, 
	    msParam_t* outRateParam, ruleExecInfo_t* rei );

#endif	/* CURRENCYCONVERTORMS_H */
