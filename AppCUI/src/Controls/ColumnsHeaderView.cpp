#include "ControlContext.hpp"
using namespace AppCUI::Utils;

namespace AppCUI::Controls
{

Graphics::CharacterBuffer null_column_reference; // use this as std::option<const T&> is not available yet

#define ICH         ((ColumnsHeaderViewControlContext*) (this->Context))
#define NULL_COLUMN Column(nullptr, 0)

ColumnsHeaderView::ColumnsHeaderView(
      string_view layout, std::initializer_list<ConstString> columnsList, ColumnsHeaderViewFlags flags)
    : ColumnsHeaderView(new ColumnsHeaderViewControlContext(this, columnsList, flags), layout)
{
}
// context MUST be a derivate of ColumnsHeaderViewControlContext
ColumnsHeaderView::ColumnsHeaderView(void* context, string_view layout) : Control(context, "", layout, false)
{
}

ColumnsHeaderView::~ColumnsHeaderView()
{
    if (this->Context)
        delete (ColumnsHeaderViewControlContext*) (this->Context);
    this->Context = nullptr;
}
Column ColumnsHeaderView::AddColumn(const ConstString columnFormat)
{
    CHECK(ICH->Header.AddColumn(columnFormat), NULL_COLUMN, "");
    return { this->Context, ICH->Header.GetColumnsCount() - 1 };
}
bool ColumnsHeaderView::AddColumns(std::initializer_list<ConstString> list)
{
    return ICH->Header.AddColumns(list);
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
void ColumnsHeaderView::DeleteColumn(uint32 columnIndex)
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
bool Column::IsColumnValueSearchable() const
{
    VALIDATE_COLUMN(false);
    return ((COLUMNREF.flags) & InternalColumnFlags::AllowValueCopy) != InternalColumnFlags::None;
}
bool Column::IsColumnValueCopyable() const
{
    VALIDATE_COLUMN(false);
    return ((COLUMNREF.flags) & InternalColumnFlags::SearcheableValue) != InternalColumnFlags::None;
}

#undef COLUMNREF
#undef HEADERREF
#undef VALIDATE_COLUMN
} // namespace AppCUI::Controls