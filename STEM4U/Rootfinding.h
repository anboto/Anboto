#ifndef _STEM4U_Rootfinding_h_
#define _STEM4U_Rootfinding_h_

#include <STEM4U/Utility.h>


namespace Upp {

template <typename T> 
static bool IsBracketed(T yfrom, T yto) {
	return !(yfrom*yto >= 0.0); 
}

template <typename T> 
T Bisection(Function <T(T)> f, const T from, const T to, const T toly, const int maxIt = 50) {
	T yfrom = f(from),
	  yto = f(to);
	  
	if (!IsBracketed(yfrom, yto))
		return Null; 

	T dx, rtb;
	if (yfrom < 0) {
		dx = to - from;
		rtb = from;
	} else {
		dx = from - to;
		rtb = to; 
	}
	
	for (int j = 0; j < maxIt; j++) { 
		T xmid;
		yto = f(xmid = rtb + (dx *= 0.5)); 
		if (yto <= 0) 
			rtb = xmid;
		if (abs(dx) <= toly || yto == 0) 
			return rtb;
	}
	return Null; 
}

template <typename T> 
T Brent(Function <T(T)> f, const T from, const T to, const T toly, const int maxIt = 50) {
	const T eps = std::numeric_limits<T>::epsilon();
	
	T xfrom = from, 
	  xto = to, 
	  c = to, d = NAN, e = NAN, 
	  yfrom = f(xfrom), 
	  yto = f(xto);
	  
	if (!IsBracketed(yfrom, yto))
		return Null; 
		
	T fc = yto;
	for (int iter = 0; iter < maxIt; iter++) {
		if ((yto > 0 && fc > 0) || (yto < 0 && fc < 0)) {
			c = xfrom; 
			fc = yfrom;
			e = d = xto - xfrom;
		}
		if (abs(fc) < abs(yto)) {
			xfrom = xto;
			xto = c;
			c = xfrom;
			yfrom = yto;
			yto = fc;
			fc = yfrom;
		}
		T tol1 = 2*eps*abs(xto) + 0.5*toly; 
		T xm = 0.5*(c - xto);
		if (abs(xm) <= tol1 || yto == 0.0) 
			return xto;
		if (abs(e) >= tol1 && abs(yfrom) > abs(yto)) {
			T s=yto/yfrom; 
			T p, q;
			if (xfrom == c) {
				p=2.0*xm*s;
				q=1.0-s;
			} else {
				q = yfrom/fc;
				T r = yto/fc;
				p = s*(2*xm*q*(q - r) - (xto - xfrom)*(r - 1));
				q = (q - 1)*(r - 1)*(s - 1);
			}
			if (p > 0) 
				q = -q;
			p = abs(p);
			
			T min1 = 3*xm*q - abs(tol1*q);
			T min2 = abs(e*q);
			if (2*p < (min1 < min2 ? min1 : min2)) {
				e = d; 
				d = p/q;
			} else {
				d = xm;
				e = d;
			}
		} else { 
			d = xm;
			e = d;
		}
		xfrom = xto; 
		yfrom = yto;
		if (abs(d) > tol1) 
			xto += d;
		else
			xto += tol1*Sign(xm);
		yto = f(xto);
	}
	return Null;
}

template <typename T> 
bool RootBracketing(Function <T(T)> f, const T init, const T mn, const T mx, const T dx, T &from, T &to, bool bidirectional) {
	T froml, fromr;
	int sfroml, stol, sfromr, stor;
		
	froml = fromr = init;
	
	T yfrom = f(init),
	  yto = f(init+dx);
	
	bool left, right;
	if (bidirectional) 
		left = right = true;
	else if (yto > yfrom) {
		left = true;
		right = false;
	} else {
		left = false;
		right = true;
	}
	
	if (left) {
		sfroml = Sign(yfrom);
		stol = Sign(yto);
	}
	if (right) {
		sfromr = Sign(yfrom);
		stor = Sign(yto);	
	}
	
	while (true) {
		if (left && froml >= mn) {
			if (sfroml != stol) {
				from = froml;
				to = froml + dx; 
				return true;
			}
			stol = sfroml;
			sfroml = Sign(f(froml -= dx));	
		}
		if (right && fromr+dx <= mx) {
			if (sfromr != stor) {
				from = fromr;
				to = fromr + dx; 
				return true;
			}		
			sfromr = stor;
			stor = Sign(f(dx + (fromr += dx)));
		}
	}
	return false;
}

template <typename T> 
T NewtonRaphson(Function <T(T)> f, Function <T(T)> df, const T init, const T toly, const int maxIt = 50) {
	T x = init;
	
	for (int i = 0; abs(f(x)) > toly; ++i) {
		if (i >= maxIt)
			return Null;
        x -= f(x)/df(x);
	}
    return x;
}

template <typename T> 
T QuasiNewtonRaphson(Function <T(T)> f, const T init, const T ddx, const T toly, const int maxIt = 50) {
	T x = init;
	
	for (int i = 0; abs(f(x)) > toly; ++i) {
		if (i >= maxIt)
			return Null;
		T fx = f(x);
		T fdx = f(x + ddx);
        x -= fx/((fdx - fx)/ddx);
	}
    return x;
}
/*
template <typename T> 
T RootFindingSlow(Function <T(T)> f, const T initx, const T tolx, const T toly, const int minData = 3, const int maxIt = 50) {
	Vector<T> x, y;
	
	T a, b;
	T nx, ny;
	
	if (abs(ny = f(initx)) <= toly)
		return initx;
	
	x << initx;
	y << ny;
	
	nx = initx + tolx;
	if (abs(ny = f(nx)) <= toly)
		return nx;
	
	x << nx;
	y << ny;

	for (int i = 0; i < maxIt; ++i) {
		LinearRegression(x, y, a, b);
		
		if (a == 0)
			return Null;
		
		nx = -b/a;		// y = ax + b; y = 0 => x = -b/a;
		ny = f(nx);
		
		if (abs(ny) <= toly)
			return nx;
		//
		for (int ix = 0; ix < x.size(); ++ix)
			if (abs(x[ix] - nx) < tolx/100.)
				return Null;//
		
		x << nx;
		y << ny;

		IndexSort(y, x, [&](T a, T b)-> bool {return abs(a) < abs(b);});	
		if (x.size() > minData) {
			x.SetCount(minData);
			y.SetCount(minData);
		}
	}
    return Null;
}
*/
}

#endif
