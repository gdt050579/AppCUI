#include "Internal.hpp"

using namespace AppCUI::Internal;
using namespace AppCUI::Controls;
using namespace AppCUI::Utils;
using namespace AppCUI::Input;

TextControlDefaultMenu::TextControlDefaultMenu()
{
    this->itemCopy  = menu.AddCommandItem("&Copy", TEXTCONTROL_CMD_COPY, Key::Ctrl | Key::C);
    this->itemCut   = menu.AddCommandItem("Cut", TEXTCONTROL_CMD_CUT, Key::Ctrl | Key::X);
    this->itemPaste = menu.AddCommandItem("Paste", TEXTCONTROL_CMD_PASTE, Key::Ctrl | Key::V);
    menu.AddSeparator();
    this->itemSelectAll = menu.AddCommandItem("Select &All", TEXTCONTROL_CMD_SELECT_ALL, Key::Ctrl | Key::A);
    this->itemDelete    = menu.AddCommandItem("&Delete selection", TEXTCONTROL_CMD_DELETE_SELECTED, Key::Delete);
}
TextControlDefaultMenu::~TextControlDefaultMenu()
{
}
void TextControlDefaultMenu::Show(Reference<Control> parent, int x, int y, bool hasSelection)
{
    menu.SetEnable(this->itemCopy, hasSelection);
    menu.SetEnable(this->itemCut, hasSelection);
    menu.SetEnable(this->itemDelete, hasSelection);
    menu.Show(parent, x, y);
}