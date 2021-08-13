#include "ControlContext.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Input;
using namespace AppCUI::Graphics;

#define TAB_DISPLAY_MODE_TOP    0
#define TAB_DISPLAY_MODE_BOTTOM 1
#define TAB_DISPLAY_MODE_LEFT   2
#define TAB_DISPLAY_MODE_LIST   3

#define TAB_DISPLAY_MODE(flags) (((flags) >> 8) & 0xF)

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
            this->Margins.Bottom = this->ControlsCount - this->CurrentControlIndex;
        }
        else
        {
            this->Margins.Top = this->ControlsCount;
            ;
            this->Margins.Bottom = 0;
        }
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
    int borderX  = -1;

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
        renderer.DrawHorizontalLineSize(0, params.Y, this->Layout.Width, ' ', this->Cfg->Tab.TabBarColor);




    for (unsigned int tr = 0; tr < this->ControlsCount; tr++, poz += (this->TabTitleSize + 1))
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
        else if (tr == this->HoveredTabIndex)
        {
            params.Color       = this->Cfg->Tab.HoverColor;
            params.HotKeyColor = this->Cfg->Tab.HoverHotKeyColor;
        }
        else
        {
            params.Color       = this->Cfg->Tab.TabBarColor;
            params.HotKeyColor = this->Cfg->Tab.TabBarHotKeyColor;
        }

        renderer.DrawHorizontalLineSize(poz, params.Y, this->TabTitleSize, ' ', params.Color);
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
        else if (tr == this->HoveredTabIndex)
        {
            params.Color       = this->Cfg->Tab.HoverColor;
            params.HotKeyColor = this->Cfg->Tab.HoverHotKeyColor;
        }
        else
        {
            params.Color       = this->Cfg->Tab.TabBarColor;
            params.HotKeyColor = this->Cfg->Tab.TabBarHotKeyColor;
        }

        renderer.DrawHorizontalLineSize(0, tr + 1, this->TabTitleSize, ' ', params.Color);
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
        else if (tr == this->HoveredTabIndex)
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
        renderer.DrawHorizontalLineSize(0, ypoz, this->Layout.Width, ' ', params.Color);
        params.HotKeyPosition = cc->HotKeyOffset;
        params.Y              = ypoz;
        renderer.WriteText(cc->Text, params);
    }
}
bool NextTab(Tab* t)
{
    CREATE_CONTROL_CONTEXT(t, Members, false);
    CHECK(Members->ControlsCount > 0, false, "No tab pages available !");
    return t->SetCurrentTabPage(((Members->CurrentControlIndex + 1) % Members->ControlsCount));
}
bool PreviousTab(Tab* t)
{
    CREATE_CONTROL_CONTEXT(t, Members, false);
    CHECK(Members->ControlsCount > 0, false, "No tab pages available !");
    return t->SetCurrentTabPage(
          ((Members->CurrentControlIndex + (Members->ControlsCount - 1)) % Members->ControlsCount));
}
//===================================================================================================================
bool TabPage::Create(Control* parent, const AppCUI::Utils::ConstString& caption)
{
    CONTROL_INIT_CONTEXT(ControlContext);
    CHECK(Init(parent, caption, "x:0,y:0,w:1,h:1", true), false, "Unable to create tab page !");
    CREATE_CONTROL_CONTEXT(this, Members, false);
    Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
    return true;
}
bool TabPage::OnBeforeResize(int newWidth, int newHeight)
{
    return true;
}
bool Tab::Create(Control* parent, const std::string_view& layout, TabFlags flags, unsigned int tabPageSize)
{
    CHECK(tabPageSize >= 10, false, "Tab page title size should be bigger than 10");
    CHECK(tabPageSize < 1000, false, "Tab page title size should be smaller than 1000");
    CONTROL_INIT_CONTEXT(TabControlContext);
    CHECK(Init(parent, "", layout, false), false, "Unable to create tab control !");
    CREATE_TYPECONTROL_CONTEXT(TabControlContext, Members, false);
    Members->Flags        = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | (unsigned int) flags;
    Members->TabTitleSize = tabPageSize;
    // margin set
    Members->currentTab      = nullptr;
    Members->HoveredTabIndex = -1;
    Members->UpdateMargins();
    return true;
}
bool Tab::SetCurrentTabPage(unsigned int index)
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
            res                 = Members->Controls[tr]->SetFocus();
            Members->currentTab = Members->Controls[tr];
            found               = true;
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
        this->RaiseEvent(Event::EVENT_TAB_CHANGED);
    return res;
}
Control* Tab::GetCurrentTab()
{
    CREATE_TYPECONTROL_CONTEXT(TabControlContext, Members, nullptr);
    return Members->currentTab;
}
bool Tab::SetTabPageName(unsigned int index, const AppCUI::Utils::ConstString& name)
{
    CREATE_TYPECONTROL_CONTEXT(TabControlContext, Members, false);
    CHECK((index < Members->ControlsCount), false, "Invalid tab index: %d", index);
    CHECK(Members->Controls[index]->SetText(name, true), false, "");
    return true;
}
void Tab::OnAfterResize(int newWidth, int newHeight)
{
    CREATE_TYPECONTROL_CONTEXT(TabControlContext, Members, );

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
void Tab::OnMouseReleased(int x, int y, AppCUI::Input::MouseButton button)
{
    CREATE_TYPECONTROL_CONTEXT(TabControlContext, Members, );
    if (Members->HoveredTabIndex >= 0)
        SetCurrentTabPage((unsigned int) Members->HoveredTabIndex);
}
bool Tab::OnKeyEvent(AppCUI::Input::Key keyCode, char AsciiCode)
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
            SetCurrentTabPage(tr);
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
void Tab::OnAfterAddControl(Control* ctrl)
{
    if (ctrl == nullptr)
        return;
    // ii fac resize
    CREATE_TYPECONTROL_CONTEXT(TabControlContext, Members, );
    CREATE_CONTROL_CONTEXT(ctrl, cMembers, );
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
