// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2022, the Anboto author and contributors
#include <Core/Core.h>
#include <Eigen/Eigen.h>

using namespace Upp;

#ifdef USE_FFTW
#include <fftw3.h>
#endif

using namespace Eigen;


void FFTTests()
{
	UppLog() << "\nFFT sample\nGets the FFT of equation"
			  "\n f(t) = 2*sin(2*PI*t/50 - PI/3) + 5*sin(2*PI*t/30 - PI/2) + 30*sin(2*PI*t/10 - PI/5)"
			  "\nsampled with a frequency of 14 samples/second";
	
	int numData = 8000;
	double samplingFrecuency = 14;	

    // Filling the data series
    VectorXd timebuf(numData);
    {
	    double t = 0;
	    for (int i = 0; i < numData; ++i, t = i/samplingFrecuency) 
	       	timebuf[i] = 2*sin(2*M_PI*t/50 - M_PI/3) + 5*sin(2*M_PI*t/30 - M_PI/2) + 30*sin(2*M_PI*t/10 - M_PI/5);
    }
    
    // FFT
    VectorXcd freqbuf;
    FFT<double> fft;
    fft.SetFlag(fft.HalfSpectrum);
    fft.fwd(freqbuf, timebuf);
	
	// Filter the FFT. Frequencies between 1/25 and 1/35 Hz are removed
	// Original FFT is not changed for saving it later
	VectorXcd freqbuf2(freqbuf.size());
	{
	    for (int i = 0; i < freqbuf.size(); ++i) {
	        double freq = i*samplingFrecuency/numData;
	        double T = 1/freq;
	        if (T > 25 && T < 35)
	            freqbuf2[i] = 0;
	        else
	            freqbuf2[i] = freqbuf[i];
	    }
	}
	
	// Inverse filtered FFT to get filtered series
	VectorXd timebuf2(numData);
	fft.inv(timebuf2, freqbuf2);
	
	String csvSep = ";";

	// Saving original and filtered FFT
	{
	    String str;
	    str << "Frec" << csvSep << "T" << csvSep << "fft" << csvSep << "Filtered fft";
	    for (int i = 0; i < freqbuf.size(); ++i) {
	        double freq = i*samplingFrecuency/numData;
	        double T = 1/freq;
	        str << "\n" << freq << csvSep << (freq > 0 ? FormatDouble(T) : "") << csvSep 
	        			<< 2*std::abs(freqbuf[i])/numData << csvSep 
	        			<< 2*std::abs(freqbuf2[i])/numData;
	    }
	    String fftFileName = GetExeDirFile("fft.csv");
	    UppLog() << "\nFFT saved in '" << fftFileName << "'";
	    VERIFY(SaveFile(fftFileName, str));
	}
	
	// Saving original and filtered series
	{
	    String str;
	    str << "Time" << csvSep << "Data" << csvSep << "Filtered data";
	    double t = 0;
	    for (int i = 0; i < numData; ++i, t = i*1/samplingFrecuency) 
	       	str << "\n" << t << csvSep << timebuf[i] << csvSep << timebuf2[i];;
	    String dataFileName = GetExeDirFile("data.csv");
	    UppLog() << "\nSource data saved in '" << dataFileName << "'";
	    VERIFY(SaveFile(dataFileName, str));
    }
}

