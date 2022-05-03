#include "ControlContext.hpp"
using namespace AppCUI::Utils;

namespace AppCUI
{
constexpr uint32 MINIM_COLUMN_WIDTH   = 3;
constexpr uint32 MAXIM_COLUMN_WIDTH   = 255;
constexpr uint32 INVALID_COLUMN_INDEX = 0xFFFFFFFF;

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

InternalColumnsHeader::InternalColumnsHeader(Reference<Control> hostControl)
{
    this->x                    = 0;
    this->y                    = 0;
    this->width                = 0;
    this->Cfg                  = AppCUI::Application::GetAppConfig();
    this->sortable             = false;
    this->sortAscendent        = true;
    this->showColumnSeparators = true;
    this->sizeableColumns      = true;
    this->host                 = hostControl;
    this->sortColumnIndex      = INVALID_COLUMN_INDEX;
    this->hoveredColumnIndex   = INVALID_COLUMN_INDEX;
    this->resizeColumnIndex    = INVALID_COLUMN_INDEX;
}
bool InternalColumnsHeader::Add(KeyValueParser& parser, bool unicodeText)
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
void InternalColumnsHeader::RecomputeColumnsSizes()
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
            col.width = AdjustedColumnWidth(this->width * ((uint32) col.widthTypeValue) / 10000U);
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
        auto fillValue = totalRequiredSpace < this->width ? ((this->width - totalRequiredSpace) / columnsWithFill) : 0;
        for (auto& col : columns)
        {
            if (col.widthType == InternalColumnWidthType::Fill)
            {
                columnsWithFill--;
                if (columnsWithFill == 0)
                {
                    // last one --> make sure that we fill the entire space
                    if (totalRequiredSpace < this->width)
                        col.width = AdjustedColumnWidth(this->width - totalRequiredSpace);
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
    // compute the positions
    auto xPoz = this->x;
    for (auto& col : columns)
    {
        col.x = xPoz;
        xPoz += ((int32) (col.width)) + 1;
    }
}
void InternalColumnsHeader::Paint(Graphics::Renderer& renderer)
{
    const auto Members     = (ControlContext*) host->Context;
    const auto state       = Members->GetControlState(ControlStateFlags::None);
    const auto defaultCol  = Cfg->Header.Text.GetColor(state);
    const auto defaultHK   = Cfg->Header.HotKey.GetColor(state);
    const auto rightMargin = this->x + (int32) this->width;
    auto colIndex          = 0U;

    renderer.FillHorizontalLine(this->x, this->y, this->width, ' ', defaultCol);

    WriteTextParams params(WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth | WriteTextFlags::OverwriteColors);
    params.Y           = y;
    params.Color       = defaultCol;
    params.HotKeyColor = defaultHK;

    for (auto& col : this->columns)
    {
        // check if the column is outside visible range
        if (((col.x + (int32) col.width) < this->x) || (col.x >= rightMargin))
        {
            colIndex++;
            continue;
        }
        if (state == ControlState::Focused)
        {
            if (colIndex == this->sortColumnIndex)
            {
                params.Color = Cfg->Header.Text.PressedOrSelected;
                renderer.FillHorizontalLineSize(col.x, this->y, col.width, ' ', params.Color); // highlight the column
            }
            else if (colIndex == this->hoveredColumnIndex)
            {
                params.Color = Cfg->Header.Text.Hovered;
                renderer.FillHorizontalLineSize(col.x, this->y, col.width, ' ', params.Color); // highlight the column
            }
            else
                params.Color = defaultCol;
        }
        params.X     = col.x + 1;
        params.Width = col.width >= 2 ? col.width - 2 : 0;
        params.Align = col.align;
        if ((col.hotKeyOffset == CharacterBuffer::INVALID_HOTKEY_OFFSET) || (!this->sortable))
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
                  this->y,
                  this->sortAscendent ? SpecialChars::TriangleUp : SpecialChars::TriangleDown,
                  Cfg->Header.HotKey.PressedOrSelected);
        }

        if (this->showColumnSeparators)
        {
            // renderer.DrawVerticalLine(separatorX, this->y, this->y, Cfg->Lines.GetColor(state));
            renderer.WriteSpecialCharacter(
                  separatorX, this->y, SpecialChars::BoxVerticalSingleLine, Cfg->Lines.GetColor(state));
        }
        colIndex++;
    }
}
void InternalColumnsHeader::MouseToColumn(int mouse_x, int mouse_y, uint32& columnID, uint32& columnSeparatorID)
{
    columnID          = INVALID_COLUMN_INDEX;
    columnSeparatorID = INVALID_COLUMN_INDEX;
    if (mouse_y != this->y)
        return; // mouse not on the column
    if ((!sortable) && (!sizeableColumns))
        return; // there is no need to search for a column or separator as you can not do anything with it

    auto idx = 0U;
    for (auto& col : this->columns)
    {
        auto sepX = col.x + (int32) col.width;
        if ((mouse_x >= col.x) && (mouse_x < sepX) && (sortable))
        {
            columnID = idx;
            return;
        }
        if ((mouse_x == sepX) && (sizeableColumns))
        {
            columnSeparatorID = idx;
            return;
        }
        idx++;
    }
}
void InternalColumnsHeader::SetPosition(int _x, int _y, uint32 _width)
{
    this->x     = _x;
    this->y     = _y;
    this->width = _width;
    this->RecomputeColumnsSizes();
}
bool InternalColumnsHeader::OnKeyEvent(Key key, char16 character)
{
    if (this->resizeColumnIndex != INVALID_COLUMN_INDEX)
    {
        // sanity check
        if (this->columns.size()==0)
        {
            this->resizeColumnIndex = INVALID_COLUMN_INDEX;
            return false;
        }
        switch (key)
        {
        case Key::Left:
            // decrease size
            return true;
        case Key::Right:
            // increase size
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
    return false;
}
} // namespace AppCUI

namespace AppCUI::Controls
{

//============================================================================================
#define ICH ((InternalColumnsHeader*) (this->data))

ColumnsHeader::ColumnsHeader(Reference<Control> hostControl)
{
    this->data = new InternalColumnsHeader(hostControl);
}
ColumnsHeader::~ColumnsHeader()
{
    if (this->data)
        delete (InternalColumnsHeader*) (this->data);
    this->data = nullptr;
}
bool ColumnsHeader::Add(const ConstString columnFormat)
{
    ConstStringObject obj(columnFormat);
    KeyValueParser parser;
    if ((obj.Encoding == StringEncoding::Ascii) || (obj.Encoding == StringEncoding::UTF8))
    {
        CHECK(parser.Parse(string_view((const char*) obj.Data, obj.Length)), false, "");
        CHECK(ICH->Add(parser, false), false, "");
        return true;
    }
    else if (obj.Encoding == StringEncoding::Unicode16)
    {
        CHECK(parser.Parse(u16string_view((const char16*) obj.Data, obj.Length)), false, "");
        CHECK(ICH->Add(parser, true), false, "");
        return true;
    }
    else
    {
        RETURNERROR(false, "Current string formate (%d) is not supported", obj.Encoding);
    }
}
bool ColumnsHeader::Add(std::initializer_list<ConstString> list)
{
    const auto newReservedCapacity = ((list.size() + ICH->columns.size()) | 7) + 1; // align to 8 columns
    ICH->columns.reserve(newReservedCapacity);
    for (auto& col : list)
    {
        CHECK(Add(col), false, "");
    }
    return true;
}
void ColumnsHeader::Paint(Graphics::Renderer& renderer)
{
    ICH->Paint(renderer);
}
bool ColumnsHeader::ProcessKeyEvent(Key key, char16 character)
{
    return ICH->OnKeyEvent(key, character);
}
#undef ICH
} // namespace AppCUI::Controls