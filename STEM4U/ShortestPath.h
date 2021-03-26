#ifndef _STEM4U_ShortestPath_h_
#define _STEM4U_ShortestPath_h_

#include <ScatterDraw/MultiDimMatrixIndex.h>

namespace Upp {

template <typename T> 
Rect_<T> Envelope(const Vector<Point_<T>> &poly, const Rect_<T> &rect = Null) {
	Rect_<T> ret;
	if (IsNull(rect))
		ret = Rect_<T>(DBL_MAX, -DBL_MAX, -DBL_MAX, DBL_MAX);
	else
		ret = rect;
	for (auto &p : poly) {
		if (ret.left > p.x)
			ret.left = p.x;
		if (ret.right < p.x)
			ret.right = p.x;
		if (ret.top < p.y)
			ret.top = p.y;
		if (ret.bottom > p.y)
			ret.bottom = p.y;
	}
	return ret;	
}

template <typename T> 
Rect_<T> EnvelopeAnd(const Vector<Vector<Point_<T>>> &polys) {
	Rect_<T> ret(DBL_MAX, -DBL_MAX, -DBL_MAX, DBL_MAX);
	for (auto &poly : polys) 
		ret = Envelope(poly, ret);
	return ret;
}

template <typename T> 
Rect_<T> EnvelopeOr(const Vector<Vector<Point_<T>>> &polys) {
	Rect_<T> ret(DBL_MAX, -DBL_MAX, -DBL_MAX, DBL_MAX);
	for (auto &poly : polys) {
		Rect_<T> r = Envelope(poly);
		if (r.left < ret.left)
			ret.left = r.left;
		if (r.bottom < ret.bottom)
			ret.bottom = r.bottom;	
		if (r.right > ret.right)
			ret.right = r.right;	
		if (r.top > ret.top)
			ret.top = r.top;	
	}
	return ret;
}

	
template <typename T>
struct SegSP : Moveable<SegSP<T>> {
	int node;
	T weight;
	SegSP() {}
	SegSP(int node, T weight) : node(node), weight(weight) {}
};

template <typename T>
inline bool operator<(const SegSP<T>& left, const SegSP<T>& right) {return left.weight < right.weight;}

template <typename T>
T SumWeight(T a, T b) {
	if (a == std::numeric_limits<T>::max() || b == std::numeric_limits<T>::max())
		return std::numeric_limits<T>::max();
	return a+b;
}

template <typename T>
Vector<SegSP<T>> Dijkstra(const Vector<Vector<SegSP<T>>> &adjList, int start) {
    Vector<SegSP<T>> dist(adjList.size());
    
    for(int i = 0; i < adjList.size(); i++)
        dist[i] = SegSP<T>(i, std::numeric_limits<T>::max());
    dist[start] = SegSP<T>(start, 0);
        
    SortedIndex<SegSP<T>> pq;
    pq << SegSP<T>(start, 0);
    
    while(!pq.empty()) {
        int u = pq[0].node;
        pq.Remove(0);
      
        for(auto &seg : adjList[u]) {
            int v = seg.node;
            T sum = dist[u].weight + seg.weight;
            if(dist[v].weight > sum) {
                dist[v].weight = sum;
                dist[v].node = u;
                pq << SegSP<T>(v, dist[v].weight);
            }
        }
    }
    return dist;
}

template <typename T>
Vector<SegSP<T>> BellmanFord(const Vector<Vector<SegSP<T>>> &adjList, int start) {
	Vector<SegSP<T>> dist(adjList.size());
    
	int n = int(adjList.size());
    for(int i = 0; i < n; i++)
		dist[i] = SegSP<T>(i, std::numeric_limits<T>::max());
	dist[start] = SegSP<T>(start, 0);
 
    for(int i = 0; i < n-1; i++) {
        for(int u = 0; u < n; u++) {
            for(auto &seg : adjList[u]) {
	            int v = seg.node;
	            T sum = SumWeight<T>(dist[u].weight, seg.weight);
	            if(dist[v].weight > sum) {
	                dist[v].weight = sum;
	                dist[v].node = u;
	            }
            }
        }
    }  
    return dist;
}

template <typename T>
Vector<Vector<T>> FloydWarshall(Vector<Vector<T>> &adjMatrix) {
    Vector<Vector<T>> dist = clone(adjMatrix);
    int n = adjMatrix.size();
    
    for(int w = 0; w < n; w++) {
        Vector<T> &dist_w = dist[w];
        for(int u = 0; u < n; u++) {
            Vector<T> &dist_u = dist[u];
            for(int v = 0; v < n; v++) {
                T sum = dist_u[w] + dist_w[v];
                if(dist_u[v] > sum)
                    dist_u[v] = sum;
            }
        }
 	}
    return dist;
}

class MapShortestPath {
public:
	MapShortestPath(const Vector<Vector<Pointf>> &perimetersAllowed,
				    const Vector<Vector<Pointf>> &perimetersForbidden, int rows = 100, int cols = 100) {
		Init(perimetersAllowed, perimetersForbidden, rows, cols);
	}
	MapShortestPath(const Vector<Vector<Pointf>> &perimetersAllowed, int rows = 100, int cols = 100) {
		Init(perimetersAllowed, rows, cols);
	}
	void Init(const Vector<Vector<Pointf>> &perimetersAllowed,
			  const Vector<Vector<Pointf>> &perimetersForbidden, int rows = 100, int cols = 100);
	void Init(const Vector<Vector<Pointf>> &perimetersAllowed, int rows = 100, int cols = 100) {
		Vector<Vector<Pointf>> perimetersForbidden;
		Init(perimetersAllowed, perimetersForbidden, rows, cols);
	}
	void SetInitPoint(int _idFrom) 			{dist = Dijkstra(adjList, idFrom = _idFrom);}
	void SetInitPoint(const Pointf &end) 	{SetInitPoint(ClosestId(end));}
	
	double Weight(int end) const {return dist[end].weight;}		
	int ClosestId(const Pointf &point) const;
	Vector<int> Track(int end) const;
	Pointf PointFromId(int id) const;
	Vector<Pointf> MapPoints() const;
	
private:	
	Vector<SegSP<double>> dist;
	Vector<Vector<SegSP<double>>> adjList;
	Buffer<bool> map;
	int rows, cols;
	Rectf rect;
	int idFrom;
	double deltaw, deltah;
	MultiDimMatrixIndex indx;

	Pointf PointFromRC(int r, int c) const {return Pointf(rect.left + c*deltaw, rect.top - r*deltah);}
};

}
	
#endif
