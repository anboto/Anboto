/**********************************************************************
 * $Id: cpl_vsil_unix_stdio_64.cpp 15219 2008-08-25 20:38:56Z mloskot $
 *
 * Project:  CPL - Common Portability Library
 * Purpose:  Implement VSI large file api for Unix platforms with fseek64()
 *           and ftell64() such as IRIX. 
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 **********************************************************************
 * Copyright (c) 2001, Frank Warmerdam
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 ****************************************************************************
 *
 * NB: Note that in wrappers we are always saving the error state (errno
 * variable) to avoid side effects during debug prints or other possible
 * standard function calls (error states will be overwritten after such
 * a call).
 *
 ****************************************************************************/

#include "cpl_port.h"

#include "cpl_vsi_virtual.h"
#include "cpl_string.h"

#if !defined(WIN32) && !defined(WIN32CE)

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

CPL_CVSID("$Id: cpl_vsil_unix_stdio_64.cpp 15219 2008-08-25 20:38:56Z mloskot $");

/************************************************************************/
/* ==================================================================== */
/*                       VSIUnixStdioFilesystemHandler                  */
/* ==================================================================== */
/************************************************************************/

class VSIUnixStdioFilesystemHandler : public VSIFilesystemHandler 
{
public:
	VSIUnixStdioFilesystemHandler();

    virtual VSIVirtualHandle *Open( const char *pszFilename, 
                                    const char *pszAccess);
    virtual int      Stat( const char *pszFilename, VSIStatBufL *pStatBuf );
    virtual int      Unlink( const char *pszFilename );
    virtual int      Rename( const char *oldpath, const char *newpath );
    virtual int      Mkdir( const char *pszDirname, long nMode );
    virtual int      Rmdir( const char *pszDirname );
    virtual char   **ReadDir( const char *pszDirname );
};

/************************************************************************/
/* ==================================================================== */
/*                        VSIUnixStdioHandle                            */
/* ==================================================================== */
/************************************************************************/

class VSIUnixStdioHandle : public VSIVirtualHandle
{
  public:
    FILE          *fp;

    virtual int       Seek( vsi_l_offset nOffset, int nWhence );
    virtual vsi_l_offset Tell();
    virtual size_t    Read( void *pBuffer, size_t nSize, size_t nMemb );
    virtual size_t    Write( const void *pBuffer, size_t nSize, size_t nMemb );
    virtual int       Eof();
    virtual int       Flush();
    virtual int       Close();
};

/************************************************************************/
/*                               Close()                                */
/************************************************************************/

int VSIUnixStdioHandle::Close()

{
    VSIDebug1( "VSIUnixStdioHandle::Close(%p)", fp );

    return fclose( fp );
}

/************************************************************************/
/*                                Seek()                                */
/************************************************************************/

int VSIUnixStdioHandle::Seek( vsi_l_offset nOffset, int nWhence )

{
    int     nResult = VSI_FSEEK64( fp, nOffset, nWhence );
    int     nError = errno;

#ifdef VSI_DEBUG

    if( nWhence == SEEK_SET )
    {
        VSIDebug3( "VSIUnixStdioHandle::Seek(%p,%d,SEEK_SET) = %d",
                   fp, nOffset, nResult );
    }
    else if( nWhence == SEEK_END )
    {
        VSIDebug3( "VSIUnixStdioHandle::Seek(%p,%d,SEEK_END) = %d",
                   fp, nOffset, nResult );
    }
    else if( nWhence == SEEK_CUR )
    {
        VSIDebug3( "VSIUnixStdioHandle::Seek(%p,%d,SEEK_CUR) = %d",
                   fp, nOffset, nResult );
    }
    else
    {
        VSIDebug4( "VSIUnixStdioHandle::Seek(%p,%d,%d-Unknown) = %d",
                   fp, nOffset, nWhence, nResult );
    }

#endif 

    errno = nError;
    return nResult;
}

/************************************************************************/
/*                                Tell()                                */
/************************************************************************/

vsi_l_offset VSIUnixStdioHandle::Tell()

{
    vsi_l_offset    nOffset = VSI_FTELL64( fp );
    int             nError = errno;

    VSIDebug2( "VSIUnixStdioHandle::Tell(%p) = %ld", fp, (long)nOffset );

    errno = nError;
    return nOffset;
}

/************************************************************************/
/*                               Flush()                                */
/************************************************************************/

int VSIUnixStdioHandle::Flush()

{
    VSIDebug1( "VSIUnixStdioHandle::Flush(%p)", fp );

    return fflush( fp );
}

/************************************************************************/
/*                                Read()                                */
/************************************************************************/

size_t VSIUnixStdioHandle::Read( void * pBuffer, size_t nSize, size_t nCount )

{
    size_t  nResult = fread( pBuffer, nSize, nCount, fp );
    int     nError = errno;

    VSIDebug4( "VSIUnixStdioHandle::Read(%p,%ld,%ld) = %ld", 
               fp, (long)nSize, (long)nCount, (long)nResult );

    errno = nError;
    return nResult;
}

/************************************************************************/
/*                               Write()                                */
/************************************************************************/

size_t VSIUnixStdioHandle::Write( const void * pBuffer, size_t nSize, 
                                  size_t nCount )

{
    size_t  nResult = fwrite( pBuffer, nSize, nCount, fp );
    int     nError = errno;

    VSIDebug4( "VSIUnixStdioHandle::Write(%p,%ld,%ld) = %ld", 
               fp, (long)nSize, (long)nCount, (long)nResult );

    errno = nError;
    return nResult;
}

/************************************************************************/
/*                                Eof()                                 */
/************************************************************************/

int VSIUnixStdioHandle::Eof()

{
    return feof( fp );
}

/************************************************************************/
/* ==================================================================== */
/*                       VSIUnixStdioFilesystemHandler                  */
/* ==================================================================== */
/************************************************************************/

VSIUnixStdioFilesystemHandler::VSIUnixStdioFilesystemHandler()
{
//	printf("UnixFH = %08x\n", (int)this);
}

/************************************************************************/
/*                                Open()                                */
/************************************************************************/

VSIVirtualHandle *
VSIUnixStdioFilesystemHandler::Open( const char *pszFilename, 
                                     const char *pszAccess )

{
    FILE    *fp = VSI_FOPEN64( pszFilename, pszAccess );
    int     nError = errno;
    
    VSIDebug3( "VSIUnixStdioFilesystemHandler::Open(\"%s\",\"%s\") = %p",
               pszFilename, pszAccess, fp );

    if( fp == NULL )
    {
        errno = nError;
        return NULL;
    }

    VSIUnixStdioHandle *poHandle = new VSIUnixStdioHandle;
    
    poHandle->fp = fp;

    errno = nError;
    return poHandle;
}

/************************************************************************/
/*                                Stat()                                */
/************************************************************************/

int VSIUnixStdioFilesystemHandler::Stat( const char * pszFilename, 
                                         VSIStatBufL * pStatBuf )

{
    return( VSI_STAT64( pszFilename, pStatBuf ) );
}

/************************************************************************/
/*                               Unlink()                               */
/************************************************************************/

int VSIUnixStdioFilesystemHandler::Unlink( const char * pszFilename )

{
    return unlink( pszFilename );
}

/************************************************************************/
/*                               Rename()                               */
/************************************************************************/

int VSIUnixStdioFilesystemHandler::Rename( const char *oldpath,
                                           const char *newpath )

{
    return rename( oldpath, newpath );
}

/************************************************************************/
/*                               Mkdir()                                */
/************************************************************************/

int VSIUnixStdioFilesystemHandler::Mkdir( const char * pszPathname,
                                          long nMode )

{
    return mkdir( pszPathname, nMode );
}

/************************************************************************/
/*                               Rmdir()                                */
/************************************************************************/

int VSIUnixStdioFilesystemHandler::Rmdir( const char * pszPathname )

{
    return rmdir( pszPathname );
}

/************************************************************************/
/*                              ReadDir()                               */
/************************************************************************/

char **VSIUnixStdioFilesystemHandler::ReadDir( const char *pszPath )

{
    DIR           *hDir;
    struct dirent *psDirEntry;
    char          **papszDir = NULL;

    if (strlen(pszPath) == 0)
        pszPath = ".";

    if ( (hDir = opendir(pszPath)) != NULL )
    {
        /* In case of really big number of files in the directory, CSLAddString */
        /* can be slow (see #2158). We then directly build the list. */
        int nItems=0;
        int nAllocatedItems=0;
        while( (psDirEntry = readdir(hDir)) != NULL )
        {
            if (nItems == 0)
            {
                papszDir = (char**) CPLCalloc(2,sizeof(char*));
                nAllocatedItems = 1;
            }
            else if (nItems >= nAllocatedItems)
            {
                nAllocatedItems = nAllocatedItems * 2;
                papszDir = (char**)CPLRealloc(papszDir, 
                                              (nAllocatedItems+2)*sizeof(char*));
            }

            papszDir[nItems] = CPLStrdup(psDirEntry->d_name);
            papszDir[nItems+1] = NULL;

            nItems++;
        }

        closedir( hDir );
    }
    else
    {
        /* Should we generate an error???  
         * For now we'll just return NULL (at the end of the function)
         */
    }

    return papszDir;
}

/************************************************************************/
/*                     VSIInstallLargeFileHandler()                     */
/************************************************************************/

void VSIInstallLargeFileHandler()

{
    VSIFileManager::InstallHandler( "", new VSIUnixStdioFilesystemHandler );
}

#endif /* ndef WIN32 */
