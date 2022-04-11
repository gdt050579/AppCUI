#include "ControlContext.hpp"

#define VALIDATE_PARENT CHECK(parent, nullptr, "Expecting a valid (non-null) parent object !");
#define POINTER         Utils::Pointer
#define REFERENCE       Utils::Reference

namespace AppCUI
{
//======[LABEL]======================================================================================
POINTER<Label> Factory::Label::Create(const ConstString& caption, string_view layout)
{
    return POINTER<Controls::Label>(new Controls::Label(caption, layout));
}
REFERENCE<Label> Factory::Label::Create(Control* parent, const ConstString& caption, string_view layout)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::Label>(Factory::Label::Create(caption, layout));
}
REFERENCE<Label> Factory::Label::Create(Control& parent, const ConstString& caption, string_view layout)
{
    return parent.AddControl<Controls::Label>(Factory::Label::Create(caption, layout));
}
//======[BUTTON]=====================================================================================
POINTER<Button> Factory::Button::Create(
      const ConstString& caption, string_view layout, int controlID, Controls::ButtonFlags flags)
{
    return POINTER<Controls::Button>(new Controls::Button(caption, layout, controlID, flags));
}
REFERENCE<Button> Factory::Button::Create(
      Control* parent, const ConstString& caption, string_view layout, int controlID, Controls::ButtonFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::Button>(Factory::Button::Create(caption, layout, controlID, flags));
}
REFERENCE<Button> Factory::Button::Create(
      Control& parent, const ConstString& caption, string_view layout, int controlID, Controls::ButtonFlags flags)
{
    return parent.AddControl<Controls::Button>(Factory::Button::Create(caption, layout, controlID, flags));
}

//======[PASSWORD]===================================================================================
POINTER<Password> Factory::Password::Create(const ConstString& caption, string_view layout)
{
    return POINTER<Controls::Password>(new Controls::Password(caption, layout));
}
REFERENCE<Password> Factory::Password::Create(Control* parent, const ConstString& caption, string_view layout)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::Password>(Factory::Password::Create(caption, layout));
}
REFERENCE<Password> Factory::Password::Create(Control& parent, const ConstString& caption, string_view layout)
{
    return parent.AddControl<Controls::Password>(Factory::Password::Create(caption, layout));
}

//======[CHECKBOX]===================================================================================
POINTER<CheckBox> Factory::CheckBox::Create(const ConstString& caption, string_view layout, int controlID)
{
    return POINTER<Controls::CheckBox>(new Controls::CheckBox(caption, layout, controlID));
}
REFERENCE<CheckBox> Factory::CheckBox::Create(
      Control* parent, const ConstString& caption, string_view layout, int controlID)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::CheckBox>(Factory::CheckBox::Create(caption, layout, controlID));
}
REFERENCE<CheckBox> Factory::CheckBox::Create(
      Control& parent, const ConstString& caption, string_view layout, int controlID)
{
    return parent.AddControl<Controls::CheckBox>(Factory::CheckBox::Create(caption, layout, controlID));
}

//======[RADIOBOX]===================================================================================
POINTER<RadioBox> Factory::RadioBox::Create(const ConstString& caption, string_view layout, int groupID, int controlID)
{
    return POINTER<Controls::RadioBox>(new Controls::RadioBox(caption, layout, groupID, controlID));
}
REFERENCE<RadioBox> Factory::RadioBox::Create(
      Control* parent, const ConstString& caption, string_view layout, int groupID, int controlID)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::RadioBox>(Factory::RadioBox::Create(caption, layout, groupID, controlID));
}
REFERENCE<RadioBox> Factory::RadioBox::Create(
      Control& parent, const ConstString& caption, string_view layout, int groupID, int controlID)
{
    return parent.AddControl<Controls::RadioBox>(Factory::RadioBox::Create(caption, layout, groupID, controlID));
}

//======[SPLITTER]===================================================================================
POINTER<Splitter> Factory::Splitter::Create(string_view layout, SplitterFlags flags)
{
    return POINTER<Controls::Splitter>(new Controls::Splitter(layout, flags));
}
REFERENCE<Splitter> Factory::Splitter::Create(Control* parent, string_view layout, SplitterFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::Splitter>(Factory::Splitter::Create(layout, flags));
}
REFERENCE<Splitter> Factory::Splitter::Create(Control& parent, string_view layout, SplitterFlags flags)
{
    return parent.AddControl<Controls::Splitter>(Factory::Splitter::Create(layout, flags));
}

//======[PANEL]======================================================================================
POINTER<Panel> Factory::Panel::Create(const ConstString& caption, string_view layout)
{
    return POINTER<Controls::Panel>(new Controls::Panel(caption, layout));
}
REFERENCE<Panel> Factory::Panel::Create(Control* parent, const ConstString& caption, string_view layout)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::Panel>(Factory::Panel::Create(caption, layout));
}
REFERENCE<Panel> Factory::Panel::Create(Control& parent, const ConstString& caption, string_view layout)
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
      Controls::Control* parent, const ConstString& caption, string_view layout, Controls::TextFieldFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::TextField>(Factory::TextField::Create(caption, layout, flags));
}
REFERENCE<TextField> Factory::TextField::Create(
      Controls::Control& parent, const ConstString& caption, string_view layout, Controls::TextFieldFlags flags)
{
    return parent.AddControl<Controls::TextField>(Factory::TextField::Create(caption, layout, flags));
}
POINTER<Controls::TextField> Factory::TextField::Create(
      const ConstString& caption, string_view layout, Controls::TextFieldFlags flags)
{
    return POINTER<Controls::TextField>(new Controls::TextField(caption, layout, flags));
}

//======[TEXTAREA]===================================================================================
REFERENCE<TextArea> Factory::TextArea::Create(
      Controls::Control* parent, const ConstString& caption, string_view layout, Controls::TextAreaFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::TextArea>(Factory::TextArea::Create(caption, layout, flags));
}
REFERENCE<TextArea> Factory::TextArea::Create(
      Controls::Control& parent, const ConstString& caption, string_view layout, Controls::TextAreaFlags flags)
{
    return parent.AddControl<Controls::TextArea>(Factory::TextArea::Create(caption, layout, flags));
}
POINTER<Controls::TextArea> Factory::TextArea::Create(
      const ConstString& caption, string_view layout, Controls::TextAreaFlags flags)
{
    return POINTER<Controls::TextArea>(new Controls::TextArea(caption, layout, flags));
}

//======[TABPAGE]====================================================================================
REFERENCE<TabPage> Factory::TabPage::Create(Controls::Control* parent, const ConstString& caption)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::TabPage>(Factory::TabPage::Create(caption));
}
REFERENCE<TabPage> Factory::TabPage::Create(Controls::Control& parent, const ConstString& caption)
{
    return parent.AddControl<Controls::TabPage>(Factory::TabPage::Create(caption));
}
POINTER<Controls::TabPage> Factory::TabPage::Create(const ConstString& caption)
{
    return POINTER<Controls::TabPage>(new Controls::TabPage(caption));
}

//======[TAB]========================================================================================
REFERENCE<Tab> Factory::Tab::Create(
      Controls::Control* parent, string_view layout, Controls::TabFlags flags, uint32 tabPageSize)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::Tab>(Factory::Tab::Create(layout, flags, tabPageSize));
}
REFERENCE<Tab> Factory::Tab::Create(
      Controls::Control& parent, string_view layout, Controls::TabFlags flags, uint32 tabPageSize)
{
    return parent.AddControl<Controls::Tab>(Factory::Tab::Create(layout, flags, tabPageSize));
}
POINTER<Tab> Factory::Tab::Create(string_view layout, Controls::TabFlags flags, uint32 tabPageSize)
{
    return POINTER<Controls::Tab>(new Controls::Tab(layout, flags, tabPageSize));
}

//======[CANVASVIEWER]===============================================================================
REFERENCE<CanvasViewer> Factory::CanvasViewer::Create(
      Controls::Control* parent,
      string_view layout,
      uint32 canvasWidth,
      uint32 canvasHeight,
      Controls::ViewerFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::CanvasViewer>(
          Factory::CanvasViewer::Create(layout, canvasWidth, canvasHeight, flags));
}
REFERENCE<CanvasViewer> Factory::CanvasViewer::Create(
      Controls::Control& parent,
      string_view layout,
      uint32 canvasWidth,
      uint32 canvasHeight,
      Controls::ViewerFlags flags)
{
    return parent.AddControl<Controls::CanvasViewer>(
          Factory::CanvasViewer::Create(layout, canvasWidth, canvasHeight, flags));
}
POINTER<Controls::CanvasViewer> Factory::CanvasViewer::Create(
      string_view layout, uint32 canvasWidth, uint32 canvasHeight, Controls::ViewerFlags flags)
{
    return POINTER<Controls::CanvasViewer>(new Controls::CanvasViewer("", layout, canvasWidth, canvasHeight, flags));
}
REFERENCE<CanvasViewer> Factory::CanvasViewer::Create(
      Controls::Control* parent,
      const ConstString& caption,
      string_view layout,
      uint32 canvasWidth,
      uint32 canvasHeight,
      Controls::ViewerFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::CanvasViewer>(
          Factory::CanvasViewer::Create(caption, layout, canvasWidth, canvasHeight, flags));
}
REFERENCE<CanvasViewer> Factory::CanvasViewer::Create(
      Controls::Control& parent,
      const ConstString& caption,
      string_view layout,
      uint32 canvasWidth,
      uint32 canvasHeight,
      Controls::ViewerFlags flags)
{
    return parent.AddControl<Controls::CanvasViewer>(
          Factory::CanvasViewer::Create(caption, layout, canvasWidth, canvasHeight, flags));
}
POINTER<Controls::CanvasViewer> Factory::CanvasViewer::Create(
      const ConstString& caption,
      string_view layout,
      uint32 canvasWidth,
      uint32 canvasHeight,
      Controls::ViewerFlags flags)
{
    return POINTER<Controls::CanvasViewer>(
          new Controls::CanvasViewer(caption, layout, canvasWidth, canvasHeight, flags));
}

//======[IMAGEVIEWER]================================================================================
POINTER<Controls::ImageView> Factory::ImageView::Create(string_view layout, Controls::ViewerFlags flags)
{
    return POINTER<Controls::ImageView>(new Controls::ImageView("", layout, flags));
}
REFERENCE<ImageView> Factory::ImageView::Create(
      Controls::Control* parent, string_view layout, Controls::ViewerFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::ImageView>(Factory::ImageView::Create("", layout, flags));
}
REFERENCE<ImageView> Factory::ImageView::Create(
      Controls::Control& parent, string_view layout, Controls::ViewerFlags flags)
{
    return parent.AddControl<Controls::ImageView>(Factory::ImageView::Create("", layout, flags));
}
POINTER<Controls::ImageView> Factory::ImageView::Create(
      const ConstString& caption, string_view layout, Controls::ViewerFlags flags)
{
    return POINTER<Controls::ImageView>(new Controls::ImageView(caption, layout, flags));
}
REFERENCE<ImageView> Factory::ImageView::Create(
      Controls::Control* parent, const ConstString& caption, string_view layout, Controls::ViewerFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::ImageView>(Factory::ImageView::Create(caption, layout, flags));
}
REFERENCE<ImageView> Factory::ImageView::Create(
      Controls::Control& parent, const ConstString& caption, string_view layout, Controls::ViewerFlags flags)
{
    return parent.AddControl<Controls::ImageView>(Factory::ImageView::Create(caption, layout, flags));
}

//======[LISTVIEW]===================================================================================
POINTER<Controls::ListView> Factory::ListView::Create(
      string_view layout, std::initializer_list<ColumnBuilder> columns, Controls::ListViewFlags flags)
{
    return POINTER<Controls::ListView>(new Controls::ListView(layout, columns, flags));
}
REFERENCE<ListView> Factory::ListView::Create(
      Controls::Control* parent,
      string_view layout,
      std::initializer_list<ColumnBuilder> columns,
      Controls::ListViewFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::ListView>(Factory::ListView::Create(layout, columns, flags));
}
REFERENCE<ListView> Factory::ListView::Create(
      Controls::Control& parent,
      string_view layout,
      std::initializer_list<ColumnBuilder> columns,
      Controls::ListViewFlags flags)
{
    return parent.AddControl<Controls::ListView>(Factory::ListView::Create(layout, columns, flags));
}

//======[COMBOBOX]===================================================================================
POINTER<Controls::ComboBox> Factory::ComboBox::Create(string_view layout, const ConstString& text, char itemsSeparator)
{
    return POINTER<Controls::ComboBox>(new Controls::ComboBox(layout, text, itemsSeparator));
}

REFERENCE<ComboBox> Factory::ComboBox::Create(
      Controls::Control* parent, string_view layout, const ConstString& text, char itemsSeparator)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::ComboBox>(Factory::ComboBox::Create(layout, text, itemsSeparator));
}
REFERENCE<ComboBox> Factory::ComboBox::Create(
      Controls::Control& parent, string_view layout, const ConstString& text, char itemsSeparator)
{
    return parent.AddControl<Controls::ComboBox>(Factory::ComboBox::Create(layout, text, itemsSeparator));
}

//======[NUMERICSELECTOR]============================================================================
POINTER<Controls::NumericSelector> Factory::NumericSelector::Create(
      const int64 minValue, const int64 maxValue, int64 value, string_view layout)
{
    return POINTER<Controls::NumericSelector>(new Controls::NumericSelector(minValue, maxValue, value, layout));
}
REFERENCE<NumericSelector> Factory::NumericSelector::Create(
      Controls::Control* parent, const int64 minValue, const int64 maxValue, int64 value, string_view layout)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::NumericSelector>(
          Factory::NumericSelector::Create(minValue, maxValue, value, layout));
}
REFERENCE<NumericSelector> Factory::NumericSelector::Create(
      Controls::Control& parent, const int64 minValue, const int64 maxValue, int64 value, string_view layout)
{
    return parent.AddControl<Controls::NumericSelector>(
          Factory::NumericSelector::Create(minValue, maxValue, value, layout));
}

//======[WINDOW]=====================================================================================
POINTER<Controls::Window> Factory::Window::Create(
      const ConstString& caption, string_view layout, Controls::WindowFlags flags)
{
    return POINTER<Controls::Window>(new Controls::Window(caption, layout, flags));
}

//======[DESKTOP]====================================================================================
POINTER<Controls::Desktop> Factory::Desktop::Create()
{
    return POINTER<Controls::Desktop>(new Controls::Desktop());
}

//======[TREE]=======================================================================================
POINTER<Controls::TreeView> Factory::TreeView::Create(
      string_view layout, std::initializer_list<ColumnBuilder> columns, TreeViewFlags flags)
{
    return POINTER<Controls::TreeView>(new Controls::TreeView(layout, columns, flags));
}

REFERENCE<TreeView> Factory::TreeView::Create(
      Controls::Control* parent,
      string_view layout,
      std::initializer_list<ColumnBuilder> columns,
      const TreeViewFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::TreeView>(Factory::TreeView::Create(layout, columns, flags));
}

REFERENCE<TreeView> Factory::TreeView::Create(
      Controls::Control& parent,
      string_view layout,
      std::initializer_list<ColumnBuilder> columns,
      const TreeViewFlags flags)
{
    return parent.AddControl<Controls::TreeView>(Factory::TreeView::Create(layout, columns, flags));
}

//======[GRID]=======================================================================================
Pointer<Controls::Grid> Factory::Grid::Create(
      string_view layout, uint32 columnsNo, uint32 rowsNo, Controls::GridFlags flags)
{
    return POINTER<Controls::Grid>(new Controls::Grid(layout, columnsNo, rowsNo, flags));
}

Reference<Controls::Grid> Factory::Grid::Create(
      Controls::Control* parent, string_view layout, uint32 columnsNo, uint32 rowsNo, Controls::GridFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::Grid>(Factory::Grid::Create(layout, columnsNo, rowsNo, flags));
}

Reference<Controls::Grid> Factory::Grid::Create(
      Controls::Control& parent, string_view layout, uint32 columnsNo, uint32 rowsNo, Controls::GridFlags flags)
{
    return parent.AddControl<Controls::Grid>(Factory::Grid::Create(layout, columnsNo, rowsNo, flags));
}

//======[PROPERTYLIST]===============================================================================
Pointer<Controls::PropertyList> Factory::PropertyList::Create(
      string_view layout, Reference<PropertiesInterface> object, PropertyListFlags flags)
{
    return POINTER<Controls::PropertyList>(new Controls::PropertyList(layout, object, flags));
}
Reference<Controls::PropertyList> Factory::PropertyList::Create(
      Controls::Control* parent, string_view layout, Reference<PropertiesInterface> object, PropertyListFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::PropertyList>(Factory::PropertyList::Create(layout, object, flags));
}
Reference<Controls::PropertyList> Factory::PropertyList::Create(
      Controls::Control& parent, string_view layout, Reference<PropertiesInterface> object, PropertyListFlags flags)
{
    return parent.AddControl<Controls::PropertyList>(Factory::PropertyList::Create(layout, object, flags));
}

//======[KEYSELECTOR]================================================================================
Pointer<Controls::KeySelector> Factory::KeySelector::Create(
      string_view layout, Input::Key keyCode, KeySelectorFlags flags)
{
    return POINTER<Controls::KeySelector>(new Controls::KeySelector(layout, keyCode, flags));
}
Reference<Controls::KeySelector> Factory::KeySelector::Create(
      Controls::Control* parent, string_view layout, Input::Key keyCode, KeySelectorFlags flags)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::KeySelector>(Factory::KeySelector::Create(layout, keyCode, flags));
}
Reference<Controls::KeySelector> Factory::KeySelector::Create(
      Controls::Control& parent, string_view layout, Input::Key keyCode, KeySelectorFlags flags)
{
    return parent.AddControl<Controls::KeySelector>(Factory::KeySelector::Create(layout, keyCode, flags));
}

//======[COLORPICKER]================================================================================
Pointer<Controls::ColorPicker> Factory::ColorPicker::Create(string_view layout, Graphics::Color color)
{
    return POINTER<Controls::ColorPicker>(new Controls::ColorPicker(layout, color));
}
Reference<Controls::ColorPicker> Factory::ColorPicker::Create(
      Controls::Control* parent, string_view layout, Graphics::Color color)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::ColorPicker>(Factory::ColorPicker::Create(layout, color));
}
Reference<Controls::ColorPicker> Factory::ColorPicker::Create(
      Controls::Control& parent, string_view layout, Graphics::Color color)
{
    return parent.AddControl<Controls::ColorPicker>(Factory::ColorPicker::Create(layout, color));
}

//======[CHARACTERTABLE]=============================================================================
Pointer<Controls::CharacterTable> Factory::CharacterTable::Create(string_view layout)
{
    return POINTER<Controls::CharacterTable>(new Controls::CharacterTable(layout));
}
Reference<Controls::CharacterTable> Factory::CharacterTable::Create(Controls::Control* parent, string_view layout)
{
    VALIDATE_PARENT;
    return parent->AddControl<Controls::CharacterTable>(Factory::CharacterTable::Create(layout));
}
Reference<Controls::CharacterTable> Factory::CharacterTable::Create(Controls::Control& parent, string_view layout)
{
    return parent.AddControl<Controls::CharacterTable>(Factory::CharacterTable::Create(layout));
}
} // namespace AppCUI
#undef VALIDATE_PARENT
#undef POINTER
#undef REFERENCE
