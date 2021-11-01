// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2021, the Anboto author and contributors
#ifndef _STEM4U_Mooring_h_
#define _STEM4U_Mooring_h_

namespace Upp {

enum MooringStatus {
	LOOSE_ON_FLOOR, 
	CATENARY_ON_FLOOR, 
	CATENARY, 
	BROKEN, 
	BL_EXCEDEED
};
	
MooringStatus Catenary(double rho_m, double rho_m3, double rho_water, double moorlen, double BL,
			double xanchorvessel, double zanchor, double zvessel, 
			double &Fhanchorvessel, double &Fvanchor, double &Fvvessel, double &xonfloor,
			Vector<double> &x, Vector<double> &z, int num);

MooringStatus Catenary(double rho_m, double rho_m3, double rho_water, double moorlen, double BL,
			double xanchorvessel, double zanchor, double zvessel, 
			double &Fhanchorvessel, double &Fvanchor, double &Fvvessel, double &xonfloor);

MooringStatus Catenary(double moorlen, double xanchorvessel, double zanchor, double zvessel, double &xonfloor,
			Vector<double> &x, Vector<double> &z, int num);

MooringStatus Catenary(double moorlen, double xanchorvessel, double zanchor, double zvessel, double &xonfloor);	

MooringStatus CatenaryGetLen(double xonfloor, double xanchorvessel, double zanchor, 
			double zvessel, double &moorlen);	

MooringStatus CatenaryGetLen(double rho_m, double rho_m3, double rho_water, double xonfloor, 
			double BL, double xanchorvessel, double zanchor, double zvessel, 
			double &Fhanchorvessel, double &Fvanchor, double &Fvvessel, double &moorlen);
			  			  
const char *MooringStatusStr(MooringStatus status);

bool IsOK(MooringStatus status);
	
}


#endif
