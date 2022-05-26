// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2022, the Anboto author and contributors
#include <Core/Core.h>
#include <Functions4U/Functions4U.h>
#include <Eigen/Eigen.h>
#include <ScatterDraw/DataSource.h>
#include <ScatterDraw/Equation.h>
#include "SeaWaves.h"


namespace Upp {

using namespace Eigen;


// From "Direct Solution of Wave Dispersion Equation" (Hunt, 1979), read from "Water Wave Mechanics for Engineers and Scientists" (Dean & Dalrymple)
double SeaWaves::WaveNumber(double T, double h, double g, bool exact) {
	double y = 4*M_PI*M_PI*h/g/(T*T);
	double k1 = 1 + 0.6666666666*y + 0.3555555555*pow(y,2) + 0.1608465608*pow(y,3) 
				+ 0.0632098765*pow(y,4) + 0.0217540484*pow(y,5) + 0.0065407983*pow(y,6);
	double k = sqrt((y*y + y/k1))/h;
	
	if (!exact)
		return k;
	
	double w = 2*M_PI/T;
	
	return SolveNonLinearEquation(k, [&](double k)->double {return w*w - g*k*tanh(k*h);});
}

SeaWaves::SEA_TYPE SeaWaves::GetSeaType(double T, double h, double g) {
    double h_L = h/WaveLength(T, h, g);
	if (h_L < 1./20)
		return SHALLOW;
	else if (h_L < 1./2)
		return INTERMEDIATE;
	return DEEP;
}

double SeaWaves::WaveLength(double T, double h, double g) 	{return 2*M_PI/WaveNumber(T, h, g);}

double SeaWaves::Celerity(double T, double h, double g)	  	{return WaveLength(T, h, g)/T;}

double SeaWaves::GroupCelerity(double T, double h, double g) {
    double k = WaveNumber(T, h, g);
    double L = 2*M_PI/k;			
    double c = L/T;               	
    double n = 0.5*(1 + 2*k*h/sinh(2*k*h));   
    return c*n;                
}

double SeaWaves::Power(double Te, double Hs, double h, double g, double rho) {
	double k = WaveNumber(Te, h, g);
	double L = 2*M_PI/k;			// Wavelength, L (m)	
	double c = L/Te;               	// Wave celerity, c (m/s)
	double n = 0.5*(1 + 2*k*h/sinh(2*k*h));   
	double Cg = c*n;                // Group celerity, Cg (m/s)
	double E = 1/16.*rho*g*Hs*Hs;	// Energy per unit area
	return E*Cg/1000.;   			// Energy flow, or power per unit width (kW/m of width)	    	
}

double SeaWaves::JONSWAP_Spectrum(double Hm0, double Tp, double gamma, double freq) {
	double sigma_f = freq <= 1/Tp ? 0.07 : 0.09;	     
	double beta = 0.0624/(0.230 + 0.0336*gamma - 0.185/(1.9 + gamma))*(1.094 - 0.01915*log(gamma));
		
	return beta*Hm0*Hm0*pow(Tp,-4)*pow(freq, -5)*exp(-5./4.*pow(Tp*freq, -4)) * pow(gamma, exp(-pow((Tp*freq - 1), 2)/(2*sigma_f*sigma_f)));	
}

bool SeaWaves::JONSWAP_Spectrum_test(double Hm0, double Tp, double gamma) {
	if ((Tp > 5*sqrt(Hm0)) || (Tp < 3.6*sqrt(Hm0)))
		return false;
	if (gamma > 7 || gamma < 1) 
		return false;
	return true;
}

bool SeaWaves::Init(double Tp, double Hs, double dirM, double h, int nd, int nf, double gamma, 
					double disp_ang, int seed, double fmin, double fmax) {
	if(!(Tp > 0 && nd > 0 && nf > 0 && gamma >= 1))
		return false;
	
	this->Tp = Tp; 				
	this->dirM = dirM; 			
	this->Hs = Hs;             	
	this->h = h;                
	this->nd = nd; 				
	this->nf = nf = nf/nd;	

	frec.resize(nf);
	k.resize(nf);
	dirs.resize(nd);
	A.resize(nd, nf);
	ph.resize(nd, nf);

	double fp = 1/Tp;      
	if (nf > 1) {
		if (fmin < 0)
			fmin = 0.25/Tp;
		else if (fmin > fp)
			return false;
		if (fmax < 0)
			fmax = 4/Tp;   
		else if (fmax < fp)
			return false;
	} else
		fmin = fmax = fp;
	
	double df = 1;
	if (nf > 1)
		df = (fmax-fmin)/(nf - 1);
	
	for(int f = 0; f < nf; f++) {
		int n = f + 1;
	    frec[f] = fmin + (n - 1)*df;
	    k[f] = WaveNumber(1/frec[f], h, g); 
	}
        
	Buffer<double> Sf_f(nf);
	
	if (nf > 1) {
		double beta = 0.0624/(0.230 + 0.0336*gamma - 0.185*(pow(1.9 + gamma, -1)))*(1.094 - 0.01915*log(gamma));
		for(int f = 0; f < nf; f++) {
			double sigma_f;
			if(frec[f] <= fp)
		        sigma_f = 0.07;
			else
				sigma_f = 0.09;	     
	
			Sf_f[f] = beta*pow(Hs, 2)*pow(Tp,-4)*pow(frec[f], -5)*exp(-1.25*pow(Tp*frec[f], -4)) 
						  *pow(gamma, exp(-pow((Tp*frec[f]-1), 2)/2/pow(sigma_f, 2)));
		}
	} else 
		Sf_f[0] = 1/2.*pow(Hs/2., 2);
	
	if (nd > 1) {
	    double dirmin = dirM - disp_ang; 	
	    double dirmax = dirM + disp_ang; 	
	    double dd = (dirmax - dirmin)/(nd-1);
	    
	    for (int d = 0; d < nd; ++d)
	        dirs[d] = dirmin + d*dd;    

	    double L0 = g*Tp*Tp/2./M_PI; 
	    double per = Hs/L0;  								
	    double Smax = pow(10, -1.2195*log10(per) - 0.5573); 
	    
		Buffer<double> incdir_d(nd);
		
		for (int d = 0; d < nd; ++d)
			incdir_d[d] = dirs[d] - dirM;
				
		for (int f = 0; f < nf; f++) { 
			double s_f;
		    if (frec[f] <= fp)
		        s_f = pow((frec[f]/fp), 5)*Smax;
		    else
		        s_f = pow((frec[f]/fp), -2.5)*Smax;   
		    
		    double G0_f = 0;  
		    for (int d = 0; d < nd; d++) 
		        G0_f += pow(cos(incdir_d[d]/2), 2*s_f)*dd;
		    G0_f = 1/G0_f;
	
	      	for (int d = 0; d < nd; d++) {
		        double G_fd = G0_f*pow(cos(incdir_d[d]/2.), 2*s_f);
		        double Sfd_fd = Sf_f[f]*G_fd;
		        A(d, f) = sqrt(2*Sfd_fd*df*dd);
		    }
		}
	} else {
	    dirs[0] = dirM;
	    for (int f = 0; f < nf; ++f)
	        A(0, f) = sqrt(2*Sf_f[f]*df);
	}
		
	if (nf > 1) {
		if (IsNull(seed)) {
			std::random_device rd;
			seed = rd();
		} 
		std::mt19937 random_engine(seed);
		std::uniform_real_distribution<double> random_distribution(-M_PI, M_PI);
		for (int f = 0; f < nf; f++) 
		    for (int d = 0; d < nd; d++) 
				ph(d, f) = random_distribution(random_engine);		
	} else {
		for (int d = 0; d < nd; d++) 
			ph(d, 0) = 0;
	}
	
	return true;
}

bool SeaWaves::Calc(double x, double y, double z, double t) {	
	ASSERT(frec.size() > 0);
	
	if (z > 0)
		return false;
	
    zSurf = dzSurf = vx = vy = vz = ax = ay = az = 0;
	
	p = -rho*g*z;    
	
	for (int ifr = 0; ifr < nf; ifr++) {
    	for (int id = 0; id < nd; id++) {
    		double f = frec[ifr];
    		double sindirsd = sin(dirs[id]);
    		double cosdirsd = cos(dirs[id]);
    		double aux = k[ifr]*(cosdirsd*x + sindirsd*y) - 2*M_PI*f*t + ph(id, ifr);
    		double cosaux = cos(aux);
    		double sinaux = sin(aux);
    		
	        double AUXsl = A(id, ifr)*cosaux;         
            zSurf += AUXsl;
            dzSurf += 2*M_PI*f*A(id, ifr)*sinaux;         
	        
	        double kp     = cosh(k[ifr]*(h+z))/cosh(k[ifr]*h);
	        double kp_sin = sinh(k[ifr]*(h+z))/cosh(k[ifr]*h);
	
	        double AUXvh = g*A(id, ifr)*k[ifr]/(2*M_PI*f)*kp*cosaux;         
	        vx += AUXvh*cosdirsd;
	        vy += AUXvh*sindirsd;
	        
	        vz += g*A(id, ifr)*k[ifr]/(2*M_PI*f)*kp_sin*sinaux;
	        
	        double AUXah = g*A(id, ifr)*pow((k[ifr]),2)/(2*M_PI*f)*kp*sinaux;
	        ax += AUXah*cosdirsd;
	        ay += AUXah*sindirsd;
	    
	        az += -g*A(id, ifr)*pow((k[ifr]),2)/(2*M_PI*f)*kp_sin*cosaux;         
	        
	   		p += rho*g*kp*AUXsl;	
    	}
	}
	if (p < 0)
		p = 0;
	return true;
}

double SeaWaves::ZSurf(double x, double y, double t) {
	ASSERT(frec.size() > 0);
	
    double zSurf = 0;
	for (int ifr = 0; ifr < nf; ifr++) 
    	for (int id = 0; id < nd; id++) 
	        zSurf += A(id, ifr)*cos(k[ifr]*cos(dirs[id])*x + k[ifr]*sin(dirs[id])*y 
	        						  - 2*M_PI*frec[ifr]*t + ph(id, ifr));  
    return zSurf;
}


bool SeaWaves::CalcWheeler(double x, double y, double z, double t) {	
	double et = ZSurf(x, y, t);
	if (z > et)
		return false;
	
	double zp = h*(h + z)/(h + et) - h;
	
	return Calc(x, y, zp, t);
}
	
double SeaWaves::Pressure(double x, double y, double z, double t) {
	ASSERT(frec.size() > 0);
	
	if (z > 0)
		return Null;
			
	double p = -rho*g*z;  
	for (int f = 0; f < nf; f++) {
    	for (int d = 0 ; d < nd; d++) {
	        double KP    = cosh(k[f]*(h+z))/cosh(k[f]*h);	
    		double AUXsl = A(d, f)*cos(k[f]*cos(dirs[d])*x + k[f]*sin(dirs[d])*y - 2*M_PI*frec[f]*t + ph(d, f));         
    		p += rho*g*KP*AUXsl;	
    	}
	}
	if (p < 0)
		p = 0;
    return p;
}

double SeaWaves::PressureWheeler(double x, double y, double z, double t) {
	double et = ZSurf(x, y, t);
	if (z > et)
		return 0;
	
	double zp = h*(z - et)/(h + et);		//   h*(h + z)/(h + et) - h;
	
	return Pressure(x, y, zp, t);	
}
	
void SeaWaves::Clear() {
	Tp = -1;	
	nd = nf = 0;
	Upp::Clear(A);
	Upp::Clear(frec);
	Upp::Clear(ph);
	Upp::Clear(k);
	Upp::Clear(dirs);
}
		
void SeaWaves::Rotate(double angleRad) {
	for (int i = 0; i < dirs.size(); ++i)
		dirs(i) += angleRad;
}

void SeaWaves::TimeShift(double deltaTime) {
	for (int i = 0; i < ph.size(); ++i)
		ph(i) -= 2*M_PI*frec(i)*deltaTime;	
}

bool SeaWaves::GetZSurf(VectorXd &t, VectorXd &et, double x, double y, double duracionRegistro, double deltaT) {
	int sz = int(duracionRegistro/deltaT + 1);
	t.resize(sz);
	et.resize(sz);
	for (int i = 0; i < sz; ++i) { 
		t[i] = i*deltaT;
		et[i] = ZSurf(x, y, t[i]);					
	}
	return true;
}

bool SeaWaves::SaveZSurf(double x, double y, String filename, double duration, double deltaT, char separator) {
	VectorXd t, et;
	
	if (!GetZSurf(t, et, x, y, duration, deltaT))
		return false;
	
	String out;
	out << "time" << separator << "fs\n";					
	for (int i = 0; i < t.size(); ++i) 
		out << t[i] << separator << et[i] << "\n";					
	return SaveFile(filename, out);
}

bool SeaWaves::LoadSeries(String filename, VectorXd &t, VectorXd &et, int separator, int col_t, int col_et, int fromRow) {
	Vector<Vector <Value> > data = ReadCSVFile(filename, separator, false, false, '.', false, fromRow);
	if (data.size() == 0)
		return false;
	t.resize(data.size());
	et.resize(data.size());
	for (int i = 0; i < data.size(); ++i) {
		t[i] = data[i][col_t];
		et[i] = data[i][col_et];
	}
	return true;
}

void SeaWaves::GetWaveParam(WaveParam &param, const Eigen::VectorXd &fs, double deltaT, double h, double g, double rho) {
	Vector<double> tMax, _Max, tMin, _Min;
	
	EigenVector vect(fs, 0, deltaT);
	
	Vector<double> zeros;
	Vector<int64> ids;
	
	vect.ZeroCrossingY(false, true, zeros, ids);
	
	for (int id = 0; id < ids.size() - 1; ++id) {
		double vmax = DBL_MIN, vmin = DBL_MAX;
		double tmax, tmin;
		for (int i = int(ids[id]); i < int(ids[id + 1]); ++i) {
			if (fs[i] > vmax) {
				vmax = fs[i];
				tmax = i*deltaT;
			}
			if (fs[i] < vmin) {
				vmin = fs[i];
				tmin = i*deltaT;
			}
		}
		tMax << tmax;
		_Max << vmax;
		tMin << tmin;
		_Min << vmin;
	}
	
	Vector<double> listaHs;
	for (int i = 0; i < tMax.size(); ++i) 
		listaHs << (_Max[i] - _Min[i]);
	Sort(listaHs);
	
	param.Havg = param.Hrms = 0;
	for (int i = 0; i < listaHs.size(); ++i) {
		param.Havg += listaHs[i];
		param.Hrms += sqr(listaHs[i]);
	}
	param.Havg /= listaHs.size();
	param.Hrms = sqrt(param.Hrms/listaHs.size());
	
	param.H1_3 = 0;
	int oneThird = listaHs.size()/3;
	for (int i = listaHs.size() - oneThird; i < listaHs.size(); ++i) 
		param.H1_3 += listaHs[i];
	param.H1_3 /= oneThird;
	
	// Hm0 calculated with m0 obtained with the variance of the free surface
	double avg = 0;
	for (int i = 0; i < fs.size(); ++i)
		avg += fs[i];
	avg /= fs.size();
	double var = 0;
	for (int i = 0; i < fs.size(); ++i)
		var += sqr(fs[i] - avg);
	var = var/(fs.size() - 1);
	param.Hm0_var = 4*sqrt(var);
	
	Vector<double> listaTz;
	for (int i = 0; i < zeros.size() - 1; ++i) 
		listaTz << (zeros[i + 1] - zeros[i]);
	param.Tz = 0;
	for (int i = 0; i < listaTz.size(); ++i) 
		param.Tz += listaTz[i];
	param.Tz /= listaTz.size();
	
	// Spectral parameters
	double totalTime = (fs.size() - 1)*deltaT;
	int numSubsets = fceil(totalTime/(6*60));		// Approx. 6 min per subset
	
	EigenVector series(fs, 0, deltaT);
	
	Vector<Pointf> psd = series.FFTY(deltaT, false, FFT_TYPE::T_PSD, FFT_WINDOW::COS, 
									numSubsets, 0.1);
	GetWaveSpectralParam(param, psd, h, g, rho);
	GetWaveTpSmooth(param, psd);
}

void SeaWaves::GetWaveSpectralParam(WaveParam &param, const Upp::Vector<Pointf> &psd, double h, double g, double rho) {
	VectorPointf psdData(psd);
		
	psdData.GetSpectralMomentsY(false, param.m_1, param.m0, param.m1, param.m2);
	
	param.Te = param.m_1/param.m0;
	param.Tm02 = sqrt(param.m0/param.m2);
	param.Hm0 = 4*sqrt(param.m0);	
	
	param.Hm0 *= .75;
	
	param.eps1 = sqrt(param.m1*param.m_1/(param.m0*param.m0) - 1);
	param.eps2 = sqrt(param.m0*param.m2/(param.m1*param.m1) - 1);
	
	param.power = SpectrumPower(psd, h, g, rho);
	param.powerTheo = Power(param.Te, param.Hm0, h, g, rho);
}

void SeaWaves::GetWaveTpSmooth(WaveParam &param, const Vector<Pointf> &psd) {	
	VectorPointf psdData(psd);
	
	double baseWidth = 2*(psd[1].x - psd[0].x);			// deltaT;
	double rangeX = psd[psd.size()-1].x - psd[0].x;	//deltaT*(fs.size() - 1);
	Vector<Pointf> secAvg;
	VectorPointf sector(secAvg);
	for(double width = baseWidth; width < rangeX/10.; width += baseWidth) {
		secAvg = psdData.SectorAverageY(width);
		Vector<int64> ids;
		sector.MaxListY(ids, 10*baseWidth);
		if (ids.size() < 5) 
			break;
	}	
	int64 idMaxPSD;
	sector.MaxY(idMaxPSD);
	Pointf p = sector.MaxSubDataImpY(idMaxPSD, 3);
	param.Tp = p.x;
	
	JONSWAP_Fit(psd, param.Hm0, param.Tp, param.gamma, param.r2gamma);
}

double SeaWaves::SpectrumPower(const Vector<Pointf> &psd, double h, double g, double rho) {
	double power = 0;
	int numData = psd.size();
	for (int i = 1; i < numData; ++i) {
		double fi   = 1/psd[i].x;
		double fi_1 = 1/psd[i-1].x;
		double deltaX = fi_1 - fi;
	
		double Si   = psd[i].y;
		double Si_1 = psd[i-1].y;
		double ki   = WaveNumber(1/fi, h, false);
		double ki_1 = WaveNumber(1/fi_1, h, false);
		
		if (fi != 0 && fi_1 != 0) {
			double right = Si*fi/ki/2*(1 + 2*ki*h/sinh(2*ki*h));
			double left = Si_1*fi_1/ki_1/2*(1 + 2*ki_1*h/sinh(2*ki_1*h));
			power += (left + right)*deltaX/2.; 
		}
	}
	power *= rho*g*2*M_PI/1000.;
	return power;
}

class JONSWAPEquation : public ExplicitEquation {
	public:
		JONSWAPEquation() 						{}
		JONSWAPEquation(double Hm0, double Tp, double gamma) 	{Init(Hm0, Tp, gamma);}
		void Init(double Hm0, double Tp, double gamma) {
			this->Hm0 = Hm0;
			this->Tp = Tp;
			this->gamma = gamma;
		}
		double f(double T)	{return SeaWaves::JONSWAP_Spectrum(Hm0, Tp, gamma, 1/T);}
		virtual String GetName() 						{return t_("JONSWAP parametric");}
		virtual String GetEquation(int numDigits = 3)	{return "";}
		void SetDegree(int num)							{NEVER();}
		virtual void GuessCoeff(DataSource &series)		{NEVER();}	
		
	private:
		double Hm0, Tp, gamma;		
};

bool SeaWaves::JONSWAP_Fit(const Vector<Pointf> &psd, double Hm0, double Tp, double &gamma, double &r2) {
	VectorPointf psdData(psd);
	gamma = 0;
	r2 = 0;
	JONSWAPEquation jonswap;
	for (double gm = 1; gm <= 10; gm += 0.1) {
		jonswap.Init(Hm0, Tp, gm);
		double cadar2 = jonswap.R2Y(psdData);
		if (r2 < cadar2) {
			r2 = cadar2;
			gamma = gm;
		}
	}
	return r2 != 0;
}



// DNV-RP-C205, page 34 
double Te_fTp(double Tp, double gamma) 	  {return Tp*(4.2 + gamma)/(5 + gamma);}
double Tp_fTe(double Te, double gamma) 	  {return Te*(5 + gamma)/(4.2 + gamma);}
double gamma_fTp_Te(double Tp, double Te) {return (Te*5 - Tp*4.2)/(Tp - Te);}
double Tp_fTm(double Tm, double gamma) 	  {return Tm/(0.7303 + 0.04936*gamma - 0.006556*pow(gamma, 2) + 0.0003610*pow(gamma, 3));}
double Tp_fTz(double Tz, double gamma) 	  {return Tz/(0.6673 + 0.05037*gamma - 0.006230*pow(gamma, 2) + 0.0003341*pow(gamma, 3));}
	
double gamma_fTp_Tz(double Tp, double Tz) {
	VectorXd x(1);
	x[0] = 5;
	if (SolveNonLinearEquations(x, [&](const VectorXd &x, VectorXd &residual)->int {
		double gamma = x[0];
		double gamma2 = gamma*gamma;
		residual[0] = - Tz/Tp + 0.6673 + 0.05037*gamma - 0.006230*gamma2 + 0.0003341*gamma*gamma2;
		return 0;
	}))
		return x[0];
	return -1;
}
	
}	