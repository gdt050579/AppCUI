#include "../../include/ControlContext.h"

using namespace AppCUI::Controls;
using namespace AppCUI::Console;
using namespace AppCUI::Input;

#define GATTR_VERTICAL		1024
#define SPLITTER_BAR_SIZE   1

#define SPLITTER_DRAG_STATUS_NONE	0
#define SPLITTER_DRAG_STATUS_MOVE	1

void Splitter_ResizeComponents(Splitter *control)
{
	CREATE_TYPE_CONTEXT(SplitterControlContext, control, Members, );
	Control *o;

    int sz = Members->SecondPanelSize + SPLITTER_BAR_SIZE;

	for (unsigned int tr = 0; tr<Members->ControlsCount; tr++)
	{
		o = Members->Controls[tr];
		if (o != nullptr)
		{
			if (tr>=2) 
				o->SetVisible(false);
			if (tr == 0)
			{
				if ((Members->Flags & GATTR_VERTICAL) != 0)
				{
					o->MoveTo(0, 0); 
					o->Resize(Members->Layout.Width - sz, Members->Layout.Height);
					o->SetVisible(Members->Layout.Width > sz);
				}
				else
				{
					o->MoveTo(0, 0); 
					o->Resize(Members->Layout.Width, Members->Layout.Height - sz);
					o->SetVisible(Members->Layout.Height > sz);
				}
                continue;
			}
			if (tr == 1)
			{
				if ((Members->Flags & GATTR_VERTICAL) != 0)
				{
					o->MoveTo(Members->Layout.Width - Members->SecondPanelSize, 0);
					o->Resize(Members->SecondPanelSize, Members->Layout.Height);
					o->SetVisible(Members->SecondPanelSize > 1);
				}
				else
				{
					o->MoveTo(0, Members->Layout.Height - Members->SecondPanelSize);
					o->Resize(Members->Layout.Width, Members->SecondPanelSize);
					o->SetVisible(Members->SecondPanelSize > 1);
				}
                continue;
			}

		}
	}
}
Splitter::~Splitter()
{
	DELETE_CONTROL_CONTEXT(SplitterControlContext);
}
bool Splitter::Create(Control *parent, const char * layout, bool vertical)
{
	CONTROL_INIT_CONTEXT(SplitterControlContext);
	CHECK(Init(parent, "", layout, false), false, "Unable to create splitter !");
	CREATE_TYPECONTROL_CONTEXT(SplitterControlContext, Members, false);
	Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
    Members->DragStatus = SPLITTER_DRAG_STATUS_NONE;
	if (vertical) 
		Members->Flags |= GATTR_VERTICAL;
	if (vertical) 
		Members->SecondPanelSize = Members->Layout.Width / 2; 
	else 
		Members->SecondPanelSize = Members->Layout.Height / 2;
	return true;
}
bool Splitter::SetSecondPanelSize(int newSize)
{
	CHECK(newSize >= 0, false, "");
	CREATE_TYPECONTROL_CONTEXT(SplitterControlContext, Members, false);
	if ((Members->Flags & GATTR_VERTICAL)!=0)
	{
        if (newSize >= Members->Layout.Width)
            newSize = Members->Layout.Width - 1;
	} else
	{
        if (newSize >= Members->Layout.Height)
            newSize = Members->Layout.Height - 1;
	}
	if (newSize<0) 
		newSize=0;
	Members->SecondPanelSize = newSize;
	Splitter_ResizeComponents(this);
	return true;
}
bool Splitter::HideSecondPanel()
{
	return SetSecondPanelSize(0);
}
bool Splitter::MaximizeSecondPanel()
{
	return SetSecondPanelSize(0xFFFF);
}
void Splitter::Paint(Console::Renderer & renderer)
{
    CREATE_TYPECONTROL_CONTEXT(SplitterControlContext, Members, );

    unsigned int col = Members->Cfg->Splitter.NormalColor;
    unsigned int poz;

    if (Members->DragStatus == SPLITTER_DRAG_STATUS_MOVE)
    {
        col = Members->Cfg->Splitter.ClickColor;
    }
    else {
        if (IsMouseOver())
            col = Members->Cfg->Splitter.HoverColor;
    }

    if ((Members->Flags & GATTR_VERTICAL) != 0)
    {
        poz = Members->Layout.Width - (Members->SecondPanelSize + SPLITTER_BAR_SIZE);
        renderer.FillVerticalLineWithSpecialChar(poz, 0, Members->Layout.Height - 1, SpecialChars::BoxVerticalSingleLine, col);
    }
    else {
        poz = Members->Layout.Height - (Members->SecondPanelSize + SPLITTER_BAR_SIZE);
        renderer.FillHorizontalLineWithSpecialChar(0, poz, Members->Layout.Width - 1, SpecialChars::BoxHorizontalSingleLine, col);
    }
}
bool Splitter::OnKeyEvent(AppCUI::Input::Key::Type keyCode, char AsciiCode)
{
	CREATE_TYPECONTROL_CONTEXT(SplitterControlContext, Members, false);
	if ((Members->Flags & GATTR_VERTICAL)!=0)
	{
		switch (keyCode)
		{
			case Key::Alt |Key::Ctrl|Key::Left						: SetSecondPanelSize(Members->SecondPanelSize + SPLITTER_BAR_SIZE); return true;
			case Key::Alt |Key::Ctrl|Key::Right						: SetSecondPanelSize(Members->SecondPanelSize - SPLITTER_BAR_SIZE); return true;
			case Key::Alt |Key::Ctrl|Key::Shift|Key::Right			: SetSecondPanelSize(0); return true;
			case Key::Alt |Key::Ctrl|Key::Shift|Key::Left			: SetSecondPanelSize(0xFFFFFF); return true;
		};
	} else
	{
		switch (keyCode)
		{
			case Key::Alt |Key::Ctrl|Key::Up						: SetSecondPanelSize(Members->SecondPanelSize + SPLITTER_BAR_SIZE); return true;
			case Key::Alt |Key::Ctrl|Key::Down						: SetSecondPanelSize(Members->SecondPanelSize - SPLITTER_BAR_SIZE); return true;
			case Key::Alt |Key::Ctrl|Key::Shift|Key::Down			: SetSecondPanelSize(0); return true;
			case Key::Alt |Key::Ctrl|Key::Shift|Key::Up				: SetSecondPanelSize(0xFFFFFF); return true;
		};
	}
	return false;
}
void Splitter::OnAfterResize(int newWidth,int newHeight)
{
	Splitter_ResizeComponents(this);
}
void Splitter::OnFocus()
{
	Splitter_ResizeComponents(this);
}
bool Splitter::OnBeforeAddControl(Control *c)
{
	CHECK(c != nullptr, false, "");
	CREATE_TYPECONTROL_CONTEXT(SplitterControlContext, Members, false);
	return (Members->ControlsCount < 2);	
}
void Splitter::OnMousePressed(int x, int y, int Button)
{
    CREATE_TYPECONTROL_CONTEXT(SplitterControlContext, Members, );
    Members->DragStatus = SPLITTER_DRAG_STATUS_MOVE;
}
void Splitter::OnMouseReleased(int x, int y, int Button)
{
    CREATE_TYPECONTROL_CONTEXT(SplitterControlContext, Members, );
    Members->DragStatus = SPLITTER_DRAG_STATUS_NONE;
}
bool Splitter::OnMouseDrag(int x, int y, int Button)
{
    CREATE_TYPECONTROL_CONTEXT(SplitterControlContext, Members, false);
    if (Members->DragStatus == SPLITTER_DRAG_STATUS_MOVE)
    {
        if (Members->Flags & GATTR_VERTICAL)
        {
            SetSecondPanelSize(Members->Layout.Width - (x + 1));
        }
        else {
            SetSecondPanelSize(Members->Layout.Height - (y + 1));
        }
        return true;
    }
    return false;
}
bool Splitter::OnMouseEnter()
{
    return true;
}
bool Splitter::OnMouseLeave()
{
    return true;
}
void Splitter::OnAfterAddControl(Control *c)
{
	Splitter_ResizeComponents(this);
}
int  Splitter::GetSplitterPosition()
{
	CREATE_TYPECONTROL_CONTEXT(SplitterControlContext, Members, -1);
    if ((Members->Flags & GATTR_VERTICAL) != 0)
        return Members->Layout.Width - (Members->SecondPanelSize + SPLITTER_BAR_SIZE);
    else
        return Members->Layout.Height - (Members->SecondPanelSize + SPLITTER_BAR_SIZE);
}
