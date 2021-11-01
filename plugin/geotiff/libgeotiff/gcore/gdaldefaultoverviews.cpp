/******************************************************************************
 * $Id: gdaldefaultoverviews.cpp 14902 2008-07-12 15:46:11Z warmerdam $
 *
 * Project:  GDAL Core
 * Purpose:  Helper code to implement overview and mask support for many 
 *           drivers with no inherent format support.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2000, 2007, Frank Warmerdam
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ****************************************************************************/

#include "gdal_priv.h"
#include "../port/cpl_string.h"

CPL_CVSID("$Id: gdaldefaultoverviews.cpp 14902 2008-07-12 15:46:11Z warmerdam $");

/************************************************************************/
/*                        GDALDefaultOverviews()                        */
/************************************************************************/

GDALDefaultOverviews::GDALDefaultOverviews()

{
    poDS = NULL;
    poODS = NULL;
    bOvrIsAux = FALSE;

    bCheckedForMask = FALSE;
    poMaskDS = NULL;

    bOwnMaskDS = FALSE;
    poBaseDS = NULL;
}

/************************************************************************/
/*                       ~GDALDefaultOverviews()                        */
/************************************************************************/

GDALDefaultOverviews::~GDALDefaultOverviews()

{
    if( poODS != NULL )
    {
        poODS->FlushCache();
        GDALClose( poODS );
        poODS = NULL;
    }

    if( poMaskDS != NULL )
    {
        if( bOwnMaskDS )
        {
            poMaskDS->FlushCache();
            GDALClose( poMaskDS );
        }
        poMaskDS = NULL;
    }
}

/************************************************************************/
/*                             Initialize()                             */
/************************************************************************/

void GDALDefaultOverviews::Initialize( GDALDataset *poDSIn,
                                       const char * pszBasename,
                                       char **papszSiblingFiles,
                                       int bNameIsOVR )

{
/* -------------------------------------------------------------------- */
/*      If we were already initialized, destroy the old overview        */
/*      file handle.                                                    */
/* -------------------------------------------------------------------- */
    if( poODS != NULL )
    {
        GDALClose( poODS );
        poODS = NULL;

        CPLDebug( "GDAL", "GDALDefaultOverviews::Initialize() called twice - this is odd and perhaps dangerous!" );
    }

/* -------------------------------------------------------------------- */
/*      Open overview dataset if it exists.                             */
/* -------------------------------------------------------------------- */
    int bExists;

    poDS = poDSIn;
    
    if( pszBasename == NULL )
        pszBasename = poDS->GetDescription();

    if( bNameIsOVR )
        osOvrFilename = pszBasename;
    else
        osOvrFilename.Printf( "%s.ovr", pszBasename );

    bExists = CPLCheckForFile( (char *) osOvrFilename.c_str(), 
                               papszSiblingFiles );

#if !defined(WIN32)
    if( !bNameIsOVR && !bExists && !papszSiblingFiles )
    {
        osOvrFilename.Printf( "%s.OVR", pszBasename );
        bExists = CPLCheckForFile( (char *) osOvrFilename.c_str(), 
                                   papszSiblingFiles );
        if( !bExists )
            osOvrFilename.Printf( "%s.ovr", pszBasename );
    }
#endif

    if( bExists )
    {
        poODS = (GDALDataset *) GDALOpen( osOvrFilename, poDS->GetAccess() );
    }

/* -------------------------------------------------------------------- */
/*      We didn't find that, so try and find a corresponding aux        */
/*      file.  Check that we are the dependent file of the aux          */
/*      file.                                                           */
/* -------------------------------------------------------------------- */
    if( !poODS )
    {
        poODS = GDALFindAssociatedAuxFile( pszBasename, poDS->GetAccess(),
                                           poDS );

        if( poODS )
        {
            bOvrIsAux = TRUE;
            osOvrFilename = poODS->GetDescription();
        }
    }

/* -------------------------------------------------------------------- */
/*      If we have an overview dataset, then mark all the overviews     */
/*      with the base dataset  Used later for finding overviews         */
/*      masks.  Uggg.                                                   */
/* -------------------------------------------------------------------- */
    if( poODS )
    {
        int nOverviewCount = GetOverviewCount(1);
        int iOver;

        for( iOver = 0; iOver < nOverviewCount; iOver++ )
        {
            GDALRasterBand *poBand = GetOverview( 1, iOver );
            GDALDataset    *poOverDS = NULL;

            if( poBand != NULL )
                poOverDS = poBand->GetDataset();
            
            if( poOverDS != NULL )
            {
                poOverDS->oOvManager.poBaseDS = poDSIn;
                poOverDS->oOvManager.poDS = poOverDS;
            }
        }
    }

/* -------------------------------------------------------------------- */
/*      If we have sibling files, we should try to find the mask        */
/*      file now, while we still have the list.                         */
/* -------------------------------------------------------------------- */
    if( papszSiblingFiles )
        HaveMaskFile( papszSiblingFiles, pszBasename );
}

/************************************************************************/
/*                          GetOverviewCount()                          */
/************************************************************************/

int GDALDefaultOverviews::GetOverviewCount( int nBand )

{
    GDALRasterBand * poBand;

    if( poODS == NULL || nBand < 1 || nBand > poODS->GetRasterCount() )
        return 0;

    poBand = poODS->GetRasterBand( nBand );
    if( poBand == NULL )
        return 0;
    else
    {
        if( bOvrIsAux )
            return poBand->GetOverviewCount();
        else
            return poBand->GetOverviewCount() + 1;
    }
}

/************************************************************************/
/*                            GetOverview()                             */
/************************************************************************/

GDALRasterBand *
GDALDefaultOverviews::GetOverview( int nBand, int iOverview )

{
    GDALRasterBand * poBand;

    if( poODS == NULL || nBand < 1 || nBand > poODS->GetRasterCount() )
        return NULL;

    poBand = poODS->GetRasterBand( nBand );
    if( poBand == NULL )
        return NULL;

    if( bOvrIsAux )
        return poBand->GetOverview( iOverview );

    else // TIFF case, base is overview 0.
    {
        if( iOverview == 0 )
            return poBand;
        else if( iOverview-1 >= poBand->GetOverviewCount() )
            return NULL;
        else
            return poBand->GetOverview( iOverview-1 );
    }
}

/************************************************************************/
/*                         GDALOvLevelAdjust()                          */
/*                                                                      */
/*      Some overview levels cannot be achieved closely enough to be    */
/*      recognised as the desired overview level.  This function        */
/*      will adjust an overview level to one that is achievable on      */
/*      the given raster size.                                          */
/*                                                                      */
/*      For instance a 1200x1200 image on which a 256 level overview    */
/*      is request will end up generating a 5x5 overview.  However,     */
/*      this will appear to the system be a level 240 overview.         */
/*      This function will adjust 256 to 240 based on knowledge of      */
/*      the image size.                                                 */
/************************************************************************/

int GDALOvLevelAdjust( int nOvLevel, int nXSize )

{
    int nOXSize = (nXSize + nOvLevel - 1) / nOvLevel;
    
    return (int) (0.5 + nXSize / (double) nOXSize);
}
    
/************************************************************************/
/*                     GDALDefaultBuildOverviews()                      */
/************************************************************************/

CPLErr
GDALDefaultOverviews::BuildOverviews( 
    const char * pszBasename,
    const char * pszResampling, 
    int nOverviews, int * panOverviewList,
    int nBands, int * panBandList,
    GDALProgressFunc pfnProgress, void * pProgressData)

{
    GDALRasterBand **pahBands;
    CPLErr       eErr;
    int          i;

/* -------------------------------------------------------------------- */
/*      If we don't already have an overview file, we need to decide    */
/*      what format to use.                                             */
/* -------------------------------------------------------------------- */
    if( poODS == NULL )
    {
        bOvrIsAux = CSLTestBoolean(CPLGetConfigOption( "USE_RRD", "NO" ));
        if( bOvrIsAux )
        {
            VSIStatBufL sStatBuf;

            osOvrFilename = CPLResetExtension(poDS->GetDescription(),"aux");

            if( VSIStatL( osOvrFilename, &sStatBuf ) == 0 )
                osOvrFilename.Printf( "%s.aux", poDS->GetDescription() );
        }
    }
/* -------------------------------------------------------------------- */
/*      If we already have the overviews open, but they are             */
/*      read-only, then try and reopen them read-write.                 */
/* -------------------------------------------------------------------- */
    else if( poODS->GetAccess() == GA_ReadOnly )
    {
        GDALClose( poODS );
        poODS = (GDALDataset *) GDALOpen( osOvrFilename, GA_Update );
        if( poODS == NULL )
            return CE_Failure;
    }

/* -------------------------------------------------------------------- */
/*      Our TIFF overview support currently only works safely if all    */
/*      bands are handled at the same time.                             */
/* -------------------------------------------------------------------- */
    if( !bOvrIsAux && nBands != poDS->GetRasterCount() )
    {
        CPLError( CE_Failure, CPLE_NotSupported,
                  "Generation of overviews in external TIFF currently only"
                  " supported when operating on all bands.\n" 
                  "Operation failed.\n" );
        return CE_Failure;
    }

/* -------------------------------------------------------------------- */
/*      If a basename is provided, use it to override the internal      */
/*      overview filename.                                              */
/* -------------------------------------------------------------------- */
    if( pszBasename == NULL && osOvrFilename.length() == 0  )
        pszBasename = poDS->GetDescription();

    if( pszBasename != NULL )
    {
        if( bOvrIsAux )
            osOvrFilename.Printf( "%s.aux", pszBasename );
        else
            osOvrFilename.Printf( "%s.ovr", pszBasename );
    }

/* -------------------------------------------------------------------- */
/*      Establish which of the overview levels we already have, and     */
/*      which are new.  We assume that band 1 of the file is            */
/*      representative.                                                 */
/* -------------------------------------------------------------------- */
    int   nNewOverviews, *panNewOverviewList = NULL;
    GDALRasterBand *poBand = poDS->GetRasterBand( 1 );

    nNewOverviews = 0;
    panNewOverviewList = (int *) CPLCalloc(sizeof(int),nOverviews);
    for( i = 0; i < nOverviews && poBand != NULL; i++ )
    {
        int   j;

        for( j = 0; j < poBand->GetOverviewCount(); j++ )
        {
            int    nOvFactor;
            GDALRasterBand * poOverview = poBand->GetOverview( j );
 
            nOvFactor = (int) 
                (0.5 + poBand->GetXSize() / (double) poOverview->GetXSize());

            if( nOvFactor == panOverviewList[i] 
                || nOvFactor == GDALOvLevelAdjust( panOverviewList[i], 
                                                   poBand->GetXSize() ) )
                panOverviewList[i] *= -1;
        }

        if( panOverviewList[i] > 0 )
            panNewOverviewList[nNewOverviews++] = panOverviewList[i];
    }

/* -------------------------------------------------------------------- */
/*      Build band list.                                                */
/* -------------------------------------------------------------------- */
    pahBands = (GDALRasterBand **) CPLCalloc(sizeof(GDALRasterBand *),nBands);
    for( i = 0; i < nBands; i++ )
        pahBands[i] = poDS->GetRasterBand( panBandList[i] );

/* -------------------------------------------------------------------- */
/*      Build new overviews - Imagine.  Keep existing file open if      */
/*      we have it.  But mark all overviews as in need of               */
/*      regeneration, since HFAAuxBuildOverviews() doesn't actually     */
/*      produce the imagery.                                            */
/* -------------------------------------------------------------------- */

#ifndef WIN32CE

    if( bOvrIsAux )
    {
        eErr = HFAAuxBuildOverviews( osOvrFilename, poDS, &poODS,
                                     nBands, panBandList,
                                     nNewOverviews, panNewOverviewList, 
                                     pszResampling, 
                                     pfnProgress, pProgressData );

        int j;
        
        for( j = 0; j < nOverviews; j++ )
        {
            if( panOverviewList[j] > 0 )
                panOverviewList[j] *= -1;
        }
    }

/* -------------------------------------------------------------------- */
/*      Build new overviews - TIFF.  Close TIFF files while we          */
/*      operate on it.                                                  */
/* -------------------------------------------------------------------- */
    else
#endif /* WIN32CE */
    {
        if( poODS != NULL )
        {
            delete poODS;
            poODS = NULL;
        }

        eErr = GTIFFBuildOverviews( osOvrFilename, nBands, pahBands, 
                                    nNewOverviews, panNewOverviewList, 
                                    pszResampling, pfnProgress, pProgressData );

        if( eErr == CE_None )
        {
            poODS = (GDALDataset *) GDALOpen( osOvrFilename, GA_Update );
            if( poODS == NULL )
                eErr = CE_Failure;
        }
    }

/* -------------------------------------------------------------------- */
/*      Refresh old overviews that were listed.                         */
/* -------------------------------------------------------------------- */
    GDALRasterBand **papoOverviewBands;

    papoOverviewBands = (GDALRasterBand **) 
        CPLCalloc(sizeof(void*),nOverviews);

    for( int iBand = 0; iBand < nBands && eErr == CE_None; iBand++ )
    {
        poBand = poDS->GetRasterBand( panBandList[iBand] );

        nNewOverviews = 0;
        for( i = 0; i < nOverviews && poBand != NULL; i++ )
        {
            int   j;
            
            for( j = 0; j < poBand->GetOverviewCount(); j++ )
            {
                int    nOvFactor;
                GDALRasterBand * poOverview = poBand->GetOverview( j );

                int bHasNoData;
                double noDataValue = poBand->GetNoDataValue(&bHasNoData);

                if (bHasNoData)
                  poOverview->SetNoDataValue(noDataValue);

                nOvFactor = (int) 
                    (0.5 + poBand->GetXSize() / (double) poOverview->GetXSize());

                if( nOvFactor == - panOverviewList[i] 
                    || nOvFactor == GDALOvLevelAdjust( -panOverviewList[i], 
                                                       poBand->GetXSize() ) )
                {
                    //panOverviewList[i] *= -1;
                    papoOverviewBands[nNewOverviews++] = poOverview;
                }
            }
        }

        if( nNewOverviews > 0 )
        {
            eErr = GDALRegenerateOverviews( (GDALRasterBandH) poBand, 
                                            nNewOverviews, 
                                            (GDALRasterBandH*)papoOverviewBands,
                                            pszResampling, 
                                            pfnProgress, pProgressData );
        }
    }

/* -------------------------------------------------------------------- */
/*      Cleanup                                                         */
/* -------------------------------------------------------------------- */
    CPLFree( papoOverviewBands );
    CPLFree( panNewOverviewList );
    CPLFree( pahBands );

/* -------------------------------------------------------------------- */
/*      If we have a mask file, we need to build it's overviews         */
/*      too.                                                            */
/* -------------------------------------------------------------------- */
    if( HaveMaskFile() && poMaskDS )
    {
        poMaskDS->BuildOverviews( pszResampling, nOverviews, panOverviewList,
                                  0, NULL, pfnProgress, pProgressData );
        if( bOwnMaskDS )
            GDALClose( poMaskDS );

        // force next request to reread mask file.
        poMaskDS = NULL;
        bOwnMaskDS = FALSE;
        bCheckedForMask = FALSE;
    }

/* -------------------------------------------------------------------- */
/*      If we have an overview dataset, then mark all the overviews     */
/*      with the base dataset  Used later for finding overviews         */
/*      masks.  Uggg.                                                   */
/* -------------------------------------------------------------------- */
    if( poODS )
    {
        int nOverviewCount = GetOverviewCount(1);
        int iOver;

        for( iOver = 0; iOver < nOverviewCount; iOver++ )
        {
            GDALRasterBand *poBand = GetOverview( 1, iOver );
            GDALDataset    *poOverDS = NULL;

            if( poBand != NULL )
                poOverDS = poBand->GetDataset();

            if (poOverDS != NULL)
            {
                poOverDS->oOvManager.poBaseDS = poDS;
                poOverDS->oOvManager.poDS = poOverDS;
            }
        }
    }

    return eErr;
}

/************************************************************************/
/*                           CreateMaskBand()                           */
/************************************************************************/

CPLErr GDALDefaultOverviews::CreateMaskBand( int nFlags, int nBand )

{
    if( nBand < 1 )
        nFlags |= GMF_PER_DATASET;

/* -------------------------------------------------------------------- */
/*      ensure existing file gets opened if there is one.               */
/* -------------------------------------------------------------------- */
    HaveMaskFile();

/* -------------------------------------------------------------------- */
/*      Try creating the mask file.                                     */
/* -------------------------------------------------------------------- */
    if( poMaskDS == NULL )
    {
        CPLString osMskFilename;
        GDALDriver *poDr = (GDALDriver *) GDALGetDriverByName( "GTiff" );
        char **papszOpt = NULL;
        int  nBX, nBY;
        int  nBands;
        
        if( poDr == NULL )
            return CE_Failure;

        GDALRasterBand *poTBand = poDS->GetRasterBand(1);
        if( poTBand == NULL )
            return CE_Failure;

        if( nFlags & GMF_PER_DATASET )
            nBands = 1;
        else
            nBands = poDS->GetRasterCount();


        papszOpt = CSLSetNameValue( papszOpt, "COMPRESS", "DEFLATE" );
        papszOpt = CSLSetNameValue( papszOpt, "INTERLEAVE", "BAND" );

        poTBand->GetBlockSize( &nBX, &nBY );

        // try to create matching tile size if legal in TIFF.
        if( (nBX % 16) == 0 && (nBY % 16) == 0 )
        {
            papszOpt = CSLSetNameValue( papszOpt, "TILED", "YES" );
            papszOpt = CSLSetNameValue( papszOpt, "BLOCKXSIZE",
                                        CPLString().Printf("%d",nBX) );
            papszOpt = CSLSetNameValue( papszOpt, "BLOCKYSIZE",
                                        CPLString().Printf("%d",nBY) );
        }

        osMskFilename.Printf( "%s.msk", poDS->GetDescription() );
        poMaskDS = poDr->Create( osMskFilename, 
                                 poDS->GetRasterXSize(),
                                 poDS->GetRasterYSize(),
                                 nBands, GDT_Byte, papszOpt );
        CSLDestroy( papszOpt );

        if( poMaskDS == NULL ) // presumably error already issued.
            return CE_Failure;

        bOwnMaskDS = TRUE;
    }
        
/* -------------------------------------------------------------------- */
/*      Save the mask flags for this band.                              */
/* -------------------------------------------------------------------- */
    if( nBand > poMaskDS->GetRasterCount() )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Attempt to create a mask band for band %d of %s,\n"
                  "but the .msk file has a PER_DATASET mask.", 
                  nBand, poDS->GetDescription() );
        return CE_Failure;
    }
    
    int iBand; 

    for( iBand = 0; iBand < poDS->GetRasterCount(); iBand++ )
    {
        // we write only the info for this band, unless we are
        // using PER_DATASET in which case we write for all.
        if( nBand != iBand + 1 && !(nFlags | GMF_PER_DATASET) )
            continue;

        poMaskDS->SetMetadataItem( 
            CPLString().Printf("INTERNAL_MASK_FLAGS_%d", iBand+1 ),
            CPLString().Printf("%d", nFlags ) );
    }

    return CE_None;
}

/************************************************************************/
/*                            GetMaskBand()                             */
/************************************************************************/

GDALRasterBand *GDALDefaultOverviews::GetMaskBand( int nBand )

{
    int nFlags = GetMaskFlags( nBand );

    if( nFlags == 0x8000 ) // secret code meaning we don't handle this band.
        return NULL;
        
    if( nFlags & GMF_PER_DATASET )
        return poMaskDS->GetRasterBand(1);

    if( nBand > 0 )
        return poMaskDS->GetRasterBand( nBand );
    else 
        return NULL;
}

/************************************************************************/
/*                            GetMaskFlags()                            */
/************************************************************************/

int GDALDefaultOverviews::GetMaskFlags( int nBand )

{
/* -------------------------------------------------------------------- */
/*      Fetch this band's metadata entry.  They are of the form:        */
/*        INTERNAL_MASK_FLAGS_n: flags                                  */
/* -------------------------------------------------------------------- */
    if( !HaveMaskFile() )
        return 0;
    
    const char *pszValue = 
        poMaskDS->GetMetadataItem( 
            CPLString().Printf( "INTERNAL_MASK_FLAGS_%d", MAX(nBand,1)) );

    if( pszValue == NULL )
        return 0x8000;
    else
        return atoi(pszValue);
}

/************************************************************************/
/*                            HaveMaskFile()                            */
/*                                                                      */
/*      Check for a mask file if we haven't already done so.            */
/*      Returns TRUE if we have one, otherwise FALSE.                   */
/************************************************************************/

int GDALDefaultOverviews::HaveMaskFile( char ** papszSiblingFiles,
                                        const char *pszBasename )

{
/* -------------------------------------------------------------------- */
/*      Have we already checked for masks?                              */
/* -------------------------------------------------------------------- */
    if( bCheckedForMask )
        return poMaskDS != NULL;

/* -------------------------------------------------------------------- */
/*      Are we an overview?  If so we need to find the corresponding    */
/*      overview in the base files mask file (if there is one).         */
/* -------------------------------------------------------------------- */
    if( poBaseDS != NULL && poBaseDS->oOvManager.HaveMaskFile() )
    {
        int iOver, nOverviewCount = 0;
        GDALRasterBand *poBaseBand = poBaseDS->GetRasterBand(1);
        GDALRasterBand *poBaseMask = NULL;

        if( poBaseBand != NULL )
            poBaseMask = poBaseBand->GetMaskBand();
        if( poBaseMask )
            nOverviewCount = poBaseMask->GetOverviewCount();

        for( iOver = 0; iOver < nOverviewCount; iOver++ )
        {
            GDALRasterBand *poOverBand = poBaseMask->GetOverview( iOver );
            
            if( poOverBand->GetXSize() == poDS->GetRasterXSize() 
                && poOverBand->GetYSize() == poDS->GetRasterYSize() )
            {
                poMaskDS = poOverBand->GetDataset();
                break;
            }
        }

        bCheckedForMask = TRUE;
        bOwnMaskDS = FALSE;
        
        CPLAssert( poMaskDS != poDS );

        return poMaskDS != NULL;
    }

/* -------------------------------------------------------------------- */
/*      Are we even initialized?  If not, we apparently don't want      */
/*      to support overviews and masks.                                 */
/* -------------------------------------------------------------------- */
    if( !IsInitialized() )
        return FALSE;

/* -------------------------------------------------------------------- */
/*      Check for .msk file.                                            */
/* -------------------------------------------------------------------- */
    CPLString osMskFilename;
    bCheckedForMask = TRUE;

    if( pszBasename == NULL )
        pszBasename = poDS->GetDescription();

    // Don't bother checking for masks of masks. 
    if( EQUAL(CPLGetExtension(pszBasename),".msk") )
        return FALSE;

    osMskFilename.Printf( "%s.msk", pszBasename );

    int bExists = CPLCheckForFile( (char *) osMskFilename.c_str(), 
                                   papszSiblingFiles );

#if !defined(WIN32)
    if( !bExists && !papszSiblingFiles )
    {
        osMskFilename.Printf( "%s.MSK", pszBasename );
        bExists = CPLCheckForFile( (char *) osMskFilename.c_str(), 
                                   papszSiblingFiles );
    }
#endif

    if( !bExists )
        return FALSE;

/* -------------------------------------------------------------------- */
/*      Open the file.                                                  */
/* -------------------------------------------------------------------- */
    poMaskDS = (GDALDataset *) GDALOpen( osMskFilename, poDS->GetAccess() );
    CPLAssert( poMaskDS != poDS );

    if( poMaskDS == NULL )
        return FALSE;

    bOwnMaskDS = TRUE;
    
    return TRUE;
}
