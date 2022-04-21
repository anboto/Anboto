// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2022, the Anboto author and contributors
#ifndef _GLCanvas_demo_GLCanvas_demo_h
#define _GLCanvas_demo_GLCanvas_demo_h

#include <CtrlLib/CtrlLib.h>
#include <GLCanvas/GLCanvas.h>

using namespace Upp;

#define LAYOUTFILE <examples/GLCanvas_demo/GLCanvas_demo.lay>
#include <CtrlCore/lay.h>

class GLCanvas_demo : public WithGLCanvas_demoLayout<TopWindow> {
public:
	typedef GLCanvas_demo CLASSNAME;
	GLCanvas_demo();
	
private:
	Surface surf;
};

#endif
