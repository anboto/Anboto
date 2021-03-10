#ifndef _Mathlib_Histogram_h_
#define _Mathlib_Histogram_h_

#include "MultiDimMatrixIndex.h"

namespace Upp {

struct HistogramDataAxis : Moveable<HistogramDataAxis> {
	HistogramDataAxis(DataSource &data, double min, double max, int numVals) : data(data), min(min), max(max), numVals(numVals) {}
	DataSource &data;
	double min, max;
	int numVals;
};

class Histogram : public DataSource, public Moveable<Histogram> {
public:
	Histogram();
	Histogram(const Histogram &hist);
	Histogram(const Nuller&) 	{SetNull();}
	
	void SetNull() 				{Clear();}
	bool IsNullInstance() const	{return ranges.IsEmpty();}
	
	virtual Histogram &operator=(const Histogram &hist);
	void Clear();
	
	Histogram &Create(DataSource &data, double min, double max, int numVals, bool isY = true);
	Histogram &Create(Upp::Array<HistogramDataAxis> &dataAxis, bool isY = true);
	Histogram &Create2D(const Vector<Vector<double> > &_ranges, const Vector<double> &data, double total);

	Histogram &Normalize(double val = 1);
	Histogram &Accumulative(bool _accum = true)	{accumulative = _accum;	return *this;}
	
	double Compare(const Histogram &hist);
	
	inline const double &GetRange(int dim, int i)	{return ranges[dim][i];};
	inline int64 GetCount(int dim) const	{return ranges[dim].GetCount();}
	inline double GetValue(int index)		{return y(index);}
	inline double GetValue(int c, int r)	{return y(valuesIdx(c, r));}
	inline bool IsValid(int c, int r)		{return valuesIdx.IsValid(c, r);}
	
	inline double y(int64 id) {
		ASSERT(values.size());
		if (!accumulative)	
			return values(ptrdiff_t(id));
		else {
			double ret = 0;
			for (ptrdiff_t i = 0; i <= ptrdiff_t(id); ++i)
				ret += values(i);
			return ret;
		}
	}
	inline double x(int64 id) {
		ASSERT(values.size());	
		return ranges[0][int(id)];
	}
	inline virtual int64 GetCount() const {return !values.size() ? 0 : values.size();};
	inline virtual int64 size() const 	  {return GetCount();};
	
	void Xmlize(XmlIO xml) {
		xml
			("totalVals", totalVals)	
			("valuesIdx", valuesIdx)	
			("ranges", ranges)	
		;	
		Upp::Xmlize(xml, values);
	}
	void Jsonize(JsonIO& json) {
		json
			("totalVals", totalVals)	
			("valuesIdx", valuesIdx)	
			("ranges", ranges)	
		;
		Upp::Jsonize(json, values);	
	}
protected:
	double totalVals;
	bool accumulative = false;
	
	MultiDimMatrixIndex valuesIdx;
	Eigen::VectorXd values; 
	Vector<Vector<double> > ranges;
};
	
}

#endif
