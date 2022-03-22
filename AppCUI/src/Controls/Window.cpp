#include "ControlContext.hpp"
#include "Internal.hpp"

namespace AppCUI
{
constexpr uint8 NO_CONTROLBAR_ITEM = 0xFF;
constexpr uint32 MAX_TAG_CHARS     = 8U;
const static CharacterBuffer tempReferenceChBuf;

struct WindowControlBarLayoutData
{
    int Left, Right, Y;
    WindowBarItem* LeftGroup;
    WindowBarItem* RighGroup;
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
bool ProcessHotKey(Control* ctrl, Input::Key KeyCode)
{
    if (ctrl == nullptr)
        return false;
    CREATE_CONTROL_CONTEXT(ctrl, Members, false);
    if (((Members->Flags & (GATTR_VISIBLE | GATTR_ENABLE)) != (GATTR_VISIBLE | GATTR_ENABLE)))
        return false;
    for (uint32 tr = 0; tr < Members->ControlsCount; tr++)
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
void UpdateWindowButtonPos(WindowBarItem* b, WindowControlBarLayoutData& layout, bool fromLeft)
{
    int next;

    bool partOfGroup = (b->Type == WindowBarItemType::Button) | (b->Type == WindowBarItemType::SingleChoice) |
                       (b->Type == WindowBarItemType::CheckBox) | (b->Type == WindowBarItemType::Text);
    WindowBarItem* group = nullptr;
    int extraX           = 0;

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
                    group->SetFlag(WindowBarItemFlags::RightGroupMarker); // new group, close previous one
                else
                    group->SetFlag(WindowBarItemFlags::LeftGroupMarker); // new group, close previous one
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
                group->SetFlag(WindowBarItemFlags::RightGroupMarker); // close previous one
            else
                group->SetFlag(WindowBarItemFlags::LeftGroupMarker); // close previous one
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
            b->SetFlag(WindowBarItemFlags::Visible);
            layout.Left = next;
            if (partOfGroup)
            {
                if (layout.LeftGroup == nullptr)
                    b->SetFlag(WindowBarItemFlags::LeftGroupMarker);
                else
                    b->RemoveFlag(WindowBarItemFlags::LeftGroupMarker);
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
            b->SetFlag(WindowBarItemFlags::Visible);
            layout.Right = next;
            if (partOfGroup)
            {
                if (layout.RighGroup == nullptr)
                    b->SetFlag(WindowBarItemFlags::RightGroupMarker);
                else
                    b->RemoveFlag(WindowBarItemFlags::RightGroupMarker);
                layout.RighGroup = b;
            }
        }
    }
}
void UpdateWindowsButtonsPoz(WindowControlContext* wcc)
{
    for (uint32 tr = 0; tr < wcc->ControlBar.Count; tr++)
        wcc->ControlBar.Items[tr].RemoveFlag(WindowBarItemFlags::Visible);

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

    auto* btn = wcc->ControlBar.Items;
    for (uint32 tr = 0; tr < wcc->ControlBar.Count; tr++, btn++)
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
        top.LeftGroup->SetFlag(WindowBarItemFlags::RightGroupMarker);
    if (top.RighGroup)
        top.RighGroup->SetFlag(WindowBarItemFlags::LeftGroupMarker);
    if (bottom.LeftGroup)
        bottom.LeftGroup->SetFlag(WindowBarItemFlags::RightGroupMarker);
    if (bottom.RighGroup)
        bottom.RighGroup->SetFlag(WindowBarItemFlags::LeftGroupMarker);

    // set title space
    wcc->TitleLeftMargin = top.Left + 1;
    wcc->TitleMaxWidth   = top.Right - wcc->TitleLeftMargin;
    if (wcc->TitleMaxWidth <= 2)
        wcc->TitleMaxWidth = 0;

    if (wcc->menu)
        wcc->menu->SetWidth(wcc->Layout.Width - 2);
}
void WindowRadioButtonClicked(WindowBarItem* start, WindowBarItem* end, WindowBarItem* current)
{
    // go back and disable check
    auto p = current;
    while (p >= start)
    {
        if (p->Layout == current->Layout)
        {
            if (p->Type == WindowBarItemType::SingleChoice)
                p->RemoveFlag(WindowBarItemFlags::Checked);
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
            if (p->Type == WindowBarItemType::SingleChoice)
                p->RemoveFlag(WindowBarItemFlags::Checked);
            else
                break;
        }
        p++;
    }
    current->SetFlag(WindowBarItemFlags::Checked);
}
void MoveWindowPosTo(Window* win, int addX, int addY, bool keepInDesktopounderies)
{
    auto x      = win->GetX() + addX;
    auto y      = win->GetY() + addY;
    const int w = win->GetWidth();
    const int h = win->GetHeight();
    Size desktopSize;
    if (AppCUI::Application::GetApplicationSize(desktopSize) == false)
        return;
    if (keepInDesktopounderies)
    {
        x = std::min<>(x, ((int) desktopSize.Width) - w);
        y = std::min<>(y, ((int) desktopSize.Height) - h);
        x = std::max<>(0, x);
        y = std::max<>(0, y);
    }
    else
    {
        x = std::min<>(x, ((int) desktopSize.Width) - 1);
        y = std::min<>(y, ((int) desktopSize.Height) - 1);
        if (x + w < 1)
            x = 1 - w;
        if (y + h < 1)
            y = 1 - h;
    }
    win->MoveTo(x, y);
}
void ResizeWindow(Window* win, int addToWidth, int addToHeight)
{
    const int w = win->GetWidth() + addToWidth;
    const int h = win->GetHeight() + addToHeight;
    win->Resize(w, h);
}
//=========================================================================================================================================================
ItemHandle Controls::WindowControlsBar::AddCommandItem(const ConstString& name, int ID, const ConstString& toolTip)
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, InvalidItemHandle);
    CHECK(Members->ControlBar.Count < MAX_WINDOWBAR_ITEMS,
          InvalidItemHandle,
          "Max number of items in a control bar was exceeded !");
    auto* b = &Members->ControlBar.Items[Members->ControlBar.Count];
    CHECK(b->Init(WindowBarItemType::Button, this->Layout, name, toolTip),
          InvalidItemHandle,
          "Fail to initialize item !");
    b->ID = ID;
    Members->ControlBar.Count++;
    UpdateWindowsButtonsPoz(Members);
    return Members->ControlBar.Count - 1;
}
ItemHandle Controls::WindowControlsBar::AddSingleChoiceItem(
      const ConstString& name, int ID, bool checked, const ConstString& toolTip)
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, InvalidItemHandle);
    CHECK(Members->ControlBar.Count < MAX_WINDOWBAR_ITEMS,
          InvalidItemHandle,
          "Max number of items in a control bar was exceeded !");
    auto* b = &Members->ControlBar.Items[Members->ControlBar.Count];
    CHECK(b->Init(WindowBarItemType::SingleChoice, this->Layout, name, toolTip),
          InvalidItemHandle,
          "Fail to initialize item !");
    b->ID = ID;
    Members->ControlBar.Count++;
    if (checked)
        WindowRadioButtonClicked(
              Members->ControlBar.Items,
              Members->ControlBar.Items + Members->ControlBar.Count,
              Members->ControlBar.Items + Members->ControlBar.Count - 1);
    UpdateWindowsButtonsPoz(Members);
    return Members->ControlBar.Count - 1;
}
ItemHandle Controls::WindowControlsBar::AddCheckItem(
      const ConstString& name, int ID, bool checked, const ConstString& toolTip)
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, InvalidItemHandle);
    CHECK(Members->ControlBar.Count < MAX_WINDOWBAR_ITEMS,
          InvalidItemHandle,
          "Max number of items in a control bar was exceeded !");
    auto* b = &Members->ControlBar.Items[Members->ControlBar.Count];
    CHECK(b->Init(WindowBarItemType::CheckBox, this->Layout, name, toolTip),
          InvalidItemHandle,
          "Fail to initialize item !");
    b->ID = ID;
    Members->ControlBar.Count++;
    if (checked)
        b->SetFlag(WindowBarItemFlags::Checked);
    else
        b->RemoveFlag(WindowBarItemFlags::Checked);
    UpdateWindowsButtonsPoz(Members);
    return Members->ControlBar.Count - 1;
}
ItemHandle Controls::WindowControlsBar::AddTextItem(const ConstString& caption, const ConstString& toolTip)
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, InvalidItemHandle);
    CHECK(Members->ControlBar.Count < MAX_WINDOWBAR_ITEMS,
          InvalidItemHandle,
          "Max number of items in a control bar was exceeded !");
    auto* b = &Members->ControlBar.Items[Members->ControlBar.Count];
    CHECK(b->Init(WindowBarItemType::Text, this->Layout, caption, toolTip),
          InvalidItemHandle,
          "Fail to initialize item !");

    Members->ControlBar.Count++;
    UpdateWindowsButtonsPoz(Members);
    return Members->ControlBar.Count - 1;
}
WindowBarItem* GetWindowControlsBarItem(void* Context, ItemHandle itemHandle)
{
    WindowControlContext* Members = (WindowControlContext*) Context;
    CHECK(Members, nullptr, "");
    uint32 id = (uint32) itemHandle;
    CHECK(id < Members->ControlBar.Count, nullptr, "Invalid item index (%d/%d)", id, Members->ControlBar.Count);
    auto* b = Members->ControlBar.Items + id;
    CHECK((b->Type == WindowBarItemType::Button) || (b->Type == WindowBarItemType::CheckBox) ||
                (b->Type == WindowBarItemType::SingleChoice) || (b->Type == WindowBarItemType::Text),
          nullptr,
          "");
    return b;
}
bool Controls::WindowControlsBar::SetItemText(ItemHandle itemHandle, const ConstString& caption)
{
    auto b = GetWindowControlsBarItem(this->Context, itemHandle);
    CHECK(b, false, "");

    CHECK(b->Text.SetWithHotKey(caption, b->HotKeyOffset, b->HotKey, Key::Alt), false, "");
    b->Size = b->Text.Len();
    if (b->Type == WindowBarItemType::CheckBox)
        b->Size += 2;
    UpdateWindowsButtonsPoz((WindowControlContext*) Context);
    return true;
}
bool Controls::WindowControlsBar::SetItemTextWithHotKey(
      ItemHandle itemHandle, const ConstString& caption, uint32 hotKeyTextOffset)
{
    CHECK(SetItemText(itemHandle, caption), false, "");
    auto b = GetWindowControlsBarItem(this->Context, itemHandle);
    CHECK(b, false, "");
    b->HotKeyOffset = CharacterBuffer::INVALID_HOTKEY_OFFSET;
    b->HotKey       = Key::None;

    ConstStringObject txt(caption);
    char16 ch = 0;
    if (hotKeyTextOffset < txt.Length)
    {
        switch (txt.Encoding)
        {
        case StringEncoding::Ascii:
            ch = (((const char*) txt.Data)[hotKeyTextOffset]);
            break;
        case StringEncoding::Unicode16:
            ch = (((const char16*) txt.Data)[hotKeyTextOffset]);
            break;
        case StringEncoding::CharacterBuffer:
            ch = (((const Character*) txt.Data)[hotKeyTextOffset].Code);
            break;
        case StringEncoding::UTF8:
            ch = (((const uint8*) txt.Data)[hotKeyTextOffset]);
            break;
        }
        if (ch != 0)
        {
            b->HotKey = Utils::KeyUtils::CreateHotKey(ch, Key::Alt);
            if (b->HotKey != Key::None)
                b->HotKeyOffset = hotKeyTextOffset;
        }
    }
    return true;
}
bool Controls::WindowControlsBar::SetItemToolTip(ItemHandle itemHandle, const ConstString& caption)
{
    auto b = GetWindowControlsBarItem(this->Context, itemHandle);
    CHECK(b, false, "");
    CHECK(b->ToolTipText.Set(caption), false, "");
    return true;
}
bool Controls::WindowControlsBar::IsItemChecked(ItemHandle itemHandle)
{
    auto b = GetWindowControlsBarItem(this->Context, itemHandle);
    CHECK(b, false, "");
    return b->IsChecked();
}
bool Controls::WindowControlsBar::SetItemCheck(ItemHandle itemHandle, bool value)
{
    auto b = GetWindowControlsBarItem(this->Context, itemHandle);
    CHECK(b, false, "");
    if (b->Type == WindowBarItemType::CheckBox)
    {
        if (value)
            b->SetFlag(WindowBarItemFlags::Checked);
        else
            b->RemoveFlag(WindowBarItemFlags::Checked);
        return true;
    }
    if (b->Type == WindowBarItemType::SingleChoice)
    {
        CHECK(value, false, "For radio buttom only 'true' can be used as a value");
        WindowControlContext* Members = (WindowControlContext*) Context;
        WindowRadioButtonClicked(Members->ControlBar.Items, Members->ControlBar.Items + Members->ControlBar.Count, b);
        return true;
    }
    RETURNERROR(false, "This method can only be applied on Check and Radio items");
}
bool Controls::WindowControlsBar::IsItemVisible(ItemHandle itemHandle)
{
    auto b = GetWindowControlsBarItem(this->Context, itemHandle);
    CHECK(b, false, "");
    return !b->IsHidden();
}
bool Controls::WindowControlsBar::IsItemShown(ItemHandle itemHandle)
{
    auto b = GetWindowControlsBarItem(this->Context, itemHandle);
    CHECK(b, false, "");
    return b->IsVisible() && (!b->IsHidden());
}
bool Controls::WindowControlsBar::SetItemVisible(ItemHandle itemHandle, bool value)
{
    auto b = GetWindowControlsBarItem(this->Context, itemHandle);
    CHECK(b, false, "");
    if ((b->Type == WindowBarItemType::CheckBox) || (b->Type == WindowBarItemType::Button) ||
        (b->Type == WindowBarItemType::SingleChoice) || (b->Type == WindowBarItemType::Text))
    {
        // change visibility
        if (value)
            b->RemoveFlag(WindowBarItemFlags::Hidden);
        else
            b->SetFlag(WindowBarItemFlags::Hidden);
        UpdateWindowsButtonsPoz((WindowControlContext*) this->Context);
        return true;
    }
    RETURNERROR(false, "This method can only be applied on Check and Radio items");
}
//=========================================================================================================================================================
bool WindowBarItem::Init(WindowBarItemType type, WindowControlsBarLayout layout, uint8 size, string_view toolTipText)
{
    this->Type         = type;
    this->Layout       = layout;
    this->Size         = size;
    this->X            = 0;
    this->Y            = 0;
    this->Flags        = WindowBarItemFlags::None;
    this->ID           = -1;
    this->HotKey       = Key::None;
    this->HotKeyOffset = CharacterBuffer::INVALID_HOTKEY_OFFSET;
    if (!toolTipText.empty())
    {
        CHECK(this->ToolTipText.Set(toolTipText), false, "");
    }
    return true;
}
bool WindowBarItem::Init(
      WindowBarItemType type, WindowControlsBarLayout layout, const ConstString& name, const ConstString& toolTip)
{
    this->Type         = type;
    this->Layout       = layout;
    this->Size         = 0;
    this->X            = 0;
    this->Y            = 0;
    this->Flags        = WindowBarItemFlags::None;
    this->ID           = -1;
    this->HotKey       = Key::None;
    this->HotKeyOffset = CharacterBuffer::INVALID_HOTKEY_OFFSET;
    // name
    ConstStringObject objName(name);
    CHECK(objName.Length > 0, false, "Expecting a valid item name (non-empty)");
    CHECK(this->Text.SetWithHotKey(name, this->HotKeyOffset, this->HotKey, Key::Alt), false, "Fail to create name !");
    this->Size = this->Text.Len();
    if (type == WindowBarItemType::CheckBox)
        this->Size += 2; // for the checkmark
    // tool tip
    ConstStringObject objToolTip(toolTip);
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
Window::Window(const ConstString& caption, string_view layout, WindowFlags Flags)
    : Control(new WindowControlContext(), caption, layout, false)
{
    auto Members              = reinterpret_cast<WindowControlContext*>(this->Context);
    Members->Layout.MaxHeight = 200000;
    Members->Layout.MaxWidth  = 200000;
    Members->Layout.MinHeight = 3;
    Members->Layout.MinWidth  = 12; // left_corner(1 char), maximize button(3chars),OneSpaceLeftPadding,
                                    // title, OneSpaceRightPadding, close
                                    // button(char),right_corner(1 char) = 10+szTitle (szTitle = min 2 chars)
    ASSERT(SetMargins(1, 1, 1, 1), "Failed to set margins !");
    Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | (uint32) Flags;

    Members->Maximized                       = false;
    Members->ResizeMoveMode                  = false;
    Members->dragStatus                      = WindowDragStatus::None;
    Members->DialogResult                    = -1;
    Members->ControlBar.Current              = NO_CONTROLBAR_ITEM;
    Members->ControlBar.IsCurrentItemPressed = false;
    Members->ControlBar.Count                = 0;
    Members->referalItemHandle               = InvalidItemHandle;
    Members->windowItemHandle                = InvalidItemHandle;

    ASSERT(Members->RecomputeLayout(nullptr), "Fail to recompute layout !");
    this->RecomputeLayout();

    // init the buttons
    if ((Flags & WindowFlags::NoCloseButton) == WindowFlags::None)
    {
        Members->ControlBar.Items[Members->ControlBar.Count++].Init(
              WindowBarItemType::CloseButton, WindowControlsBarLayout::TopBarFromRight, 3, "Close window");
    }
    if ((Flags & WindowFlags::Sizeable) != WindowFlags::None)
    {
        Members->ControlBar.Items[Members->ControlBar.Count++].Init(
              WindowBarItemType::MaximizeRestoreButton,
              WindowControlsBarLayout::TopBarFromLeft,
              3,
              "Maximize or restore the size of this window");
        Members->ControlBar.Items[Members->ControlBar.Count++].Init(
              WindowBarItemType::WindowResize,
              WindowControlsBarLayout::BottomBarFromRight,
              1,
              "Click and drag to resize this window");
    }
    // hot key
    Members->ControlBar.Items[Members->ControlBar.Count].Init(
          WindowBarItemType::HotKeY,
          WindowControlsBarLayout::TopBarFromRight,
          3,
          "Press Alt+xx to switch to this window");
    // the button exists but it is hidden
    Members->ControlBar.Items[Members->ControlBar.Count].SetFlag(WindowBarItemFlags::Hidden);
    Members->ControlBar.Count++;

    // TAG
    Members->ControlBar.Items[Members->ControlBar.Count].Init(
          WindowBarItemType::Tag, WindowControlsBarLayout::TopBarFromLeft, 3, "");
    // the button exists but it is hidden
    Members->ControlBar.Items[Members->ControlBar.Count].SetFlag(WindowBarItemFlags::Hidden);
    Members->ControlBar.Count++;

    UpdateWindowsButtonsPoz(Members);

    if ((Flags & WindowFlags::Maximized) == WindowFlags::Maximized)
    {
        ASSERT(MaximizeRestore(), "Fail to maximize window !");
    }
    if ((Flags & WindowFlags::Menu) == WindowFlags::Menu)
    {
        Members->menu = std::make_unique<Internal::MenuBar>(this, 1, 1);
        Members->Margins.Top += 1;
        Members->menu->SetWidth(Members->Layout.Width - 2);
    }
}

void Window::Paint(Graphics::Renderer& renderer)
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, );
    ColorPair colorTitle, colorWindow, colorBorder, colorStartEndSeparators, tmpCol, tmpHK;
    LineType lineType;

    if ((Members->Flags & WindowFlags::WarningWindow) != WindowFlags::None)
        colorWindow = { Color::Black, Members->Cfg->Window.Background.Warning };
    else if ((Members->Flags & WindowFlags::ErrorWindow) != WindowFlags::None)
        colorWindow = { Color::Black, Members->Cfg->Window.Background.Error };
    else if ((Members->Flags & WindowFlags::NotifyWindow) != WindowFlags::None)
        colorWindow = { Color::Black, Members->Cfg->Window.Background.Info };
    else
        colorWindow = { Color::Black, Members->Cfg->Window.Background.Normal };

    const auto sepColor = Members->Focused ? Members->Cfg->Lines.Normal : Members->Cfg->Lines.Inactive;

    if (Members->Focused)
    {
        colorTitle  = Members->Cfg->Text.Focused;
        colorBorder = Members->dragStatus == WindowDragStatus::None ? Members->Cfg->Border.Focused
                                                                    : Members->Cfg->Border.PressedOrSelected;
        lineType    = Members->dragStatus == WindowDragStatus::None ? LineType::Double : LineType::Single;
        if (Members->ResizeMoveMode)
            colorBorder = Members->Cfg->Border.PressedOrSelected;
    }
    else
    {
        colorTitle              = Members->Cfg->Text.Normal;
        colorWindow.Background  = Members->Cfg->Window.Background.Inactive;
        colorBorder             = Members->Cfg->Border.Normal;
        lineType                = LineType::Single;
        Members->ResizeMoveMode = false;
    }
    renderer.Clear(' ', colorWindow);
    renderer.DrawRectSize(0, 0, Members->Layout.Width, Members->Layout.Height, colorBorder, lineType);

    auto* btn = Members->ControlBar.Items;
    for (uint32 tr = 0; tr < Members->ControlBar.Count; tr++, btn++)
    {
        if ((!btn->IsVisible()) || (btn->IsHidden()))
            continue;
        bool fromLeft = (btn->Layout == WindowControlsBarLayout::TopBarFromLeft) ||
                        (btn->Layout == WindowControlsBarLayout::BottomBarFromLeft);
        bool showChecked        = false;
        colorStartEndSeparators = colorBorder;
        auto state              = ControlState::Normal;

        if (Members->ControlBar.Current == tr)
        {
            // hover or pressed
            if (Members->ControlBar.IsCurrentItemPressed)
                state = ControlState::PressedOrSelected;
            else
            {
                showChecked = ((Members->Focused) && (btn->IsChecked()));
                state       = ControlState::Hovered;
            }
        }
        else
        {
            if (Members->Focused)
            {
                showChecked = btn->IsChecked();
                state       = ControlState::Focused;
            }
            else
                state = ControlState::Inactive;
        }
        bool hoverOrPressed = (state == ControlState::Hovered) || (state == ControlState::PressedOrSelected);
        bool drawSeparators = false;
        switch (btn->Type)
        {
        case WindowBarItemType::CloseButton:
            renderer.WriteSingleLineText(
                  btn->X, btn->Y, "[ ]", Members->GetSymbolColor(state, colorStartEndSeparators));
            renderer.WriteCharacter(
                  btn->X + 1, btn->Y, 'x', Members->GetSymbolColor(state, Members->Cfg->Symbol.Close));
            break;
        case WindowBarItemType::MaximizeRestoreButton:
            renderer.WriteSingleLineText(
                  btn->X, btn->Y, "[ ]", Members->GetSymbolColor(state, colorStartEndSeparators));
            renderer.WriteSpecialCharacter(
                  btn->X + 1,
                  btn->Y,
                  Members->Maximized ? SpecialChars::ArrowUpDown : SpecialChars::ArrowUp,
                  Members->GetSymbolColor(state, Members->Cfg->Symbol.Maximized));
            break;
        case WindowBarItemType::WindowResize:
            if (Members->Focused)
                renderer.WriteSpecialCharacter(
                      btn->X,
                      btn->Y,
                      SpecialChars::BoxBottomRightCornerSingleLine,
                      Members->GetSymbolColor(state, Members->Cfg->Symbol.Resize));
            break;
        case WindowBarItemType::HotKeY:
            renderer.WriteCharacter(btn->X, btn->Y, '[', colorStartEndSeparators);
            tmpCol = Members->Focused ? Members->Cfg->Text.Normal : Members->Cfg->Text.Inactive;
            renderer.WriteSingleLineText(btn->X + 1, btn->Y, KeyUtils::GetKeyName(Members->HotKey), tmpCol);
            renderer.WriteCharacter(btn->X + btn->Size - 1, btn->Y, ']', colorStartEndSeparators);
            break;
        case WindowBarItemType::Tag:
            renderer.WriteCharacter(btn->X, btn->Y, '[', colorStartEndSeparators);
            tmpCol = Members->Focused ? Members->Cfg->Text.Emphasized2 : Members->Cfg->Text.Inactive;
            renderer.WriteSingleLineText(btn->X + 1, btn->Y, btn->Text, tmpCol);
            renderer.WriteCharacter(btn->X + btn->Size - 1, btn->Y, ']', colorStartEndSeparators);
            break;

        case WindowBarItemType::Button:
        case WindowBarItemType::SingleChoice:
            switch (state)
            {
            case ControlState::Hovered:
                tmpCol = Members->Cfg->Button.Text.Hovered;
                tmpHK  = Members->Cfg->Button.Text.Hovered;
                break;
            case ControlState::Normal:
                tmpCol = Members->Cfg->Text.Normal;
                tmpHK  = Members->Cfg->Text.HotKey;
                break;
            case ControlState::Focused:
                tmpCol = Members->Cfg->Text.Normal;
                tmpHK  = Members->Cfg->Text.HotKey;
                break;
            case ControlState::PressedOrSelected:
                tmpCol = Members->Cfg->Button.Text.PressedOrSelected;
                tmpHK  = Members->Cfg->Button.Text.PressedOrSelected;
                break;
            default:
                tmpHK = tmpCol = Members->Cfg->Text.Inactive;
                break;
            }
            if (showChecked)
                renderer.WriteSingleLineText(
                      btn->X,
                      btn->Y,
                      btn->Text,
                      Members->Cfg->Button.Text.PressedOrSelected,
                      Members->Cfg->Button.HotKey.PressedOrSelected,
                      btn->HotKeyOffset);
            else
                renderer.WriteSingleLineText(btn->X, btn->Y, btn->Text, tmpCol, tmpHK, btn->HotKeyOffset);
            drawSeparators = true;
            break;
        case WindowBarItemType::CheckBox:
            switch (state)
            {
            case ControlState::Hovered:
                tmpCol = Members->Cfg->Button.Text.Hovered;
                tmpHK  = Members->Cfg->Button.Text.Hovered;
                break;
            case ControlState::Normal:
                tmpCol = Members->Cfg->Text.Normal;
                tmpHK  = Members->Cfg->Text.HotKey;
                break;
            case ControlState::Focused:
                tmpCol = Members->Cfg->Text.Normal;
                tmpHK  = Members->Cfg->Text.HotKey;
                break;
            case ControlState::PressedOrSelected:
                tmpCol = Members->Cfg->Button.Text.PressedOrSelected;
                tmpHK  = Members->Cfg->Button.Text.PressedOrSelected;
                break;
            default:
                tmpHK = tmpCol = Members->Cfg->Text.Inactive;
                break;
            }
            renderer.FillHorizontalLine(btn->X, btn->Y, btn->X + 1, ' ', tmpCol);
            renderer.WriteSingleLineText(btn->X + 2, btn->Y, btn->Text, tmpCol, tmpHK, btn->HotKeyOffset);
            if (btn->IsChecked())
            {
                // tmpCol = (Members->Focused && (!hoverOrPressed)) ? wcfg->ControlBar.CheckMark : c_i->Text;
                tmpCol = Members->GetSymbolColor(state, Members->Cfg->Symbol.Checked);
                renderer.WriteSpecialCharacter(btn->X, btn->Y, SpecialChars::CheckMark, tmpCol);
            }
            drawSeparators = true;
            break;
        case WindowBarItemType::Text:
            tmpCol = Members->Focused ? Members->Cfg->Text.Normal : Members->Cfg->Text.Inactive;
            renderer.WriteSingleLineText(btn->X, btn->Y, btn->Text, tmpCol);
            drawSeparators = true;
            break;
        }
        // separators
        if (drawSeparators)
        {
            if ((uint8) btn->Flags & (uint8) WindowBarItemFlags::LeftGroupMarker)
                renderer.WriteCharacter(btn->X - 1, btn->Y, '[', colorStartEndSeparators);
            else if (fromLeft)
                renderer.WriteCharacter(btn->X - 1, btn->Y, '|', sepColor);
            if ((uint8) btn->Flags & (uint8) WindowBarItemFlags::RightGroupMarker)
                renderer.WriteCharacter(btn->X + btn->Size, btn->Y, ']', colorStartEndSeparators);
            else if (!fromLeft)
                renderer.WriteCharacter(btn->X + btn->Size, btn->Y, '|', sepColor);
        }
    }

    // Title
    if (Members->TitleMaxWidth >= 2)
    {
        WriteTextParams params(
              WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth | WriteTextFlags::FitTextToWidth |
                    WriteTextFlags::OverwriteColors | WriteTextFlags::LeftMargin | WriteTextFlags::RightMargin,
              TextAlignament::Center);
        params.X     = Members->TitleLeftMargin;
        params.Y     = 0;
        params.Color = colorTitle;
        params.Width = Members->TitleMaxWidth;
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
        CHECK(Application::GetDesktopSize(sz), false, "Fail to get desktop size");
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
    AppCUI::Application::GetApplication()->RepaintStatus = REPAINT_STATUS_COMPUTE_POSITION;
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
void Window::OnMousePressed(int x, int y, Input::MouseButton button)
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, );
    Members->dragStatus                      = WindowDragStatus::None;
    Members->ControlBar.IsCurrentItemPressed = false;
    Members->ResizeMoveMode                  = false;

    if (Members->menu)
    {
        if (Members->menu->OnMousePressed(x, y, button))
            return;
    }

    // win buttons
    Members->ControlBar.Current = NO_CONTROLBAR_ITEM;
    for (uint32 tr = 0; tr < Members->ControlBar.Count; tr++)
    {
        if (Members->ControlBar.Items[tr].Contains(x, y))
        {
            Members->ControlBar.Current              = tr; // set current button
            Members->ControlBar.IsCurrentItemPressed = true;
            if (Members->ControlBar.Items[tr].Type == WindowBarItemType::WindowResize)
                Members->dragStatus = WindowDragStatus::Resize;
            return;
        }
    }
    // Hide tool tip
    HideToolTip();

    if ((Members->Flags & WindowFlags::FixedPosition) == WindowFlags::None)
    {
        Members->dragStatus  = WindowDragStatus::Move;
        Members->dragOffsetX = x;
        Members->dragOffsetY = y;
    }
}
bool Window::ProcessControlBarItem(uint32 index)
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, false);
    CHECK(index < Members->ControlBar.Count, false, "");
    auto& b = Members->ControlBar.Items[index];
    switch (b.Type)
    {
    case WindowBarItemType::CloseButton:
        RaiseEvent(Event::WindowClose);
        return true;
    case WindowBarItemType::MaximizeRestoreButton:
        MaximizeRestore();
        return true;
    case WindowBarItemType::Button:
        RaiseEvent(Event::Command, b.ID);
        return true;
    case WindowBarItemType::SingleChoice:
        WindowRadioButtonClicked(
              Members->ControlBar.Items,
              Members->ControlBar.Items + Members->ControlBar.Count,
              &Members->ControlBar.Items[index]);
        RaiseEvent(Event::Command, b.ID);
        return true;
    case WindowBarItemType::CheckBox:
        if (b.IsChecked())
            b.RemoveFlag(WindowBarItemFlags::Checked);
        else
            b.SetFlag(WindowBarItemFlags::Checked);
        RaiseEvent(Event::Command, b.ID);
        return true;
    }
    return false;
}
void Window::OnMouseReleased(int, int, Input::MouseButton)
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, );
    Members->ControlBar.IsCurrentItemPressed = false;
    Members->ResizeMoveMode                  = false;
    if (Members->dragStatus != WindowDragStatus::None)
    {
        Members->dragStatus = WindowDragStatus::None;
        return;
    }
    if (Members->ControlBar.Current != NO_CONTROLBAR_ITEM)
    {
        if (ProcessControlBarItem(Members->ControlBar.Current))
            return;
    }

    // if (Members->fnMouseReleaseHandler != nullptr)
    //{
    //	// daca vreau sa tratez eu evenimentul
    //	if (Members->fnMouseReleaseHandler(this, x, y, butonState, Members->fnMouseHandlerContext))
    //		return;
    //}
}
bool Window::OnMouseDrag(int x, int y, Input::MouseButton)
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, false);
    Members->ResizeMoveMode = false;
    if (Members->dragStatus == WindowDragStatus::Resize)
    {
        bool res = Resize(x + 1, y + 1);
        UpdateWindowsButtonsPoz(Members);
        return res;
    }
    if (Members->dragStatus == WindowDragStatus::Move)
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
    for (uint32 tr = 0; tr < Members->ControlBar.Count; tr++)
    {
        if (Members->ControlBar.Items[tr].Contains(x, y))
        {
            if (!Members->ControlBar.Items[tr].ToolTipText.IsEmpty())
            {
                ShowToolTip(
                      Members->ControlBar.Items[tr].ToolTipText,
                      Members->ControlBar.Items[tr].CenterX(),
                      Members->ControlBar.Items[tr].Y);
            }
            else
            {
                HideToolTip();
            }
            Members->ControlBar.Current = tr; // set current button
            return true;
        }
    }
    // if I reach this point - tool tip should not be shown and there is no win button selected
    HideToolTip();

    if (Members->ControlBar.Current == NO_CONTROLBAR_ITEM)
        return false; // already outside any window button
    Members->ControlBar.Current = NO_CONTROLBAR_ITEM;
    return true;
}
bool Window::OnMouseLeave()
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, false);
    Members->ControlBar.IsCurrentItemPressed = false;
    if (Members->ControlBar.Current == NO_CONTROLBAR_ITEM)
        return false; // already outside any window button
    Members->ControlBar.Current = NO_CONTROLBAR_ITEM;
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
void Window::RemoveMe()
{
    auto app = Application::GetApplication();
    if (!app)
        return;
    // check if I am part of the modal stack
    for (auto i = 0U; i < app->ModalControlsCount; i++)
        if (app->ModalControlsStack[i] == this)
            return;
    if (!app->AppDesktop)
        return;
    // all good -> I am a top level window --> remove me
    app->AppDesktop->RemoveControl(this);
    app->CheckIfAppShouldClose();
}
bool Window::OnEvent(Reference<Control>, Event eventType, int)
{
    if ((eventType == Event::WindowClose) || (eventType == Event::WindowAccept))
    {
        // check if current win is a modal dialog
        auto app = Application::GetApplication();
        if ((app->ModalControlsCount > 0) && (app->ModalControlsStack[app->ModalControlsCount - 1] == this))
        {
            if (eventType == Event::WindowClose)
                return Exit(Dialogs::Result::Cancel);
            else
                return Exit(Dialogs::Result::Ok);
        }
        else
        {
            RemoveMe();
            return true;
        }
    }
    return false;
}
bool Window::OnKeyEvent(Input::Key KeyCode, char16)
{
    Control* tmp;
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, false);

    if (Members->ResizeMoveMode)
    {
        switch (KeyCode)
        {
        case Key::Escape:
        case Key::Enter:
        case Key::Space:
        case Key::Tab:
            Members->ResizeMoveMode = false;
            return true;
        case Key::Up:
            MoveWindowPosTo(this, 0, -1, false);
            return true;
        case Key::Down:
            MoveWindowPosTo(this, 0, 1, false);
            return true;
        case Key::Left:
            MoveWindowPosTo(this, -1, 0, false);
            return true;
        case Key::Right:
            MoveWindowPosTo(this, 1, 0, false);
            return true;
        case Key::Alt | Key::Up:
            MoveWindowPosTo(this, 0, -1000000, true);
            return true;
        case Key::Alt | Key::Down:
            MoveWindowPosTo(this, 0, 1000000, true);
            return true;
        case Key::Alt | Key::Left:
            MoveWindowPosTo(this, -1000000, 0, true);
            return true;
        case Key::Alt | Key::Right:
            MoveWindowPosTo(this, 1000000, 0, true);
            return true;
        case Key::C:
            CenterScreen();
            return true;
        case Key::M:
        case Key::R:
            MaximizeRestore();
            return true;
        case Key::Ctrl | Key::Up:
            ResizeWindow(this, 0, -1);
            return true;
        case Key::Ctrl | Key::Down:
            ResizeWindow(this, 0, 1);
            return true;
        case Key::Ctrl | Key::Left:
            ResizeWindow(this, -1, 0);
            return true;
        case Key::Ctrl | Key::Right:
            ResizeWindow(this, 1, 0);
            return true;
        }
    }
    else
    {
        switch (KeyCode)
        {
        case Key::Ctrl | Key::Alt | Key::M:
        case Key::Ctrl | Key::Alt | Key::R:
            Members->ResizeMoveMode = true;
            return true;

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
            if (!(Members->Flags && WindowFlags::NoCloseButton))
            {
                RaiseEvent(Event::WindowClose);
                return true;
            }
            return false;
        case Key::Enter:
            if (Members->Flags && WindowFlags::ProcessReturn)
            {
                RaiseEvent(Event::WindowAccept);
                return true;
            }
            return false;
        }
        // first we check menu hot keys
        if (Members->menu)
        {
            if (Members->menu->OnKeyEvent(KeyCode))
                return true;
        }
        // check cntrols hot keys
        if ((((uint32) KeyCode) & (uint32) (Key::Shift | Key::Alt | Key::Ctrl)) == ((uint32) Key::Alt))
        {
            if (ProcessHotKey(this, KeyCode))
                return true;
            auto* b = Members->ControlBar.Items;
            auto* e = b + Members->ControlBar.Count;
            while (b < e)
            {
                if (b->HotKey == KeyCode)
                {
                    if (ProcessControlBarItem((uint32) (b - Members->ControlBar.Items)))
                        return true;
                }
                b++;
            }
        }
    }
    // key was not prcessed, pass it to my parent
    return false;
}
void Window::OnHotKeyChanged()
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, );
    // find hotkey win button
    WindowBarItem* btnHotKey = nullptr;
    for (uint32 tr = 0; tr < Members->ControlBar.Count; tr++)
        if (Members->ControlBar.Items[tr].Type == WindowBarItemType::HotKeY)
        {
            btnHotKey = &Members->ControlBar.Items[tr];
            break;
        }
    // sanity check (in reality the pointer should always be valid)
    if (!btnHotKey)
        return;

    if (Members->HotKey == Key::None)
    {
        btnHotKey->SetFlag(WindowBarItemFlags::Hidden);
    }
    else
    {
        btnHotKey->Size = (int) (KeyUtils::GetKeyName(Members->HotKey).size() + 2);
        btnHotKey->ToolTipText.Set("Press Alt+");
        btnHotKey->ToolTipText.Add(KeyUtils::GetKeyName(Members->HotKey));
        btnHotKey->ToolTipText.Add(" to activate this window");
        btnHotKey->RemoveFlag(WindowBarItemFlags::Hidden);
    }
    UpdateWindowsButtonsPoz(Members);
}
void Window::SetTag(const ConstString& name, const ConstString& toolTipText)
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, );
    // find tag win button
    WindowBarItem* b = nullptr;
    for (uint32 tr = 0; tr < Members->ControlBar.Count; tr++)
        if (Members->ControlBar.Items[tr].Type == WindowBarItemType::Tag)
        {
            b = &Members->ControlBar.Items[tr];
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
    b->RemoveFlag(WindowBarItemFlags::Hidden);
    UpdateWindowsButtonsPoz(Members);
}
const Graphics::CharacterBuffer& Window::GetTag()
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, tempReferenceChBuf);
    // find tag win button
    for (uint32 tr = 0; tr < Members->ControlBar.Count; tr++)
    {
        if (Members->ControlBar.Items[tr].Type == WindowBarItemType::Tag)
        {
            return Members->ControlBar.Items[tr].Text;
        }
    }

    return tempReferenceChBuf;
}

bool Window::Exit(int dialogResult)
{
    CHECK(dialogResult >= 0, false, "Dialog result code must be bigger than 0 !");
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, false);
    Members->DialogResult                     = dialogResult;
    Members->ResizeMoveMode                   = false;
    Application::GetApplication()->loopStatus = Internal::LoopStatus::StopCurrent;
    return true;
}
bool Window::Exit(Dialogs::Result dialogResult)
{
    return this->Exit(static_cast<int>(dialogResult));
}
int Window::Show()
{
    CHECK(GetParent() == nullptr, -1, "Unable to run modal window if it is attached to another control !");
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, -1);
    CHECK(Members->RecomputeLayout(nullptr), -1, "Fail to recompute layout !");
    this->RecomputeLayout();
    CHECK(Application::GetApplication()->ExecuteEventLoop(this), -1, "Modal execution failed !");

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
    return (Members->dragStatus == WindowDragStatus::Resize);
}
bool Window::EnableResizeMode()
{
    CHECK(this->HasFocus(), false, "To enable resize mode a window must be focused !");
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, false);
    Members->ResizeMoveMode = true;
    return true; 
}
Reference<Menu> Window::AddMenu(const ConstString& name)
{
    CREATE_TYPECONTROL_CONTEXT(WindowControlContext, Members, nullptr);
    CHECK(Members->menu, nullptr, "Application was not initialized with Menu option set up !");
    ItemHandle itm         = Members->menu->AddMenu(name);
    Controls::Menu* result = Members->menu->GetMenu(itm);
    CHECK(result, nullptr, "Fail to create menu !");
    return Reference<Menu>(result);
}
WindowControlsBar Window::GetControlBar(WindowControlsBarLayout layout)
{
    if ((this->Context) && (layout != WindowControlsBarLayout::None))
        return WindowControlsBar(this->Context, layout);
    else
        return WindowControlsBar(nullptr, WindowControlsBarLayout::None);
}
} // namespace AppCUI