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

InternalColumn::InternalColumn(const InternalColumn& obj)
{
}
InternalColumn::InternalColumn(InternalColumn&& obj)
{
}
InternalColumn& InternalColumn::operator=(const InternalColumn& obj)
{
}
InternalColumn& InternalColumn::operator=(InternalColumn& obj)
{
}
void InternalColumn::Reset()
{
    this->HotKeyCode   = Key::None;
    this->HotKeyOffset = CharacterBuffer::INVALID_HOTKEY_OFFSET;
    this->Flags        = 0;
    this->Align        = TextAlignament::Left;
    this->Width        = 10;
    this->Name.Clear();
}
bool InternalColumn::SetName(const ConstString& text)
{
    this->HotKeyCode   = Key::None;
    this->HotKeyOffset = CharacterBuffer::INVALID_HOTKEY_OFFSET;

    CHECK(Name.SetWithHotKey(text, this->HotKeyOffset, this->HotKeyCode, Key::Ctrl),
          false,
          "Fail to set name to column !");

    return true;
}
bool InternalColumn::SetAlign(TextAlignament align)
{
    if ((align == TextAlignament::Left) || (align == TextAlignament::Right) || (align == TextAlignament::Center))
    {
        this->Align = align;
        return true;
    }
    RETURNERROR(
          false,
          "align parameter can only be one of the following: TextAlignament::Left, TextAlignament::Right or "
          "TextAlignament::Center");
}
void InternalColumn::SetWidth(uint32 width)
{
    width       = std::max<>(width, MINIM_COLUMN_WIDTH);
    width       = std::min<>(width, MAXIM_COLUMN_WIDTH);
    this->Width = (uint8) width;
}


bool InternalColumnsHeader::Add(KeyValueParser& parser, bool unicodeText)
{
    LocalString<256> error;
    ColumnParser::Type columnParamType;
    string_view asciiName;
    u16string_view unicodeTextName;
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
            break;
        case ColumnParser::Type::Align:
            break;
        default:
            error.Set("Internal error - fail to parse item: ");
            error.Add((const char*) item.Key.data, item.Key.dataSize);
            ASSERT(false, error.GetText());
            return false;
        }
    }
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