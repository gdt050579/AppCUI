#include "Internal.hpp"

using namespace AppCUI::Internal;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;
using namespace AppCUI::Input;

#define NO_ITEM_SELECTED    0xFFFFFFFF

MenuBarItem::MenuBarItem()
{
    this->HotKey       = AppCUI::Input::Key::None;
    this->HotKeyOffset = AppCUI::Graphics::CharacterBuffer::INVALID_HOTKEY_OFFSET;
    this->X            = 0;
}
MenuBar::MenuBar()
{
    this->ItemsCount  = 0;
    this->OpenedItem  = NO_ITEM_SELECTED;
    this->HoveredItem = NO_ITEM_SELECTED;
    this->Cfg         = AppCUI::Application::GetAppConfig();
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
    if (!Items[this->ItemsCount])
        Items[this->ItemsCount] = std::make_unique<MenuBarItem>();
    auto* i = Items[this->ItemsCount].get();
    CHECK(i->Name.SetWithHotKey(name, i->HotKeyOffset), ItemHandle{ NO_ITEM_SELECTED }, "Fail to set Menu name");
    if (i->HotKeyOffset != CharacterBuffer::INVALID_HOTKEY_OFFSET)
    {
        char16_t ch = i->Name.GetBuffer()[i->HotKeyOffset].Code;
        if ((ch >= 'A') && (ch <= 'Z'))
            i->HotKey = static_cast<Key>((unsigned int) Key::A + (ch - 'A'));
        else if ((ch >= 'a') && (ch <= 'z'))
            i->HotKey = static_cast<Key>((unsigned int) Key::A + (ch - 'a'));
        else if ((ch >= '0') && (ch <= '9'))
            i->HotKey = static_cast<Key>((unsigned int) Key::N0 + (ch - '0'));
        else
            i->HotKeyOffset = CharacterBuffer::INVALID_HOTKEY_OFFSET; // invalid hot key
    }
    this->ItemsCount++;
    RecomputePositions();
    return ItemHandle{ this->ItemsCount-1 };
}
void MenuBar::RecomputePositions()
{
    int x = 0;
    for (unsigned int tr=0;tr<this->ItemsCount;tr++)
    {
        Items[tr]->X = x;
        x += (int)(2 + Items[tr]->Name.Len());
    }
}
void MenuBar::SetWidth(unsigned int value)
{
    Width = value;
    RecomputePositions();
}
bool MenuBar::OnMouseMove(int x, int y)
{
    unsigned int idx = MousePositionToItem(x, y);
    if (idx != this->HoveredItem)
    {
        this->HoveredItem = idx;
        return true;
    }
    return false;
}
unsigned int MenuBar::MousePositionToItem(int x, int y)
{
    if (y!=0)
        return NO_ITEM_SELECTED;
    for (unsigned int tr = 0; tr < this->ItemsCount; tr++)
    {
        if ((x >= Items[tr]->X) && (x < (Items[tr]->X + (int)Items[tr]->Name.Len() + 2)))
            return tr;
    }
    return NO_ITEM_SELECTED;
}
bool MenuBar::OnMousePressed(int x, int y, AppCUI::Input::MouseButton button)
{
    unsigned int idx = MousePositionToItem(x, y);
    if (idx != this->OpenedItem)
    {
        this->OpenedItem = idx;
        if (idx != NO_ITEM_SELECTED)
        {
            Items[idx]->Mnu.Show(Items[idx]->X, 1);
        }
        return true;
    }
    return false;
}
void MenuBar::Paint(AppCUI::Graphics::Renderer& renderer)
{
    renderer.DrawHorizontalLine(0, 0, Width, ' ', Cfg->MenuBar.BackgroundColor);
    WriteTextParams params(
          WriteTextFlags::SingleLine | WriteTextFlags::LeftMargin | WriteTextFlags::RightMargin |
          WriteTextFlags::OverwriteColors | WriteTextFlags::HighlightHotKey,
          TextAlignament::Left);
    params.Y = 0;

    for (unsigned int tr = 0; tr < this->ItemsCount; tr++)
    {
        params.X              = Items[tr]->X + 1;
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
#undef NO_ITEM_SELECTED