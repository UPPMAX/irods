/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* fileDriverTable.h - header file for the global file driver table
 */



#ifndef FILE_DRIVER_TABLE_H
#define FILE_DRIVER_TABLE_H

#include "rods.h"
#include "fileDriver.h"
#ifdef _WIN32
#include "ntFileDriver.h"
#else
#include "unixFileDriver.h"
#endif
#include "miscServerFunct.h"
#ifdef HPSS
#include "hpssFileDriver.h"
#endif
#ifdef AMAZON_S3
#include "s3FileDriver.h"
#endif
#include "univMSSDriver.h"
#ifdef DDN_WOS
#include "wosFileDriver.h"
#endif

#define NO_FILE_DRIVER_FUNCTIONS intNoSupport,intNoSupport,intNoSupport,intNoSupport,intNoSupport,intNoSupport,intNoSupport,intNoSupport,longNoSupport,intNoSupport,intNoSupport,intNoSupport,intNoSupport,intNoSupport,intNoSupport,intNoSupport,intNoSupport, intNoSupport, longNoSupport, intNoSupport, intNoSupport, intNoSupport

fileDriver_t FileDriverTable[] = {
#ifndef windows_platform
    {UNIX_FILE_TYPE, unixFileCreate, unixFileOpen, unixFileRead, unixFileWrite,
    unixFileClose, unixFileUnlink, unixFileStat, unixFileFstat, unixFileLseek,
    unixFileFsync, unixFileMkdir, unixFileChmod, unixFileRmdir, unixFileOpendir,
    unixFileClosedir, unixFileReaddir, unixFileStage, unixFileRename,
    unixFileGetFsFreeSpace, unixFileTruncate, intNoSupport, intNoSupport},
#ifdef HPSS
    {HPSS_FILE_TYPE, intNoSupport, intNoSupport, intNoSupport, intNoSupport,
    intNoSupport, hpssFileUnlink, hpssFileStat, intNoSupport, longNoSupport,
    intNoSupport, hpssFileMkdir, hpssFileChmod, hpssFileRmdir, hpssFileOpendir,
    hpssFileClosedir, hpssFileReaddir, intNoSupport, hpssFileRename,
    hpssFileGetFsFreeSpace, intNoSupport, hpssStageToCache, hpssSyncToArch},
#else
    {HPSS_FILE_TYPE, NO_FILE_DRIVER_FUNCTIONS},
#endif
#else
	{NT_FILE_TYPE, ntFileCreate, ntFileOpen, ntFileRead, ntFileWrite,
    ntFileClose, ntFileUnlink, ntFileStat, ntFileFstat, ntFileLseek,
    intNoSupport, ntFileMkdir, ntFileChmod, ntFileRmdir, ntFileOpendir,
    ntFileClosedir, ntFileReaddir, intNoSupport, ntFileRename,
    longNoSupport, intNoSupport, intNoSupport, intNoSupport},
#endif

#ifndef windows_platform
#ifdef AMAZON_S3
    {S3_FILE_TYPE, intNoSupport, intNoSupport, intNoSupport, intNoSupport,
    intNoSupport, s3FileUnlink, s3FileStat, intNoSupport, longNoSupport,
    intNoSupport, s3FileMkdir, s3FileChmod, s3FileRmdir, intNoSupport,
    intNoSupport, intNoSupport, intNoSupport, s3FileRename,
    s3FileGetFsFreeSpace, intNoSupport, s3StageToCache, s3SyncToArch},
#else
    {S3_FILE_TYPE, NO_FILE_DRIVER_FUNCTIONS},
#endif
    {TEST_STAGE_FILE_TYPE,intNoSupport,intNoSupport, intNoSupport, intNoSupport,
    intNoSupport, unixFileUnlink, unixFileStat, unixFileFstat, longNoSupport,
    intNoSupport, unixFileMkdir, unixFileChmod, unixFileRmdir, unixFileOpendir,
    unixFileClosedir, unixFileReaddir, intNoSupport, unixFileRename,
    unixFileGetFsFreeSpace, intNoSupport, unixStageToCache, unixSyncToArch},
    {UNIV_MSS_FILE_TYPE,intNoSupport, intNoSupport, intNoSupport, intNoSupport,
    intNoSupport, univMSSFileUnlink, univMSSFileStat, intNoSupport, longNoSupport,
    intNoSupport, univMSSFileMkdir, univMSSFileChmod, intNoSupport, intNoSupport,
    intNoSupport, intNoSupport, intNoSupport, intNoSupport,
    longNoSupport, intNoSupport, univMSSStageToCache, univMSSSyncToArch},
#endif
#ifdef DDN_WOS
    {WOS_FILE_TYPE, intNoSupport, intNoSupport, intNoSupport, intNoSupport,
    intNoSupport, wosFileUnlink, wosFileStat, intNoSupport, longNoSupport,
    intNoSupport, intNoSupport, intNoSupport, intNoSupport, intNoSupport,
    intNoSupport, intNoSupport, intNoSupport, intNoSupport,
    wosFileGetFsFreeSpace, intNoSupport, wosStageToCache, wosSyncToArch},
#else
    {WOS_FILE_TYPE, NO_FILE_DRIVER_FUNCTIONS},
#endif
    {NON_BLOCKING_FILE_TYPE,unixFileCreate,unixFileOpen,nbFileRead,nbFileWrite,
    unixFileClose, unixFileUnlink, unixFileStat, unixFileFstat, unixFileLseek,
    unixFileFsync, unixFileMkdir, unixFileChmod, unixFileRmdir, unixFileOpendir,
    unixFileClosedir, unixFileReaddir, unixFileStage, unixFileRename,
    unixFileGetFsFreeSpace, unixFileTruncate, intNoSupport, intNoSupport},
};

int NumFileDriver = sizeof (FileDriverTable) / sizeof (fileDriver_t);

#endif	/* FILE_DRIVER_TABLE_H */
