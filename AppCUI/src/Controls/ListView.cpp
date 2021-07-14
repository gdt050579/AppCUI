#include "../../include/ControlContext.h"


using namespace AppCUI::Controls;
using namespace AppCUI::Console;
using namespace AppCUI::Input;

#define ITEM_FLAG_CHECKED				0x0001
#define ITEM_FLAG_SELECTED				0x0002
#define COLUMN_DONT_COPY				1
#define COLUMN_DONT_FILTER				2
#define INVALID_COLUMN_INDEX            0xFFFFFFFF
#define MINIM_COLUMN_WIDTH              3
#define MAXIM_COLUMN_WIDTH              256
#define NO_HOTKEY_FOR_COLUMN            0xFF


#define PREPARE_LISTVIEW_ITEM(index,returnValue) \
    CHECK(index<ItemsList.size(),returnValue,"Invalid index: %d",index); \
    ListViewItem &i = ItemsList[index];

#define WRAPPER	((ListViewControlContext*)this->Context)

void ListViewHeader::Reset()
{
    this->HotKeyCode = Key::None;
    this->HotKeyOffset = NO_HOTKEY_FOR_COLUMN;
    this->Name[0] = 0;
    this->NameLength = 0;
    this->Flags = 0;
    this->Align = TextAlignament::Left;
    this->Width = 10;
}
bool ListViewHeader::SetName(const char * text)
{
    CHECK(text, false, "Expecting a valid name for the column (non null)");

    this->HotKeyCode = Key::None;
    this->HotKeyOffset = NO_HOTKEY_FOR_COLUMN;
    char * p = this->Name;
    char * e = p + MAX_LISTVIEW_HEADER_TEXT - 2;
    while ((p < e) && (*text))
    {
        if ((*text) == '&')
        {
            char hotKey = *text + 1;
            if ((hotKey >= 'a') && (hotKey <= 'z'))
            {
                this->HotKeyCode = (Key::Type)(Key::Ctrl | ((unsigned int)Key::A + (hotKey - 'a')));
                this->HotKeyOffset = (unsigned char)(p - this->Name);
            }
            if ((hotKey >= 'A') && (hotKey <= 'Z'))
            {
                this->HotKeyCode = (Key::Type)(Key::Ctrl | ((unsigned int)Key::A + (hotKey - 'A')));
                this->HotKeyOffset = (unsigned char)(p - this->Name);
            }
            if ((hotKey >= '0') && (hotKey <= '9'))
            {
                this->HotKeyCode = (Key::Type)(Key::Ctrl | ((unsigned int)Key::N0 + (hotKey - '0')));
                this->HotKeyOffset = (unsigned char)(p - this->Name);
            }
            text++;
            continue;
        }
        *p = *text;
        p++; text++;
    }
    *p = 0;
    this->NameLength = (unsigned char)(p - this->Name);
    return true;
}
bool ListViewHeader::SetAlign(TextAlignament align)
{
    if ((align == TextAlignament::Left) || (align == TextAlignament::Right) || (align == TextAlignament::Center))
    {
        this->Align = align;
        return true;
    }
    RETURNERROR(false, "align parameter can only be one of the following: TextAlignament::Left, TextAlignament::Right or TextAlignament::Center");
}
void ListViewHeader::SetWidth(unsigned int width)
{
    width = MAXVALUE(width, MINIM_COLUMN_WIDTH);
    width = MINVALUE(width, MAXIM_COLUMN_WIDTH);
    this->Width = (unsigned char)width;
}

ListViewItem::ListViewItem()
{
    this->Flags = 0;
    this->ItemColor = DefaultColorPair;
    this->Height = 1;
    this->Data.UInt64Value = 0;
    this->XOffset = 0;
}

ListViewItem* ListViewControlContext::GetFilteredItem(unsigned int index)
{
    unsigned int idx;
    CHECK(ItemsIndexes.Get(index, idx), nullptr, "Fail to get index value for item with ID: %d", index);
    CHECK(idx < ItemsList.size(), nullptr, "Invalid index (%d)", idx);
    return &ItemsList[idx];
}

void ListViewControlContext::DrawHeader(Console::Renderer & renderer)
{
    auto * defaultCol = &this->Cfg->ListView.ColumnNormal;
    if (!(this->Flags & GATTR_ENABLE))
        defaultCol = &this->Cfg->ListView.ColumnInactive;
    auto * lvCol = defaultCol;


    renderer.DrawHorizontalLine(1, 1, Layout.Width - 2, ' ', defaultCol->Text);

    int		x = 1 - Px;

    ListViewHeader * header = this->H;
    for (unsigned int tr = 0; tr<NrHeaders; tr++, header++)
    {
        if (this->Focused)
        {
            if (tr == sortColumnIndex) {
                lvCol = &this->Cfg->ListView.ColumnSort;
                renderer.DrawHorizontalLineSize(x, 1, header->Width, ' ', lvCol->Text); // highlight the column
            }
            else if (tr == columnHoverOver)
            {
                lvCol = &this->Cfg->ListView.ColumnHover;
                renderer.DrawHorizontalLineSize(x, 1, header->Width, ' ', lvCol->Text); // highlight the column
            }
            else
                lvCol = defaultCol;
        }
        renderer.WriteSingleLineText(x + 1, 1, header->Name, header->Width - 2, lvCol->Text, header->Align, header->NameLength);
        x += header->Width;
        if ((this->Focused) && (tr == sortColumnIndex))
            renderer.WriteSpecialCharacter(x - 1, 1, this->sortAscendent?SpecialChars::TriangleUp:SpecialChars::TriangleDown, lvCol->HotKey);
            	
        if (this->Focused)
        {
            if (((resizeColumnMode) && (tr == columnToResize)) || (tr == columnSeparatorHoverOver))
                renderer.DrawVerticalLineWithSpecialChar(x, 1, Layout.Height, SpecialChars::BoxVerticalSingleLine, Cfg->ListView.ColumnHover.Separator);
            else
                renderer.DrawVerticalLineWithSpecialChar(x, 1, Layout.Height, SpecialChars::BoxVerticalSingleLine, defaultCol->Separator);
        }
        else
            renderer.DrawVerticalLineWithSpecialChar(x, 1, Layout.Height, SpecialChars::BoxVerticalSingleLine, defaultCol->Separator);
        x++;
    }
}
void ListViewControlContext::DrawItem(Console::Renderer & renderer,bool activ, unsigned int index, int y)
{
}

void ListViewControlContext::Paint(Console::Renderer & renderer)
{
    int y = 0;
    auto * lvCol = &this->Cfg->ListView.Normal;
    if (!(this->Flags & GATTR_ENABLE))
        lvCol = &this->Cfg->ListView.Inactive;
    else if (this->Focused)
        lvCol = &this->Cfg->ListView.Focused;
    else if (this->MouseIsOver)
        lvCol = &this->Cfg->ListView.Hover;

    renderer.DrawRectSize(0, 0, this->Layout.Width, this->Layout.Height, lvCol->Border, false);
    renderer.SetClipMargins(1, 1, 1, 1);
    if ((Flags & ListViewFlags::NOHEADERS) == 0) 
    { 
    	DrawHeader(renderer); 
    	y++; 
    }
}
// coloane	
bool ListViewControlContext::AddColumn(const char *text, TextAlignament Align, unsigned int width)
{
	CHECK(text != nullptr, false, "");
	CHECK(NrHeaders < MAX_LISTVIEW_HEADERS, false, "");
    H[NrHeaders].Reset();
    CHECK(H[NrHeaders].SetName(text), false, "Fail to set header name: %s", text);
    CHECK(H[NrHeaders].SetAlign(Align), false, "Fail to set alignament to: %d", Align);
    H[NrHeaders].SetWidth(width);
	NrHeaders++;
    return true;
}

bool ListViewControlContext::DeleteColumn(unsigned int index)
{
    CHECK(index < NrHeaders, false, "Invalid column index: %d (should be smaller than %d)", index, NrHeaders);
	for (unsigned int tr = index; tr<NrHeaders; tr++) 
		H[tr] = H[tr + 1];
	NrHeaders--;
	return true;
}
void ListViewControlContext::DeleteAllColumns()
{
	NrHeaders = 0;
}
int  ListViewControlContext::GetNrColumns()
{
	return NrHeaders;
}

ItemHandle  ListViewControlContext::AddItem(const char *text)
{
    CHECK(text, InvalidItemHandle, "Expecting a valid (non-null) text");
    ItemsList.push_back(ListViewItem());
    ItemHandle idx = ItemsList.size();
    SetItemText(idx, 0, text);
	return idx;
}
bool ListViewControlContext::SetItemText(ItemHandle item, unsigned int subItem,const char *text)
{
    PREPARE_LISTVIEW_ITEM(item, false);
    CHECK(subItem < NrHeaders, false, "Invalid column index (%d), should be smaller than %d", subItem, NrHeaders);
    CHECK(text, false, "Expecting a valid (non-null) text for a subitem value");
    CHECK(i.SubItem[subItem].Set(text), false, "Fail to set text to a sub-item: %s", text);
	return true;
}
const char* ListViewControlContext::GetItemText(ItemHandle item, unsigned int subItem)
{
    PREPARE_LISTVIEW_ITEM(item, nullptr);
    CHECK(subItem < NrHeaders, nullptr, "Invalid column index (%d), should be smaller than %d", subItem, NrHeaders);
	return i.SubItem[subItem].GetText();
}
bool ListViewControlContext::SetItemCheck(ItemHandle item, bool check)
{
    PREPARE_LISTVIEW_ITEM(item, false);
	if (check)
		i.Flags |= ITEM_FLAG_CHECKED;
	else
		i.Flags &= (0xFFFFFFFF - ITEM_FLAG_CHECKED);
	return true;
}
bool ListViewControlContext::SetItemSelect(ItemHandle item, bool check)
{
    PREPARE_LISTVIEW_ITEM(item, false);
    if (check)
        i.Flags |= ITEM_FLAG_SELECTED;
    else
        i.Flags &= (0xFFFFFFFF - ITEM_FLAG_SELECTED);
    return true;
}
bool ListViewControlContext::SetItemColor(ItemHandle item, ColorPair color)
{
    PREPARE_LISTVIEW_ITEM(item, false);
	CHECK((i.Flags >> 16) == (unsigned int)ListViewItemType::REGULAR, false, "Item color only applies to regular item. Use SetItemType to change item type !");
	i.ItemColor = color;
	return true;
}
bool ListViewControlContext::SetItemType(ItemHandle item, ListViewItemType type)
{
    PREPARE_LISTVIEW_ITEM(item, false);
	i.Flags = (i.Flags & 0xFFFF) | (((unsigned int)type)<<16);
	return true;
}
void ListViewControlContext::SetCheckCharacter(int character)
{
	if (character != 0)
		checkCharacter = character;
}
void ListViewControlContext::SetUncheckCharacter(int character)
{
	if (character != 0)
		uncheckCharacter = character;
}
bool ListViewControlContext::SetItemData(ItemHandle item, ItemData Data)
{
    PREPARE_LISTVIEW_ITEM(item, false);
	i.Data = Data;
	return true;
}
ItemData* ListViewControlContext::GetItemData(ItemHandle item)
{
    PREPARE_LISTVIEW_ITEM(item, nullptr);
	return &i.Data;
}
bool ListViewControlContext::SetItemXOffset(ItemHandle item, unsigned int XOffset)
{
    PREPARE_LISTVIEW_ITEM(item, false);	
	i.XOffset = (unsigned int)XOffset;
	return true;
} 
unsigned int  ListViewControlContext::GetItemXOffset(ItemHandle item)
{
    PREPARE_LISTVIEW_ITEM(item, 0);
    return i.XOffset;
}
bool ListViewControlContext::SetItemHeight(ItemHandle item, unsigned int itemHeight)
{
    CHECK(itemHeight > 0, false, "Item height should be bigger than 0");
    PREPARE_LISTVIEW_ITEM(item, false);	
	i.Height = itemHeight;
	return true;
}
unsigned int ListViewControlContext::GetItemHeight(ItemHandle item)
{
    PREPARE_LISTVIEW_ITEM(item, 0);
	return i.Height;
}
void ListViewControlContext::SetClipboardSeparator(char ch)
{
	clipboardSeparator = ch;
}
bool ListViewControlContext::SetColumnClipboardCopyState(unsigned int columnIndex, bool allowCopy)
{
    CHECK(columnIndex < NrHeaders, false, "Invalid column index: %d (should be smaller than %d)", columnIndex, NrHeaders);
	if (allowCopy)
		H[columnIndex].Flags -= H[columnIndex].Flags & COLUMN_DONT_COPY;
	else
		H[columnIndex].Flags |= COLUMN_DONT_COPY;
	return true;
}
bool ListViewControlContext::SetColumnFilterMode(unsigned int columnIndex, bool allowFilterForThisColumn)
{
    CHECK(columnIndex<NrHeaders,false, "Invalid column index (%d), should be smaller than %d", columnIndex, NrHeaders);
	if (allowFilterForThisColumn)
		H[columnIndex].Flags -= H[columnIndex].Flags & COLUMN_DONT_FILTER;
	else
		H[columnIndex].Flags |= COLUMN_DONT_FILTER;
	return true;
}
bool ListViewControlContext::IsItemChecked(ItemHandle item)
{
    PREPARE_LISTVIEW_ITEM(item, false);
    return (bool)((i.Flags & ITEM_FLAG_CHECKED) != 0);
}
bool ListViewControlContext::IsItemSelected(ItemHandle item)
{
    PREPARE_LISTVIEW_ITEM(item, false);
    return (bool)((i.Flags & ITEM_FLAG_SELECTED) != 0);
}
void ListViewControlContext::SelectAllItems()
{
	UpdateSelection(0, ItemsIndexes.Len(), true);
}
void ListViewControlContext::UnSelectAllItems()
{
	UpdateSelection(0, ItemsIndexes.Len(), false);
}
void ListViewControlContext::CheckAllItems()
{
    unsigned int sz = ItemsIndexes.Len();
    unsigned int *indexes = ItemsIndexes.GetUInt32Array();
	if (indexes == nullptr)
		return;
	for (unsigned int tr = 0; tr<sz; tr++,indexes++)
		SetItemCheck(*indexes, true);
}
void ListViewControlContext::UncheckAllItems()
{
	unsigned int sz = ItemsIndexes.Len();
    unsigned int *indexes = ItemsIndexes.GetUInt32Array();
	if (indexes == nullptr)
		return;
	for (unsigned int tr = 0; tr<sz; tr++, indexes++)
		SetItemCheck(*indexes, false);
}
unsigned int ListViewControlContext::GetCheckedItemsCount()
{
	unsigned int count = 0;
    unsigned int sz = ItemsIndexes.Len();
    unsigned int *indexes = ItemsIndexes.GetUInt32Array();
	if (indexes == nullptr)
		return 0;
	for (unsigned int tr = 0; tr < sz; tr++, indexes++)
	{
        if ((*indexes) < sz)
        {
            if ((ItemsList[*indexes].Flags & ITEM_FLAG_CHECKED) != 0)
                count++;
        }
	}

	return count;
}
void ListViewControlContext::SetSelectionColor(unsigned int color)
{
	selectionColor = color;
}

bool ListViewControlContext::SetCurrentIndex(ItemHandle item)
{
    CHECK((unsigned int)item < ItemsIndexes.Len(), false, "Invalid index: %d (should be smaller than %d)", item, ItemsIndexes.Len());
	MoveTo((int)item);
	return true;
}
int	 ListViewControlContext::GetFirstVisibleLine()
{
	return Py;
}
bool ListViewControlContext::SetFirstVisibleLine(ItemHandle item)
{
	if (SetCurrentIndex(item) == false)
		return false;
	Py = CurentItemIndex;
	return true;
}
void ListViewControlContext::DeleteAllItems()
{
	ItemsList.clear();
	ItemsIndexes.Clear();
	Px = 0;
	Py = 0;
	CurentItemIndex = 0;
	searchMode = false;
	searchString.Set("");
}
// movement
int  ListViewControlContext::GetVisibleItemsCount()
{
	int vis = Layout.Height - 3;
	if ((Flags & ListViewFlags::NOHEADERS) != 0)
		vis++;
	int dim = 0, poz = Py, nrItems = 0;
	int sz = (int)ItemsIndexes.Len();
	while ((dim<vis) && (poz<sz))
	{
		ListViewItem *i = GetFilteredItem(poz);
		if (i)
			dim += i->Height;
		if ((Flags & ListViewFlags::ITEMSEPARATORS) != 0)
			dim++;
		nrItems++;
		poz++;
	}
	return nrItems;
}
void ListViewControlContext::UpdateSelection(int start, int end, bool select)
{
	ListViewItem	*i;
	int		totalItems = ItemsIndexes.Len();

	while ((start != end) && (start >= 0) && (start<totalItems))
	{
		i = GetFilteredItem(start);
		if (i != nullptr) {
			if (select)
				i->Flags |= ITEM_FLAG_SELECTED;
			else
				i->Flags &= (0xFFFFFFFF - ITEM_FLAG_SELECTED);
		}
		if (start<end)
			start++;
		else
			start--;
	}
}
void ListViewControlContext::MoveTo(int index)
{
	int count = ItemsIndexes.Len();
	if (count <= 0)
		return;
	if (index >= count)
		index = count - 1;
	if (index<0) 
		index = 0;
	if (index == CurentItemIndex) 
		return;
	int vis = GetVisibleItemsCount();
	int rel = index - Py;
	int originalPoz = CurentItemIndex;
	CurentItemIndex = index;
	if (rel < 0) 
		Py = index;
	if (rel >= vis) 
		Py = (index - vis) + 1;
	if (originalPoz != index) 
		SendMsg(Event::EVENT_LISTVIEW_CURRENTITEM_CHANGED);
}
bool ListViewControlContext::OnKeyEvent(AppCUI::Input::Key::Type keyCode, char AsciiCode)
{
	Utils::String	temp;
	int				tr, gr;
	ListViewItem	*lvi;
	bool			selected;

	if (resizeColumnMode)
	{
		searchMode = false;
		switch (keyCode)
		{
			case Key::Ctrl | Key::Right: 
                columnToResize++;
				if (columnToResize >= NrHeaders)
                    columnToResize = 0;
				return true;
			case Key::Ctrl | Key::Left: 
                if (columnToResize>0)
                    columnToResize--;
                else 
                    columnToResize = NrHeaders - 1;
				return true;
			case Key::Left: 
                H[columnToResize].SetWidth(((unsigned int)H[columnToResize].Width) - 1);
				return true;
			case Key::Right: 
                H[columnToResize].SetWidth(((unsigned int)H[columnToResize].Width) + 1);
				return true;
		};
		if ((AsciiCode>0) || (keyCode>0))
		{
			resizeColumnMode = false;
            columnToResize = INVALID_COLUMN_INDEX;
			return true;
		}
	}
	else {
		if (Flags & ListViewFlags::ALLOWSELECTION)
		{
			lvi = GetFilteredItem(CurentItemIndex);
			if (lvi != nullptr)
				selected = ((lvi->Flags & ITEM_FLAG_SELECTED) != 0);
			else
				selected = false;
			switch (keyCode)
			{
			case Key::Shift | Key::Up:
				UpdateSelection(CurentItemIndex, CurentItemIndex - 1, !selected);
				MoveTo(CurentItemIndex - 1);
				searchMode = false;
				SendMsg(Event::EVENT_LISTVIEW_SELECTION_CHANGED);
				return true;
			case Key::Down | Key::Shift:
				UpdateSelection(CurentItemIndex, CurentItemIndex + 1, !selected);
				MoveTo(CurentItemIndex + 1);
				searchMode = false;
				SendMsg(Event::EVENT_LISTVIEW_SELECTION_CHANGED);
				return true;
			case Key::PageUp | Key::Shift:
				UpdateSelection(CurentItemIndex, CurentItemIndex - GetVisibleItemsCount(), !selected);
				MoveTo(CurentItemIndex - GetVisibleItemsCount());
				searchMode = false;
				SendMsg(Event::EVENT_LISTVIEW_SELECTION_CHANGED);
				return true;
			case Key::PageDown | Key::Shift:
				UpdateSelection(CurentItemIndex, CurentItemIndex + GetVisibleItemsCount(), !selected);
				MoveTo(CurentItemIndex + GetVisibleItemsCount());
				searchMode = false;
				SendMsg(Event::EVENT_LISTVIEW_SELECTION_CHANGED);
				return true;
			case Key::Home | Key::Shift:
				UpdateSelection(CurentItemIndex, 0, !selected);
				MoveTo(0);
				searchMode = false;
				SendMsg(Event::EVENT_LISTVIEW_SELECTION_CHANGED);
				return true;
			case Key::End | Key::Shift:
				UpdateSelection(CurentItemIndex, ItemsIndexes.Len(), !selected);
				MoveTo(ItemsIndexes.Len());
				searchMode = false;
				SendMsg(Event::EVENT_LISTVIEW_SELECTION_CHANGED);
				return true;
			};
		}
		switch (keyCode)
		{
		case Key::Up: MoveTo(CurentItemIndex - 1); searchMode = false; return true;
		case Key::Down: MoveTo(CurentItemIndex + 1); searchMode = false; return true;
		case Key::PageUp: MoveTo(CurentItemIndex - GetVisibleItemsCount()); searchMode = false; return true;
		case Key::PageDown: MoveTo(CurentItemIndex + GetVisibleItemsCount()); searchMode = false; return true;
		case Key::Left: if (Px>0) Px--; searchMode = false; return true;
		case Key::Right: Px++; searchMode = false; return true;
		case Key::Home: MoveTo(0); searchMode = false; return true;
		case Key::End: MoveTo(((int)ItemsIndexes.Len()) - 1); searchMode = false; return true;
		case Key::Backspace:
			searchMode = true;
			if (searchString.Len()>0)
			{
				searchString.Truncate(searchString.Len() - 1);
				UpdateSearch(0);
			}
			return true;
		case Key::Space:
			if (!searchMode)
			{
				if (!(Flags & ListViewFlags::HASCHECKBOX))
					return false;
				lvi = GetFilteredItem(CurentItemIndex);
				if (lvi != nullptr)
				{
					if ((lvi->Flags & ITEM_FLAG_CHECKED) != 0)
						lvi->Flags -= ITEM_FLAG_CHECKED;
					else
						lvi->Flags |= ITEM_FLAG_CHECKED;
				}
				SendMsg(Event::EVENT_LISTVIEW_ITEM_CHECKED);
			}
			else
			{
				searchString.AddChar(' '); UpdateSearch(0);
			}
			return true;
		case Key::Enter | Key::Ctrl:
			if (searchMode)
			{
				UpdateSearch(CurentItemIndex + 1);
				return true; // de vazut daca are send
			}
			return false;
		case Key::Enter:
			SendMsg(Event::EVENT_LISTVIEW_ITEM_CLICKED);
			return true;
		case Key::Escape:
			if (searchMode)
			{
				if (searchString.Len() > 0)
				{
					searchString.Truncate(0);
					UpdateSearch(0);
					return true;
				}
				else {
					searchMode = false;
					return true;
				}
			}
			if ((Flags & ListViewFlags::SEARCHMODE) == 0)
			{
				if (searchString.Len() > 0)
				{
					searchString.Truncate(0);
					UpdateSearch(0);
					return true;
				}
			}
			return false;

		case Key::Ctrl | Key::Right:
		case Key::Ctrl | Key::Left: 
            columnToResize = 0;
			resizeColumnMode = true; 
			return true;
		case Key::Ctrl | Key::C:
		case Key::Ctrl | Key::Insert:
			temp.Create(256);
			if (ItemsIndexes.Len()>0)
			{
				for (tr = 0; tr<NrHeaders; tr++)
				{
					if ((H[tr].Flags & COLUMN_DONT_COPY) == 0)
					{
						temp.Add(GetFilteredItem(CurentItemIndex)->SubItem[tr].GetText());
						if (clipboardSeparator != 0) temp.AddChar(clipboardSeparator);
					}
				}
				AppCUI::OS::Clipboard::SetText(temp);
			}
			return true;
		case Key::Ctrl | Key::Alt | Key::Insert:
			temp.Create(4096);
			for (gr = 0; gr<(int)ItemsIndexes.Len(); gr++)
			{
				for (tr = 0; tr<NrHeaders; tr++)
				{
					if ((H[tr].Flags & COLUMN_DONT_COPY) == 0)
					{
						temp.Add(GetFilteredItem(gr)->SubItem[tr].GetText());
						if (clipboardSeparator != 0) temp.AddChar(clipboardSeparator);
					}
				}
				temp.Add("\r\n");
			}
			AppCUI::OS::Clipboard::SetText(temp);
			return true;
		};
		// caut sort
		if (searchMode == false)
		{
			for (int tr = 0; tr < NrHeaders; tr++)
			{
				if (H[tr].HotKeyCode == keyCode)
				{
					ColumnSort(tr);
					return true;
				}
			}
		}
		// search mode
		if (AsciiCode>0)
		{
			searchMode = true;
			searchString.AddChar(AsciiCode);
			UpdateSearch(0);
			return true;
		}
	}
	return false;
}
bool ListViewControlContext::MouseToHeader(int x, int y, unsigned int &HeaderIndex, unsigned int &HeaderColumnIndex)
{
    int xx = 1 - Px;
    ListViewHeader * header = this->H;
    for (unsigned int tr = 0; tr < NrHeaders; tr++,header++)
    {
        if ((x >= xx) && (x <= xx + header->Width) && (x > 1) && (x < (this->Layout.Width-2)))
        {            
            if (x == (xx + header->Width)) {
                HeaderColumnIndex = tr;
                HeaderIndex = INVALID_COLUMN_INDEX;
            } else {
                HeaderColumnIndex = INVALID_COLUMN_INDEX;
                HeaderIndex = tr;
            }
            return true;
        }
        xx += header->Width;
        xx++;
    }
    HeaderColumnIndex = INVALID_COLUMN_INDEX;
    HeaderIndex = INVALID_COLUMN_INDEX;
    return false;
}
void ListViewControlContext::OnMouseReleased(int x, int y, int butonState)
{
	resizeColumnMode = false;
	columnToResize = INVALID_COLUMN_INDEX;
    columnSeparatorHoverOver = INVALID_COLUMN_INDEX;
    columnHoverOver = INVALID_COLUMN_INDEX;
	if (((Flags & ListViewFlags::NOHEADERS) == 0) && (y == 1))
	{
        unsigned int hIndex, hColumn;
        if ((MouseToHeader(x, y, hIndex, hColumn)) && (hIndex != INVALID_COLUMN_INDEX))
            ColumnSort(hIndex);
	}
	else
	{
		if ((Flags & ListViewFlags::NOHEADERS) != 0) 
			y--; 
		else 
			y -= 2;
		if (y<GetVisibleItemsCount()) 
			MoveTo(y + Py);
	}
}
void ListViewControlContext::OnMousePressed(int x, int y, int butonState)
{

}
bool ListViewControlContext::OnMouseDrag(int x, int y, int butonState)
{
    if (columnSeparatorHoverOver != INVALID_COLUMN_INDEX)
    {
        int xx = 1 - Px;
        ListViewHeader * header = this->H;
        for (unsigned int tr = 0; tr < columnSeparatorHoverOver; tr++, header++)
            xx += (((unsigned int)header->Width)+1);
        // xx = the start of column
        if (x > xx)
            header->SetWidth((unsigned int)(x - xx));
        return true;
    }
    return false;
}
bool ListViewControlContext::OnMouseOver(int x, int y)
{
    if ((Flags & ListViewFlags::NOHEADERS) == 0)
    {
        unsigned int hIndex, hColumn;
        MouseToHeader(x, y, hIndex, hColumn);
        if ((hIndex != columnHoverOver) && (y == 1) && (Flags & ListViewFlags::SORTCOLUMNS))
        {
            columnHoverOver = hIndex;
            columnSeparatorHoverOver = INVALID_COLUMN_INDEX;
            return true;
        }
        if (hColumn != columnSeparatorHoverOver)
        {
            columnHoverOver = INVALID_COLUMN_INDEX;
            columnSeparatorHoverOver = hColumn;
            return true;
        }
    }
    return false;
}
// sort
void ListViewControlContext::SetSortColumn(unsigned int colIndex)
{    
    if (colIndex >= NrHeaders)
		this->sortColumnIndex = INVALID_COLUMN_INDEX;
	else 
		this->sortColumnIndex = colIndex;
	if ((Flags & ListViewFlags::SORTCOLUMNS) == 0)
		this->sortColumnIndex = INVALID_COLUMN_INDEX;
}
//void ListViewControlContext::SetSortOrder(int direction)
//{
//	if (direction>0) sortOrder = 1; else sortOrder = -1;
//}
//void ListViewControlContext::SetCompareFunction(int(*compareFnc)(ListViewControlContext::Item *i1, ListViewControlContext::Item *i2, int columnIndex))
//{
//	itemCompare = compareFnc;
//}
//void ListViewControlContext::CopyItem(ListViewItem *dest, ListViewItem *source)
//{
//	dest->Flags = source->Flags;
//	dest->Color = source->Color;
//	dest->Data = source->Data;
//	dest->XOffset = source->XOffset;
//	for (int tr = 0; tr<NrHeaders; tr++)
//		dest->SubItem[tr] = source->SubItem[tr];
//}
void ListViewControlContext::ColumnSort(unsigned int columnIndex)
{
	if ((Flags & ListViewFlags::SORTCOLUMNS) == 0)
	{ 
		this->sortColumnIndex = INVALID_COLUMN_INDEX;
		return; 
	}
	if (columnIndex != sortColumnIndex) 
		SetSortColumn(columnIndex); 
	else 
        sortAscendent = !sortAscendent;
	Sort();
}
//----------------------

int  SortIndexesCompareFunction(unsigned int indx1, unsigned int indx2, void *context)
{
	ListViewControlContext *lvcc = (ListViewControlContext*)context;
	if (lvcc->sortFunction)
	{
		return lvcc->sortFunction((ListView*)lvcc->Host, indx1, indx2, lvcc->sortColumnIndex, lvcc->sortFunctionContext);
	}
	else {
        unsigned int itemsCount = lvcc->ItemsList.size();
        if ((indx1 < itemsCount) && (indx2 < itemsCount) && (lvcc->sortColumnIndex != INVALID_COLUMN_INDEX))
        {
            return lvcc->ItemsList[indx1].SubItem[lvcc->sortColumnIndex].CompareWith(lvcc->ItemsList[indx2].SubItem[lvcc->sortColumnIndex].GetText(), true);
        }
        else {
            if (indx1 < indx2)
                return -1;
            else if (indx1 > indx2)
                return 1;
            else
                return 0;
        }
	}
}
bool ListViewControlContext::Sort()
{
	if (sortFunction == nullptr)
	{
		// sanity check
		CHECK(sortColumnIndex < NrHeaders, false, "No sort column or custom sort function defined !");
	}
	ItemsIndexes.Sort(SortIndexesCompareFunction, sortAscendent, this);
	return true;
}
int  ListViewControlContext::SearchItem(int startPoz, unsigned int colIndex)
{
	unsigned int	originalStartPoz;
	ListViewItem	*i;

	unsigned int count = ItemsIndexes.Len();
	if (startPoz >= count) 
		startPoz = 0;
	if (count == 0)
		return -1;
	originalStartPoz = startPoz;

	do
	{
		if ((i = GetFilteredItem(startPoz)) != nullptr)
		{
			if (i->SubItem[colIndex].Contains(searchString, true))
				return startPoz;
		}
		startPoz++;
		if (startPoz >= count) 
			startPoz = 0;

	} while (startPoz != originalStartPoz);
	return -1;
}
void ListViewControlContext::FilterItems()
{
	ItemsIndexes.Clear();
    unsigned int count = ItemsList.size();
	for (unsigned int tr = 0; tr < count; tr++)
	{
		bool isOK = false;
        ListViewItem& lvi = ItemsList[tr];
		for (unsigned int gr = 0; gr < NrHeaders; gr++)
		{
			if ((H[gr].Flags & COLUMN_DONT_FILTER) != 0)
				continue;
			if (lvi.SubItem[gr].Contains(this->searchString, true))
			{
				isOK = true;
				break;
			}
		}
		if (isOK)
			ItemsIndexes.Push(tr);
	}
	this->Py = 0;
	this->CurentItemIndex = 0;
	SendMsg(Event::EVENT_LISTVIEW_CURRENTITEM_CHANGED);
}
void ListViewControlContext::UpdateSearch(int startPoz)
{
	int index;
	int	cCol = 0;

	// daca fac filtrare
	if ((Flags & ListViewFlags::SEARCHMODE) == 0)
	{
		FilterItems();
	}
	else 
	{
		if (sortColumnIndex < NrHeaders)
			cCol = sortColumnIndex;

		if ((index = SearchItem(startPoz, cCol)) >= 0)
		{
			MoveTo(index);
		}
		else
		{
			if (searchString.Len()>0)
				searchString.Truncate(searchString.Len() - 1);
		}
	}
}
void ListViewControlContext::SendMsg(Event::Type eventType)
{
	Host->RaiseEvent(eventType);
}

//=====================================================================================================
ListView::~ListView()
{
	DeleteAllItems();
	DeleteAllColumns();
	DELETE_CONTROL_CONTEXT(ListViewControlContext);
}
bool		ListView::Create(Control *parent, const char * layout, ListViewFlags flags)
{
    CONTROL_INIT_CONTEXT(ListViewControlContext);
    CREATE_TYPECONTROL_CONTEXT(ListViewControlContext, Members, false);
    Members->Layout.MinWidth = 5;
    Members->Layout.MinHeight = 3;
	CHECK(Init(parent, "", layout, false), false, "Failed to create list view !");	
	Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | GATTR_HSCROLL | GATTR_VSCROLL | (unsigned int)flags;
    Members->ScrollBars.LeftMargin = 25;
    // allocate items
    CHECK(Members->ItemsIndexes.Create(32), false, "Fail to allocate indexes");
    Members->ItemsList.reserve(32);
    
    // initialize
	Members->NrHeaders = 0;
	Members->Px = 0;
	Members->Py = 0;
	Members->CurentItemIndex = 0;
    
	Members->sortColumnIndex = INVALID_COLUMN_INDEX;
    Members->columnHoverOver = INVALID_COLUMN_INDEX;
    Members->columnSeparatorHoverOver = INVALID_COLUMN_INDEX;
	Members->sortAscendent = true;
	Members->sortFunction = nullptr;
	Members->sortFunctionContext = nullptr;
	Members->resizeColumnMode = false;
	Members->searchMode = false;
	Members->columnToResize = INVALID_COLUMN_INDEX;
	Members->clipboardSeparator = '\t';
    Members->checkCharacter = 'v';
	Members->uncheckCharacter = 'x';
	//Members->selectionColor = SC(0, 8);
	Members->Host = this;
	Members->searchString.Create(Members->searchStringData, MAX_LISTVIEW_SEARCH_STRING, true);
	Members->statusString.Create(Members->statusStringData, 20, true);
	// all is good
	return true;
}
void		ListView::Paint(Console::Renderer & renderer)
{
	CREATE_TYPECONTROL_CONTEXT(ListViewControlContext, Members, );
	Members->Paint(renderer);
}
bool		ListView::OnKeyEvent(AppCUI::Input::Key::Type keyCode, char AsciiCode)
{
	return WRAPPER->OnKeyEvent(keyCode, AsciiCode);
}
void        ListView::OnUpdateScrollBars()
{
    CREATE_TYPECONTROL_CONTEXT(ListViewControlContext, Members, );
    //GDT: must be precomputed (cached)
    unsigned int sum = 0;
    for (unsigned int tr = 0; tr < Members->NrHeaders; tr++)
        sum += ((unsigned int)(Members->H[tr].Width)) + 1;
    UpdateHScrollBar(Members->Px, sum);
    
}

bool		ListView::AddColumn(const char *text, TextAlignament Align, unsigned int Size)
{
	return WRAPPER->AddColumn(text, Align, Size);
}
bool		ListView::SetColumnText(unsigned int columnIndex, const char *text)
{
    CHECK(columnIndex < WRAPPER->NrHeaders, false, "Invalid column index:%d (should be smaller than %d)", columnIndex, WRAPPER->NrHeaders);
    return WRAPPER->H[columnIndex].SetName(text);
}
bool		ListView::SetColumnAlignament(unsigned int columnIndex, TextAlignament Align)
{
    CHECK(columnIndex < WRAPPER->NrHeaders, false, "Invalid column index:%d (should be smaller than %d)", columnIndex, WRAPPER->NrHeaders);
    return WRAPPER->H[columnIndex].SetAlign(Align);
}
bool		ListView::SetColumnWidth(unsigned int columnIndex, unsigned int width)
{
    CHECK(columnIndex < WRAPPER->NrHeaders, false, "Invalid column index:%d (should be smaller than %d)",columnIndex,WRAPPER->NrHeaders);
    WRAPPER->H[columnIndex].SetWidth(width);
    return true;
}
bool		ListView::SetColumnClipboardCopyState(unsigned int columnIndex, bool allowCopy)
{
	return WRAPPER->SetColumnClipboardCopyState(columnIndex, allowCopy);
}
bool		ListView::SetColumnFilterMode(unsigned int columnIndex, bool allowFilterForThisColumn)
{
	return WRAPPER->SetColumnFilterMode(columnIndex, allowFilterForThisColumn);
}
bool		ListView::DeleteColumn(unsigned int columnIndex)
{
	return WRAPPER->DeleteColumn(columnIndex);
}
void		ListView::DeleteAllColumns()
{
	if (Context != nullptr) {
		WRAPPER->DeleteAllColumns();
	}
}
unsigned int ListView::GetColumnsCount()
{
	return WRAPPER->GetNrColumns();
}

ItemHandle ListView::AddItem(const char *text)
{
	return WRAPPER->AddItem(text);
}
ItemHandle ListView::AddItem(const char *text, const char * subItem1)
{
	int handle = WRAPPER->AddItem(text);
	CHECK(handle != InvalidItemHandle, InvalidItemHandle, "Fail to allocate item for ListView");
	CHECK(WRAPPER->SetItemText(handle, 1, subItem1), -1, "");
	return handle;
}
ItemHandle ListView::AddItem(const char *text, const char * subItem1, const char * subItem2)
{
    ItemHandle handle = WRAPPER->AddItem(text);
    CHECK(handle != InvalidItemHandle, InvalidItemHandle, "Fail to allocate item for ListView");
	CHECK(WRAPPER->SetItemText(handle, 1, subItem1), -1, "");
	CHECK(WRAPPER->SetItemText(handle, 2, subItem2), -1, "");
	return handle;
}
ItemHandle ListView::AddItem(const char *text, const char * subItem1, const char * subItem2, const char * subItem3)
{
    ItemHandle handle = WRAPPER->AddItem(text);
    CHECK(handle != InvalidItemHandle, InvalidItemHandle, "Fail to allocate item for ListView");
	CHECK(WRAPPER->SetItemText(handle, 1, subItem1), -1, "");
	CHECK(WRAPPER->SetItemText(handle, 2, subItem2), -1, "");
	CHECK(WRAPPER->SetItemText(handle, 3, subItem3), -1, "");
	return handle;
}
ItemHandle ListView::AddItem(const char *text, const char * subItem1, const char * subItem2, const char * subItem3, const char * subItem4)
{
    ItemHandle handle = WRAPPER->AddItem(text);
    CHECK(handle != InvalidItemHandle, InvalidItemHandle, "Fail to allocate item for ListView");
	CHECK(WRAPPER->SetItemText(handle, 1, subItem1), -1, "");
	CHECK(WRAPPER->SetItemText(handle, 2, subItem2), -1, "");
	CHECK(WRAPPER->SetItemText(handle, 3, subItem3), -1, "");
	CHECK(WRAPPER->SetItemText(handle, 4, subItem4), -1, "");
	return handle;
}
ItemHandle ListView::AddItem(const char *text, const char * subItem1, const char * subItem2, const char * subItem3, const char * subItem4, const char * subItem5)
{
    ItemHandle handle = WRAPPER->AddItem(text);
    CHECK(handle != InvalidItemHandle, InvalidItemHandle, "Fail to allocate item for ListView");
	CHECK(WRAPPER->SetItemText(handle, 1, subItem1), -1, "");
	CHECK(WRAPPER->SetItemText(handle, 2, subItem2), -1, "");
	CHECK(WRAPPER->SetItemText(handle, 3, subItem3), -1, "");
	CHECK(WRAPPER->SetItemText(handle, 4, subItem4), -1, "");
	CHECK(WRAPPER->SetItemText(handle, 5, subItem5), -1, "");
	return handle;
}

bool		ListView::SetItemText(ItemHandle item, unsigned int subItem, const char *text)
{
	return WRAPPER->SetItemText(item, subItem, text);
}
const char*	ListView::GetItemText(ItemHandle item, unsigned int subItemIndex)
{
	return WRAPPER->GetItemText(item, subItemIndex);
}
bool		ListView::SetItemCheck(ItemHandle item, bool check)
{
	return WRAPPER->SetItemCheck(item, check);
}
bool		ListView::SetItemSelect(ItemHandle item, bool select)
{
	return WRAPPER->SetItemSelect(item, select);
}
bool		ListView::SetItemColor(ItemHandle item, ColorPair col)
{
	return WRAPPER->SetItemColor(item, col);
}
bool		ListView::SetItemType(ItemHandle item, ListViewItemType type)
{
	return WRAPPER->SetItemType(item, type);
}
bool		ListView::IsItemChecked(ItemHandle item)
{
	return WRAPPER->IsItemChecked(item);
}
bool		ListView::IsItemSelected(ItemHandle item)
{
	return WRAPPER->IsItemSelected(item);
}
bool		ListView::SetItemData(ItemHandle item, ItemData Data)
{
	return WRAPPER->SetItemData(item, Data);
}
ItemData*	ListView::GetItemData(ItemHandle item)
{
	return WRAPPER->GetItemData(item);
}
bool		ListView::SetItemXOffset(ItemHandle item, unsigned int XOffset)
{
	return WRAPPER->SetItemXOffset(item, XOffset);
}
unsigned int ListView::GetItemXOffset(ItemHandle item)
{
	return WRAPPER->GetItemXOffset(item);
}
bool		ListView::SetItemHeight(ItemHandle item, unsigned int Height)
{
	return WRAPPER->SetItemHeight(item, Height);
}
unsigned int ListView::GetItemHeight(ItemHandle item)
{
	return WRAPPER->GetItemHeight(item);
}
void		ListView::DeleteAllItems()
{
	if (Context != nullptr)
	{
		WRAPPER->DeleteAllItems();
	}
}
unsigned int ListView::GetItemsCount()
{
	if (Context != nullptr) {
		return (unsigned int)WRAPPER->ItemsList.size();
	}
	return 0;
}
unsigned int ListView::GetCurrentItem()
{
	ListViewControlContext * lvcc = ((ListViewControlContext*)this->Context);
	if (lvcc->CurentItemIndex < 0)
		return -1;
    unsigned int *indexes = lvcc->ItemsIndexes.GetUInt32Array();
	return indexes[lvcc->CurentItemIndex];

}
bool		ListView::SetCurrentItem(ItemHandle item)
{
	ListViewControlContext * lvcc = ((ListViewControlContext*)this->Context);
    unsigned int *indexes = lvcc->ItemsIndexes.GetUInt32Array();
	unsigned int count = lvcc->ItemsIndexes.Len();
	if (count <= 0)
		return false;
	// caut indexul
	for (int tr = 0; tr < count; tr++, indexes++)
	{
		if ((*indexes) == item)
			return WRAPPER->SetCurrentIndex(tr);
	}
	return false;
}
void		ListView::SelectAllItems()
{
	WRAPPER->SelectAllItems();
}
void		ListView::UnSelectAllItems()
{
	WRAPPER->UnSelectAllItems();
}
void		ListView::CheckAllItems()
{
	WRAPPER->CheckAllItems();
}
void		ListView::UncheckAllItems()
{
	WRAPPER->UncheckAllItems();
}
unsigned int ListView::GetCheckedItemsCount()
{
	return WRAPPER->GetCheckedItemsCount();
}

void		ListView::SetSelectionColor(int color)
{
	WRAPPER->SetSelectionColor(color);
}
void		ListView::SetClipboardSeparator(char ch)
{
	WRAPPER->SetClipboardSeparator(ch);
}
void		ListView::SetCheckCharacter(int character)
{
	WRAPPER->SetCheckCharacter(character);
}
void		ListView::SetUncheckCharacter(int character)
{
	WRAPPER->SetUncheckCharacter(character);
}
void		ListView::OnMouseReleased(int x, int y, int butonState)
{
	WRAPPER->OnMouseReleased(x, y, butonState);
}
void	    ListView::OnMousePressed(int x, int y, int butonState)
{
    WRAPPER->OnMousePressed(x, y, butonState);
}
bool	    ListView::OnMouseDrag(int x, int y, int butonState)
{
    return WRAPPER->OnMouseDrag(x, y, butonState);
}
bool        ListView::OnMouseOver(int x, int y)
{
    return WRAPPER->OnMouseOver(x, y);
}
bool        ListView::OnMouseLeave()
{
    WRAPPER->sortColumnIndex = INVALID_COLUMN_INDEX;
    WRAPPER->columnHoverOver = INVALID_COLUMN_INDEX;
    return true;
}
void		ListView::OnFocus()
{
    WRAPPER->sortColumnIndex = INVALID_COLUMN_INDEX;
    WRAPPER->columnHoverOver = INVALID_COLUMN_INDEX;
	WRAPPER->searchMode = false;
}
void		ListView::SetItemCompareFunction(Handlers::ListViewItemComparer fnc, void *Context)
{
	WRAPPER->sortFunction = fnc;
	WRAPPER->sortFunctionContext = Context;
}
bool		ListView::Sort()
{
	return WRAPPER->Sort();
}
bool		ListView::Sort(unsigned int columnIndex, bool ascendent)
{
    CHECK(columnIndex < WRAPPER->NrHeaders, false, "Invalid column index (%d). Should be smaller than %d", columnIndex, WRAPPER->NrHeaders);
    CHECK(WRAPPER->Flags & ListViewFlags::SORTCOLUMNS, false, "Can not sort items (have you set 'ListViewFlags::SORTCOLUMNS' when creating ListView ?)");
	WRAPPER->sortColumnIndex = columnIndex;
    WRAPPER->sortAscendent = ascendent;
	return WRAPPER->Sort();
}
