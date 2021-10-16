#include "Internal.hpp"
#include "ControlContext.hpp"

using namespace AppCUI::Graphics;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;

#define CTX ((MenuContext*) this->Context)
#define CHECK_VALID_ITEM(retValue)     CHECK(menuItem < CTX->ItemsCount, retValue, "Invalid index: %u (should be a value between [0..%u)",(unsigned int)menuItem,CTX->ItemsCount);
#define NO_MENUITEM_SELECTED    0xFFFFFFFF



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
MenuItem::MenuItem(MenuItemType type, const AppCUI::Utils::ConstString& text, int cmdID, bool checked, AppCUI::Input::Key shortcutKey)
{
    Type = MenuItemType::Invalid;
    if (Name.SetWithHotKey(text, HotKeyOffset, HotKey))
    {
        Type        = type;
        Enabled     = true;
        Checked     = checked;
        SubMenu     = nullptr;
        CommandID   = cmdID;
        ShortcutKey = shortcutKey;
    }
}
MenuItem::MenuItem(const AppCUI::Utils::ConstString& text, Menu* subMenu)
{
    Type = MenuItemType::Invalid;
    if (Name.SetWithHotKey(text, HotKeyOffset, HotKey))
    {
        Type        = MenuItemType::SubMenu;
        Enabled     = true;
        Checked     = false;
        ShortcutKey = AppCUI::Input::Key::None;
        SubMenu     = subMenu;
    }
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
    this->Owner             = nullptr;
    this->Cfg               = Application::GetAppConfig();
    this->FirstVisibleItem  = 0;
    this->VisibleItemsCount = 0;
    this->CurrentItem       = NO_MENUITEM_SELECTED;
    this->Width             = 0;
    this->TextWidth         = 0;
    this->ItemsCount        = 0;
    this->ButtonUp          = MenuButtonState::Normal;
    this->ButtonDown        = MenuButtonState::Normal;
}
ItemHandle MenuContext::AddItem(std::unique_ptr<MenuItem> itm)
{
    if (itm->Type == MenuItemType::Invalid)
        return InvalidItemHandle;
    CHECK(this->ItemsCount < MAX_NUMBER_OF_MENU_ITEMS,InvalidItemHandle, "A maximum of 256 items can be added to a Menu");

    auto res = ItemHandle{ (unsigned int) this->ItemsCount };
    Items[this->ItemsCount] = std::move(itm);
    this->ItemsCount++;
    return res;
}
void MenuContext::Paint(AppCUI::Graphics::Renderer& renderer, bool activ)
{
    auto* col = &this->Cfg->Menu.Activ;
    if (!activ)
        col = &this->Cfg->Menu.Parent;

    auto* itemCol = &col->Normal;
    WriteTextParams textParams(
          WriteTextFlags::SingleLine | WriteTextFlags::OverwriteColors | WriteTextFlags::HighlightHotKey |
          WriteTextFlags::ClipToWidth | WriteTextFlags::FitTextToWidth,
          TextAlignament::Left);
    textParams.Width = this->TextWidth;

    renderer.Clear(' ', col->Background);
    renderer.DrawRectSize(0, 0, ScreenClip.ClipRect.Width, ScreenClip.ClipRect.Height, col->Background, false);
    // draw scroll buttons if case
    if (this->VisibleItemsCount < this->ItemsCount)
    {
        ColorPair c;
        // top button
        if (this->FirstVisibleItem == 0)
            c = col->Button.Inactive;
        else
        {
            switch (this->ButtonUp)
            {
                case MenuButtonState::Normal: c = col->Button.Normal; break;
                case MenuButtonState::Hovered: c = col->Button.Hover; break;
                case MenuButtonState::Pressed: c = col->Button.Pressed; break;
            }
        }
            
        renderer.WriteSpecialCharacter(1 + this->Width / 2, 0, SpecialChars::TriangleUp, c);

        // bottom button
        if (this->FirstVisibleItem+this->VisibleItemsCount>=this->ItemsCount)
            c = col->Button.Inactive;
        else
        {
            switch (this->ButtonDown)
            {
                case MenuButtonState::Normal: c = col->Button.Normal; break;
                case MenuButtonState::Hovered: c = col->Button.Hover; break;
                case MenuButtonState::Pressed: c = col->Button.Pressed; break;
            }
        }

        renderer.WriteSpecialCharacter(1 + this->Width / 2, ScreenClip.ClipRect.Height-1, SpecialChars::TriangleDown, c);
    }

    // draw items
    for (unsigned int tr=1;tr<=this->VisibleItemsCount;tr++)
    {
        unsigned int actualIndex = this->FirstVisibleItem + tr - 1;
        if (actualIndex >= ItemsCount)
            break;
        MenuItem* item           = this->Items[actualIndex].get();
        if (item->Enabled == false)
            itemCol = &col->Inactive;
        else
        {
            if (actualIndex == this->CurrentItem)
            {
                itemCol = &col->Selected;
                renderer.FillHorizontalLine(1, tr, Width, ' ', col->Selected.Text);
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
            renderer.DrawHorizontalLine(1, tr, this->Width, col->Background);
            break;
        case MenuItemType::Command:
            textParams.X = 2;
            renderer.WriteText(item->Name, textParams);
            break;
        case MenuItemType::Check:
            textParams.X = 4;
            renderer.WriteText(item->Name, textParams);
            if (item->Checked)
                renderer.WriteSpecialCharacter(2, tr, SpecialChars::CheckMark, itemCol->Check);
            break;    
        case MenuItemType::Radio:
            textParams.X = 4;
            renderer.WriteText(item->Name, textParams);
            if (item->Checked)
                renderer.WriteSpecialCharacter(2, tr, SpecialChars::CircleFilled, itemCol->Check);
            else
                renderer.WriteSpecialCharacter(2, tr, SpecialChars::CircleEmpty, itemCol->Uncheck);
            break; 
        case MenuItemType::SubMenu:
            textParams.X = 2;
            renderer.WriteText(item->Name, textParams);
            renderer.WriteSpecialCharacter(this->Width - 1, tr, SpecialChars::TriangleRight, itemCol->Text);
            break; 
        }     
        if (item->ShortcutKey != Key::None)
        {
            auto k_n = KeyUtils::GetKeyName(item->ShortcutKey);
            auto m_n = KeyUtils::GetKeyModifierName(item->ShortcutKey);
            renderer.WriteSingleLineText(this->Width - (unsigned int) k_n.size(), tr, k_n, itemCol->ShortCut);
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
        while (((index < this->ItemsCount)) && (this->Items[index]->Type == MenuItemType::Radio))
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
    if ((x >= 1) && (y >= 1) && (x <= (int)Width) && (y <= (int)VisibleItemsCount))
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
    mpi.IsOnMenu       = (x >= 0) && (y >= 0) && (x < (int)this->Width + 2) && (y < (int)this->VisibleItemsCount + 2);
    mpi.IsOnUpButton   = (y == 0) && (static_cast<unsigned>(x) == (1 + this->Width / 2));
    mpi.IsOnDownButton = (y == ScreenClip.ClipRect.Height - 1) && (static_cast<unsigned>(x) == (1 + this->Width / 2));
}
bool MenuContext::OnMouseMove(int x, int y, bool& repaint)
{
    MenuMousePositionInfo mpi;
    ComputeMousePositionInfo(x, y, mpi);
    auto buttonUpStatus   = mpi.IsOnUpButton ? MenuButtonState::Hovered : MenuButtonState::Normal;
    auto buttonDownStatus = mpi.IsOnDownButton ? MenuButtonState::Hovered : MenuButtonState::Normal;
    auto processed        = mpi.IsOnMenu;
    if (buttonUpStatus != this->ButtonUp)
    {
        this->ButtonUp = buttonUpStatus;
        repaint        = true;
    }
    if (buttonDownStatus != this->ButtonDown)
    {
        this->ButtonDown = buttonDownStatus;
        repaint          = true;
    }
    if (CurrentItem != mpi.ItemIndex)
    {
        CurrentItem = mpi.ItemIndex;
        repaint     = true;
    }
    return processed;
}
MousePressedResult MenuContext::OnMousePressed(int x, int y)
{
    MenuMousePositionInfo mpi;
    ComputeMousePositionInfo(x, y, mpi);
    // check buttons
    if (this->VisibleItemsCount < this->ItemsCount)
    {
        if ((mpi.IsOnUpButton) && (this->FirstVisibleItem>0))
        {
            this->ButtonUp = MenuButtonState::Pressed;
            OnMouseWheel(x, y, MouseWheel::Up);
            return MousePressedResult::Repaint;
        }
        if ((mpi.IsOnDownButton) && (this->FirstVisibleItem+this->VisibleItemsCount < this->ItemsCount))
        {
            this->ButtonDown = MenuButtonState::Pressed;
            OnMouseWheel(x, y, MouseWheel::Down);
            return MousePressedResult::Repaint;
        }
    }
    // if click on a valid item, apply the action and close the menu
    if (mpi.ItemIndex != NO_MENUITEM_SELECTED)
    {
        RunItemAction(mpi.ItemIndex);
        // other type of items
        return MousePressedResult::Repaint;
    }
    // is it's on the menu -> do nothing
    if (mpi.IsOnMenu)
        return MousePressedResult::None;
    // if it's outsize, check if mouse is on one of its parens
    return MousePressedResult::CheckParent;
}
bool MenuContext::IsOnMenu(int x, int y)
{
    MenuMousePositionInfo mpi;
    ComputeMousePositionInfo(x, y, mpi);
    return mpi.IsOnMenu;
}
bool MenuContext::OnMouseWheel(int, int, AppCUI::Input::MouseWheel direction)
{
    if (this->VisibleItemsCount >= this->ItemsCount)
        return false; // nothing to scroll
    if ((direction == MouseWheel::Up) && (this->FirstVisibleItem>0))
    {
        this->FirstVisibleItem--;
        return true;
    }
    if ((direction == MouseWheel::Down) && ((this->FirstVisibleItem + this->VisibleItemsCount) < this->ItemsCount))
    {
        this->FirstVisibleItem++;
        return true;
    }
    return false;
}
void MenuContext::CreateAvailableItemsList(unsigned int* indexes, unsigned int& count)
{
    // assume indexes is valid and has a size of MAX_NUMBER_OF_MENU_ITEMS
    count = 0;
    for (unsigned int tr = 0; tr < ItemsCount; tr++)
    {
        if (!Items[tr]->Enabled)
            continue;
        if ((Items[tr]->Type == MenuItemType::Command) || (Items[tr]->Type == MenuItemType::Check) ||
            (Items[tr]->Type == MenuItemType::Radio) || (Items[tr]->Type == MenuItemType::SubMenu))
        {
            indexes[count++] = tr;
        }
    }
}
void MenuContext::RunItemAction(unsigned int itemIndex)
{
    if (itemIndex >= this->ItemsCount)
        return;
    auto itm      = this->Items[itemIndex].get();
    int commandID = -1;
    switch (itm->Type)
    {
    case MenuItemType::Check:
        this->SetChecked(itemIndex, !itm->Checked);
        commandID = itm->CommandID;
        break;
    case MenuItemType::Radio:
        this->SetChecked(itemIndex, true);
        commandID = itm->CommandID;
        break;
    case MenuItemType::SubMenu:
        itm->SubMenu->Show(Width + ScreenClip.ScreenPosition.X, ScreenClip.ScreenPosition.Y + 1 + itemIndex - FirstVisibleItem);
        // transfer owner
        (reinterpret_cast<MenuContext*>(itm->SubMenu->Context))->Owner = this->Owner;
        break;
    case MenuItemType::Command:
        commandID = itm->CommandID;
        break;
    }
    if (commandID >= 0)
    {
        Application::GetApplication()->CloseContextualMenu();
        Application::GetApplication()->SendCommand(commandID);
    }
}
void MenuContext::CloseMenu()
{
    if (this->Parent)
        AppCUI::Application::GetApplication()->ShowContextualMenu(this->Parent);
    else
        AppCUI::Application::GetApplication()->CloseContextualMenu();
}
void MenuContext::UpdateFirstVisibleItem()
{
    // if no current item -> exit
    if (this->CurrentItem >= this->ItemsCount)
        return;
    if (this->CurrentItem < this->FirstVisibleItem)
        this->FirstVisibleItem = this->CurrentItem;
    if ((this->CurrentItem - this->FirstVisibleItem) >= this->VisibleItemsCount)
        this->FirstVisibleItem = (this->CurrentItem - this->VisibleItemsCount) + 1;
}
void MenuContext::MoveCurrentItemTo(AppCUI::Input::Key keyCode)
{
    unsigned int idx[MAX_NUMBER_OF_MENU_ITEMS];
    unsigned int idxCount;
    CreateAvailableItemsList(idx, idxCount);
    if (idxCount == 0)
    {
        // no items or all items are disabled
        this->CurrentItem = NO_MENUITEM_SELECTED;
        return;
    }
    // if CurrentItem is NO_MENUITEM_SELECTED ==> select the first available item
    if (this->CurrentItem >= this->ItemsCount)
    {
        this->CurrentItem = idx[0]; 
    }
    else
    {
        // make sure that this->CurrentItem is part of the list
        unsigned int currentIdx = 0xFFFFFFFF;
        unsigned int bestDiff   = 0xFFFFFFFF;
        for (unsigned int tr = 0; tr < idxCount; tr++)
        {
            unsigned int diff;
            if (idx[tr] < this->CurrentItem)
                diff = this->CurrentItem - idx[tr];
            else
                diff = idx[tr] - this->CurrentItem;
            if (diff < bestDiff)
            {
                bestDiff   = diff;
                currentIdx = tr;
            }
        }
        // sanity check
        if (currentIdx>=idxCount)
        {
            // no item is selected
            this->CurrentItem = NO_MENUITEM_SELECTED;
            return;          
        }
        // compute the new position
        switch (keyCode)
        {
        case Key::Up:
            if (currentIdx > 0)
                currentIdx--;
            else
                currentIdx = idxCount - 1;
            break;
        case Key::Down:
            currentIdx++;
            if (currentIdx >= idxCount)
                currentIdx = 0;
            break;
        case Key::PageUp:
            if (currentIdx >= this->VisibleItemsCount)
                currentIdx -= this->VisibleItemsCount;
            else
                currentIdx = 0;
            break;
        case Key::PageDown:
            currentIdx += this->VisibleItemsCount;
            if (currentIdx >= idxCount)
                currentIdx = idxCount - 1;
            break;
        case Key::Home:
            currentIdx = 0;
            break;
        case Key::End:
            currentIdx = idxCount - 1;
            break;
        }
        this->CurrentItem = idx[currentIdx];
    }
    UpdateFirstVisibleItem();
}
// key events
bool MenuContext::OnKeyEvent(AppCUI::Input::Key keyCode)
{
    // check movement keys
    switch (keyCode)
    {
        case Key::Up:
        case Key::Down:
        case Key::Home:
        case Key::End:
        case Key::PageUp:
        case Key::PageDown:
            MoveCurrentItemTo(keyCode);
            return true;
        case Key::Enter:
        case Key::Space:
            RunItemAction(this->CurrentItem);
            return true;
        case Key::Escape:
            CloseMenu();
            return true;
        case Key::Right:
            if ((this->CurrentItem < ItemsCount) && 
                (Items[this->CurrentItem]->Enabled) && 
                (Items[this->CurrentItem]->Type == MenuItemType::SubMenu))
            {
                RunItemAction(this->CurrentItem);
                return true;
            }
            return false;
        case Key::Left:
            if (this->Parent)
            {
                CloseMenu();
                return true;
            }
            return false;

    }
    // check short keys
    for (unsigned int tr=0;tr<ItemsCount;tr++)
    {
        if ((Items[tr]->HotKey != Key::None) && (Items[tr]->HotKey == keyCode) && (Items[tr]->Enabled))
        {
            this->CurrentItem = tr;
            UpdateFirstVisibleItem();
            RunItemAction(tr);
            return true;
        }
    }
    // no binding
    return false;
}
bool MenuContext::ProcessShortCut(AppCUI::Input::Key keyCode)
{
    for (unsigned int tr=0;tr<this->ItemsCount;tr++)
    {
        if (!Items[tr]->Enabled)
            continue;
        if ((Items[tr]->Type == MenuItemType::Command) ||
            (Items[tr]->Type == MenuItemType::Check) ||
            (Items[tr]->Type == MenuItemType::Radio))
        {
            if (Items[tr]->ShortcutKey == keyCode)
            {
                if (Items[tr]->Type == MenuItemType::Check)
                    this->SetChecked(tr, !Items[tr]->Checked);
                if (Items[tr]->Type == MenuItemType::Radio)
                    this->SetChecked(tr, true);
                if (Items[tr]->CommandID >= 0)
                {
                    Application::GetApplication()->SendCommand(Items[tr]->CommandID);
                }
                return true; // key was processed
            }                
        }
        if ((Items[tr]->Type == MenuItemType::SubMenu) && (Items[tr]->SubMenu))
        {
            MenuContext* ctx = reinterpret_cast<MenuContext*>(Items[tr]->SubMenu->Context);
            if (ctx->ProcessShortCut(keyCode))
                return true;
        }
    }
    // if nothing matched - return false;
    return false;
}

void MenuContext::Show(AppCUI::Controls::Menu* me, AppCUI::Controls::Control* relativeControl, int x, int y, const AppCUI::Graphics::Size& maxSize)
{
    // compute abosolute position
    while (relativeControl)
    {
        x += relativeControl->GetX();        
        y += relativeControl->GetY();
        // move to parent        
        relativeControl = relativeControl->GetParent();
        // add parent margins
        if (relativeControl)
        {
            x += ((ControlContext*) relativeControl->Context)->Margins.Left;
            y += ((ControlContext*) relativeControl->Context)->Margins.Top;
        }
    }
    // compute best width
    unsigned int maxWidthLeft   = 0;
    unsigned int maxHotKeyWidth = 0;
    for (unsigned int tr = 0; tr < this->ItemsCount;tr++)
    {
        auto i               = this->Items[tr].get();
        unsigned int w_left = i->Name.Len()+4;
        unsigned int w_right = 0;
        if ((i->Type == MenuItemType::Radio) || (i->Type == MenuItemType::Check))
            w_left += 2;
        if (i->ShortcutKey != Key::None)
        {
            w_right += (unsigned int)KeyUtils::GetKeyName(i->ShortcutKey).size();
            w_right += (unsigned int)KeyUtils::GetKeyModifierName(i->ShortcutKey).size();     
            if (w_right > 0)
                w_right += 2;
        }
        maxWidthLeft   = std::max<>(maxWidthLeft, w_left);
        maxHotKeyWidth = std::max<>(maxHotKeyWidth, w_right);
    }
    unsigned int BestWidth = maxWidthLeft + maxHotKeyWidth;
    BestWidth              = BestWidth | 1; // make sure it's not an odd number (this will help better position Arrow Up and Down)
    // Check agains app size
    Size appSize;
    if (!Application::GetApplicationSize(appSize))
    {
        LOG_WARNING("Unable to retrieve application size --> contextual menu will not be display !");
        return;
    }
    if ((appSize.Height < 5) || (appSize.Width < 10))
    {
        LOG_WARNING(
              "Current application size %d x %d is too small to display a contextual menu (a size of at least 10 x 5 "
              "is required)",
              appSize.Width,
              appSize.Height);
        return;
    }
    // adjust X and Y to be on the screen
    x = std::max<>(x, 0);
    y = std::max<>(y, 0);
    x = std::min<>(x, (int) appSize.Width);
    y = std::min<>(y, (int) appSize.Height);

    // validate max and min limits for menu width and height
    auto maxWidthForCurrentScreen  = std::max<>((appSize.Width / 4), 37U); // use a non-odd number (31 / 33 / 35 --> bigger them 30)
    auto maxHeightForCurrentScreen = std::max<>((appSize.Height - 4), 5U);  
    if (maxSize.Width >= 30)
        maxWidthForCurrentScreen   = std::min<>(maxWidthForCurrentScreen, (maxSize.Width|1));
    if (maxSize.Height>=5)
        maxHeightForCurrentScreen  = std::min<>(maxHeightForCurrentScreen, maxSize.Height);
    unsigned int menuWidth         = std::min<>(BestWidth + 2, maxWidthForCurrentScreen);
    unsigned int menuHeight        = std::min<>(this->ItemsCount + 2, maxHeightForCurrentScreen);  

    // Set direction
    bool toLeft, toBottom;
    if (x + menuWidth <= appSize.Width)
        toLeft = true; // best fit on left
    else if (x >= (int)menuWidth)
        toLeft = false; // best fit on right
    else
        toLeft = x < (int)(appSize.Width / 2); // if x is closest to right edge - expand to left, otherwise to right

    if (y + menuHeight <= appSize.Height)
        toBottom = true; // best fit on bottom
    else if (y >= (int)menuHeight)
        toBottom = false; // best fit on top
    else
    {
        toBottom = y < (int) (appSize.Height / 2); // if y is closest to top edge - expand to top, otherwise to bottom
        if (toBottom)
            menuHeight = std::max<>(appSize.Height - y, 5U);
        else
            menuHeight = std::max<>(y, 5); // y - 0 = y
    }
        
    VisibleItemsCount = menuHeight - 2;
    Width             = menuWidth - 2;
    TextWidth         = Width - (maxHotKeyWidth+2);
    // set the actual clip
    if (toLeft)
    {
        if (toBottom)
            this->ScreenClip.Set(x, y, menuWidth, menuHeight);
        else
            this->ScreenClip.Set(x, y + 1 - (int) menuHeight, menuWidth, menuHeight);
    }
    else
    {
        if (toBottom)
            this->ScreenClip.Set(x + 1 - (int) menuWidth, y, menuWidth, menuHeight);
        else
            this->ScreenClip.Set(x + 1 - (int) menuWidth, y + 1 - (int) menuHeight, menuWidth, menuHeight);
    }
    // clear selection & buttons
    this->FirstVisibleItem = 0;
    this->CurrentItem      = NO_MENUITEM_SELECTED;
    this->ButtonUp         = MenuButtonState::Normal;
    this->ButtonDown       = MenuButtonState::Normal;
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
    return CTX->AddItem(std::make_unique<MenuItem>(MenuItemType::Command, text, CommandID, false, shortcutKey));
}
ItemHandle Menu::AddCheckItem(const AppCUI::Utils::ConstString& text, int CommandID, bool checked, AppCUI::Input::Key shortcutKey)
{
    return CTX->AddItem(std::make_unique<MenuItem>(MenuItemType::Check, text, CommandID, checked, shortcutKey));
}
ItemHandle Menu::AddRadioItem(const AppCUI::Utils::ConstString& text, int CommandID, bool checked, AppCUI::Input::Key shortcutKey)
{
    return CTX->AddItem(std::make_unique<MenuItem>(MenuItemType::Radio, text, CommandID, checked, shortcutKey));
}
ItemHandle Menu::AddSeparator()
{
    return CTX->AddItem(std::make_unique<MenuItem>());
}
ItemHandle Menu::AddSubMenu(const AppCUI::Utils::ConstString& text)
{
    try
    {
        Menu* SubMenu                               = new Menu(); 
        ((MenuContext*) (SubMenu->Context))->Parent = this;
        return CTX->AddItem(std::make_unique<MenuItem>(text, SubMenu));
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

void Menu::Show(int x, int y, const AppCUI::Graphics::Size& maxSize)
{
    CTX->Show(this, nullptr, x, y, maxSize);
}
void Menu::Show(Control* parent, int relativeX, int relativeY, const AppCUI::Graphics::Size& maxSize)
{
    CTX->Show(this, parent, relativeX, relativeY, maxSize);
}
#undef CTX