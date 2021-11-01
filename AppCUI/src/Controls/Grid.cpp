#include "ControlContext.hpp"

Grid::Grid(std::string_view layout) : Control(new ControlContext(), "", layout, false)
{
    auto Members              = reinterpret_cast<ControlContext*>(this->Context);
    Members->Layout.MinHeight = 1;
    Members->Layout.MinWidth  = 1;
    Members->Flags            = GATTR_ENABLE | GATTR_VISIBLE;
}

void Grid::Paint(Graphics::Renderer& renderer)
{
    renderer.WriteSingleLineText(2, 0, "Work in progress...", ColorPair{ Color::White, Color::Transparent });
}
