#include "../ControlContext.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;
using namespace AppCUI::Input;

#define PANEL_ATTR_BORDER 1024

Panel::Panel(const AppCUI::Utils::ConstString& caption, std::string_view layout)
    : Control(new ControlContext(), caption, layout, false)
{
    auto Members = reinterpret_cast<ControlContext*>(this->Context);
    if (Members->Text.Len()==0)
    {
        Members->Flags = GATTR_VISIBLE | GATTR_ENABLE;
    }
    else
    {
        Members->Flags = GATTR_VISIBLE | GATTR_ENABLE | PANEL_ATTR_BORDER;
        ASSERT(this->SetMargins(1, 1, 1, 1), "Failed to set margins !");
    }
}

void Panel::Paint(Graphics::Renderer& renderer)
{
    CREATE_CONTROL_CONTEXT(this, Members, );
    renderer.Clear(' ', Members->Cfg->Panel.NormalColor);
    if (Members->Flags & PANEL_ATTR_BORDER)
    {
        renderer.DrawRectSize(
              0, 0, Members->Layout.Width, Members->Layout.Height, Members->Cfg->Panel.NormalColor, false);
        if (Members->Layout.Width > 6)
        {
            WriteTextParams params(
                  WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth | WriteTextFlags::OverwriteColors |
                  WriteTextFlags::LeftMargin | WriteTextFlags::RightMargin, TextAlignament::Left);
            params.X              = 3;
            params.Y              = 0;
            params.Color          = Members->Cfg->Panel.NormalColor;
            params.Width          = Members->Layout.Width - 6;
            renderer.WriteText(Members->Text, params);
        }
    }
}