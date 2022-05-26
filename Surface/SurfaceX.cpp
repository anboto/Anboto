// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2022, the Anboto author and contributors
#include <Core/Core.h>

#include <Eigen/Eigen.h>
#include <Surface/Surface.h>
#include <Geom/Geom.h>
#include <Functions4U/Functions4U.h>
#include <numeric> 

namespace Upp {
using namespace Eigen;

void SurfaceX::Load(const Surface &p) {
	parent = &p;
	
	surfaces.resize(p.panels.size());
	centroids.resize(p.panels.size(), 3);
	normals.resize(p.panels.size(), 3);
	
	for (int ip = 0; ip < p.panels.size(); ++ip) {
		const Panel &pan = p.panels[ip];
		surfaces(ip) = pan.surface0 + pan.surface1;
		centroids(ip, 0) = pan.centroidPaint.x;
		centroids(ip, 1) = pan.centroidPaint.y;
		centroids(ip, 2) = pan.centroidPaint.z;
		normals(ip, 0) = pan.normalPaint.x;
		normals(ip, 1) = pan.normalPaint.y;
		normals(ip, 2) = pan.normalPaint.z;
	}
}

double SurfaceX::GetVolume(const SurfaceX &surf, double &vx, double &vy, double &vz) {
	vx = vy = vz = 0;
	
	for (int ip = 0; ip < surf.surfaces.size(); ++ip) {
		const double &area = surf.surfaces[ip];
		
		vx += area*surf.normals(ip, 0)*surf.centroids(ip, 0);
		vy += area*surf.normals(ip, 1)*surf.centroids(ip, 1);
		vz += area*surf.normals(ip, 2)*surf.centroids(ip, 2);
	}
	return avg(vx, vy, vz);
}

double SurfaceX::GetVolume(const SurfaceX &surf) {
	double vz = 0;
	
	for (int ip = 0; ip < surf.surfaces.size(); ++ip) 
		vz += surf.surfaces[ip]*surf.normals(ip, 2)*surf.centroids(ip, 2);
	return vz;
}

double SurfaceX::GetSurface(const SurfaceX &surf) {
	return surf.surfaces.sum();
}

double SurfaceX::GetSurface(const SurfaceX &surf, bool (*Fun)(double,double,double)) {
	double area = 0;
	for (int ip = 0; ip < surf.surfaces.size(); ++ip) {
		if (Fun(surf.centroids(ip, 0), surf.centroids(ip, 1), surf.centroids(ip, 2)))
			area += surf.surfaces(ip);
	}
	return area;
}

double SurfaceX::GetVolume(const SurfaceX &surf, bool (*Fun)(double,double,double)) {
	double vz = 0;
	for (int ip = 0; ip < surf.surfaces.size(); ++ip) {
		if (Fun(surf.centroids(ip, 0), surf.centroids(ip, 1), surf.centroids(ip, 2)))
			vz += surf.surfaces[ip]*surf.normals(ip, 2)*surf.centroids(ip, 2);
	}
	return vz;
}

void SurfaceX::GetTransformFast(MatrixXd &mat, double dx, double dy, double dz, double ax, double ay, double az) {
    double ax2 = sqr(ax);
    double ay2 = sqr(ay);
    double az2 = sqr(az);
    double sq_o1o2o3 = ax2 + ay2 + az2;
    double sqrt_o1o2o3 = sqrt(1 + sq_o1o2o3);

	MatrixXd rotMat = MatrixXd::Zero(4, 4);
    rotMat(0,0) = ax2*sqrt_o1o2o3 + ay2 + az2;
    rotMat(1,0) = az*sq_o1o2o3 + ax*ay*(sqrt_o1o2o3-1);
    rotMat(2,0) = -ay*sq_o1o2o3 + ax*az*(sqrt_o1o2o3-1);
    rotMat(0,1) = -az*sq_o1o2o3 + ax*ay*(sqrt_o1o2o3-1);
    rotMat(1,1) = ax2 + ay2*sqrt_o1o2o3 + az2;
    rotMat(2,1) = ax*sq_o1o2o3 + ay*az*(sqrt_o1o2o3-1);
    rotMat(0,2) = ay*sq_o1o2o3 + ax*az*(sqrt_o1o2o3-1);
    rotMat(1,2) = -ax*sq_o1o2o3 + ay*az*(sqrt_o1o2o3-1);
    rotMat(2,2) = ax2 + ay2 + az2*sqrt_o1o2o3;
    if (sq_o1o2o3 != 0)
        rotMat *= (1/(sq_o1o2o3*sqrt_o1o2o3));
    rotMat(3,3) = 1;
    
    mat = MatrixXd::Zero(4, 4);
    mat(0,0) = mat(1,1) = mat(2,2) = mat(3,3) = 1;
    mat(0,3) = dx;
    mat(1,3) = dy;
    mat(2,3) = dz;
    
    if (sq_o1o2o3 != 0)	// If True, the rotation matrix has a division by 0 (means no rotation happened)
        mat *= rotMat;
}

void SurfaceX::TransRotFast(double &x, double &y, double &z, double x0, double y0, double z0, const MatrixXd &mat) {
    Vector4d position = {x0, y0, z0, 1};
    
	position = mat*position;
	x = position[0];
	y = position[1];
	z = position[2];
}

void SurfaceX::TransRotFast(double &x, double &y, double &z, double x0, double y0, double z0,
					 double dx, double dy, double dz, double ax, double ay, double az) {
	MatrixXd mat;		
	GetTransformFast(mat, dx, dy, dz, ax, ay, az);
	TransRotFast(x, y, z, x0, y0, z0, mat);
}

void SurfaceX::GetTransform(Affine3d &aff, double dx, double dy, double dz, double ax, double ay, double az) {
	Vector3d d(dx, dy, dz);	
	aff = Translation3d(d) *
		  AngleAxisd(ax, Vector3d::UnitX()) *
		  AngleAxisd(ay, Vector3d::UnitY()) *
		  AngleAxisd(az, Vector3d::UnitZ());
}

void SurfaceX::TransRot(double &x, double &y, double &z, double x0, double y0, double z0, const Affine3d &quat) {
	Vector3d pnt0(x0, y0, z0);	
	Vector3d pnt = quat * pnt0;

	x = pnt[0];
	y = pnt[1];
	z = pnt[2];
}

void SurfaceX::TransRot(SurfaceX &surf, const SurfaceX &surf0, const Affine3d &quat) {
	surf.centroids = quat * surf0.centroids.colwise().homogeneous();
	surf.normals = quat * surf0.normals.colwise().homogeneous();
}

void SurfaceX::TransRot(double &x, double &y, double &z, double x0, double y0, double z0,
					 double dx, double dy, double dz, double ax, double ay, double az) {
	Affine3d aff;		
	GetTransform(aff, dx, dy, dz, ax, ay, az);
	TransRot(x, y, z, x0, y0, z0, aff);
}
	
	
}