// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2021, the Anboto author and contributors
#include <Core/Core.h>
#include <Functions4U/Functions4U.h>
#include <Eigen/Eigen.h>
#include "Utility.h"

namespace Upp {

using namespace Eigen;


double R2(const VectorXd &serie, const VectorXd &serie0, double mean) {
	if (IsNull(mean))
		mean = serie.mean();
	double sse = 0, sst = 0;
	Eigen::Index sz = min(serie.size(), serie0.size());
	for (Eigen::Index i = 0; i < sz; ++i) {
		double y = serie(i);
		double err = y - serie0(i);
		sse += err*err;
		double d = y - mean;
		sst += d*d;
	}
	if (sst < 1E-50 || sse > sst)
		return 0;
	return 1 - sse/sst;
}

}