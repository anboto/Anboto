// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2022, the Anboto author and contributors
#include <Core/Core.h>
#include "Eigen.h"

namespace Upp {

using namespace Eigen;
	
bool NonLinearOptimization(VectorXd &y, Eigen::Index numData, 
			Function <int(const VectorXd &b, VectorXd &residual)> Residual,
			double xtol, double ftol, int maxfev) {
	Basic_functor functor(Residual);
	functor.unknowns = y.size();
	functor.datasetLen = numData;
	Eigen::NumericalDiff<Basic_functor> numDiff(functor);
	Eigen::LevenbergMarquardt<Eigen::NumericalDiff<Basic_functor> > lm(numDiff);
	if (!IsNull(xtol))
		lm.parameters.xtol *= xtol;
	if (!IsNull(ftol))
		lm.parameters.ftol *= ftol;
	if (!IsNull(maxfev))
		lm.parameters.maxfev = maxfev;
	int ret = lm.minimize(y);
	if (ret == Eigen::LevenbergMarquardtSpace::ImproperInputParameters || 
		ret == Eigen::LevenbergMarquardtSpace::TooManyFunctionEvaluation ||
		ret == Eigen::LevenbergMarquardtSpace::CosinusTooSmall) 
		return false;
	return true;
}

bool SolveNonLinearEquations(VectorXd &y, Function <int(const VectorXd &b, VectorXd &residual)> Residual,
			double xtol, int maxfev, double factor) {
	Basic_functor functor(Residual);
	HybridNonLinearSolver<Basic_functor> solver(functor);
	if (!IsNull(xtol))
		solver.parameters.xtol *= xtol;
	if (!IsNull(maxfev))
		solver.parameters.maxfev = maxfev;
	if (!IsNull(factor))
		solver.parameters.factor = factor;
	int ret = solver.solveNumericalDiff(y);
	if (ret == HybridNonLinearSolverSpace::ImproperInputParameters ||
	    ret == HybridNonLinearSolverSpace::TooManyFunctionEvaluation ||
	    ret == HybridNonLinearSolverSpace::NotMakingProgressJacobian ||
	    ret == HybridNonLinearSolverSpace::NotMakingProgressIterations)
		return false;
	return true;
}

double SolveNonLinearEquation(double y, Function <double(double b)> Residual, double xtol, int maxfev, double factor) {
	VectorXd x(1), res;
	x[0] = y;
	
	auto Residual2 = [&](const VectorXd &b, VectorXd &residual)->int {
		residual[0] = Residual(b[0]);
       	if (IsNull(residual[0]))
       		return 1;
       	return 0;
	};
	if (SolveNonLinearEquations(x, Residual2, xtol, maxfev, factor))
		return x[0];

	return Null;		
}

}