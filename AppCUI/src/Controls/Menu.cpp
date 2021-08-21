#include "AppCUI.hpp"
#include "ControlContext.hpp"

using namespace AppCUI::Graphics;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;

#define CTX ((MenuContext*) Data)
#define CHECK_VALID_ITEM(retValue)     CHECK((size_t) menuItem < CTX->Items.size(), retValue, "Invalid index: %u (should be a value between [0..%z)",(unsigned int)menuItem,CTX->Items.size());



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

    void Copy(const MenuItem& obj);
    void Swap(MenuItem& obj) noexcept;

    MenuItem(); // line
    MenuItem(MenuItemType type, const AppCUI::Utils::ConstString& text, int CommandID, AppCUI::Input::Key hotKey); // commands
    MenuItem(const AppCUI::Utils::ConstString& text, Menu* subMenu); // submenu
    MenuItem(const MenuItem& obj);
    MenuItem(MenuItem&& obj) noexcept;
    ~MenuItem();

    inline MenuItem& operator=(const MenuItem& obj) { Copy(obj); return *this; }
    inline MenuItem& operator=(MenuItem&& obj) noexcept { Swap(obj); return *this; }
};

struct MenuContext
{
    std::vector<MenuItem> Items;
    Menu* Parent;
    AppCUI::Graphics::Clip ScreenClip;


    MenuContext();
    MenuContext(unsigned int itemsCount);
    ItemHandle AddItem(MenuItem&& itm);
};

void MenuItem::Copy(const MenuItem& obj)
{
    this->Type         = obj.Type;
    this->Checked      = obj.Checked;
    this->CommandID    = obj.CommandID;
    this->Enabled      = obj.Enabled;
    this->HotKey       = obj.HotKey;
    this->HotKeyOffset = obj.HotKeyOffset;
    this->Name         = obj.Name;
    this->ShortcutKey  = obj.ShortcutKey;
    // delete the old menu
    if (this->SubMenu)
    {
        delete this->SubMenu;
        this->SubMenu = nullptr;
    }
    if (obj.SubMenu)
        this->SubMenu = new Menu(*obj.SubMenu);
}
void MenuItem::Swap(MenuItem& obj) noexcept
{
    // copy some values
    this->Type         = obj.Type;
    this->Checked      = obj.Checked;
    this->CommandID    = obj.CommandID;
    this->Enabled      = obj.Enabled;
    this->HotKey       = obj.HotKey;
    this->HotKeyOffset = obj.HotKeyOffset;
    this->ShortcutKey  = obj.ShortcutKey;

    // swap others
    this->Name.Swap(obj.Name);
    std::swap(this->SubMenu, obj.SubMenu);
}
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
    if (Name.SetWithHotKey(text, HotKeyOffset))
    {
        Type        = MenuItemType::SubMenu;
        Enabled     = true;
        Checked     = true;
        ShortcutKey = AppCUI::Input::Key::None;
        HotKey      = AppCUI::Input::Key::None;
        SubMenu     = subMenu;
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
    this->SubMenu      = nullptr;
    Copy(obj);
}
MenuItem::MenuItem(MenuItem&& obj) noexcept
{
    this->SubMenu = nullptr;
    Swap(obj);
}
MenuItem::~MenuItem()
{
    if (SubMenu)
        delete SubMenu;
    SubMenu = nullptr;
    Name.Destroy();
}

MenuContext::MenuContext()
{
    this->Parent = nullptr;
}
MenuContext::MenuContext(unsigned int itemsCount)
{
    this->Parent = nullptr;
    Items.reserve(itemsCount);
}
ItemHandle MenuContext::AddItem(MenuItem&& itm)
{
    if (itm.Type == MenuItemType::Invalid)
        return InvalidItemHandle;
    CHECK(Items.size() < 0xFFFF, false, "A maximum of 0xFFFF items can be added to a Menu");
    try
    {
        auto res = ItemHandle{ (unsigned int) Items.size() };
        Items.push_back(itm);
        return res;
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
Menu::Menu(const Menu& obj)
{
    this->Data = new MenuContext();
    if (obj.Data)
    {
        CTX->Items = ((MenuContext*) (obj.Data))->Items;
    }    
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
ItemHandle Menu::AddSubMenu(const AppCUI::Utils::ConstString& text)
{
    try
    {
        Menu* SubMenu                            = new Menu(16); // 16 reserved items by default
        ((MenuContext*) (SubMenu->Data))->Parent = this;
        return CTX->AddItem(MenuItem(text, SubMenu));
    }
    catch (...)
    {
        return InvalidItemHandle; // could not allocate 
    }    
}
Menu* Menu::GetParentMenu()
{
    if (!Data)
        return nullptr;
    return CTX->Parent;
}
bool Menu::SetEnable(ItemHandle menuItem, bool status)
{
    CHECK_VALID_ITEM(false);
    CTX->Items[(unsigned int) menuItem].Enabled = status;
    return true;
}
bool Menu::SetChecked(ItemHandle menuItem, bool status)
{
    CHECK_VALID_ITEM(false);
    CTX->Items[(unsigned int) menuItem].Checked = status;
    return true;
}

Menu* Menu::GetSubMenu(ItemHandle menuItem)
{
    CHECK_VALID_ITEM(nullptr);
    return CTX->Items[(unsigned int) menuItem].SubMenu;
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