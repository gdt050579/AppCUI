#include "ControlContext.hpp"

using namespace AppCUI::Controls;
#define VALIDATE_PARENT CHECK(parent, nullptr, "Expecting a valid (non-null) parent object !");
#define POINTER AppCUI::Controls::Pointer

//======[LABEL]======================================================================================
POINTER<Label> Factory::Label::Create(const AppCUI::Utils::ConstString& caption, const std::string_view& layout)
{
    return POINTER<AppCUI::Controls::Label>(new AppCUI::Controls::Label(caption, layout));
}
Label* Factory::Label::Create(
      Control* parent, const AppCUI::Utils::ConstString& caption, const std::string_view& layout)
{
    VALIDATE_PARENT;
    return parent->AddControl<AppCUI::Controls::Label>(Factory::Label::Create(caption, layout));
}

//======[BUTTON]=====================================================================================
POINTER<Button> Factory::Button::Create(
      const AppCUI::Utils::ConstString& caption,
      const std::string_view& layout,
      int controlID,
      AppCUI::Controls::ButtonFlags flags)
{
    return POINTER<AppCUI::Controls::Button>(new AppCUI::Controls::Button(caption, layout, controlID, flags));
}
Button* Factory::Button::Create(
      Control* parent,
      const AppCUI::Utils::ConstString& caption,
      const std::string_view& layout,
      int controlID,
      AppCUI::Controls::ButtonFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<AppCUI::Controls::Button>(Factory::Button::Create(caption, layout, controlID, flags));
}

//======[CHECKBOX]===================================================================================
POINTER<CheckBox> Factory::CheckBox::Create(
      const AppCUI::Utils::ConstString& caption, const std::string_view& layout, int controlID)
{
    return POINTER<AppCUI::Controls::CheckBox>(new AppCUI::Controls::CheckBox(caption, layout, controlID));
}
CheckBox* Factory::CheckBox::Create(
      Control* parent, const AppCUI::Utils::ConstString& caption, const std::string_view& layout, int controlID)
{
    VALIDATE_PARENT;
    return parent->AddControl<AppCUI::Controls::CheckBox>(Factory::CheckBox::Create(caption, layout, controlID));
}

//======[RADIOBOX]===================================================================================
POINTER<RadioBox> Factory::RadioBox::Create(
      const AppCUI::Utils::ConstString& caption, const std::string_view& layout, int groupID, int controlID)
{
    return POINTER<AppCUI::Controls::RadioBox>(
          new AppCUI::Controls::RadioBox(caption, layout, groupID, controlID));
}

RadioBox* Factory::RadioBox::Create(
      Control* parent,
      const AppCUI::Utils::ConstString& caption,
      const std::string_view& layout,
      int groupID,
      int controlID)
{
    VALIDATE_PARENT;
    return parent->AddControl<AppCUI::Controls::RadioBox>(
          Factory::RadioBox::Create(caption, layout, groupID, controlID));
}

//======[SPLITTER]===================================================================================
POINTER<Splitter> Factory::Splitter::Create(const std::string_view& layout, bool vertical)
{
    return POINTER<AppCUI::Controls::Splitter>(new AppCUI::Controls::Splitter(layout, vertical));
}
Splitter* Factory::Splitter::Create(Control* parent, const std::string_view& layout, bool vertical)
{
    VALIDATE_PARENT;
    return parent->AddControl<AppCUI::Controls::Splitter>(Factory::Splitter::Create(layout, vertical));
}

//======[PANEL]======================================================================================
POINTER<Panel> Factory::Panel::Create(const AppCUI::Utils::ConstString& caption, const std::string_view& layout)
{
    return POINTER<AppCUI::Controls::Panel>(new AppCUI::Controls::Panel(caption, layout));
}
Panel* Factory::Panel::Create(
      Control* parent, const AppCUI::Utils::ConstString& caption, const std::string_view& layout)
{
    VALIDATE_PARENT;
    return parent->AddControl<AppCUI::Controls::Panel>(Factory::Panel::Create(caption, layout));
}
POINTER<Panel> Factory::Panel::Create(const std::string_view& layout)
{
    return POINTER<AppCUI::Controls::Panel>(new AppCUI::Controls::Panel("", layout));
}
Panel* Factory::Panel::Create(Control* parent, const std::string_view& layout)
{
    VALIDATE_PARENT;
    return parent->AddControl<AppCUI::Controls::Panel>(Factory::Panel::Create(layout));
}

//======[TEXTFIELD]==================================================================================
AppCUI::Controls::TextField* Factory::TextField::Create(
      AppCUI::Controls::Control* parent,
      const AppCUI::Utils::ConstString& caption,
      const std::string_view& layout,
      AppCUI::Controls::TextFieldFlags flags,
      Handlers::SyntaxHighlightHandler handler,
      void* handlerContext)
{
    VALIDATE_PARENT;
    return parent->AddControl<AppCUI::Controls::TextField>(
          Factory::TextField::Create(caption, layout, flags, handler, handlerContext));
}
POINTER<AppCUI::Controls::TextField> Factory::TextField::Create(
      const AppCUI::Utils::ConstString& caption,
      const std::string_view& layout,
      AppCUI::Controls::TextFieldFlags flags,
      Handlers::SyntaxHighlightHandler handler,
      void* handlerContext)
{
    return POINTER<AppCUI::Controls::TextField>(
          new AppCUI::Controls::TextField(caption, layout, flags, handler, handlerContext));
}

//======[TEXTAREA]===================================================================================
AppCUI::Controls::TextArea* Factory::TextArea::Create(
      AppCUI::Controls::Control* parent,
      const AppCUI::Utils::ConstString& caption,
      const std::string_view& layout,
      AppCUI::Controls::TextAreaFlags flags,
      Handlers::SyntaxHighlightHandler handler,
      void* handlerContext)
{
    VALIDATE_PARENT;
    return parent->AddControl<AppCUI::Controls::TextArea>(
          Factory::TextArea::Create(caption, layout, flags, handler, handlerContext));
}
POINTER<AppCUI::Controls::TextArea> Factory::TextArea::Create(
      const AppCUI::Utils::ConstString& caption,
      const std::string_view& layout,
      AppCUI::Controls::TextAreaFlags flags,
      Handlers::SyntaxHighlightHandler handler,
      void* handlerContext)
{
    return POINTER<AppCUI::Controls::TextArea>(
          new AppCUI::Controls::TextArea(caption, layout, flags, handler, handlerContext));
}

//======[TABPAGE]====================================================================================
AppCUI::Controls::TabPage* Factory::TabPage::Create(
      AppCUI::Controls::Control* parent, const AppCUI::Utils::ConstString& caption)
{
    VALIDATE_PARENT;
    return parent->AddControl<AppCUI::Controls::TabPage>(Factory::TabPage::Create(caption));
}
POINTER<AppCUI::Controls::TabPage> Factory::TabPage::Create(const AppCUI::Utils::ConstString& caption)
{
    return POINTER<AppCUI::Controls::TabPage>(new AppCUI::Controls::TabPage(caption));
}

//======[TAB]========================================================================================
AppCUI::Controls::Tab* Factory::Tab::Create(
      AppCUI::Controls::Control* parent,
      const std::string_view& layout,
      AppCUI::Controls::TabFlags flags,
      unsigned int tabPageSize)
{
    VALIDATE_PARENT;
    return parent->AddControl<AppCUI::Controls::Tab>(Factory::Tab::Create(layout, flags, tabPageSize));
}
POINTER<AppCUI::Controls::Tab> Factory::Tab::Create(
      const std::string_view& layout, AppCUI::Controls::TabFlags flags, unsigned int tabPageSize)
{
    return POINTER<AppCUI::Controls::Tab>(new AppCUI::Controls::Tab(layout, flags, tabPageSize));
}

//======[CANVASVIEWER]===============================================================================
AppCUI::Controls::CanvasViewer* Factory::CanvasViewer::Create(
      AppCUI::Controls::Control* parent,
      const std::string_view& layout,
      unsigned int canvasWidth,
      unsigned int canvasHeight,
      AppCUI::Controls::ViewerFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<AppCUI::Controls::CanvasViewer>(
          Factory::CanvasViewer::Create(layout, canvasWidth, canvasHeight, flags));
}
POINTER<AppCUI::Controls::CanvasViewer> Factory::CanvasViewer::Create(
      const std::string_view& layout,
      unsigned int canvasWidth,
      unsigned int canvasHeight,
      AppCUI::Controls::ViewerFlags flags)
{
    return POINTER<AppCUI::Controls::CanvasViewer>(
          new AppCUI::Controls::CanvasViewer("", layout, canvasWidth, canvasHeight, flags));
}
AppCUI::Controls::CanvasViewer* Factory::CanvasViewer::Create(
      AppCUI::Controls::Control* parent,
      const AppCUI::Utils::ConstString& caption,
      const std::string_view& layout,
      unsigned int canvasWidth,
      unsigned int canvasHeight,
      AppCUI::Controls::ViewerFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<AppCUI::Controls::CanvasViewer>(
          Factory::CanvasViewer::Create(caption, layout, canvasWidth, canvasHeight, flags));
}
POINTER<AppCUI::Controls::CanvasViewer> Factory::CanvasViewer::Create(
      const AppCUI::Utils::ConstString& caption,
      const std::string_view& layout,
      unsigned int canvasWidth,
      unsigned int canvasHeight,
      AppCUI::Controls::ViewerFlags flags)
{
    return POINTER<AppCUI::Controls::CanvasViewer>(
          new AppCUI::Controls::CanvasViewer(caption, layout, canvasWidth, canvasHeight, flags));
}

//======[IMAGEVIEWER]================================================================================
POINTER<AppCUI::Controls::ImageViewer> Factory::ImageViewer::Create(
      const std::string_view& layout, AppCUI::Controls::ViewerFlags flags)
{
    return POINTER<AppCUI::Controls::ImageViewer>(new AppCUI::Controls::ImageViewer("", layout, flags));
}
AppCUI::Controls::ImageViewer* Factory::ImageViewer::Create(
      AppCUI::Controls::Control* parent, const std::string_view& layout, AppCUI::Controls::ViewerFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<AppCUI::Controls::ImageViewer>(Factory::ImageViewer::Create("", layout, flags));
}
POINTER<AppCUI::Controls::ImageViewer> Factory::ImageViewer::Create(
      const AppCUI::Utils::ConstString& caption, const std::string_view& layout, AppCUI::Controls::ViewerFlags flags)
{
    return POINTER<AppCUI::Controls::ImageViewer>(new AppCUI::Controls::ImageViewer(caption, layout, flags));
}
AppCUI::Controls::ImageViewer* Factory::ImageViewer::Create(
      AppCUI::Controls::Control* parent,
      const AppCUI::Utils::ConstString& caption,
      const std::string_view& layout,
      AppCUI::Controls::ViewerFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<AppCUI::Controls::ImageViewer>(Factory::ImageViewer::Create(caption, layout, flags));
}

//======[LISTVIEW]===================================================================================
POINTER<AppCUI::Controls::ListView> Factory::ListView::Create(
      const std::string_view& layout, AppCUI::Controls::ListViewFlags flags)
{
    return POINTER<AppCUI::Controls::ListView>(new AppCUI::Controls::ListView(layout, flags));
}
AppCUI::Controls::ListView* Factory::ListView::Create(
      AppCUI::Controls::Control* parent, const std::string_view& layout, AppCUI::Controls::ListViewFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<AppCUI::Controls::ListView>(Factory::ListView::Create(layout, flags));
}

//======[COMBOBOX]===================================================================================
POINTER<AppCUI::Controls::ComboBox> Factory::ComboBox::Create(
      const std::string_view& layout, const AppCUI::Utils::ConstString& text, char itemsSeparator)
{
    return POINTER<AppCUI::Controls::ComboBox>(new AppCUI::Controls::ComboBox(layout, text, itemsSeparator));
}

AppCUI::Controls::ComboBox* Factory::ComboBox::Create(
      AppCUI::Controls::Control* parent,
      const std::string_view& layout,
      const AppCUI::Utils::ConstString& text,
      char itemsSeparator)
{
    VALIDATE_PARENT;
    return parent->AddControl<AppCUI::Controls::ComboBox>(Factory::ComboBox::Create(layout, text, itemsSeparator));
}

//======[NUMERICSELECTOR]============================================================================
POINTER<AppCUI::Controls::NumericSelector> Factory::NumericSelector::Create(
      const long long minValue, const long long maxValue, long long value, const std::string_view& layout)
{
    return POINTER<AppCUI::Controls::NumericSelector>(
          new AppCUI::Controls::NumericSelector(minValue, maxValue, value, layout));
}
AppCUI::Controls::NumericSelector* Factory::NumericSelector::Create(
      AppCUI::Controls::Control* parent,
      const long long minValue,
      const long long maxValue,
      long long value,
      const std::string_view& layout)
{
    VALIDATE_PARENT;
    return parent->AddControl<AppCUI::Controls::NumericSelector>(
          Factory::NumericSelector::Create(minValue, maxValue, value, layout));
}


 //======[WINDOW]=====================================================================================
POINTER<AppCUI::Controls::Window> Factory::Window::Create(
      const AppCUI::Utils::ConstString& caption,
      const std::string_view& layout,
      AppCUI::Controls::WindowFlags flags)
{
    return POINTER<AppCUI::Controls::Window>(new AppCUI::Controls::Window(caption, layout, flags));
}

 //======[DESKTOP]====================================================================================
POINTER<AppCUI::Controls::Desktop> Factory::Desktop::Create()
{
    return POINTER<AppCUI::Controls::Desktop>(new AppCUI::Controls::Desktop());
}

#undef VALIDATE_PARENT
#undef POINTER
