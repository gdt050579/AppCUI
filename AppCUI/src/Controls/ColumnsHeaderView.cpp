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
bool ColumnsHeaderView::AddColumns(std::initializer_list<ConstString> list)
{
    const auto newReservedCapacity = ((list.size() + ICH->columns.size()) | 7) + 1; // align to 8 columns
    ICH->columns.reserve(newReservedCapacity);
    for (auto& col : list)
    {
        CHECK(Add(col), false, "");
    }
    return true;
}
void ColumnsHeaderView::Paint(Graphics::Renderer& renderer)
{
    ICH->Paint(renderer);
}
bool ColumnsHeaderView::ProcessKeyEvent(Key key, char16 character)
{
    return ICH->OnKeyEvent(key, character);
}
#undef ICH
} // namespace AppCUI::Controls