#include "ControlContext.hpp"
#include "Internal.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;
using namespace AppCUI::Input;

constexpr unsigned char WINBUTTON_NONE = 0xFF;
constexpr unsigned int MAX_TAG_CHARS   = 8U;

struct WindowControlBarLayoutData
{
    int Left, Right, Y;
    WindowButton* LeftGroup;
    WindowButton* RighGroup;
};

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
void UpdateWindowButtonPos(WindowButton* b, WindowControlBarLayoutData& layout, bool fromLeft)
{
    int next;

    bool partOfGroup = (b->Type == WindowButtonType::Button) | (b->Type == WindowButtonType::Radio) |
                       (b->Type == WindowButtonType::CheckBox) | (b->Type == WindowButtonType::Text);
    WindowButton* group = nullptr;
    int extraX          = 0;

    if (fromLeft)
        group = layout.LeftGroup;
    else
        group = layout.RighGroup;

    // analyze current group
    if (partOfGroup)
    {
        if (group)
        {
            if (group->Type != b->Type)
            {
                if (fromLeft)
                    group->SetFlag(WindowButtonFlags::RightGroupMarker); // new group, close previous one
                else
                    group->SetFlag(WindowButtonFlags::LeftGroupMarker); // new group, close previous one
                group  = nullptr;
                extraX = 2;
            }
        }
        else
            extraX = 1;
    }
    else
    {
        if (group)
        {
            if (fromLeft)
                group->SetFlag(WindowButtonFlags::RightGroupMarker); // close previous one
            else
                group->SetFlag(WindowButtonFlags::LeftGroupMarker); // close previous one
            group = nullptr;
        }
    }
    if (fromLeft)
        layout.LeftGroup = group;
    else
        layout.RighGroup = group;

    b->Y = layout.Y;
    if (fromLeft)
    {
        b->X = layout.Left + extraX;
        next = b->X + b->Size + 1;
        if (next < layout.Right)
        {
            b->SetFlag(WindowButtonFlags::Visible);
            layout.Left = next;
            if (partOfGroup)
            {
                if (layout.LeftGroup == nullptr)
                    b->SetFlag(WindowButtonFlags::LeftGroupMarker);
                else
                    b->RemoveFlag(WindowButtonFlags::LeftGroupMarker);
                layout.LeftGroup = b;
            }
        }
    }
    else
    {
        b->X = layout.Right - b->Size + 1;
        b->X -= extraX;
        next = b->X - 2;
        if (next > layout.Left)
        {
            b->SetFlag(WindowButtonFlags::Visible);
            layout.Right = next;
            if (partOfGroup)
            {
                if (layout.RighGroup == nullptr)
                    b->SetFlag(WindowButtonFlags::RightGroupMarker);
                else
                    b->RemoveFlag(WindowButtonFlags::RightGroupMarker);
                layout.RighGroup = b;
            }
        }
    }
}
void UpdateWindowsButtonsPoz(WindowControlContext* wcc)
{
    for (unsigned int tr = 0; tr < wcc->WinButtonsCount; tr++)
        wcc->WinButtons[tr].RemoveFlag(WindowButtonFlags::Visible);

    WindowControlBarLayoutData top, bottom;
    top.Left         = 1;
    bottom.Left      = 1;
    top.Y            = 0;
    bottom.Y         = wcc->Layout.Height - 1;
    top.Right        = wcc->Layout.Width - 2;
    bottom.Right     = wcc->Layout.Width - 1;
    top.LeftGroup    = nullptr;
    top.RighGroup    = nullptr;
    bottom.LeftGroup = nullptr;
    bottom.RighGroup = nullptr;

    auto* btn = wcc->WinButtons;
    for (unsigned int tr = 0; tr < wcc->WinButtonsCount; tr++, btn++)
    {
        if (btn->IsHidden())
            continue;
        switch (btn->Layout)
        {
        case WindowControlsBarLayout::TopBarFromLeft:
            UpdateWindowButtonPos(btn, top, true);
            break;
        case WindowControlsBarLayout::TopBarFromRight:
            UpdateWindowButtonPos(btn, top, false);
            break;
        case WindowControlsBarLayout::BottomBarFromLeft:
            UpdateWindowButtonPos(btn, bottom, true);
            break;
        case WindowControlsBarLayout::BottomBarFromRight:
            UpdateWindowButtonPos(btn, bottom, false);
            break;
        }
    }
    // group flags
    if (top.LeftGroup)
        top.LeftGroup->SetFlag(WindowButtonFlags::RightGroupMarker);
    if (top.RighGroup)
        top.RighGroup->SetFlag(WindowButtonFlags::LeftGroupMarker);
    if (bottom.LeftGroup)
        bottom.LeftGroup->SetFlag(WindowButtonFlags::RightGroupMarker);
    if (bottom.RighGroup)
        bottom.RighGroup->SetFlag(WindowButtonFlags::LeftGroupMarker);

    if (wcc->menu)
        wcc->menu->SetWidth(wcc->Layout.Width - 2);
}
void WindowRadioButtonClicked(WindowButton* start, WindowButton* end, WindowButton* current)
{
    // go back and disable check
    auto p = current;
    while (p >= start)
    {
        if (p->Layout == current->Layout)
        {
            if (p->Type == WindowButtonType::Radio)
                p->RemoveFlag(WindowButtonFlags::Checked);
            else
                break;
        }
        p--;
    }
    p = current;
    while (p < end)
    {
        if (p->Layout == current->Layout)
        {
            if (p->Type == WindowButtonType::Radio)
                p->RemoveFlag(WindowButtonFlags::Checked);
            else
                break;
        }
        p++;
    }
    current->SetFlag(WindowButtonFlags::Checked);
}
//=========================================================================================================================================================
ItemHandle AppCUI::Controls::WindowControlsBar::AddCommandItem(
      const AppCUI::Utils::ConstString& name, int ID, const AppCUI::Utils::ConstString& toolTip)
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, InvalidItemHandle);
    CHECK(Members->WinButtonsCount < MAX_WINDOWBAR_ITEMS,
          InvalidItemHandle,
          "Max number of items in a control bar was exceeded !");
    auto* b = &Members->WinButtons[Members->WinButtonsCount];
    CHECK(b->Init(WindowButtonType::Button, this->Layout, name, toolTip),
          InvalidItemHandle,
          "Fail to initialize item !");
    b->ID = ID;
    Members->WinButtonsCount++;
    UpdateWindowsButtonsPoz(Members);
    return Members->WinButtonsCount-1;
}
ItemHandle AppCUI::Controls::WindowControlsBar::AddRadioItem(
      const AppCUI::Utils::ConstString& name, int ID, bool checked, const AppCUI::Utils::ConstString& toolTip)
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, InvalidItemHandle);
    CHECK(Members->WinButtonsCount < MAX_WINDOWBAR_ITEMS,
          InvalidItemHandle,
          "Max number of items in a control bar was exceeded !");
    auto* b = &Members->WinButtons[Members->WinButtonsCount];
    CHECK(b->Init(WindowButtonType::Radio, this->Layout, name, toolTip),
          InvalidItemHandle,
          "Fail to initialize item !");
    b->ID = ID;
    Members->WinButtonsCount++;
    if (checked)
        WindowRadioButtonClicked(
              Members->WinButtons,
              Members->WinButtons + Members->WinButtonsCount,
              Members->WinButtons + Members->WinButtonsCount - 1);
    UpdateWindowsButtonsPoz(Members);
    return Members->WinButtonsCount - 1;
}
ItemHandle AppCUI::Controls::WindowControlsBar::AddCheckItem(
      const AppCUI::Utils::ConstString& name, int ID, bool checked, const AppCUI::Utils::ConstString& toolTip)
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, InvalidItemHandle);
    CHECK(Members->WinButtonsCount < MAX_WINDOWBAR_ITEMS,
          InvalidItemHandle,
          "Max number of items in a control bar was exceeded !");
    auto* b = &Members->WinButtons[Members->WinButtonsCount];
    CHECK(b->Init(WindowButtonType::CheckBox, this->Layout, name, toolTip),
          InvalidItemHandle,
          "Fail to initialize item !");
    b->ID = ID;
    Members->WinButtonsCount++;
    if (checked)
        b->SetFlag(WindowButtonFlags::Checked);
    else
        b->RemoveFlag(WindowButtonFlags::Checked);
    UpdateWindowsButtonsPoz(Members);
    return Members->WinButtonsCount - 1;
}
ItemHandle AppCUI::Controls::WindowControlsBar::AddTextItem(
      const AppCUI::Utils::ConstString& caption, const AppCUI::Utils::ConstString& toolTip)
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, InvalidItemHandle);
    CHECK(Members->WinButtonsCount < MAX_WINDOWBAR_ITEMS,
          InvalidItemHandle,
          "Max number of items in a control bar was exceeded !");
    auto* b = &Members->WinButtons[Members->WinButtonsCount];
    CHECK(b->Init(WindowButtonType::Text, this->Layout, caption, toolTip),
          InvalidItemHandle,
          "Fail to initialize item !");

    Members->WinButtonsCount++;
    UpdateWindowsButtonsPoz(Members);
    return Members->WinButtonsCount - 1;
}
WindowButton* GetWindowControlsBarItem(void * Context, ItemHandle itemHandle)
{                             
    WindowControlContext* Members = (WindowControlContext*) Context;
    CHECK(Members, nullptr, "");
    unsigned int id = (unsigned int) itemHandle;
    CHECK(id < Members->WinButtonsCount, nullptr, "Invalid item index (%d/%d)", id, Members->WinButtonsCount);
    auto* b = Members->WinButtons + id;
    CHECK((b->Type == WindowButtonType::Button) || (b->Type == WindowButtonType::CheckBox) ||
                (b->Type == WindowButtonType::Radio) || (b->Type == WindowButtonType::Text),
          nullptr,
          "");
    return b;
}
bool AppCUI::Controls::WindowControlsBar::SetItemText(ItemHandle itemHandle, const AppCUI::Utils::ConstString& caption)
{
    auto b = GetWindowControlsBarItem(this->Context, itemHandle);
    CHECK(b, false, "");
    CHECK(b->Text.Set(caption), false, "");
    b->Size = b->Text.Len();
    if (b->Type == WindowButtonType::CheckBox)
        b->Size += 2;
    UpdateWindowsButtonsPoz((WindowControlContext*) Context);
    return true;
}
bool AppCUI::Controls::WindowControlsBar::SetItemToolTip(ItemHandle itemHandle, const AppCUI::Utils::ConstString& caption)
{
    auto b = GetWindowControlsBarItem(this->Context, itemHandle);
    CHECK(b, false, "");
    CHECK(b->ToolTipText.Set(caption), false, "");
    return true;
}
//=========================================================================================================================================================
bool WindowButton::Init(
      WindowButtonType type, WindowControlsBarLayout layout, unsigned char size, std::string_view toolTipText)
{
    this->Type         = type;
    this->Layout       = layout;
    this->Size         = size;
    this->X            = 0;
    this->Y            = 0;
    this->Flags        = WindowButtonFlags::None;
    this->ID           = -1;
    this->HotKey       = Key::None;
    this->HotKeyOffset = CharacterBuffer::INVALID_HOTKEY_OFFSET;
    if (!toolTipText.empty())
    {
        CHECK(this->ToolTipText.Set(toolTipText), false, "");
    }
    return true;
}
bool WindowButton::Init(
      WindowButtonType type,
      WindowControlsBarLayout layout,
      const AppCUI::Utils::ConstString& name,
      const AppCUI::Utils::ConstString& toolTip)
{
    this->Type         = type;
    this->Layout       = layout;
    this->Size         = 0;
    this->X            = 0;
    this->Y            = 0;
    this->Flags        = WindowButtonFlags::None;
    this->ID           = -1;
    this->HotKey       = Key::None;
    this->HotKeyOffset = CharacterBuffer::INVALID_HOTKEY_OFFSET;
    // name
    AppCUI::Utils::ConstStringObject objName(name);
    CHECK(objName.Length > 0, false, "Expecting a valid item name (non-empty)");
    CHECK(this->Text.SetWithHotKey(name, this->HotKeyOffset, this->HotKey, Key::Alt), false, "Fail to create name !");
    this->Size = this->Text.Len();
    if (type == WindowButtonType::CheckBox)
        this->Size += 2; // for the checkmark
    // tool tip
    AppCUI::Utils::ConstStringObject objToolTip(toolTip);
    if (objToolTip.Length > 0)
    {
        CHECK(this->ToolTipText.Set(toolTip), false, "");
    }
    // all good
    return true;
}
//=========================================================================================================================================================
Window::~Window()
{
    DELETE_CONTROL_CONTEXT(WindowControlContext);
}
bool Window::Create(const AppCUI::Utils::ConstString& caption, const std::string_view& layout, WindowFlags Flags)
{
    CONTROL_INIT_CONTEXT(WindowControlContext);
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, false);
    Members->Layout.MaxHeight = 200000;
    Members->Layout.MaxWidth  = 200000;
    Members->Layout.MinHeight = 3;
    Members->Layout.MinWidth  = 12; // left_corner(1 char), maximize button(3chars),OneSpaceLeftPadding,
                                    // title, OneSpaceRightPadding, close
                                    // button(char),right_corner(1 char) = 10+szTitle (szTitle = min 2 chars)
    CHECK(Init(nullptr, caption, layout, false), false, "Failed to create window !");
    CHECK(SetMargins(1, 1, 1, 1), false, "Failed to set margins !");
    Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | (unsigned int) Flags;

    Members->Maximized               = false;
    Members->dragStatus              = WINDOW_DRAG_STATUS_NONE;
    Members->DialogResult            = -1;
    Members->CurrentWinButtom        = WINBUTTON_NONE;
    Members->CurrentWinButtomPressed = false;
    Members->WinButtonsCount         = 0;

    // init the buttons
    if ((Flags & WindowFlags::NoCloseButton) == WindowFlags::None)
    {
        Members->WinButtons[Members->WinButtonsCount++].Init(
              WindowButtonType::CloseButton, WindowControlsBarLayout::TopBarFromRight, 3, "Close window");
    }
    if ((Flags & WindowFlags::Sizeable) != WindowFlags::None)
    {
        Members->WinButtons[Members->WinButtonsCount++].Init(
              WindowButtonType::MaximizeRestoreButton,
              WindowControlsBarLayout::TopBarFromLeft,
              3,
              "Maximize or restore the size of this window");
        Members->WinButtons[Members->WinButtonsCount++].Init(
              WindowButtonType::WindowResize,
              WindowControlsBarLayout::BottomBarFromRight,
              1,
              "Click and drag to resize this window");
    }
    // hot key
    Members->WinButtons[Members->WinButtonsCount].Init(
          WindowButtonType::HotKeY,
          WindowControlsBarLayout::TopBarFromRight,
          3,
          "Press Alt+xx to switch to this window");
    // the button exists but it is hidden
    Members->WinButtons[Members->WinButtonsCount].SetFlag(WindowButtonFlags::Hidden);
    Members->WinButtonsCount++;

    // TAG
    Members->WinButtons[Members->WinButtonsCount].Init(
          WindowButtonType::Tag, WindowControlsBarLayout::TopBarFromLeft, 3, "");
    // the button exists but it is hidden
    Members->WinButtons[Members->WinButtonsCount].SetFlag(WindowButtonFlags::Hidden);
    Members->WinButtonsCount++;

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
    ColorPair colorTitle, colorWindow, c1, c2;
    bool doubleLine;

    if ((Members->Flags & WindowFlags::WarningWindow) != WindowFlags::None)
        wcfg = &Members->Cfg->DialogWarning;
    else if ((Members->Flags & WindowFlags::ErrorWindow) != WindowFlags::None)
        wcfg = &Members->Cfg->DialogError;
    else if ((Members->Flags & WindowFlags::NotifyWindow) != WindowFlags::None)
        wcfg = &Members->Cfg->DialogNotify;

    auto* c_i     = &wcfg->ControlBar.Item.Normal;
    auto sepColor = wcfg->ControlBar.Separators.Normal;

    if (Members->Focused)
    {
        sepColor    = wcfg->ControlBar.Separators.Focused;
        colorTitle  = wcfg->TitleActiveColor;
        colorWindow = wcfg->ActiveColor;
        doubleLine  = Members->dragStatus != WINDOW_DRAG_STATUS_SIZE;
    }
    else
    {
        colorTitle  = wcfg->TitleInactiveColor;
        colorWindow = wcfg->InactiveColor;
        doubleLine  = false;
    }
    renderer.Clear(' ', colorWindow);
    renderer.DrawRectSize(0, 0, Members->Layout.Width, Members->Layout.Height, colorWindow, doubleLine);

    auto* btn = Members->WinButtons;
    for (unsigned int tr = 0; tr < Members->WinButtonsCount; tr++, btn++)
    {
        if ((!btn->IsVisible()) || (btn->IsHidden()))
            continue;
        bool fromLeft = (btn->Layout == WindowControlsBarLayout::TopBarFromLeft) ||
                        (btn->Layout == WindowControlsBarLayout::BottomBarFromLeft);
        bool showChecked = false;
        if (Members->CurrentWinButtom == tr)
        {
            // hover or pressed
            if (Members->CurrentWinButtomPressed)
                c_i = &wcfg->ControlBar.Item.Pressed;
            else
            {
                showChecked = ((Members->Focused) && (btn->IsChecked()));
                c_i         = &wcfg->ControlBar.Item.Hover;
            }
        }
        else
        {
            if (Members->Focused)
            {
                showChecked = btn->IsChecked();
                c_i         = &wcfg->ControlBar.Item.Focused;
            }
            else
                c_i = &wcfg->ControlBar.Item.Normal;
        }
        bool hoverOrPressed = (c_i == &wcfg->ControlBar.Item.Hover) || (c_i == &wcfg->ControlBar.Item.Pressed);
        bool drawSeparators = false;
        switch (btn->Type)
        {
        case WindowButtonType::CloseButton:
            if (hoverOrPressed)
            {
                c1 = c2 = c_i->Text;
            }
            else
            {
                c1 = sepColor;
                c2 = Members->Focused ? wcfg->ControlBar.CloseButton : c_i->Text;
            }
            renderer.WriteSingleLineText(btn->X, btn->Y, "[ ]", c1);
            renderer.WriteCharacter(btn->X + 1, btn->Y, 'x', c2);
            break;
        case WindowButtonType::MaximizeRestoreButton:
            if (hoverOrPressed)
                renderer.WriteSingleLineText(btn->X, btn->Y, "[ ]", c_i->Text);
            else
                renderer.WriteSingleLineText(btn->X, btn->Y, "[ ]", sepColor);
            if (Members->Maximized)
                renderer.WriteSpecialCharacter(btn->X + 1, btn->Y, SpecialChars::ArrowUpDown, c_i->Text);
            else
                renderer.WriteSpecialCharacter(btn->X + 1, btn->Y, SpecialChars::ArrowUp, c_i->Text);
            break;
        case WindowButtonType::WindowResize:
            if (Members->Focused)
                renderer.WriteSpecialCharacter(btn->X, btn->Y, SpecialChars::BoxBottomRightCornerSingleLine, c_i->Text);
            break;
        case WindowButtonType::HotKeY:
            renderer.WriteCharacter(btn->X, btn->Y, '[', sepColor);
            c1 = Members->Focused ? wcfg->ControlBar.Item.Focused.Text : wcfg->ControlBar.Item.Normal.Text;
            renderer.WriteSingleLineText(btn->X + 1, btn->Y, KeyUtils::GetKeyName(Members->HotKey), c1);
            renderer.WriteCharacter(btn->X + btn->Size - 1, btn->Y, ']', sepColor);
            break;
        case WindowButtonType::Tag:
            renderer.WriteCharacter(btn->X, btn->Y, '[', sepColor);
            c1 = Members->Focused ? wcfg->ControlBar.Tag : wcfg->ControlBar.Item.Normal.Text;
            renderer.WriteSingleLineText(btn->X + 1, btn->Y, btn->Text, c1);
            renderer.WriteCharacter(btn->X + btn->Size - 1, btn->Y, ']', sepColor);
            break;

        case WindowButtonType::Button:
        case WindowButtonType::Radio:
            if (showChecked)
                renderer.WriteSingleLineText(
                      btn->X,
                      btn->Y,
                      btn->Text,
                      wcfg->ControlBar.Item.Checked.Text,
                      wcfg->ControlBar.Item.Checked.HotKey,
                      btn->HotKeyOffset);
            else
                renderer.WriteSingleLineText(btn->X, btn->Y, btn->Text, c_i->Text, c_i->HotKey, btn->HotKeyOffset);
            drawSeparators = true;
            break;
        case WindowButtonType::CheckBox:
            renderer.FillHorizontalLine(btn->X, btn->Y, btn->X + 1, ' ', c_i->HotKey);
            renderer.WriteSingleLineText(btn->X + 2, btn->Y, btn->Text, c_i->Text, c_i->HotKey, btn->HotKeyOffset);
            if (btn->IsChecked())
            {
                c1 = Members->Focused & (!hoverOrPressed) ? wcfg->ControlBar.CheckMark : c_i->Text;
                renderer.WriteSpecialCharacter(btn->X, btn->Y, SpecialChars::CheckMark, c1);
            }
            drawSeparators = true;
            break;
        case WindowButtonType::Text:
            c1 = Members->Focused ? wcfg->ControlBar.Text : wcfg->ControlBar.Item.Normal.Text;
            renderer.WriteSingleLineText(btn->X, btn->Y, btn->Text, c1);
            drawSeparators = true;
            break;
        }
        // separators
        if (drawSeparators)
        {
            if ((unsigned char) btn->Flags & (unsigned char) WindowButtonFlags::LeftGroupMarker)
                renderer.WriteCharacter(btn->X - 1, btn->Y, '[', sepColor);
            else if (fromLeft)
                renderer.WriteCharacter(btn->X - 1, btn->Y, '|', sepColor);
            if ((unsigned char) btn->Flags & (unsigned char) WindowButtonFlags::RightGroupMarker)
                renderer.WriteCharacter(btn->X + btn->Size, btn->Y, ']', sepColor);
            else if (!fromLeft)
                renderer.WriteCharacter(btn->X + btn->Size, btn->Y, '|', sepColor);
        }
    }

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
    Members->dragStatus              = WINDOW_DRAG_STATUS_NONE;
    Members->CurrentWinButtomPressed = false;

    if (Members->menu)
    {
        if (Members->menu->OnMousePressed(x, y, button))
            return;
    }

    // win buttons
    Members->CurrentWinButtom = WINBUTTON_NONE;
    for (unsigned int tr = 0; tr < Members->WinButtonsCount; tr++)
    {
        if (Members->WinButtons[tr].Contains(x, y))
        {
            Members->CurrentWinButtom        = tr; // set current button
            Members->CurrentWinButtomPressed = true;
            if (Members->WinButtons[tr].Type == WindowButtonType::WindowResize)
                Members->dragStatus = WINDOW_DRAG_STATUS_SIZE;
            return;
        }
    }
    // Hide tool tip
    HideToolTip();

    if ((Members->Flags & WindowFlags::FixedPosition) == WindowFlags::None)
    {
        Members->dragStatus  = WINDOW_DRAG_STATUS_MOVE;
        Members->dragOffsetX = x;
        Members->dragOffsetY = y;
    }
}
void Window::OnMouseReleased(int, int, AppCUI::Input::MouseButton)
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, );
    Members->CurrentWinButtomPressed = false;
    if (Members->dragStatus != WINDOW_DRAG_STATUS_NONE)
    {
        Members->dragStatus = WINDOW_DRAG_STATUS_NONE;
        return;
    }
    if (Members->CurrentWinButtom != WINBUTTON_NONE)
    {
        auto& b = Members->WinButtons[Members->CurrentWinButtom];
        switch (b.Type)
        {
        case WindowButtonType::CloseButton:
            RaiseEvent(Event::WindowClose);
            return;
        case WindowButtonType::MaximizeRestoreButton:
            MaximizeRestore();
            return;
        case WindowButtonType::Button:
            RaiseEvent(Event::Command, b.ID);
            return;
        case WindowButtonType::Radio:
            WindowRadioButtonClicked(
                  Members->WinButtons,
                  Members->WinButtons + Members->WinButtonsCount,
                  &Members->WinButtons[Members->CurrentWinButtom]);
            RaiseEvent(Event::Command, b.ID);
            return;
        case WindowButtonType::CheckBox:
            if (b.IsChecked())
                b.RemoveFlag(WindowButtonFlags::Checked);
            else
                b.SetFlag(WindowButtonFlags::Checked);
            RaiseEvent(Event::Command, b.ID);
            return;
        }
    }

    // if (Members->fnMouseReleaseHandler != nullptr)
    //{
    //	// daca vreau sa tratez eu evenimentul
    //	if (Members->fnMouseReleaseHandler(this, x, y, butonState, Members->fnMouseHandlerContext))
    //		return;
    //}
}
bool Window::OnMouseDrag(int x, int y, AppCUI::Input::MouseButton)
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

    // check buttons
    for (unsigned int tr = 0; tr < Members->WinButtonsCount; tr++)
    {
        if (Members->WinButtons[tr].Contains(x, y))
        {
            if (!Members->WinButtons[tr].ToolTipText.IsEmpty())
            {
                ShowToolTip(
                      Members->WinButtons[tr].ToolTipText,
                      Members->WinButtons[tr].CenterX(),
                      Members->WinButtons[tr].Y);
            }
            else
            {
                HideToolTip();
            }
            Members->CurrentWinButtom = tr; // set current button
            return true;
        }
    }
    // if I reach this point - tool tip should not be shown and there is no win button selected
    HideToolTip();

    if (Members->CurrentWinButtom == WINBUTTON_NONE)
        return false; // already outside any window button
    Members->CurrentWinButtom = WINBUTTON_NONE;
    return true;
}
bool Window::OnMouseLeave()
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, false);
    Members->CurrentWinButtomPressed = false;
    if (Members->CurrentWinButtom == WINBUTTON_NONE)
        return false; // already outside any window button
    Members->CurrentWinButtom = WINBUTTON_NONE;
    return true;
}
bool Window::OnBeforeResize(int newWidth, int newHeight)
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, false);
    if ((Members->Flags & WindowFlags::Sizeable) == WindowFlags::None)
        return false;
    return (newWidth >= Members->Layout.MinWidth) && (newWidth <= Members->Layout.MaxWidth) &&
           (newHeight >= Members->Layout.MinHeight) && (newHeight <= Members->Layout.MaxHeight);
}
void Window::OnAfterResize(int, int)
{
    WindowControlContext* Members = (WindowControlContext*) this->Context;
    if (Members)
    {
        UpdateWindowsButtonsPoz(Members);
    }
}
bool Window::OnEvent(Control*, Event eventType, int)
{
    if ((eventType == Event::WindowClose) || (eventType == Event::WindowAccept))
    {
        // check if current win is a modal dialog
        auto app = AppCUI::Application::GetApplication();
        if ((app->ModalControlsCount > 0) && (app->ModalControlsStack[app->ModalControlsCount - 1] == this))
        {
            if (eventType == Event::WindowClose)
                return Exit(AppCUI::Dialogs::Result::Cancel);
            else
                return Exit(AppCUI::Dialogs::Result::Ok);
        }
        else
        {
            // top level window -> closing the app
            Application::Close();
            return true;
        }
    }
    return false;
}
bool Window::OnKeyEvent(AppCUI::Input::Key KeyCode, char16_t)
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
        RaiseEvent(Event::WindowClose);
        return true;
    case Key::Enter:
        RaiseEvent(Event::WindowAccept);
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
void Window::OnHotKeyChanged()
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, );
    // find hotkey win button
    WindowButton* btnHotKey = nullptr;
    for (unsigned int tr = 0; tr < Members->WinButtonsCount; tr++)
        if (Members->WinButtons[tr].Type == WindowButtonType::HotKeY)
        {
            btnHotKey = &Members->WinButtons[tr];
            break;
        }
    // sanity check (in reality the pointer should always be valid)
    if (!btnHotKey)
        return;

    if (Members->HotKey == Key::None)
    {
        btnHotKey->SetFlag(WindowButtonFlags::Hidden);
    }
    else
    {
        btnHotKey->Size = (int) (KeyUtils::GetKeyName(Members->HotKey).size() + 2);
        btnHotKey->ToolTipText.Set("Press Alt+");
        btnHotKey->ToolTipText.Add(KeyUtils::GetKeyName(Members->HotKey));
        btnHotKey->ToolTipText.Add(" to activate this window");
        btnHotKey->RemoveFlag(WindowButtonFlags::Hidden);
    }
    UpdateWindowsButtonsPoz(Members);
}
void Window::SetTag(const AppCUI::Utils::ConstString& name, const AppCUI::Utils::ConstString& toolTipText)
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, );
    // find hotkey win button
    WindowButton* b = nullptr;
    for (unsigned int tr = 0; tr < Members->WinButtonsCount; tr++)
        if (Members->WinButtons[tr].Type == WindowButtonType::Tag)
        {
            b = &Members->WinButtons[tr];
            break;
        }
    // sanity check (in reality the pointer should always be valid)
    if (!b)
        return;
    if (!b->ToolTipText.Set(toolTipText))
        return;
    if (!b->Text.Set(name))
        return;
    if (b->Text.Len() > MAX_TAG_CHARS)
        if (!b->Text.Delete(MAX_TAG_CHARS, b->Text.Len()))
            return;
    // all good
    b->Size = (int) (b->Text.Len() + 2);
    b->RemoveFlag(WindowButtonFlags::Hidden);
    UpdateWindowsButtonsPoz(Members);
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
int Window::Show()
{
    CHECK(GetParent() == nullptr, -1, "Unable to run modal window if it is attached to another control !");
    CHECK(AppCUI::Application::GetApplication()->ExecuteEventLoop(this), -1, "Modal execution failed !");
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, -1);
    return Members->DialogResult;
}
int Window::GetDialogResult()
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
WindowControlsBar Window::GetControlBar(WindowControlsBarLayout layout)
{
    if ((this->Context) && (layout != WindowControlsBarLayout::None))
        return WindowControlsBar(this->Context, layout);
    else
        return WindowControlsBar(nullptr, WindowControlsBarLayout::None);
}