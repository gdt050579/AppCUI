#include "ControlContext.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Input;
using namespace AppCUI::Graphics;

enum class TabDisplay
{
    Top    = 0,
    Bottom = 1,
    Left   = 2,
    List   = 3,
    NoTabs = 4,
};

#define TAB_DISPLAY_MODE(flags) static_cast<TabDisplay>((((flags) >> 8) & 0xF))

void TabControlContext::UpdateMargins()
{
    switch (TAB_DISPLAY_MODE(Flags))
    {
    case TabDisplay::Top:
        Margins.Left = Margins.Right = Margins.Bottom = 0;
        Margins.Top                                   = 1;
        break;
    case TabDisplay::Bottom:
        Margins.Left = Margins.Right = Margins.Top = 0;
        Margins.Bottom                             = 1;
        break;
    case TabDisplay::Left:
        Margins.Bottom = Margins.Right = Margins.Top = 0;
        Margins.Left                                 = TabTitleSize;
        break;
    case TabDisplay::List:
        Margins.Left = Margins.Right = 0;
        if (CurrentControlIndex < ControlsCount)
        {
            Margins.Top    = CurrentControlIndex + 1;
            Margins.Bottom = ControlsCount - (CurrentControlIndex + 1);
        }
        else
        {
            Margins.Top    = ControlsCount;
            Margins.Bottom = 0;
        }
        break;
    case TabDisplay::NoTabs:
        Margins.Left = Margins.Right = Margins.Bottom = Margins.Top = 0;
        break;
    default:
        LOG_ERROR("Unknwon TAB display mode: %d", TAB_DISPLAY_MODE(Flags));
        break;
    }
}

int TabControlContext::MousePositionToPanel(int x, int y)
{
    int idx;
    switch (TAB_DISPLAY_MODE(this->Flags))
    {
    case TabDisplay::Top:
        if (y > 0)
            return -1;
        idx = (x - 1) / (this->TabTitleSize + 1);
        if (idx >= (int) this->ControlsCount)
            return -1;
        return idx;
    case TabDisplay::Bottom:
        if (y < this->Layout.Height - 1)
            return -1;
        idx = (x - 1) / (this->TabTitleSize + 1);
        if (idx >= (int) this->ControlsCount)
            return -1;
        return idx;
    case TabDisplay::Left:
        if ((x < 0) || (x > (int) this->TabTitleSize) || (y < 1))
            return -1;
        idx = y - 1;
        if (idx >= (int) this->ControlsCount)
            return -1;
        return idx;
    case TabDisplay::List:
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
    case TabDisplay::NoTabs:
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
    int poz      = 1;

    if (onTop)
        params.Y = 0;
    else
        params.Y = this->Layout.Height - 1;

    if ((this->Flags & TabFlags::TransparentBackground) != TabFlags::TransparentBackground)
    {
        if (onTop)
            renderer.FillRectSize(0, 1, this->Layout.Width, this->Layout.Height - 1, ' ', this->Cfg->Tab.PageColor);
        else
            renderer.FillRectSize(0, 0, this->Layout.Width, this->Layout.Height - 1, ' ', this->Cfg->Tab.PageColor);
    }

    if ((this->Flags & TabFlags::TabsBar) == TabFlags::TabsBar)
        renderer.FillHorizontalLineSize(0, params.Y, this->Layout.Width, ' ', this->Cfg->Tab.TabBarColor);

    for (unsigned tr = 0; tr < this->ControlsCount; tr++, poz += (this->TabTitleSize + 1))
    {
        if (this->Controls[tr] == nullptr)
            continue;
        ControlContext* cc = (ControlContext*) this->Controls[tr]->Context;
        if (cc == nullptr)
            continue;

        if (tr == this->CurrentControlIndex)
        {
            params.Color       = this->Cfg->Tab.PageColor;
            params.HotKeyColor = this->Cfg->Tab.PageHotKeyColor;
        }
        else if (tr == static_cast<unsigned>(this->HoveredTabIndex))
        {
            params.Color       = this->Cfg->Tab.HoverColor;
            params.HotKeyColor = this->Cfg->Tab.HoverHotKeyColor;
        }
        else
        {
            params.Color       = this->Cfg->Tab.TabBarColor;
            params.HotKeyColor = this->Cfg->Tab.TabBarHotKeyColor;
        }

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
              this->Cfg->Tab.PageColor);
    if ((this->Flags & TabFlags::TabsBar) == TabFlags::TabsBar)
        renderer.FillRectSize(0, 0, this->TabTitleSize, this->Layout.Height, ' ', this->Cfg->Tab.TabBarColor);

    WriteTextParams params(
          WriteTextFlags::OverwriteColors | WriteTextFlags::SingleLine | WriteTextFlags::HighlightHotKey |
          WriteTextFlags::ClipToWidth | WriteTextFlags::FitTextToWidth);
    params.Width = this->TabTitleSize - 2;
    params.X     = 1;

    for (unsigned int tr = 0; tr < this->ControlsCount; tr++)
    {
        if (this->Controls[tr] == nullptr)
            continue;
        ControlContext* cc = (ControlContext*) this->Controls[tr]->Context;
        if (cc == nullptr)
            continue;
        if (tr == this->CurrentControlIndex)
        {
            params.Color       = this->Cfg->Tab.PageColor;
            params.HotKeyColor = this->Cfg->Tab.PageHotKeyColor;
        }
        else if (tr == static_cast<unsigned>(this->HoveredTabIndex))
        {
            params.Color       = this->Cfg->Tab.HoverColor;
            params.HotKeyColor = this->Cfg->Tab.HoverHotKeyColor;
        }
        else
        {
            params.Color       = this->Cfg->Tab.TabBarColor;
            params.HotKeyColor = this->Cfg->Tab.TabBarHotKeyColor;
        }

        renderer.FillHorizontalLineSize(0, tr + 1, this->TabTitleSize, ' ', params.Color);
        params.HotKeyPosition = cc->HotKeyOffset;
        params.Y              = tr + 1;
        renderer.WriteText(cc->Text, params);
    }
}

void TabControlContext::PaintListPanelTab(Graphics::Renderer& renderer)
{
    if ((this->Flags & TabFlags::TransparentBackground) != TabFlags::TransparentBackground)
        renderer.Clear(' ', this->Cfg->Tab.PageColor);

    WriteTextParams params(
          WriteTextFlags::OverwriteColors | WriteTextFlags::SingleLine | WriteTextFlags::HighlightHotKey |
          WriteTextFlags::ClipToWidth | WriteTextFlags::FitTextToWidth);
    params.Width = this->Layout.Width - 2;
    params.X     = 1;
    int ypoz;
    for (unsigned int tr = 0; tr < this->ControlsCount; tr++)
    {
        if (this->Controls[tr] == nullptr)
            continue;
        ControlContext* cc = (ControlContext*) this->Controls[tr]->Context;
        if (cc == nullptr)
            continue;

        if (tr == this->CurrentControlIndex)
        {
            params.Color       = this->Cfg->Tab.ListSelectedPageColor;
            params.HotKeyColor = this->Cfg->Tab.ListSelectedPageHotKey;
        }
        else if (tr == static_cast<unsigned>(this->HoveredTabIndex))
        {
            params.Color       = this->Cfg->Tab.HoverColor;
            params.HotKeyColor = this->Cfg->Tab.HoverHotKeyColor;
        }
        else
        {
            params.Color       = this->Cfg->Tab.TabBarColor;
            params.HotKeyColor = this->Cfg->Tab.TabBarHotKeyColor;
        }
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
        renderer.Clear(' ', this->Cfg->Tab.PageColor);
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

//===================================================================================================================

TabPage::TabPage(const AppCUI::Utils::ConstString& caption)
    : Control(new ControlContext(), caption, "x:0,y:0,w:100%,h:100%", true)
{
    auto Members   = reinterpret_cast<ControlContext*>(this->Context);
    Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
}

bool TabPage::OnBeforeResize(int, int)
{
    return true;
}

Tab::Tab(std::string_view layout, TabFlags flags, unsigned int tabPageSize)
    : Control(new TabControlContext(), "", layout, false)
{
    tabPageSize = std::min<>(1000U, tabPageSize);
    tabPageSize = std::max<>(10U, tabPageSize);

    auto Members          = reinterpret_cast<TabControlContext*>(this->Context);
    Members->Flags        = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | (unsigned int) flags;
    Members->TabTitleSize = tabPageSize;
    // margin set
    Members->HoveredTabIndex = -1;
    Members->UpdateMargins();
}

bool Tab::SetCurrentTabPageByIndex(unsigned int index)
{
    CREATE_TYPECONTROL_CONTEXT(TabControlContext, Members, false);
    CHECK((index < Members->ControlsCount), false, "Invalid tab page index: %d", index);
    bool res   = false;
    bool found = false;
    for (unsigned int tr = 0; tr < Members->ControlsCount; tr++)
    {
        if (tr == index)
        {
            // ca sa fortez calcularea focusului
            ((ControlContext*) Members->Controls[tr]->Context)->Focused = false;
            Members->Controls[tr]->SetVisible(true);
            Members->Controls[tr]->SetEnabled(true);
            res   = Members->Controls[tr]->SetFocus();
            found = true;
        }
        else
        {
            ((ControlContext*) Members->Controls[tr]->Context)->Focused = false;
            Members->Controls[tr]->SetVisible(false);
        }
    }
    if (Members->Flags && TabFlags::ListView)
    {
        Members->UpdateMargins();
        AppCUI::Application::RecomputeControlsLayout();
    }
    if (found)
        this->RaiseEvent(Event::TabChanged);
    return res;
}
bool Tab::SetCurrentTabPage(Reference<TabPage> page)
{
    unsigned int index = 0xFFFFFFFF;
    CHECK(Control::GetChildIndex(*reinterpret_cast<Reference<Control>*>(&page), index), false, "Fail to find page index in current tab!");
    return SetCurrentTabPageByIndex(index);
}
Reference<Control> Tab::GetCurrentTab()
{
    CREATE_TYPECONTROL_CONTEXT(TabControlContext, Members, nullptr);
    if (Members->CurrentControlIndex >= Members->ControlsCount)
        return nullptr;
    return Members->Controls[Members->CurrentControlIndex];
}

bool Tab::SetTabPageName(unsigned int index, const AppCUI::Utils::ConstString& name)
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
    for (unsigned int tr = 0; tr < Members->ControlsCount; tr++)
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
    AppCUI::Application::RecomputeControlsLayout();
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

void Tab::OnMouseReleased(int, int, AppCUI::Input::MouseButton)
{
    CREATE_TYPECONTROL_CONTEXT(TabControlContext, Members, );
    if (Members->HoveredTabIndex >= 0)
        SetCurrentTabPageByIndex((unsigned int) Members->HoveredTabIndex);
}

bool Tab::OnKeyEvent(AppCUI::Input::Key keyCode, char16_t)
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
    for (unsigned int tr = 0; tr < Members->ControlsCount; tr++)
        if (keyCode == Members->Controls[tr]->GetHotKey())
        {
            SetCurrentTabPageByIndex(tr);
            return true;
        }
    return false;
}

void Tab::Paint(Graphics::Renderer& renderer)
{
    CREATE_TYPECONTROL_CONTEXT(TabControlContext, Members, );

    switch (TAB_DISPLAY_MODE(Members->Flags))
    {
    case TabDisplay::Top:
        Members->PaintTopBottomPanelTab(renderer, true);
        break;
    case TabDisplay::Bottom:
        Members->PaintTopBottomPanelTab(renderer, false);
        break;
    case TabDisplay::Left:
        Members->PaintLeftPanelTab(renderer);
        break;
    case TabDisplay::List:
        Members->PaintListPanelTab(renderer);
        break;
    case TabDisplay::NoTabs:
        Members->PaintNoTabsPanelTab(renderer);
        break;
    }
}

bool Tab::SetTabPageTitleSize(unsigned int newSize)
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
