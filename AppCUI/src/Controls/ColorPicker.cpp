#include "ControlContext.hpp"

namespace AppCUI
{
void ColorPickerContext::PaintHeader(int x, int y, uint32 width, Graphics::Renderer& renderer)
{
    auto* cbc = &this->Cfg->ComboBox.Normal;
    if ((this->Flags & GATTR_ENABLE) == 0)
        cbc = &this->Cfg->ComboBox.Inactive;
    if (this->Focused)
        cbc = &this->Cfg->ComboBox.Focus;
    else if (this->MouseIsOver)
        cbc = &this->Cfg->ComboBox.Hover;

    if (width>5)
    {
        renderer.FillHorizontalLine(x, y, x + (int)width - 6, ' ', cbc->Text);       
        renderer.WriteSingleLineText(x + 3, y, width - 4, ColorUtils::GetColorName(this->color), cbc->Text);
        renderer.WriteSpecialCharacter(
              x + 1, y, SpecialChars::BlockCentered, ColorPair{ this->color, Color::Transparent });
    }
    if (width >= 3)
    {
        renderer.WriteSingleLineText(x + (int)width - 3, y, "   ", cbc->Button);
        renderer.WriteSpecialCharacter(x + (int) width - 2, y, SpecialChars::TriangleDown, cbc->Button);
    }
}
void ColorPickerContext::Paint(Graphics::Renderer& renderer)
{
    PaintHeader(0,0,this->Layout.Width,renderer);
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
}
bool ColorPicker::OnMouseLeave()
{
    NOT_IMPLEMENTED(false);
}
bool ColorPicker::OnMouseEnter()
{
    NOT_IMPLEMENTED(false);
}
bool ColorPicker::OnMouseOver(int x, int y)
{
    NOT_IMPLEMENTED(false);
}
void ColorPicker::OnMousePressed(int x, int y, Input::MouseButton button)
{
}
bool ColorPicker::OnMouseWheel(int x, int y, Input::MouseWheel direction)
{
    NOT_IMPLEMENTED(false);
}
void ColorPicker::OnExpandView(Graphics::Clip& expandedClip)
{
}
void ColorPicker::OnPackView()
{
}

} // namespace AppCUI