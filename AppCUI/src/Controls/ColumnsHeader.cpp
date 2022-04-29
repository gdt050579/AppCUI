#include "ControlContext.hpp"
using namespace AppCUI::Utils;

namespace AppCUI
{
constexpr uint32 MINIM_COLUMN_WIDTH = 3;
constexpr uint32 MAXIM_COLUMN_WIDTH = 256;

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

InternalColumn::InternalColumn(const InternalColumn& obj)
{
    this->CopyObject(obj);
}
InternalColumn::InternalColumn(InternalColumn&& obj)
{
    Reset();
    this->SwapObject(obj);
}
InternalColumn& InternalColumn::operator=(const InternalColumn& obj)
{
    this->CopyObject(obj);
}
InternalColumn& InternalColumn::operator=(InternalColumn&& obj)
{
    this->SwapObject(obj);
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
    _width               = std::max<>(_width, MINIM_COLUMN_WIDTH);
    _width               = std::min<>(_width, MAXIM_COLUMN_WIDTH);
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
} // namespace AppCUI

namespace AppCUI::Controls
{

//============================================================================================
#define ICH ((InternalColumnsHeader*) (this->data))

ColumnsHeader::ColumnsHeader()
{
    this->data = new InternalColumnsHeader();
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
    }
    else if (obj.Encoding == StringEncoding::Unicode16)
    {
        CHECK(parser.Parse(u16string_view((const char16*) obj.Data, obj.Length)), false, "");
    }
    else
    {
        RETURNERROR(false, "Current string formate (%d) is not supported", obj.Encoding);
    }
    NOT_IMPLEMENTED(false);
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
#undef ICH
} // namespace AppCUI::Controls