#include "Internal.hpp"

using namespace AppCUI::Internal;
using namespace AppCUI::Controls;

#define NO_ITEM_SELECTED    0xFFFFFFFF

MenuBarItem::MenuBarItem()
{
    this->HotKey       = AppCUI::Input::Key::None;
    this->HotKeyOffset = AppCUI::Graphics::CharacterBuffer::INVALID_HOTKEY_OFFSET;
}
MenuBar::MenuBar()
{
    this->ItemsCount  = 0;
    this->CurrentItem = NO_ITEM_SELECTED;
}
Menu* MenuBar::GetMenu(ItemHandle itemHandle)
{
    CHECK((unsigned int) itemHandle < this->ItemsCount,
          nullptr,
          "Invalid item handle (%08X)",
          (unsigned int) itemHandle);
    return &Items[(unsigned int) itemHandle]->Mnu;
}
ItemHandle MenuBar::AddMenu(const AppCUI::Utils::ConstString& name)
{
    CHECK(this->ItemsCount < MenuBar::MAX_ITEMS,
          ItemHandle{ NO_ITEM_SELECTED },
          "Too many menu items - max allowed is: %d",
          MenuBar::MAX_ITEMS);
    NOT_IMPLEMENTED(ItemHandle{ NO_ITEM_SELECTED });
}

#undef NO_ITEM_SELECTED