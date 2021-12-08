#include "ControlContext.hpp"
#include "Internal.hpp"

namespace AppCUI::Internal
{
constexpr unsigned int NO_ITEM_SELECTED = 0xFFFFFFFFU;

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
    CHECK((unsigned int) itemHandle < this->ItemsCount,
          nullptr,
          "Invalid item handle (%08X)",
          (unsigned int) itemHandle);
    return &Items[(unsigned int) itemHandle]->Mnu;
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
    for (unsigned int tr = 0; tr < this->ItemsCount; tr++)
    {
        Items[tr]->X = x;
        x += (int) (2 + Items[tr]->Name.Len());
    }
}
void MenuBar::SetWidth(unsigned int value)
{
    Width = value;
    RecomputePositions();
}
bool MenuBar::OnMouseMove(int x, int y, bool& repaint)
{
    unsigned int idx = MousePositionToItem(x, y);
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
unsigned int MenuBar::MousePositionToItem(int x, int y)
{
    x -= this->X;
    y -= this->Y;
    if (y != 0)
        return NO_ITEM_SELECTED;
    for (unsigned int tr = 0; tr < this->ItemsCount; tr++)
    {
        if ((x >= Items[tr]->X) && (x < (Items[tr]->X + (int) Items[tr]->Name.Len() + 2)))
            return tr;
    }
    return NO_ITEM_SELECTED;
}
void MenuBar::Open(unsigned int menuIndex)
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
    unsigned int idx = MousePositionToItem(x, y);
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
        for (unsigned int tr = 0; tr < this->ItemsCount; tr++)
        {
            if (this->Items[tr]->HotKey == keyCode)
            {
                Open(tr);
                return true;
            }
        }
    }
    // nothing to process
    return false;
}
void MenuBar::Paint(Graphics::Renderer& renderer)
{
    renderer.FillHorizontalLine(this->X, this->Y, this->X + Width - 1, ' ', Cfg->MenuBar.BackgroundColor);
    WriteTextParams params(
          WriteTextFlags::SingleLine | WriteTextFlags::LeftMargin | WriteTextFlags::RightMargin |
                WriteTextFlags::OverwriteColors | WriteTextFlags::HighlightHotKey,
          TextAlignament::Left);
    params.Y = this->Y;

    for (unsigned int tr = 0; tr < this->ItemsCount; tr++)
    {
        params.X              = this->X + Items[tr]->X + 1;
        params.HotKeyPosition = Items[tr]->HotKeyOffset;

        if (tr == this->OpenedItem)
        {
            params.Color       = Cfg->MenuBar.Pressed.NameColor;
            params.HotKeyColor = Cfg->MenuBar.Pressed.HotKeyColor;
        }
        else if (tr == this->HoveredItem)
        {
            params.Color       = Cfg->MenuBar.Hover.NameColor;
            params.HotKeyColor = Cfg->MenuBar.Hover.HotKeyColor;
        }
        else
        {
            params.Color       = Cfg->MenuBar.Normal.NameColor;
            params.HotKeyColor = Cfg->MenuBar.Normal.HotKeyColor;
        }

        renderer.WriteText(Items[tr]->Name, params);
    }
}
} // namespace AppCUI::Internal
