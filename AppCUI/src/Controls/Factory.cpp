#include "ControlContext.hpp"

#define VALIDATE_PARENT CHECK(parent, nullptr, "Expecting a valid (non-null) parent object !");
#define POINTER         Utils::Pointer
#define REFERENCE       Utils::Reference

namespace AppCUI
{
//======[LABEL]======================================================================================
POINTER<Label> Factory::Label::Create(const Utils::ConstString& caption, string_view layout)
{
    return POINTER<Controls::Label>(new Controls::Label(caption, layout));
}
REFERENCE<Label> Factory::Label::Create(Control* parent, const Utils::ConstString& caption, string_view layout)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::Label>(Factory::Label::Create(caption, layout));
}
REFERENCE<Label> Factory::Label::Create(Control& parent, const Utils::ConstString& caption, string_view layout)
{
    return parent.AddControl<Controls::Label>(Factory::Label::Create(caption, layout));
}
//======[BUTTON]=====================================================================================
POINTER<Button> Factory::Button::Create(
      const Utils::ConstString& caption, string_view layout, int controlID, Controls::ButtonFlags flags)
{
    return POINTER<Controls::Button>(new Controls::Button(caption, layout, controlID, flags));
}
REFERENCE<Button> Factory::Button::Create(
      Control* parent,
      const Utils::ConstString& caption,
      string_view layout,
      int controlID,
      Controls::ButtonFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::Button>(Factory::Button::Create(caption, layout, controlID, flags));
}
REFERENCE<Button> Factory::Button::Create(
      Control& parent,
      const Utils::ConstString& caption,
      string_view layout,
      int controlID,
      Controls::ButtonFlags flags)
{
    return parent.AddControl<Controls::Button>(Factory::Button::Create(caption, layout, controlID, flags));
}

//======[PASSWORD]===================================================================================
POINTER<Password> Factory::Password::Create(const Utils::ConstString& caption, string_view layout)
{
    return POINTER<Controls::Password>(new Controls::Password(caption, layout));
}
REFERENCE<Password> Factory::Password::Create(
      Control* parent, const Utils::ConstString& caption, string_view layout)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::Password>(Factory::Password::Create(caption, layout));
}
REFERENCE<Password> Factory::Password::Create(
      Control& parent, const Utils::ConstString& caption, string_view layout)
{
    return parent.AddControl<Controls::Password>(Factory::Password::Create(caption, layout));
}

//======[CHECKBOX]===================================================================================
POINTER<CheckBox> Factory::CheckBox::Create(const Utils::ConstString& caption, string_view layout, int controlID)
{
    return POINTER<Controls::CheckBox>(new Controls::CheckBox(caption, layout, controlID));
}
REFERENCE<CheckBox> Factory::CheckBox::Create(
      Control* parent, const Utils::ConstString& caption, string_view layout, int controlID)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::CheckBox>(Factory::CheckBox::Create(caption, layout, controlID));
}
REFERENCE<CheckBox> Factory::CheckBox::Create(
      Control& parent, const Utils::ConstString& caption, string_view layout, int controlID)
{
    return parent.AddControl<Controls::CheckBox>(Factory::CheckBox::Create(caption, layout, controlID));
}

//======[RADIOBOX]===================================================================================
POINTER<RadioBox> Factory::RadioBox::Create(
      const Utils::ConstString& caption, string_view layout, int groupID, int controlID)
{
    return POINTER<Controls::RadioBox>(new Controls::RadioBox(caption, layout, groupID, controlID));
}
REFERENCE<RadioBox> Factory::RadioBox::Create(
      Control* parent, const Utils::ConstString& caption, string_view layout, int groupID, int controlID)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::RadioBox>(Factory::RadioBox::Create(caption, layout, groupID, controlID));
}
REFERENCE<RadioBox> Factory::RadioBox::Create(
      Control& parent, const Utils::ConstString& caption, string_view layout, int groupID, int controlID)
{
    return parent.AddControl<Controls::RadioBox>(Factory::RadioBox::Create(caption, layout, groupID, controlID));
}

//======[SPLITTER]===================================================================================
POINTER<Splitter> Factory::Splitter::Create(string_view layout, bool vertical)
{
    return POINTER<Controls::Splitter>(new Controls::Splitter(layout, vertical));
}
REFERENCE<Splitter> Factory::Splitter::Create(Control* parent, string_view layout, bool vertical)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::Splitter>(Factory::Splitter::Create(layout, vertical));
}
REFERENCE<Splitter> Factory::Splitter::Create(Control& parent, string_view layout, bool vertical)
{
    return parent.AddControl<Controls::Splitter>(Factory::Splitter::Create(layout, vertical));
}

//======[PANEL]======================================================================================
POINTER<Panel> Factory::Panel::Create(const Utils::ConstString& caption, string_view layout)
{
    return POINTER<Controls::Panel>(new Controls::Panel(caption, layout));
}
REFERENCE<Panel> Factory::Panel::Create(Control* parent, const Utils::ConstString& caption, string_view layout)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::Panel>(Factory::Panel::Create(caption, layout));
}
REFERENCE<Panel> Factory::Panel::Create(Control& parent, const Utils::ConstString& caption, string_view layout)
{
    return parent.AddControl<Controls::Panel>(Factory::Panel::Create(caption, layout));
}
POINTER<Panel> Factory::Panel::Create(string_view layout)
{
    return POINTER<Controls::Panel>(new Controls::Panel("", layout));
}
REFERENCE<Panel> Factory::Panel::Create(Control* parent, string_view layout)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::Panel>(Factory::Panel::Create(layout));
}
REFERENCE<Panel> Factory::Panel::Create(Control& parent, string_view layout)
{
    return parent.AddControl<Controls::Panel>(Factory::Panel::Create(layout));
}

//======[TEXTFIELD]==================================================================================
REFERENCE<TextField> Factory::TextField::Create(
      Controls::Control* parent,
      const Utils::ConstString& caption,
      string_view layout,
      Controls::TextFieldFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::TextField>(Factory::TextField::Create(caption, layout, flags));
}
REFERENCE<TextField> Factory::TextField::Create(
      Controls::Control& parent,
      const Utils::ConstString& caption,
      string_view layout,
      Controls::TextFieldFlags flags)
{
    return parent.AddControl<Controls::TextField>(Factory::TextField::Create(caption, layout, flags));
}
POINTER<Controls::TextField> Factory::TextField::Create(
      const Utils::ConstString& caption, string_view layout, Controls::TextFieldFlags flags)
{
    return POINTER<Controls::TextField>(new Controls::TextField(caption, layout, flags));
}

//======[TEXTAREA]===================================================================================
REFERENCE<TextArea> Factory::TextArea::Create(
      Controls::Control* parent,
      const Utils::ConstString& caption,
      string_view layout,
      Controls::TextAreaFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::TextArea>(Factory::TextArea::Create(caption, layout, flags));
}
REFERENCE<TextArea> Factory::TextArea::Create(
      Controls::Control& parent,
      const Utils::ConstString& caption,
      string_view layout,
      Controls::TextAreaFlags flags)
{
    return parent.AddControl<Controls::TextArea>(Factory::TextArea::Create(caption, layout, flags));
}
POINTER<Controls::TextArea> Factory::TextArea::Create(
      const Utils::ConstString& caption, string_view layout, Controls::TextAreaFlags flags)
{
    return POINTER<Controls::TextArea>(new Controls::TextArea(caption, layout, flags));
}

//======[TABPAGE]====================================================================================
REFERENCE<TabPage> Factory::TabPage::Create(Controls::Control* parent, const Utils::ConstString& caption)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::TabPage>(Factory::TabPage::Create(caption));
}
REFERENCE<TabPage> Factory::TabPage::Create(Controls::Control& parent, const Utils::ConstString& caption)
{
    return parent.AddControl<Controls::TabPage>(Factory::TabPage::Create(caption));
}
POINTER<Controls::TabPage> Factory::TabPage::Create(const Utils::ConstString& caption)
{
    return POINTER<Controls::TabPage>(new Controls::TabPage(caption));
}

//======[TAB]========================================================================================
REFERENCE<Tab> Factory::Tab::Create(
      Controls::Control* parent, string_view layout, Controls::TabFlags flags, unsigned int tabPageSize)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::Tab>(Factory::Tab::Create(layout, flags, tabPageSize));
}
REFERENCE<Tab> Factory::Tab::Create(
      Controls::Control& parent, string_view layout, Controls::TabFlags flags, unsigned int tabPageSize)
{
    return parent.AddControl<Controls::Tab>(Factory::Tab::Create(layout, flags, tabPageSize));
}
POINTER<Tab> Factory::Tab::Create(string_view layout, Controls::TabFlags flags, unsigned int tabPageSize)
{
    return POINTER<Controls::Tab>(new Controls::Tab(layout, flags, tabPageSize));
}

//======[CANVASVIEWER]===============================================================================
REFERENCE<CanvasViewer> Factory::CanvasViewer::Create(
      Controls::Control* parent,
      string_view layout,
      unsigned int canvasWidth,
      unsigned int canvasHeight,
      Controls::ViewerFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::CanvasViewer>(
          Factory::CanvasViewer::Create(layout, canvasWidth, canvasHeight, flags));
}
REFERENCE<CanvasViewer> Factory::CanvasViewer::Create(
      Controls::Control& parent,
      string_view layout,
      unsigned int canvasWidth,
      unsigned int canvasHeight,
      Controls::ViewerFlags flags)
{
    return parent.AddControl<Controls::CanvasViewer>(
          Factory::CanvasViewer::Create(layout, canvasWidth, canvasHeight, flags));
}
POINTER<Controls::CanvasViewer> Factory::CanvasViewer::Create(
      string_view layout, unsigned int canvasWidth, unsigned int canvasHeight, Controls::ViewerFlags flags)
{
    return POINTER<Controls::CanvasViewer>(new Controls::CanvasViewer("", layout, canvasWidth, canvasHeight, flags));
}
REFERENCE<CanvasViewer> Factory::CanvasViewer::Create(
      Controls::Control* parent,
      const Utils::ConstString& caption,
      string_view layout,
      unsigned int canvasWidth,
      unsigned int canvasHeight,
      Controls::ViewerFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::CanvasViewer>(
          Factory::CanvasViewer::Create(caption, layout, canvasWidth, canvasHeight, flags));
}
REFERENCE<CanvasViewer> Factory::CanvasViewer::Create(
      Controls::Control& parent,
      const Utils::ConstString& caption,
      string_view layout,
      unsigned int canvasWidth,
      unsigned int canvasHeight,
      Controls::ViewerFlags flags)
{
    return parent.AddControl<Controls::CanvasViewer>(
          Factory::CanvasViewer::Create(caption, layout, canvasWidth, canvasHeight, flags));
}
POINTER<Controls::CanvasViewer> Factory::CanvasViewer::Create(
      const Utils::ConstString& caption,
      string_view layout,
      unsigned int canvasWidth,
      unsigned int canvasHeight,
      Controls::ViewerFlags flags)
{
    return POINTER<Controls::CanvasViewer>(
          new Controls::CanvasViewer(caption, layout, canvasWidth, canvasHeight, flags));
}

//======[IMAGEVIEWER]================================================================================
POINTER<Controls::ImageViewer> Factory::ImageViewer::Create(string_view layout, Controls::ViewerFlags flags)
{
    return POINTER<Controls::ImageViewer>(new Controls::ImageViewer("", layout, flags));
}
REFERENCE<ImageViewer> Factory::ImageViewer::Create(
      Controls::Control* parent, string_view layout, Controls::ViewerFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::ImageViewer>(Factory::ImageViewer::Create("", layout, flags));
}
REFERENCE<ImageViewer> Factory::ImageViewer::Create(
      Controls::Control& parent, string_view layout, Controls::ViewerFlags flags)
{
    return parent.AddControl<Controls::ImageViewer>(Factory::ImageViewer::Create("", layout, flags));
}
POINTER<Controls::ImageViewer> Factory::ImageViewer::Create(
      const Utils::ConstString& caption, string_view layout, Controls::ViewerFlags flags)
{
    return POINTER<Controls::ImageViewer>(new Controls::ImageViewer(caption, layout, flags));
}
REFERENCE<ImageViewer> Factory::ImageViewer::Create(
      Controls::Control* parent,
      const Utils::ConstString& caption,
      string_view layout,
      Controls::ViewerFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::ImageViewer>(Factory::ImageViewer::Create(caption, layout, flags));
}
REFERENCE<ImageViewer> Factory::ImageViewer::Create(
      Controls::Control& parent,
      const Utils::ConstString& caption,
      string_view layout,
      Controls::ViewerFlags flags)
{
    return parent.AddControl<Controls::ImageViewer>(Factory::ImageViewer::Create(caption, layout, flags));
}

//======[LISTVIEW]===================================================================================
POINTER<Controls::ListView> Factory::ListView::Create(string_view layout, Controls::ListViewFlags flags)
{
    return POINTER<Controls::ListView>(new Controls::ListView(layout, flags));
}
REFERENCE<ListView> Factory::ListView::Create(
      Controls::Control* parent, string_view layout, Controls::ListViewFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::ListView>(Factory::ListView::Create(layout, flags));
}
REFERENCE<ListView> Factory::ListView::Create(
      Controls::Control& parent, string_view layout, Controls::ListViewFlags flags)
{
    return parent.AddControl<Controls::ListView>(Factory::ListView::Create(layout, flags));
}

//======[COMBOBOX]===================================================================================
POINTER<Controls::ComboBox> Factory::ComboBox::Create(
      string_view layout, const Utils::ConstString& text, char itemsSeparator)
{
    return POINTER<Controls::ComboBox>(new Controls::ComboBox(layout, text, itemsSeparator));
}

REFERENCE<ComboBox> Factory::ComboBox::Create(
      Controls::Control* parent, string_view layout, const Utils::ConstString& text, char itemsSeparator)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::ComboBox>(Factory::ComboBox::Create(layout, text, itemsSeparator));
}
REFERENCE<ComboBox> Factory::ComboBox::Create(
      Controls::Control& parent, string_view layout, const Utils::ConstString& text, char itemsSeparator)
{
    return parent.AddControl<Controls::ComboBox>(Factory::ComboBox::Create(layout, text, itemsSeparator));
}

//======[NUMERICSELECTOR]============================================================================
POINTER<Controls::NumericSelector> Factory::NumericSelector::Create(
      const long long minValue, const long long maxValue, long long value, string_view layout)
{
    return POINTER<Controls::NumericSelector>(new Controls::NumericSelector(minValue, maxValue, value, layout));
}
REFERENCE<NumericSelector> Factory::NumericSelector::Create(
      Controls::Control* parent,
      const long long minValue,
      const long long maxValue,
      long long value,
      string_view layout)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::NumericSelector>(
          Factory::NumericSelector::Create(minValue, maxValue, value, layout));
}
REFERENCE<NumericSelector> Factory::NumericSelector::Create(
      Controls::Control& parent,
      const long long minValue,
      const long long maxValue,
      long long value,
      string_view layout)
{
    return parent.AddControl<Controls::NumericSelector>(
          Factory::NumericSelector::Create(minValue, maxValue, value, layout));
}

//======[WINDOW]=====================================================================================
POINTER<Controls::Window> Factory::Window::Create(
      const Utils::ConstString& caption, string_view layout, Controls::WindowFlags flags)
{
    return POINTER<Controls::Window>(new Controls::Window(caption, layout, flags));
}

//======[DESKTOP]====================================================================================
POINTER<Controls::Desktop> Factory::Desktop::Create()
{
    return POINTER<Controls::Desktop>(new Controls::Desktop());
}

//======[TREE]=======================================================================================
POINTER<Controls::Tree> Factory::Tree::Create(string_view layout, TreeFlags flags, const unsigned int noOfColumns)
{
    return POINTER<Controls::Tree>(new Controls::Tree(layout, flags, noOfColumns));
}

REFERENCE<Tree> Factory::Tree::Create(
      Controls::Control* parent, string_view layout, const TreeFlags flags, const unsigned int noOfColumns)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::Tree>(Factory::Tree::Create(layout, flags, noOfColumns));
}

REFERENCE<Tree> Factory::Tree::Create(
      Controls::Control& parent, string_view layout, const TreeFlags flags, const unsigned int noOfColumns)
{
    return parent.AddControl<Controls::Tree>(Factory::Tree::Create(layout, flags, noOfColumns));
}

//======[GRID]=======================================================================================
Pointer<Controls::Grid> Factory::Grid::Create(
      string_view layout, unsigned int columnsNo, unsigned int rowsNo, Controls::GridFlags flags)
{
    return POINTER<Controls::Grid>(new Controls::Grid(layout, columnsNo, rowsNo, flags));
}

Reference<Controls::Grid> Factory::Grid::Create(
      Controls::Control* parent,
      string_view layout,
      unsigned int columnsNo,
      unsigned int rowsNo,
      Controls::GridFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::Grid>(Factory::Grid::Create(layout, columnsNo, rowsNo, flags));
}

Reference<Controls::Grid> Factory::Grid::Create(
      Controls::Control& parent,
      string_view layout,
      unsigned int columnsNo,
      unsigned int rowsNo,
      Controls::GridFlags flags)
{
    return parent.AddControl<Controls::Grid>(Factory::Grid::Create(layout, columnsNo, rowsNo, flags));
}
} // namespace AppCUI
#undef VALIDATE_PARENT
#undef POINTER
#undef REFERENCE
