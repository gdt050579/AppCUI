#include <Internal.hpp>

using namespace AppCUI::Internal;
using namespace AppCUI::Graphics;

ToolTipController::ToolTipController()
{
    this->Visible = false;
    this->Cfg     = nullptr;
}
bool ToolTipController::Show(
      const AppCUI::Utils::ConstString& text, AppCUI::Graphics::Rect& objRect, int screenWidth, int screenHeight)
{
    Visible = false;
    // update Cfg
    if (!this->Cfg)
        this->Cfg = AppCUI::Application::GetAppConfig();
    // find best position
    if (objRect.GetTop() > 3)
    {
        CHECK(Text.Set(text), false, "Fail to copy text");
        unsigned int sz = (Text.Len() / 2) + 1;
        const int cx    = objRect.GetCenterX();
        if (cx >= (screenWidth / 2))
            sz = std::min<>(sz, (unsigned int) (screenWidth - cx));
        else
            sz = std::min<>(sz, (unsigned int) cx);
        ScreenClip.Set(cx - sz, objRect.GetTop() - 3, sz * 2 + 1, 3);
        Visible = true;
    }
    return Visible;
}
void ToolTipController::Hide()
{
    this->Visible = false;
}
void ToolTipController::Paint(AppCUI::Graphics::Renderer& renderer)
{
    if (!Visible)
        return;
    renderer.Clear(' ', Cfg->ToolTip.Border);
    renderer.DrawRectSize(0, 0, ScreenClip.ClipRect.Width, ScreenClip.ClipRect.Height, Cfg->ToolTip.Border, false);
    WriteTextParams params(
          WriteTextFlags::SingleLine | WriteTextFlags::OverwriteColors | WriteTextFlags::ClipToWidth |
          WriteTextFlags::FitTextToWidth);
    params.X     = 1;
    params.Y     = 1;
    params.Width = ScreenClip.ClipRect.Width - 2;
    params.Color = Cfg->ToolTip.Text;
    renderer.WriteText(this->Text, params);
}