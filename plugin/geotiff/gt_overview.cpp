/******************************************************************************
 * $Id: gt_overview.cpp 15787 2008-11-22 00:42:05Z warmerdam $
 *
 * Project:  GeoTIFF Driver
 * Purpose:  Code to build overviews of external databases as a TIFF file. 
 *           Only used by the GDALDefaultOverviews::BuildOverviews() method.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2000, Frank Warmerdam
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

#include "libgeotiff/gcore/gdal_priv.h"
#define CPL_SERV_H_INCLUDED

#include <plugin/tif/lib/tiffio.h>
#include "libgeotiff/xtiffio.h"
#include "libgeotiff/geotiff.h"
#include "gt_overview.h"

CPL_CVSID("$Id: gt_overview.cpp 15787 2008-11-22 00:42:05Z warmerdam $");

#define TIFFTAG_GDAL_METADATA  42112

CPL_C_START
void    GTiffOneTimeInit();
CPL_C_END

/************************************************************************/
/*                         GTIFFWriteDirectory()                        */
/*                                                                      */
/*      Create a new directory, without any image data for an overview  */
/*      or a mask                                                       */
/*      Returns offset of newly created directory, but the              */
/*      current directory is reset to be the one in used when this      */
/*      function is called.                                             */
/************************************************************************/

long GTIFFWriteDirectory(TIFF *hTIFF, int nSubfileType, int nXSize, int nYSize,
                           int nBitsPerPixel, int nPlanarConfig, int nSamples, 
                           int nBlockXSize, int nBlockYSize,
                           int bTiled, int nCompressFlag, int nPhotometric,
                           int nSampleFormat, 
                           unsigned short *panRed,
                           unsigned short *panGreen,
                           unsigned short *panBlue,
                           int nExtraSamples,
                           unsigned short *panExtraSampleValues,
                           const char *pszMetadata )

{
    toff_t	nBaseDirOffset;
    toff_t	nOffset;

    nBaseDirOffset = TIFFCurrentDirOffset( hTIFF );

#if defined(TIFFLIB_VERSION) && TIFFLIB_VERSION >= 20051201 /* 3.8.0 */
    TIFFFreeDirectory( hTIFF );
#endif

    TIFFCreateDirectory( hTIFF );
    
/* -------------------------------------------------------------------- */
/*      Setup TIFF fields.                                              */
/* -------------------------------------------------------------------- */
    TIFFSetField( hTIFF, TIFFTAG_IMAGEWIDTH, nXSize );
    TIFFSetField( hTIFF, TIFFTAG_IMAGELENGTH, nYSize );
    if( nSamples == 1 )
        TIFFSetField( hTIFF, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG );
    else
        TIFFSetField( hTIFF, TIFFTAG_PLANARCONFIG, nPlanarConfig );

    TIFFSetField( hTIFF, TIFFTAG_BITSPERSAMPLE, nBitsPerPixel );
    TIFFSetField( hTIFF, TIFFTAG_SAMPLESPERPIXEL, nSamples );
    TIFFSetField( hTIFF, TIFFTAG_COMPRESSION, nCompressFlag );
    TIFFSetField( hTIFF, TIFFTAG_PHOTOMETRIC, nPhotometric );
    TIFFSetField( hTIFF, TIFFTAG_SAMPLEFORMAT, nSampleFormat );

    if( bTiled )
    {
        TIFFSetField( hTIFF, TIFFTAG_TILEWIDTH, nBlockXSize );
        TIFFSetField( hTIFF, TIFFTAG_TILELENGTH, nBlockYSize );
    }
    else
        TIFFSetField( hTIFF, TIFFTAG_ROWSPERSTRIP, nBlockYSize );

    TIFFSetField( hTIFF, TIFFTAG_SUBFILETYPE, nSubfileType );

    if (panExtraSampleValues != NULL)
    {
        TIFFSetField(hTIFF, TIFFTAG_EXTRASAMPLES, nExtraSamples, panExtraSampleValues );
    }
    
/* -------------------------------------------------------------------- */
/*	Write color table if one is present.				*/
/* -------------------------------------------------------------------- */
    if( panRed != NULL )
    {
        TIFFSetField( hTIFF, TIFFTAG_COLORMAP, panRed, panGreen, panBlue );
    }

/* -------------------------------------------------------------------- */
/*      Write metadata if we have some.                                 */
/* -------------------------------------------------------------------- */
    if( pszMetadata && strlen(pszMetadata) > 0 )
        TIFFSetField( hTIFF, TIFFTAG_GDAL_METADATA, pszMetadata );

/* -------------------------------------------------------------------- */
/*      Write directory, and return byte offset.                        */
/* -------------------------------------------------------------------- */
    if( TIFFWriteCheck( hTIFF, bTiled, "GTIFFWriteDirectory" ) == 0 )
    {
        TIFFSetSubDirectory( hTIFF, nBaseDirOffset );
        return 0;
    }

    TIFFWriteDirectory( hTIFF );
    TIFFSetDirectory( hTIFF, (tdir_t) (TIFFNumberOfDirectories(hTIFF)-1) );

    nOffset = TIFFCurrentDirOffset( hTIFF );

    TIFFSetSubDirectory( hTIFF, nBaseDirOffset );

    return nOffset;
}

/************************************************************************/
/*                     GTIFFBuildOverviewMetadata()                     */
/************************************************************************/

void GTIFFBuildOverviewMetadata( const char *pszResampling,
                                 GDALDataset *poBaseDS, 
                                 CPLString &osMetadata )

{
    osMetadata = "<GDALMetadata>";

    if( pszResampling && EQUALN(pszResampling,"AVERAGE_BIT2",12) )
        osMetadata += "<Item name=\"RESAMPLING\" sample=\"0\">AVERAGE_BIT2GRAYSCALE</Item>";

    if( poBaseDS->GetMetadataItem( "INTERNAL_MASK_FLAGS_1" ) )
    {
        int iBand;

        for( iBand = 0; iBand < 200; iBand++ )
        {
            CPLString osItem;
            CPLString osName;

            osName.Printf( "INTERNAL_MASK_FLAGS_%d", iBand+1 );
            if( poBaseDS->GetMetadataItem( osName ) )
            {
                osItem.Printf( "<Item name=\"%s\">%s</Item>", 
                               osName.c_str(), 
                               poBaseDS->GetMetadataItem( osName ) );
                osMetadata += osItem;
            }
        }
    }

    const char* pszNoDataValues = poBaseDS->GetMetadataItem("NODATA_VALUES");
    if (pszNoDataValues)
    {
        CPLString osItem;
        osItem.Printf( "<Item name=\"NODATA_VALUES\">%s</Item>", pszNoDataValues );
        osMetadata += osItem;
    }

    if( !EQUAL(osMetadata,"<GDALMetadata>") )
        osMetadata += "</GDALMetadata>";
    else
        osMetadata = "";
}

/************************************************************************/
/*                        GTIFFBuildOverviews()                         */
/************************************************************************/

CPLErr 
GTIFFBuildOverviews( const char * pszFilename,
                     int nBands, GDALRasterBand **papoBandList, 
                     int nOverviews, int * panOverviewList,
                     const char * pszResampling, 
                     GDALProgressFunc pfnProgress, void * pProgressData )

{
    TIFF    *hOTIFF;
    int     nBitsPerPixel=0, nCompression=COMPRESSION_NONE, nPhotometric=0;
    int     nSampleFormat=0, nPlanarConfig, iOverview, iBand;
    int     nXSize=0, nYSize=0;

    if( nBands == 0 || nOverviews == 0 )
        return CE_None;

    GTiffOneTimeInit();

/* -------------------------------------------------------------------- */
/*      Verify that the list of bands is suitable for emitting in       */
/*      TIFF file.                                                      */
/* -------------------------------------------------------------------- */
    for( iBand = 0; iBand < nBands; iBand++ )
    {
        int     nBandBits, nBandFormat;
        GDALRasterBand *hBand = papoBandList[iBand];

        switch( hBand->GetRasterDataType() )
        {
          case GDT_Byte:
            nBandBits = 8;
            nBandFormat = SAMPLEFORMAT_UINT;
            break;

          case GDT_UInt16:
            nBandBits = 16;
            nBandFormat = SAMPLEFORMAT_UINT;
            break;

          case GDT_Int16:
            nBandBits = 16;
            nBandFormat = SAMPLEFORMAT_INT;
            break;

          case GDT_UInt32:
            nBandBits = 32;
            nBandFormat = SAMPLEFORMAT_UINT;
            break;

          case GDT_Int32:
            nBandBits = 32;
            nBandFormat = SAMPLEFORMAT_INT;
            break;

          case GDT_Float32:
            nBandBits = 32;
            nBandFormat = SAMPLEFORMAT_IEEEFP;
            break;

          case GDT_Float64:
            nBandBits = 64;
            nBandFormat = SAMPLEFORMAT_IEEEFP;
            break;

          case GDT_CInt16:
            nBandBits = 32;
            nBandFormat = SAMPLEFORMAT_COMPLEXINT;
            break;

          case GDT_CFloat32:
            nBandBits = 64;
            nBandFormat = SAMPLEFORMAT_COMPLEXIEEEFP;
            break;

          case GDT_CFloat64:
            nBandBits = 128;
            nBandFormat = SAMPLEFORMAT_COMPLEXIEEEFP;
            break;

          default:
            CPLAssert( FALSE );
            return CE_Failure;
        }

        if( iBand == 0 )
        {
            nBitsPerPixel = nBandBits;
            nSampleFormat = nBandFormat;
            nXSize = hBand->GetXSize();
            nYSize = hBand->GetYSize();
        }
        else if( nBitsPerPixel != nBandBits || nSampleFormat != nBandFormat )
        {
            CPLError( CE_Failure, CPLE_NotSupported, 
                      "GTIFFBuildOverviews() doesn't support a mixture of band"
                      " data types." );
            return CE_Failure;
        }
        else if( hBand->GetColorTable() != NULL )
        {
            CPLError( CE_Failure, CPLE_NotSupported, 
                      "GTIFFBuildOverviews() doesn't support building"
                      " overviews of multiple colormapped bands." );
            return CE_Failure;
        }
        else if( hBand->GetXSize() != nXSize 
                 || hBand->GetYSize() != nYSize )
        {
            CPLError( CE_Failure, CPLE_NotSupported, 
                      "GTIFFBuildOverviews() doesn't support building"
                      " overviews of different sized bands." );
            return CE_Failure;
        }
    }

/* -------------------------------------------------------------------- */
/*      Use specified compression method.                               */
/* -------------------------------------------------------------------- */
    const char *pszCompress = CPLGetConfigOption( "COMPRESS_OVERVIEW", NULL );

    if( pszCompress != NULL && pszCompress[0] != '\0' )
    {
        if( EQUAL( pszCompress, "JPEG" ) )
            nCompression = COMPRESSION_JPEG;
        else if( EQUAL( pszCompress, "LZW" ) )
            nCompression = COMPRESSION_LZW;
        else if( EQUAL( pszCompress, "PACKBITS" ))
            nCompression = COMPRESSION_PACKBITS;
        else if( EQUAL( pszCompress, "DEFLATE" ) || EQUAL( pszCompress, "ZIP" ))
            nCompression = COMPRESSION_ADOBE_DEFLATE;
        else
            CPLError( CE_Warning, CPLE_IllegalArg, 
                      "COMPRESS_OVERVIEW=%s value not recognised, ignoring.",
                      pszCompress );
    }

/* -------------------------------------------------------------------- */
/*      Figure out the planar configuration to use.                     */
/* -------------------------------------------------------------------- */
    if( nBands == 1 )
        nPlanarConfig = PLANARCONFIG_CONTIG;
    else
        nPlanarConfig = PLANARCONFIG_SEPARATE;

    const char* pszInterleave = CPLGetConfigOption( "INTERLEAVE_OVERVIEW", NULL );
    if (pszInterleave != NULL && pszInterleave[0] != '\0')
    {
        if( EQUAL( pszInterleave, "PIXEL" ) )
            nPlanarConfig = PLANARCONFIG_CONTIG;
        else if( EQUAL( pszInterleave, "BAND" ) )
            nPlanarConfig = PLANARCONFIG_SEPARATE;
        else
        {
            CPLError( CE_Failure, CPLE_AppDefined, 
                      "INTERLEAVE_OVERVIEW=%s unsupported, value must be PIXEL or BAND. ignoring",
                      pszInterleave );
        }
    }

/* -------------------------------------------------------------------- */
/*      Figure out the photometric interpretation to use.               */
/* -------------------------------------------------------------------- */
    if( nBands == 3 )
        nPhotometric = PHOTOMETRIC_RGB;
    else if( papoBandList[0]->GetColorTable() != NULL 
             && !EQUALN(pszResampling,"AVERAGE_BIT2",12) )
    {
        nPhotometric = PHOTOMETRIC_PALETTE;
        /* should set the colormap up at this point too! */
    }
    else
        nPhotometric = PHOTOMETRIC_MINISBLACK;

    const char* pszPhotometric = CPLGetConfigOption( "PHOTOMETRIC_OVERVIEW", NULL );
    if (pszPhotometric != NULL && pszPhotometric[0] != '\0')
    {
        if( EQUAL( pszPhotometric, "MINISBLACK" ) )
            nPhotometric = PHOTOMETRIC_MINISBLACK;
        else if( EQUAL( pszPhotometric, "MINISWHITE" ) )
            nPhotometric = PHOTOMETRIC_MINISWHITE;
        else if( EQUAL( pszPhotometric, "RGB" ))
        {
            nPhotometric = PHOTOMETRIC_RGB;
        }
        else if( EQUAL( pszPhotometric, "CMYK" ))
        {
            nPhotometric = PHOTOMETRIC_SEPARATED;
        }
        else if( EQUAL( pszPhotometric, "YCBCR" ))
        {
            nPhotometric = PHOTOMETRIC_YCBCR;
        }
        else if( EQUAL( pszPhotometric, "CIELAB" ))
        {
            nPhotometric = PHOTOMETRIC_CIELAB;
        }
        else if( EQUAL( pszPhotometric, "ICCLAB" ))
        {
            nPhotometric = PHOTOMETRIC_ICCLAB;
        }
        else if( EQUAL( pszPhotometric, "ITULAB" ))
        {
            nPhotometric = PHOTOMETRIC_ITULAB;
        }
        else
        {
            CPLError( CE_Warning, CPLE_IllegalArg, 
                      "PHOTOMETRIC_OVERVIEW=%s value not recognised, ignoring.\n",
                      pszPhotometric );
        }
    }

/* -------------------------------------------------------------------- */
/*      Create the file, if it does not already exist.                  */
/* -------------------------------------------------------------------- */
    VSIStatBuf  sStatBuf;

    if( VSIStat( pszFilename, &sStatBuf ) != 0 )
    {
        hOTIFF = XTIFFOpen( pszFilename, "w+" );
        if( hOTIFF == NULL )
        {
            if( CPLGetLastErrorNo() == 0 )
                CPLError( CE_Failure, CPLE_OpenFailed,
                          "Attempt to create new tiff file `%s'\n"
                          "failed in XTIFFOpen().\n",
                          pszFilename );

            return CE_Failure;
        }
    }
/* -------------------------------------------------------------------- */
/*      Otherwise just open it for update access.                       */
/* -------------------------------------------------------------------- */
    else 
    {
        hOTIFF = XTIFFOpen( pszFilename, "r+" );
        if( hOTIFF == NULL )
        {
            if( CPLGetLastErrorNo() == 0 )
                CPLError( CE_Failure, CPLE_OpenFailed,
                          "Attempt to create new tiff file `%s'\n"
                          "failed in XTIFFOpen().\n",
                          pszFilename );

            return CE_Failure;
        }
    }

/* -------------------------------------------------------------------- */
/*      Do we have a palette?  If so, create a TIFF compatible version. */
/* -------------------------------------------------------------------- */
    unsigned short      *panRed=NULL, *panGreen=NULL, *panBlue=NULL;

    if( nPhotometric == PHOTOMETRIC_PALETTE )
    {
        GDALColorTable *poCT = papoBandList[0]->GetColorTable();
        int nColorCount;

        if( nBitsPerPixel <= 8 )
            nColorCount = 256;
        else
            nColorCount = 65536;

        panRed   = (unsigned short *) 
            CPLCalloc(nColorCount,sizeof(unsigned short));
        panGreen = (unsigned short *) 
            CPLCalloc(nColorCount,sizeof(unsigned short));
        panBlue  = (unsigned short *) 
            CPLCalloc(nColorCount,sizeof(unsigned short));

        for( int iColor = 0; iColor < nColorCount; iColor++ )
        {
            GDALColorEntry  sRGB;

            if( poCT->GetColorEntryAsRGB( iColor, &sRGB ) )
            {
                panRed[iColor] = (unsigned short) (257 * sRGB.c1);
                panGreen[iColor] = (unsigned short) (257 * sRGB.c2);
                panBlue[iColor] = (unsigned short) (257 * sRGB.c3);
            }
        }
    }

/* -------------------------------------------------------------------- */
/*      Do we need some metadata for the overviews?                     */
/* -------------------------------------------------------------------- */
    CPLString osMetadata;
    GDALDataset *poBaseDS = papoBandList[0]->GetDataset();

    GTIFFBuildOverviewMetadata( pszResampling, poBaseDS, osMetadata );

/* -------------------------------------------------------------------- */
/*      Loop, creating overviews.                                       */
/* -------------------------------------------------------------------- */
    for( iOverview = 0; iOverview < nOverviews; iOverview++ )
    {
        int    nOXSize, nOYSize;
        uint32 nDirOffset;

        nOXSize = (nXSize + panOverviewList[iOverview] - 1) 
            / panOverviewList[iOverview];
        nOYSize = (nYSize + panOverviewList[iOverview] - 1) 
            / panOverviewList[iOverview];

        nDirOffset = 
            GTIFFWriteDirectory(hOTIFF, FILETYPE_REDUCEDIMAGE,
                                nOXSize, nOYSize, nBitsPerPixel, 
                                nPlanarConfig, nBands,
                                128, 128, TRUE, nCompression,
                                nPhotometric, nSampleFormat, 
                                panRed, panGreen, panBlue,
                                0, NULL, /* FIXME ? how can we fetch extrasamples from here */
                                osMetadata );
    }

    if (panRed)
    {
        CPLFree(panRed);
        CPLFree(panGreen);
        CPLFree(panBlue);
        panRed = panGreen = panBlue = NULL;
    }

    XTIFFClose( hOTIFF );

/* -------------------------------------------------------------------- */
/*      Open the overview dataset so that we can get at the overview    */
/*      bands.                                                          */
/* -------------------------------------------------------------------- */
    GDALDataset *hODS;

    hODS = (GDALDataset *) GDALOpen( pszFilename, GA_Update );
    if( hODS == NULL )
        return CE_Failure;
    
/* -------------------------------------------------------------------- */
/*      Loop writing overview data.                                     */
/* -------------------------------------------------------------------- */

    if (nCompression != COMPRESSION_NONE &&
        nPlanarConfig == PLANARCONFIG_CONTIG &&
        GDALDataTypeIsComplex(papoBandList[0]->GetRasterDataType()) == FALSE &&
        papoBandList[0]->GetColorTable() == NULL &&
        (EQUALN(pszResampling, "NEAR", 4) || EQUAL(pszResampling, "AVERAGE") || EQUAL(pszResampling, "GAUSS")))
    {
        /* In the case of pixel interleaved compressed overviews, we want to generate */
        /* the overviews for all the bands block by block, and not band after band, */
        /* in order to write the block once and not loose space in the TIFF file */

        GDALRasterBand ***papapoOverviewBands;

        papapoOverviewBands = (GDALRasterBand ***) CPLCalloc(sizeof(void*),nBands);
        for( iBand = 0; iBand < nBands; iBand++ )
        {
            GDALRasterBand    *hDstBand = hODS->GetRasterBand( iBand+1 );
            papapoOverviewBands[iBand] = (GDALRasterBand **) CPLCalloc(sizeof(void*),nOverviews);
            papapoOverviewBands[iBand][0] = hDstBand;
            for( int i = 0; i < nOverviews-1; i++ )
            {
                papapoOverviewBands[iBand][i+1] = hDstBand->GetOverview(i);
            }
        }

        GDALRegenerateOverviewsMultiBand(nBands, papoBandList,
                                         nOverviews, papapoOverviewBands,
                                         pszResampling, pfnProgress, pProgressData );

        for( iBand = 0; iBand < nBands; iBand++ )
        {
            CPLFree(papapoOverviewBands[iBand]);
        }
        CPLFree(papapoOverviewBands);
    }
    else
    {
        GDALRasterBand   **papoOverviews;

        papoOverviews = (GDALRasterBand **) CPLCalloc(sizeof(void*),128);

        for( iBand = 0; iBand < nBands; iBand++ )
        {
            GDALRasterBand    *hSrcBand = papoBandList[iBand];
            GDALRasterBand    *hDstBand;
            int               nDstOverviews;
            CPLErr            eErr;

            hDstBand = hODS->GetRasterBand( iBand+1 );

            papoOverviews[0] = hDstBand;
            nDstOverviews = hDstBand->GetOverviewCount() + 1;
            CPLAssert( nDstOverviews < 128 );
            nDstOverviews = MIN(128,nDstOverviews);

            for( int i = 0; i < nDstOverviews-1; i++ )
            {
                papoOverviews[i+1] = hDstBand->GetOverview(i);
            }

            void         *pScaledProgressData;

            pScaledProgressData = 
                GDALCreateScaledProgress( iBand / (double) nBands, 
                                        (iBand+1) / (double) nBands,
                                        pfnProgress, pProgressData );

            eErr = 
                GDALRegenerateOverviews( (GDALRasterBandH) hSrcBand, 
                                        nDstOverviews, 
                                        (GDALRasterBandH *) papoOverviews, 
                                        pszResampling,
                                        GDALScaledProgress, 
                                        pScaledProgressData);

            GDALDestroyScaledProgress( pScaledProgressData );

            if( eErr != CE_None )
            {
                delete hODS;
                return eErr;
            }
        }

        CPLFree( papoOverviews );
    }

/* -------------------------------------------------------------------- */
/*      Cleanup                                                         */
/* -------------------------------------------------------------------- */
    hODS->FlushCache();
    delete hODS;

    pfnProgress( 1.0, NULL, pProgressData );

    return CE_None;
}
    
