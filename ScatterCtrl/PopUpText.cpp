// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2022, the Anboto author and contributors
#include <CtrlLib/CtrlLib.h>

#include "PopUpText.h"
	
namespace Upp {
		
static Size GetEditSize(const String &_str, const Font &font) {
	WString str(_str);
	Size ret(0, 0);
	int retx = 0, nlines = 1;
	for (int i = 0; i < str.GetCount(); ++i) {
		int c = str[i];
		if (c == '\n') {
			nlines++;
			ret.cx = max(ret.cx, retx);
			retx = 0;
		} else
			retx += font.GetWidth(c);
	}
	ret.cx = max(ret.cx, retx);
	ret.cy = nlines*font.GetHeight() + font.GetDescent();
	return ret;
}

PopUpText &PopUpText::SetText(String _text) {
	text = _text;		
	sz = GetEditSize(text, font);
	return *this;
}

PopUpInfo::PopUpInfo(): color(SColorInfo()) {
	Transparent(false);
	NoWantFocus();
	IgnoreMouse();
	SetAlign(ALIGN_CENTER);
	SetFrame(BlackFrame());
	opened = false;
}

}