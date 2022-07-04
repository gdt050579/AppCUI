#include "ControlContext.hpp"
using namespace AppCUI::Utils;

namespace AppCUI::Controls
{

Graphics::CharacterBuffer null_column_reference; // use this as std::option<const T&> is not available yet

#define ICH         ((ColumnsHeaderViewControlContext*) (this->Context))
#define NULL_COLUMN Column(nullptr, 0)

constexpr uint8 TABLE_BUILDER_STATE_NONE      = 0;
constexpr uint8 TABLE_BUILDER_STATE_STARTED   = 1;
constexpr uint8 TABLE_BUILDER_STATE_ROW_ADDED = 2;

bool AddTextAsHTMLFormat(UnicodeStringBuilder& output, ConstString text)
{
    LocalUnicodeStringBuilder<512> temp;
    NumericFormatter n;
    CHECK(temp.Set(text), false, "");
    for (auto i : temp.ToStringView())
    {
        if (i == 0)
            continue;
        if ((i == '%') || (i == '<') || (i == '>') || (i == ';') || (i > 126))
        {
            CHECK(output.Add("&#"), false, "");
            CHECK(output.Add(n.ToDec((uint32) i)), false, "");
            CHECK(output.AddChar(';'), false, "");
        }
        else
        {
            CHECK(output.AddChar(i), false, "");
        }
    }
    return true;
}
bool AddTextAsCSVFormat(UnicodeStringBuilder& output, ConstString text)
{
    LocalUnicodeStringBuilder<512> temp;
    CHECK(temp.Set(text), false, "");
    // first check if a special character is present
    bool specialFormat = false;
    for (auto i : temp.ToStringView())
    {
        specialFormat |= ((i == '"') || (i == ',') || (i == '\n') || (i == '\r') || (i == '\t'));
    }
    if (specialFormat)
    {
        CHECK(output.Add("\""), false, "");
        for (auto i : temp.ToStringView())
        {
            if (i == 0)
                continue;
            if (i == '"')
            {
                CHECK(output.Add("\"\""), false, "");
            }
            else
            {
                CHECK(output.AddChar(i), false, "");
            }
        }
        CHECK(output.Add("\""), false, "");
    }
    else
    {
        CHECK(output.Add(temp), false, "");
    }

    return true;
}
ColumnsHeaderView::TableBuilder::TableBuilder(ColumnsHeaderView* obj, UnicodeStringBuilder& _output)
    : output(_output), Context(obj->Context), state(TABLE_BUILDER_STATE_NONE)
{
    output.Clear();
}
bool ColumnsHeaderView::TableBuilder::Start()
{
    CHECK(state == TABLE_BUILDER_STATE_NONE,
          false,
          "Fail to start the builder (you have already called this method) !");
    switch (ICH->CopyToClipboard.format)
    {
    case CopyClipboardFormat::CSV:
    case CopyClipboardFormat::TextWithTabs:
        // nothing to do
        break;
    case CopyClipboardFormat::HTML:
        CHECK(output.Add("<html><table>"), false, "");
        break;
    default:
        RETURNERROR(false, "Unknwon clipboard format");
    }
    state = TABLE_BUILDER_STATE_STARTED;

    // check flags
    if ((ICH->CopyToClipboard.flags & CopyClipboardFlags::CopyHeader)!=CopyClipboardFlags::None)
    {
        // add header
        CHECK(AddNewRow(), false, "");
        for (uint32 index = 0; index < ICH->Header.GetColumnsCount();index++)
        {
            CHECK(AddString(index, ICH->Header[index].name), false, "");
        }
    }


    return true;
}
bool ColumnsHeaderView::TableBuilder::AddNewRow()
{
    CHECK(state != TABLE_BUILDER_STATE_NONE, false, "Fail to add new row. Have you call `Start()` method ?");
    switch (ICH->CopyToClipboard.format)
    {
    case CopyClipboardFormat::CSV:
    case CopyClipboardFormat::TextWithTabs:
        if (state != TABLE_BUILDER_STATE_STARTED) // no raw added
        {
            CHECK(output.Add("\n"), false, "");
        }
        break;
    case CopyClipboardFormat::HTML:
        if (state == TABLE_BUILDER_STATE_STARTED) // first raw
        {
            CHECK(output.Add("<tr>"), false, "");
        }
        else
        {
            CHECK(output.Add("</tr><tr>"), false, "");
        }
        break;
    default:
        RETURNERROR(false, "Unknwon clipboard format");
    }
    state = TABLE_BUILDER_STATE_ROW_ADDED;
    return true;
}
bool ColumnsHeaderView::TableBuilder::AddString(uint32 columnIndex, ConstString text)
{
    CHECK(state != TABLE_BUILDER_STATE_NONE, false, "Fail to add string. Have you call `Start()` method ?");
    CHECK(columnIndex < ICH->Header.GetColumnsCount(), false, "Invalid column index: %u", columnIndex);
    CHECK((ICH->Header[columnIndex].flags & InternalColumnFlags::AllowValueCopy) == InternalColumnFlags::AllowValueCopy,
          false,
          "");

    switch (ICH->CopyToClipboard.format)
    {
    case CopyClipboardFormat::CSV:
        CHECK(AddTextAsCSVFormat(output, text), false, "");
        CHECK(output.Add(","), false, "");
        break;
    case CopyClipboardFormat::TextWithTabs:
        CHECK(output.Add(text), false, "");
        CHECK(output.Add("\t"), false, "");
        break;
    case CopyClipboardFormat::HTML:
        CHECK(output.Add("<td>"), false, "");
        CHECK(AddTextAsHTMLFormat(output, text), false, "");
        CHECK(output.Add("</td>"), false, "");
        break;
    default:
        RETURNERROR(false, "Unknwon clipboard format");
    }
    return true;
}
bool ColumnsHeaderView::TableBuilder::Finalize()
{
    CHECK(state != TABLE_BUILDER_STATE_NONE, false, "Fail to complete the buffer. Have you call `Start()` method ?");
    switch (ICH->CopyToClipboard.format)
    {
    case CopyClipboardFormat::CSV:
    case CopyClipboardFormat::TextWithTabs:
        break;
    case CopyClipboardFormat::HTML:
        CHECK(output.Add("</table></html>"), false, "");
        break;
    default:
        RETURNERROR(false, "Unknwon clipboard format");
    }
    state = TABLE_BUILDER_STATE_NONE;
    return true;
}

ColumnsHeaderView::ColumnsHeaderView(
      string_view layout, std::initializer_list<ConstString> columnsList, ColumnsHeaderViewFlags flags)
    : ColumnsHeaderView(new ColumnsHeaderViewControlContext(this, columnsList, flags), layout)
{
}
// context MUST be a derivate of ColumnsHeaderViewControlContext
ColumnsHeaderView::ColumnsHeaderView(void* context, string_view layout) : Control(context, "", layout, false)
{
    ICH->CopyToClipboard.format = CopyClipboardFormat::TextWithTabs;
    ICH->CopyToClipboard.flags  = CopyClipboardFlags::None;
}

void ColumnsHeaderView::SetClipboardFormat(CopyClipboardFormat format, CopyClipboardFlags flags)
{
    ICH->CopyToClipboard.format = format;
    ICH->CopyToClipboard.flags  = flags;
}
bool ColumnsHeaderView::HeaderHasMouseCaption() const
{
    return ICH->Header.HasMouseCaption();
}
bool ColumnsHeaderView::SetColumnClipRect(Graphics::Renderer& renderer, uint32 columnIndex)
{
    return ICH->Header.SetColumnClipRect(renderer, columnIndex);
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
std::optional<uint32> ColumnsHeaderView::GetSortColumnIndex() const
{
    return ICH->Header.GetSortColumnIndex();
}
Column ColumnsHeaderView::GetSortColumn()
{
    auto colIndex = ICH->Header.GetSortColumnIndex();
    if (colIndex.has_value())
        return { this->Context, colIndex.value() };
    return NULL_COLUMN;
}
void ColumnsHeaderView::DeleteAllColumns()
{
    ICH->Header.DeleteAllColumns();
}
void ColumnsHeaderView::DeleteColumn(uint32 columnIndex)
{
    ICH->Header.DeleteColumn(columnIndex);
}
void ColumnsHeaderView::SetFrozenColumnsCount(uint32 count)
{
    ICH->Header.SetFrozenColumnsCount(count);
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
    ICH->Header.OnMouseReleased(x, y, button);
}
void ColumnsHeaderView::OnMousePressed(int x, int y, Input::MouseButton button)
{
    ICH->Header.OnMousePressed(x, y, button);
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
void ColumnsHeaderView::OnAfterResize(int /*newWidth*/, int /*newHeight*/)
{
    const auto r = this->GetHeaderLayout();
    ICH->Header.SetPosition(r.GetLeft(), r.GetTop(), r.GetWidth(), r.GetHeight());
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
bool Column::SetVisible(bool value)
{
    VALIDATE_COLUMN(false);
    COLUMNREF.SetVisible(value);
    HEADERREF.RecomputeColumnsSizes();
    return true;
}
bool Column::IsColumnValueSearchable() const
{
    VALIDATE_COLUMN(false);
    return ((COLUMNREF.flags) & InternalColumnFlags::SearcheableValue) != InternalColumnFlags::None;
}
bool Column::SetSearchable(bool searchable)
{
    VALIDATE_COLUMN(false);
    if (searchable)
        COLUMNREF.AddFlags(InternalColumnFlags::SearcheableValue);
    else
        COLUMNREF.RemoveFlags(InternalColumnFlags::SearcheableValue);
    return true;
}
bool Column::IsColumnValueCopyable() const
{
    VALIDATE_COLUMN(false);
    return ((COLUMNREF.flags) & InternalColumnFlags::AllowValueCopy) != InternalColumnFlags::None;
}
bool Column::SetClipboardCopyable(bool clipboardCopyable)
{
    VALIDATE_COLUMN(false);
    if (clipboardCopyable)
        COLUMNREF.AddFlags(InternalColumnFlags::AllowValueCopy);
    else
        COLUMNREF.RemoveFlags(InternalColumnFlags::AllowValueCopy);
    return true;
}

#undef COLUMNREF
#undef HEADERREF
#undef VALIDATE_COLUMN
} // namespace AppCUI::Controls