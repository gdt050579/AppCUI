#include "ControlContext.hpp"
using namespace AppCUI::Utils;


namespace AppCUI::Controls
{

//============================================================================================
#define ICH ((ColumnsHeaderViewControlContext*) (this->Context))
;
ColumnsHeaderView::ColumnsHeaderView(string_view layout, std::initializer_list<ConstString> columnsList)
    : Control(new ColumnsHeaderViewControlContext(this), "", layout, false)
{
    auto Members                           = (ColumnsHeaderViewControlContext*) this->Context;

}
ColumnsHeaderView::~ColumnsHeaderView()
{
    if (this->Context)
        delete (ColumnsHeaderViewControlContext*) (this->Context);
    this->Context = nullptr;
}
bool ColumnsHeaderView::AddColumn(const ConstString columnFormat)
{
    ConstStringObject obj(columnFormat);
    KeyValueParser parser;
    if ((obj.Encoding == StringEncoding::Ascii) || (obj.Encoding == StringEncoding::UTF8))
    {
        CHECK(parser.Parse(string_view((const char*) obj.Data, obj.Length)), false, "");
        CHECK(ICH->Header.Add(parser, false), false, "");
        return true;
    }
    else if (obj.Encoding == StringEncoding::Unicode16)
    {
        CHECK(parser.Parse(u16string_view((const char16*) obj.Data, obj.Length)), false, "");
        CHECK(ICH->Header.Add(parser, true), false, "");
        return true;
    }
    else
    {
        RETURNERROR(false, "Current string formate (%d) is not supported", obj.Encoding);
    }
}
bool ColumnsHeaderView::AddColumns(std::initializer_list<ConstString> list)
{
    const auto newReservedCapacity = ((list.size() + ICH->Header.GetColumnsCount()) | 7) + 1; // align to 8 columns
    ICH->Header.Reserve(newReservedCapacity);
    for (auto& col : list)
    {
        CHECK(AddColumn(col), false, "");
    }
    return true;
}

void ColumnsHeaderView::Paint(Graphics::Renderer& renderer)
{
    ICH->Header.Paint(renderer);
}
bool ColumnsHeaderView::OnKeyEvent(Input::Key keyCode, char16 UnicodeChar)
{
    return ICH->Header.OnKeyEvent(keyCode, UnicodeChar);
}
void ColumnsHeaderView::OnMouseReleased(int x, int y, Input::MouseButton button)
{
}
void ColumnsHeaderView::OnMousePressed(int x, int y, Input::MouseButton button)
{
}
bool ColumnsHeaderView::OnMouseDrag(int x, int y, Input::MouseButton button)
{
    return ICH->Header.OnMouseDrag(x, y, button);
}
bool ColumnsHeaderView::OnMouseWheel(int x, int y, Input::MouseWheel direction)
{
}
bool ColumnsHeaderView::OnMouseOver(int x, int y)
{
    return ICH->Header.OnMouseOver(x,y);
}
bool ColumnsHeaderView::OnMouseLeave()
{
    return ICH->Header.OnMouseLeave();
}
void ColumnsHeaderView::OnLoseFocus()
{
    ICH->Header.OnLoseFocus();
}

#undef ICH
} // namespace AppCUI::Controls