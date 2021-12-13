#include "ControlContext.hpp"

namespace AppCUI
{
PropertyList::PropertyList(string_view layout, Reference<PropertiesInterface> object)
    : Control(new ControlContext(), "", layout, false)
{
}
PropertyList::~PropertyList()
{
}
void PropertyList::Paint(Graphics::Renderer& renderer)
{
}
bool PropertyList::OnKeyEvent(Input::Key keyCode, char16 UnicodeChar)
{
    NOT_IMPLEMENTED(false);
}
void PropertyList::OnMouseReleased(int x, int y, Input::MouseButton button)
{
}
void PropertyList::OnMousePressed(int x, int y, Input::MouseButton button)
{
}
bool PropertyList::OnMouseDrag(int x, int y, Input::MouseButton button)
{
    NOT_IMPLEMENTED(false);
}
bool PropertyList::OnMouseWheel(int x, int y, Input::MouseWheel direction)
{
    NOT_IMPLEMENTED(false);
}
bool PropertyList::OnMouseOver(int x, int y)
{
    NOT_IMPLEMENTED(false);
}
bool PropertyList::OnMouseLeave()
{
    NOT_IMPLEMENTED(false);
}

void PropertyList::OnUpdateScrollBars()
{
}

} // namespace AppCUI::Controls