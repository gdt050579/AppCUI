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

//======[TEXTFIELD]==================================================================================
AppCUI::Controls::TextField* Factory::TextField::Create(
      AppCUI::Controls::Control& parent,
      const AppCUI::Utils::ConstString& caption,
      const std::string_view& layout,
      AppCUI::Controls::TextFieldFlags flags,
      Handlers::SyntaxHighlightHandler handler,
      void* handlerContext)
{
    return parent.AddControl<AppCUI::Controls::TextField>(
          Factory::TextField::Create(caption, layout, flags, handler, handlerContext));
}
std::unique_ptr<AppCUI::Controls::TextField> Factory::TextField::Create(
      const AppCUI::Utils::ConstString& caption,
      const std::string_view& layout,
      AppCUI::Controls::TextFieldFlags flags,
      Handlers::SyntaxHighlightHandler handler,
      void* handlerContext)
{
    return std::unique_ptr<AppCUI::Controls::TextField>(
          new AppCUI::Controls::TextField(caption, layout, flags, handler, handlerContext));
}

//======[TEXTAREA]===================================================================================
AppCUI::Controls::TextArea* Factory::TextArea::Create(
      AppCUI::Controls::Control& parent,
      const AppCUI::Utils::ConstString& caption,
      const std::string_view& layout,
      AppCUI::Controls::TextAreaFlags flags,
      Handlers::SyntaxHighlightHandler handler,
      void* handlerContext)
{
    return parent.AddControl<AppCUI::Controls::TextArea>(
          Factory::TextArea::Create(caption, layout, flags, handler, handlerContext));
}
std::unique_ptr<AppCUI::Controls::TextArea> Factory::TextArea::Create(
      const AppCUI::Utils::ConstString& caption,
      const std::string_view& layout,
      AppCUI::Controls::TextAreaFlags flags,
      Handlers::SyntaxHighlightHandler handler,
      void* handlerContext)
{
    return std::unique_ptr<AppCUI::Controls::TextArea>(
          new AppCUI::Controls::TextArea(caption, layout, flags, handler, handlerContext));
}

//======[TABPAGE]====================================================================================
AppCUI::Controls::TabPage* Factory::TabPage::Create(
      AppCUI::Controls::Control& parent, const AppCUI::Utils::ConstString& caption)
{
    return parent.AddControl<AppCUI::Controls::TabPage>(Factory::TabPage::Create(caption));
}
std::unique_ptr<AppCUI::Controls::TabPage> Factory::TabPage::Create(const AppCUI::Utils::ConstString& caption)
{
    return std::unique_ptr<AppCUI::Controls::TabPage>(new AppCUI::Controls::TabPage(caption));
}

//======[TAB]========================================================================================
AppCUI::Controls::Tab* Factory::Tab::Create(
      AppCUI::Controls::Control& parent,
      const std::string_view& layout,
      AppCUI::Controls::TabFlags flags,
      unsigned int tabPageSize)
{
    return parent.AddControl<AppCUI::Controls::Tab>(Factory::Tab::Create(layout, flags, tabPageSize));
}
std::unique_ptr<AppCUI::Controls::Tab> Factory::Tab::Create(
      const std::string_view& layout, AppCUI::Controls::TabFlags flags, unsigned int tabPageSize)
{
    return std::unique_ptr<AppCUI::Controls::Tab>(new AppCUI::Controls::Tab(layout, flags, tabPageSize));
}

//======[CANVASVIEWER]===============================================================================
AppCUI::Controls::CanvasViewer* Factory::CanvasViewer::Create(
      AppCUI::Controls::Control& parent,
      const std::string_view& layout,
      unsigned int canvasWidth,
      unsigned int canvasHeight,
      AppCUI::Controls::ViewerFlags flags)
{
    return parent.AddControl<AppCUI::Controls::CanvasViewer>(
          Factory::CanvasViewer::Create(layout, canvasWidth, canvasHeight, flags));
}
std::unique_ptr<AppCUI::Controls::CanvasViewer> Factory::CanvasViewer::Create(
      const std::string_view& layout,
      unsigned int canvasWidth,
      unsigned int canvasHeight,
      AppCUI::Controls::ViewerFlags flags)
{
    return std::unique_ptr<AppCUI::Controls::CanvasViewer>(
          new AppCUI::Controls::CanvasViewer("", layout, canvasWidth, canvasHeight, flags));
}
AppCUI::Controls::CanvasViewer* Factory::CanvasViewer::Create(
      AppCUI::Controls::Control& parent,
      const AppCUI::Utils::ConstString& caption,
      const std::string_view& layout,
      unsigned int canvasWidth,
      unsigned int canvasHeight,
      AppCUI::Controls::ViewerFlags flags)
{
    return parent.AddControl<AppCUI::Controls::CanvasViewer>(
          Factory::CanvasViewer::Create(caption, layout, canvasWidth, canvasHeight, flags));
}
std::unique_ptr<AppCUI::Controls::CanvasViewer> Factory::CanvasViewer::Create(
      const AppCUI::Utils::ConstString& caption,
      const std::string_view& layout,
      unsigned int canvasWidth,
      unsigned int canvasHeight,
      AppCUI::Controls::ViewerFlags flags)
{
    return std::unique_ptr<AppCUI::Controls::CanvasViewer>(
          new AppCUI::Controls::CanvasViewer(caption, layout, canvasWidth, canvasHeight, flags));
}

//======[IMAGEVIEWER]================================================================================
std::unique_ptr<AppCUI::Controls::ImageViewer> Factory::ImageViewer::Create(
      const std::string_view& layout, AppCUI::Controls::ViewerFlags flags)
{
    return std::unique_ptr<AppCUI::Controls::ImageViewer>(new AppCUI::Controls::ImageViewer("", layout, flags));
}
AppCUI::Controls::ImageViewer* Factory::ImageViewer::Create(
      AppCUI::Controls::Control& parent, const std::string_view& layout, AppCUI::Controls::ViewerFlags flags)
{
    return parent.AddControl<AppCUI::Controls::ImageViewer>(Factory::ImageViewer::Create("", layout, flags));
}
std::unique_ptr<AppCUI::Controls::ImageViewer> Factory::ImageViewer::Create(
      const AppCUI::Utils::ConstString& caption, const std::string_view& layout, AppCUI::Controls::ViewerFlags flags)
{
    return std::unique_ptr<AppCUI::Controls::ImageViewer>(new AppCUI::Controls::ImageViewer(caption, layout, flags));
}
AppCUI::Controls::ImageViewer* Factory::ImageViewer::Create(
      AppCUI::Controls::Control& parent,
      const AppCUI::Utils::ConstString& caption,
      const std::string_view& layout,
      AppCUI::Controls::ViewerFlags flags)
{
    return parent.AddControl<AppCUI::Controls::ImageViewer>(Factory::ImageViewer::Create(caption, layout, flags));
}

//======[LISTVIEW]===================================================================================
std::unique_ptr<AppCUI::Controls::ListView> Factory::ListView::Create(
      const std::string_view& layout, AppCUI::Controls::ListViewFlags flags)
{
    return std::unique_ptr<AppCUI::Controls::ListView>(new AppCUI::Controls::ListView(layout, flags));
}
AppCUI::Controls::ListView* Factory::ListView::Create(
      AppCUI::Controls::Control& parent,
      const std::string_view& layout,
      AppCUI::Controls::ListViewFlags flags)
{
    return parent.AddControl<AppCUI::Controls::ListView>(Factory::ListView::Create(layout, flags));
}


//======[COMBOBOX]===================================================================================
std::unique_ptr<AppCUI::Controls::ComboBox> Factory::ComboBox::Create(
      const std::string_view& layout,
      const AppCUI::Utils::ConstString& text,
      char itemsSeparator)
{
    return std::unique_ptr<AppCUI::Controls::ComboBox>(new AppCUI::Controls::ComboBox(layout, text, itemsSeparator));
}

AppCUI::Controls::ComboBox* Factory::ComboBox::Create(
      AppCUI::Controls::Control& parent,
      const std::string_view& layout,
      const AppCUI::Utils::ConstString& text,
      char itemsSeparator)
{
    return parent.AddControl<AppCUI::Controls::ComboBox>(Factory::ComboBox::Create(layout, text, itemsSeparator));
}