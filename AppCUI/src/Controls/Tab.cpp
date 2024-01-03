#include "ControlContext.hpp"

#define TAB_DISPLAY_MODE(flags) (((flags) >> 8) & 0xF)

namespace AppCUI
{
constexpr uint32 TAB_DISPLAY_MODE_TOP    = 0;
constexpr uint32 TAB_DISPLAY_MODE_BOTTOM = 1;
constexpr uint32 TAB_DISPLAY_MODE_LEFT   = 2;
constexpr uint32 TAB_DISPLAY_MODE_LIST   = 3;
constexpr uint32 TAB_DISPLAY_MODE_NOTABS = 4;

void TabControlContext::UpdateMargins()
{
    switch (TAB_DISPLAY_MODE(this->Flags))
    {
    case TAB_DISPLAY_MODE_TOP:
        this->Margins.Left = this->Margins.Right = this->Margins.Bottom = 0;
        this->Margins.Top                                               = 1;
        break;
    case TAB_DISPLAY_MODE_BOTTOM:
        this->Margins.Left = this->Margins.Right = this->Margins.Top = 0;
        this->Margins.Bottom                                         = 1;
        break;
    case TAB_DISPLAY_MODE_LEFT:
        this->Margins.Bottom = this->Margins.Right = this->Margins.Top = 0;
        this->Margins.Left                                             = this->TabTitleSize;
        break;
    case TAB_DISPLAY_MODE_LIST:
        this->Margins.Left = this->Margins.Right = 0;
        if (this->CurrentControlIndex < this->ControlsCount)
        {
            this->Margins.Top    = this->CurrentControlIndex + 1;
            this->Margins.Bottom = this->ControlsCount - (this->CurrentControlIndex + 1);
        }
        else
        {
            this->Margins.Top    = this->ControlsCount;
            this->Margins.Bottom = 0;
        }
        break;
    case TAB_DISPLAY_MODE_NOTABS:
        this->Margins.Left = this->Margins.Right = this->Margins.Bottom = this->Margins.Top = 0;
        break;
    default:
        LOG_ERROR("Unknwon TAB display mode: %d", TAB_DISPLAY_MODE(this->Flags));
        break;
    }
}
int TabControlContext::MousePositionToPanel(int x, int y)
{
    int idx;
    switch (TAB_DISPLAY_MODE(this->Flags))
    {
    case TAB_DISPLAY_MODE_TOP:
        if (y > 0)
            return -1;
        idx = (x - 1) / (this->TabTitleSize + 1);
        if (idx >= (int) this->ControlsCount)
            return -1;
        return idx;
    case TAB_DISPLAY_MODE_BOTTOM:
        if (y < this->Layout.Height - 1)
            return -1;
        idx = (x - 1) / (this->TabTitleSize + 1);
        if (idx >= (int) this->ControlsCount)
            return -1;
        return idx;
    case TAB_DISPLAY_MODE_LEFT:
        if ((x < 0) || (x > (int) this->TabTitleSize) || (y < 1))
            return -1;
        idx = y - 1;
        if (idx >= (int) this->ControlsCount)
            return -1;
        return idx;
    case TAB_DISPLAY_MODE_LIST:
        if (this->CurrentControlIndex >= this->ControlsCount)
            return -1;
        // assume top allignament
        if (y <= (int) this->CurrentControlIndex)
            return y;
        // assume bottom allignament
        idx = (this->ControlsCount) - (this->Layout.Height - y);
        if ((idx > (int) this->CurrentControlIndex) && (idx >= 0))
            return idx;
        return -1;
    case TAB_DISPLAY_MODE_NOTABS:
        return -1; // no tabs to click on
    }
    LOG_ERROR("Unknwon TAB display mode: %d", TAB_DISPLAY_MODE(this->Flags));
    return -1;
}
void TabControlContext::PaintTopBottomPanelTab(Graphics::Renderer& renderer, bool onTop)
{
    WriteTextParams params(
          WriteTextFlags::OverwriteColors | WriteTextFlags::SingleLine | WriteTextFlags::HighlightHotKey |
          WriteTextFlags::ClipToWidth | WriteTextFlags::FitTextToWidth);

    params.Width = this->TabTitleSize - 2;
    params.Align = TextAlignament::Center;
    params.Y     = onTop ? 0 : this->Layout.Height - 1;
    int poz      = 1;

    if ((this->Flags & TabFlags::TransparentBackground) != TabFlags::TransparentBackground)
        renderer.FillRectSize(0, onTop ? 1 : 0, this->Layout.Width, this->Layout.Height - 1, ' ', this->GetPageColor());
 
    if ((this->Flags & TabFlags::TabsBar) == TabFlags::TabsBar)
        renderer.FillHorizontalLineSize(0, params.Y, this->Layout.Width, ' ', this->GetTabBarColor());

    for (uint32 tr = 0; tr < this->ControlsCount; tr++, poz += (this->TabTitleSize + 1))
    {
        if (this->Controls[tr] == nullptr)
            continue;
        ControlContext* cc = (ControlContext*) this->Controls[tr]->Context;
        if (cc == nullptr)
            continue;

        const auto state = this->GetComponentState(
              ControlStateFlags::All,
              tr == static_cast<uint32>(this->HoveredTabIndex),
              tr == this->CurrentControlIndex);
        params.Color       = this->Cfg->Tab.Text.GetColor(state);
        params.HotKeyColor = this->Cfg->Tab.HotKey.GetColor(state);

        renderer.FillHorizontalLineSize(poz, params.Y, this->TabTitleSize, ' ', params.Color);
        params.HotKeyPosition = cc->HotKeyOffset;
        params.X              = poz + 1;
        renderer.WriteText(cc->Text, params);
    }
}
void TabControlContext::PaintLeftPanelTab(Graphics::Renderer& renderer)
{
    if ((this->Flags & TabFlags::TransparentBackground) != TabFlags::TransparentBackground)
        renderer.FillRectSize(
              this->TabTitleSize,
              0,
              this->Layout.Width - this->TabTitleSize,
              this->Layout.Height,
              ' ',
              this->GetPageColor());
    if ((this->Flags & TabFlags::TabsBar) == TabFlags::TabsBar)
        renderer.FillRectSize(0, 0, this->TabTitleSize, this->Layout.Height, ' ', this->GetTabBarColor());

    WriteTextParams params(
          WriteTextFlags::OverwriteColors | WriteTextFlags::SingleLine | WriteTextFlags::HighlightHotKey |
          WriteTextFlags::ClipToWidth | WriteTextFlags::FitTextToWidth);
    params.Width = this->TabTitleSize - 2;
    params.X     = 1;

    for (uint32 tr = 0; tr < this->ControlsCount; tr++)
    {
        if (this->Controls[tr] == nullptr)
            continue;
        ControlContext* cc = (ControlContext*) this->Controls[tr]->Context;
        if (cc == nullptr)
            continue;
        const auto state = this->GetComponentState(
              ControlStateFlags::All,
              tr == static_cast<uint32>(this->HoveredTabIndex),
              tr == this->CurrentControlIndex);
        params.Color       = this->Cfg->Tab.Text.GetColor(state);
        params.HotKeyColor = this->Cfg->Tab.HotKey.GetColor(state);

        renderer.FillHorizontalLineSize(0, tr + 1, this->TabTitleSize, ' ', params.Color);
        params.HotKeyPosition = cc->HotKeyOffset;
        params.Y              = tr + 1;
        renderer.WriteText(cc->Text, params);
    }
}
void TabControlContext::PaintListPanelTab(Graphics::Renderer& renderer)
{
    if ((this->Flags & TabFlags::TransparentBackground) != TabFlags::TransparentBackground)
        renderer.Clear(' ', this->GetPageColor());

    WriteTextParams params(
          WriteTextFlags::OverwriteColors | WriteTextFlags::SingleLine | WriteTextFlags::HighlightHotKey |
          WriteTextFlags::ClipToWidth | WriteTextFlags::FitTextToWidth);
    params.Width = this->Layout.Width - 2;
    params.X     = 1;
    int ypoz;
    for (uint32 tr = 0; tr < this->ControlsCount; tr++)
    {
        if (this->Controls[tr] == nullptr)
            continue;
        ControlContext* cc = (ControlContext*) this->Controls[tr]->Context;
        if (cc == nullptr)
            continue;

        const auto state = this->GetComponentState(
              ControlStateFlags::All,
              tr == static_cast<uint32>(this->HoveredTabIndex),
              tr == this->CurrentControlIndex);
        params.Color       = this->Cfg->Tab.ListText.GetColor(state);
        params.HotKeyColor = this->Cfg->Tab.ListHotKey.GetColor(state);

        if (tr <= this->CurrentControlIndex)
            ypoz = tr;
        else
            ypoz = this->Layout.Height - (this->ControlsCount - tr);
        renderer.FillHorizontalLineSize(0, ypoz, this->Layout.Width, ' ', params.Color);
        params.HotKeyPosition = cc->HotKeyOffset;
        params.Y              = ypoz;
        renderer.WriteText(cc->Text, params);
    }
}
void TabControlContext::PaintNoTabsPanelTab(Graphics::Renderer& renderer)
{
    if ((this->Flags & TabFlags::TransparentBackground) != TabFlags::TransparentBackground)
        renderer.Clear(' ', this->GetPageColor());
}
bool NextTab(Tab* t)
{
    CREATE_CONTROL_CONTEXT(t, Members, false);
    CHECK(Members->ControlsCount > 0, false, "No tab pages available !");
    return t->SetCurrentTabPageByIndex(((Members->CurrentControlIndex + 1) % Members->ControlsCount));
}
bool PreviousTab(Tab* t)
{
    CREATE_CONTROL_CONTEXT(t, Members, false);
    CHECK(Members->ControlsCount > 0, false, "No tab pages available !");
    return t->SetCurrentTabPageByIndex(
          ((Members->CurrentControlIndex + (Members->ControlsCount - 1)) % Members->ControlsCount));
}
bool Tab_SetCurrentTabPageByIndex(Tab* t, uint32 index, bool forceFocus)
{
    CREATE_TYPE_CONTEXT(TabControlContext, t, Members, false);
    CHECK((index < Members->ControlsCount), false, "Invalid tab page index: %d", index);
    bool res = true;

    // hide the rest of the tabs
    for (uint32 tr = 0; tr < Members->ControlsCount; tr++)
    {
        if (tr != index)
            Members->Controls[tr]->SetVisible(false);
    }
    // current tab
    //((ControlContext*) Members->Controls[tr]->Context)->Focused = false;
    Members->Controls[index]->SetVisible(true);
    Members->Controls[index]->SetEnabled(true);
    if ((Members->Focused) || (forceFocus))
    {
        res = Members->Controls[index]->SetFocus();
        if (!res)
            Members->CurrentControlIndex = index;
        // for the cases SetFocus has failed (currentContrlIndex still needs to be changed !
    }
    else
        Members->CurrentControlIndex = index;

    if (Members->Flags && TabFlags::ListView)
    {
        Members->UpdateMargins();
        AppCUI::Application::GetApplication()->RepaintStatus = REPAINT_STATUS_COMPUTE_POSITION;
    }
    t->RaiseEvent(Event::TabChanged);
    return res;
}
//===================================================================================================================

TabPage::TabPage(const ConstString& caption) : Control(new ControlContext(), caption, "x:0,y:0,w:100%,h:100%", true)
{
    auto Members   = reinterpret_cast<ControlContext*>(this->Context);
    Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
}

bool TabPage::OnBeforeResize(int, int)
{
    return true;
}
Tab::Tab(string_view layout, TabFlags flags, uint32 tabPageSize) : Control(new TabControlContext(), "", layout, false)
{
    tabPageSize = std::min<>(1000U, tabPageSize);
    tabPageSize = std::max<>(10U, tabPageSize);

    auto Members          = reinterpret_cast<TabControlContext*>(this->Context);
    Members->Flags        = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | (uint32) flags;
    Members->TabTitleSize = tabPageSize;
    // margin set
    Members->HoveredTabIndex = -1;
    Members->UpdateMargins();
}

bool Tab::SetCurrentTabPageByIndex(uint32 index, bool setFocus)
{
    return Tab_SetCurrentTabPageByIndex(this, index, setFocus);
}
bool Tab::GoToNextTabPage()
{
    return NextTab(this);
}
bool Tab::GoToPreviousTabPage()
{
    return PreviousTab(this);
}
bool Tab::SetCurrentTabPageByRef(Reference<Control> page, bool setFocus)
{
    uint32 index = 0xFFFFFFFF;
    CHECK(Control::GetChildIndex(page, index), false, "Fail to find page index in current tab!");
    return SetCurrentTabPageByIndex(index, setFocus);
}
Reference<Control> Tab::GetCurrentTab()
{
    CREATE_TYPECONTROL_CONTEXT(TabControlContext, Members, nullptr);
    if (Members->CurrentControlIndex >= Members->ControlsCount)
        return nullptr;
    return Members->Controls[Members->CurrentControlIndex];
}
bool Tab::SetTabPageName(uint32 index, const ConstString& name)
{
    CREATE_TYPECONTROL_CONTEXT(TabControlContext, Members, false);
    CHECK((index < Members->ControlsCount), false, "Invalid tab index: %d", index);
    CHECK(Members->Controls[index]->SetText(name, true), false, "");
    return true;
}
void Tab::OnAfterResize(int, int)
{
    CREATE_TYPECONTROL_CONTEXT(TabControlContext, Members, );

    Members->UpdateMargins();
    int nw = Members->Layout.Width - (Members->Margins.Left + Members->Margins.Right);
    int nh = Members->Layout.Height - (Members->Margins.Top + Members->Margins.Bottom);
    for (uint32 tr = 0; tr < Members->ControlsCount; tr++)
    {
        Control* copil = Members->Controls[tr];
        copil->SetEnabled(true);
        copil->SetVisible(tr == Members->CurrentControlIndex);
        copil->MoveTo(0, 0);
        copil->Resize(nw, nh);
    }
}
void Tab::OnFocus()
{
    OnAfterResize(0, 0);
    CREATE_TYPECONTROL_CONTEXT(TabControlContext, Members, );
    Members->UpdateMargins();
    AppCUI::Application::GetApplication()->RepaintStatus = REPAINT_STATUS_COMPUTE_POSITION;
}
bool Tab::OnMouseLeave()
{
    CREATE_TYPECONTROL_CONTEXT(TabControlContext, Members, false);
    if (Members->HoveredTabIndex != -1)
    {
        Members->HoveredTabIndex = -1;
        return true;
    }
    return true;
}
bool Tab::OnMouseOver(int x, int y)
{
    CREATE_TYPECONTROL_CONTEXT(TabControlContext, Members, false);
    int idx = Members->MousePositionToPanel(x, y);
    if (Members->HoveredTabIndex != idx)
    {
        Members->HoveredTabIndex = idx;
        return true;
    }
    return false;
}
void Tab::OnMouseReleased(int, int, Input::MouseButton, Input::Key)
{
    CREATE_TYPECONTROL_CONTEXT(TabControlContext, Members, );
    if (Members->HoveredTabIndex >= 0)
        SetCurrentTabPageByIndex((uint32) Members->HoveredTabIndex);
}
bool Tab::OnKeyEvent(Input::Key keyCode, char16)
{
    switch (keyCode)
    {
    case Key::Ctrl | Key::Tab:
        return NextTab(this);
    case Key::Ctrl | Key::Shift | Key::Tab:
        return PreviousTab(this);
    }
    // verific si hot-key-urile
    CREATE_TYPECONTROL_CONTEXT(TabControlContext, Members, false);
    for (uint32 tr = 0; tr < Members->ControlsCount; tr++)
        if (keyCode == Members->Controls[tr]->GetHotKey())
        {
            Tab_SetCurrentTabPageByIndex(this, tr, true);
            return true;
        }
    return false;
}
void Tab::Paint(Graphics::Renderer& renderer)
{
    CREATE_TYPECONTROL_CONTEXT(TabControlContext, Members, );

    switch (TAB_DISPLAY_MODE(Members->Flags))
    {
    case TAB_DISPLAY_MODE_TOP:
        Members->PaintTopBottomPanelTab(renderer, true);
        break;
    case TAB_DISPLAY_MODE_BOTTOM:
        Members->PaintTopBottomPanelTab(renderer, false);
        break;
    case TAB_DISPLAY_MODE_LEFT:
        Members->PaintLeftPanelTab(renderer);
        break;
    case TAB_DISPLAY_MODE_LIST:
        Members->PaintListPanelTab(renderer);
        break;
    case TAB_DISPLAY_MODE_NOTABS:
        Members->PaintNoTabsPanelTab(renderer);
        break;
    }
}
bool Tab::SetTabPageTitleSize(uint32 newSize)
{
    CHECK(newSize >= 5, false, "Tab page title size should be bigger than 5");
    CHECK(newSize < 256, false, "Tab page title size should be smaller than 256");
    CREATE_TYPECONTROL_CONTEXT(TabControlContext, Members, false);
    Members->TabTitleSize = newSize;
    Members->UpdateMargins();
    return true;
}
void Tab::OnAfterAddControl(Reference<Control> ctrl)
{
    if (ctrl == nullptr)
        return;
    // ii fac resize
    CREATE_TYPECONTROL_CONTEXT(TabControlContext, Members, );
    CREATE_CONTROL_CONTEXT(ctrl, cMembers, );
    Members->UpdateMargins();

    cMembers->Layout.X = cMembers->Layout.Y = 0;
    cMembers->Layout.Width                  = Members->Layout.Width - (Members->Margins.Left + Members->Margins.Right);
    cMembers->Layout.Height                 = Members->Layout.Height - (Members->Margins.Top + Members->Margins.Bottom);
    cMembers->Flags |= GATTR_ENABLE;
    cMembers->Flags -= (cMembers->Flags & GATTR_VISIBLE);
    if (Members->ControlsCount == 1)
    {
        cMembers->Flags |= GATTR_VISIBLE;
        Members->CurrentControlIndex = 0;
    }
}
} // namespace AppCUI