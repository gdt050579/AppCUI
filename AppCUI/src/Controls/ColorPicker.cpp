#include "ControlContext.hpp"

namespace AppCUI
{
constexpr int32 COLORPICEKR_HEIGHT     = 7;
constexpr uint32 NO_COLOR_OBJECT       = 0xFFFFFFFF;
constexpr static Color reverse_color[] = {
    Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::White, Color::Black,
    Color::Black, Color::White, Color::Black, Color::Black, Color::White, Color::White, Color::Black, Color::Black,
};
void ColorPickerContext::OnExpandView(Graphics::Clip& expandedClip)
{
    Size size;
    if (!AppCUI::Application::GetApplicationSize(size))
        return;
    expandedClip.ClipRect.Height = COLORPICEKR_HEIGHT;
    this->headerYOffset          = 0;
    this->yOffset                = 1;
    this->colorObject            = NO_COLOR_OBJECT;
    if (expandedClip.ScreenPosition.Y + COLORPICEKR_HEIGHT >= (int32) size.Height)
    {
        this->headerYOffset = COLORPICEKR_HEIGHT - 1;
        this->yOffset       = 0;
        expandedClip.ScreenPosition.Y -= this->headerYOffset;
        expandedClip.ClipRect.Y = expandedClip.ScreenPosition.Y;
    }
}
void ColorPickerContext::PaintHeader(int x, int y, uint32 width, Graphics::Renderer& renderer)
{
    auto* cbc = &this->Cfg->ComboBox.Normal;
    if ((this->Flags & GATTR_ENABLE) == 0)
        cbc = &this->Cfg->ComboBox.Inactive;
    if (this->Focused)
        cbc = &this->Cfg->ComboBox.Focus;
    else if (this->MouseIsOver)
        cbc = &this->Cfg->ComboBox.Hover;

    if (width > 5)
    {
        renderer.FillHorizontalLine(x, y, x + (int) width - 6, ' ', cbc->Text);
        renderer.WriteSingleLineText(x + 3, y, width - 4, ColorUtils::GetColorName(this->color), cbc->Text);
        renderer.WriteSpecialCharacter(
              x + 1, y, SpecialChars::BlockCentered, ColorPair{ this->color, Color::Transparent });
    }
    if (width >= 3)
    {
        renderer.WriteSingleLineText(x + (int) width - 3, y, "   ", cbc->Button);
        renderer.WriteSpecialCharacter(x + (int) width - 2, y, SpecialChars::TriangleDown, cbc->Button);
    }
}
void ColorPickerContext::PaintColorBox(Graphics::Renderer& renderer)
{
    const auto col = Cfg->ComboBox.Focus.Text;
    renderer.FillRect(0, this->yOffset, this->Layout.Width - 1, this->yOffset + COLORPICEKR_HEIGHT - 2, ' ', col);
    // draw colors
    for (auto y = 0U; y < 4; y++)
    {
        for (auto x = 0U; x < 4; x++)
        {
            auto c = static_cast<Color>(y * 4 + x);
            renderer.FillHorizontalLineSize(x * 3 + 1, y + 1 + this->yOffset, 3, ' ', ColorPair{ Color::Black, c });
            if (c == color)
            {
                auto c2 = reverse_color[y * 4 + x];
                renderer.WriteSpecialCharacter(
                      x * 3 + 2, y + 1 + this->yOffset, SpecialChars::CheckMark, ColorPair{ c2, c });
            }
            if (y * 4 + x == colorObject)
            {
                auto c2 = reverse_color[y * 4 + x];
                renderer.WriteSpecialCharacter(
                      x * 3 + 1, y + 1 + this->yOffset, SpecialChars::TriangleRight, ColorPair{ c2, c });
                renderer.WriteSpecialCharacter(
                      x * 3 + 3, y + 1 + this->yOffset, SpecialChars::TriangleLeft, ColorPair{ c2, c });
                renderer.SetCursor(x * 3 + 2, y + 1 + this->yOffset);
            }
        }
    }
    if (colorObject == (uint32)Color::Transparent)
    {
        renderer.WriteSingleLineText(15, 1 + this->yOffset, "[ ] Transparent", Cfg->ComboBox.Focus.Button);
        renderer.SetCursor(16, 1 + this->yOffset);
    }
    else
    {
        renderer.WriteSingleLineText(15, 1 + this->yOffset, "[ ] Transparent", col);
    }
    
    if (color == Color::Transparent)
        renderer.WriteSpecialCharacter(16, 1 + this->yOffset, SpecialChars::CheckMark, Cfg->ComboBox.Focus.Button);
    renderer.DrawVerticalLine(13, 1 + this->yOffset, 4 + this->yOffset, col, true);
    renderer.DrawRect(0, this->yOffset, this->Layout.Width - 1, this->yOffset + COLORPICEKR_HEIGHT - 2, col, false);
}
void ColorPickerContext::Paint(Graphics::Renderer& renderer)
{
    PaintHeader(0, this->headerYOffset, this->Layout.Width, renderer);
    if (this->Flags & GATTR_EXPANDED)
        PaintColorBox(renderer);
}
uint32 ColorPickerContext::MouseToObject(int x, int y)
{
    if (!(this->Flags & GATTR_EXPANDED))
        return NO_COLOR_OBJECT;
    if ((x > 0) && (x < 13) && (y > this->yOffset) && (y < this->yOffset + 5))
        return (((x - 1) / 3) + (y - (this->yOffset + 1)) * 4);
    if ((y == 1 + this->yOffset) && (x >= 15) && (x <= 29))
        return (uint32) (Color::Transparent);
    return NO_COLOR_OBJECT;
}
bool ColorPickerContext::OnMouseOver(int x, int y)
{
    auto obj = MouseToObject(x, y);
    if (obj != this->colorObject)
    {
        this->colorObject = obj;
        return true;
    }
    return false;
}
void ColorPickerContext::OnMousePressed(int x, int y, Input::MouseButton button)
{
    auto obj = MouseToObject(x, y);
    if (obj != NO_COLOR_OBJECT)
        this->color = static_cast<Color>((uint8)obj);
}
ColorPicker::ColorPicker(string_view layout, Graphics::Color _color)
    : Control(new ColorPickerContext(), "", layout, false)
{
    auto Members              = reinterpret_cast<ColorPickerContext*>(this->Context);
    Members->Layout.MinWidth  = 7;
    Members->Layout.MinHeight = 1;
    Members->Layout.MaxHeight = 1;
    Members->Flags            = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
    Members->color            = _color;
    Members->headerYOffset    = 0;
    Members->yOffset          = 1;
    Members->colorObject      = NO_COLOR_OBJECT;
}
ColorPicker::~ColorPicker()
{
}
void ColorPicker::Paint(Graphics::Renderer& renderer)
{
    reinterpret_cast<ColorPickerContext*>(this->Context)->Paint(renderer);
}
bool ColorPicker::OnKeyEvent(Input::Key keyCode, char16 UnicodeChar)
{
    NOT_IMPLEMENTED(false);
}
void ColorPicker::OnHotKey()
{
    SetChecked(!IsChecked());
    if (IsChecked())
        this->ExpandView();
    else
    {
        this->PackView();
    }
}
bool ColorPicker::OnMouseLeave()
{
    return true;
}
bool ColorPicker::OnMouseEnter()
{
    return true;
}
bool ColorPicker::OnMouseOver(int x, int y)
{
    return reinterpret_cast<ColorPickerContext*>(this->Context)->OnMouseOver(x, y);
}
void ColorPicker::OnMousePressed(int x, int y, Input::MouseButton button)
{
    reinterpret_cast<ColorPickerContext*>(this->Context)->OnMousePressed(x, y, button);
    OnHotKey();
}
void ColorPicker::OnExpandView(Graphics::Clip& expandedClip)
{
    reinterpret_cast<ColorPickerContext*>(this->Context)->OnExpandView(expandedClip);
}
void ColorPicker::OnPackView()
{
    reinterpret_cast<ColorPickerContext*>(this->Context)->headerYOffset = 0; // reset position
}

} // namespace AppCUI