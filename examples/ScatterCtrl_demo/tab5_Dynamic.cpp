// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2022, the Anboto author and contributors
#include "ScatterCtrl_Demo.h"


void Tab5_Dynamic::Init()
{
	CtrlLayout(*this);	
	SizePos();
	
	scatter.SetRange(10, 100);
	scatter.AddSeries(s1).Legend("U-t").Units("V", "s").NoMark();
	scatter.AddSeries(s2).Legend("I-t").Units("A", "s").NoMark();
	scatter.SetFastViewX(true).SetSequentialXAll(true);
	bStart <<=(THISBACK(Start));
	bStop <<=(THISBACK(Stop));
	bReset <<=(THISBACK(Reset));
	bPgDown <<=(THISBACK(PgDown));
	bPgUp <<=(THISBACK(PgUp));
	bPlus <<=(THISBACK(Plus));
	bMinus <<=(THISBACK(Minus));
	t = 0;
	bStop.Disable();	
}

void Tab5_Dynamic::Animate()
{
	s1 << Pointf(t, 50+20*sin(t));
	s2 << Pointf(t, 50+30*cos(t));
	scatter.Refresh();
	t += 0.1;
	if((t-scatter.GetXMin()) >= scatter.GetXRange())
		scatter.SetXYMin(scatter.GetXMin() + 0.1, 0);
}

void Tab5_Dynamic::Start()
{
	SetTimeCallback(-5, THISBACK(Animate));
	bStart.Disable();
	bStop.Enable();
}

void Tab5_Dynamic::Stop()
{
	KillTimeCallback();
	bStart.Enable();
	bStop.Disable();
}

void Tab5_Dynamic::Reset()
{
	t = 0;
	s1.Clear();
	s2.Clear();
	scatter.SetXYMin(0, 0);	
	scatter.Refresh();
}

void Tab5_Dynamic::PgDown()
{
	scatter.SetXYMin(scatter.GetXMin()-5, 0);
	scatter.Refresh();
}

void Tab5_Dynamic::PgUp()
{
	scatter.SetXYMin(scatter.GetXMin()+5, 0);
	scatter.Refresh();
}

void Tab5_Dynamic::Plus()
{
	scatter.SetRange(scatter.GetXRange()/2, 100);
	scatter.Refresh();
}

void Tab5_Dynamic::Minus()
{
	scatter.SetRange(scatter.GetXRange()*2, 100);
	scatter.Refresh();
}

ScatterDemo *Construct5()
{
	static Tab5_Dynamic tab;
	return &tab;
}

INITBLOCK {
	RegisterExample("Dynamic", Construct5, __FILE__);
}