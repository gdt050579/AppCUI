#include <Internal.hpp>

using namespace AppCUI::Internal;
using namespace AppCUI::Graphics;


ToolTipController::ToolTipController()
{
    this->Visible = false;
    this->Cfg     = nullptr;
}
bool ToolTipController::Show(const AppCUI::Utils::ConstString& text, AppCUI::Graphics::Rect& objRect, int screenWidth, int screenHeight)
{
    Visible = false;
    // update Cfg
    if (!this->Cfg)
        this->Cfg = AppCUI::Application::GetAppConfig();
    CHECK(Text.Set(text), false, "Fail to copy text");
    // compute best size
    auto p        = Text.GetBuffer();
    auto e        = p + Text.Len();
    int nrLines   = 0;
    int maxWidth  = screenWidth / 2;
    int w         = 0;
    int bestWidth = 0;
    while (p<e)
    {
        if (p->Code ==NEW_LINE_CODE)
        {
            bestWidth = std::max<>(bestWidth, w);
            p++;
            w = 0;
            if (p < e)
                nrLines++;
            continue;
        }
        p++;
        w++;
        if (w>=maxWidth)
        {
            bestWidth = maxWidth;
            w         = 0;
            if (p < e)
                nrLines++;
        }
    }
    if (w>0)
    {
        bestWidth = std::max<>(bestWidth, w);
        nrLines++;
    }
    // max number of lines must not be bigger than 25% of the height
    nrLines   = std::min<>(nrLines, screenHeight / 4);
    nrLines   = std::max<>(nrLines, 1);   // minimum one line  (sanity check)
    bestWidth = std::max<>(bestWidth, 5); // minimum 5 chars width (sanity check)
    bestWidth += 2; // one character padding (left & right)

    // set TextParams
    if (nrLines == 1)
        TxParams.Flags = WriteTextFlags::OverwriteColors | WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth;
    else
        TxParams.Flags = WriteTextFlags::OverwriteColors | WriteTextFlags::MultipleLines | WriteTextFlags::WrapToWidth;

    // find best position  (prefer on-top)
    if (objRect.GetTop()>=(nrLines+1))
    {
        const int cx    = objRect.GetCenterX();
        int x           = cx - bestWidth / 2;
        auto bestX      = x;
        x               = std::min<>(x, screenWidth - bestWidth);
        x               = std::max<>(x, 0);
        ScreenClip.Set(x, objRect.GetTop() - (nrLines+1), bestWidth, nrLines+1);
        TextRect.Create(0, 0, bestWidth, nrLines, Alignament::TopLeft);
        Arrow.Set(bestWidth / 2 + (bestX - x), nrLines);
        TxParams.X     = 1;
        TxParams.Y     = 0;
        TxParams.Color = Cfg->ToolTip.Text;
        TxParams.Width = bestWidth-2;
        ArrowChar      = SpecialChars::ArrowDown;

        Visible = true; 
        return true;
    }
    // check bottom position
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
    
    renderer.FillRect(TextRect.GetLeft(), TextRect.GetTop(), TextRect.GetRight(), TextRect.GetBottom(), ' ', Cfg->ToolTip.Text);
    renderer.WriteSpecialCharacter(Arrow.X, Arrow.Y, ArrowChar, ColorPair{ Color::Aqua, Color::Black });
    renderer.WriteText(this->Text, TxParams);
}