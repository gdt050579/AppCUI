#include "AppCUI.hpp"
#include "ControlContext.hpp"

using namespace AppCUI::Graphics;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;

#define CTX ((MenuContext*) Data)


enum class MenuItemType: unsigned int
{
    Invalid,
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
    AppCUI::Input::Key ShortcutKey;
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

struct MenuContext
{
    std::vector<MenuItem> Items;

    MenuContext();
    MenuContext(unsigned int itemsCount);
    ItemHandle AddItem(MenuItem&& itm);
};


MenuItem::MenuItem()
{
    Type              = MenuItemType::Line;
    Enabled           = true;
    Checked           = true;
    SubMenu           = nullptr;
    CommandID         = -1;
    HotKey            = AppCUI::Input::Key::None;
    ShortcutKey       = AppCUI::Input::Key::None;
    HotKeyOffset      = CharacterBuffer::INVALID_HOTKEY_OFFSET;
}
MenuItem::MenuItem(MenuItemType type, const AppCUI::Utils::ConstString& text, int cmdID, AppCUI::Input::Key shortcutKey)
{
    Type = MenuItemType::Invalid;
    if (Name.SetWithHotKey(text, HotKeyOffset))
    {
        Type        = type;
        Enabled     = true;
        Checked     = true;
        SubMenu     = nullptr;
        CommandID   = cmdID;
        ShortcutKey = shortcutKey;
        HotKey      = AppCUI::Input::Key::None;
        if (HotKeyOffset != CharacterBuffer::INVALID_HOTKEY_OFFSET)
        {
            char16_t ch = Name.GetBuffer()[HotKeyOffset].Code;
            if ((ch >= 'A') && (ch <= 'Z'))
                HotKey = static_cast<Key>((unsigned int) Key::A + (ch - 'A'));
            else if ((ch >= 'a') && (ch <= 'z'))
                HotKey = static_cast<Key>((unsigned int) Key::A + (ch - 'a'));
            else if ((ch >= '0') && (ch <= '9'))
                HotKey = static_cast<Key>((unsigned int) Key::N0 + (ch - '0'));
            else
                HotKeyOffset = CharacterBuffer::INVALID_HOTKEY_OFFSET; // invalid hot key
        }
    }
}
MenuItem::MenuItem(const AppCUI::Utils::ConstString& text, Menu* subMenu)
{
    Type = MenuItemType::Invalid;
    if (subMenu == nullptr)
    {
        LOG_ERROR("Expecting a non-null submenu parameter");        
        return;
    }
    if (Name.SetWithHotKey(text, HotKeyOffset))
    {
        Type        = MenuItemType::SubMenu;
        Enabled     = true;
        Checked     = true;
        SubMenu     = nullptr;
        ShortcutKey = AppCUI::Input::Key::None;
        HotKey      = AppCUI::Input::Key::None;
        if (HotKeyOffset != CharacterBuffer::INVALID_HOTKEY_OFFSET)
        {
            char16_t ch = Name.GetBuffer()[HotKeyOffset].Code;
            if ((ch >= 'A') && (ch <= 'Z'))
                HotKey = static_cast<Key>((unsigned int) Key::A + (ch - 'A'));
            else if ((ch >= 'a') && (ch <= 'z'))
                HotKey = static_cast<Key>((unsigned int) Key::A + (ch - 'a'));
            else if ((ch >= '0') && (ch <= '9'))
                HotKey = static_cast<Key>((unsigned int) Key::N0 + (ch - '0'));
            else
                HotKeyOffset = CharacterBuffer::INVALID_HOTKEY_OFFSET; // invalid hot key
        }
    }
}
MenuItem::MenuItem(const MenuItem& obj)
{
    this->Checked      = obj.Checked;
    this->CommandID    = obj.CommandID;
    this->Enabled      = obj.Enabled;
    this->HotKey       = obj.HotKey;
    this->HotKeyOffset = obj.HotKeyOffset;
    this->Name         = obj.Name;
    this->ShortcutKey  = obj.ShortcutKey;
    this->SubMenu      = nullptr;
    if (obj.SubMenu)
    {
        // make a copy of the submenu
        // GDT: need to rearchiteturize this part !
    }
}
MenuItem::MenuItem(MenuItem&& obj)
{
    // copy some values
    this->Checked      = obj.Checked;
    this->CommandID    = obj.CommandID;
    this->Enabled      = obj.Enabled;
    this->HotKey       = obj.HotKey;
    this->HotKeyOffset = obj.HotKeyOffset;
    this->ShortcutKey  = obj.ShortcutKey;
    this->SubMenu      = obj.SubMenu;
    obj.SubMenu        = nullptr; 
    // swap other
    this->Name.Swap(obj.Name);
}

MenuContext::MenuContext()
{
}
MenuContext::MenuContext(unsigned int itemsCount)
{
    Items.reserve(itemsCount);
}
ItemHandle MenuContext::AddItem(MenuItem&& itm)
{
    if (itm.Type == MenuItemType::Invalid)
        return InvalidItemHandle;
    CHECK(Items.size() < 0xFFFF, false, "A maximum of 0xFFFF items can be added to a Menu");
    try
    {
        Items.push_back(itm);
        return ItemHandle{ (unsigned int)Items.size() };
    }
    catch (...)
    {
        RETURNERROR(InvalidItemHandle , "Exception thrown while adding menu item to std::vector");
    }
}

Menu::Menu()
{
    this->Data = new MenuContext();
}
Menu::Menu(unsigned int itemsCount)
{
    this->Data = new MenuContext(itemsCount);
}
Menu::~Menu()
{
    if (this->Data)
        delete ((MenuContext*) Data);
    this->Data = nullptr;
}

ItemHandle Menu::AddCommandItem(const AppCUI::Utils::ConstString& text, int CommandID, AppCUI::Input::Key shortcutKey)
{
    return CTX->AddItem(MenuItem(MenuItemType::Command, text, CommandID, shortcutKey));
}
ItemHandle Menu::AddCheckItem(const AppCUI::Utils::ConstString& text, int CommandID, AppCUI::Input::Key shortcutKey)
{
    return CTX->AddItem(MenuItem(MenuItemType::Check, text, CommandID, shortcutKey));
}
ItemHandle Menu::AddRadioItem(const AppCUI::Utils::ConstString& text, int CommandID, AppCUI::Input::Key shortcutKey)
{
    return CTX->AddItem(MenuItem(MenuItemType::Radio, text, CommandID, shortcutKey));
}
ItemHandle Menu::AddSeparator()
{
    return CTX->AddItem(MenuItem());
}
ItemHandle Menu::AddSubMenu(const AppCUI::Utils::ConstString& text, Menu* subMenu)
{
    return CTX->AddItem(MenuItem(text, subMenu));
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

void Menu::OnMouseMove(int x, int y)
{
}
bool Menu::OnMousePressed(int x, int y, AppCUI::Input::MouseButton button)
{
    NOT_IMPLEMENTED(false);
}
void Menu::OnMouseWheel(int x, int y, AppCUI::Input::MouseWheel direction)
{
}

// key events
bool Menu::OnKeyEvent(AppCUI::Input::Key keyCode, char AsciiCode)
{
    NOT_IMPLEMENTED(false);
}

#undef CTX