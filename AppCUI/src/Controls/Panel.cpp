#include "ControlContext.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;
using namespace AppCUI::Input;

#define PANEL_ATTR_BORDER 1024

std::unique_ptr<Panel> Panel::Create(const AppCUI::Utils::ConstString& caption, const std::string_view& layout)
{
    INIT_CONTROL(Panel, ControlContext);
    CHECK(me->Init(caption, layout, false), nullptr, "Failed to create panel !");
    Members->Flags = GATTR_VISIBLE | GATTR_ENABLE | PANEL_ATTR_BORDER;
    CHECK(me->SetMargins(1, 1, 1, 1), nullptr, "Failed to set margins !");
    return me;
}
Panel* Panel::Create(Control& parent, const AppCUI::Utils::ConstString& caption, const std::string_view& layout)
{
    auto me = Panel::Create(caption, layout);
    CHECK(me, nullptr, "Fail to create a panel control !");
    return parent.AddControl<Panel>(std::move(me));
}
std::unique_ptr<Panel> Panel::Create(const std::string_view& layout)
{
    INIT_CONTROL(Panel, ControlContext);
    CHECK(me->Init("", layout, false), nullptr, "Failed to create panel !");
    Members->Flags = GATTR_VISIBLE | GATTR_ENABLE;
    return me;
}
Panel* Panel::Create(Control& parent, const std::string_view& layout)
{
    auto me = Panel::Create(layout);
    CHECK(me, nullptr, "Fail to create a panel control !");
    return parent.AddControl<Panel>(std::move(me));
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