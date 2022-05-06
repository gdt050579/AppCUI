#include "ControlContext.hpp"
using namespace AppCUI::Utils;

namespace AppCUI::Controls
{

Graphics::CharacterBuffer null_column_reference; // use this as std::option<const T&> is not available yet

#define ICH         ((ColumnsHeaderViewControlContext*) (this->Context))
#define NULL_COLUMN Column(nullptr, 0)

ColumnsHeaderView::ColumnsHeaderView(string_view layout, std::initializer_list<ConstString> columnsList, ColumnsHeaderViewFlags flags)
    : Control(new ColumnsHeaderViewControlContext(this, flags), "", layout, false)
{
    auto Members = (ColumnsHeaderViewControlContext*) this->Context;
}
ColumnsHeaderView::~ColumnsHeaderView()
{
    if (this->Context)
        delete (ColumnsHeaderViewControlContext*) (this->Context);
    this->Context = nullptr;
}
Column ColumnsHeaderView::AddColumn(const ConstString columnFormat)
{
    ConstStringObject obj(columnFormat);
    KeyValueParser parser;
    if ((obj.Encoding == StringEncoding::Ascii) || (obj.Encoding == StringEncoding::UTF8))
    {
        CHECK(parser.Parse(string_view((const char*) obj.Data, obj.Length)), NULL_COLUMN, "");
        CHECK(ICH->Header.Add(parser, false), NULL_COLUMN, "");
        return { this->Context, ICH->Header.GetColumnsCount() - 1 };
    }
    else if (obj.Encoding == StringEncoding::Unicode16)
    {
        CHECK(parser.Parse(u16string_view((const char16*) obj.Data, obj.Length)), NULL_COLUMN, "");
        CHECK(ICH->Header.Add(parser, true), NULL_COLUMN, "");
        return { this->Context, ICH->Header.GetColumnsCount() - 1 };
    }
    else
    {
        RETURNERROR(NULL_COLUMN, "Current string formate (%d) is not supported", obj.Encoding);
    }
}
bool ColumnsHeaderView::AddColumns(std::initializer_list<ConstString> list)
{
    const auto newReservedCapacity = ((list.size() + ICH->Header.GetColumnsCount()) | 7) + 1; // align to 8 columns
    ICH->Header.Reserve((uint32)newReservedCapacity);
    for (auto& col : list)
    {
        CHECK(AddColumn(col).IsValid(), false, "");
    }
    return true;
}
Column ColumnsHeaderView::GetColumn(uint32 index)
{
    CHECK(index < ICH->Header.GetColumnsCount(), NULL_COLUMN, "");
    return { this->Context, index };
}
uint32 ColumnsHeaderView::GetColumnCount() const
{
    return ICH->Header.GetColumnsCount();
}
void ColumnsHeaderView::DeleteAllColumns()
{
    ICH->Header.DeleteAllColumns();
}
bool ColumnsHeaderView::DeleteColumn(uint32 columnIndex)
{
    ICH->Header.DeleteColumn(columnIndex);
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
bool ColumnsHeaderView::OnMouseOver(int x, int y)
{
    return ICH->Header.OnMouseOver(x, y);
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

#define HEADERREF ((ColumnsHeaderViewControlContext*) this->context)->Header
#define COLUMNREF ((ColumnsHeaderViewControlContext*) this->context)->Header[this->index]
#define VALIDATE_COLUMN(result)                                                                                        \
    if (this->context == nullptr)                                                                                      \
        return result;                                                                                                 \
    CHECK(index < HEADERREF.GetColumnsCount(),                                                                         \
          result,                                                                                                      \
          "Invalid column index:%d (should be smaller than %d)",                                                       \
          index,                                                                                                       \
          HEADERREF.GetColumnsCount());

bool Column::IsValid() const
{
    VALIDATE_COLUMN(false);
    return true;
}
bool Column::SetText(const ConstString& text)
{
    VALIDATE_COLUMN(false);
    if (COLUMNREF.SetName(text))
    {
        HEADERREF.RecomputeColumnsSizes();
        return true;
    }
    return false;
}
const Graphics::CharacterBuffer& Column::GetText() const
{
    null_column_reference.Destroy();
    VALIDATE_COLUMN(null_column_reference);
    return COLUMNREF.name;
}
bool Column::SetAlignament(TextAlignament align)
{
    VALIDATE_COLUMN(false);
    return COLUMNREF.SetAlign(align);
}
bool Column::SetWidth(uint32 width)
{
    VALIDATE_COLUMN(false);
    COLUMNREF.SetWidth(width);
    HEADERREF.RecomputeColumnsSizes();
    return true;
}
bool Column::SetWidth(float percentage)
{
    VALIDATE_COLUMN(false);
    COLUMNREF.SetWidth(percentage);
    HEADERREF.RecomputeColumnsSizes();
    return true;
}
bool Column::SetWidth(double percentage)
{
    VALIDATE_COLUMN(false);
    COLUMNREF.SetWidth(percentage);
    HEADERREF.RecomputeColumnsSizes();
    return true;
}
uint32 Column::GetWidth() const
{
    VALIDATE_COLUMN(0);
    return COLUMNREF.width;
}

#undef COLUMNREF
#undef HEADERREF
#undef VALIDATE_COLUMN
} // namespace AppCUI::Controls