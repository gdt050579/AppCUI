#include "../../include/ControlContext.h"

using namespace AppCUI::Controls;
using namespace AppCUI::Console;
using namespace AppCUI::Input;

#define CHECK_INDEX(idx, returnValue) CHECK(idx < (unsigned int)Members->Items.size(), returnValue, "Invalid index (%d) , should be smaller than %d", (int)idx, (int)Members->Items.size());
static ItemData null_combobox_item = { 0 };

bool ComboBox_AddItem(ComboBox *control, const char *itemName, int itemSize = -1, ItemData userData = { 0 })
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
void ComboBox_MoveTo(ComboBox *control,int newPoz)
{
	CREATE_TYPE_CONTEXT(ComboBoxControlContext, control, Members, );
	int old = Members->CurentItem;
    int itemsCount = Members->Items.size();
	if (itemsCount == 0) return;
	if (newPoz<0) newPoz = 0;
	if (newPoz> itemsCount - 1) newPoz = itemsCount - 1;
	
	while (Members->CurentItem != newPoz)
	{
		if (Members->CurentItem>newPoz) Members->CurentItem--;
		if (Members->CurentItem<newPoz) Members->CurentItem++;
		if (Members->CurentItem<Members->ky) Members->ky = Members->CurentItem;
		if (Members->CurentItem >= Members->ky + Members->VisibleItems)
		{
			Members->ky = Members->CurentItem - Members->VisibleItems + 1;
			if (Members->ky<0) Members->ky = 0;
		}
	}
	if (old != Members->CurentItem)
		control->RaiseEvent(Event::EVENT_COMBOBOX_SELECTED_ITEM_CHANGED);
}
//====================================================================================================
ComboBox::~ComboBox()
{
	DELETE_CONTROL_CONTEXT(ComboBoxControlContext);
}
bool ComboBox::Create(Control *parent, const char * layout, const char* items, char itemsSeparator)
{
	CONTROL_INIT_CONTEXT(ComboBoxControlContext);
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    Members->Layout.MinWidth = 7;
    Members->Layout.MinHeight = 1;
    Members->Layout.MaxHeight = 1;
	CHECK(Init(parent, "", layout, false), false, "Failed to create combo box !");
	
	Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
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
	Members->VisibleItems = 4;
	Members->CurentItem = ComboBox::NO_ITEM_SELECTED;
	Members->ky = 0;
	return true;
}
unsigned int ComboBox::GetItemsCount()
{
	CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, 0);
    return (unsigned int)Members->Items.size();
}
const char* ComboBox::GetUnsafeCurrentItemText()
{
	CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, nullptr);
    return GetUnsafeItemText(Members->CurentItem);
}
bool        ComboBox::GetCurrentItemtext(Utils::String &itemText)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    return GetItemText(Members->CurentItem, itemText);
}
unsigned int ComboBox::GetCurrentItemIndex()
{
	CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, ComboBox::NO_ITEM_SELECTED);
	return Members->CurentItem;
}
ItemData	ComboBox::GetCurrentItemUserData()
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, null_combobox_item);
    return GetItemUserData(Members->CurentItem);
}
ItemData	ComboBox::GetItemUserData(unsigned int index)
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
bool ComboBox::GetItemText(unsigned int index,Utils::String &itemText)
{
    CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    CHECK_INDEX(index, false);
    CHECK(itemText.Set(Members->Items[index].Nume), false, "Fail to copy item #%d to string", (int)index);
    return true;
}
bool ComboBox::SetItemUserData(unsigned int index, ItemData userData)
{
	CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    CHECK_INDEX(index, false);
    Members->Items[index].Data = userData;
	return true;
}
bool ComboBox::AddItem(const char *ss, ItemData userData)
{
	CHECK(ss != nullptr, false, "");
	CHECK(ComboBox_AddItem(this, ss, -1, userData), false, "");
	CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
	if ((Members->CurentItem<0) && (Members->Items.size()>0)) 
	{ 
		Members->CurentItem = 0; 
		Members->ky = 0;
	}
	return true;
}
void ComboBox::DeleteAllItems()
{
	CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, );
    Members->Items.clear();
	Members->CurentItem = ComboBox::NO_ITEM_SELECTED;
    Members->ky = 0;
}
bool ComboBox::SetCurentItemIndex(unsigned int index)
{
	CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
    CHECK_INDEX(index, false);
	ComboBox_MoveTo(this, index);
    return true;
}
void ComboBox::SetNoIndexSelected()
{
	CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, );
	Members->CurentItem = ComboBox::NO_ITEM_SELECTED;
	Members->ky = 0;
	RaiseEvent(Event::EVENT_COMBOBOX_SELECTED_ITEM_CHANGED);
}
void ComboBox::OnAfterResize(int newWidth,int newHeight)
{
	//CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, );
}
bool ComboBox::OnKeyEvent(AppCUI::Input::Key::Type keyCode, char AsciiCode)
{
	CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, false);
	switch (keyCode)
	{
		case Key::Up: 
			ComboBox_MoveTo(this,((int)Members->CurentItem) - 1); 
			return true;
		case Key::Down:
			ComboBox_MoveTo(this, ((int)Members->CurentItem) + 1);
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
void ComboBox::OnHotKey()
{
	SetChecked(!IsChecked());
	CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, );
	if (!IsChecked())
        RaiseEvent(Event::EVENT_COMBO_CLOSED);
}
void ComboBox::OnMouseReleased(int x, int y,  int butonState)
{
	CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, );

	if (y == 0)
		OnHotKey();
	if ((y>1) && (y<2 + Members->VisibleItems))
	{
		ComboBox_MoveTo(this, Members->ky + y - 2);//MenuIsVisible=false;
		RaiseEvent(Event::EVENT_COMBO_CLOSED);
	}
}

void ComboBox::Paint(Console::Renderer & renderer)
{
	CREATE_TYPECONTROL_CONTEXT(ComboBoxControlContext, Members, );

    auto * cbc = &Members->Cfg->ComboBox.Normal;
    if (!this->IsEnabled())
        cbc = &Members->Cfg->ComboBox.Inactive;
    if (Members->Focused)
        cbc = &Members->Cfg->ComboBox.Focus;
    else if (Members->MouseIsOver)
        cbc = &Members->Cfg->ComboBox.Hover;

    renderer.DrawHorizontalLine(0, 0, Members->Layout.Width - 5, ' ', cbc->Text);
    const char * txt = GetUnsafeCurrentItemText();
    if (txt)
        renderer.WriteSingleLineText(1, 0, txt, Members->Layout.Width - 6, cbc->Text);
    renderer.WriteSingleLineText(Members->Layout.Width - 3, 0, "   ", cbc->Button, 3);
    renderer.WriteSpecialCharacter(Members->Layout.Width - 2, 0, SpecialChars::TriangleDown, cbc->Button);

}
bool    ComboBox::OnMouseLeave()
{
    return true;
}
bool    ComboBox::OnMouseEnter()
{
    return true;
}
