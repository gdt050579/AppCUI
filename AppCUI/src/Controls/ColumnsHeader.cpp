#include "ControlContext.hpp"

namespace AppCUI::Controls
{
using namespace AppCUI::Utils;
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
    for (auto& col : list)
    {
        CHECK(Add(col), false, "");
    }
    return true;
}
} // namespace AppCUI::Controls