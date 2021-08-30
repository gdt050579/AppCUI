#include <Internal.hpp>

using namespace AppCUI::Internal;


ToolTipController::ToolTipController()
{
    this->Visible = false;
    this->Cfg     = AppCUI::Application::GetAppConfig();
}
bool ToolTipController::Show(const AppCUI::Utils::ConstString& text, AppCUI::Graphics::Rect& objRect)
{
    NOT_IMPLEMENTED(false);
}
void ToolTipController::Hide()
{
    this->Visible = false;
}
void ToolTipController::Paint(AppCUI::Graphics::Renderer& renderer)
{
    if (!Visible)
        return;
}