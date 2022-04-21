// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2022, the Anboto author and contributors
#ifndef _STEM4U_Sundials_h_
#define _STEM4U_Sundials_h_

namespace Upp {
	
void SolveDAE(const Eigen::VectorXd &y, const Eigen::VectorXd &dy, double dt, double maxt, Array<Eigen::VectorXd> &res,  
		Function <bool(double t, Eigen::Index iiter, const double y[], const double dy[], double residual[])>Residual, int numZero = 0,
		Function <bool(double t, Eigen::Index iiter, const double y[], const double dy[], double residual[])>ResidualZero = Null);
void SolveDAE(const Eigen::VectorXd &y, const Eigen::VectorXd &dy, double dt, double maxt, 
		Array<Eigen::VectorXd> &res, Array<Eigen::VectorXd> &dres,  
		Function <bool(double t, Eigen::Index iiter, const double y[], const double dy[], double residual[])>Residual, int numZero = 0,
		Function <bool(double t, Eigen::Index iiter, const double y[], const double dy[], double residual[])>ResidualZero = Null);
void SolveDAE(const double y[], const double dy[], int numEq, double dt, double maxt, 
		Function <bool(double t, Eigen::Index iiter, const double y[], const double dy[], double residual[])>Residual, int numZero = 0,  
		Function <bool(double t, Eigen::Index iiter, const double y[], const double dy[], double residual[])>ResidualZero = Null,
		Function <bool(double t, Eigen::Index iiter, const double y[], const double dy[], bool isZero, int *whichZero)>OnIteration = Null);

void SolveNonLinearEquationsSun(double y[], int numEq,  
			Function <bool(const double b[], double residuals[])> Residual, int constraints[] = nullptr);

}

#endif
