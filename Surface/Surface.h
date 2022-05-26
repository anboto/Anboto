// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2022, the Anboto author and contributors
#ifndef _GLCanvas_surface_h_
#define _GLCanvas_surface_h_

#include <Eigen/Eigen.h>
#include <Functions4U/Functions4U.h>

namespace Upp {
using namespace Eigen;

const double EPS_XYZ = 0.001;

template<class T>
inline T avg(T a, T b) 			{return T(a+b)/2;}
template<class T>
inline T avg(T a, T b, T c)		{return T(a+b+c)/3;}
 
template<class T>
void Sort(T& a, T& b, T& c, T& d) {
	if (a > b) 
		Swap(a, b);
	if (c > d) 
		Swap(c, d);
	if (a > c) 
		Swap(a, c);
	if (b > d) 
		Swap(b, d);
	if (b > c) 
		Swap(b, c);
}

template<class T>
void Sort(T& a, T& b, T& c) {
	if (a > b) 
		Swap(a, b);
	if (a > c) 
		Swap(a, c);
	if (b > c) 
		Swap(b, c);
}
	

class Value3D : public Moveable<Value3D> {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	
	double x, y, z;
	
	Value3D() 									{}
	Value3D(const Nuller&) 						{SetNull();}
	Value3D(const Value3D &p) 					{Set(p);}
	Value3D(const Vector3d &p) 					{Set(p);}
	Value3D(double _x, double _y, double _z) 	{Set(_x, _y, _z);}
	
	void SetNull() 				{x = Null; y = 0;}
	bool IsNullInstance() const	{return IsNull(x);}
	
	void Reset() 			{x = y = z = 0;}
	
	Value3D(bool positive)	{x = Null; y = positive ? 1 : -1;}
	bool IsPosInf()			{return IsNull(x) && y == 1;}
	bool IsNegInf()			{return IsNull(x) && y == -1;}
	
	void Set(const Value3D &p) 					{x = p.x;	y = p.y;	z = p.z;}
	void Set(const Vector3d &p) 				{x = p(0);	y = p(1);	z = p(2);}
	void Set(double _x, double _y, double _z) 	{x = _x;  	y = _y;  	z = _z;}
	
	inline Value3D operator=(const Value3D &p)	{Set(p);	return *this;}
	inline Value3D operator=(const Vector3d &p)	{Set(p);	return *this;}
	
	String ToString() const {return Format("x: %s. y: %s. z: %s", FDS(x, 10, true), FDS(y, 10, true), FDS(z, 10, true));}
	
	inline bool IsSimilar(const Value3D &p, double similThres) const {
		if (abs(p.x - x) < similThres && abs(p.y - y) < similThres && abs(p.z - z) < similThres)
			return true;
		return false;
	}
	#pragma GCC diagnostic ignored "-Wattributes"
	friend bool operator==(const Value3D& a, const Value3D& b) {return a.IsSimilar(b, EPS_XYZ);}
	friend bool operator!=(const Value3D& a, const Value3D& b) {return !a.IsSimilar(b, EPS_XYZ);}
	#pragma GCC diagnostic warning "-Wattributes"
	
	void Translate(double dx, double dy, double dz);
	void TransRot(const Affine3d &quat);
	void TransRot(double dx, double dy, double dz, double ax, double ay, double az, double cx, double cy, double cz);
	void Rotate(double ax, double ay, double az, double cx, double cy, double cz);	
		
	// Dot product or scalar product
	double dot(const Value3D& a) const {return x*a.x + y*a.y + z*a.z;}
	
	// Cross product or vector product X (or wedge product ∧ in 3D) 
	inline friend Value3D operator%(const Value3D& a, const Value3D& b) {return Value3D(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x);}
	
	inline friend Value3D operator+(const Value3D& a, const Value3D& b) {return Value3D(a.x+b.x, a.y+b.y, a.z+b.z);}
	inline friend Value3D operator-(const Value3D& a, const Value3D& b) {return Value3D(a.x-b.x, a.y-b.y, a.z-b.z);}
	inline friend Value3D operator*(const Value3D& a, double b) 		{return Value3D(a.x*b, a.y*b, a.z*b);}

	inline void operator+=(const Value3D& a) {x += a.x; y += a.y; z += a.z;}
	inline void operator-=(const Value3D& a) {x -= a.x; y -= a.y; z -= a.z;}

	double Length() const {return ::sqrt(x*x + y*y + z*z);}
	Value3D &Normalize() {
		double length = Length();
		
		if (length < 1e-10) 
			Reset();
		else {
		    x = x/length;
		    y = y/length;
		    z = z/length;
		}
		return *this;
	}
	double Distance(const Value3D &p)  		const {return ::sqrt(sqr(x-p.x) + sqr(y-p.y) + sqr(z-p.z));}
	inline double Length(const Value3D &p) 	const {return Distance(p);}
	double Manhattan(const Value3D &p) 		const {return abs(x-p.x) + abs(y-p.y) + abs(z-p.z);}
	double Manhattan() 				   		const {return abs(x) + abs(y) + abs(z);}
	
	double Angle(const Value3D &p) const {return acos(dot(p)/(Length()*p.Length()));}
	
	void SimX() {x = -x;}
	void SimY() {y = -y;}
	void SimZ() {z = -z;}
	
	void Mirror(const Value3D &p0) {
		x = 2*p0.x - x;
		y = 2*p0.y - y;
		z = 2*p0.z - z;
	}
	void Mirror() {
		SimX();
		SimY();
		SimZ();
	}
	void Jsonize(JsonIO &json) {
		json
			("x", x)
			("y", y)
			("z", z)
		;
	}
};

typedef Value3D Direction3D;
typedef Value3D Point3D;

void GetTransform(Affine3d &aff, double ax, double ay, double az, double cx, double cy, double cz);
void GetTransform(Affine3d &aff, double dx, double dy, double dz, double ax, double ay, double az, double cx, double cy, double cz);	

void TransRot(const Point3D &pos, const Point3D &ref, VectorXd &x, VectorXd &y, VectorXd &z, VectorXd &rx, VectorXd &ry, VectorXd &rz);

class Value6D {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	
	Value6D() {}
	Value6D(const Value6D &f)	{Set(f);}
	Value6D(const VectorXd &v)	{Set(v);}
	template<typename T>
	Value6D(const T *v)			{Set(v);}
	Value6D(double v0, double v1, double v2, double v3, double v4, double v5) {Set(v0, v1, v2, v3, v4, v5);}
	
	void Set(const Value6D &f)	{t.x = f.t.x;	t.y = f.t.y;	t.z = f.t.z;	r.x = f.r.x;	r.y = f.r.y;	r.z = f.r.z;}
	void Set(const VectorXd &v) {
		ASSERT(v.size() == 6);
		Set(v.data());
	}
	template<typename T>
	void Set(const T *v) {
		t.x = v[0];	t.y = v[1];	t.z = v[2];	r.x = v[3];	r.y = v[4];	r.z = v[5];
	}
	template<typename T>
	void Add(const T *v) {
		t.x+= v[0];	t.y+= v[1];	t.z+= v[2];	r.x+= v[3];	r.y+= v[4];	r.z+= v[5];
	}
	void Add(const VectorXd &v) {
		ASSERT(v.size() == 6);
		Add(v.data());
	}
	void Set(double v0, double v1, double v2, double v3, double v4, double v5) {
		t.x = v0;	t.y = v1;	t.z = v2;	r.x = v3;	r.y = v4;	r.z = v5;
	}
	void Reset() {t.Reset();	r.Reset();}
	
	void operator*=(double v) 	{t.x *= v;	t.y *= v;		t.z *= v;		r.x *= v;	r.y *= v;	r.z *= v;}
	
	double& operator[](int id) {
		ASSERT(id >= 0 && id < 6);
		switch (id) {
		case 0:	return t.x;
		case 1:	return t.y;
		case 2:	return t.z;
		case 3:	return r.x;
		case 4:	return r.y;
		default:return r.z;
		}
	}
	String ToString() const {
		return Format("x: %s. y: %s. z: %s. rx: %s. ry: %s. rz: %s", 
			FDS(t.x, 10, true), FDS(t.y, 10, true), FDS(t.z, 10, true),
			FDS(r.x, 10, true), FDS(r.y, 10, true), FDS(r.z, 10, true));
	}
	VectorXd ToVector() const {
		VectorXd v(6);
		ToC(v.data());
		return v;
	}
	template <typename T>
	void ToC(T *v) const {
		v[0] = T(t.x);	v[1] = T(t.y);	v[2] = T(t.z);	
		v[3] = T(r.x);	v[4] = T(r.y);	v[5] = T(r.z);
	}
	
	static Value6D Zero() {return Value6D(0, 0, 0, 0, 0, 0);}
	
	Value3D t, r;
};

class ForceVector;

class Force6D : public Value6D {
public:
	Force6D() {}
	Force6D(const VectorXd &v) : Value6D(v) {}
	Force6D(double v0, double v1, double v2, double v3, double v4, double v5) : Value6D(v0, v1, v2, v3, v4, v5) {}
	
	void AddLinear(const Direction3D &dir, const Point3D &point, const Point3D &c0);	
	void Add(const Force6D &force, const Point3D &point, const Point3D &c0);
	void Add(const ForceVector &force, const Point3D &c0);
	
	static Force6D Zero() {
		Force6D f;
		f.Zero();
		return f;
	}
};

class ForceVector {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	
	ForceVector() {}
	ForceVector(double x, double y, double z, double fx, double fy, double fz, double rx, double ry, double rz) {
		Set(x, y, z, fx, fy, fz, rx, ry, rz);
	}
	void Set(double x, double y, double z, double fx, double fy, double fz, double rx, double ry, double rz) {
		point.x = x;
		point.y = y;
		point.z = z;
		force.t.x = fx;
		force.t.y = fy;
		force.t.z = fz;
		force.r.x = rx;
		force.r.y = ry;
		force.r.z = rz;
	}
	
	ForceVector &TransRot(double dx, double dy, double dz, double ax, double ay, double az, double cx, double cy, double cz);
	ForceVector &TransRot(const Affine3d &aff);
	String ToString() const {return "From: " + point.ToString() + ". Value: " + force.ToString();}
	
	Force6D force;
	Point3D point;
};

// For dummies: https://dynref.engr.illinois.edu/rkg.html
class Velocity6D : public Value6D {
public:
	template<typename T>
	Velocity6D(const T *v) {Set(v);}
	
	void Translate(const Point3D &from, const Point3D &to) {
		Direction3D rpq = to - from;
		Translate(rpq);
	}
	void Translate(const Direction3D &rpq) {
		t += r%rpq;
	}
};

class Acceleration6D : public Value6D {
public:
	template<typename T>
	Acceleration6D(const T *v) {Set(v);}
	
	void Translate(const Point3D &from, const Point3D &to, const Velocity6D &vel) {
		Direction3D rpq = to - from;
		Translate(rpq, vel);
	}
	void Translate(const Direction3D &rpq, const Velocity6D &vel) {
		t += r%rpq + vel.r%(vel.r%rpq);
	}
};

VectorXd C6ToVector(const double *c);
VectorXd C6ToVector(const float *c);
void Vector6ToC(const VectorXd &v, double *c);
void Vector6ToC(const VectorXd &v, float *c);


double Length(const Point3D &p1, const Point3D &p2);

class Segment3D : public Moveable<Segment3D> {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	
	Point3D from, to;
	
	Segment3D() {}
	Segment3D(const Nuller&) {SetNull();}
	Segment3D(const Point3D &_from, const Point3D &_to) : from(_from), to(_to) {}
	Segment3D(const Point3D &_from, const Direction3D &normal, double length) : from(_from) {
		to = Point3D(from.x + length*normal.x, from.y + length*normal.y, from.z + length*normal.z);
	}
	void SetNull() 				{from = Null;}
	bool IsNullInstance() const	{return IsNull(from);}
		
	void Set(const Point3D &_from, const Point3D &_to) {
		from.Set(_from);
		to.Set(_to);
	}
	void Set(const Point3D &_from, const Direction3D &normal, double length) {
		from.Set(_from);
		to.Set(from.x + length*normal.x, from.y + length*normal.y, from.z + length*normal.z);
	}
	void SimX() {
		from.SimX();
		to.SimX();
	}
	void SimY() {
		from.SimY();
		to.SimY();
	}
	void SimZ() {
		from.SimZ();
		to.SimZ();
	}
	double Length() const {return from.Distance(to);}
	double Dx()	const 	  {return to.x - from.x;}
	double Dy()	const 	  {return to.y - from.y;}	
	double Dz()	const 	  {return to.z - from.z;}
	
	void Mirror(const Point3D &p0) {
		from.Mirror(p0);
		to.Mirror(p0);
	}
	
	Direction3D Direction() {return Direction3D(to - from);}
	
	Point3D IntersectionPlaneX(double x);
	Point3D IntersectionPlaneY(double y);
	Point3D IntersectionPlaneZ(double z);
	
	Point3D Intersection(const Point3D &planePoint, const Direction3D &planeNormal);
	
	bool PointIn(const Point3D &p) const;
	bool SegmentIn(const Segment3D &in, double in_len) const;
	bool SegmentIn(const Segment3D &in) const;
	
	void Translate(double dx, double dy, double dz) {
		from.Translate(dx, dy, dz);
		to.Translate(dx, dy, dz);
	}
	
	void TransRot(const Affine3d &quat) {
		from.TransRot(quat);
		to.TransRot(quat);
	}
};

void DeleteVoidSegments(Vector<Segment3D> &segs);
void DeleteDuplicatedSegments(Vector<Segment3D> &segs);
	
Point3D GetCentroid(const Point3D &a, const Point3D &b);
Point3D GetCentroid(const Point3D &a, const Point3D &b, const Point3D &c);
Direction3D GetNormal(const Point3D &a, const Point3D &b, const Point3D &c);

Point3D Intersection(const Direction3D &lineVector, const Point3D &linePoint, const Direction3D &planeNormal, const Point3D &planePoint);

void TranslateForce(const Point3D &from, const VectorXd &ffrom, Point3D &to, VectorXd &fto);
	
bool PointInSegment(const Point3D &p, const Segment3D &seg);
bool SegmentInSegment(const Segment3D &in, double in_len, const Segment3D &seg);
bool SegmentInSegment(const Segment3D &in, const Segment3D &seg);

Vector<double> GetPolyAngles(const Array<Pointf> &bound);

template <typename T>
inline T const& maxNotNull(T const& a, T const& b) {
	if (IsNull(a))
		return b;
	else if (IsNull(b))
		return a;
	else
    	return a > b ? a : b;
}

template <typename T>
inline T const& minNotNull(T const& a, T const& b) {
	if (IsNull(a))
		return b;
	else if (IsNull(b))
		return a;
	else
    	return a < b ? a : b;
}

class Panel : public MoveableAndDeepCopyOption<Panel> {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	
	int id[4];
	Point3D centroid0, centroid1, centroidPaint;
	Point3D normal0, normal1, normalPaint;
	double surface0, surface1;

	Panel() {}
	Panel(const Panel &orig, int) {
		memcpy(id, orig.id, sizeof(orig.id));
		centroid0 = orig.centroid0;
		centroid1 = orig.centroid1;
		centroidPaint = orig.centroidPaint;
		normal0 = orig.normal0;
		normal1 = orig.normal1;
		normalPaint = orig.normalPaint;
		surface0 = orig.surface0;
		surface1 = orig.surface1;
	}
	bool operator==(const Panel &p) const {
		int id0 = id[0], id1 = id[1], id2 = id[2], id3 = id[3];
		int pid0 = p.id[0], pid1 = p.id[1], pid2 = p.id[2], pid3 = p.id[3];
		if (id0 + id1 + id2 + id3 != pid0 + pid1 + pid2 + pid3)
			return false;
		Sort(id0, id1, id2, id3);
		Sort(pid0, pid1, pid2, pid3);
		if (id0 == pid0 && id1 == pid1 && id2 == pid2 && id3 == pid3)
			return true;
		return false;
	}
	inline void Swap() {
		if (IsTriangle())
			Upp::Swap(id[1], id[2]);
		else
			Upp::Swap(id[1], id[3]);
	}
	inline bool IsTriangle() const	{return id[0] == id[1] || id[0] == id[2] || id[0] == id[3] || 
											id[1] == id[2] || id[1] == id[3] || id[2] == id[3];}
	void RedirectTriangles();
	void ShiftNodes(int shift);
	inline int GetNumNodes() const	{return IsTriangle() ? 3 : 4;}
	bool FirstNodeIs0(int in0, int in1) const;
	static double GetSurface(const Point3D &p0, const Point3D &p1, const Point3D &p2);
	
	String ToString() const { return FormatInt(id[0]) + "," + FormatInt(id[1]) + "," + FormatInt(id[2]) + "," + FormatInt(id[3]); }

	void Jsonize(JsonIO &json) {
		Vector<int> ids;
		if (json.IsStoring()) {
			ids << id[0];
			ids << id[1];
			ids << id[2];
			ids << id[3];	
		}
		json
			("ids", ids)
		;
		if (json.IsLoading()) {
			id[0] = ids[0];
			id[1] = ids[1];
			id[2] = ids[2];
			id[3] = ids[3];
		}
	}
};

class Segment : public MoveableAndDeepCopyOption<Segment> {
public:
	Segment() {}
	Segment(const Segment &orig, int) {
		inode0 = orig.inode0;
		inode1 = orig.inode1;
		panels = clone(orig.panels);
	}
	int inode0, inode1;
	Upp::Index<int> panels;
};

class VolumeEnvelope : MoveableAndDeepCopyOption<VolumeEnvelope> {
public:
	VolumeEnvelope() {Reset();}
	void Reset() 	 {maxX = minX = maxY = minY = maxZ = minZ = Null;}
	VolumeEnvelope(const VolumeEnvelope &orig, int) {
		maxX = orig.maxX;
		minX = orig.minX;
		maxY = orig.maxY;
		minY = orig.minY;
		maxZ = orig.maxZ;
		minZ = orig.minZ;
	}	
	
	void MixEnvelope(VolumeEnvelope &env);
	double Max()	{return max(max(max(abs(maxX), abs(minX)), max(abs(maxY), abs(minY))), max(abs(maxZ), abs(minZ)));}
	double LenRef()	{return max(max(maxX - minX, maxY - minY), maxZ - minZ);}
	
	double maxX, minX, maxY, minY, maxZ, minZ;
};

class Surface : DeepCopyOption<Surface> {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	
	Surface() {}
	Surface(const Surface &surf, int);
		
	void Clear();
	bool IsEmpty() const;
	
	Vector<Point3D> nodes;
	Vector<Panel> panels;
	
	int GetNumNodes() const		{return nodes.GetCount();}
	int GetNumPanels() const	{return panels.GetCount();}
	
	Vector<Segment3D> skewed;
	Vector<Segment3D> segWaterlevel, segTo1panel, segTo3panel;
	
	VolumeEnvelope env;
	
	String Heal(bool basic, Function <bool(String, int pos)> Status = Null);
	void Orient();
	void Image(int axis);
	const VolumeEnvelope &GetEnvelope(); 
	void GetPanelParams();
	String CheckErrors() const;
	double GetSurface();
	double GetSurfaceXProjection(bool positive, bool negative) const;
	double GetSurfaceYProjection(bool positive, bool negative) const;
	double GetSurfaceZProjection(bool positive, bool negative) const;
	Pointf GetSurfaceZProjectionCG() const;
	void GetSegments();
	double GetAvgLenSegment()	{return avgLenSegment;}
	void GetVolume();
	int VolumeMatch(double ratioWarning, double ratioError) const;
	Point3D GetCenterOfBuoyancy() const;
	void GetInertia33(Matrix3d &inertia, const Point3D &center, bool refine = false) const;
	void GetInertia66(MatrixXd &inertia, const Point3D &center, bool refine) const;
	Force6D GetHydrostaticForce(const Point3D &c0, double rho, double g) const;
	Force6D GetHydrostaticForceNormalized(const Point3D &c0) const;
	Force6D GetHydrostaticForceCB(const Point3D &c0, const Point3D &cb, double rho, double g) const;
	Force6D GetHydrostaticForceCBNormalized(const Point3D &c0, const Point3D &cb) const;
	static Force6D GetMassForce(const Point3D &c0, const Point3D &cg, const double mass, const double g);
	void GetHydrostaticStiffness(MatrixXd &c, const Point3D &c0, const Point3D &cg, 
				const Point3D &cb, double rho, double g, double mass);
	double GetWaterPlaneArea() const;
	static Vector<Point3D> GetClosedPolygons(Vector<Segment3D> &segs);
	static Array<Pointf> Point3dto2D(const Vector<Point3D> &bound);
	void AddWaterSurface(Surface &surf, const Surface &under, char c);
	static Vector<Segment3D> GetWaterLineSegments(const Surface &orig);
	bool GetDryPanels(const Surface &surf, bool onlywaterplane);
	char IsWaterPlaneMesh() const; 
	
	void CutX(const Surface &orig, int factor = 1);
	void CutY(const Surface &orig, int factor = 1);
	void CutZ(const Surface &orig, int factor = 1);
	
	void Join(const Surface &orig);
	Vector<Vector<int>> GetPanelSets(Function <bool(String, int pos)> Status);
	
	void TriangleToQuad(int ip);
	void TriangleToQuad(Panel &pan);
		
	void Translate(double dx, double dy, double dz);
	void Rotate(double ax, double ay, double az, double _c_x, double _c_y, double _c_z);
	void TransRot(double dx, double dy, double dz, double ax, double ay, double az, double _c_x, double _c_y, double _c_z);
	
	bool TranslateArchimede(double mass, double rho, double &dz, Surface &under);
	bool Archimede(double mass, Point3D &cg, const Point3D &c0, double rho, double g, double &dz, double &droll, double &dpitch, Surface &under);
	
	bool healing{false};
	int numTriangles, numBiQuads, numMonoQuads;
	Vector<Segment> segments;
	double avgLenSegment = -1;
	int numDupPan, numDupP, numSkewed, numUnprocessed;
	
	double surface = -1, volume = -1, volumex = -1, volumey = -1, volumez = -1;
	double avgFacetSideLen;
	
	void DeployXSymmetry();
	void DeployYSymmetry();
	
	Surface &SelPanels(Vector<int> &_selPanels) {selPanels = pick(_selPanels);	return *this;}
	Surface &SelNodes(Vector<int> &_selNodes) 	{selNodes = pick(_selNodes);	return *this;}
	const Vector<int> &GetSelPanels() const		{return selPanels;}
	const Vector<int> &GetSelNodes() const		{return selNodes;}
	
	void AddNode(Point3D &p);
	int FindNode(Point3D &p);
	
	void AddFlatPanel(double lenX, double lenY, double panelWidth);
	void AddRevolution(Vector<Pointf> &points, double panelWidth);
	void AddPolygonalPanel(Vector<Pointf> &bound, double panelWidth, bool adjustSize);
	void AddPolygonalPanel2(Array<Pointf> &poly, double panelWidth, bool adjustSize);
		
	static int RemoveDuplicatedPanels(Vector<Panel> &_panels);
	static int RemoveTinyPanels(Vector<Panel> &_panels);
	static int RemoveDuplicatedPointsAndRenumber(Vector<Panel> &_panels, Vector<Point3D> &_nodes);
	static void DetectTriBiP(Vector<Panel> &panels) {int dum;	DetectTriBiP(panels, dum, dum, dum);}
	
	//inline const Point3D &GetPos() const 	{return pos;}
	//inline const Point3D &GetAngle() const	{return angle;}
	
	void Jsonize(JsonIO &json) {
		json
			("nodes", nodes)
			("panels", panels)
		;
	}
	
protected:
	struct PanelPoints {
		Point3D data[4];
	};
	void SetPanelPoints(Array<PanelPoints> &pans);
	
private:
	inline bool CheckId(int id) {return id >= 0 && id < nodes.GetCount()-1;}
	bool side = true;
	
	static void DetectTriBiP(Vector<Panel> &panels, int &numTri, int &numBi, int &numP);
	int FixSkewed();
	bool FixSkewed(int ipanel);
	int SegmentInSegments(int iseg) const;
	void AnalyseSegments(double zTolerance);
	void AddSegment(int ip0, int ip1, int ipanel);
	bool ReorientPanels0(bool side);
	void ReorientPanel(int ip);
	bool GetLowest(int &iLowSeg, int &iLowPanel);
	bool SameOrderPanel(int ip0, int ip1, int in0, int in1);
	static int PanelGetNumNodes(const Vector<Panel> &_panels, int ip) {return _panels[ip].GetNumNodes();}
	bool IsPanelTriangle(int ip) 	{return panels[ip].IsTriangle();}
	void GetPanelParams(Panel &panel) const;
	void JointTriangularPanels(int ip0, int ip1, int inode0, int inode1);
	bool FindMatchingPanels(const Array<PanelPoints> &pans, double x, double y, 
							double panelWidth, int &idpan1, int &idpan2);
	
	Vector<int> selPanels, selNodes;
	
	//Point3D pos, angle;
};

class SurfaceMass  {
public:
	double mass;
	Point3D cg;
	Surface surface;
	
	void Jsonize(JsonIO &json) {
		if (json.IsLoading())
			mass = Null;
		json
			("mass", mass)
			("cg", cg)
			("surface", surface)
		;
	}	
};

class SurfaceX : DeepCopyOption<SurfaceX> {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	
	SurfaceX() {}
	SurfaceX(const SurfaceX &surf, int) {}
	
	void Load(const Surface &parent);
	
	static double GetVolume(const SurfaceX &surf, double &vx, double &vy, double &vz);
	static double GetVolume(const SurfaceX &surf);
	static double GetSurface(const SurfaceX &surf);
	
	static double GetSurface(const SurfaceX &surf, bool (*Fun)(double,double,double));
	static double GetVolume(const SurfaceX &surf, bool (*Fun)(double,double,double));
	
	static void TransRot(SurfaceX &surf, const SurfaceX &surf0, const Affine3d &quat); 
	
	static void GetTransformFast(MatrixXd &mat, double dx, double dy, double dz, double ax, double ay, double az);
	static void TransRotFast(double &x, double &y, double &z, double x0, double y0, double z0, const MatrixXd &mat);
	static void TransRotFast(double &x, double &y, double &z, double x0, double y0, double z0,
						 	 double dx, double dy, double dz, double ax, double ay, double az);
	
	static void GetTransform(Affine3d &aff, double dx, double dy, double dz, double ax, double ay, double az);
	static void TransRot(double &x, double &y, double &z, double x0, double y0, double z0, const Affine3d &quat);
	static void TransRot(double &x, double &y, double &z, double x0, double y0, double z0,
						 double dx, double dy, double dz, double ax, double ay, double az);
	
private:
	const Surface *parent = nullptr;

public:
	VectorXd surfaces;	// (panels)
	MatrixXd centroids;	// (panels, 3)
	MatrixXd normals;	// (panels, 3)
};

void LoadStl(String fileName, Surface &surf, bool &isText, String &header);
void SaveStlTxt(String fileName, const Surface &surf, double factor);
void SaveStlBin(String fileName, const Surface &surf, double factor);

void LoadTDynMsh(String fileName, Surface &surf);

void LoadMesh(String fileName, Surface &surf, double &mass, Point3D &cg);
void SaveMesh(String fileName, const Surface &surf, double mass, const Point3D &cg);
	
}

#endif
