// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2022, the Anboto author and contributors
#include "ScatterCtrl_Demo.h"


void Tab9_Big::Init()
{
	CtrlLayout(*this);	
	SizePos();
	
	int dataRange = 100000;
	
	scatter.SetRange(dataRange, 40).SetXYMin(0, -20);
	scatter.SetMouseHandling(true).SetMaxRange(500000).SetMinRange(2);
	for (int t = 0; t < dataRange; ++t) {
		s1 << Pointf(t,20*sin(2*M_PI*t/dataRange));
		s2 << Pointf(t,15*cos(2*M_PI*t/dataRange));
	}
	scatter.AddSeries(s1).Legend("series1").NoMark();
	scatter.AddSeries(s2).Legend("series2").NoMark();
	scatter.ShowAllMenus();	
	fastView.WhenAction = THISBACK(OnFastView);
	sequentialX.WhenAction = THISBACK(OnSequentialX);
	fastView = true;
	sequentialX = true;
	OnFastView();
	OnSequentialX();
}

ScatterDemo *Construct9()
{
	static Tab9_Big tab;
	return &tab;
}

INITBLOCK {
	RegisterExample("Big dataset", Construct9, __FILE__);
}