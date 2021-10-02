#include "ControlContext.hpp"

using namespace AppCUI::Controls;

//======[LABEL]======================================================================================
std::unique_ptr<Label> Factory::Label::Create(const AppCUI::Utils::ConstString& caption, const std::string_view& layout)
{
    return std::unique_ptr<AppCUI::Controls::Label>(new AppCUI::Controls::Label(caption, layout));
}
Label* Factory::Label::Create(
      Control& parent, const AppCUI::Utils::ConstString& caption, const std::string_view& layout)
{
    return parent.AddControl<AppCUI::Controls::Label>(Factory::Label::Create(caption, layout));
}