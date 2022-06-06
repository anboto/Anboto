// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2022, the Anboto author and contributors
#ifndef _STEM4U_SeaWaves_h_
#define _STEM4U_SeaWaves_h_


namespace Upp {

struct WaveParam {
	String ToString() {
		return Format("H1/3=%.3f, Hm0=%.3f, Tp=%.3f, Tz=%.3f, Te=%.3f, Tm02=%.3f, gamma=%.3f (%.3f)",
					H1_3, Hm0, Tp, Tz, Te, Tm02, gamma, r2gamma);
	}
	double H1_3;
	double Hm0_var;		// 4*sqrt(free surface variance)
	double Hrms;
	double Havg;
	double Tz;
	double Hm0;			// 4*sqrt(m0)		
	double Tp;
	double Te;			// Tm-1,0 = m-1/m0
	double Tm02;		// sqrt(m0/m2)
	double eps1, eps2;	// Spectral bandwidth
	double m_1, m0, m1, m2;
	double power, powerTheo;
	double gamma;
	double r2gamma;
};
	
class SeaWaves {
public:
	double rho; 	//	Water density (kg/m2) 
	double g; 		//	Gravity (m/s2)

	SeaWaves() : rho(1024), g(9.81), nf(0) {}
	SeaWaves(double _rho, double _g) : rho(_rho), g(_g), nf(0) {}
	
	// Calculation results	
	double zSurf; 		// Free surface at x, y
	double dzSurf;
	double vx, vy, vz; 	// Velocities of a particle at x, y, z (z=0 is free surface)
	double ax, ay, az; 	// Accelerations of a particle at x, y, z (z=0 is free surface)
	double p;			// Pressure (hydrostatic + dynamic)

	bool Init(double _Tp, double _Hs, double _dirM = 0, double _h = 70, int _nd = 15, int _nf = 600, 
				double gamma = 3.3, double disp_ang = M_PI/3., int seed = 123456, double fmin = -1, double fmax = -1);
	bool Calc(double x, double y, double z, double t);	
	double ZSurf(double x, double y, double t);
	double Pressure(double x, double y, double z, double t);
	double ZWheelerStretching(double z, double et);
		
	void Clear();
	
	static double Power(double Te, double Hs, double h, double g, double rho);

	static double WaveNumber(double T, double h, double g, bool exact = true);
	static double WaveLength(double T, double h, double g);
	static double Celerity(double T, double h, double g);		
	static double GroupCelerity(double T, double h, double g);
	
	static double JONSWAP_Spectrum(double Hm0, double Tp, double gamma, double freq);
	bool JONSWAP_Spectrum_test(double Hm0, double Tp, double gamma);
		
	enum SEA_TYPE {SHALLOW, INTERMEDIATE, DEEP};
	static SEA_TYPE GetSeaType(double Tp, double h, double g);
	
	void Rotate(double angleRad);
	void TimeShift(double deltaTime);

	static bool LoadSeries(String filename, Eigen::VectorXd &t, Eigen::VectorXd &et, int separator = ',', int col_t = 0, int ool_et = 1, int fromRow = 1);
	bool GetZSurf(Eigen::VectorXd &t, Eigen::VectorXd &et, double x, double y, double duracionRegistro, double deltaT);
	bool SaveZSurf(double x, double y, String filename, double duration, double deltaT, char separator = ';');
	
	static void GetWaveParam(WaveParam &param, const Eigen::VectorXd &fs, double deltaT, double h, double g, double rho);
	
	void Xmlize(XmlIO& xml) 	{Ize(xml);}
	void Jsonize(JsonIO& json) 	{Ize(json);}
	
	int GetNf() 				{return nf;}
	
private:
	double Tp; 		// Peak period (s)
	double Hs; 		// Significant wave height (m)
	double dirM;
	double h;  		// Sea depth (m)

	// Spectral characteristics
	int nd; 	// Directional components number
	int nf; 	// Frequential components number	

	Eigen::MatrixXd A;			// m	Amplitudes
	Eigen::VectorXd frec;		// Hz	Frequencies
	Eigen::MatrixXd ph;			// Hz	Phases
	Eigen::VectorXd k; 			// m-1	Wave numbers
	Eigen::VectorXd dirs;		// rad	Directions
	
	static void GetWaveSpectralParam(WaveParam &param, const UVector<Pointf> &psd, double h, double g, double rho);
	static void GetWaveTpSmooth(WaveParam &param, const Vector<Pointf> &psd);
	static double SpectrumPower(const UVector<Pointf> &psd, double height, double g, double rho);
	static bool JONSWAP_Fit(const UVector<Pointf> &psd, double Hm0, double Tp, double &gamma, double &r2);
	
	template <class T>
	void Ize(T& io) { 
		io
			("Hs", Hs)
			("Tp", Tp)
			("dirM", dirM)
			("h", h)
			("nd", nd)
			("nf", nf)
			("A", A)
			("frec", frec)
			("ph", ph)
			("k", k)
			("dirs", dirs)
			("rho", rho)
			("g", g)
		;
	}
};


double Te_fTp(double Tp, double gamma);
double Tp_fTe(double Te, double gamma);
double gamma_fTp_Te(double Tp, double Te);
double Tp_fTm(double Tm, double gamma); 
double Tp_fTz(double Tz, double gamma);
double gamma_fTp_Tz(double Tp, double Tz);
	
}

#endif
