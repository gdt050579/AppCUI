#include "ControlContext.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Console;
using namespace AppCUI::Input;

#define CHECK_INDEX(idx, returnValue)                                                                                  \
    CHECK(idx < (unsigned int) Members->Items.size(),                                                                  \
          returnValue,                                                                                                 \
          "Invalid index (%d) , should be smaller than %d",                                                            \
          (int) idx,                                                                                                   \
          (int) Members->Items.size());
static ItemData null_combobox_item = { 0 };

bool ComboBox_AddItem(ComboBox* control, const char* itemName, int itemSize = -1, ItemData userData = { 0 })
{
    CREATE_TYPE_CONTEXT(ComboBoxControlContext, control, Members, false);
    CHECK(itemName != nullptr, false, "");
    COMBO_ITEM_INFO item;
    if (itemSize < 0)
        itemSize = String::Len(itemName);
    itemSize = MINVALUE(COMBOBOX_ITEM_SIZE - 1, itemSize);
    CHECK(String::Set(item.Nume, itemName, COMBOBOX_ITEM_SIZE, itemSize), false, "");
    item.Data = userData;
    Members->Items.push_back(item);
    return true;
}
void ComboBox_MoveTo(ComboBox* control, unsigned int newPoz)
{
    CREATE_TYPE_CONTEXT(ComboBoxControlContext, control, Members, );
    unsigned int old        = Members->CurentItemIndex;
    unsigned int itemsCount = (unsigned int) Members->Items.size();
    if (itemsCount == 0)
        return;
    if ((itemsCount > 0) && (newPoz > itemsCount - 1))
        newPoz = itemsCount - 1;

    while (Members->CurentItemIndex != newPoz)
    {
        if (Members->CurentItemIndex > newPoz)
            Members->CurentItemIndex--;
        if (Members->CurentItemIndex < newPoz)
            Members->CurentItemIndex++;
        if (Members->CurentItemIndex < Members->FirstVisibleItem)
            Members->FirstVisibleItem = Members->CurentItemIndex;
        if (Members->CurentItemIndex >= Members->FirstVisibleItem + Members->VisibleItems)
        {
            Members->FirstVisibleItem = (Members->CurentItemIndex - Members->VisibleItems) + 1;
        }
    }
    if (old != Members->CurentItemIndex)
        control->RaiseEvent(Event::EVENT_COMBOBOX_SELECTED_ITEM_CHANGED);
}
//====================================================================================================
ComboBox::~ComboBox()
{
    DELETE_CONTROL_CONTEXT(ComboBoxControlContext);
}
bool ComboBox::Create(Control* parent, const char* layout, const char* items, char itemsSeparator)
{
    CONTROL_INIT_CONTEXT(ComboBoxControlContext);
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    Members->Layout.MinWidth  = 7;
    Members->Layout.MinHeight = 1;
    Members->Layout.MaxHeight = 1;
    CHECK(Init(parent, "", layout, false), false, "Failed to create combo box !");

    Members->Flags                        = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
    unsigned int initialAllocatedElements = 8;
    if (items)
    {
        unsigned int count = 1; // assume at least one element (if not item separator is found)
        for (unsigned int tr = 0; items[tr] != 0; tr++)
        {
            if (items[tr] == itemsSeparator)
                count++;
        }
        if (count > initialAllocatedElements)
            initialAllocatedElements = (count | 3) + 1;
    }
    Members->Items.reserve(initialAllocatedElements);
    if (items)
    {
        unsigned int start = 0, tr;
        for (tr = 0; items[tr] != 0; tr++)
        {
            if (items[tr] == itemsSeparator)
            {
                if (start < tr)
                {
                    CHECK(ComboBox_AddItem(this, &items[start], tr - start), false, "");
                }
                start = tr + 1;
            }
        }
        if (start < tr)
        {
            CHECK(ComboBox_AddItem(this, &items[start], tr - start), false, "");
        }
    }
    Members->VisibleItems     = 1;
    Members->CurentItemIndex  = ComboBox::NO_ITEM_SELECTED;
    Members->FirstVisibleItem = 0;
    return true;
}
unsigned int ComboBox::GetItemsCount()
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, 0);
    return (unsigned int) Members->Items.size();
}
const char* ComboBox::GetUnsafeCurrentItemText()
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, nullptr);
    return GetUnsafeItemText(Members->CurentItemIndex);
}
bool ComboBox::GetCurrentItemtext(Utils::String& itemText)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    return GetItemText(Members->CurentItemIndex, itemText);
}
unsigned int ComboBox::GetCurrentItemIndex()
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, ComboBox::NO_ITEM_SELECTED);
    return Members->CurentItemIndex;
}
ItemData ComboBox::GetCurrentItemUserData()
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, null_combobox_item);
    return GetItemUserData(Members->CurentItemIndex);
}
ItemData ComboBox::GetItemUserData(unsigned int index)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, null_combobox_item);
    CHECK_INDEX(index, null_combobox_item);
    return Members->Items[index].Data;
}
const char* ComboBox::GetUnsafeItemText(unsigned int index)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, nullptr);
    CHECK_INDEX(index, nullptr);
    return Members->Items[index].Nume;
}
bool ComboBox::GetItemText(unsigned int index, Utils::String& itemText)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    CHECK_INDEX(index, false);
    CHECK(itemText.Set(Members->Items[index].Nume), false, "Fail to copy item #%d to string", (int) index);
    return true;
}
bool ComboBox::SetItemUserData(unsigned int index, ItemData userData)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    CHECK_INDEX(index, false);
    Members->Items[index].Data = userData;
    return true;
}
bool ComboBox::AddItem(const char* ss, ItemData userData)
{
    CHECK(ss != nullptr, false, "");
    CHECK(ComboBox_AddItem(this, ss, -1, userData), false, "");
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    if ((Members->CurentItemIndex < 0) && (Members->Items.size() > 0))
    {
        Members->CurentItemIndex  = 0;
        Members->FirstVisibleItem = 0;
    }
    return true;
}
void ComboBox::DeleteAllItems()
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, );
    Members->Items.clear();
    Members->CurentItemIndex  = ComboBox::NO_ITEM_SELECTED;
    Members->FirstVisibleItem = 0;
}
bool ComboBox::SetCurentItemIndex(unsigned int index)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    CHECK_INDEX(index, false);
    if ((Members->CurentItemIndex == ComboBox::NO_ITEM_SELECTED) && (Members->Items.size() > 0))
        Members->CurentItemIndex = 0;
    ComboBox_MoveTo(this, index);
    return true;
}
void ComboBox::SetNoIndexSelected()
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, );
    Members->CurentItemIndex  = ComboBox::NO_ITEM_SELECTED;
    Members->FirstVisibleItem = 0;
    RaiseEvent(Event::EVENT_COMBOBOX_SELECTED_ITEM_CHANGED);
}

bool ComboBox::OnKeyEvent(AppCUI::Input::Key keyCode, char AsciiCode)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    switch (keyCode)
    {
    case Key::Up:
        if ((Members->CurentItemIndex == ComboBox::NO_ITEM_SELECTED) && (Members->Items.size() > 0))
            Members->CurentItemIndex = 0;
        else if (Members->CurentItemIndex > 0)
            ComboBox_MoveTo(this, Members->CurentItemIndex - 1);
        return true;
    case Key::Down:
        if ((Members->CurentItemIndex == ComboBox::NO_ITEM_SELECTED) && (Members->Items.size() > 0))
            Members->CurentItemIndex = 0;
        else if (Members->CurentItemIndex != ComboBox::NO_ITEM_SELECTED)
            ComboBox_MoveTo(this, Members->CurentItemIndex + 1);
        return true;
    case Key::Home:
        ComboBox_MoveTo(this, 0);
        return true;
    case Key::End:
        ComboBox_MoveTo(this, 0xFFFF);
        return true;

    case Key::Space:
    case Key::Enter:
        OnHotKey();
        return true;
    }
    return false;
}
void ComboBox::OnExpandView(AppCUI::Console::Clip& expandedClip)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, );
    AppCUI::Console::Size appSize;
    Members->VisibleItems     = 4;
    Members->HoveredIndexItem = ComboBox::NO_ITEM_SELECTED;
    if ((Application::GetApplicationSize(appSize)) && (expandedClip.ClipRect.Y >= 0))
    {
        if (appSize.Height > (unsigned int) (expandedClip.ClipRect.Y + 3))
            Members->VisibleItems = (appSize.Height - (unsigned int) (expandedClip.ClipRect.Y + 3));
    }
    if (Members->VisibleItems > Members->Items.size())
        Members->VisibleItems = (unsigned int) Members->Items.size();
    expandedClip.ClipRect.Height += Members->VisibleItems + 2;
    Members->ExpandedHeight = Members->VisibleItems + 2;
}
void ComboBox::OnPackView()
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, );
    Members->VisibleItems     = 1;
    Members->HoveredIndexItem = ComboBox::NO_ITEM_SELECTED;
    if (Members->CurentItemIndex != ComboBox::NO_ITEM_SELECTED)
        Members->FirstVisibleItem = Members->CurentItemIndex;
}
void ComboBox::OnHotKey()
{
    SetChecked(!IsChecked());
    if (IsChecked())
        this->ExpandView();
    else
    {
        this->PackView();
        RaiseEvent(Event::EVENT_COMBO_CLOSED);
    }
}
void ComboBox::OnMousePressed(int x, int y, int butonState)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, );

    if ((y > 1) && (y < (int) (2 + Members->VisibleItems)))
    {
        if ((Members->CurentItemIndex == ComboBox::NO_ITEM_SELECTED) && (Members->Items.size() > 0))
            Members->CurentItemIndex = 0;
        ComboBox_MoveTo(this, Members->FirstVisibleItem + y - 2); // MenuIsVisible=false;
    }
    OnHotKey();
}
bool ComboBox::OnMouseOver(int x, int y)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    if ((y > 1) && (y < (int) (2 + Members->VisibleItems)))
    {
        unsigned int newIndex = ((unsigned int) (y - 2)) + Members->FirstVisibleItem;
        if (newIndex >= Members->Items.size())
            newIndex = ComboBox::NO_ITEM_SELECTED;
            
        if (newIndex != Members->HoveredIndexItem)
        {
            Members->HoveredIndexItem = newIndex;
            return true; // repaint required
        }
    }
    else
    {
        if (ComboBox::NO_ITEM_SELECTED != Members->HoveredIndexItem)
        {
            Members->HoveredIndexItem = ComboBox::NO_ITEM_SELECTED;
            return true; // repaint required
        }
    }
    return false;
}
bool ComboBox::OnMouseLeave()
{
    return true;
}
bool ComboBox::OnMouseEnter()
{
    return true;
}
void ComboBox::Paint(Console::Renderer& renderer)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, );

    auto* cbc = &Members->Cfg->ComboBox.Normal;
    if (!this->IsEnabled())
        cbc = &Members->Cfg->ComboBox.Inactive;
    if (Members->Focused)
        cbc = &Members->Cfg->ComboBox.Focus;
    else if (Members->MouseIsOver)
        cbc = &Members->Cfg->ComboBox.Hover;

    renderer.DrawHorizontalLine(0, 0, Members->Layout.Width - 5, ' ', cbc->Text);
    const char* txt = GetUnsafeCurrentItemText();
    if (txt)
        renderer.WriteSingleLineText(1, 0, txt, Members->Layout.Width - 6, cbc->Text);
    renderer.WriteSingleLineText(Members->Layout.Width - 3, 0, "   ", cbc->Button, 3);
    renderer.WriteSpecialCharacter(Members->Layout.Width - 2, 0, SpecialChars::TriangleDown, cbc->Button);

    if (Members->Flags & GATTR_EXPANDED)
    {
        renderer.FillRect(
              0, 1, Members->Layout.Width - 1, Members->ExpandedHeight, ' ', Members->Cfg->ComboBox.Focus.Text);
        renderer.DrawRect(
              0, 1, Members->Layout.Width - 1, Members->ExpandedHeight, Members->Cfg->ComboBox.Focus.Text, false);
        for (unsigned int tr = 0; tr < Members->VisibleItems; tr++)
        {
            renderer.WriteSingleLineText(
                  2,
                  tr + 2,
                  GetUnsafeItemText(tr + Members->FirstVisibleItem),
                  Members->Layout.Width - 2,
                  Members->Cfg->ComboBox.Focus.Text,
                  TextAlignament::Left);
            if ((tr + Members->FirstVisibleItem) == Members->CurentItemIndex)
                renderer.DrawHorizontalLine(1, tr + 2, Members->Layout.Width - 2, -1, Members->Cfg->ComboBox.Selection);
            else if ((tr + Members->FirstVisibleItem) == Members->HoveredIndexItem)
                renderer.DrawHorizontalLine(
                      1, tr + 2, Members->Layout.Width - 2, -1, Members->Cfg->ComboBox.HoverOveItem);
        }
    }
}
