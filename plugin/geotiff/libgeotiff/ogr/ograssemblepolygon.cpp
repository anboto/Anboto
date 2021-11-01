/******************************************************************************
 * $Id: ograssemblepolygon.cpp 10645 2007-01-18 02:22:39Z warmerdam $
 *
 * Project:  S-57 Reader
 * Purpose:  Implements polygon assembly from a bunch of arcs.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 1999, Frank Warmerdam
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

#include "ogr_geometry.h"
#include "ogr_api.h"
#include "../port/cpl_conv.h"

CPL_CVSID("$Id: ograssemblepolygon.cpp 10645 2007-01-18 02:22:39Z warmerdam $");

/************************************************************************/
/*                            CheckPoints()                             */
/*                                                                      */
/*      Check if two points are closer than the current best            */
/*      distance.  Update the current best distance if they are.        */
/************************************************************************/

static int CheckPoints( OGRLineString *poLine1, int iPoint1,
                        OGRLineString *poLine2, int iPoint2,
                        double *pdfDistance )

{
    double      dfDeltaX, dfDeltaY, dfDistance;

    if( pdfDistance == NULL || *pdfDistance == 0 )
        return poLine1->getX(iPoint1) == poLine2->getX(iPoint2)
            && poLine1->getY(iPoint1) == poLine2->getY(iPoint2);

    dfDeltaX = poLine1->getX(iPoint1) - poLine2->getX(iPoint2);
    dfDeltaY = poLine1->getY(iPoint1) - poLine2->getY(iPoint2);

    dfDeltaX = ABS(dfDeltaX);
    dfDeltaY = ABS(dfDeltaY);
    
    if( dfDeltaX > *pdfDistance || dfDeltaY > *pdfDistance )
        return FALSE;
    
    dfDistance = sqrt(dfDeltaX*dfDeltaX + dfDeltaY*dfDeltaY);

    if( dfDistance < *pdfDistance )
    {
        *pdfDistance = dfDistance;
        return TRUE;
    }
    else
        return FALSE;
}

/************************************************************************/
/*                           AddEdgeToRing()                            */
/************************************************************************/

static void AddEdgeToRing( OGRLinearRing * poRing, OGRLineString * poLine,
                           int bReverse )

{
/* -------------------------------------------------------------------- */
/*      Establish order and range of traverse.                          */
/* -------------------------------------------------------------------- */
    int         iStart=0, iEnd=0, iStep=0;
    int         nVertToAdd = poLine->getNumPoints();

    if( !bReverse )
    {
        iStart = 0;
        iEnd = nVertToAdd - 1;
        iStep = 1;
    }
    else
    {
        iStart = nVertToAdd - 1;
        iEnd = 0;
        iStep = -1;
    }

/* -------------------------------------------------------------------- */
/*      Should we skip a repeating vertex?                              */
/* -------------------------------------------------------------------- */
    if( poRing->getNumPoints() > 0 
        && CheckPoints( poRing, poRing->getNumPoints()-1, 
                        poLine, iStart, NULL ) )
    {
        iStart += iStep;
    }

    poRing->addSubLineString( poLine, iStart, iEnd );
}

/************************************************************************/
/*                      OGRBuildPolygonFromEdges()                      */
/************************************************************************/

/**
 * Build a ring from a bunch of arcs.
 *
 * @param hLines handle to an OGRGeometryCollection (or OGRMultiLineString) containing the line string geometries to be built into rings.
 * @param bBestEffort not yet implemented???.
 * @param bAutoClose indicates if the ring should be close when first and
 * last points of the ring are the same.
 * @param dfTolerance tolerance into which two arcs are considered
 * close enough to be joined.
 * @param peErr OGRERR_NONE on success, or OGRERR_FAILURE on failure.
 * @return an handle to the new geometry, a polygon.
 *
 */

OGRGeometryH OGRBuildPolygonFromEdges( OGRGeometryH hLines,
                                       int bBestEffort, 
                                       int bAutoClose,
                                       double dfTolerance, 
                                       OGRErr * peErr )

{
    int         bSuccess = TRUE;
    OGRGeometryCollection *poLines = (OGRGeometryCollection *) hLines;
    OGRPolygon  *poPolygon = new OGRPolygon();

    (void) bBestEffort;

/* -------------------------------------------------------------------- */
/*      Setup array of line markers indicating if they have been        */
/*      added to a ring yet.                                            */
/* -------------------------------------------------------------------- */
    int         nEdges = poLines->getNumGeometries();
    int         *panEdgeConsumed, nRemainingEdges = nEdges;

    panEdgeConsumed = (int *) CPLCalloc(sizeof(int),nEdges);

/* ==================================================================== */
/*      Loop generating rings.                                          */
/* ==================================================================== */
    while( nRemainingEdges > 0 )
    {
        int             iEdge;
        OGRLineString   *poLine;
        
/* -------------------------------------------------------------------- */
/*      Find the first unconsumed edge.                                 */
/* -------------------------------------------------------------------- */
        for( iEdge = 0; panEdgeConsumed[iEdge]; iEdge++ ) {}

        poLine = (OGRLineString *) poLines->getGeometryRef(iEdge);

/* -------------------------------------------------------------------- */
/*      Start a new ring, copying in the current line directly          */
/* -------------------------------------------------------------------- */
        OGRLinearRing   *poRing = new OGRLinearRing();
        
        AddEdgeToRing( poRing, poLine, FALSE );

        panEdgeConsumed[iEdge] = TRUE;
        nRemainingEdges--;
        
/* ==================================================================== */
/*      Loop adding edges to this ring until we make a whole pass       */
/*      within finding anything to add.                                 */
/* ==================================================================== */
        int             bWorkDone = TRUE;
        double          dfBestDist = dfTolerance;

        while( !CheckPoints(poRing,0,poRing,poRing->getNumPoints()-1,NULL)
               && nRemainingEdges > 0
               && bWorkDone )
        {
            int         iBestEdge = -1, bReverse = FALSE;

            bWorkDone = FALSE;
            dfBestDist = dfTolerance;

            // We consider linking the end to the beginning.  If this is
            // closer than any other option we will just close the loop.

            //CheckPoints(poRing,0,poRing,poRing->getNumPoints()-1,&dfBestDist);
            
            // Find unused edge with end point closest to our loose end.
            for( iEdge = 0; iEdge < nEdges; iEdge++ )
            {
                if( panEdgeConsumed[iEdge] )
                    continue;

                poLine = (OGRLineString *) poLines->getGeometryRef(iEdge);
                
                if( CheckPoints(poLine,0,poRing,poRing->getNumPoints()-1,
                                &dfBestDist) )
                {
                    iBestEdge = iEdge;
                    bReverse = FALSE;
                }
                if( CheckPoints(poLine,poLine->getNumPoints()-1,
                                poRing,poRing->getNumPoints()-1,
                                &dfBestDist) )
                {
                    iBestEdge = iEdge;
                    bReverse = TRUE;
                }
            }

            // We found one within tolerance - add it.
            if( iBestEdge != -1 )
            {
                poLine = (OGRLineString *) 
                    poLines->getGeometryRef(iBestEdge);
                
                AddEdgeToRing( poRing, poLine, bReverse );
                    
                panEdgeConsumed[iBestEdge] = TRUE;
                nRemainingEdges--;
                bWorkDone = TRUE;
            }
        }

/* -------------------------------------------------------------------- */
/*      Did we fail to complete the ring?                               */
/* -------------------------------------------------------------------- */
        dfBestDist = dfTolerance;

        if( !CheckPoints(poRing,0,poRing,poRing->getNumPoints()-1,
                         &dfBestDist) )
        {
            CPLDebug( "OGR", 
                     "Failed to close ring %d.\n"
                     "End Points are: (%.8f,%.7f) and (%.7f,%.7f)\n",
                     poPolygon->getNumInteriorRings()+1,
                     poRing->getX(0), poRing->getY(0), 
                     poRing->getX(poRing->getNumPoints()-1), 
                     poRing->getY(poRing->getNumPoints()-1) );

            bSuccess = FALSE;
        }

/* -------------------------------------------------------------------- */
/*      Do we need to auto-close this ring?                             */
/* -------------------------------------------------------------------- */
        if( bAutoClose
            && !CheckPoints(poRing,0,poRing,poRing->getNumPoints()-1,NULL) )
        {
            poRing->addPoint( poRing->getX(0), 
                              poRing->getY(0), 
                              poRing->getZ(0));
        }

        poPolygon->addRingDirectly( poRing );
    } /* next ring */

/* -------------------------------------------------------------------- */
/*      Cleanup.                                                        */
/* -------------------------------------------------------------------- */
    CPLFree( panEdgeConsumed );

// Eventually we should at least identify the external ring properly,
// perhaps even ordering the direction of rings, though this isn't
// required by the OGC geometry model.

    if( peErr != NULL )
    {
        if( bSuccess )
            *peErr = OGRERR_NONE;
        else
            *peErr = OGRERR_FAILURE;
    }
    
    return (OGRGeometryH) poPolygon;
}



