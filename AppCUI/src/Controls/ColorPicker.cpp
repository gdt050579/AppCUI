#include "ControlContext.hpp"

namespace AppCUI::Controls
{
ColorPicker::ColorPicker(string_view layout, Graphics::Color _color)
    : Control(new ColorPickerContext(), "", layout, false)
{
    auto Members   = reinterpret_cast<ColorPickerContext*>(this->Context);
    Members->color = _color;
}
ColorPicker::~ColorPicker()
{
}
void ColorPicker::Paint(Graphics::Renderer& renderer)
{
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

} // namespace AppCUI::Controls