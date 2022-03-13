// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2022, the Anboto author and contributors
#include "trackball.h"
#include "trkball.h"

#include <GL/glu.h>

namespace Upp {
	
TrackBall::TrackBall() {
	owner = NULL;
	tracking = false;
	zoomAngle = 10;
	sign = true;
	ViewXYZ(true, true, true, sign);
}

void TrackBall::ViewXYZ(bool x, bool y, bool z, bool sign) {
	curquat[0] = curquat[1] = curquat[2] = curquat[3] = 0;
	if (x && y && z) {
		if (sign) {
			curquat[0] = 0.15;	curquat[1] = 0.35;	curquat[2] = 0.85;	curquat[3] = 0.35;
		} else {
			curquat[0] = 0.15;	curquat[1] = 0.35;	curquat[2] = 0.85;	curquat[3] = 0.35;
		}
	} else if (y && z) {
		if (sign)
			curquat[0] = curquat[1] = curquat[2] = curquat[3] = 0.5;
		else {
			curquat[0] = 0.5;	curquat[1] = -0.5;	curquat[2] = -0.5;	curquat[3] = 0.5;
		}
	} else if (x && z) {
		if (sign)
			curquat[0] = curquat[3] = sqrt(0.5);
		else
			curquat[1] = curquat[2] = sqrt(0.5);
	} else if (x && y) {
		if (sign)
			curquat[2] = curquat[3] = sqrt(0.5);
		else
			curquat[0] = curquat[1] = sqrt(0.5);
	}
}

void TrackBall::Init(Ctrl *ow, int _buttonRot) {
	owner = ow;
	buttonRot = _buttonRot;
}

void TrackBall::Matrix() {
	GLdouble m[4][4];

	build_rotmatrix(m, curquat);
	glMultMatrixd(&m[0][0]);
}

void TrackBall::Reshape(int w, int h) {
	width  = w;
	height = h;
}

void TrackBall::Zoom(int zdelta) {
	zoomAngle = min<double>(180., zoomAngle * (1 - zdelta/1000.));
}

Image TrackBall::MouseEvent(int event, Point p, int zdelta, dword ) {
	if ((event & Ctrl::ACTION) == Ctrl::MOUSEWHEEL) 
		Zoom(zdelta);
	else if ((event & Ctrl::BUTTON) == buttonRot && (event & Ctrl::ACTION) == Ctrl::DOWN) {
		tracking = true;
		beginx = p.x;
		beginy = p.y;
	} else {
		if ((event & Ctrl::BUTTON) == buttonRot && (event & Ctrl::ACTION) == Ctrl::UP)
			tracking = false;
		else if((event & Ctrl::ACTION) == Ctrl::MOUSEMOVE) {
			if (tracking) {
				trackball(
					lastquat,
					float((2.0 * beginx - width) / width),
					float((height - 2.0 * beginy) / height),
					float((2.0 * p.x - width) / width),
					float((height - 2.0 * p.y) / height)
				);
				beginx = p.x;
				beginy = p.y;
				add_quats(lastquat, curquat, curquat);
			}
		}
	}
	return Image::Hand();
}

}