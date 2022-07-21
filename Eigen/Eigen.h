// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2022, the Anboto author and contributors
#ifndef _Eigen_Eigen_h
#define _Eigen_Eigen_h

#define EIGEN_MATRIX_PLUGIN 	<Eigen/ToStringPlugin.h>
#define EIGEN_DENSEBASE_PLUGIN 	<Eigen/ToStringPlugin.h>
#define EIGEN_TENSOR_PLUGIN		<Eigen/ToStringPlugin.h>

#define EIGEN_MPL2_ONLY

#ifndef _DEBUG
#define EIGEN_NO_DEBUG
#else
#define EIGEN_INITIALIZE_MATRICES_BY_NAN 
#endif

#define eigen_assert(x) ASSERT(x)

#undef Success  
#include <plugin/eigen/Eigen/Dense>
#include <plugin/eigen/unsupported/Eigen/NonLinearOptimization>
#undef Complex
#include <plugin/eigen/unsupported/Eigen/FFT>
#include <plugin/eigen/unsupported/Eigen/CXX11/Tensor>


namespace Upp {

template <class T>
using UVector = Upp::Vector<T>;

template <class T>
using UArray = Upp::Array<T>;

template <class T>
using UIndex = Upp::Index<T>;

template<typename _Scalar, ptrdiff_t nx = Eigen::Dynamic, ptrdiff_t ny = Eigen::Dynamic>
struct NonLinearOptimizationFunctor {
	typedef _Scalar Scalar;
	enum {
		InputsAtCompileTime = nx,
		ValuesAtCompileTime = ny
	};
	typedef Eigen::Matrix<double, InputsAtCompileTime, 1> InputType;
	typedef Eigen::Matrix<double, ValuesAtCompileTime, 1> ValueType;
	typedef Eigen::Matrix<double, ValuesAtCompileTime, InputsAtCompileTime> JacobianType;
	
	Eigen::Index unknowns, datasetLen;
	
	NonLinearOptimizationFunctor() : unknowns(InputsAtCompileTime), datasetLen(ValuesAtCompileTime) {}
	NonLinearOptimizationFunctor(int unknowns, int datasetLen) : unknowns(unknowns), datasetLen(datasetLen) {}
	
	ptrdiff_t inputs() const {return ptrdiff_t(unknowns);}
	ptrdiff_t values() const {return ptrdiff_t(datasetLen);}
	virtual void operator() (const InputType& , ValueType* , JacobianType*  = 0) const {};
};

struct Basic_functor : NonLinearOptimizationFunctor<double> {
	Basic_functor(Function <int(const Eigen::VectorXd &b, Eigen::VectorXd &err)> _function) : function(_function) {}
	int operator()(const Eigen::VectorXd &b, Eigen::VectorXd &fvec) const {return function(b, fvec);}
	Function <int(const Eigen::VectorXd &b, Eigen::VectorXd &err)> function;
};

bool NonLinearOptimization(Eigen::VectorXd &y, Eigen::Index numData, 
			Function <int(const Eigen::VectorXd &y, Eigen::VectorXd &residual)>residual,
			double xtol = Null, double ftol = Null, int maxfev = Null);
bool SolveNonLinearEquations(Eigen::VectorXd &y, Function <int(const Eigen::VectorXd &b, Eigen::VectorXd &residual)> Residual,
			double xtol = Null, int maxfev = Null, double factor = Null);
double SolveNonLinearEquation(double y, Function <double(double b)> Residual, double xtol = Null, int maxfev = Null, double factor = Null);
	
template <class T>
void Xmlize(XmlIO &xml, Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> &mat) {
	Size_<int64> sz(mat.cols(), mat.rows());
	xml ("size", sz);
	if(xml.IsStoring()) {
		for(int r = 0; r < mat.rows(); r++)
			for(int c = 0; c < mat.cols(); c++) {
				XmlIO io = xml.Add("item");
				T data = mat(r, c);
				Xmlize(io, data);
			}
	} else {
		mat.resize(ptrdiff_t(sz.cy), ptrdiff_t(sz.cx));
		int r = 0, c = 0;
		for(int i = 0; i < xml->GetCount(); i++) 
			if(xml->Node(i).IsTag("item")) {
				XmlIO io = xml.At(i);
				T data;
				Xmlize(io, data);
				mat(r, c) = data;
				++c;
				if (c == sz.cx) {
					c = 0;
					r++;
				}
			}
	}
}

template <class T>
void Xmlize(XmlIO &xml, Eigen::Matrix<T, Eigen::Dynamic, 1> &vec) {
	int64 sz = vec.size();
	xml ("size", sz);
	if(xml.IsStoring()) {
		for(int r = 0; r < sz; r++) {
			XmlIO io = xml.Add("item");
			T data = vec(r);
			Xmlize(io, data);
		}
	} else {
		vec.resize(ptrdiff_t(sz));
		int r = 0;
		for(int i = 0; i < xml->GetCount(); i++)
			if(xml->Node(i).IsTag("item")) {
				XmlIO io = xml.At(i);
				T data;
				Xmlize(io, data);
				vec(r++) = data;
			}
	}
}

template <class T>
void Jsonize(JsonIO &io, Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> &mat) {
	Array<T> vector;
	Size_<int64> sz(mat.cols(), mat.rows());
	io("size", sz);
	if(io.IsStoring()) {
		vector.SetCount(int(sz.cx)*int(sz.cy));
		int i = 0;
		for(int r = 0; r < mat.rows(); r++)
			for(int c = 0; c < mat.cols(); c++) 
				vector[i++] = mat(r, c);
		io("data", vector);
	} else {
		io("data", vector);
		mat.resize(ptrdiff_t(sz.cy), ptrdiff_t(sz.cx));
		int r = 0, c = 0;
		for (int i = 0; i < vector.GetCount(); ++i) {
			mat(r, c) = vector[i];
			++c;
			if (c == sz.cx) {
				c = 0;
				r++;
			}
			if (r == sz.cy)
				break;
		}
	}
}

template <class T>
void Jsonize(JsonIO &io, Eigen::Matrix<T, Eigen::Dynamic, 1> &vec) {
	Array<T> vector;
	int64 sz = vec.size();
	io("size", sz);
	if(io.IsStoring()) {
		vector.SetCount(int(sz));
		for (int i = 0; i < sz; ++i)
			vector[i] = vec(i);
		io("data", vector);
	} else {
		io("data", vector);
		vec.resize(ptrdiff_t(sz));
		for (int i = 0; i < vec.size(); ++i)
			vec(i) = vector[i];
	}
}

template <class T>
void Serialize(Stream& stream, Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> &mat) {
	Size_<int64> sz(mat.cols(), mat.rows());
	stream % sz;
	if(stream.IsStoring()) {
		for(int r = 0; r < mat.rows(); r++)
			for(int c = 0; c < mat.cols(); c++) {
				T data = mat(r, c);
				stream % data;	
			}
	} else {
		mat.resize(ptrdiff_t(sz.cy), ptrdiff_t(sz.cx));
		int r = 0, c = 0;
		for(int i = 0; i < sz.cy*sz.cx; i++) {
			T data;
			stream % data;
			mat(r, c) = data;
			++c;
			if (c == sz.cx) {
				c = 0;
				r++;
			}
			if (r == sz.cy)
				break;
		}	
	}
}

template <class T>
void Serialize(Stream& stream, Eigen::Matrix<T, Eigen::Dynamic, 1> &vec) {
	int64 sz = vec.size();
	stream % sz;
	if(stream.IsStoring()) {
		for (int i = 0; i < sz; ++i) {
			T data = vec(i);
			stream % data;
		}
	} else {
		vec.resize(ptrdiff_t(sz));
		for (int i = 0; i < sz; ++i) {
			T data;
			stream % data;
			vec(i) = data;
		}
	}
}

// These functions serve both for Eigen, std and U++ Vectors

template <typename T>
void Resize(Vector<T> &v, size_t len) {v.SetCount(int(len));}
template <typename T>
void Resize(Vector<T> &v, size_t len, const T& init) {
	v.SetCount(int(len));
	std::fill(v.begin(), v.end(), init);
}
template <typename T>
void ResizeConservative(Vector<T> &v, size_t len) {v.SetCount(int(len));}
template <typename T>
void ResizeConservative(Vector<T> &v, size_t len, const T& init) {v.SetCount(int(len), init);}
template <typename T>
void Clear(Vector<T> &v) {v.Clear();}

template <typename T>
void Resize(Eigen::Matrix<T, Eigen::Dynamic, 1> &v, size_t len) {v.resize(len);}
template <typename T>
void Resize(Eigen::Matrix<T, Eigen::Dynamic, 1> &v, size_t len, const T& init) {v.setConstant(len, 1, init);}
template <typename T>
void ResizeConservative(Eigen::Matrix<T, Eigen::Dynamic, 1> &v, size_t len) {v.conservativeResize(len);}
template <typename T>
void ResizeConservative(Eigen::Matrix<T, Eigen::Dynamic, 1> &v, size_t len, const T& init) {
	size_t len0 = v.size();
	v.conservativeResize(len);
	if (len > len0)
		std::fill(&v[len0], v.data() + len, init);	
}
template <typename T>
void Clear(Eigen::Matrix<T, Eigen::Dynamic, 1> &v) {v.resize(0);}
template <typename T>
void Clear(Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> &m) {m.resize(0, 0);}

template <typename T>
void PrePad(Eigen::Matrix<T, Eigen::Dynamic, 1> &v, size_t len, const T& init) {
	size_t len0 = v.size();
	v.conservativeResize(len);
	if (len > len0) {
		size_t delta = len - len0;
		std::copy(&v[len0 - delta], v.data() + len0, &v[len0]);	
		std::copy(v.data(), v.data() + len0 - delta, &v[delta]);	
		std::fill(v.data(), v.data() + delta, init);	
	}
}


template <typename T>
void Resize(std::vector<T> &v, size_t len) {v.resize(len);}
template <typename T>
void Resize(std::vector<T> &v, size_t len, const T& init) {
	v.resize(len);
	std::fill(v.begin(), v.end(), init);
}
template <typename T>
void ResizeConservative(std::vector<T> &v, size_t len) {v.resize(len);}
template <typename T>
void ResizeConservative(std::vector<T> &v, size_t len, const T& init) {v.resize(len, init);}
template <typename T>
void Clear(std::vector<T> &v) {v.clear();}

#define PostPad ResizeConservative


template <typename T>
auto Begin(const Vector<T> &v)			{return v.Begin();}
template <typename T>
auto Begin(Vector<T> &v)				{return v.Begin();}
template <typename T>
auto End(const Vector<T> &v)			{return v.End();}
template <typename T>
auto End(Vector<T> &v)					{return v.End();}


template <typename T>
auto Begin(const Array<T> &v)			{return v.Begin();}
template <typename T>
auto Begin(Array<T> &v)					{return v.Begin();}
template <typename T>
auto End(const Array<T> &v)				{return v.End();}
template <typename T>
auto End(Array<T> &v)					{return v.End();}

template <typename T>
auto Begin(const std::vector<T> &v)		{return v.begin();}
template <typename T>
auto Begin(std::vector<T> &v)			{return v.begin();}
template <typename T>
auto End(const std::vector<T> &v)		{return v.end();}

template <typename T>
auto Begin(const Eigen::Matrix<T, Eigen::Dynamic, 1> &v){return v.data();}
template <typename T>
auto Begin(Eigen::Matrix<T, Eigen::Dynamic, 1> &v)		{return v.data();}
template <typename T>
auto End(const Eigen::Matrix<T, Eigen::Dynamic, 1> &v)	{return v.data() + v.size();}

template <class Range>
auto &Last(Range &data) {
	return data[data.size()-1];	
}

template <typename T>
void Reverse(Vector<T> &v) {
	T *first = v.begin();
	T *last = v.end();
	while ((first != last) && (first != --last)) 
		Swap(*first++, *last);
}

template <typename T>
void Reverse(std::vector<T> &v) {std::reverse(v.begin(), v.end());}

template <typename T>
void Reverse(Eigen::Matrix<T, Eigen::Dynamic, 1> &v) {v.reverseInPlace();}


template <class Range1, class Range2>
void Copy(const Range1& in, Range2 &out) {
	Resize(out, in.size());
	std::copy(Begin(in), End(in), Begin(out));
}

}

#endif
