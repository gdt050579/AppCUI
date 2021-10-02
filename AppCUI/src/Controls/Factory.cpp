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


//======[BUTTON]=====================================================================================
std::unique_ptr<Button> Factory::Button::Create(
      const AppCUI::Utils::ConstString& caption,
      const std::string_view& layout,
      int controlID,
      AppCUI::Controls::ButtonFlags flags)
{
    return std::unique_ptr<AppCUI::Controls::Button>(new AppCUI::Controls::Button(caption, layout, controlID, flags));
}
Button* Factory::Button::Create(
      Control& parent,
      const AppCUI::Utils::ConstString& caption,
      const std::string_view& layout,
      int controlID,
      AppCUI::Controls::ButtonFlags flags)
{
    return parent.AddControl<AppCUI::Controls::Button>(Factory::Button::Create(caption, layout, controlID, flags));
}


//======[CHECKBOX]===================================================================================
std::unique_ptr<CheckBox> Factory::CheckBox::Create(
      const AppCUI::Utils::ConstString& caption, const std::string_view& layout, int controlID)
{
    return std::unique_ptr<AppCUI::Controls::CheckBox>(new AppCUI::Controls::CheckBox(caption, layout, controlID));
}
CheckBox* Factory::CheckBox::Create(
      Control& parent, const AppCUI::Utils::ConstString& caption, const std::string_view& layout, int controlID)
{
    return parent.AddControl<AppCUI::Controls::CheckBox>(Factory::CheckBox::Create(caption, layout, controlID));
}


//======[RADIOBOX]===================================================================================
std::unique_ptr<RadioBox> Factory::RadioBox::Create(
      const AppCUI::Utils::ConstString& caption, const std::string_view& layout, int groupID, int controlID)
{
    return std::unique_ptr<AppCUI::Controls::RadioBox>(
          new AppCUI::Controls::RadioBox(caption, layout, groupID, controlID));
}

RadioBox* Factory::RadioBox::Create(
      Control& parent,
      const AppCUI::Utils::ConstString& caption,
      const std::string_view& layout,
      int groupID,
      int controlID)
{
    return parent.AddControl<AppCUI::Controls::RadioBox>(
          Factory::RadioBox::Create(caption, layout, groupID, controlID));
}

//======[SPLITTER]===================================================================================
std::unique_ptr<Splitter> Factory::Splitter::Create(const std::string_view& layout, bool vertical)
{
    return std::unique_ptr<AppCUI::Controls::Splitter>(new AppCUI::Controls::Splitter(layout, vertical));
}
Splitter* Factory::Splitter::Create(Control& parent, const std::string_view& layout, bool vertical)
{
    return parent.AddControl<AppCUI::Controls::Splitter>(Factory::Splitter::Create(layout, vertical));
}


//======[PANEL]======================================================================================
std::unique_ptr<Panel> Factory::Panel::Create(const AppCUI::Utils::ConstString& caption, const std::string_view& layout)
{
    return std::unique_ptr<AppCUI::Controls::Panel>(new AppCUI::Controls::Panel(caption, layout));
}
Panel* Factory::Panel::Create(
      Control& parent, const AppCUI::Utils::ConstString& caption, const std::string_view& layout)
{
    return parent.AddControl<AppCUI::Controls::Panel>(Factory::Panel::Create(caption, layout));
}
std::unique_ptr<Panel> Factory::Panel::Create(const std::string_view& layout)
{
    return std::unique_ptr<AppCUI::Controls::Panel>(new AppCUI::Controls::Panel("", layout));
}
Panel* Factory::Panel::Create(Control& parent, const std::string_view& layout)
{
    return parent.AddControl<AppCUI::Controls::Panel>(Factory::Panel::Create(layout));
}