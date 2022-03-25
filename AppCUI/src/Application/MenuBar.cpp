#include "ControlContext.hpp"
#include "Internal.hpp"

namespace AppCUI::Internal
{
constexpr uint32 NO_ITEM_SELECTED = 0xFFFFFFFFU;

MenuBarItem::MenuBarItem()
{
    this->HotKey       = Input::Key::None;
    this->HotKeyOffset = Graphics::CharacterBuffer::INVALID_HOTKEY_OFFSET;
    this->X            = 0;
}
MenuBar::MenuBar(Controls::Control* parent, int x, int y)
{
    this->ItemsCount  = 0;
    this->Width       = 0;
    this->OpenedItem  = NO_ITEM_SELECTED;
    this->HoveredItem = NO_ITEM_SELECTED;
    this->Cfg         = Application::GetAppConfig();
    this->X           = x;
    this->Y           = y;
    this->Parent      = parent;
}
Menu* MenuBar::GetMenu(ItemHandle itemHandle)
{
    CHECK((uint32) itemHandle < this->ItemsCount,
          nullptr,
          "Invalid item handle (%08X)",
          (uint32) itemHandle);
    return &Items[(uint32) itemHandle]->Mnu;
}
ItemHandle MenuBar::AddMenu(const ConstString& name)
{
    CHECK(this->ItemsCount < MenuBar::MAX_ITEMS,
          ItemHandle{ NO_ITEM_SELECTED },
          "Too many menu items - max allowed is: %d",
          MenuBar::MAX_ITEMS);
    if (!Items[this->ItemsCount])
        Items[this->ItemsCount] = std::make_unique<MenuBarItem>();
    auto* i = Items[this->ItemsCount].get();
    CHECK(i->Name.SetWithHotKey(name, i->HotKeyOffset, i->HotKey, Key::Alt),
          ItemHandle{ NO_ITEM_SELECTED },
          "Fail to set Menu name");

    this->ItemsCount++;
    RecomputePositions();
    return ItemHandle{ this->ItemsCount - 1 };
}
void MenuBar::RecomputePositions()
{
    int x = 0;
    for (uint32 tr = 0; tr < this->ItemsCount; tr++)
    {
        Items[tr]->X = x;
        x += (int) (2 + Items[tr]->Name.Len());
    }
}
void MenuBar::SetWidth(uint32 value)
{
    Width = value;
    RecomputePositions();
}
bool MenuBar::OnMouseMove(int x, int y, bool& repaint)
{
    uint32 idx = MousePositionToItem(x, y);
    if (idx != this->HoveredItem)
    {
        this->HoveredItem = idx;
        repaint           = true;
        // if MenuBar is already opened, moving a mouse over another menu will implicetely open that menu
        if ((this->OpenedItem != NO_ITEM_SELECTED) && (idx != NO_ITEM_SELECTED))
            Open(idx);
        return true;
    }
    if (y == this->Y)
    {
        repaint = false;
        return true;
    }
    return false;
}
uint32 MenuBar::MousePositionToItem(int x, int y)
{
    x -= this->X;
    y -= this->Y;
    if (y != 0)
        return NO_ITEM_SELECTED;
    for (uint32 tr = 0; tr < this->ItemsCount; tr++)
    {
        if ((x >= Items[tr]->X) && (x < (Items[tr]->X + (int) Items[tr]->Name.Len() + 2)))
            return tr;
    }
    return NO_ITEM_SELECTED;
}
void MenuBar::Open(uint32 menuIndex)
{
    this->OpenedItem = menuIndex;
    if (menuIndex < ItemsCount)
    {
        Items[menuIndex]->Mnu.Show(this->Parent, this->X + Items[menuIndex]->X, this->Y + 1);
        // set the owner
        ((MenuContext*) (Items[menuIndex]->Mnu.Context))->Owner = this;
    }
}
bool MenuBar::OnMousePressed(int x, int y, Input::MouseButton /*button*/)
{
    uint32 idx = MousePositionToItem(x, y);
    if (idx != this->OpenedItem)
    {
        Open(idx);
        return true;
    }
    return false;
}
void MenuBar::Close()
{
    this->OpenedItem  = NO_ITEM_SELECTED;
    this->HoveredItem = NO_ITEM_SELECTED;
}
bool MenuBar::IsOpened()
{
    return this->OpenedItem != NO_ITEM_SELECTED;
}
bool MenuBar::OnKeyEvent(Input::Key keyCode)
{
    if (IsOpened())
    {
        switch (keyCode)
        {
        case Key::Left:
            if (this->OpenedItem > 0)
                Open(this->OpenedItem - 1);
            else
                Open(this->ItemsCount - 1);
            return true;
        case Key::Right:
            if (this->OpenedItem + 1 < ItemsCount)
                Open(this->OpenedItem + 1);
            else
                Open(0);
            return true;
        default:
            break;
        }
    }
    else
    {
        // if not open - check for hot keys
        for (uint32 tr = 0; tr < this->ItemsCount; tr++)
        {
            if (this->Items[tr]->HotKey == keyCode)
            {
                Open(tr);
                return true;
            }
        }
    }
    // check recursivelly if a shortcut key was not pressed
    for (uint32 tr = 0; tr < this->ItemsCount; tr++)
    {
        if (this->Items[tr]->Mnu.ProcessShortcutKey(keyCode))
        {
            Close();
            return true;
        }
    }
    // nothing to process
    return false;
}
void MenuBar::Paint(Graphics::Renderer& renderer)
{
    renderer.FillHorizontalLine(this->X, this->Y, this->X + Width - 1, ' ', Cfg->Menu.Text.Normal);
    WriteTextParams params(
          WriteTextFlags::SingleLine | WriteTextFlags::LeftMargin | WriteTextFlags::RightMargin |
                WriteTextFlags::OverwriteColors | WriteTextFlags::HighlightHotKey,
          TextAlignament::Left);
    params.Y = this->Y;

    for (uint32 tr = 0; tr < this->ItemsCount; tr++)
    {
        params.X              = this->X + Items[tr]->X + 1;
        params.HotKeyPosition = Items[tr]->HotKeyOffset;

        if (tr == this->OpenedItem)
        {
            params.Color       = Cfg->Menu.Text.PressedOrSelected;
            params.HotKeyColor = Cfg->Menu.HotKey.PressedOrSelected;
        }
        else if (tr == this->HoveredItem)
        {
            params.Color       = Cfg->Menu.Text.Hovered;
            params.HotKeyColor = Cfg->Menu.HotKey.Hovered;
        }
        else
        {
            params.Color       = Cfg->Menu.Text.Normal;
            params.HotKeyColor = Cfg->Menu.HotKey.Normal;
        }

        renderer.WriteText(Items[tr]->Name, params);
    }
}
} // namespace AppCUI::Internal
