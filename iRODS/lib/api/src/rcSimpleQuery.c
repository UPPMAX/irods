/**
 * @file  rcSimpleQuery.c
 *
 */
/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code.  */ 
/* See simpleQuery.h for a description of this API call.*/

#include "simpleQuery.h"
/**
 * \fn rcSimpleQuery (rcComm_t *conn, simpleQueryInp_t *simpleQueryInp, simpleQueryOut_t **simpleQueryOut)
 *
 * \brief Perform a simple (pre-defined) query, allowed for Admin only, used in iadmin
 *
 * \user client
 *
 * \category metadata operations
 *
 * \since 1.0
 *
 * \author  Wayne Schroeder
 * \date    2007 
 *
 * \remark none
 *
 * \note none
 *
 * \usage
 * Perform a simple (pre-defined) query:
 * \n See the SQL-Based_Queries on irods.org
 * \n and examples in iquest.c (function execAndShowSimpleQuery).
 *
 * \param[in] conn - A rcComm_t connection handle to the server.
 * \param[in] simpleQueryInp - input sql or alias (must match definition on server), and arguments
 * \param[out] genQueryOut - the same returned structure as general-query.
 * \return integer
 * \retval 0 on success
 *
 * \sideeffect none
 * \pre none
 * \post none
 * \sa none
 * \bug  no known bugs
**/
int
rcSimpleQuery (rcComm_t *conn, simpleQueryInp_t *simpleQueryInp, 
simpleQueryOut_t **simpleQueryOut)
{
    int status;
    status = procApiRequest (conn, SIMPLE_QUERY_AN,  simpleQueryInp, NULL, 
        (void **) simpleQueryOut, NULL);

    return (status);
}
