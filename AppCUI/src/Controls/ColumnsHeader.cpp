#include "ControlContext.hpp"
using namespace AppCUI::Utils;

namespace AppCUI
{
constexpr uint32 MINIM_COLUMN_WIDTH = 3;
constexpr uint32 MAXIM_COLUMN_WIDTH = 255;

namespace ColumnParser
{

    enum class Type : uint8
    {
        Width = 0,
        Align = 1,
        Name  = 2,

    };

    static constexpr uint8 Values[26] = {
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        static_cast<uint8>(Type::Name),
        0xFF,
        0xFF,
        static_cast<uint8>(Type::Name),
        0xFF,
        static_cast<uint8>(Type::Name),
        0xFF,
        0xFF,
        static_cast<uint8>(Type::Align),
        0xFF,
        0xFF,
        static_cast<uint8>(Type::Width),
        0xFF,
        0xFF,
        static_cast<uint8>(Type::Width),
        0xFF,
        0xFF,
        static_cast<uint8>(Type::Name),
        static_cast<uint8>(Type::Align),
        0xFF,
    };
    static constexpr uint64 Hashes[26] = {
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0xFA04F4EF1995407E,
        0x0,
        0x0,
        0xAF63E34C8601F871,
        0x0,
        0xAF63E94C860202A3,
        0x0,
        0x0,
        0x509903BB65AC341E,
        0x0,
        0x0,
        0xDBDACD932FD1E9BF,
        0x0,
        0x0,
        0xAF63EA4C86020456,
        0x0,
        0x0,
        0xC4BFDDBA8E65A2ED,
        0xAF63DC4C8601EC8C,
        0x0,
    };
    inline bool HashToType(uint64 hash, Type& resultedType)
    {
        const auto entry = hash % 26;
        if (Hashes[entry] != hash)
            return false;
        const auto res = Values[entry];
        if (res == 0xFF) // invalid value
            return false;
        resultedType = static_cast<Type>(res);
        return true;
    }
}; // namespace ColumnParser

namespace ColumnAlign
{

    static constexpr uint8 Values[9] = {
        static_cast<uint8>(TextAlignament::Center), 0xFF, static_cast<uint8>(TextAlignament::Right),
        static_cast<uint8>(TextAlignament::Left),   0xFF, static_cast<uint8>(TextAlignament::Center),
        static_cast<uint8>(TextAlignament::Left),   0xFF, static_cast<uint8>(TextAlignament::Right),
    };
    static constexpr uint64 Hashes[9] = {
        0xAF63DE4C8601EFF2, 0x0, 0x76AAAA535714D805, 0xAF63E14C8601F50B, 0x0, 0x6F4B7EC4DCAA8AC4,
        0x24B070ADA2041CB0, 0x0, 0xAF63EF4C86020CD5,
    };
    inline bool HashToType(uint64 hash, TextAlignament& resultedType)
    {
        const auto entry = hash % 9;
        if (Hashes[entry] != hash)
            return false;
        const auto res = Values[entry];
        if (res == 0xFF) // invalid value
            return false;
        resultedType = static_cast<TextAlignament>(res);
        return true;
    }
}; // namespace ColumnAlign

namespace ColumnWidth
{

    enum class Type : uint8
    {
        Fill      = 0,
        Default   = 1,
        MatchName = 2,

    };

    static constexpr uint8 Values[24] = {
        static_cast<uint8>(Type::Fill),
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        static_cast<uint8>(Type::Default),
        0xFF,
        static_cast<uint8>(Type::MatchName),
        static_cast<uint8>(Type::Fill),
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        static_cast<uint8>(Type::MatchName),
        static_cast<uint8>(Type::Default),
        0xFF,
        static_cast<uint8>(Type::MatchName),
        static_cast<uint8>(Type::MatchName),
        0xFF,
    };
    static constexpr uint64 Hashes[24] = {
        0xAAD01878F02A7608,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0xEBADA5168620C5FE,
        0x0,
        0xAF63E04C8601F358,
        0xAF63DB4C8601EAD9,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x8A95807B55044C2,
        0xAF63D94C8601E773,
        0x0,
        0xE7438E486E283D15,
        0xC3BFE3A4FE4C13F6,
        0x0,
    };
    inline bool HashToType(uint64 hash, Type& resultedType)
    {
        const auto entry = hash % 24;
        if (Hashes[entry] != hash)
            return false;
        const auto res = Values[entry];
        if (res == 0xFF) // invalid value
            return false;
        resultedType = static_cast<Type>(res);
        return true;
    }
}; // namespace ColumnWidth

constexpr inline uint16 AdjustedColumnWidth(uint32 value)
{
    if (value < MINIM_COLUMN_WIDTH)
        return (uint16) (MINIM_COLUMN_WIDTH);
    else if (value > MAXIM_COLUMN_WIDTH)
        return (uint16) (MAXIM_COLUMN_WIDTH);
    else
        return (uint16) value;
}

InternalColumn::InternalColumn()
{
    this->Reset();
}
InternalColumn::InternalColumn(const InternalColumn& obj)
{
    this->CopyObject(obj);
}
InternalColumn::InternalColumn(InternalColumn&& obj)
{
    this->Reset();
    this->SwapObject(obj);
}
InternalColumn& InternalColumn::operator=(const InternalColumn& obj)
{
    this->CopyObject(obj);
    return *this;
}
InternalColumn& InternalColumn::operator=(InternalColumn&& obj)
{
    this->SwapObject(obj);
    return *this;
}
void InternalColumn::Reset()
{
    this->hotKeyCode     = Key::None;
    this->hotKeyOffset   = CharacterBuffer::INVALID_HOTKEY_OFFSET;
    this->align          = TextAlignament::Left;
    this->width          = MINIM_COLUMN_WIDTH;
    this->widthType      = InternalColumnWidthType::Value;
    this->widthTypeValue = MINIM_COLUMN_WIDTH;
    this->x              = 0;
    this->flags          = InternalColumnFlags::AllowValueCopy | InternalColumnFlags::SearcheableValue;
    this->name.Clear();
}
void InternalColumn::CopyObject(const InternalColumn& obj)
{
    this->hotKeyCode     = obj.hotKeyCode;
    this->hotKeyOffset   = obj.hotKeyOffset;
    this->align          = obj.align;
    this->widthType      = obj.widthType;
    this->widthTypeValue = obj.widthTypeValue;
    this->name           = obj.name;

    // these values need to be recomputed
    this->width = MINIM_COLUMN_WIDTH;
    this->x     = 0;
}
void InternalColumn::SwapObject(InternalColumn& obj)
{
    // direct copy (nothing to swap)
    this->hotKeyCode     = obj.hotKeyCode;
    this->hotKeyOffset   = obj.hotKeyOffset;
    this->align          = obj.align;
    this->widthType      = obj.widthType;
    this->widthTypeValue = obj.widthTypeValue;
    // these values need to be recomputed
    this->width = MINIM_COLUMN_WIDTH;
    this->x     = 0;
    // swap name
    this->name.Swap(obj.name);
}
bool InternalColumn::SetName(const ConstString& text)
{
    this->hotKeyCode   = Key::None;
    this->hotKeyOffset = CharacterBuffer::INVALID_HOTKEY_OFFSET;

    CHECK(this->name.SetWithHotKey(text, this->hotKeyOffset, this->hotKeyCode, Key::Ctrl),
          false,
          "Fail to set name to column !");

    return true;
}
bool InternalColumn::SetAlign(TextAlignament align)
{
    if ((align == TextAlignament::Left) || (align == TextAlignament::Right) || (align == TextAlignament::Center))
    {
        this->align = align;
        return true;
    }
    RETURNERROR(
          false,
          "align parameter can only be one of the following: TextAlignament::Left, TextAlignament::Right or "
          "TextAlignament::Center");
}
void InternalColumn::SetWidth(uint32 _width)
{
    this->widthTypeValue = (uint16) _width;
    this->widthType      = InternalColumnWidthType::Value;
}
void InternalColumn::SetWidth(float percentage)
{
    if (percentage < 0)
        return;
    this->widthTypeValue = (uint16) (percentage * 10000);
    this->widthType      = InternalColumnWidthType::Percentage;
}
void InternalColumn::SetWidth(double percentage)
{
    if (percentage < 0)
        return;
    this->widthTypeValue = (uint16) (percentage * 10000);
    this->widthType      = InternalColumnWidthType::Percentage;
}
void InternalColumn::AddFlags(InternalColumnFlags flagsToAdd)
{
    this->flags = this->flags | flagsToAdd;
}
void InternalColumn::RemoveFlags(InternalColumnFlags flagsToRemove)
{
    this->flags = static_cast<InternalColumnFlags>(((uint32) this->flags) & (~((uint32) flagsToRemove)));
}
ColumnsHeader::ColumnsHeader(
      Reference<ColumnsHeaderView> hostControl,
      std::initializer_list<ConstString> list,
      ColumnsHeaderViewFlags headerFlags)
{
    this->Location.x          = 0;
    this->Location.y          = 0;
    this->Location.width      = 0;
    this->Location.totalWidth = 0;
    this->Location.listHeight = 0;
    this->Location.scrollX    = 0;
    this->sortDirection       = SortDirection::Ascendent;
    this->hasMouseCaption     = false;
    this->host                = hostControl;
    this->flags               = static_cast<uint32>(headerFlags);
    this->sortColumnIndex     = INVALID_COLUMN_INDEX;
    this->hoveredColumnIndex  = INVALID_COLUMN_INDEX;
    this->resizeColumnIndex   = INVALID_COLUMN_INDEX;
    this->toolTipColumnIndex  = INVALID_COLUMN_INDEX;
    this->currentApp          = Application::GetApplication();
    // some check up
    // 1. if it is sortable it is alsa clickable
    if (this->flags && ColumnsHeaderViewFlags::Sortable)
        this->flags |= static_cast<uint32>(ColumnsHeaderViewFlags::Clickable);
    // finally --> add columns
    AddColumns(list);
}
void ColumnsHeader::ClearKeyboardAndMouseLocks()
{
    this->hasMouseCaption    = false;
    this->hoveredColumnIndex = INVALID_COLUMN_INDEX;
    this->resizeColumnIndex  = INVALID_COLUMN_INDEX;
    this->toolTipColumnIndex = INVALID_COLUMN_INDEX;
}
bool ColumnsHeader::Add(KeyValueParser& parser, bool unicodeText)
{
    LocalString<256> error;
    ColumnParser::Type columnParamType;
    ColumnWidth::Type columnWidthType;
    string_view asciiName;
    u16string_view unicodeTextName;
    TextAlignament textAlign          = TextAlignament::Left;
    InternalColumnWidthType widthType = InternalColumnWidthType::Value;
    uint16 widthValue                 = MINIM_COLUMN_WIDTH;

    for (auto idx = 0U; idx < parser.GetCount(); idx++)
    {
        const auto& item = parser[idx];
        if (ColumnParser::HashToType(item.Key.hash, columnParamType) == false)
        {
            error.Set("Unknwon layout item: ");
            error.Add((const char*) item.Key.data, item.Key.dataSize);
            ASSERT(false, error.GetText());
        }
        switch (columnParamType)
        {
        case ColumnParser::Type::Name:
            if (unicodeText)
                unicodeTextName = u16string_view((const char16*) item.Value.data, item.Value.dataSize >> 1);
            else
                asciiName = string_view((const char*) item.Value.data, item.Value.dataSize);
            break;
        case ColumnParser::Type::Width:
            if (item.Value.type == KeyValuePair::Type::Number)
            {
                ASSERT(item.Value.number > 0, "Column width should be bigger than 0");
                widthType  = InternalColumnWidthType::Value;
                widthValue = item.Value.number;
            }
            else if (item.Value.type == KeyValuePair::Type::Percentage)
            {
                ASSERT(item.Value.number > 0, "Column width (even in percentage) should be bigger than 0");
                widthType  = InternalColumnWidthType::Percentage;
                widthValue = item.Value.number;
            }
            else
            {
                if (ColumnWidth::HashToType(item.Value.hash, columnWidthType) == false)
                {
                    error.Set("Unknwon column width type: ");
                    error.Add((const char*) item.Key.data, item.Key.dataSize);
                    ASSERT(false, error.GetText());
                }
                else
                {
                    if (columnWidthType == ColumnWidth::Type::Fill)
                        widthType = InternalColumnWidthType::Fill;
                    else if (columnWidthType == ColumnWidth::Type::MatchName)
                        widthType = InternalColumnWidthType::MatchName;
                    else
                    {
                        // default value
                        widthType  = InternalColumnWidthType::Value;
                        widthValue = 12;
                    }
                }
            }
            break;
        case ColumnParser::Type::Align:
            if (ColumnAlign::HashToType(item.Value.hash, textAlign) == false)
            {
                error.Set("Unknwon column align value: ");
                error.Add((const char*) item.Value.data, item.Value.dataSize);
                ASSERT(false, error.GetText());
            }
            break;
        default:
            error.Set("Internal error - fail to parse item: ");
            error.Add((const char*) item.Key.data, item.Key.dataSize);
            ASSERT(false, error.GetText());
            return false;
        }
    }
    // add the column
    auto& col = columns.emplace_back();
    col.SetAlign(textAlign);
    if (unicodeText)
        col.SetName(unicodeTextName);
    else
        col.SetName(asciiName);
    col.widthTypeValue = widthValue;
    col.widthType      = widthType;
    col.x              = 0;

    return true;
}
bool ColumnsHeader::AddColumn(const ConstString columnFormat)
{
    ConstStringObject obj(columnFormat);
    KeyValueParser parser;
    if ((obj.Encoding == StringEncoding::Ascii) || (obj.Encoding == StringEncoding::UTF8))
    {
        CHECK(parser.Parse(string_view((const char*) obj.Data, obj.Length)), false, "");
        CHECK(this->Add(parser, false), false, "");
        return true;
    }
    else if (obj.Encoding == StringEncoding::Unicode16)
    {
        CHECK(parser.Parse(u16string_view((const char16*) obj.Data, obj.Length)), false, "");
        CHECK(this->Add(parser, true), false, "");
        return true;
    }
    else
    {
        RETURNERROR(false, "Current string formate (%d) is not supported", obj.Encoding);
    }
}
bool ColumnsHeader::AddColumns(std::initializer_list<ConstString> list)
{
    const auto newReservedCapacity = ((list.size() + this->columns.size()) | 7) + 1; // align to 8 columns
    this->columns.reserve(newReservedCapacity);
    for (auto& col : list)
    {
        CHECK(AddColumn(col), false, "");
    }
    return true;
}
void ColumnsHeader::DeleteAllColumns()
{
    this->columns.clear();
}
void ColumnsHeader::DeleteColumn(uint32 columnIndex)
{
    if (columnIndex >= this->columns.size())
        return; // nothing to delete
    this->columns.erase(this->columns.begin() + columnIndex);
    this->RecomputeColumnsSizes();
}
void ColumnsHeader::RecomputeColumnsSizes()
{
    uint32 columnsWithFill  = 0;
    uint32 columnsTotalSize = 0;
    for (auto& col : columns)
    {
        switch (col.widthType)
        {
        case InternalColumnWidthType::Value:
            col.width = AdjustedColumnWidth(col.widthTypeValue);
            break;
        case InternalColumnWidthType::MatchName:
            col.width = AdjustedColumnWidth(col.name.Len() + 3);
            break;
        case InternalColumnWidthType::Percentage:
            col.width = AdjustedColumnWidth(this->Location.width * ((uint32) col.widthTypeValue) / 10000U);
            break;
        case InternalColumnWidthType::Fill:
            columnsWithFill++;
            col.width = 0; // don't know it yet
            break;
        default:
            // safety
            col.width = 0;
            break;
        }
        columnsTotalSize += col.width;
    }
    if (columnsWithFill > 0)
    {
        // add columns.size()-1 as the number of vertical separator (except for the last one)
        auto totalRequiredSpace = columnsTotalSize + (uint32) columns.size() - 1;
        auto fillValue          = totalRequiredSpace < this->Location.width
                                        ? ((this->Location.width - totalRequiredSpace) / columnsWithFill)
                                        : 0;
        for (auto& col : columns)
        {
            if (col.widthType == InternalColumnWidthType::Fill)
            {
                columnsWithFill--;
                if (columnsWithFill == 0)
                {
                    // last one --> make sure that we fill the entire space
                    if (totalRequiredSpace < this->Location.width)
                        col.width = AdjustedColumnWidth(this->Location.width - totalRequiredSpace);
                    else
                        col.width = AdjustedColumnWidth(fillValue);
                }
                else
                {
                    col.width = AdjustedColumnWidth(fillValue);
                }
                totalRequiredSpace += col.width;
            }
        }
    }
    // compute the positions and total width
    auto xPoz                 = this->Location.x;
    this->Location.totalWidth = 0;
    for (auto& col : columns)
    {
        col.x = xPoz;
        xPoz += ((int32) (col.width)) + 1;
        this->Location.totalWidth += (col.width + 1);
    }
}
void ColumnsHeader::Paint(Graphics::Renderer& renderer)
{
    const auto Members     = (ControlContext*) host->Context;
    const auto state       = Members->GetControlState(ControlStateFlags::None);
    const auto Cfg         = Members->Cfg;
    const auto defaultCol  = Cfg->Header.Text.GetColor(state);
    const auto defaultHK   = Cfg->Header.HotKey.GetColor(state);
    const auto rightMargin = this->Location.x + (int32) this->Location.width;
    auto colIndex          = 0U;

    // first reset the view
    renderer.ResetClip();
    renderer.SetClipRect(
          { { this->Location.x, this->Location.y }, { this->Location.width, this->Location.listHeight } });

    renderer.FillHorizontalLine(this->Location.x, this->Location.y, this->Location.width, ' ', defaultCol);

    WriteTextParams params(WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth | WriteTextFlags::OverwriteColors);
    params.Y           = this->Location.y;
    params.Color       = defaultCol;
    params.HotKeyColor = defaultHK;

    for (auto& col : this->columns)
    {
        // check if the column is outside visible range
        if (((col.x + (int32) col.width) < this->Location.x) || (col.x >= rightMargin))
        {
            colIndex++;
            continue;
        }
        if ((state == ControlState::Focused) && (colIndex == this->sortColumnIndex))
        {
            params.Color       = Cfg->Header.Text.PressedOrSelected;
            params.HotKeyColor = Cfg->Header.HotKey.PressedOrSelected;
            renderer.FillHorizontalLineSize(
                  col.x, this->Location.y, col.width, ' ', params.Color); // highlight the column
        }
        else if ((colIndex == this->hoveredColumnIndex) && (state != ControlState::Inactive))
        {
            params.Color       = Cfg->Header.Text.Hovered;
            params.HotKeyColor = Cfg->Header.HotKey.Hovered;
            renderer.FillHorizontalLineSize(
                  col.x, this->Location.y, col.width, ' ', params.Color); // highlight the column
        }
        else
        {
            params.Color       = defaultCol;
            params.HotKeyColor = defaultHK;
        }

        params.X     = col.x + 1;
        params.Width = col.width >= 2 ? col.width - 2 : 0;
        params.Align = col.align;
        if ((col.hotKeyOffset == CharacterBuffer::INVALID_HOTKEY_OFFSET) || (!this->IsClickable()))
        {
            params.Flags = WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth | WriteTextFlags::OverwriteColors;
            renderer.WriteText(col.name, params);
        }
        else
        {
            params.Flags = WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth | WriteTextFlags::OverwriteColors |
                           WriteTextFlags::HighlightHotKey;
            if (state == ControlState::Focused)
            {
                if (colIndex == this->sortColumnIndex)
                {
                    params.HotKeyColor = Cfg->Header.HotKey.PressedOrSelected;
                }
                else if (colIndex == this->hoveredColumnIndex)
                {
                    params.HotKeyColor = Cfg->Header.HotKey.Hovered;
                }
                else
                    params.HotKeyColor = defaultHK;
            }
            params.HotKeyPosition = col.hotKeyOffset;
            renderer.WriteText(col.name, params);
        }
        const auto separatorX = col.x + (int32) col.width;
        if ((state == ControlState::Focused) && (colIndex == this->sortColumnIndex))
        {
            renderer.WriteSpecialCharacter(
                  separatorX - 1,
                  this->Location.y,
                  (this->sortDirection == SortDirection::Ascendent) ? SpecialChars::TriangleUp
                                                                    : SpecialChars::TriangleDown,
                  Cfg->Header.HotKey.PressedOrSelected);
        }

        if (!(this->flags && ColumnsHeaderViewFlags::HideSeparators))
        {
            auto sepState = state;
            if (this->resizeColumnIndex == colIndex)
            {
                sepState = this->hasMouseCaption ? ControlState::Hovered : ControlState::Hovered;
            }

            renderer.DrawVerticalLine(
                  separatorX,
                  this->Location.y,
                  this->Location.y + this->Location.listHeight,
                  Cfg->Lines.GetColor(sepState));
        }
        colIndex++;
    }
    renderer.ResetClip();
}
uint32 ColumnsHeader::MouseToColumn(int mouse_x, int mouse_y)
{
    if (mouse_y != this->Location.y)
        return INVALID_COLUMN_INDEX; // mouse not on the column

    auto idx = 0U;
    for (auto& col : this->columns)
    {
        auto sepX = col.x + (int32) col.width;
        if ((mouse_x >= col.x) && (mouse_x < sepX))
            return idx;
        idx++;
    }
    return INVALID_COLUMN_INDEX;
}
uint32 ColumnsHeader::MouseToColumnSeparator(int mouse_x, int mouse_y)
{
    if (mouse_y < this->Location.y)
        return INVALID_COLUMN_INDEX; // mouse not on the column
    if (this->flags && ColumnsHeaderViewFlags::FixedSized)
        return INVALID_COLUMN_INDEX; // there is no need to search for a column if is not sizeable

    auto idx = 0U;
    for (auto& col : this->columns)
    {
        auto sepX = col.x + (int32) col.width;
        if (mouse_x == sepX)
            return idx;
        idx++;
    }
    return INVALID_COLUMN_INDEX;
}
void ColumnsHeader::SetPosition(int x, int y, uint32 width, uint32 listHeight)
{
    this->Location.x          = x;
    this->Location.y          = y;
    this->Location.width      = width;
    this->Location.listHeight = listHeight;
    this->RecomputeColumnsSizes();
}
bool ColumnsHeader::SetSortColumn(uint32 index)
{
    CHECK(this->IsSortable(), false, "Header is not sortable. Have you added ColumnsHeaderViewFlags::Sortable flag");
    CHECK(index < columns.size(), false, "");
    if (index == this->sortColumnIndex)
    {
        this->sortDirection =
              this->sortDirection == SortDirection::Ascendent ? SortDirection::Descendent : SortDirection::Ascendent;
    }
    else
    {
        this->sortColumnIndex = index;
        this->sortDirection   = SortDirection::Ascendent;
    }
    return true;
}
bool ColumnsHeader::SetSortColumn(uint32 index, SortDirection direction)
{
    CHECK(this->IsSortable(), false, "Header is not sortable. Have you added ColumnsHeaderViewFlags::Sortable flag");
    CHECK(index < columns.size(), false, "");
    this->sortColumnIndex = index;
    this->sortDirection   = direction;
    return true;
}
void ColumnsHeader::ProcessColumnClickRequest(uint32 index)
{
    if (!IsClickable())
        return;
    if (index >= this->columns.size())
        return;
    if (this->IsSortable())
    {
        this->SetSortColumn(index);
    }
    this->host->OnColumnClicked(index);
}
void ColumnsHeader::ResizeColumn(bool increase)
{
    if (this->flags && ColumnsHeaderViewFlags::FixedSized)
        return; // nothing to resize
    if (this->resizeColumnIndex >= this->columns.size())
        return; // safety check
    auto& col = this->columns[this->resizeColumnIndex];
    if (increase)
    {
        if (col.width < MAXIM_COLUMN_WIDTH)
            col.SetWidth((uint32) col.width + 1U);
    }
    else
    {
        if (col.width > 0)
            col.SetWidth((uint32) col.width - 1U);
    }
    this->RecomputeColumnsSizes();
}
bool ColumnsHeader::OnKeyEvent(Key key, char16 character)
{
    if (this->resizeColumnIndex != INVALID_COLUMN_INDEX)
    {
        // sanity check
        if (this->columns.size() == 0)
        {
            this->resizeColumnIndex = INVALID_COLUMN_INDEX;
            return false;
        }
        switch (key)
        {
        case Key::Left:
            ResizeColumn(false);
            return true;
        case Key::Right:
            ResizeColumn(true);
            return true;
        case Key::Ctrl | Key::Left:
            this->resizeColumnIndex =
                  this->resizeColumnIndex > 0 ? this->resizeColumnIndex - 1 : (uint32) (this->columns.size() - 1);
            return true;
        case Key::Ctrl | Key::Right:
            this->resizeColumnIndex++;
            if (this->resizeColumnIndex >= this->columns.size())
                this->resizeColumnIndex = 0;
            return true;
        }
        // for any other key --> exit resize column mode and tranfer the key to its host
        this->resizeColumnIndex = INVALID_COLUMN_INDEX;
        return false;
    }
    else
    {
        switch (key)
        {
        case Key::Ctrl | Key::Left:
        case Key::Ctrl | Key::Right:
            this->resizeColumnIndex = 0;
            return true;
        case Key::Left:
            SetScrollX(Location.scrollX - 1);
            return true;
        case Key::Right:
            this->RecomputeColumnsSizes();
            SetScrollX(Location.scrollX + 1);
            return true;
        }

        // check for Hot Key
        auto idx = 0U;
        for (auto& col : this->columns)
        {
            if (col.hotKeyCode == key)
            {
                this->ProcessColumnClickRequest(idx);
                return true;
            }
            idx++;
        }
    }
    return false;
}
void ColumnsHeader::OnMouseReleased(int x, int y, Input::MouseButton button)
{
    this->ClearKeyboardAndMouseLocks();
    this->ProcessColumnClickRequest(MouseToColumn(x, y));
}
void ColumnsHeader::OnMousePressed(int x, int y, Input::MouseButton button)
{
    auto colIdx = MouseToColumn(x, y);
    if ((colIdx != this->sortColumnIndex) && (this->IsClickable()))
    {
        this->sortColumnIndex = colIdx;
        this->hasMouseCaption = true;
    }
    auto sepIdx = MouseToColumnSeparator(x, y);
    if (sepIdx != INVALID_COLUMN_INDEX)
    {
        this->resizeColumnIndex = sepIdx;
        this->hasMouseCaption   = true;
    }
}
bool ColumnsHeader::OnMouseDrag(int x, int y, Input::MouseButton button)
{
    if (this->resizeColumnIndex != INVALID_COLUMN_INDEX)
    {
        auto colX = this->columns[this->resizeColumnIndex].x;
        if (colX < x)
        {
            this->columns[this->resizeColumnIndex].SetWidth((uint32) (x - colX));
            this->RecomputeColumnsSizes();
            return true;
        }
    }
    return false;
}
bool ColumnsHeader::OnMouseWheel(int x, int y, Input::MouseWheel direction)
{
    return false; // left-right scroll ? should it be treated ?
}
bool ColumnsHeader::OnMouseOver(int x, int y)
{
    auto colIdx = MouseToColumn(x, y);
    auto sepIdx = MouseToColumnSeparator(x, y);
    bool result = false;

    if (colIdx != this->toolTipColumnIndex)
    {   
        this->toolTipColumnIndex = colIdx;
        result                   = true;
        if (this->toolTipColumnIndex == INVALID_COLUMN_INDEX)
            this->currentApp->ToolTip.Hide();
        else
        {
            const auto& col = this->columns[colIdx];
            if (col.name.Len() + 3 > (uint32) col.width)
            {
                LocalUnicodeStringBuilder<128> temp(col.name);
                this->currentApp->SetToolTip(
                      this->host.ToBase<Control>(),
                      temp.ToStringView(),
                      col.x + (int32) (col.width >> 1),
                      this->Location.y);
            }
            else
            {
                this->currentApp->ToolTip.Hide();
            }
        }
    }
    if (!IsClickable())
        colIdx = INVALID_COLUMN_INDEX;
    if ((colIdx != this->hoveredColumnIndex) || (sepIdx != this->resizeColumnIndex))
    {
        this->hoveredColumnIndex = colIdx;
        this->resizeColumnIndex  = sepIdx;
        result                   = true;
    }
    return result;
}
bool ColumnsHeader::OnMouseLeave()
{
    this->currentApp->ToolTip.Hide();
    if ((this->hasMouseCaption) || (this->hoveredColumnIndex == INVALID_COLUMN_INDEX))
    {
        this->ClearKeyboardAndMouseLocks();
        return true;
    }
    return false;
}
void ColumnsHeader::OnLoseFocus()
{
    this->ClearKeyboardAndMouseLocks();
}
} // namespace AppCUI
