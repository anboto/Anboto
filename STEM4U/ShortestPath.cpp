// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2022, the Anboto author and contributors
#include <Core/Core.h>
#include <Geom/Geom.h>
#include "ShortestPath.h"

namespace Upp {

void MapShortestPath::Init(const Vector<Vector<Pointf>> &perimetersAllowed,
						   const Vector<Vector<Pointf>> &perimetersForbidden, int rows, int cols) {
    this->rows = rows;
    this->cols = cols;
    
    rect = EnvelopeOr(perimetersAllowed);
    double width  = rect.GetWidth();
    deltaw = width/(cols-1);
    double height = abs(rect.GetHeight());
    deltah = height/(rows-1);
    
    indx.SetAxis(rows, cols);
    map.Alloc(indx.size());
    for (int r = 0; r < rows; ++r) {
    	for (int c = 0; c < cols; ++c) {
    		Pointf p(rect.left + c*deltaw, rect.top - r*deltah);
    		bool allowed = false;
    		for (auto &per: perimetersAllowed) {
    			Array<Pointf> aper(per.size());
    			for (int i = 0; i < aper.size(); ++i)
    				aper[i] = per[i];
    			if (ContainsPoint(aper, p) != CMP_OUT) {
    				allowed = true;
    				break;
    			}
    		}
    		if (allowed) {
	    		for (auto &per: perimetersForbidden) {
	    			Array<Pointf> aper(per.size());
	    			for (int i = 0; i < aper.size(); ++i)
	    				aper[i] = per[i];
	    			if (ContainsPoint(aper, p) != CMP_OUT) {
	    				allowed = false;
	    				break;
	    			}
	    		}
    		}
    		map[indx(r, c)] = allowed;	    		
    	}
    }
    
    adjList.SetCount(indx.size());
    
    double diag = sqrt(deltaw*deltaw + deltah*deltah);
    for (int r = 0; r < rows; ++r) {
    	for (int c = 0; c < cols; ++c) {
    		int id = indx(r, c);	
			double wx   = map[id] ? deltaw : DBL_MAX;	    		
			double wy   = map[id] ? deltah : DBL_MAX;	    		
			double wang = map[id] ? diag   : DBL_MAX;
			Vector<SegSP<double>> &lst = adjList[id];	
			if (r > 0) {
				int id2 = indx(r-1, c);
				lst << SegSP<double>(id2, map[id2] ? wy : DBL_MAX);
				if (c > 0) {
					int id2 = indx(r-1, c-1);
					lst << SegSP<double>(id2, map[id2] ? wang : DBL_MAX);
				}
				if (c < cols-1) {
					int id2 = indx(r-1, c+1);
					lst << SegSP<double>(id2, map[id2] ? wang : DBL_MAX);
				}
			}
			if (c > 0) {
				int id2 = indx(r, c-1);
				lst << SegSP<double>(id2, map[id2] ? wx : DBL_MAX);
			}
			if (r < rows-1) {
				int id2 = indx(r+1, c);
				lst << SegSP<double>(id2, map[id2] ? wy : DBL_MAX);
				if (c > 0) {
					int id2 = indx(r+1, c-1);
					lst << SegSP<double>(id2, map[id2] ? wang : DBL_MAX);
				}
				if (c < cols-1) {
					int id2 = indx(r+1, c+1);
					lst << SegSP<double>(id2, map[id2] ? wang : DBL_MAX);
				}
			}
			if (c < cols-1) {
				int id2 = indx(r, c+1);
				lst << SegSP<double>(id2, map[id2] ? wx : DBL_MAX);
			}
    	}
	}
}

int MapShortestPath::ClosestId(const Pointf &point) const {
	int minr, minc;
	double minD = DBL_MAX;
    for (int r = 0; r < rows; ++r) {
    	for (int c = 0; c < cols; ++c) {
    		Pointf p = PointFromRC(r, c);
    		double d = Distance(p, point);
    		if (minD > d) { 
    			minD = d;
    			minr = r;
    			minc = c;
    		}
    	}
    }
    return indx(minr, minc);	
}

Vector<int> MapShortestPath::Track(int end) const {
    Vector<int> ret;
    
    ret << end;
    while(end != idFrom) 
        ret << (end = dist[end].node);
    return ret;
}

Pointf MapShortestPath::PointFromId(int id) const {
	int c = int(id/rows);
	int r = id - c*rows;
	
	return PointFromRC(r, c);
}

Vector<Pointf> MapShortestPath::MapPoints() const {
	Vector<Pointf> ret;
    for (int r = 0; r < rows; ++r) {
    	for (int c = 0; c < cols; ++c) {
    		if (map[indx(r, c)])
    			ret << PointFromRC(r, c);		
    	}
    }
    return ret;
}

}