#include "AppCUI.hpp"
#include "ControlContext.hpp"

using namespace AppCUI::Graphics;
using namespace AppCUI::Controls;

enum class MenuItemType: unsigned int
{
    Command,
    Check,
    Radio,
    Line,
    SubMenu
};
struct MenuItem
{
    CharacterBuffer Name;
    unsigned int HotKeyOffset;
    AppCUI::Input::Key HotKey;
    MenuItemType Type;
    int CommandID;
    bool Enabled;
    bool Checked;
    Menu* SubMenu;

    MenuItem();
    MenuItem(MenuItemType type, const AppCUI::Utils::ConstString& text, int CommandID, AppCUI::Input::Key hotKey);
    MenuItem(const AppCUI::Utils::ConstString& text, Menu* subMenu);
    MenuItem(const MenuItem& obj);
    MenuItem(MenuItem&& obj);
};



Menu::Menu()
{
}
Menu::Menu(unsigned int itemsCount)
{
}

ItemHandle Menu::AddCommandItem(const AppCUI::Utils::ConstString & text, int CommandID, AppCUI::Input::Key hotKey)
{
    NOT_IMPLEMENTED(InvalidItemHandle);
}
ItemHandle Menu::AddCheckItem(const AppCUI::Utils::ConstString & text, int CommandID, AppCUI::Input::Key hotKey)
{
    NOT_IMPLEMENTED(InvalidItemHandle);
}
ItemHandle Menu::AddRadioItem(const AppCUI::Utils::ConstString & text, int CommandID, AppCUI::Input::Key hotKey)
{
    NOT_IMPLEMENTED(InvalidItemHandle);
}
ItemHandle Menu::AddSeparator()
{
    NOT_IMPLEMENTED(InvalidItemHandle);
}
ItemHandle Menu::AddSubMenu(const AppCUI::Utils::ConstString& text, Menu* subMenu)
{
    NOT_IMPLEMENTED(InvalidItemHandle);
}

bool Menu::SetEnable(ItemHandle menuItem, bool status)
{
    NOT_IMPLEMENTED(false);
}
bool Menu::SetChecked(ItemHandle menuItem, bool status)
{
    NOT_IMPLEMENTED(false);
}

void Menu::Show(int x, int y)
{
}

void Menu::Paint(AppCUI::Graphics::Renderer& renderer)
{
}