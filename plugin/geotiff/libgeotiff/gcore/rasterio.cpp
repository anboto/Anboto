/******************************************************************************
 * $Id: rasterio.cpp 15667 2008-10-31 20:20:55Z rouault $
 *
 * Project:  GDAL Core
 * Purpose:  Contains default implementation of GDALRasterBand::IRasterIO()
 *           and supporting functions of broader utility.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 1998, Frank Warmerdam
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

CPL_CVSID("$Id: rasterio.cpp 15667 2008-10-31 20:20:55Z rouault $");

/************************************************************************/
/*                             IRasterIO()                              */
/*                                                                      */
/*      Default internal implementation of RasterIO() ... utilizes      */
/*      the Block access methods to satisfy the request.  This would    */
/*      normally only be overridden by formats with overviews.          */
/************************************************************************/

CPLErr GDALRasterBand::IRasterIO( GDALRWFlag eRWFlag,
                                  int nXOff, int nYOff, int nXSize, int nYSize,
                                  void * pData, int nBufXSize, int nBufYSize,
                                  GDALDataType eBufType,
                                  int nPixelSpace, int nLineSpace )

{
    int         nBandDataSize = GDALGetDataTypeSize( eDataType ) / 8;
    int         nBufDataSize = GDALGetDataTypeSize( eBufType ) / 8;
    GByte       *pabySrcBlock = NULL;
    GDALRasterBlock *poBlock = NULL;
    int         nLBlockX=-1, nLBlockY=-1, iBufYOff, iBufXOff, iSrcY;

/* ==================================================================== */
/*      A common case is the data requested with the destination        */
/*      is packed, and the block width is the raster width.             */
/* ==================================================================== */
    if( nPixelSpace == nBufDataSize
        && nLineSpace == nPixelSpace * nXSize
        && nBlockXSize == GetXSize()
        && nBufXSize == nXSize 
        && nBufYSize == nYSize )
    {
//        printf( "IRasterIO(%d,%d,%d,%d) rw=%d case 1\n", 
//                nXOff, nYOff, nXSize, nYSize, 
//                (int) eRWFlag );

        for( iBufYOff = 0; iBufYOff < nBufYSize; iBufYOff++ )
        {
            int         nSrcByteOffset;
            
            iSrcY = iBufYOff + nYOff;
            
            if( iSrcY < nLBlockY * nBlockYSize
                || iSrcY >= (nLBlockY+1) * nBlockYSize )
            {
                nLBlockY = iSrcY / nBlockYSize;
                int bJustInitialize = 
                    eRWFlag == GF_Write
                    && nXOff == 0 && nXSize == nBlockXSize
                    && nYOff <= nLBlockY * nBlockYSize
                    && nYOff + nYSize >= (nLBlockY+1) * nBlockYSize;

                if( poBlock )
                    poBlock->DropLock();

                poBlock = GetLockedBlockRef( 0, nLBlockY, bJustInitialize );
                if( poBlock == NULL )
                {
                    CPLError( CE_Failure, CPLE_AppDefined,
			"GetBlockRef failed at X block offset %d, "
                        "Y block offset %d", 0, nLBlockY );
		    return( CE_Failure );
                }

                if( eRWFlag == GF_Write )
                    poBlock->MarkDirty();
                
                pabySrcBlock = (GByte *) poBlock->GetDataRef();
                if( pabySrcBlock == NULL )
                {
                    poBlock->DropLock();
                    return CE_Failure;
                }
            }

            nSrcByteOffset = ((iSrcY-nLBlockY*nBlockYSize)*nBlockXSize + nXOff)
                * nBandDataSize;
            
            if( eDataType == eBufType )
            {
                if( eRWFlag == GF_Read )
                    memcpy( ((GByte *) pData) + (size_t)iBufYOff * nLineSpace,
                            pabySrcBlock + nSrcByteOffset, 
                            nLineSpace );
                else
                    memcpy( pabySrcBlock + nSrcByteOffset, 
                            ((GByte *) pData) + (size_t)iBufYOff * nLineSpace,
                            nLineSpace );
            }
            else
            {
                /* type to type conversion */
                
                if( eRWFlag == GF_Read )
                    GDALCopyWords( pabySrcBlock + nSrcByteOffset,
                                   eDataType, nBandDataSize,
                                   ((GByte *) pData) + (size_t)iBufYOff * nLineSpace,
                                   eBufType, nPixelSpace, nBufXSize );
                else
                    GDALCopyWords( ((GByte *) pData) + (size_t)iBufYOff * nLineSpace,
                                   eBufType, nPixelSpace,
                                   pabySrcBlock + nSrcByteOffset,
                                   eDataType, nBandDataSize, nBufXSize );
            }
        }

        if( poBlock )
            poBlock->DropLock();

        return CE_None;
    }
    
/* ==================================================================== */
/*      Do we have overviews that would be appropriate to satisfy       */
/*      this request?                                                   */
/* ==================================================================== */
    if( (nBufXSize < nXSize || nBufYSize < nYSize)
        && GetOverviewCount() > 0 && eRWFlag == GF_Read )
    {
        if( OverviewRasterIO( eRWFlag, nXOff, nYOff, nXSize, nYSize, 
                              pData, nBufXSize, nBufYSize, 
                              eBufType, nPixelSpace, nLineSpace ) == CE_None )
            return CE_None;
    }

/* ==================================================================== */
/*      The second case when we don't need subsample data but likely    */
/*      need data type conversion.                                      */
/* ==================================================================== */
    int         iSrcX;

    if ( /* nPixelSpace == nBufDataSize
            && */ nXSize == nBufXSize
         && nYSize == nBufYSize )    
    {
//        printf( "IRasterIO(%d,%d,%d,%d) rw=%d case 2\n", 
//                nXOff, nYOff, nXSize, nYSize, 
//                (int) eRWFlag );

/* -------------------------------------------------------------------- */
/*      Loop over buffer computing source locations.                    */
/* -------------------------------------------------------------------- */
        int     nLBlockXStart, nXSpanEnd;

        // Calculate starting values out of loop
        nLBlockXStart = nXOff / nBlockXSize;
        nXSpanEnd = nBufXSize + nXOff;

        int nYInc = 0;
        for( iBufYOff = 0, iSrcY = nYOff; iBufYOff < nBufYSize; iBufYOff+=nYInc, iSrcY+=nYInc )
        {
            size_t  iBufOffset, iSrcOffset;
            int     nXSpan;

            iBufOffset = (size_t)iBufYOff * nLineSpace;
            nLBlockY = iSrcY / nBlockYSize;
            nLBlockX = nLBlockXStart;
            iSrcX = nXOff;
            while( iSrcX < nXSpanEnd )
            {
                int nXSpanSize;

                nXSpan = (nLBlockX + 1) * nBlockXSize;
                nXSpan = ( ( nXSpan < nXSpanEnd )?nXSpan:nXSpanEnd ) - iSrcX;
                nXSpanSize = nXSpan * nPixelSpace;

                int bJustInitialize = 
                    eRWFlag == GF_Write
                    && nYOff <= nLBlockY * nBlockYSize
                    && nYOff + nYSize >= (nLBlockY+1) * nBlockYSize
                    && nXOff <= nLBlockX * nBlockXSize
                    && nXOff + nXSize >= (nLBlockX+1) * nBlockXSize;

//                printf( "bJustInitialize = %d (%d,%d,%d,%d)\n", 
//                        bJustInitialize,
//                        nYOff, nYSize, 
//                        nLBlockY, nBlockYSize );
//                bJustInitialize = FALSE;
                

/* -------------------------------------------------------------------- */
/*      Ensure we have the appropriate block loaded.                    */
/* -------------------------------------------------------------------- */
                poBlock = GetLockedBlockRef( nLBlockX, nLBlockY, bJustInitialize );
                if( !poBlock )
                {
                    CPLError( CE_Failure, CPLE_AppDefined,
			"GetBlockRef failed at X block offset %d, "
                        "Y block offset %d", nLBlockX, nLBlockY );
                    return( CE_Failure );
                }

                if( eRWFlag == GF_Write )
                    poBlock->MarkDirty();
                
                pabySrcBlock = (GByte *) poBlock->GetDataRef();
                if( pabySrcBlock == NULL )
                {
                    poBlock->DropLock();
                    return CE_Failure;
                }

/* -------------------------------------------------------------------- */
/*      Copy over this chunk of data.                                   */
/* -------------------------------------------------------------------- */
                iSrcOffset = ((size_t)iSrcX - (size_t)nLBlockX*nBlockXSize
                    + ((size_t)(iSrcY) - (size_t)nLBlockY*nBlockYSize) * nBlockXSize)*nBandDataSize;
                /* Fill up as many rows as possible for the loaded block */
                int kmax = MIN(nBlockYSize - (iSrcY % nBlockYSize), nBufYSize - iBufYOff);
                for(int k=0; k<kmax;k++)
                {
                    if( eDataType == eBufType 
                        && nPixelSpace == nBufDataSize )
                    {
                        if( eRWFlag == GF_Read )
                            memcpy( ((GByte *) pData) + iBufOffset + k * nLineSpace,
                                    pabySrcBlock + iSrcOffset, nXSpanSize );
                        else
                            memcpy( pabySrcBlock + iSrcOffset, 
                                    ((GByte *) pData) + iBufOffset + k * nLineSpace, nXSpanSize );
                    }
                    else
                    {
                        /* type to type conversion */
                        
                        if( eRWFlag == GF_Read )
                            GDALCopyWords( pabySrcBlock + iSrcOffset,
                                        eDataType, nBandDataSize,
                                        ((GByte *) pData) + iBufOffset + k * nLineSpace,
                                        eBufType, nPixelSpace, nXSpan );
                        else
                            GDALCopyWords( ((GByte *) pData) + iBufOffset + k * nLineSpace,
                                        eBufType, nPixelSpace,
                                        pabySrcBlock + iSrcOffset,
                                        eDataType, nBandDataSize, nXSpan );
                    }
                    
                    iSrcOffset += nBlockXSize * nBandDataSize;
                }

                iBufOffset += nXSpanSize;
                nLBlockX++;
                iSrcX+=nXSpan;

                poBlock->DropLock();
                poBlock = NULL;
            }

            /* Compute the increment to go on a block boundary */
            nYInc = nBlockYSize - (iSrcY % nBlockYSize);
        }

        return CE_None;
    }

/* ==================================================================== */
/*      Loop reading required source blocks to satisfy output           */
/*      request.  This is the most general implementation.              */
/* ==================================================================== */

/* -------------------------------------------------------------------- */
/*      Compute stepping increment.                                     */
/* -------------------------------------------------------------------- */
    double dfSrcXInc, dfSrcYInc;
    dfSrcXInc = nXSize / (double) nBufXSize;
    dfSrcYInc = nYSize / (double) nBufYSize;


//    printf( "IRasterIO(%d,%d,%d,%d) rw=%d case 3\n", 
//            nXOff, nYOff, nXSize, nYSize, 
//            (int) eRWFlag );
    if (eRWFlag == GF_Write)
    {
/* -------------------------------------------------------------------- */
/*    Write case                                                        */
/*    Loop over raster window computing source locations in the buffer. */
/* -------------------------------------------------------------------- */
        int iDstX, iDstY;
        GByte* pabyDstBlock = NULL;

        for( iDstY = nYOff; iDstY < nYOff + nYSize; iDstY ++)
        {
            size_t   iBufOffset, iDstOffset;
            iBufYOff = (int)((iDstY - nYOff) / dfSrcYInc);

            for( iDstX = nXOff; iDstX < nXOff + nXSize; iDstX ++)
            {
                iBufXOff = (int)((iDstX - nXOff) / dfSrcXInc);
                iBufOffset = (size_t)iBufYOff * nLineSpace + iBufXOff * nPixelSpace;

    /* -------------------------------------------------------------------- */
    /*      Ensure we have the appropriate block loaded.                    */
    /* -------------------------------------------------------------------- */
                if( iDstX < nLBlockX * nBlockXSize
                    || iDstX >= (nLBlockX+1) * nBlockXSize
                    || iDstY < nLBlockY * nBlockYSize
                    || iDstY >= (nLBlockY+1) * nBlockYSize )
                {
                    nLBlockX = iDstX / nBlockXSize;
                    nLBlockY = iDstY / nBlockYSize;

                    int bJustInitialize = 
                           nYOff <= nLBlockY * nBlockYSize
                        && nYOff + nYSize >= (nLBlockY+1) * nBlockYSize
                        && nXOff <= nLBlockX * nBlockXSize
                        && nXOff + nXSize >= (nLBlockX+1) * nBlockXSize;

                    if( poBlock != NULL )
                        poBlock->DropLock();

                    poBlock = GetLockedBlockRef( nLBlockX, nLBlockY, 
                                                bJustInitialize );
                    if( poBlock == NULL )
                    {
                        return( CE_Failure );
                    }

                    poBlock->MarkDirty();

                    pabyDstBlock = (GByte *) poBlock->GetDataRef();
                    if( pabyDstBlock == NULL )
                    {
                        poBlock->DropLock();
                        return CE_Failure;
                    }
                }

    /* -------------------------------------------------------------------- */
    /*      Copy over this pixel of data.                                   */
    /* -------------------------------------------------------------------- */
                iDstOffset = ((size_t)iDstX - (size_t)nLBlockX*nBlockXSize
                    + ((size_t)iDstY - (size_t)nLBlockY*nBlockYSize) * nBlockXSize)*nBandDataSize;

                if( eDataType == eBufType )
                {
                    memcpy( pabyDstBlock + iDstOffset, 
                            ((GByte *) pData) + iBufOffset, nBandDataSize );
                }
                else
                {
                    /* type to type conversion ... ouch, this is expensive way
                    of handling single words */

                    GDALCopyWords( ((GByte *) pData) + iBufOffset, eBufType, 0,
                                pabyDstBlock + iDstOffset, eDataType, 0,
                                1 );
                }
            }
        }
    }
    else
    {
        double      dfSrcX, dfSrcY;
/* -------------------------------------------------------------------- */
/*      Read case                                                       */
/*      Loop over buffer computing source locations.                    */
/* -------------------------------------------------------------------- */
        for( iBufYOff = 0; iBufYOff < nBufYSize; iBufYOff++ )
        {
            size_t   iBufOffset, iSrcOffset;

            dfSrcY = (iBufYOff+0.5) * dfSrcYInc + nYOff;
            iSrcY = (int) dfSrcY;

            iBufOffset = (size_t)iBufYOff * nLineSpace;

            for( iBufXOff = 0; iBufXOff < nBufXSize; iBufXOff++ )
            {
                dfSrcX = (iBufXOff+0.5) * dfSrcXInc + nXOff;

                iSrcX = (int) dfSrcX;

    /* -------------------------------------------------------------------- */
    /*      Ensure we have the appropriate block loaded.                    */
    /* -------------------------------------------------------------------- */
                if( iSrcX < nLBlockX * nBlockXSize
                    || iSrcX >= (nLBlockX+1) * nBlockXSize
                    || iSrcY < nLBlockY * nBlockYSize
                    || iSrcY >= (nLBlockY+1) * nBlockYSize )
                {
                    nLBlockX = iSrcX / nBlockXSize;
                    nLBlockY = iSrcY / nBlockYSize;

                    if( poBlock != NULL )
                        poBlock->DropLock();

                    poBlock = GetLockedBlockRef( nLBlockX, nLBlockY, 
                                                 FALSE );
                    if( poBlock == NULL )
                    {
                        return( CE_Failure );
                    }

                    pabySrcBlock = (GByte *) poBlock->GetDataRef();
                    if( pabySrcBlock == NULL )
                    {
                        poBlock->DropLock();
                        return CE_Failure;
                    }
                }

    /* -------------------------------------------------------------------- */
    /*      Copy over this pixel of data.                                   */
    /* -------------------------------------------------------------------- */
                iSrcOffset = ((size_t)iSrcX - (size_t)nLBlockX*nBlockXSize
                    + ((size_t)iSrcY - (size_t)nLBlockY*nBlockYSize) * nBlockXSize)*nBandDataSize;

                if( eDataType == eBufType )
                {
                    memcpy( ((GByte *) pData) + iBufOffset,
                            pabySrcBlock + iSrcOffset, nBandDataSize );
                }
                else
                {
                    /* type to type conversion ... ouch, this is expensive way
                    of handling single words */
                    GDALCopyWords( pabySrcBlock + iSrcOffset, eDataType, 0,
                                ((GByte *) pData) + iBufOffset, eBufType, 0,
                                1 );
                }

                iBufOffset += nPixelSpace;
            }
        }
    }

    if( poBlock != NULL )
        poBlock->DropLock();

    return( CE_None );
}

/************************************************************************/
/*                           GDALSwapWords()                            */
/************************************************************************/

/**
 * Byte swap words in-place.
 *
 * This function will byte swap a set of 2, 4 or 8 byte words "in place" in
 * a memory array.  No assumption is made that the words being swapped are
 * word aligned in memory.  Use the CPL_LSB and CPL_MSB macros from cpl_port.h
 * to determine if the current platform is big endian or little endian.  Use
 * The macros like CPL_SWAP32() to byte swap single values without the overhead
 * of a function call. 
 * 
 * @param pData pointer to start of data buffer.
 * @param nWordSize size of words being swapped in bytes. Normally 2, 4 or 8.
 * @param nWordCount the number of words to be swapped in this call. 
 * @param nWordSkip the byte offset from the start of one word to the start of
 * the next. For packed buffers this is the same as nWordSize. 
 */

void CPL_STDCALL GDALSwapWords( void *pData, int nWordSize, int nWordCount,
                                int nWordSkip )

{
    VALIDATE_POINTER0( pData, "GDALSwapWords" );

    int         i;
    GByte       *pabyData = (GByte *) pData;

    switch( nWordSize )
    {
      case 1:
        break;

      case 2:
        CPLAssert( nWordSkip >= 2 || nWordCount == 1 );
        for( i = 0; i < nWordCount; i++ )
        {
            GByte       byTemp;

            byTemp = pabyData[0];
            pabyData[0] = pabyData[1];
            pabyData[1] = byTemp;

            pabyData += nWordSkip;
        }
        break;
        
      case 4:
        CPLAssert( nWordSkip >= 4 || nWordCount == 1 );
        for( i = 0; i < nWordCount; i++ )
        {
            GByte       byTemp;

            byTemp = pabyData[0];
            pabyData[0] = pabyData[3];
            pabyData[3] = byTemp;

            byTemp = pabyData[1];
            pabyData[1] = pabyData[2];
            pabyData[2] = byTemp;

            pabyData += nWordSkip;
        }
        break;

      case 8:
        CPLAssert( nWordSkip >= 8 || nWordCount == 1 );
        for( i = 0; i < nWordCount; i++ )
        {
            GByte       byTemp;

            byTemp = pabyData[0];
            pabyData[0] = pabyData[7];
            pabyData[7] = byTemp;

            byTemp = pabyData[1];
            pabyData[1] = pabyData[6];
            pabyData[6] = byTemp;

            byTemp = pabyData[2];
            pabyData[2] = pabyData[5];
            pabyData[5] = byTemp;

            byTemp = pabyData[3];
            pabyData[3] = pabyData[4];
            pabyData[4] = byTemp;

            pabyData += nWordSkip;
        }
        break;

      default:
        CPLAssert( FALSE );
    }
}

/************************************************************************/
/*                           GDALCopyWords()                            */
/************************************************************************/

/**
 * Copy pixel words from buffer to buffer.
 *
 * This function is used to copy pixel word values from one memory buffer
 * to another, with support for conversion between data types, and differing
 * step factors.  The data type conversion is done using the normal GDAL 
 * rules.  Values assigned to a lower range integer type are clipped.  For
 * instance assigning GDT_Int16 values to a GDT_Byte buffer will cause values
 * less the 0 to be set to 0, and values larger than 255 to be set to 255. 
 * Assignment from floating point to integer uses default C type casting
 * semantics.   Assignment from non-complex to complex will result in the 
 * imaginary part being set to zero on output.  Assigment from complex to 
 * non-complex will result in the complex portion being lost and the real
 * component being preserved (<i>not magnitidue!</i>). 
 *
 * No assumptions are made about the source or destination words occuring
 * on word boundaries.  It is assumed that all values are in native machine
 * byte order. 
 *
 * @param pSrcData 
 *
 * 
 */ 

void CPL_STDCALL 
GDALCopyWords( void * pSrcData, GDALDataType eSrcType, int nSrcPixelOffset,
               void * pDstData, GDALDataType eDstType, int nDstPixelOffset,
               int nWordCount )

{
/* -------------------------------------------------------------------- */
/*      Special case when no data type translation is required.         */
/* -------------------------------------------------------------------- */
    if( eSrcType == eDstType )
    {
        int     nWordSize = GDALGetDataTypeSize(eSrcType)/8;
        int     i;

        // contiguous blocks.
        if( nWordSize == nSrcPixelOffset && nWordSize == nDstPixelOffset )
        {
            memcpy( pDstData, pSrcData, nSrcPixelOffset * nWordCount );
            return;
        }

        // Moving single bytes, avoid any possible memcpy() overhead.
        if( nWordSize == 1 )
        {
            GByte *pabySrc = (GByte *) pSrcData;
            GByte *pabyDst = (GByte *) pDstData;

            if (nWordCount > 100)
            {
/* ==================================================================== */
/*     Optimization for high number of words to transfer and some       */
/*     typical source and destination pixel spacing : we unroll the     */
/*     loop.                                                            */
/* ==================================================================== */
#define ASSIGN(_nSrcPixelOffset, _nDstPixelOffset, _k) \
                 pabyDst[_nDstPixelOffset * _k] = pabySrc[_nSrcPixelOffset * _k]
#define ASSIGN_LOOP(_nSrcPixelOffset, _nDstPixelOffset) \
                for( i = nWordCount / 16 ; i != 0; i-- ) \
                { \
                    ASSIGN(_nSrcPixelOffset, _nDstPixelOffset, 0); \
                    ASSIGN(_nSrcPixelOffset, _nDstPixelOffset, 1); \
                    ASSIGN(_nSrcPixelOffset, _nDstPixelOffset, 2); \
                    ASSIGN(_nSrcPixelOffset, _nDstPixelOffset, 3); \
                    ASSIGN(_nSrcPixelOffset, _nDstPixelOffset, 4); \
                    ASSIGN(_nSrcPixelOffset, _nDstPixelOffset, 5); \
                    ASSIGN(_nSrcPixelOffset, _nDstPixelOffset, 6); \
                    ASSIGN(_nSrcPixelOffset, _nDstPixelOffset, 7); \
                    ASSIGN(_nSrcPixelOffset, _nDstPixelOffset, 8); \
                    ASSIGN(_nSrcPixelOffset, _nDstPixelOffset, 9); \
                    ASSIGN(_nSrcPixelOffset, _nDstPixelOffset, 10); \
                    ASSIGN(_nSrcPixelOffset, _nDstPixelOffset, 11); \
                    ASSIGN(_nSrcPixelOffset, _nDstPixelOffset, 12); \
                    ASSIGN(_nSrcPixelOffset, _nDstPixelOffset, 13); \
                    ASSIGN(_nSrcPixelOffset, _nDstPixelOffset, 14); \
                    ASSIGN(_nSrcPixelOffset, _nDstPixelOffset, 15); \
                    pabyDst += _nDstPixelOffset * 16; \
                    pabySrc += _nSrcPixelOffset * 16; \
                } \
                nWordCount = nWordCount % 16;

                if (nSrcPixelOffset == 3 && nDstPixelOffset == 1)
                {
                    ASSIGN_LOOP(3, 1)
                }
                else if (nSrcPixelOffset == 1 && nDstPixelOffset == 3)
                {
                    ASSIGN_LOOP(1, 3)
                }
                else if (nSrcPixelOffset == 4 && nDstPixelOffset == 1)
                {
                    ASSIGN_LOOP(4, 1)
                }
                else if (nSrcPixelOffset == 1 && nDstPixelOffset == 4)
                {
                    ASSIGN_LOOP(1, 4)
                }
                else if (nSrcPixelOffset == 3 && nDstPixelOffset == 4)
                {
                    ASSIGN_LOOP(3, 4)
                }
                else if (nSrcPixelOffset == 4 && nDstPixelOffset == 3)
                {
                    ASSIGN_LOOP(4, 3)
                }
            }

            for( i = nWordCount; i != 0; i-- )
            {
                *pabyDst = *pabySrc;
                pabyDst += nDstPixelOffset;
                pabySrc += nSrcPixelOffset;
            }
            return;
        }

        // source or destination is not contiguous
        for( i = 0; i < nWordCount; i++ )
        {
            memcpy( ((GByte *)pDstData) + i * nDstPixelOffset,
                    ((GByte *)pSrcData) + i * nSrcPixelOffset,
                    nWordSize );
        }

        return;
    }

/* ----------------------------------------------------------------------- */
/* Special case when the source data is always the same value              */
/* (for VRTSourcedRasterBand::IRasterIO and VRTDerivedRasterBand::IRasterIO*/
/*  for example)                                                           */
/* ----------------------------------------------------------------------- */
    if (nSrcPixelOffset == 0 && nWordCount > 1)
    {
        /* Let the general translation case do the necessary conversions */
        /* on the first destination element */
        GDALCopyWords(pSrcData, eSrcType, nSrcPixelOffset,
                      pDstData, eDstType, nDstPixelOffset,
                      1 );

        /* Now copy the first element to the nWordCount - 1 following destination */
        /* elements */
        nWordCount--;
        GByte *pabyDstWord = ((GByte *)pDstData) + nDstPixelOffset;

        switch (eDstType)
        {
            case GDT_Byte:
            {
                if (nDstPixelOffset == 1)
                {
                    memset(pabyDstWord, *(GByte*)pDstData, nWordCount - 1);
                }
                else
                {
                    GByte valSet = *(GByte*)pDstData;
                    while(nWordCount--)
                    {
                        *pabyDstWord = valSet;
                        pabyDstWord += nDstPixelOffset;
                    }
                }
                break;
            }

#define CASE_DUPLICATE_SIMPLE(enum_type, c_type) \
            case enum_type:\
            { \
                c_type valSet = *(c_type*)pDstData; \
                while(nWordCount--) \
                { \
                    *(c_type*)pabyDstWord = valSet; \
                    pabyDstWord += nDstPixelOffset; \
                } \
                break; \
            }

            CASE_DUPLICATE_SIMPLE(GDT_UInt16, GUInt16)
            CASE_DUPLICATE_SIMPLE(GDT_Int16,  GInt16)
            CASE_DUPLICATE_SIMPLE(GDT_UInt32, GUInt32)
            CASE_DUPLICATE_SIMPLE(GDT_Int32,  GInt32)
            CASE_DUPLICATE_SIMPLE(GDT_Float32,float)
            CASE_DUPLICATE_SIMPLE(GDT_Float64,double)

#define CASE_DUPLICATE_COMPLEX(enum_type, c_type) \
            case enum_type:\
            { \
                c_type valSet1 = ((c_type*)pDstData)[0]; \
                c_type valSet2 = ((c_type*)pDstData)[1]; \
                while(nWordCount--) \
                { \
                    ((c_type*)pabyDstWord)[0] = valSet1; \
                    ((c_type*)pabyDstWord)[1] = valSet2; \
                    pabyDstWord += nDstPixelOffset; \
                } \
                break; \
            }

            CASE_DUPLICATE_COMPLEX(GDT_CInt16, GInt16)
            CASE_DUPLICATE_COMPLEX(GDT_CInt32, GInt32)
            CASE_DUPLICATE_COMPLEX(GDT_CFloat32, float)
            CASE_DUPLICATE_COMPLEX(GDT_CFloat64, double)

            default:
                CPLAssert( FALSE );
        }

        return;
    }
    
/* ==================================================================== */
/*      General translation case                                        */
/* ==================================================================== */
    for( int iWord = 0; iWord < nWordCount; iWord++ )
    {
        void   *pSrcWord, *pDstWord;
        double  dfPixelValue=0.0, dfPixelValueI=0.0;

        pSrcWord = static_cast<GByte *>(pSrcData) + iWord * nSrcPixelOffset;
        pDstWord = static_cast<GByte *>(pDstData) + iWord * nDstPixelOffset;

/* -------------------------------------------------------------------- */
/*      Fetch source value based on data type.                          */
/* -------------------------------------------------------------------- */
        switch( eSrcType )
        {
          case GDT_Byte:
          {
              GByte byVal = *static_cast<GByte *>(pSrcWord);
              switch( eDstType )
              {
                case GDT_UInt16:
                  *static_cast<GUInt16 *>(pDstWord) = byVal;
                  continue;
                case GDT_Int16:
                  *static_cast<GInt16 *>(pDstWord) = byVal;
                  continue;
                case GDT_UInt32:
                  *static_cast<GUInt32 *>(pDstWord) = byVal;
                  continue;
                case GDT_Int32:
                  *static_cast<GInt32 *>(pDstWord) = byVal;
                  continue;
                case GDT_CInt16:
                {
                    GInt16 *panDstWord = static_cast<GInt16 *>(pDstWord);
                    panDstWord[0] = byVal;
                    panDstWord[1] = 0;
                    continue;
                }
                case GDT_CInt32:
                {
                    GInt32 *panDstWord = static_cast<GInt32 *>(pDstWord);
                    panDstWord[0] = byVal;
                    panDstWord[1] = 0;
                    continue;
                }
                default:
                  break;
              }
              dfPixelValue = byVal;
          }
          break;

          case GDT_UInt16:
          {
              GUInt16 nVal = *static_cast<GUInt16 *>(pSrcWord);
              switch( eDstType )
              {
                case GDT_Byte:
                {
                    GByte byVal;
                    if( nVal > 255 )
                        byVal = 255;
                    else
                        byVal = static_cast<GByte>(nVal);
                    *static_cast<GByte *>(pDstWord) = byVal;
                    continue;
                }
                case GDT_Int16:
                  if( nVal > 32767 )
                      nVal = 32767;
                  *static_cast<GInt16 *>(pDstWord) = nVal;
                  continue;
                case GDT_UInt32:
                  *static_cast<GUInt32 *>(pDstWord) = nVal;
                  continue;
                case GDT_Int32:
                  *static_cast<GInt32 *>(pDstWord) = nVal;
                  continue;
                case GDT_CInt16:
                {
                    GInt16 *panDstWord = static_cast<GInt16 *>(pDstWord);
                    if( nVal > 32767 )
                        nVal = 32767;
                    panDstWord[0] = nVal;
                    panDstWord[1] = 0;
                    continue;
                }
                case GDT_CInt32:
                {
                    GInt32 *panDstWord = static_cast<GInt32 *>(pDstWord);
                    panDstWord[0] = nVal;
                    panDstWord[1] = 0;
                    continue;
                }
                default:
                  break;
              }
              dfPixelValue = nVal;
          }
          break;
          
          case GDT_Int16:
          {
              GInt16 nVal = *static_cast<GInt16 *>(pSrcWord);
              switch( eDstType )
              {
                case GDT_Byte:
                {
                    GByte byVal;
                    if( nVal > 255 )
                        byVal = 255;
                    else if (nVal < 0)
                        byVal = 0;
                    else
                        byVal = static_cast<GByte>(nVal);
                    *static_cast<GByte *>(pDstWord) = byVal;
                    continue;
                }
                case GDT_UInt16:
                  if( nVal < 0 )
                      nVal = 0;
                  *static_cast<GUInt16 *>(pDstWord) = nVal;
                  continue;
                case GDT_UInt32:
                  if( nVal < 0 )
                      nVal = 0;
                  *static_cast<GUInt32 *>(pDstWord) = nVal;
                  continue;
                case GDT_Int32:
                  *static_cast<GInt32 *>(pDstWord) = nVal;
                  continue;
                case GDT_CInt16:
                {
                    GInt16 *panDstWord = static_cast<GInt16 *>(pDstWord);
                    panDstWord[0] = nVal;
                    panDstWord[1] = 0;
                    continue;
                }
                case GDT_CInt32:
                {
                    GInt32 *panDstWord = static_cast<GInt32 *>(pDstWord);
                    panDstWord[0] = nVal;
                    panDstWord[1] = 0;
                    continue;
                }
                default:
                  break;
              }
              dfPixelValue = nVal;
          }
          break;
          
          case GDT_Int32:
          {
              GInt32 nVal = *static_cast<GInt32 *>(pSrcWord);
              switch( eDstType )
              {
                case GDT_Byte:
                {
                    GByte byVal;
                    if( nVal > 255 )
                        byVal = 255;
                    else if (nVal < 0)
                        byVal = 0;
                    else
                        byVal = nVal;
                    *static_cast<GByte *>(pDstWord) = byVal;
                    continue;
                }
                case GDT_UInt16:
                  if( nVal > 65535 )
                      nVal = 65535;
                  else if( nVal < 0 )
                      nVal = 0;
                  *static_cast<GUInt16 *>(pDstWord) = nVal;
                  continue;
                case GDT_Int16:
                  if( nVal > 32767 )
                      nVal = 32767;
                  else if( nVal < -32768)
                      nVal = -32768;
                  *static_cast<GInt16 *>(pDstWord) = nVal;
                  continue;
                case GDT_UInt32:
                  if( nVal < 0 )
                      nVal = 0;
                  *static_cast<GUInt32 *>(pDstWord) = nVal;
                  continue;
                case GDT_CInt16:
                {
                    GInt16 *panDstWord = static_cast<GInt16 *>(pDstWord);
                    if( nVal > 32767 )
                        nVal = 32767;
                    else if( nVal < -32768)
                        nVal = -32768;
                    panDstWord[0] = nVal;
                    panDstWord[1] = 0;
                    continue;
                }
                case GDT_CInt32:
                {
                    GInt32 *panDstWord = static_cast<GInt32 *>(pDstWord);
                    panDstWord[0] = nVal;
                    panDstWord[1] = 0;
                    continue;
                }
                default:
                  break;
              }
              dfPixelValue = nVal;
          }
          break;
          
          case GDT_UInt32:
          {
              GUInt32 nVal = *static_cast<GUInt32 *>(pSrcWord);
              switch( eDstType )
              {
                case GDT_Byte:
                {
                    GByte byVal;
                    if( nVal > 255 )
                        byVal = 255;
                    else if (nVal < 0)
                        byVal = 0;
                    else
                        byVal = nVal;
                    *static_cast<GByte *>(pDstWord) = byVal;
                    continue;
                }
                case GDT_UInt16:
                  if( nVal > 65535 )
                      nVal = 65535;
                  *static_cast<GUInt16 *>(pDstWord) = nVal;
                  continue;
                case GDT_Int16:
                  if( nVal > 32767 )
                      nVal = 32767;
                  *static_cast<GInt16 *>(pDstWord) = nVal;
                  continue;
                case GDT_Int32:
                  if( nVal > 2147483647UL )
                      nVal = 2147483647UL;
                  *static_cast<GInt32 *>(pDstWord) = nVal;
                  continue;
                case GDT_CInt16:
                {
                    GInt16 *panDstWord = static_cast<GInt16 *>(pDstWord);
                    if( nVal > 32767 )
                        nVal = 32767;
                    panDstWord[0] = nVal;
                    panDstWord[1] = 0;
                    continue;
                }
                case GDT_CInt32:
                {
                    GInt32 *panDstWord = static_cast<GInt32 *>(pDstWord);
                    if( nVal > 2147483647UL )
                        nVal = 2147483647UL;
                    panDstWord[0] = nVal;
                    panDstWord[1] = 0;
                    continue;
                }
                default:
                  break;
              }
              dfPixelValue = nVal;
          }
          break;
          
          case GDT_CInt16:
          {
              GInt16 *panSrcWord = static_cast<GInt16 *>(pSrcWord);
              GInt16 nVal = panSrcWord[0];
              switch( eDstType )
              {
                case GDT_Byte:
                {
                    GByte byVal;
                    if( nVal > 255 )
                        byVal = 255;
                    else if (nVal < 0)
                        byVal = 0;
                    else
                        byVal = static_cast<GByte>(nVal);
                    *static_cast<GByte *>(pDstWord) = byVal;
                    continue;
                }
                case GDT_Int16:
                  *static_cast<GInt16 *>(pDstWord) = nVal;
                  continue;
                case GDT_UInt16:
                  if( nVal < 0 )
                      nVal = 0;
                  *static_cast<GUInt16 *>(pDstWord) = nVal;
                  continue;
                case GDT_UInt32:
                  if( nVal < 0 )
                      nVal = 0;
                  *static_cast<GUInt32 *>(pDstWord) = nVal;
                  continue;
                case GDT_Int32:
                  *static_cast<GInt32 *>(pDstWord) = nVal;
                  continue;
                case GDT_CInt32:
                {
                    GInt32 *panDstWord = static_cast<GInt32 *>(pDstWord);
                    panDstWord[0] = panSrcWord[0];
                    panDstWord[1] = panSrcWord[1];
                    continue;
                }
                default:
                  break;
              }
              dfPixelValue = panSrcWord[0];
              dfPixelValueI = panSrcWord[1];
          }
          break;
          
          case GDT_CInt32:
          {
              GInt32 *panSrcWord = static_cast<GInt32 *>(pSrcWord);
              GInt32 nVal = panSrcWord[0];
              switch( eDstType )
              {
                case GDT_Byte:
                {
                    GByte byVal;
                    if( nVal > 255 )
                        byVal = 255;
                    else if (nVal < 0)
                        byVal = 0;
                    else
                        byVal = nVal;
                    *static_cast<GByte *>(pDstWord) = byVal;
                    continue;
                }
                case GDT_Int16:
                  if( nVal > 32767 )
                      nVal = 32767;
                  else if( nVal < -32768)
                      nVal = -32768;
                  *static_cast<GInt16 *>(pDstWord) = nVal;
                  continue;
                case GDT_UInt16:
                  if( nVal > 65535 )
                      nVal = 65535;
                  else if( nVal < 0 )
                      nVal = 0;
                  *static_cast<GUInt16 *>(pDstWord) = nVal;
                  continue;
                case GDT_UInt32:
                  if( nVal < 0 )
                      nVal = 0;
                  *static_cast<GUInt32 *>(pDstWord) = nVal;
                  continue;
                case GDT_Int32:
                  *static_cast<GInt32 *>(pDstWord) = nVal;
                  continue;
                case GDT_CInt16:
                {
                    GInt16 *panDstWord = static_cast<GInt16 *>(pDstWord);
                    if( nVal > 32767 )
                        nVal = 32767;
                    else if( nVal < -32768)
                        nVal = -32768;
                    panDstWord[0] = nVal;
                    nVal = panSrcWord[1];
                    if( nVal > 32767 )
                        nVal = 32767;
                    else if( nVal < -32768)
                        nVal = -32768;
                    panDstWord[1] = nVal;
                    continue;
                }
                default:
                  break;
              }
              dfPixelValue = panSrcWord[0];
              dfPixelValueI = panSrcWord[1];
          }
          break;

          case GDT_Float32:
          {
              float fVal = *static_cast<float *>(pSrcWord);
              dfPixelValue = fVal;
          }
          break;
          
          case GDT_Float64:
          {
              dfPixelValue = *static_cast<double *>(pSrcWord);
          }
          break;

          case GDT_CFloat32:
          {
              float *pafSrcWord = static_cast<float *>(pSrcWord);
              dfPixelValue = pafSrcWord[0];
              dfPixelValueI = pafSrcWord[1];
          }
          break;
          
          case GDT_CFloat64:
          {
              double *padfSrcWord = static_cast<double *>(pSrcWord);
              dfPixelValue = padfSrcWord[0];
              dfPixelValueI = padfSrcWord[1];
          }
          break;

          default:
            CPLAssert( FALSE );
        }
        
/* -------------------------------------------------------------------- */
/*      Set the destination pixel, doing range clipping as needed.      */
/* -------------------------------------------------------------------- */
        switch( eDstType )
        {
          case GDT_Byte:
          {
              GByte *pabyDstWord = static_cast<GByte *>(pDstWord);

              dfPixelValue += (float) 0.5;

              if( dfPixelValue < 0.0 )
                  *pabyDstWord = 0;
              else if( dfPixelValue > 255.0 )
                  *pabyDstWord = 255;
              else
                  *pabyDstWord = (GByte) dfPixelValue;
          }
          break;

          case GDT_UInt16:
          {
              GUInt16   nVal;
              
              dfPixelValue += 0.5;

              if( dfPixelValue < 0.0 )
                  nVal = 0;
              else if( dfPixelValue > 65535.0 )
                  nVal = 65535;
              else
                  nVal = (GUInt16) dfPixelValue;

              *static_cast<GUInt16 *>(pDstWord) = nVal;
          }
          break;

          case GDT_Int16:
          {
              GInt16    nVal;
              
              dfPixelValue += 0.5;

              if( dfPixelValue < -32768 )
                  nVal = -32768;
              else if( dfPixelValue > 32767 )
                  nVal = 32767;
              else
                  nVal = (GInt16) floor(dfPixelValue);

              *static_cast<GInt16 *>(pDstWord) = nVal;
          }
          break;
          
          case GDT_UInt32:
          {
              GUInt32   nVal;
              
              dfPixelValue += 0.5;

              if( dfPixelValue < 0 )
                  nVal = 0;
              else if( dfPixelValue > 4294967295U )
                  nVal = 4294967295U;
              else
                  nVal = (GInt32) dfPixelValue;

              *static_cast<GUInt32 *>(pDstWord) = nVal;
          }
          break;
          
          case GDT_Int32:
          {
              GInt32    nVal;
              
              dfPixelValue += 0.5;

              if( dfPixelValue < -2147483647.0 )
                  nVal = -2147483647;
              else if( dfPixelValue > 2147483647 )
                  nVal = 2147483647;
              else
                  nVal = (GInt32) floor(dfPixelValue);

              *static_cast<GInt32 *>(pDstWord) = nVal;
          }
          break;

          case GDT_Float32:
          {
              *static_cast<float *>(pDstWord) = static_cast<float>(dfPixelValue);
          }
          break;

          case GDT_Float64:
            *static_cast<double *>(pDstWord) = dfPixelValue;
            break;
              
          case GDT_CInt16:
          {
              GInt16    nVal;
              GInt16 *panDstWord = static_cast<GInt16 *>(pDstWord);
              
              dfPixelValue += 0.5;
              dfPixelValueI += 0.5;

              if( dfPixelValue < -32768 )
                  nVal = -32768;
              else if( dfPixelValue > 32767 )
                  nVal = 32767;
              else
                  nVal = (GInt16) floor(dfPixelValue);
              panDstWord[0] = nVal;

              if( dfPixelValueI < -32768 )
                  nVal = -32768;
              else if( dfPixelValueI > 32767 )
                  nVal = 32767;
              else
                  nVal = (GInt16) floor(dfPixelValueI);
              panDstWord[1] = nVal;
          }
          break;
          
          case GDT_CInt32:
          {
              GInt32    nVal;
              GInt32 *panDstWord = static_cast<GInt32 *>(pDstWord);
              
              dfPixelValue += 0.5;
              dfPixelValueI += 0.5;

              if( dfPixelValue < -2147483647.0 )
                  nVal = -2147483647;
              else if( dfPixelValue > 2147483647 )
                  nVal = 2147483647;
              else
                  nVal = (GInt32) floor(dfPixelValue);

              panDstWord[0] = nVal;

              if( dfPixelValueI < -2147483647.0 )
                  nVal = -2147483647;
              else if( dfPixelValueI > 2147483647 )
                  nVal = 2147483647;
              else
                  nVal = (GInt32) floor(dfPixelValueI);

              panDstWord[1] = nVal;
          }
          break;

          case GDT_CFloat32:
          {
              float *pafDstWord = static_cast<float *>(pDstWord);
              pafDstWord[0] = static_cast<float>(dfPixelValue);
              pafDstWord[1] = static_cast<float>(dfPixelValueI);
          }
          break;

          case GDT_CFloat64:
          {
              double *padfDstWord = static_cast<double *>(pDstWord);
              padfDstWord[0] = dfPixelValue;
              padfDstWord[1] = dfPixelValueI;
          }
          break;
              
          default:
            CPLAssert( FALSE );
        }
    } /* next iWord */
}

/************************************************************************/
/*                            GDALCopyBits()                            */
/************************************************************************/

/**
 * Bitwise word copying.
 *
 * A function for moving sets of partial bytes around.  Loosely
 * speaking this is a bitswise analog to GDALCopyWords().
 *
 * It copies nStepCount "words" where each word is nBitCount bits long. 
 * The nSrcStep and nDstStep are the number of bits from the start of one
 * word to the next (same as nBitCount if they are packed).  The nSrcOffset
 * and nDstOffset are the offset into the source and destination buffers
 * to start at, also measured in bits. 
 *
 * All bit offsets are assumed to start from the high order bit in a byte
 * (ie. most significant bit first).  Currently this function is not very
 * optimized, but it may be improved for some common cases in the future 
 * as needed. 
 *
 * @param pabySrcData the source data buffer.
 * @param nSrcOffset the offset (in bits) in pabySrcData to the start of the 
 * first word to copy.
 * @param nSrcStep the offset in bits from the start one source word to the 
 * start of the next. 
 * @param pabyDstData the destination data buffer.
 * @param nDstOffset the offset (in bits) in pabyDstData to the start of the 
 * first word to copy over.
 * @param nDstStep the offset in bits from the start one word to the 
 * start of the next. 
 * @param nBitCount the number of bits in a word to be copied.
 * @param nStepCount the number of words to copy.
 */

void GDALCopyBits( const GByte *pabySrcData, int nSrcOffset, int nSrcStep, 
                   GByte *pabyDstData, int nDstOffset, int nDstStep,
                   int nBitCount, int nStepCount )

{
    VALIDATE_POINTER0( pabySrcData, "GDALCopyBits" );

    int iStep;
    int iBit;

    for( iStep = 0; iStep < nStepCount; iStep++ )
    {
        for( iBit = 0; iBit < nBitCount; iBit++ )
        {
            if( pabySrcData[nSrcOffset>>3] 
                & (0x80 >>(nSrcOffset & 7)) )
                pabyDstData[nDstOffset>>3] |= (0x80 >> (nDstOffset & 7));
            else
                pabyDstData[nDstOffset>>3] &= ~(0x80 >> (nDstOffset & 7));


            nSrcOffset++;
            nDstOffset++;
        } 

        nSrcOffset += (nSrcStep - nBitCount);
        nDstOffset += (nDstStep - nBitCount);
    }
}

/************************************************************************/
/*                          OverviewRasterIO()                          */
/*                                                                      */
/*      Special work function to utilize available overviews to         */
/*      more efficiently satisfy downsampled requests.  It will         */
/*      return CE_Failure if there are no appropriate overviews         */
/*      available but it doesn't emit any error messages.               */
/************************************************************************/

CPLErr GDALRasterBand::OverviewRasterIO( GDALRWFlag eRWFlag,
                                int nXOff, int nYOff, int nXSize, int nYSize,
                                void * pData, int nBufXSize, int nBufYSize,
                                GDALDataType eBufType,
                                int nPixelSpace, int nLineSpace )


{
    GDALRasterBand      *poBestOverview = NULL;
    int                 nOverviewCount = GetOverviewCount();
    double              dfDesiredResolution, dfBestResolution = 1.0;

/* -------------------------------------------------------------------- */
/*      Find the Compute the desired resolution.  The resolution is     */
/*      based on the least reduced axis, and represents the number      */
/*      of source pixels to one destination pixel.                      */
/* -------------------------------------------------------------------- */
    if( (nXSize / (double) nBufXSize) < (nYSize / (double) nBufYSize ) 
        || nBufYSize == 1 )
        dfDesiredResolution = nXSize / (double) nBufXSize;
    else
        dfDesiredResolution = nYSize / (double) nBufYSize;

/* -------------------------------------------------------------------- */
/*      Find the overview level that largest resolution value (most     */
/*      downsampled) that is still less than (or only a little more)    */
/*      downsampled than the request.                                   */
/* -------------------------------------------------------------------- */
    for( int iOverview = 0; iOverview < nOverviewCount; iOverview++ )
    {
        GDALRasterBand  *poOverview = GetOverview( iOverview );
        double          dfResolution;

        // What resolution is this?
        if( (GetXSize() / (double) poOverview->GetXSize())
            < (GetYSize() / (double) poOverview->GetYSize()) )
            dfResolution = 
                GetXSize() / (double) poOverview->GetXSize();
        else
            dfResolution = 
                GetYSize() / (double) poOverview->GetYSize();

        // Is it nearly the requested resolution and better (lower) than
        // the current best resolution?
        if( dfResolution >= dfDesiredResolution * 1.2 
            || dfResolution <= dfBestResolution )
            continue;

        // Ignore AVERAGE_BIT2GRAYSCALE overviews for RasterIO purposes.
        const char *pszResampling = 
            poOverview->GetMetadataItem( "RESAMPLING" );

        if( pszResampling != NULL && EQUALN(pszResampling,"AVERAGE_BIT2",12))
            continue;

        // OK, this is our new best overview.
        poBestOverview = poOverview;
        dfBestResolution = dfResolution;
    }

/* -------------------------------------------------------------------- */
/*      If we didn't find an overview that helps us, just return        */
/*      indicating failure and the full resolution image will be used.  */
/* -------------------------------------------------------------------- */
    if( poBestOverview == NULL )
        return CE_Failure;

/* -------------------------------------------------------------------- */
/*      Recompute the source window in terms of the selected            */
/*      overview.                                                       */
/* -------------------------------------------------------------------- */
    int         nOXOff, nOYOff, nOXSize, nOYSize;
    double      dfXRes, dfYRes;
    
    dfXRes = GetXSize() / (double) poBestOverview->GetXSize();
    dfYRes = GetYSize() / (double) poBestOverview->GetYSize();

    nOXOff = MIN(poBestOverview->GetXSize()-1,(int) (nXOff/dfXRes+0.5));
    nOYOff = MIN(poBestOverview->GetYSize()-1,(int) (nYOff/dfYRes+0.5));
    nOXSize = MAX(1,(int) (nXSize/dfXRes + 0.5));
    nOYSize = MAX(1,(int) (nYSize/dfYRes + 0.5));
    if( nOXOff + nOXSize > poBestOverview->GetXSize() )
        nOXSize = poBestOverview->GetXSize() - nOXOff;
    if( nOYOff + nOYSize > poBestOverview->GetYSize() )
        nOYSize = poBestOverview->GetYSize() - nOYOff;

/* -------------------------------------------------------------------- */
/*      Recast the call in terms of the new raster layer.               */
/* -------------------------------------------------------------------- */
    return poBestOverview->RasterIO( eRWFlag, nOXOff, nOYOff, nOXSize, nOYSize,
                                     pData, nBufXSize, nBufYSize, eBufType,
                                     nPixelSpace, nLineSpace );
}

/************************************************************************/
/*                         BlockBasedRasterIO()                         */
/*                                                                      */
/*      This convenience function implements a dataset level            */
/*      RasterIO() interface based on calling down to fetch blocks,     */
/*      much like the GDALRasterBand::IRasterIO(), but it handles       */
/*      all bands at once, so that a format driver that handles a       */
/*      request for different bands of the same block efficiently       */
/*      (ie. without re-reading interleaved data) will efficiently.     */
/*                                                                      */
/*      This method is intended to be called by an overridden           */
/*      IRasterIO() method in the driver specific GDALDataset           */
/*      derived class.                                                  */
/*                                                                      */
/*      Default internal implementation of RasterIO() ... utilizes      */
/*      the Block access methods to satisfy the request.  This would    */
/*      normally only be overridden by formats with overviews.          */
/*                                                                      */
/*      To keep things relatively simple, this method does not          */
/*      currently take advantage of some special cases addressed in     */
/*      GDALRasterBand::IRasterIO(), so it is likely best to only       */
/*      call it when you know it will help.  That is in cases where     */
/*      data is at 1:1 to the buffer, you don't want to take            */
/*      advantage of overviews, and you know the driver is              */
/*      implementing interleaved IO efficiently on a block by block     */
/*      basis.                                                          */
/************************************************************************/

CPLErr 
GDALDataset::BlockBasedRasterIO( GDALRWFlag eRWFlag,
                                 int nXOff, int nYOff, int nXSize, int nYSize,
                                 void * pData, int nBufXSize, int nBufYSize,
                                 GDALDataType eBufType,
                                 int nBandCount, int *panBandMap,
                                 int nPixelSpace, int nLineSpace,int nBandSpace)
    
{
    GByte      **papabySrcBlock = NULL;
    GDALRasterBlock *poBlock;
    GDALRasterBlock **papoBlocks;
    int         nLBlockX=-1, nLBlockY=-1, iBufYOff, iBufXOff, iSrcY, iBand;
    int         nBlockXSize=1, nBlockYSize=1;
    CPLErr      eErr = CE_None;
    GDALDataType eDataType = GDT_Byte;

/* -------------------------------------------------------------------- */
/*      Ensure that all bands share a common block size and data type.  */
/* -------------------------------------------------------------------- */

    for( iBand = 0; iBand < nBandCount; iBand++ )
    {
        GDALRasterBand *poBand = GetRasterBand( panBandMap[iBand] );
        int nThisBlockXSize, nThisBlockYSize;

        if( iBand == 0 )
        {
            poBand->GetBlockSize( &nBlockXSize, &nBlockYSize );
            eDataType = poBand->GetRasterDataType();
        }
        else
        {
            poBand->GetBlockSize( &nThisBlockXSize, &nThisBlockYSize );
            if( nThisBlockXSize != nBlockXSize 
                || nThisBlockYSize != nBlockYSize )
            {
                CPLDebug( "GDAL", 
                          "GDALDataset::BlockBasedRasterIO() ... "
                          "mismatched block sizes, use std method." );
                return GDALDataset::IRasterIO( eRWFlag, 
                                               nXOff, nYOff, nXSize, nYSize, 
                                               pData, nBufXSize, nBufYSize, 
                                               eBufType, 
                                               nBandCount, panBandMap,
                                               nPixelSpace, nLineSpace, 
                                               nBandSpace );
            }

            if( eDataType != poBand->GetRasterDataType() 
                && (nXSize != nBufXSize || nYSize != nBufYSize) )
            {
                CPLDebug( "GDAL", 
                          "GDALDataset::BlockBasedRasterIO() ... "
                          "mismatched band data types, use std method." );
                return GDALDataset::IRasterIO( eRWFlag, 
                                               nXOff, nYOff, nXSize, nYSize, 
                                               pData, nBufXSize, nBufYSize, 
                                               eBufType, 
                                               nBandCount, panBandMap,
                                               nPixelSpace, nLineSpace, 
                                               nBandSpace );
            }
        }
    }

/* ==================================================================== */
/*      In this special case at full resolution we step through in      */
/*      blocks, turning the request over to the per-band                */
/*      IRasterIO(), but ensuring that all bands of one block are       */
/*      called before proceeding to the next.                           */
/* ==================================================================== */
    if( nXSize == nBufXSize && nYSize == nBufYSize )    
    {
        int nChunkYSize, nChunkXSize, nChunkXOff, nChunkYOff;

        for( iBufYOff = 0; iBufYOff < nBufYSize; iBufYOff += nChunkYSize )
        {
            nChunkYSize = nBlockYSize;
            nChunkYOff = iBufYOff + nYOff;
            nChunkYSize = nBlockYSize - (nChunkYOff % nBlockYSize);
            if( nChunkYSize == 0 )
                nChunkYSize = nBlockYSize;
            if( nChunkYOff + nChunkYSize > nYOff + nYSize )
                nChunkYSize = (nYOff + nYSize) - nChunkYOff;

            for( iBufXOff = 0; iBufXOff < nBufXSize; iBufXOff += nChunkXSize )
            {
                nChunkXSize = nBlockXSize;
                nChunkXOff = iBufXOff + nXOff;
                nChunkXSize = nBlockXSize - (nChunkXOff % nBlockXSize);
                if( nChunkXSize == 0 )
                    nChunkXSize = nBlockXSize;
                if( nChunkXOff + nChunkXSize > nXOff + nXSize )
                    nChunkXSize = (nXOff + nXSize) - nChunkXOff;

                GByte *pabyChunkData;

                pabyChunkData = ((GByte *) pData) 
                    + iBufXOff * nPixelSpace 
                    + iBufYOff * nLineSpace;

                for( iBand = 0; iBand < nBandCount; iBand++ )
                {
                    GDALRasterBand *poBand = GetRasterBand(panBandMap[iBand]);
                    
                    eErr = 
                        poBand->GDALRasterBand::IRasterIO( 
                            eRWFlag, nChunkXOff, nChunkYOff, 
                            nChunkXSize, nChunkYSize, 
                            pabyChunkData + iBand * nBandSpace, 
                            nChunkXSize, nChunkYSize, eBufType, 
                            nPixelSpace, nLineSpace );
                    if( eErr != CE_None )
                        return eErr;
                }
            }
        }

        return CE_None;
    }

/* ==================================================================== */
/*      Loop reading required source blocks to satisfy output           */
/*      request.  This is the most general implementation.              */
/* ==================================================================== */

    int         nBandDataSize = GDALGetDataTypeSize( eDataType ) / 8;

    papabySrcBlock = (GByte **) CPLCalloc(sizeof(GByte*),nBandCount);
    papoBlocks = (GDALRasterBlock **) CPLCalloc(sizeof(void*),nBandCount);

/* -------------------------------------------------------------------- */
/*      Compute stepping increment.                                     */
/* -------------------------------------------------------------------- */
    double      dfSrcX, dfSrcY, dfSrcXInc, dfSrcYInc;
    
    dfSrcXInc = nXSize / (double) nBufXSize;
    dfSrcYInc = nYSize / (double) nBufYSize;

/* -------------------------------------------------------------------- */
/*      Loop over buffer computing source locations.                    */
/* -------------------------------------------------------------------- */
    for( iBufYOff = 0; iBufYOff < nBufYSize; iBufYOff++ )
    {
        int     iBufOffset, iSrcOffset;
        
        dfSrcY = (iBufYOff+0.5) * dfSrcYInc + nYOff;
        iSrcY = (int) dfSrcY;

        iBufOffset = iBufYOff * nLineSpace;
        
        for( iBufXOff = 0; iBufXOff < nBufXSize; iBufXOff++ )
        {
            int iSrcX;

            dfSrcX = (iBufXOff+0.5) * dfSrcXInc + nXOff;
            
            iSrcX = (int) dfSrcX;

/* -------------------------------------------------------------------- */
/*      Ensure we have the appropriate block loaded.                    */
/* -------------------------------------------------------------------- */
            if( iSrcX < nLBlockX * nBlockXSize
                || iSrcX >= (nLBlockX+1) * nBlockXSize
                || iSrcY < nLBlockY * nBlockYSize
                || iSrcY >= (nLBlockY+1) * nBlockYSize )
            {
                nLBlockX = iSrcX / nBlockXSize;
                nLBlockY = iSrcY / nBlockYSize;

                int bJustInitialize = 
                    eRWFlag == GF_Write
                    && nYOff <= nLBlockY * nBlockYSize
                    && nYOff + nYSize >= (nLBlockY+1) * nBlockYSize
                    && nXOff <= nLBlockX * nBlockXSize
                    && nXOff + nXSize >= (nLBlockX+1) * nBlockXSize;

                for( iBand = 0; iBand < nBandCount; iBand++ )
                {
                    GDALRasterBand *poBand = GetRasterBand( panBandMap[iBand]);
                    poBlock = poBand->GetLockedBlockRef( nLBlockX, nLBlockY, 
                                                         bJustInitialize );
                    if( poBlock == NULL )
                    {
                        eErr = CE_Failure;
                        goto CleanupAndReturn;
                    }

                    if( eRWFlag == GF_Write )
                        poBlock->MarkDirty();

                    if( papoBlocks[iBand] != NULL )
                        papoBlocks[iBand]->DropLock();
                
                    papoBlocks[iBand] = poBlock;

                    papabySrcBlock[iBand] = (GByte *) poBlock->GetDataRef();
                    if( papabySrcBlock[iBand] == NULL )
                    {
                        eErr = CE_Failure; 
                        goto CleanupAndReturn;
                    }
                }
            }

/* -------------------------------------------------------------------- */
/*      Copy over this pixel of data.                                   */
/* -------------------------------------------------------------------- */
            iSrcOffset = (iSrcX - nLBlockX*nBlockXSize
                + (iSrcY - nLBlockY*nBlockYSize) * nBlockXSize)*nBandDataSize;

            for( iBand = 0; iBand < nBandCount; iBand++ )
            {
                GByte *pabySrcBlock = papabySrcBlock[iBand];
                int iBandBufOffset = iBufOffset + iBand * nBandSpace;
                
                if( eDataType == eBufType )
                {
                    if( eRWFlag == GF_Read )
                        memcpy( ((GByte *) pData) + iBandBufOffset,
                                pabySrcBlock + iSrcOffset, nBandDataSize );
                else
                    memcpy( pabySrcBlock + iSrcOffset, 
                            ((GByte *)pData) + iBandBufOffset, nBandDataSize );
                }
                else
                {
                    /* type to type conversion ... ouch, this is expensive way
                       of handling single words */
                    
                    if( eRWFlag == GF_Read )
                        GDALCopyWords( pabySrcBlock + iSrcOffset, eDataType, 0,
                                       ((GByte *) pData) + iBandBufOffset, 
                                       eBufType, 0, 1 );
                    else
                        GDALCopyWords( ((GByte *) pData) + iBandBufOffset, 
                                       eBufType, 0,
                                       pabySrcBlock + iSrcOffset, eDataType, 0,
                                       1 );
                }
            }

            iBufOffset += nPixelSpace;
        }
    }

/* -------------------------------------------------------------------- */
/*      CleanupAndReturn.                                               */
/* -------------------------------------------------------------------- */
  CleanupAndReturn:
    CPLFree( papabySrcBlock );
    if( papoBlocks != NULL )
    {
        for( iBand = 0; iBand < nBandCount; iBand++ )
        {
            if( papoBlocks[iBand] != NULL )
                papoBlocks[iBand]->DropLock();
        }
        CPLFree( papoBlocks );
    }

    return( CE_None );
}

/************************************************************************/
/*                     GDALDatasetCopyWholeRaster()                     */
/************************************************************************/

/**
 * Copy all dataset raster data.
 *
 * This function copies the complete raster contents of one dataset to 
 * another similarly configured dataset.  The source and destination 
 * dataset must have the same number of bands, and the same width
 * and height.  The bands do not have to have the same data type. 
 *
 * This function is primarily intended to support implementation of 
 * driver specific CreateCopy() functions.  It implements efficient copying,
 * in particular "chunking" the copy in substantial blocks and, if appropriate,
 * performing the transfer in a pixel interleaved fashion.
 *
 * Currently the only papszOptions value supported is "INTERLEAVE=PIXEL"
 * to force pixel interleaved operation.  More options may be supported in
 * the future.  
 *
 * @param hSrcDS the source dataset
 * @param hDstDS the destination dataset
 * @param papszOptions transfer hints in "StringList" Name=Value format.
 * @param pfnProgress progress reporting function.
 * @param pProgressData callback data for progress function.
 *
 * @return CE_None on success, or CE_Failure on failure. 
 */

CPLErr CPL_STDCALL GDALDatasetCopyWholeRaster(
    GDALDatasetH hSrcDS, GDALDatasetH hDstDS, char **papszOptions, 
    GDALProgressFunc pfnProgress, void *pProgressData )

{
    VALIDATE_POINTER1( hSrcDS, "GDALDatasetCopyWholeRaster", CE_Failure );
    VALIDATE_POINTER1( hDstDS, "GDALDatasetCopyWholeRaster", CE_Failure );

    GDALDataset *poSrcDS = (GDALDataset *) hSrcDS;
    GDALDataset *poDstDS = (GDALDataset *) hDstDS;
    CPLErr eErr = CE_None;

    if( pfnProgress == NULL )
        pfnProgress = GDALDummyProgress;

/* -------------------------------------------------------------------- */
/*      Confirm the datasets match in size and band counts.             */
/* -------------------------------------------------------------------- */
    int nXSize = poDstDS->GetRasterXSize(), 
        nYSize = poDstDS->GetRasterYSize(),
        nBandCount = poDstDS->GetRasterCount();

    if( poSrcDS->GetRasterXSize() != nXSize 
        || poSrcDS->GetRasterYSize() != nYSize
        || poSrcDS->GetRasterCount() != nBandCount )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Input and output dataset sizes or band counts do not\n"
                  "match in GDALDatasetCopyWholeRaster()" );
        return CE_Failure;
    }

/* -------------------------------------------------------------------- */
/*      Report preliminary (0) progress.                                */
/* -------------------------------------------------------------------- */
    if( !pfnProgress( 0.0, NULL, pProgressData ) )
    {
        CPLError( CE_Failure, CPLE_UserInterrupt, 
                  "User terminated CreateCopy()" );
        return CE_Failure;
    }

/* -------------------------------------------------------------------- */
/*      Get our prototype band, and assume the others are similarly     */
/*      configured.                                                     */
/* -------------------------------------------------------------------- */
    if( nBandCount == 0 )
        return CE_None;
    
    GDALRasterBand *poPrototypeBand = poDstDS->GetRasterBand(1);
    GDALDataType eDT = poPrototypeBand->GetRasterDataType();
    int nBlockXSize, nBlockYSize;

    poPrototypeBand->GetBlockSize( &nBlockXSize, &nBlockYSize );

/* -------------------------------------------------------------------- */
/*      Do we want to try and do the operation in a pixel               */
/*      interleaved fashion?                                            */
/* -------------------------------------------------------------------- */
    int bInterleave = FALSE;
    const char *pszInterleave = NULL;
    
    pszInterleave = poSrcDS->GetMetadataItem( "INTERLEAVE", "IMAGE_STRUCTURE");
    if( pszInterleave != NULL 
        && (EQUAL(pszInterleave,"PIXEL") || EQUAL(pszInterleave,"LINE")) )
        bInterleave = TRUE;

    pszInterleave = poDstDS->GetMetadataItem( "INTERLEAVE", "IMAGE_STRUCTURE");
    if( pszInterleave != NULL 
        && (EQUAL(pszInterleave,"PIXEL") || EQUAL(pszInterleave,"LINE")) )
        bInterleave = TRUE;

    pszInterleave = CSLFetchNameValue( papszOptions, "INTERLEAVE" );
    if( pszInterleave != NULL 
        && (EQUAL(pszInterleave,"PIXEL") || EQUAL(pszInterleave,"LINE")) )
        bInterleave = TRUE;

    /* If the destination is compressed, we must try to write blocks just once, to save */
    /* disk space (GTiff case for example), and to avoid data loss (JPEG compression for example) */
    int bDstIsCompressed = FALSE;
    const char* pszDstCompressed= CSLFetchNameValue( papszOptions, "COMPRESSED" );
    if (pszDstCompressed != NULL && CSLTestBoolean(pszDstCompressed))
        bDstIsCompressed = TRUE;

/* -------------------------------------------------------------------- */
/*      What will our swath size be?                                    */
/* -------------------------------------------------------------------- */
    /* When writing interleaved data in a compressed format, we want to be sure */
    /* that each block will only be written once, so the swath size must not be */
    /* greater than the block cache. */
    /* So as the default cache size is 40 MB, 10 MB is a safe value */
    int nTargetSwathSize = atoi(CPLGetConfigOption("GDAL_SWATH_SIZE", "10000000"));
    if (nTargetSwathSize < 1000000)
        nTargetSwathSize = 1000000;

    /* But let's check that  */
    if (bDstIsCompressed && bInterleave && nTargetSwathSize > GDALGetCacheMax())
    {
        CPLError(CE_Warning, CPLE_AppDefined,
                 "When translating into a compressed interleave format, the block cache size (%d) "
                 "should be at least the size of the swath (%d)", GDALGetCacheMax(), nTargetSwathSize);
    }

    void *pSwathBuf;

    int nPixelSize = (GDALGetDataTypeSize(eDT) / 8);
    if( bInterleave)
        nPixelSize *= nBandCount;

    // aim for one row of blocks.  Do not settle for less.
    int nSwathCols  = nXSize;
    int nSwathLines = nBlockYSize;

    int nMemoryPerLine = nXSize * nPixelSize;

    /* Do the computation on a big int since for example when translating */
    /* the JPL WMS layer, we overflow 32 bits*/
    GIntBig nSwathBufSize = (GIntBig)nMemoryPerLine * nSwathLines;
    if (nSwathBufSize > (GIntBig)nTargetSwathSize)
    {
        nSwathLines = nTargetSwathSize / nMemoryPerLine;
        if (nSwathLines == 0)
            nSwathLines = 1;
        CPLDebug( "GDAL", 
              "GDALDatasetCopyWholeRaster(): adjusting to %d line swath "
              "since requirement (%d * %d bytes) exceed target swath size (%d bytes) ",
              nSwathLines, nBlockYSize, nMemoryPerLine, nTargetSwathSize);
    }
    // If we are processing single scans, try to handle several at once.
    // If we are handling swaths already, only grow the swath if a row
    // of blocks is substantially less than our target buffer size.
    else if( nSwathLines == 1 
        || nMemoryPerLine * nSwathLines < nTargetSwathSize / 10 )
        nSwathLines = MIN(nYSize,MAX(1,nTargetSwathSize/nMemoryPerLine));


    if (bDstIsCompressed)
    {
        if (nSwathLines < nBlockYSize)
        {
            nSwathLines = nBlockYSize;

            /* Number of pixels that can be read/write simultaneously */
            nSwathCols = nTargetSwathSize / (nSwathLines * nPixelSize);
            nSwathCols = (nSwathCols / nBlockXSize) * nBlockXSize;
            if (nSwathCols == 0)
                nSwathCols = nBlockXSize;
            if (nSwathCols > nXSize)
                nSwathCols = nXSize;

            CPLDebug( "GDAL", 
              "GDALDatasetCopyWholeRaster(): because of compression and too high block,\n"
              "use partial width at one time");
        }
        else
        {
            /* Round on a multiple of nBlockYSize */
            nSwathLines = (nSwathLines / nBlockYSize) * nBlockYSize;
            CPLDebug( "GDAL", 
              "GDALDatasetCopyWholeRaster(): because of compression, \n"
              "round nSwathLines to block height : %d", nSwathLines);
        }
    }

    pSwathBuf = VSIMalloc3(nSwathCols, nSwathLines, nPixelSize );
    if( pSwathBuf == NULL )
    {
        CPLError( CE_Failure, CPLE_OutOfMemory,
                "Failed to allocate %d*%d*%d byte swath buffer in\n"
                "GDALDatasetCopyWholeRaster()",
                nSwathCols, nSwathLines, nPixelSize );
        return CE_Failure;
    }

    CPLDebug( "GDAL", 
            "GDALDatasetCopyWholeRaster(): %d*%d swaths, bInterleave=%d", 
            nSwathCols, nSwathLines, bInterleave );

/* ==================================================================== */
/*      Band oriented (uninterleaved) case.                             */
/* ==================================================================== */
    if( !bInterleave )
    {
        int iBand, iX, iY;

        for( iBand = 0; iBand < nBandCount && eErr == CE_None; iBand++ )
        {
            int nBand = iBand+1;

            for( iY = 0; iY < nYSize && eErr == CE_None; iY += nSwathLines )
            {
                int nThisLines = nSwathLines;

                if( iY + nThisLines > nYSize )
                    nThisLines = nYSize - iY;

                for( iX = 0; iX < nXSize && eErr == CE_None; iX += nSwathCols )
                {
                    int nThisCols = nSwathCols;

                    if( iX + nThisCols > nXSize )
                        nThisCols = nXSize - iX;

                    eErr = poSrcDS->RasterIO( GF_Read, 
                                            iX, iY, nThisCols, nThisLines,
                                            pSwathBuf, nThisCols, nThisLines, 
                                            eDT, 1, &nBand, 
                                            0, 0, 0 );

                    if( eErr == CE_None )
                        eErr = poDstDS->RasterIO( GF_Write, 
                                                iX, iY, nThisCols, nThisLines,
                                                pSwathBuf, nThisCols, nThisLines, 
                                                eDT, 1, &nBand,
                                                0, 0, 0 );

                    if( eErr == CE_None 
                        && !pfnProgress( 
                            iBand / (float)nBandCount
                            + (iY+nThisLines) / (float) (nYSize*nBandCount),
                            NULL, pProgressData ) )
                    {
                        eErr = CE_Failure;
                        CPLError( CE_Failure, CPLE_UserInterrupt, 
                                "User terminated CreateCopy()" );
                    }
                }
            }
        }
    }

/* ==================================================================== */
/*      Pixel interleaved case.                                         */
/* ==================================================================== */
    else /* if( bInterleave ) */
    {
        int iY, iX;

        for( iY = 0; iY < nYSize && eErr == CE_None; iY += nSwathLines )
        {
            int nThisLines = nSwathLines;

            if( iY + nThisLines > nYSize )
                nThisLines = nYSize - iY;

            for( iX = 0; iX < nXSize && eErr == CE_None; iX += nSwathCols )
            {
                int nThisCols = nSwathCols;

                if( iX + nThisCols > nXSize )
                    nThisCols = nXSize - iX;

                eErr = poSrcDS->RasterIO( GF_Read, 
                                        iX, iY, nThisCols, nThisLines,
                                        pSwathBuf, nThisCols, nThisLines, 
                                        eDT, nBandCount, NULL, 
                                        0, 0, 0 );

                if( eErr == CE_None )
                    eErr = poDstDS->RasterIO( GF_Write, 
                                            iX, iY, nThisCols, nThisLines,
                                            pSwathBuf, nThisCols, nThisLines, 
                                            eDT, nBandCount, NULL, 
                                            0, 0, 0 );

                if( eErr == CE_None 
                    && !pfnProgress( (iY+nThisLines) / (float) nYSize,
                                    NULL, pProgressData ) )
                {
                    eErr = CE_Failure;
                    CPLError( CE_Failure, CPLE_UserInterrupt, 
                            "User terminated CreateCopy()" );
                }
            }
        }
    }

/* -------------------------------------------------------------------- */
/*      Cleanup                                                         */
/* -------------------------------------------------------------------- */
    CPLFree( pSwathBuf );

    return eErr;
}
