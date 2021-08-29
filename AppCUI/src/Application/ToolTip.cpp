#include <Internal.hpp>

using namespace AppCUI::Internal;


ToolTip::ToolTip()
{
    this->Visible = false;
    this->Cfg     = AppCUI::Application::GetAppConfig();
}
void ToolTip::Show(const AppCUI::Utils::ConstString& text)
{
        
}
void ToolTip::Hide()
{
    this->Visible = false;
}
void ToolTip::Paint(AppCUI::Graphics::Renderer& renderer)
{
    if (!Visible)
        return;
}