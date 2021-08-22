#include "Internal.hpp"
#include "ControlContext.hpp"

using namespace AppCUI::Graphics;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;

#define CTX ((MenuContext*) this->Context)
#define CHECK_VALID_ITEM(retValue)     CHECK((size_t) menuItem < CTX->Items.size(), retValue, "Invalid index: %u (should be a value between [0..%z)",(unsigned int)menuItem,CTX->Items.size());


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
    this->Parent            = nullptr;
    this->Cfg               = Application::GetAppConfig();
    this->FirstVisibleItem  = 0;
    this->VisibleItemsCount = 0;
    this->Width             = 0;
}
MenuContext::MenuContext(unsigned int itemsCount)
{
    this->Parent            = nullptr;
    this->Cfg               = Application::GetAppConfig();
    this->FirstVisibleItem  = 0;
    this->VisibleItemsCount = 0;
    this->Width             = 0;
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
void MenuContext::Paint(AppCUI::Graphics::Renderer& renderer, bool activ)
{
    auto &col = this->Cfg->Menu.Activ;
    if (!activ)
        col = this->Cfg->Menu.Parent;

    auto& itemCol = col.Normal;
    WriteTextParams textParams(WriteTextFlags::SingleLine | WriteTextFlags::HighlightHotKey, TextAlignament::Left);

    renderer.Clear(' ', col.Background);
    renderer.DrawRectSize(0, 0, ScreenClip.ClipRect.Width, ScreenClip.ClipRect.Height, col.Background, false);
    for (unsigned int tr=1;tr<=this->VisibleItemsCount;tr++)
    {
        MenuItem& item            = this->Items[tr - 1];
        textParams.Color          = itemCol.Text;
        textParams.HotKeyColor    = itemCol.HotKey;
        textParams.HotKeyPosition = item.HotKeyOffset;
        textParams.Y              = tr;
        switch (item.Type)
        {
        case MenuItemType::Line:
            renderer.DrawHorizontalLineWithSpecialChar(1, tr, this->Width, SpecialChars::BoxHorizontalSingleLine, col.Background);
            break;
        case MenuItemType::Command:
            textParams.X = 1;
            renderer.WriteText(item.Name, textParams);
            break;
        case MenuItemType::Check:
            textParams.X = 3;
            renderer.WriteText(item.Name, textParams);
            break;    
        case MenuItemType::Radio:
            textParams.X = 3;
            renderer.WriteText(item.Name, textParams);
            break; 
        case MenuItemType::SubMenu:
            textParams.X = 1;
            renderer.WriteText(item.Name, textParams);
            renderer.WriteSpecialCharacter(this->Width, tr, SpecialChars::TriangleRight, itemCol.Text);
            break; 
        }        
    }
}

void MenuContext::OnMouseMove(int x, int y)
{
}
bool MenuContext::OnMousePressed(int x, int y, AppCUI::Input::MouseButton button)
{
    NOT_IMPLEMENTED(false);
}
void MenuContext::OnMouseWheel(int x, int y, AppCUI::Input::MouseWheel direction)
{
}

// key events
bool MenuContext::OnKeyEvent(AppCUI::Input::Key keyCode, char AsciiCode)
{
    NOT_IMPLEMENTED(false);
}

void MenuContext::Show(AppCUI::Controls::Menu* me, AppCUI::Controls::Control* relativeControl, int x, int y)
{
    // compute abosolute position
    while (relativeControl)
    {
        x += relativeControl->GetX() + ((ControlContext*) relativeControl->Context)->Margins.Left;
        y += relativeControl->GetY() + ((ControlContext*) relativeControl->Context)->Margins.Top;
        relativeControl = relativeControl->GetParent();
    }
    // compute Width & Visible Items Count
    Width = 20;
    VisibleItemsCount = this->Items.size();
    // Set the clip
    this->ScreenClip.Set(x, y, Width+2, VisibleItemsCount+2);
    // link to application
    AppCUI::Application::SetContextualMenu(me);
}

//=====================================================================================[Menu]====
Menu::Menu()
{
    this->Context = new MenuContext();
}
Menu::Menu(unsigned int itemsCount)
{
    this->Context = new MenuContext(itemsCount);
}
Menu::Menu(const Menu& obj)
{
    this->Context = new MenuContext();
    if (obj.Context)
    {
        CTX->Items = ((MenuContext*) (obj.Context))->Items;
    }    
}
Menu::~Menu()
{
    if (this->Context)
        delete ((MenuContext*) Context);
    this->Context = nullptr;
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
        ((MenuContext*) (SubMenu->Context))->Parent = this;
        return CTX->AddItem(MenuItem(text, SubMenu));
    }
    catch (...)
    {
        return InvalidItemHandle; // could not allocate 
    }    
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
    CTX->Show(this, nullptr, x, y);
}
void Menu::Show(Control* parent, int relativeX, int relativeY)
{
    CTX->Show(this, parent, relativeX, relativeY);
}
#undef CTX