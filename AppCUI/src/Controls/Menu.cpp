#include "Internal.hpp"
#include "ControlContext.hpp"

using namespace AppCUI::Graphics;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;

#define CTX ((MenuContext*) this->Context)
#define CHECK_VALID_ITEM(retValue)     CHECK(menuItem < CTX->ItemsCount, retValue, "Invalid index: %u (should be a value between [0..%u)",(unsigned int)menuItem,CTX->ItemsCount);
#define NO_MENUITEM_SELECTED    0xFFFFFFFF


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
        Checked     = false;
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
    this->CurrentItem       = NO_MENUITEM_SELECTED;
    this->Width             = 0;
    this->ItemsCount        = 0;
}
ItemHandle MenuContext::AddItem(MenuItem* itm)
{
    if (itm->Type == MenuItemType::Invalid)
        return InvalidItemHandle;
    CHECK(this->ItemsCount < MAX_NUMBER_OF_MENU_ITEMS,
          InvalidItemHandle,
          "A maximum of 256 items can be added to a Menu");

    auto res = ItemHandle{ (unsigned int) this->ItemsCount };
    Items[this->ItemsCount].reset(itm);
    return res;
}
void MenuContext::Paint(AppCUI::Graphics::Renderer& renderer, bool activ)
{
    auto* col = &this->Cfg->Menu.Activ;
    if (!activ)
        col = &this->Cfg->Menu.Parent;

    auto* itemCol = &col->Normal;
    WriteTextParams textParams(
          WriteTextFlags::SingleLine | WriteTextFlags::OverwriteColors | WriteTextFlags::HighlightHotKey,
          TextAlignament::Left);

    renderer.Clear(' ', col->Background);
    renderer.DrawRectSize(0, 0, ScreenClip.ClipRect.Width, ScreenClip.ClipRect.Height, col->Background, false);
    for (unsigned int tr=1;tr<=this->VisibleItemsCount;tr++)
    {
        MenuItem* item = this->Items[tr - 1].get();
        if (item->Enabled == false)
            itemCol = &col->Inactive;
        else
        {
            if (tr - 1 == this->CurrentItem)
            {
                itemCol = &col->Selected;
                renderer.DrawHorizontalLine(1, tr, Width, ' ', col->Selected.Text);
            }
            else
                itemCol = &col->Normal;
        }
        
        textParams.Color          = itemCol->Text;
        textParams.HotKeyColor    = itemCol->HotKey;
        textParams.HotKeyPosition = item->HotKeyOffset;
        textParams.Y              = tr;
              
        switch (item->Type)
        {
        case MenuItemType::Line:
            renderer.DrawHorizontalLineWithSpecialChar(1, tr, this->Width, SpecialChars::BoxHorizontalSingleLine, col->Background);
            break;
        case MenuItemType::Command:
            textParams.X = 1;
            renderer.WriteText(item->Name, textParams);
            break;
        case MenuItemType::Check:
            textParams.X = 3;
            renderer.WriteText(item->Name, textParams);
            if (item->Checked)
                renderer.WriteSpecialCharacter(1, tr, SpecialChars::CheckMark, itemCol->Check);
            break;    
        case MenuItemType::Radio:
            textParams.X = 3;
            renderer.WriteText(item->Name, textParams);
            if (item->Checked)
                renderer.WriteSpecialCharacter(1, tr, SpecialChars::CircleFilled, itemCol->Check);
            else
                renderer.WriteSpecialCharacter(1, tr, SpecialChars::CircleEmpty, itemCol->Uncheck);
            break; 
        case MenuItemType::SubMenu:
            textParams.X = 1;
            renderer.WriteText(item->Name, textParams);
            renderer.WriteSpecialCharacter(this->Width, tr, SpecialChars::TriangleRight, itemCol->Text);
            break; 
        }     
        if (item->ShortcutKey != Key::None)
        {
            auto k_n = KeyUtils::GetKeyName(item->ShortcutKey);
            auto m_n = KeyUtils::GetKeyModifierName(item->ShortcutKey);
            renderer.WriteSingleLineText(this->Width - (unsigned int)k_n.size(), tr, k_n, itemCol->ShortCut);
            renderer.WriteSingleLineText(this->Width - (unsigned int)(k_n.size()+m_n.size()), tr, m_n, itemCol->ShortCut);
        }


    }
}
bool MenuContext::SetChecked(unsigned int menuIndex, bool status)
{
    CHECK(menuIndex < ItemsCount, false, "Invalid menu index (%u) , should be between 0 and less than %u",menuIndex, ItemsCount);
    auto i = this->Items[menuIndex].get();
    CHECK((i->Type == MenuItemType::Check) || (i->Type == MenuItemType::Radio),
          false,
          "Only Check and Radio item can change their state");
    if (i->Type == MenuItemType::Radio)
    {
        // radio menu item -> uncheck all items that are radioboxes
        unsigned int index = menuIndex;
        while (((index >= 0) && (index < this->ItemsCount)) && (this->Items[index]->Type == MenuItemType::Radio))
        {
            this->Items[index]->Checked = false;
            index--;
        }
        index = menuIndex + 1;
        while ((index < this->ItemsCount) && (this->Items[index]->Type == MenuItemType::Radio))
        {
            this->Items[index]->Checked = false;
            index++;
        }
    }
    i->Checked = status;
    return true;
}
void MenuContext::ComputeMousePositionInfo(int x, int y, MenuMousePositionInfo& mpi)
{
    if ((x >= 1) && (y >= 1) && (x <= Width) && (y <= VisibleItemsCount))
    {
        mpi.ItemIndex = (y - 1) + FirstVisibleItem;
        if ((mpi.ItemIndex < ItemsCount) && 
            (Items[mpi.ItemIndex]->Enabled) &&
            (Items[mpi.ItemIndex]->Type != MenuItemType::Line))
        {
            // all good - current item is valid
        }
        else
        {
            mpi.ItemIndex = NO_MENUITEM_SELECTED;
        }
    }
    else
    {
        mpi.ItemIndex = NO_MENUITEM_SELECTED;
    }
    mpi.IsOnMenu       = (x >= 0) && (y >= 0) && (x < this->Width + 2) && (y < this->VisibleItemsCount + 2);
    mpi.IsOnUpButton   = false;
    mpi.IsOnDownButton = false;
}
bool MenuContext::OnMouseMove(int x, int y)
{
    MenuMousePositionInfo mpi;
    ComputeMousePositionInfo(x, y, mpi);
    if (CurrentItem != mpi.ItemIndex)
    {
        CurrentItem = mpi.ItemIndex;
        return true;
    }
    return false;
}
MousePressedResult MenuContext::OnMousePressed(int x, int y)
{
    MenuMousePositionInfo mpi;
    ComputeMousePositionInfo(x, y, mpi);
    // if click on a valid item, apply the action and close the menu
    if (mpi.ItemIndex != NO_MENUITEM_SELECTED)
    {
        auto itm = this->Items[mpi.ItemIndex].get();
        int commandID = -1;
        switch (itm->Type)
        {
        case MenuItemType::Check:
            this->SetChecked(mpi.ItemIndex, !itm->Checked);
            commandID = itm->CommandID;
            break;
        case MenuItemType::Radio:
            this->SetChecked(mpi.ItemIndex, true);
            commandID = itm->CommandID;
            break;
        case MenuItemType::SubMenu:
            itm->SubMenu->Show(Width + ScreenClip.ScreenPosition.X, y + ScreenClip.ScreenPosition.Y);
            return MousePressedResult::Repaint;
        case MenuItemType::Command:
            commandID = itm->CommandID;
            break;
        }
        if (commandID >= 0)
        {
            Application::GetApplication()->CloseContextualMenu();
            Application::GetApplication()->SendCommand(commandID);
            return MousePressedResult::Repaint;
        }
        // other type of items
        return MousePressedResult::None;
    }
    // is it's on the menu -> do nothing
    if (mpi.IsOnMenu)
        return MousePressedResult::None;
    // if it's outsize, check if mouse is on one of its parens
    return MousePressedResult::CheckParent;
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
    // compute best width
    Width             = 0;
    for (size_t tr = 0; tr < this->ItemsCount;tr++)
    {
        auto i               = this->Items[tr].get();
        unsigned int w_left = i->Name.Len();
        unsigned int w_right = 0;
        if ((i->Type == MenuItemType::Radio) || (i->Type == MenuItemType::Check))
            w_left += 2;
        if (i->ShortcutKey != Key::None)
        {
            w_right += KeyUtils::GetKeyName(i->ShortcutKey).size();
            w_right += KeyUtils::GetKeyModifierName(i->ShortcutKey).size();     
            if (w_right > 0)
                w_right += 4;
        }

        Width = MAXVALUE(Width, w_left + w_right);

    }
    VisibleItemsCount = this->ItemsCount;
    // Set the clip
    this->ScreenClip.Set(x, y, Width+2, VisibleItemsCount+2);
    // link to application
    auto* app = AppCUI::Application::GetApplication();
    if (app)
        app->ShowContextualMenu(me);
}

//=====================================================================================[Menu]====
Menu::Menu()
{
    this->Context = new MenuContext();
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
        Menu* SubMenu                               = new Menu(); 
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
    CTX->Items[(unsigned int) menuItem]->Enabled = status;
    return true;
}
bool Menu::SetChecked(ItemHandle menuItem, bool status)
{
    CHECK_VALID_ITEM(false);
    return CTX->SetChecked((unsigned int) menuItem, status);
}

Menu* Menu::GetSubMenu(ItemHandle menuItem)
{
    CHECK_VALID_ITEM(nullptr);
    return CTX->Items[(unsigned int) menuItem]->SubMenu;
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