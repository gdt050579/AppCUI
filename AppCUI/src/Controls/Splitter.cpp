#include "ControlContext.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;
using namespace AppCUI::Input;

#define GATTR_VERTICAL    1024
#define SPLITTER_BAR_SIZE 1

#define SPLITTER_DRAG_STATUS_NONE 0
#define SPLITTER_DRAG_STATUS_MOVE 1

constexpr int MIN_SPLITTER_SIZE = 4;

constexpr int NO_TOOLTIP_TEXT             = -1;
constexpr int SPLITTER_TOOLTIPTEXT_DRAG   = 0;
constexpr int SPLITTER_TOOLTIPTEXT_RIGHT  = 1;
constexpr int SPLITTER_TOOLTIPTEXT_BOTTOM = 2;
constexpr int SPLITTER_TOOLTIPTEXT_LEFT   = 3;
constexpr int SPLITTER_TOOLTIPTEXT_TOP    = 4;

constexpr std::string_view splitterToolTipTexts[] = { "Drag to resize panels",
                                                      "Click to maximize right panel",
                                                      "Click to maximize bottom panel",
                                                      "Click to maximize left panel",
                                                      "Click to maximize top panel" };

SplitterMouseStatus MousePozToSplitterMouseStatus(SplitterControlContext* Members, int x, int y, bool pressed)
{
    int v = 0;
    if (Members->Flags & GATTR_VERTICAL)
    {
        if (x != (Members->Layout.Width - (Members->SecondPanelSize + SPLITTER_BAR_SIZE)))
            return SplitterMouseStatus::None;
        if (Members->Layout.Height > MIN_SPLITTER_SIZE)
            v = ((y == 1) || (y == 2)) ? y : 0;
    }
    else
    {
        if (y != Members->Layout.Height - (Members->SecondPanelSize + SPLITTER_BAR_SIZE))
            return SplitterMouseStatus::None;
        if (Members->Layout.Width > MIN_SPLITTER_SIZE)
            v = ((x == 1) || (x == 2)) ? x : 0;
    }
    switch (v)
    {
    case 1:
        if (pressed)
            return SplitterMouseStatus::ClickOnButton1;
        else
            return SplitterMouseStatus::OnButton1;
    case 2:
        if (pressed)
            return SplitterMouseStatus::ClickOnButton2;
        else
            return SplitterMouseStatus::OnButton2;
    default:
        if (pressed)
            return SplitterMouseStatus::Drag;
        else
            return SplitterMouseStatus::OnBar;
    }
}
void Splitter_ResizeComponents(Splitter* control)
{
    CREATE_TYPE_CONTEXT(SplitterControlContext, control, Members, );
    Control* o;

    int sz = Members->SecondPanelSize + SPLITTER_BAR_SIZE;

    for (unsigned int tr = 0; tr < Members->ControlsCount; tr++)
    {
        o = Members->Controls[tr];
        if (o != nullptr)
        {
            if (tr >= 2)
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
                    o->SetVisible(Members->SecondPanelSize >= 1);
                }
                else
                {
                    o->MoveTo(0, Members->Layout.Height - Members->SecondPanelSize);
                    o->Resize(Members->Layout.Width, Members->SecondPanelSize);
                    o->SetVisible(Members->SecondPanelSize >= 1);
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
Splitter::Splitter(std::string_view layout, bool vertical) : Control(new SplitterControlContext(), "", layout, false)
{
    auto Members         = reinterpret_cast<SplitterControlContext*>(this->Context);
    Members->Flags       = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
    Members->mouseStatus = SplitterMouseStatus::None;
    if (vertical)
        Members->Flags |= GATTR_VERTICAL;
    if (vertical)
        Members->SecondPanelSize = Members->Layout.Width / 2;
    else
        Members->SecondPanelSize = Members->Layout.Height / 2;
}

bool Splitter::SetSecondPanelSize(int newSize)
{
    CHECK(newSize >= 0, false, "");
    CREATE_TYPECONTROL_CONTEXT(SplitterControlContext, Members, false);
    if ((Members->Flags & GATTR_VERTICAL) != 0)
    {
        if (newSize >= Members->Layout.Width)
            newSize = Members->Layout.Width - 1;
    }
    else
    {
        if (newSize >= Members->Layout.Height)
            newSize = Members->Layout.Height - 1;
    }
    if (newSize < 0)
        newSize = 0;
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
void Splitter::Paint(Graphics::Renderer& renderer)
{
    CREATE_TYPECONTROL_CONTEXT(SplitterControlContext, Members, );

    ColorPair c1  = Members->Cfg->Splitter.Buttons.Normal;
    ColorPair c2  = Members->Cfg->Splitter.Buttons.Normal;
    ColorPair col = Members->Cfg->Splitter.NormalColor;
    unsigned int poz;

    switch (Members->mouseStatus)
    {
    case SplitterMouseStatus::ClickOnButton1:
        c1 = Members->Cfg->Splitter.Buttons.Clicked;
        break;
    case SplitterMouseStatus::OnButton1:
        c1 = Members->Cfg->Splitter.Buttons.Hover;
        break;
    case SplitterMouseStatus::ClickOnButton2:
        c2 = Members->Cfg->Splitter.Buttons.Clicked;
        break;
    case SplitterMouseStatus::OnButton2:
        c2 = Members->Cfg->Splitter.Buttons.Hover;
        break;
    case SplitterMouseStatus::OnBar:
        col = Members->Cfg->Splitter.HoverColor;
        break;
    case SplitterMouseStatus::Drag:
        col = Members->Cfg->Splitter.ClickColor;
        break;
    }

    if ((Members->Flags & GATTR_VERTICAL) != 0)
    {
        poz = Members->Layout.Width - (Members->SecondPanelSize + SPLITTER_BAR_SIZE);
        renderer.DrawVerticalLine(poz, 0, Members->Layout.Height - 1, col);
        if (Members->Layout.Height > MIN_SPLITTER_SIZE)
        {
            renderer.WriteSpecialCharacter(poz, 1, SpecialChars::TriangleLeft, c1);
            renderer.WriteSpecialCharacter(poz, 2, SpecialChars::TriangleRight, c2);
        }
    }
    else
    {
        poz = Members->Layout.Height - (Members->SecondPanelSize + SPLITTER_BAR_SIZE);
        renderer.DrawHorizontalLine(0, poz, Members->Layout.Width - 1, col);
        if (Members->Layout.Width > MIN_SPLITTER_SIZE)
        {
            renderer.WriteSpecialCharacter(1, poz, SpecialChars::TriangleUp, c1);
            renderer.WriteSpecialCharacter(2, poz, SpecialChars::TriangleDown, c2);
        }
    }
}
bool Splitter::OnKeyEvent(AppCUI::Input::Key keyCode, char16_t)
{
    CREATE_TYPECONTROL_CONTEXT(SplitterControlContext, Members, false);
    if ((Members->Flags & GATTR_VERTICAL) != 0)
    {
        switch (keyCode)
        {
        case Key::Alt | Key::Ctrl | Key::Left:
            SetSecondPanelSize(Members->SecondPanelSize + SPLITTER_BAR_SIZE);
            return true;
        case Key::Alt | Key::Ctrl | Key::Right:
            SetSecondPanelSize(Members->SecondPanelSize - SPLITTER_BAR_SIZE);
            return true;
        case Key::Alt | Key::Ctrl | Key::Shift | Key::Right:
            SetSecondPanelSize(0);
            return true;
        case Key::Alt | Key::Ctrl | Key::Shift | Key::Left:
            SetSecondPanelSize(0xFFFFFF);
            return true;
        };
    }
    else
    {
        switch (keyCode)
        {
        case Key::Alt | Key::Ctrl | Key::Up:
            SetSecondPanelSize(Members->SecondPanelSize + SPLITTER_BAR_SIZE);
            return true;
        case Key::Alt | Key::Ctrl | Key::Down:
            SetSecondPanelSize(Members->SecondPanelSize - SPLITTER_BAR_SIZE);
            return true;
        case Key::Alt | Key::Ctrl | Key::Shift | Key::Down:
            SetSecondPanelSize(0);
            return true;
        case Key::Alt | Key::Ctrl | Key::Shift | Key::Up:
            SetSecondPanelSize(0xFFFFFF);
            return true;
        };
    }
    return false;
}
void Splitter::OnAfterResize(int, int)
{
    Splitter_ResizeComponents(this);
}
void Splitter::OnFocus()
{
    //Splitter_ResizeComponents(this);    ==> remove as it will cause a stack overflow if called in OnFocus method
}
bool Splitter::OnBeforeAddControl(Reference<Control> c)
{
    CHECK(c != nullptr, false, "");
    CREATE_TYPECONTROL_CONTEXT(SplitterControlContext, Members, false);
    return (Members->ControlsCount < 2);
}
void Splitter::OnMousePressed(int x, int y, AppCUI::Input::MouseButton)
{
    CREATE_TYPECONTROL_CONTEXT(SplitterControlContext, Members, );
    Members->mouseStatus = MousePozToSplitterMouseStatus(Members, x, y, true);
}
void Splitter::OnMouseReleased(int x, int y, AppCUI::Input::MouseButton)
{
    CREATE_TYPECONTROL_CONTEXT(SplitterControlContext, Members, );
    Members->mouseStatus = MousePozToSplitterMouseStatus(Members, x, y, false);
    if (Members->mouseStatus == SplitterMouseStatus::OnButton2)
        SetSecondPanelSize(0);
    if (Members->mouseStatus == SplitterMouseStatus::OnButton1)
        SetSecondPanelSize(0xFFFFFF);

    Members->mouseStatus = SplitterMouseStatus::None;
}
bool Splitter::OnMouseDrag(int x, int y, AppCUI::Input::MouseButton)
{
    CREATE_TYPECONTROL_CONTEXT(SplitterControlContext, Members, false);
    if (Members->mouseStatus == SplitterMouseStatus::Drag)
    {
        if (Members->Flags & GATTR_VERTICAL)
        {
            SetSecondPanelSize(Members->Layout.Width - (x + 1));
        }
        else
        {
            SetSecondPanelSize(Members->Layout.Height - (y + 1));
        }
        return true;
    }
    return false;
}
bool Splitter::OnMouseOver(int x, int y)
{
    CREATE_TYPECONTROL_CONTEXT(SplitterControlContext, Members, false);
    auto res = MousePozToSplitterMouseStatus(Members, x, y, false);
    if (res == Members->mouseStatus)
        return false;

    Members->mouseStatus = res;
    int toolTipTextID    = NO_TOOLTIP_TEXT;

    switch (Members->mouseStatus)
    {
    case SplitterMouseStatus::OnBar:
        toolTipTextID = SPLITTER_TOOLTIPTEXT_DRAG;
        break;
    case SplitterMouseStatus::OnButton1:
        toolTipTextID = Members->Flags & GATTR_VERTICAL ? SPLITTER_TOOLTIPTEXT_RIGHT : SPLITTER_TOOLTIPTEXT_BOTTOM;
        break;
    case SplitterMouseStatus::OnButton2:
        toolTipTextID = Members->Flags & GATTR_VERTICAL ? SPLITTER_TOOLTIPTEXT_LEFT : SPLITTER_TOOLTIPTEXT_TOP;
        break;
    case SplitterMouseStatus::None:
        HideToolTip();
        break;
    }
    if (toolTipTextID != NO_TOOLTIP_TEXT)
        ShowToolTip(splitterToolTipTexts[toolTipTextID], x, y);
    return true;
}
bool Splitter::OnMouseEnter()
{
    CREATE_TYPECONTROL_CONTEXT(SplitterControlContext, Members, false);
    Members->mouseStatus = SplitterMouseStatus::None;
    return true;
}
bool Splitter::OnMouseLeave()
{
    CREATE_TYPECONTROL_CONTEXT(SplitterControlContext, Members, false);
    Members->mouseStatus = SplitterMouseStatus::None;
    return true;
}
void Splitter::OnAfterAddControl(Reference<Control>)
{
    Splitter_ResizeComponents(this);
}
int Splitter::GetSplitterPosition()
{
    CREATE_TYPECONTROL_CONTEXT(SplitterControlContext, Members, -1);
    if ((Members->Flags & GATTR_VERTICAL) != 0)
        return Members->Layout.Width - (Members->SecondPanelSize + SPLITTER_BAR_SIZE);
    else
        return Members->Layout.Height - (Members->SecondPanelSize + SPLITTER_BAR_SIZE);
}
