/* This is script-generated code.  */ 
/* See fileSyncToArch.h for a description of this API call.*/

#include "fileSyncToArch.h"

int
rcFileSyncToArch (rcComm_t *conn, fileStageSyncInp_t *fileSyncToArchInp)
{
    int status;
    status = procApiRequest (conn, FILE_SYNC_TO_ARCH_AN, 
      fileSyncToArchInp, NULL, (void **) NULL, NULL);

    return (status);
}
