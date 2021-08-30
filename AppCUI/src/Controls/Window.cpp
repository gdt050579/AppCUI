#include "ControlContext.hpp"
#include "Internal.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;
using namespace AppCUI::Input;

#define WINBUTTON_STATE_NONE     0
#define WINBUTTON_STATE_CLOSE    1
#define WINBUTTON_STATE_MAXIMIZE 2
#define WINBUTTON_STATE_RESIZE   4
#define WINBUTTON_STATE_CLICKED  32

Control* FindNextControl(Control* parent, bool forward, bool startFromCurrentOne, bool rootLevel, bool noSteps)
{
    if (parent == nullptr)
        return nullptr;
    CREATE_CONTROL_CONTEXT(parent, Members, nullptr);
    // daca am copii
    if (Members->ControlsCount != 0)
    {
        int start, end;
        if (startFromCurrentOne)
            start = Members->CurrentControlIndex;
        else
            start = -1;
        if (start < 0)
        {
            if (forward)
                start = 0;
            else
                start = Members->ControlsCount - 1;
        }
        // calculez si end
        if (forward)
            end = Members->ControlsCount;
        else
            end = -1;
        // sanity check
        if (((forward) && (start >= end)) || ((!forward) && (start <= end)))
            return nullptr;
        // ma plimb intre elemente
        bool firstElement = true;
        while (true)
        {
            Control* copil = Members->Controls[start];
            if ((copil != nullptr) && (copil->Context != nullptr))
            {
                ControlContext* cMembers = (ControlContext*) copil->Context;
                // am un element posibil ok
                if (((cMembers->Flags & (GATTR_VISIBLE | GATTR_ENABLE)) == (GATTR_VISIBLE | GATTR_ENABLE)))
                {
                    Control* res = FindNextControl(
                          copil, forward, startFromCurrentOne & firstElement, false, noSteps & firstElement);
                    if (res != nullptr)
                        return res;
                }
            }

            if (forward)
                start++;
            else
                start--;
            noSteps = false;
            if (start == end)
            {
                if ((!rootLevel) || (startFromCurrentOne == false))
                    return nullptr;
                // am ajuns la finalul listei si nu am gasit sau am parcurs toata lista
                // daca nu - parcurg si restul listei
                if (forward)
                {
                    start = 0;
                    end   = Members->CurrentControlIndex + 1;
                }
                else
                {
                    start = Members->ControlsCount - 1;
                    end   = Members->CurrentControlIndex + 1;
                }
                // sanity check
                if (((forward) && (start >= end)) || ((!forward) && (start <= end)))
                    return nullptr;
                rootLevel    = false;
                firstElement = false;
            }
            firstElement = false;
        }
    }
    // daca nu am copii
    if (((Members->Flags & GATTR_TABSTOP) != 0) && (noSteps == false))
        return parent;
    return nullptr;
}
bool ProcessHotKey(Control* ctrl, AppCUI::Input::Key KeyCode)
{
    if (ctrl == nullptr)
        return false;
    CREATE_CONTROL_CONTEXT(ctrl, Members, false);
    if (((Members->Flags & (GATTR_VISIBLE | GATTR_ENABLE)) != (GATTR_VISIBLE | GATTR_ENABLE)))
        return false;
    for (unsigned int tr = 0; tr < Members->ControlsCount; tr++)
    {
        if (ProcessHotKey(Members->Controls[tr], KeyCode))
            return true;
    }
    if (ctrl->GetHotKey() == KeyCode)
    {
        ctrl->SetFocus();
        ctrl->OnHotKey();
        return true;
    }
    return false;
}
void UpdateWindowsButtonsPoz(WindowControlContext* wcc)
{
    if ((wcc->Flags & WindowFlags::NoCloseButton) != WindowFlags::NoCloseButton)
    {
        wcc->rCloseButton.Y     = 0;
        wcc->rCloseButton.Left  = wcc->Layout.Width - 4;
        wcc->rCloseButton.Right = wcc->Layout.Width - 2;
    }
    if ((wcc->Flags & WindowFlags::Sizeable) == WindowFlags::Sizeable)
    {
        wcc->rMaximizeButton.Y     = 0;
        wcc->rMaximizeButton.Left  = 1;
        wcc->rMaximizeButton.Right = 3;
        wcc->rResizeButton.Y       = wcc->Layout.Height - 1;
        wcc->rResizeButton.Left    = wcc->Layout.Width - 1;
        wcc->rResizeButton.Right   = wcc->rResizeButton.Left;
    }
    if (wcc->menu)
        wcc->menu->SetWidth(wcc->Layout.Width - 2);
}
//=========================================================================================================================================================
Window::~Window()
{
    DELETE_CONTROL_CONTEXT(WindowControlContext);
}
bool Window::Create(const AppCUI::Utils::ConstString & caption, const std::string_view& layout, WindowFlags Flags)
{
    CONTROL_INIT_CONTEXT(WindowControlContext);
    CHECK(Init(nullptr, caption, layout, false), false, "Failed to create window !");
    CHECK(SetMargins(1, 1, 1, 1), false, "Failed to set margins !");
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, false);
    Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | (unsigned int) Flags;
    Members->MinWidth =     12; // left_corner(1 char), maximize button(3chars),OneSpaceLeftPadding, 
                                // title, OneSpaceRightPadding, close
                                // button(char),right_corner(1 char) = 10+szTitle (szTitle = min 2 chars)
    Members->MinHeight      = 3;
    Members->MaxWidth       = 100000;
    Members->MaxHeight      = 100000;
    Members->Maximized      = false;
    Members->dragStatus     = WINDOW_DRAG_STATUS_NONE;
    Members->DialogResult   = -1;
    Members->winButtonState = WINBUTTON_STATE_NONE;
    UpdateWindowsButtonsPoz(Members);

    if ((Flags & WindowFlags::Maximized) == WindowFlags::Maximized)
    {
        CHECK(MaximizeRestore(), false, "Fail to maximize window !");
    }
    if ((Flags & WindowFlags::Menu) == WindowFlags::Menu)
    {
        Members->menu = std::make_unique<AppCUI::Internal::MenuBar>(this, 1, 1);
        Members->Margins.Top += 1;
        Members->menu->SetWidth(Members->Layout.Width - 2);
    }
    return true;
}
void Window::Paint(Graphics::Renderer& renderer)
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, );
    auto* wcfg = &Members->Cfg->Window;
    ColorPair colorTitle, colorWindow, colorWindowButton, c1, c2;
    bool doubleLine;

    if ((Members->Flags & WindowFlags::WarningWindow) != WindowFlags::None)
        wcfg = &Members->Cfg->DialogWarning;
    else if ((Members->Flags & WindowFlags::ErrorWindow) != WindowFlags::None)
        wcfg = &Members->Cfg->DialogError;
    else if ((Members->Flags & WindowFlags::NotifyWindow) != WindowFlags::None)
        wcfg = &Members->Cfg->DialogNotify;

    if (Members->Focused)
    {
        colorTitle        = wcfg->TitleActiveColor;
        colorWindow       = wcfg->ActiveColor;
        colorWindowButton = wcfg->ControlButtonColor;
        if (Members->dragStatus == WINDOW_DRAG_STATUS_SIZE)
        {
            colorWindow = wcfg->ControlButtonColor;
            doubleLine  = false;
        }
        else
        {
            doubleLine = true;
        }
    }
    else
    {
        colorTitle        = wcfg->TitleInactiveColor;
        colorWindow       = wcfg->InactiveColor;
        colorWindowButton = wcfg->ControlButtonInactiveColor;
        doubleLine        = false;
    }
    renderer.Clear(' ', colorWindow);
    renderer.DrawRectSize(0, 0, Members->Layout.Width, Members->Layout.Height, colorWindow, doubleLine);

    // Title
    if (Members->Layout.Width > 10)
    {
        WriteTextParams params(
              WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth | WriteTextFlags::FitTextToWidth |
              WriteTextFlags::OverwriteColors | WriteTextFlags::LeftMargin | WriteTextFlags::RightMargin,
              TextAlignament::Center);
        params.X     = 5;
        params.Y     = 0;
        params.Color = colorTitle;
        params.Width = Members->Layout.Width - 10;
        renderer.WriteText(Members->Text, params);
    }
    // close button
    if ((Members->Flags & WindowFlags::NoCloseButton) == WindowFlags::None)
    {
        switch (Members->winButtonState)
        {
        case WINBUTTON_STATE_CLOSE:
            c1 = c2 = wcfg->ControlButtonHoverColor;
            break;
        case WINBUTTON_STATE_CLOSE | WINBUTTON_STATE_CLICKED:
            c1 = c2 = wcfg->ControlButtonPressedColor;
            break;
        default:
            c1 = colorTitle;
            c2 = colorWindowButton;
            break;
        }
        renderer.WriteSingleLineText(Members->rCloseButton.Left, Members->rCloseButton.Y, "[ ]", c1);
        renderer.WriteCharacter(Members->rCloseButton.Left + 1, Members->rCloseButton.Y, 'x', c2);
    }
    // maximize button
    if ((Members->Flags & WindowFlags::Sizeable) != WindowFlags::None)
    {
        switch (Members->winButtonState)
        {
        case WINBUTTON_STATE_MAXIMIZE:
            c1 = c2 = wcfg->ControlButtonHoverColor;
            break;
        case WINBUTTON_STATE_MAXIMIZE | WINBUTTON_STATE_CLICKED:
            c1 = c2 = wcfg->ControlButtonPressedColor;
            break;
        default:
            c1 = colorTitle;
            c2 = colorWindowButton;
            break;
        }
        renderer.WriteSingleLineText(Members->rMaximizeButton.Left, Members->rMaximizeButton.Y, "[ ]", c1);
        if (Members->Maximized)
            renderer.WriteSpecialCharacter(
                  Members->rMaximizeButton.Left + 1, Members->rMaximizeButton.Y, SpecialChars::ArrowUpDown, c2);
        else
            renderer.WriteSpecialCharacter(
                  Members->rMaximizeButton.Left + 1, Members->rMaximizeButton.Y, SpecialChars::ArrowUp, c2);
        if (Members->Focused)
        {
            if (Members->dragStatus == WINDOW_DRAG_STATUS_SIZE)
                c1 = wcfg->ControlButtonPressedColor;
            else if (Members->winButtonState == WINBUTTON_STATE_RESIZE)
                c1 = wcfg->ControlButtonHoverColor;
            else
                c1 = colorWindowButton;
            renderer.WriteSpecialCharacter(
                  Members->rResizeButton.Left,
                  Members->rResizeButton.Y,
                  SpecialChars::BoxBottomRightCornerSingleLine,
                  c1);
        }
    }
    // menu
    if (Members->menu)
        Members->menu->Paint(renderer);
        
}
bool Window::MaximizeRestore()
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, false);
    if (Members->Maximized == false)
    {
        Members->oldPosX = GetX();
        Members->oldPosY = GetY();
        Members->oldW    = GetWidth();
        Members->oldH    = GetHeight();
        Size sz;
        CHECK(AppCUI::Application::GetDesktopSize(sz), false, "Fail to get desktop size");
        this->MoveTo(0, 0);
        if (this->Resize(sz.Width, sz.Height))
            Members->Maximized = true;
    }
    else
    {
        this->MoveTo(Members->oldPosX, Members->oldPosY);
        this->Resize(Members->oldW, Members->oldH);
        Members->Maximized = false;
    }
    UpdateWindowsButtonsPoz(Members);
    AppCUI::Application::RecomputeControlsLayout();
    return true;
}
bool Window::CenterScreen()
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, false);
    Size sz;
    CHECK(Application::GetDesktopSize(sz), false, "Fail to get desktop size !");
    MoveTo(((sz.Width - Members->Layout.Width) / 2), ((sz.Height - Members->Layout.Height) / 2));
    UpdateWindowsButtonsPoz(Members);
    return true;
}
void Window::OnMousePressed(int x, int y, AppCUI::Input::MouseButton button)
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, );
    Members->dragStatus = WINDOW_DRAG_STATUS_NONE;
    if (Members->menu)
    {
        if (Members->menu->OnMousePressed(x, y, button))
            return;
    }
    if (((Members->Flags & WindowFlags::NoCloseButton) == WindowFlags::None) && (Members->rCloseButton.Contains(x, y)))
    {
        Members->winButtonState = WINBUTTON_STATE_CLICKED | WINBUTTON_STATE_CLOSE;
        return;
    }
    if ((Members->Flags & WindowFlags::Sizeable) != WindowFlags::None)
    {
        if (Members->rMaximizeButton.Contains(x, y))
        {
            Members->winButtonState = WINBUTTON_STATE_CLICKED | WINBUTTON_STATE_MAXIMIZE;
            return;
        }
        if (Members->rResizeButton.Contains(x, y))
        {
            Members->dragStatus     = WINDOW_DRAG_STATUS_SIZE;
            Members->winButtonState = WINBUTTON_STATE_NONE;
            return;
        }
    }
    // if (Members->fnMousePressedHandler != nullptr)
    //{
    //	// daca vreau sa tratez eu evenimentul
    //	if (Members->fnMousePressedHandler(this, x, y, butonState,Members->fnMouseHandlerContext))
    //		return;
    //}
    if ((Members->Flags & WindowFlags::FixedPosition) == WindowFlags::None)
    {
        Members->dragStatus  = WINDOW_DRAG_STATUS_MOVE;
        Members->dragOffsetX = x;
        Members->dragOffsetY = y;
    }
}
void Window::OnMouseReleased(int x, int y, AppCUI::Input::MouseButton button)
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, );
    if (Members->dragStatus != WINDOW_DRAG_STATUS_NONE)
    {
        Members->dragStatus = WINDOW_DRAG_STATUS_NONE;
        return;
    }
    if (Members->winButtonState == (WINBUTTON_STATE_CLICKED | WINBUTTON_STATE_MAXIMIZE))
    {
        MaximizeRestore();
        return;
    }
    if (Members->winButtonState == (WINBUTTON_STATE_CLICKED | WINBUTTON_STATE_CLOSE))
    {
        RaiseEvent(Event::EVENT_WINDOW_CLOSE);
        return;
    }
    // if (Members->fnMouseReleaseHandler != nullptr)
    //{
    //	// daca vreau sa tratez eu evenimentul
    //	if (Members->fnMouseReleaseHandler(this, x, y, butonState, Members->fnMouseHandlerContext))
    //		return;
    //}
}
bool Window::OnMouseDrag(int x, int y, AppCUI::Input::MouseButton button)
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, false);
    if (Members->dragStatus == WINDOW_DRAG_STATUS_SIZE)
    {
        bool res = Resize(x + 1, y + 1);
        UpdateWindowsButtonsPoz(Members);
        return res;
    }
    if (Members->dragStatus == WINDOW_DRAG_STATUS_MOVE)
    {
        this->MoveTo(
              x + Members->ScreenClip.ScreenPosition.X - Members->dragOffsetX,
              y + Members->ScreenClip.ScreenPosition.Y - Members->dragOffsetY);
        return true;
    }
    return false;
}
bool Window::OnMouseOver(int x, int y)
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, false);
    if (Members->menu)
    {
        bool repaint;
        if (Members->menu->OnMouseMove(x, y, repaint))
        {
            HideToolTip();
            return true;
        }
    }

    if (((Members->Flags & WindowFlags::NoCloseButton) == WindowFlags::None) && (Members->rCloseButton.Contains(x, y)))
    {
        if (Members->winButtonState == WINBUTTON_STATE_CLOSE)
            return false; // suntem deja pe buton
        Members->winButtonState = WINBUTTON_STATE_CLOSE;
        ShowToolTip("Close window", (Members->rCloseButton.Left + Members->rCloseButton.Right)/2, Members->rCloseButton.Y);
        return true;
    }
    // if I reach this point - tool tip should not be shown
    HideToolTip();
    if ((Members->Flags & WindowFlags::Sizeable) != WindowFlags::None)
    {
        if (Members->rMaximizeButton.Contains(x, y))
        {
            if (Members->winButtonState == WINBUTTON_STATE_MAXIMIZE)
                return false; // suntem deja pe buton
            Members->winButtonState = WINBUTTON_STATE_MAXIMIZE;
            return true;
        }
        if (Members->rResizeButton.Contains(x, y))
        {
            if (Members->winButtonState == WINBUTTON_STATE_RESIZE)
                return false; // suntem deja pe buton
            Members->winButtonState = WINBUTTON_STATE_RESIZE;
            return true;
        }
    }
    if (Members->winButtonState == WINBUTTON_STATE_NONE)
        return false; // suntem deja in afara
    Members->winButtonState = WINBUTTON_STATE_NONE;
    return true;
}
bool Window::OnMouseLeave()
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, false);
    if (Members->winButtonState == WINBUTTON_STATE_NONE)
        return false;
    Members->winButtonState = WINBUTTON_STATE_NONE;
    return true;
}
bool Window::OnBeforeResize(int newWidth, int newHeight)
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, false);
    if ((Members->Flags & WindowFlags::Sizeable) == WindowFlags::None)
        return false;
    return (newWidth >= Members->MinWidth) && (newWidth <= Members->MaxWidth) && (newHeight >= Members->MinHeight) &&
           (newHeight <= Members->MaxHeight);
}
void Window::OnAfterResize(int newWidth, int newHeight)
{
    WindowControlContext* Members = (WindowControlContext*) this->Context;
    if (Members)
    {
        UpdateWindowsButtonsPoz(Members);
    }
}
bool Window::OnKeyEvent(AppCUI::Input::Key KeyCode, char16_t UnicodeChar)
{
    Control* tmp;
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, false);

    switch (KeyCode)
    {
    case Key::Tab | Key::Shift:
        tmp = FindNextControl(this, false, true, true, true);
        if (tmp != nullptr)
            tmp->SetFocus();
        return true;
    case Key::Tab:
        tmp = FindNextControl(this, true, true, true, true);
        if (tmp != nullptr)
            tmp->SetFocus();
        return true;
    case Key::Escape:
        RaiseEvent(Event::EVENT_WINDOW_CLOSE);
        return true;
    case Key::Enter:
        RaiseEvent(Event::EVENT_WINDOW_ACCEPT);
        return true;
    }
    // first we check menu hot keys
    if (Members->menu)
    {
        if (Members->menu->OnKeyEvent(KeyCode))
            return true;
    }        
    // check cntrols hot keys
    if ((((unsigned int) KeyCode) & (unsigned int) (Key::Shift | Key::Alt | Key::Ctrl)) == ((unsigned int) Key::Alt))
    {
        if (ProcessHotKey(this, KeyCode))
            return true;
    }
    // key was not prcessed, pass it to my parent
    return false;
}
bool Window::Exit(int dialogResult)
{
    CHECK(dialogResult >= 0, false, "Dialog result code must be bigger than 0 !");
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, false);
    Members->DialogResult                             = dialogResult;
    AppCUI::Application::GetApplication()->LoopStatus = LOOP_STATUS_STOP_CURRENT;
    return true;
}
bool Window::Exit(Dialogs::Result dialogResult)
{
    return this->Exit(static_cast<int>(dialogResult));
}
int  Window::Show()
{
    CHECK(GetParent() == nullptr, -1, "Unable to run modal window if it is attached to another control !");
    CHECK(AppCUI::Application::GetApplication()->ExecuteEventLoop(this), -1, "Modal execution failed !");
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, -1);
    return Members->DialogResult;
}
int  Window::GetDialogResult()
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, -1);
    return Members->DialogResult;
}
bool Window::IsWindowInResizeMode()
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, false);
    return (Members->dragStatus == WINDOW_DRAG_STATUS_SIZE);
}

Menu* Window::AddMenu(const AppCUI::Utils::ConstString& name)
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, nullptr);
    CHECK(Members->menu, nullptr, "Application was not initialized with Menu option set up !");
    ItemHandle itm                 = Members->menu->AddMenu(name);
    AppCUI::Controls::Menu* result = Members->menu->GetMenu(itm);
    CHECK(result, nullptr, "Fail to create menu !");
    return result;
}