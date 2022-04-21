// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2022, the Anboto author and contributors
#ifndef _ScatterDraw_Equation_h_
#define _ScatterDraw_Equation_h_

#include <Eigen/Eigen.h>

namespace Upp {

#define FormatCoeff(id, numDigits)		(IsNull(numDigits) ? (String("C") + FormatInt(id)) : FormatF(coeff[id], numDigits))

class ExplicitEquation : public DataSource {
public:
	ExplicitEquation() {isExplicit = true;}

	virtual void SetDegree(int num) = 0;
	enum FitError {
		NoError = 1, 
		InadequateDataSource = -1,
		SmallDataSource = -2,
		ImproperInputParameters = -3,
		TooManyFunctionEvaluation = -4
	};
	virtual FitError Fit(DataSource &series, double &r2);
	FitError Fit(DataSource &series)		{double dummy; return Fit(series, dummy);}
	virtual void GuessCoeff(DataSource &series)	= 0;
	void SetWeight(Eigen::VectorXd &w)		{weight = clone(w);}
	
	virtual double f(double ) 				= 0;
	virtual double f(double , double ) 		{NEVER(); return Null;}
	virtual double f(Vector <double> ) 		{NEVER(); return Null;}
	virtual double x(int64 ) 				{NEVER(); return Null;}
	virtual double y(int64 ) 				{NEVER(); return Null;}	
	double x_from_y(double y, double x0);
	virtual String GetName() = 0;
	virtual String GetFullName()			{return GetName();}
	virtual String GetEquation(int numDigits = 3) = 0;
	virtual inline int64 GetCount() const	{return coeff.GetCount() > 0 ? 0 : int64(Null);}
	
	void SetMaxFitFunctionEvaluations(int n){maxFitFunctionEvaluations = n;}
	int GetMaxFitFunctionEvaluations()		{return maxFitFunctionEvaluations;}
	
	friend struct Equation_functor;
	
	const Vector<double> &GetCoeff()			{return coeff;}
	double GetCoeff(int i)						{return coeff[i];}

	template<class T>	
	static void Register(const String& name) {
		classMap().FindAdd(name, __Create<T>);
	}
	static void Unregister(const String& name) {
		int i = NameIndex(name);
		ASSERT(i >= 0);
		classMap().Remove(i);
	}
	static int            	 NameIndex(const String& name) {return classMap().Find(name);}
	static int            	 GetEquationCount()            {return classMap().GetCount();}
	static ExplicitEquation* Create(int i)                 {return classMap()[i]();}
	
	int GetNumCoeff(int ) 	{return coeff.GetCount();}
	
	ExplicitEquation &operator=(ExplicitEquation &other) {
		if (this != &other) {
			numcoeff = other.numcoeff;
			coeff = clone(other.coeff);
		}
		return *this;
	}
	double R2Y(DataSource &series, double mean = Null);
	
protected:
	Vector<double> coeff;
	int numcoeff = -1;
	Eigen::VectorXd weight;
	
	static int maxFitFunctionEvaluations;
	
	void SetNumCoeff(int num);
	void SetCoeff(const Vector<double>& c)          {coeff = clone(c); numcoeff = coeff.size();}
	void SetCoeff(double c0, double c1, double c2, double c3)	{coeff.Clear();	coeff << c0 << c1 << c2 << c3; numcoeff = 4;}
	void SetCoeff(double c0, double c1, double c2)	{coeff.Clear();	coeff << c0 << c1 << c2; numcoeff = 3;}
	void SetCoeff(double c0, double c1) 			{coeff.Clear();	coeff << c0 << c1; numcoeff = 2;}
	void SetCoeff(double c0) 						{coeff.Clear();	coeff << c0; numcoeff = 1;}
	void SetCoeffVal(int id, double c) 				{coeff[id] = c;}
	
	typedef ExplicitEquation* (*CreateFunc)();
	template<class T>	
	static ExplicitEquation*                      __Create() {return new T;}
	static VectorMap<String, CreateFunc>& classMap() {static VectorMap<String, CreateFunc> cMap; return cMap;}
};

class AvgEquation : public ExplicitEquation {
public:
	AvgEquation() 						{SetCoeff(0);}
	AvgEquation(double c0)				{SetCoeff(c0);}
	double f(double ) 					{return coeff[0];}
	virtual String GetName() 			{return t_("Average");}
	virtual String GetEquation(int _numDigits = 3) {	
		String ret = Format("%s", FormatCoeff(0, _numDigits));
		return ret;
	}
	void SetDegree(int )				{NEVER();}
	virtual void GuessCoeff(DataSource &series)	{coeff[0] = series.AvgY();}
};

class LinearEquation : public ExplicitEquation {
public:
	LinearEquation() 					{SetCoeff(0, 0);}
	LinearEquation(double c0, double c1){SetCoeff(c0, c1);}
	double f(double x) 					{
		return coeff[0] + x*coeff[1];
	}
	virtual String GetName() 			{return t_("Linear");}
	virtual String GetEquation(int _numDigits = 3) {	
		String ret = Format("%s + %s*x", FormatCoeff(0, _numDigits), FormatCoeff(1, _numDigits));
		ret.Replace("+ -", "- ");
		return ret;
	}
	void SetDegree(int )				{NEVER();}
	virtual void GuessCoeff(DataSource &series)	{coeff[0] = series.AvgY();}
};

class PolynomialEquation : public ExplicitEquation {
public:
	PolynomialEquation() 				       	{}
	PolynomialEquation(const Vector<double>& c) {SetCoeff(c);}
	double f(double x);
	virtual String GetName() 			       	{return t_("Polynomial");}
	virtual String GetFullName() 		       	{return t_("Polynomial") + String(" n = ") + FormatInt(numcoeff);}
	virtual String GetEquation(int numDigits = 3);
	void SetDegree(int num)				       	{numcoeff = num+1;	SetNumCoeff(num + 1);}
	virtual void GuessCoeff(DataSource &series) {
		coeff[0] = series.AvgY();
		int realDegree = numcoeff;
		for (numcoeff = 2; numcoeff < realDegree; numcoeff++) 
			Fit(series);
	}
};	

class PolynomialEquation2 : public PolynomialEquation {
public:
	PolynomialEquation2() {SetDegree(2);}
};

class PolynomialEquation3 : public PolynomialEquation {
public:
	PolynomialEquation3() {SetDegree(3);}
};

class PolynomialEquation4 : public PolynomialEquation {
public:
	PolynomialEquation4() {SetDegree(4);}
};

class PolynomialEquation5 : public PolynomialEquation {
public:
	PolynomialEquation5() {SetDegree(5);}
};
/*
class PolynomialQuotient : public ExplicitEquation {
public:
	PolynomialQuotient() 				  	{}
	double f(double x);
	virtual String GetName() 			    {return t_("Polynomial Quotient");}
	virtual String GetFullName() 		    {return t_("Polynomial Quotient") + Format(" n/d = %d/%d", nnum, nden);}
	virtual String GetEquation(int numDigits = 3);
	void SetDegree(int num)					{NEVER();}
	void SetDegreeNumDen(int num, int den)	{
		nnum = num + 1;
		nden = den;
		numcoeff = num;	
		SetNumCoeff(nnum + nden + 1);
	}
	virtual void GuessCoeff(DataSource &) {}
private:
	int nnum, nden;
};

class PolynomialQuotient2_2 : public PolynomialQuotient {
public:
	PolynomialQuotient2_2() {SetDegreeNumDen(2, 2);}
};*/

class SinEquation : public ExplicitEquation {
public:
	SinEquation() 					{coeff.Clear();	coeff << 0. << 0.1 << 0.1 << 0.1; numcoeff = 4;}
	SinEquation(double offset, double A, double w, double phi) 	{Init(offset, A, w, phi);}
	void Init(double offset, double A, double w, double phi) 	{coeff.Clear();	coeff << offset << A << w << phi; numcoeff = 4;}
	double f(double x)				{return coeff[0] + coeff[1]*sin(coeff[2]*x + coeff[3]);}
	virtual String GetName() 		{return t_("Sine");}
	virtual String GetEquation(int _numDigits = 3) {
		String ret = Format("%s + %s*sin(%s*t + %s)", FormatCoeff(0, _numDigits), FormatCoeff(1, _numDigits)
													, FormatCoeff(2, _numDigits), FormatCoeff(3, _numDigits));
		ret.Replace("+ -", "- ");
		return ret;
	}													
	void SetDegree(int )				{NEVER();}
	virtual void GuessCoeff(DataSource &series)	{
		coeff[0] = series.AvgY();	
		coeff[1] = series.SinEstim_Amplitude(coeff[0]);
		double frequency, phase;
		if (series.SinEstim_FreqPhase(frequency, phase, coeff[0])) {
			coeff[2] = frequency;
			coeff[3] = phase;
		}
	}
};

class DampedSinEquation : public ExplicitEquation {
public:
	DampedSinEquation() 					{coeff.Clear();	coeff << 0. << 0.1 << 0.1 << 0.1 << 0.; numcoeff = 5;}
	DampedSinEquation(double offset, double A, double lambda, double w, double phi) {Init(offset, A, lambda, w, phi);}
	void Init(double offset, double A, double lambda, double w, double phi) 	{coeff.Clear();	coeff << offset << A << lambda << w << phi; numcoeff = 5;}
	double f(double x)				{return coeff[0] + coeff[1]*exp(-coeff[2]*x)*cos(coeff[3]*x + coeff[4]);}
	virtual String GetName() 		{return t_("DampedSinusoidal");}
	virtual String GetEquation(int _numDigits = 3) {
		double zeta  = coeff[2]/coeff[3];
		String szeta = IsNull(_numDigits) ? String("ζ") : FormatF(zeta, _numDigits);

		String ret = Format("%s + %s*e^(-%s*t)*cos(%s*t + %s) (ζ: %s)", FormatCoeff(0, _numDigits), 
			FormatCoeff(1, _numDigits), FormatCoeff(2, _numDigits), FormatCoeff(3, _numDigits), FormatCoeff(4, _numDigits),
			szeta);
		ret.Replace("+ -", "- ");
		return ret;
	}													
	void SetDegree(int )						{NEVER();}
	virtual void GuessCoeff(DataSource &series)	{
		coeff[0] = series.AvgY();	
		coeff[1] = series.SinEstim_Amplitude(coeff[0]);
		double frequency, phase;
		if (series.SinEstim_FreqPhase(frequency, phase, coeff[0])) {
			coeff[3] = frequency;
			coeff[4] = phase;
		}
	}
};

class Sin_DampedSinEquation : public ExplicitEquation {
public:
	Sin_DampedSinEquation() 			{coeff.Clear();	coeff << 0. << 0.1 << 0.1 << 0.1 << 0.1 << 0.1 << 0.1 << 0.1; numcoeff = 8;}
	Sin_DampedSinEquation(double offset, double A1, double w1, double phi1, double A2, 
			double lambda, double w2, double phi2) {Init(offset, A1, w1, phi1, A2, lambda, w2, phi2);}
	void Init(double offset, double A1, double w1, double phi1, double A2, double lambda, 
		double w2, double phi2) {coeff.Clear();	
								coeff << offset << A1 << w1 << phi1 << A2 << lambda << w2 << phi2;
								numcoeff = 8;}
	double f(double x)				{return coeff[0] + coeff[1]*cos(coeff[2]*x + coeff[3]) + coeff[4]*exp(-coeff[5]*x)*cos(coeff[6]*x + coeff[7]);}
	virtual String GetName() 		{return t_("Sin_DampedSinusoidal");}
	virtual String GetEquation(int _numDigits = 3) {
		String ret = Format("%s + %s*cos(%s*t + %s) + %s*e^(-%s*t)*cos(%s*t + %s)", 
			FormatCoeff(0, _numDigits), FormatCoeff(1, _numDigits), FormatCoeff(2, _numDigits), 
			FormatCoeff(3, _numDigits), FormatCoeff(4, _numDigits), FormatCoeff(5, _numDigits),
			FormatCoeff(6, _numDigits), FormatCoeff(7, _numDigits));
		ret.Replace("+ -", "- ");
		return ret;
	}													
	void SetDegree(int )				{NEVER();}
	virtual void GuessCoeff(DataSource &series)	{
		coeff[0] = series.AvgY();	
		coeff[1] = series.SinEstim_Amplitude(coeff[0]);
		double frequency, phase;
		if (series.SinEstim_FreqPhase(frequency, phase, coeff[0])) {
			coeff[2] = frequency;
			coeff[3] = phase;
		}
	}
};
	
class FourierEquation : public ExplicitEquation {
public:
	FourierEquation() 					        {}
	FourierEquation(const Vector<double>& c) 	{SetCoeff(c);}
	double f(double x);
	virtual String GetName() 			        {return t_("Fourier");}
	virtual String GetFullName() 		        {return t_("Fourier") + String(" n = ") + FormatInt(numcoeff);}
	virtual String GetEquation(int numDigits = 3);
	virtual void GuessCoeff(DataSource &series) {coeff[0] = series.AvgY();}
	void SetDegree(int num)				        {numcoeff = num;	SetNumCoeff(2*num + 2);}
};

class FourierEquation1 : public FourierEquation {
public:
	FourierEquation1() {SetDegree(1);}
};

class FourierEquation2 : public FourierEquation {
public:
	FourierEquation2() {SetDegree(2);}
};

class FourierEquation3 : public FourierEquation {
public:
	FourierEquation3() {SetDegree(3);}
};

class FourierEquation4 : public FourierEquation {
public:
	FourierEquation4() {SetDegree(4);}
};

class ExponentialEquation : public ExplicitEquation {
public:
	ExponentialEquation() 						{SetCoeff(1, 0);}
	ExponentialEquation(double c0, double c1)	{SetCoeff(c0, c1);}
	double f(double x) 							{return coeff[0]*exp(-x) + coeff[1];}
	virtual String GetName() 					{return t_("Exponential");}
	virtual String GetEquation(int nDig = 3) {
		String ret = Format("%s*e^-x + %s", FormatCoeff(0, nDig), FormatCoeff(1, nDig));
		ret.Replace("+ -", "- ");
		return ret;
	}	
	virtual void GuessCoeff(DataSource &) {}
	void SetDegree(int )				{NEVER();}
};

class ExponentialEquation2 : public ExplicitEquation {
public:
	ExponentialEquation2() 						{SetCoeff(1, 0, 0.01, 0);}
	ExponentialEquation2(double c0, double c1, double c2, double c3)	{SetCoeff(c0, c1, c2, c3);}
	double f(double x) 							{return coeff[0]*exp(-(x-coeff[3])*coeff[2]) + coeff[1];}
	virtual String GetName() 					{return t_("Exponential2");}
	virtual String GetEquation(int nDig = 3) {
		String ret = Format("%s*e^-%s*(x-%s) + %s", FormatCoeff(0, nDig), FormatCoeff(3, nDig), FormatCoeff(2, nDig), FormatCoeff(1, nDig));
		ret.Replace("+ -", "- ");
		return ret;
	}	
	virtual void GuessCoeff(DataSource &series) {
		coeff[3] = series.MinX();
		coeff[1] = series.MaxY();
		if (series.size() > 2) {
			Vector<double> y = series.Y();
			Vector<Pointf> der = series.DerivativeY(1, 2);
			double d = 0;
			for (int i = 0; i < der.size(); ++i)
				d += der[i].y/y[i];
			d /= der.size();
			coeff[2] = -d;
		}	
	}
	void SetDegree(int )				{NEVER();}
};


class RealExponentEquation : public ExplicitEquation {
public:
	RealExponentEquation() 						{SetCoeff(1, 1);}
	RealExponentEquation(double a, double b)	{SetCoeff(a, b);}
	double f(double x) 							{
		if (x < 0)
			return Null;
		return coeff[0]*::pow(x, coeff[1]);
	}
	virtual String GetName() 					{return t_("RealExponent");}
	virtual String GetEquation(int nDig = 3) {
		String ret = Format("%s*x^%s", FormatCoeff(0, nDig), FormatCoeff(1, nDig));
		ret.Replace("+ -", "- ");
		return ret;
	}	
	virtual void GuessCoeff(DataSource &) {}
	void SetDegree(int )				{NEVER();}
};

class WeibullCumulativeEquation : public ExplicitEquation {
public:
	WeibullCumulativeEquation() 						{SetCoeff(1, 1);}
	WeibullCumulativeEquation(double k, double lambda)	{ASSERT(k > 0);	ASSERT(lambda > 0);	SetCoeff(k, lambda);}
	double f(double x) {
		if (x < 0)
			return 0;
		double k =  coeff[0];
		double lambda = coeff[1];
		return 1 - ::exp(double(-::pow(x/lambda, k)));
	}
	virtual String GetName() 					{return t_("Weibull cumulative");}
	virtual String GetEquation(int nDig = 3) {
		String k =  FormatCoeff(0, nDig);
		String lambda = FormatCoeff(1, nDig);
		String ret = Format("1 - e^(-((x/%s)^%s))", lambda, k);
		ret.Replace("+ -", "- ");
		return ret;
	}	
	virtual void GuessCoeff(DataSource &) {}
	void SetDegree(int )				{NEVER();}
};

class WeibullEquation : public ExplicitEquation {
public:
	WeibullEquation() 							{SetCoeff(1, 1, 1);}
	WeibullEquation(double k, double lambda, double factor)	{ASSERT(k > 0);	ASSERT(lambda > 0);	SetCoeff(k, lambda, factor);}
	double f(double x) {
		double k =  coeff[0];
		double lambda = coeff[1];
		double factor = coeff[2];
		if (x/lambda < 0)
			return 0;
		return factor*(k/lambda)*(::pow(x/lambda, k-1))*::exp(double(-::pow(x/lambda, k)));
	}
	virtual String GetName() 					{return t_("Weibull");}
	virtual String GetEquation(int nDig = 3) {
		String k =  FormatCoeff(0, nDig);
		String lambda = FormatCoeff(1, nDig);
		String sfactor = FormatCoeff(2, nDig);
		String ret = Format("%s*(%s/%s)*(x/%s)^(%s-1)*e^(-((x/%s)^%s))", sfactor, k, lambda, lambda, k, lambda, k);
		ret.Replace("+ -", "- ");
		return ret;
	}	
	virtual void GuessCoeff(DataSource &series) {
		Vector<Pointf> cumulative = series.CumulativeY();
		double fac = cumulative.Top().y;
		for (int i = 0; i < cumulative.GetCount(); ++i)
			cumulative[i].y /= fac;
		VectorPointf data(cumulative);
		WeibullCumulativeEquation weibullCumulative;
		ExplicitEquation::FitError error = weibullCumulative.Fit(data);
		if (error == ExplicitEquation::NoError) {
			double k = weibullCumulative.GetCoeff()[0];
			double lambda = weibullCumulative.GetCoeff()[1];
			SetCoeff(k, lambda, 1);
		}
	}
	void SetDegree(int )					{NEVER();}
};

class NormalEquation : public ExplicitEquation {
public:
	NormalEquation() 						{SetCoeff(1, 1, 1);}
	NormalEquation(double c, double mean, double std)	{SetCoeff(c, mean, std);}
	double f(double x) {
		double c    = coeff[0];
		double mean = coeff[1];
		double std  = coeff[2];
		return c*exp(-0.5*sqr((x - mean)/std))/(std*sqrt(2*M_PI));
	}
	virtual String GetName() 					{return t_("Normal");}
	virtual String GetEquation(int nDig = 3) {
		String c = FormatCoeff(0, nDig);
		String mean = FormatCoeff(1, nDig);
		String std = FormatCoeff(2, nDig);
		String ret = Format("(%s/(%s*sqrt(2*PI)))*e^(-(1/2)*((x-%s)/%s)^2)", c, std, mean, std);
		ret.Replace("+ -", "- ");
		return ret;
	}	
	virtual void GuessCoeff(DataSource &series) {
		double mean = series.AvgX();
		double std = series.StdDevX();
		double c = series.MaxY()*std*sqrt(2*M_PI);
		SetCoeff(c, mean, std);
	}
	void SetDegree(int )					{NEVER();}
};

class Rational1Equation : public ExplicitEquation {
public:
	Rational1Equation() 				{SetCoeff(1, 0, 0);}
	Rational1Equation(double c0, double c1, double c2)	{SetCoeff(c0, c1, c2);}
	double f(double x) 					{return coeff[0]/(x + coeff[1]) + coeff[2];}
	virtual String GetName() 			{return t_("Rational_1");}
	virtual String GetEquation(int nDig = 3) {
		String ret = Format("%s/(x + %s) + %s", FormatCoeff(0, nDig), FormatCoeff(1, nDig), FormatCoeff(2, nDig));
		ret.Replace("+ -", "- ");
		return ret;
	}			
	virtual void GuessCoeff(DataSource &) {}
	void SetDegree(int )				{NEVER();}
};

class AsymptoticEquation : public ExplicitEquation {
public:
	AsymptoticEquation() 		{SetCoeff(0, 1, 1);}
	AsymptoticEquation(int deg)	{ASSERT(deg > 2); SetNumCoeff(deg);}
	double f(double x) 			{
		double ret = coeff[0];
		for (int i = 1; i < numcoeff; ++i)
			ret += double(coeff[i]/::pow(x, double(i)));
		return ret;
	}
	virtual String GetName() 	{return t_("Asymptotic");}
	virtual String GetFullName(){return t_("Asymptotic") + String(" n = ") + FormatInt(numcoeff);}
	virtual String GetEquation(int numDigits = 3) {
		String ret = FormatCoeff(0, numDigits);
		ret += Format(" + %s/x", FormatCoeff(1, numDigits));
		for (int i = 2; i < numcoeff; ++i)
			ret += Format(" + %s/x^%d", FormatCoeff(i, numDigits), i);
		ret.Replace("+ -", "- ");
		return ret;
	}
	virtual void GuessCoeff(DataSource &series) {
		int64 idMax, idMin;
		series.MaxX(idMax);
		series.MinX(idMin);
		coeff[0] = series.y(idMax);
		int realDegree = numcoeff;
		for (numcoeff = 2; numcoeff < realDegree; numcoeff++) {
			coeff[numcoeff-1] = sqr(coeff[numcoeff-2]);
			Fit(series);
		}
		coeff[numcoeff-1] = sqr(coeff[numcoeff-2]);
	}
	void SetDegree(int num)	{numcoeff = num+1; SetNumCoeff(num+1);}
};

class AsymptoticEquation1 : public AsymptoticEquation {
public:
	AsymptoticEquation1() {SetDegree(1);}
};

class AsymptoticEquation2 : public AsymptoticEquation {
public:
	AsymptoticEquation2() {SetDegree(2);}
};

class AsymptoticEquation3 : public AsymptoticEquation {
public:
	AsymptoticEquation3() {SetDegree(3);}
};

class AsymptoticEquation4 : public AsymptoticEquation {
public:
	AsymptoticEquation4() {SetDegree(4);}
};

class Spline {
public:
	Spline()													  {}
	Spline(const Vector<double> &x, const Vector<double> &y)	  {Init(x, y);}
	Spline(const Eigen::VectorXd &x, const Eigen::VectorXd &y) 	  {Init(x, y);}
	Spline(const double *x, const double *y, int n)				  {Init(x, y, n);}	
	void Init(const Vector<double> &x, const Vector<double> &y)	  {Init(x.begin(), y.begin(), x.GetCount());}
	void Init(const Eigen::VectorXd &x, const Eigen::VectorXd &y) {Init(x.data(), y.data(), int(x.size()));}
	void Init(const double *x, const double *y, int n);
	double f(double x) const;
	double df(double x) const;
	double d2f(double x) const;
	double Integral(double from = Null, double to = Null) const;
	
private:
	struct Coeff {double a, b, c, d, x;};
    Buffer<Coeff> scoeff;
    int nscoeff = 0;
    double xlast;
    
    int GetPieceIndex(double x) const;
	static double Integral0(const Coeff &c, double x);
};

class SplineEquation : public ExplicitEquation, public Spline {
public:
	SplineEquation() 					{}
	double f(double x)					{return Spline::f(x);}
	virtual String GetName() 			{return t_("Spline");}
	void SetDegree(int )				{NEVER();}
	void GuessCoeff(DataSource &)		{}
	String GetEquation(int)				{return t_("Spline");}
	FitError Fit(DataSource &series, double &r2);
	FitError Fit(DataSource &series)	{double dummy; return Fit(series, dummy);}
};

class Unit : public Moveable<Unit> {
public:
	Unit() 							{SetNull();}
	Unit(const Nuller&)	: Unit()	{}
	Unit(double _m, double _l, double _t) : m(_m), l(_l), t(_t) {}
	
	void Set(const Unit &d) {
		m = d.m;
		l = d.l;
		t = d.t;
	}
	String GetString() {
		if (IsNullInstance())
			return String();
		String ret;
		if (m != 0) {
			ret << "Kg";
			if (m != 1)
				ret << "^" << m;
		}
		if (l != 0) {
			if (!ret.IsEmpty())
				ret << "*";
			ret << "m";
			if (l != 1) 
				ret << "^" << l;
		}
		if (t != 0) {
			if (!ret.IsEmpty())
				ret << "*";
			ret << "sec";
			if (t != 1)
				ret << "^" << t;
		}
		return ret;
	}
	bool IsEqual(const Unit &un) {return m == un.m && l == un.l && t == un.t;}
	void Mult(const Unit &un) {
		m += un.m;
		l += un.l;
		t += un.t;
	}
	void Div(const Unit &un) {
		m -= un.m;
		l -= un.l;
		t -= un.t;
	}
	void Exp(double exp) {
		m *= exp;
		l *= exp;
		t *= exp; 
	}
	void Sqrt() {
		m /= 2.;
		l /= 2.;
		t /= 2.; 
	}
	
	void SetNull()             	{m = Null;}
	bool IsNullInstance() const	{return Upp::IsNull(m);}
	bool IsAdim() const			{return (m == 0) && (l == 0) && (t == 0);}
	
	double m, l, t;	
};

class doubleUnit : public Moveable<doubleUnit> {
public:
	doubleUnit() : doubleUnit(0)						{}
	doubleUnit(double _val) : val(_val), sval(String::GetVoid()), unit(0, 0, 0)	{}
	doubleUnit(const Nuller&) 							{SetNull();}
	
	double val;
	String sval;
	Unit unit;
	
	void Set(const doubleUnit &d) {
		val = d.val;
		sval = d.sval;
		unit.Set(d.unit);
	}
	void Sum(const doubleUnit &d) {
		if (!(unit.IsEqual(d.unit) || IsNull(unit) || IsNull(d.unit)))
			throw Exc(t_("Units does not match in summation"));
		val += d.val;
	}
	void Sub(const doubleUnit &d) {
		if (!(unit.IsEqual(d.unit) || IsNull(unit) || IsNull(d.unit)))
			throw Exc(t_("Units does not match in substraction"));
		val -= d.val;
	}
	void Mult(const doubleUnit &d) {
		unit.Mult(d.unit);
		val *= d.val;
	}
	void Div(const doubleUnit &d) {
		if (abs(d.val) < 1e-100)
			throw Exc(t_("Division by zero"));
		unit.Div(d.unit);
		val /= d.val;
	}
	void Neg() {
		val = -val;
	}
	void Exp(const doubleUnit &d) {
		if (!(IsNull(d.unit) || d.unit.IsAdim()))
			throw Exc(t_("Exponent cannot have units"));
		unit.Mult(d.unit);
		val = ::pow(val, d.val);
	}
	void Sqrt() {
		if (val < 0) 
			throw Exc(t_("Negative number sqrt"));
		val = sqrt(val);
		unit.Sqrt();
	}
	void ResParallel(const doubleUnit &d) {
		if (abs(val + d.val) < 1e-100 && abs(val*d.val) > 1e-100)
			throw Exc(t_("Division by zero"));
		if (!(unit.IsEqual(d.unit) || IsNull(unit) || IsNull(d.unit)))
			throw Exc(t_("Units does not match in resistor parallel"));
		if (abs(val*d.val) < 1e-100)
			val = 0.0;
		else
			val = val*d.val/(val + d.val);
	}	
	String ToString() const {
		if (sval.IsVoid()) 
			return FormatDouble(val);
		else
			return sval;
	}
	double ToDouble() const {
		if (sval.IsVoid()) 
			return val;
		else
			return ScanDouble(sval);
	}
	void SetNull()                 {val = Null;}
	bool IsNullInstance() const    {return IsNull(unit) && IsNull(val);}
};

template<> inline bool IsNull(const doubleUnit& r)  {return IsNull(r.val);}

class CParserPP : public CParser {
public:
	CParserPP() : CParser() {}
	CParserPP(const char *ptr) : CParser(ptr) {}
	
	String ReadIdPP() {
		if(!IsId())
			ThrowError("missing id");
		String result;
		const char *b = term;
		const char *p = b;
		
		while (true) {
			while (iscid(*p))
				p++;
			if ((*p) == '[') {
				p++;
				const char *p0 = p;
				while ((*p) >= '0' && (*p) <= '9')
					p++;
				if (p0 == p)
					ThrowError("empty vector index");
				if ((*p) != ']')
					ThrowError("wrong token found closing vector index");
				else
					p++;
			} else if ((*p) == '.')
				p++;
			else
				break;
		}
		term = p;
		DoSpaces();
		return String(b, (int)(uintptr_t)(p - b));
	}
};

class EvalExpr {
public:
	EvalExpr();
	void Clear() {
		variables.Clear();
		lastVariableSetId = -1;
	}
	doubleUnit Eval(String line);
	doubleUnit AssignVariable(String var, String expr);
	doubleUnit AssignVariable(String var, double d);
	String EvalStr(String line, int numDigits = 3);
	EvalExpr &SetCaseSensitivity(bool val = true) 			{noCase = !val;				return *this;}
	EvalExpr &SetErrorUndefined(bool val = true)  			{errorIfUndefined = val;	return *this;}
	EvalExpr &SetAllowString(bool val = true)  				{allowString = val;			return *this;}
	
	const String &GetFunction(int id) 						{return functions.GetKey(id);}
	int GetFunctionsCount() 								{return functions.GetCount();}
	
	void SetConstant(String name, doubleUnit value)			{constants.GetAdd(name) = value;}
	void SetConstant(int id, doubleUnit value)				{constants[id] = value;}
	const doubleUnit &GetConstant(String name) 				{return constants.Get(name, doubleUnit(Null));}
	void GetConstant(int id, String &name, doubleUnit &val)	{name = constants.GetKey(id); val = constants[id];}
	int GetConstantId(String &name)							{return constants.Find(name);}
	int GetConstantsCount() 								{return constants.GetCount();}

	void RenameVariable(String name, String newname);
	
	doubleUnit &GetVariable(String name) {
		int id = FindVariable(name);
		if (id >= 0)
			return variables[id];
		if (errorIfUndefined)
			EvalThrowError(p, Format(t_("Unknown identifier '%s'"), name));
		return variables.Add(name, Null);
	}
	doubleUnit &GetVariable(int id)							{return variables[id];}
	void GetVariable(int id, String &name, doubleUnit &val)	const {name = variables.GetKey(id); val = variables[id];}
	const String &GetVariableName(int id) const				{return variables.GetKey(id);}
	Vector<int> FindPattern(String yes, String no) const;
	Vector<int> FindPattern(String yes) const 				{return FindPattern(yes, "");}
	Vector<int> FindPattern(String yes, String no, String yes2) const;
	int GetVariablesCount() 								{return variables.GetCount();}
	void ClearVariables();
	String &GetLastError()									{return lastError;}
	
	VectorMap<String, doubleUnit> constants;
	VectorMap<String, doubleUnit (*)(doubleUnit)> functions;

	CParserPP p;

	static void EvalThrowError(CParserPP &p, const char *s);
	
	virtual int FindVariable(String strId) 					{return variables.Find(strId);}
	Vector<int> FindVariableList(String patternyes, String patternno = "") {
		Vector<int> ret;
		if (noCase) {
			patternyes = ToLower(patternyes);
			patternno = ToLower(patternno);
		}
		for (int i = 0; i < variables.GetCount(); ++i) {
			if (PatternMatch(patternyes, variables.GetKey(i)) && !PatternMatch(patternno, variables.GetKey(i)))
				ret << i;
		}
		return ret;
	}
		
protected:
	doubleUnit Exp(CParserPP& p);

	void SetVariable(String name, doubleUnit value)	{
		lastVariableSetId = variables.FindAdd(name);
		variables[lastVariableSetId].Set(value);
	}
	void SetVariable(int id, doubleUnit value) {
		lastVariableSetId = id;
		variables[id].Set(value);
	}
	int FindAddVariable(String name) {
		return lastVariableSetId = variables.FindAdd(name);
	}
	void RemoveVariable(int id) {
		variables.Remove(id);
	}
	bool noCase;
	String lastError;
	VectorMap<String, doubleUnit> variables;
	bool errorIfUndefined;
	bool allowString;
	int lastVariableSetId = -1;
		
	bool IsFunction(String str)			{return functions.Get(str, 0);}
	bool IsConstant(String str)			{return !IsNull(GetConstant(str));}
	
private:
	void *Functions_Get(CParserPP& p);
	doubleUnit Term(CParserPP& p);
	doubleUnit Pow(CParserPP& p);
	doubleUnit Mul(CParserPP& p);
	
	String TermStr(CParserPP& p, int numDigits);
	String PowStr(CParserPP& p, int numDigits);
	String MulStr(CParserPP& p, int numDigits);
	String ExpStr(CParserPP& p, int numDigits);
};

class UserEquation : public ExplicitEquation {
public:
	UserEquation() {}
	UserEquation(String _name, String _strEquation, String varHoriz = "x")	{Init(_name, _strEquation, varHoriz);}
	void Init(String _name, String _strEquation, String varHoriz = "x") {
		name = _name;
		_strEquation.Replace(" ", "");
		StringStream str(_strEquation);
		Vector<String> parts = GetCsvLine(str, ';', CHARSET_DEFAULT);
		if (parts.IsEmpty())
			return;
		strEquation = parts[0];
		eval.Clear();
		eval.SetConstant(varHoriz, doubleUnit(23));
		idx = eval.GetConstantsCount() - 1;
		eval.EvalStr(strEquation);
		coeff.Clear();
		varNames.Clear();
		if (eval.GetVariablesCount() == 0)
			coeff.SetCount(1);
		else {
			for (int i = 0; i < eval.GetVariablesCount(); ++i) {
				String varName;
				doubleUnit dummy;
				eval.GetVariable(i, varName, dummy);
				varNames << varName;
				int istr;
				for (istr = 1; istr < parts.GetCount(); ++istr) {
					String strVar = varName + "=";
					int ifound = parts[istr].Find(strVar);
					if (ifound >= 0) {
						double val = ScanDouble(parts[istr].Mid(strVar.GetCount()));
						coeff << val;
						break;
					}
				}
				if (istr == parts.GetCount())
					coeff << 0.1;	
			}
		}
		numcoeff = coeff.size();
	}
	double f(double x)  {
		eval.SetConstant(idx, doubleUnit(x));
		for (int i = 0; i < varNames.GetCount(); ++i) 
			eval.AssignVariable(varNames[i], coeff[i]);
		return eval.Eval(strEquation).val;
	}
	void SetName(String _name) 					    {name = _name;}
	virtual String GetName() 						{return name;}
	virtual String GetEquation(int numDigits = 3)	{return eval.EvalStr(strEquation, numDigits);}
	virtual void GuessCoeff(DataSource &)		 	{}
	void SetDegree(int)								{NEVER();}

private:
	String name;
	String strEquation;
	Vector<String> varNames;
	EvalExpr eval;
	int idx;
};

}

#endif
