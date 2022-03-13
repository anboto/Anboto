// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2022, the Anboto author and contributors
#ifndef _Controls4U_SplitterButton_h_
#define _Controls4U_SplitterButton_h_


namespace Upp {
	
class SplitterButton : public StaticRect {
public:
	typedef SplitterButton CLASSNAME;

	SplitterButton();
	virtual ~SplitterButton() {};
	
	SplitterButton& Horz(Ctrl &left, Ctrl &right);
	SplitterButton& Vert(Ctrl& top, Ctrl& bottom);
	
	SplitterButton &SetPositions(const Vector<int> &_positions);
	SplitterButton &SetPositions(int pos1);
	SplitterButton &SetPositions(int pos1, int pos2);
	SplitterButton &SetPositions(int pos1, int pos2, int pos3);
	const Vector<int> &GetPositions() const {return positions;}
	
	int GetPos()							{return splitter.GetPos();}
	
	SplitterButton &SetInitialPositionId(int id);
	
	SplitterButton &SetButtonNumber(int _buttonNumber) {
		ASSERT(_buttonNumber > 0 && _buttonNumber <= 2);
		buttonNumber = _buttonNumber;
		button2.Show(buttonNumber > 1);
		return *this;
	}
		
	SplitterButton &SetButtonWidth(int width) {buttonWidth = width; return *this;}
	void SetButton(int id);
	
private:
	bool movingRight;		
	int buttonWidth;
	Vector<int> positions;
	int positionId;
	int buttonNumber;
	
	struct SplitterLay : Splitter {
		Callback1<int> WhenLayout;
		
		virtual void Layout() {
			Splitter::Layout();
			WhenLayout(PosToClient(GetPos()));
		}
	};
	SplitterLay splitter;
	
	Button button1, button2;
	
	void OnLayout(int pos);
	void SetArrows();
};

class Box : public Splitter {
public:
	typedef Box CLASSNAME;

	Box() : Splitter() {SetStyle(StyleDefault());}

	static const Style& StyleDefault();

private:	
	virtual void   MouseMove(Point , dword ) 	{};
	virtual void   LeftDown(Point , dword ) 	{};
	virtual void   LeftUp(Point , dword ) 		{};
	virtual Image  CursorImage(Point , dword ) {return Null;};	
};

}

#endif
