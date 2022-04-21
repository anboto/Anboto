// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2022, the Anboto author and contributors
#ifndef _STEM4U_utility_h_
#define _STEM4U_utility_h_

namespace Upp {

template <class Range>
void CleanOutliers(const Range &x, const Range &y, const Range &filtery, Range &rretx, Range &rrety, 
				   const typename Range::value_type& ratio, const typename Range::value_type& zero = 0) {
	ASSERT(x.size() == y.size() && x.size() == filtery.size());
 
 	Range retx(x.size()), rety(x.size());
 
 	int id = 0;
	for (int i = 0; i < x.size(); ++i) {
		if (EqualRatio(y[i], filtery[i], ratio, zero)) {
			retx[id] = x[i];
			rety[id] = y[i];
			id++;
		}
	}
	ResizeConservative(retx, id); 
	ResizeConservative(rety, id);
	
	rretx = pick(retx);
	rrety = pick(rety);
}

template <class Range>
void CleanCondition(const Range &x, const Range &y, Range &rretx, Range &rrety, 
		Function <bool(int id)> Cond) {
	ASSERT(x.size() == y.size());
 
 	Range retx(x.size()), rety(x.size());
 	
 	int id = 0;
	for (int i = 0; i < x.size(); ++i) {
		if (Cond(i)) {
			retx[id] = x[i];
			rety[id] = y[i];
			id++;
		}
	}
	ResizeConservative(retx, id); 
	ResizeConservative(rety, id);

	rretx = pick(retx);
	rrety = pick(rety);
}

template <class Range>	// Gets the most probable sample rate, or the average if the most probable probability is lower than rate
typename Range::value_type GetSampleRate(const Range &x, int numDecimals, double rate) {
	using Scalar = typename Range::value_type;
	
	int n = int(x.size());
	if (n < 2)
		return Null;
	
	Vector<Scalar> delta;
	Vector<int> num;
	
	for (int i = 1; i < n; ++i) {
		Scalar d = x[i]-x[i-1];
		int id = FindRoundDecimals(delta, d, numDecimals);	
		if (id >= 0)
			num[id]++;
		else {
			delta << d;
			num << 1;		
		}
	}
	int nummx = num[0], idmx = 0;
	for (int i = 1; i < delta.size(); ++i) {
		if (num[i] > nummx) {
			nummx = num[i];
			idmx = i;
		}
	}
	if (num[idmx]/double(n-1) > rate)
		return delta[idmx];
	else {
		Scalar avg = 0;
		for (int i = 1; i < n; ++i) 
			avg += (x[i]-x[i-1]);
		return avg/(n-1);
	}
}

template <typename T>
void LinSpaced(Eigen::Matrix<T, Eigen::Dynamic, 1> &v, Eigen::Index n, T min, T max) {
	v.LinSpaced(n, min, max);
}

template <class Range>
void LinSpaced(Range &v, int n, typename Range::value_type min, typename Range::value_type max) {
	ASSERT(n > 0);
	Resize(v, n);
	if (n == 1)
		v[0] = min;
	else {
		typename Range::value_type d = (max - min)/(n - 1);
		for (int i = 0; i < n; ++i)
			v[i] = min + d*i;
	}
}


template <class Range>
void CircShift(const Range& in, int len, Range &out) {
	std::rotate_copy(in, in + len, End(in), out);
}

template <typename T>
void CircShift(const Eigen::Matrix<T, Eigen::Dynamic, 1> &in, int len, Eigen::Matrix<T, Eigen::Dynamic, 1> &out) {
	Resize(out, in.size());
	out.segment(len, in.size() - len) = in.segment(0, in.size() - len); 
	out.segment(0, len) = in.segment(in.size() - len, len);
}

template <class Range>
void NextPow2(const Range& in, Range &out) {
	Resize(out, in.size());
	for (int i = 0; i < in.size(); ++i)
		out[i] = ceil(log(abs(in[i])))/log(2);
}

template <typename T>
T NextPow2(const T& in) {
	ASSERT(in > 0);
	return T(ceil(log(in))/log(2));
}

template <class Range>
inline typename Range::value_type Avg(const Range &d) {
	return std::accumulate(d.begin(), d.end(), 0.)/d.size();
}

template <typename T>
inline T Avg(const Eigen::Matrix<T, Eigen::Dynamic, 1> &d) {
	return d.mean();
}

template <class Range>
typename Range::value_type R2(const Range &serie, const Range &serie0, typename Range::value_type mean = Null) {
	using Scalar = typename Range::value_type;
	
	if (IsNull(mean))
		mean = Avg(serie);
	Scalar sse = 0, sst = 0;
	auto sz = min(serie.size(), serie0.size());
	for (auto i = 0; i < sz; ++i) {
		auto y = serie[i];
		auto err = y - serie0(i);
		sse += err*err;
		auto d = y - mean;
		sst += d*d;
	}
	if (sst < 1E-50 || sse > sst)
		return 0;
	return 1 - sse/sst;
}

template <class Range>
typename Range::value_type SawTeethRatio(const Range &d) {
	using Scalar = typename Range::value_type;
	
	if (d.size() < 3)
		return 0;
	
	Scalar mean = Avg(d);
	int numcrosses = 0;
	for (int i = 1; i < d.size(); ++i) {
		if ((d[i] > mean && d[i-1] <= mean) || (d[i] < mean && d[i-1] >= mean)) 
			numcrosses++;
	}
	return Scalar(numcrosses)/(d.size()-1);
}


// y = ax + b
template <class Range>
void LinearFitting(const Range &x, const Range &y, typename Range::value_type &a, typename Range::value_type &b, const int clen = Null) {
	int len = IsNull(clen) ? x.size() : clen;
	ASSERT(len <= x.size() && len <= y.size());
	using Scalar = typename Range::value_type;

	a = 0;	
	Scalar sumx = 0, sumy = 0;	
	for (int i = 0; i < len; i++) { 
		sumx += x[i];
		sumy += y[i];
	}
	Scalar sum = len;
	Scalar sxoss = sumx/sum;
	Scalar sumtotal = 0;
	for (int i = 0; i < len; i++) {
		Scalar ti = x[i] - sxoss;
		sumtotal += ti*ti;
		a += ti*y[i];
	}
	a /= sumtotal; 
	b = (sumy - sumx*a)/sum;
}

#define LinearRegression	LinearFitting


}

#endif
