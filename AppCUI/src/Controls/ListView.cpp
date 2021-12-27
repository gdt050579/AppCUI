#include "ControlContext.hpp"

namespace AppCUI
{
constexpr uint32 ITEM_FLAG_CHECKED         = 0x0001;
constexpr uint32 ITEM_FLAG_SELECTED        = 0x0002;
constexpr uint32 COLUMN_DONT_COPY          = 1;
constexpr uint32 COLUMN_DONT_FILTER        = 2;
constexpr uint32 INVALID_COLUMN_INDEX      = 0xFFFFFFFF;
constexpr uint32 MINIM_COLUMN_WIDTH        = 3;
constexpr uint32 MAXIM_COLUMN_WIDTH        = 256;
constexpr uint32 NO_HOTKEY_FOR_COLUMN      = 0xFFFFFFFF;
constexpr uint32 LISTVIEW_SEARCH_BAR_WIDTH = 12;

#define PREPARE_LISTVIEW_ITEM(index, returnValue)                                                                      \
    CHECK(index < Items.List.size(), returnValue, "Invalid index: %d", index);                                         \
    ListViewItem& i = Items.List[index];

#define CONST_PREPARE_LISTVIEW_ITEM(index, returnValue)                                                                \
    CHECK(index < Items.List.size(), returnValue, "Invalid index: %d", index);                                         \
    const ListViewItem& i = Items.List[index];

#define WRAPPER ((ListViewControlContext*) this->Context)

Graphics::CharacterBuffer
      __temp_listviewitem_reference_object__; // use this as std::option<const T&> is not available yet

void ListViewColumn::Reset()
{
    this->HotKeyCode   = Key::None;
    this->HotKeyOffset = NO_HOTKEY_FOR_COLUMN;
    this->Flags        = 0;
    this->Align        = TextAlignament::Left;
    this->Width        = 10;
    this->Name.Clear();
}
bool ListViewColumn::SetName(const ConstString& text)
{
    this->HotKeyCode   = Key::None;
    this->HotKeyOffset = NO_HOTKEY_FOR_COLUMN;

    CHECK(Name.SetWithHotKey(text, this->HotKeyOffset, this->HotKeyCode, Key::Ctrl),
          false,
          "Fail to set name to column !");

    return true;
}
bool ListViewColumn::SetAlign(TextAlignament align)
{
    if ((align == TextAlignament::Left) || (align == TextAlignament::Right) || (align == TextAlignament::Center))
    {
        this->Align = align;
        return true;
    }
    RETURNERROR(
          false,
          "align parameter can only be one of the following: TextAlignament::Left, TextAlignament::Right or "
          "TextAlignament::Center");
}
void ListViewColumn::SetWidth(uint32 width)
{
    width       = std::max<>(width, MINIM_COLUMN_WIDTH);
    width       = std::min<>(width, MAXIM_COLUMN_WIDTH);
    this->Width = (uint8) width;
}

ListViewItem::ListViewItem() : Data(nullptr)
{
    this->Flags     = 0;
    this->Type      = ListViewItemType::Normal;
    this->ItemColor = DefaultColorPair;
    this->Height    = 1;
    this->XOffset   = 0;
}
ListViewItem::ListViewItem(const ListViewItem& obj) : Data(obj.Data)
{
    this->Flags     = obj.Flags;
    this->Type      = obj.Type;
    this->ItemColor = obj.ItemColor;
    this->Height    = obj.Height;
    this->XOffset   = obj.XOffset;
    for (uint32 tr = 0; tr < MAX_LISTVIEW_COLUMNS; tr++)
    {
        this->SubItem[tr] = obj.SubItem[tr];
    }
}
ListViewItem::ListViewItem(ListViewItem&& obj) noexcept : Data(obj.Data)
{
    this->Flags     = obj.Flags;
    this->Type      = obj.Type;
    this->ItemColor = obj.ItemColor;
    this->Height    = obj.Height;
    this->XOffset   = obj.XOffset;
    for (uint32 tr = 0; tr < MAX_LISTVIEW_COLUMNS; tr++)
    {
        this->SubItem[tr] = obj.SubItem[tr];
    }
}

ListViewItem* ListViewControlContext::GetFilteredItem(uint32 index)
{
    uint32 idx;
    CHECK(Items.Indexes.Get(index, idx), nullptr, "Fail to get index value for item with ID: %d", index);
    CHECK(idx < Items.List.size(), nullptr, "Invalid index (%d)", idx);
    return &Items.List[idx];
}

void ListViewControlContext::DrawColumnSeparatorsForResizeMode(Graphics::Renderer& renderer)
{
    int x                  = 1 - Columns.XOffset;
    ListViewColumn* column = this->Columns.List;
    for (uint32 tr = 0; (tr < Columns.Count) && (x < (int) this->Layout.Width); tr++, column++)
    {
        x += column->Width;
        if (((Columns.ResizeModeEnabled) && (tr == Columns.ResizeColumnIndex)) ||
            (tr == Columns.HoverSeparatorColumnIndex))
            renderer.DrawVerticalLine(x, 1, Layout.Height, Cfg->ListView.ColumnHover.Separator);
        x++;
    }
}
void ListViewControlContext::DrawColumn(Graphics::Renderer& renderer)
{
    auto* defaultCol = &this->Cfg->ListView.ColumnNormal;
    if (!(this->Flags & GATTR_ENABLE))
        defaultCol = &this->Cfg->ListView.ColumnInactive;
    auto* lvCol = defaultCol;

    renderer.FillHorizontalLine(1, 1, Layout.Width - 2, ' ', defaultCol->Text);

    int x = 1 - Columns.XOffset;

    ListViewColumn* column = this->Columns.List;
    WriteTextParams params(WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth | WriteTextFlags::OverwriteColors);
    params.Y = 1;

    for (uint32 tr = 0; (tr < Columns.Count) && (x < (int) this->Layout.Width); tr++, column++)
    {
        if (this->Focused)
        {
            if (tr == SortParams.ColumnIndex)
            {
                lvCol = &this->Cfg->ListView.ColumnSort;
                renderer.FillHorizontalLineSize(x, 1, column->Width, ' ', lvCol->Text); // highlight the column
            }
            else if (tr == Columns.HoverColumnIndex)
            {
                lvCol = &this->Cfg->ListView.ColumnHover;
                renderer.FillHorizontalLineSize(x, 1, column->Width, ' ', lvCol->Text); // highlight the column
            }
            else
                lvCol = defaultCol;
        }
        params.X     = x + 1;
        params.Width = column->Width - 2;
        params.Color = lvCol->Text;
        params.Align = column->Align;
        if ((column->HotKeyOffset == NO_HOTKEY_FOR_COLUMN) ||
            ((Flags & ListViewFlags::Sortable) == ListViewFlags::None))
        {
            params.Flags = WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth | WriteTextFlags::OverwriteColors;
            renderer.WriteText(column->Name, params);
        }
        else
        {
            params.Flags = WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth | WriteTextFlags::OverwriteColors |
                           WriteTextFlags::HighlightHotKey;
            params.HotKeyColor    = lvCol->HotKey;
            params.HotKeyPosition = column->HotKeyOffset;
            renderer.WriteText(column->Name, params);
        }
        x += column->Width;
        if ((this->Focused) && (tr == SortParams.ColumnIndex))
            renderer.WriteSpecialCharacter(
                  x - 1,
                  1,
                  this->SortParams.Ascendent ? SpecialChars::TriangleUp : SpecialChars::TriangleDown,
                  lvCol->HotKey);

        if ((Flags & ListViewFlags::HideColumnsSeparator) == ListViewFlags::None)
        {
            renderer.DrawVerticalLine(x, 1, Layout.Height, defaultCol->Separator);
        }
        x++;
    }
}
void ListViewControlContext::DrawItem(Graphics::Renderer& renderer, ListViewItem* item, int y, bool currentItem)
{
    int x = 1 - Columns.XOffset;
    int itemStarts;
    ListViewColumn* column   = this->Columns.List;
    CharacterBuffer* subitem = item->SubItem;
    ColorPair itemCol        = Cfg->ListView.Item.Regular;
    ColorPair checkCol, uncheckCol;
    WriteTextParams params(WriteTextFlags::SingleLine | WriteTextFlags::OverwriteColors | WriteTextFlags::ClipToWidth);
    params.Y = y;

    if (Flags & GATTR_ENABLE)
    {
        checkCol   = Cfg->ListView.CheckedSymbol;
        uncheckCol = Cfg->ListView.UncheckedSymbol;
    }
    else
    {
        checkCol = uncheckCol = Cfg->ListView.InactiveColor;
    }
    // select color based on item type
    switch (item->Type)
    {
    case ListViewItemType::Normal:
        itemCol = item->ItemColor;
        break;
    case ListViewItemType::Highlighted:
        itemCol = Cfg->ListView.Item.Highligheted;
        break;
    case ListViewItemType::ErrorInformation:
        itemCol = Cfg->ListView.Item.Error;
        break;
    case ListViewItemType::WarningInformation:
        itemCol = Cfg->ListView.Item.Warning;
        break;
    case ListViewItemType::GrayedOut:
        itemCol = Cfg->ListView.Item.Inactive;
        break;
    case ListViewItemType::Emphasized_1:
        itemCol = Cfg->ListView.Item.Emphasized1;
        break;
    case ListViewItemType::Emphasized_2:
        itemCol = Cfg->ListView.Item.Emphasized2;
        break;
    case ListViewItemType::Category:
        itemCol = Cfg->ListView.Item.Category;
        break;
    default:
        break;
    }
    // disable is not active
    if (!(Flags & GATTR_ENABLE))
        itemCol = Cfg->ListView.Item.Inactive;
    else
    {
        // if activ and filtered
        if (this->Filter.SearchText.Len() > 0)
        {
            params.Flags =
                  static_cast<WriteTextFlags>((uint32) params.Flags - (uint32) WriteTextFlags::OverwriteColors);
        }
    }
    // prepare params
    params.Color = itemCol;

    // for chategory items a special draw is made (only first comlumn is shown)
    if (item->Type == ListViewItemType::Category)
    {
        if (Focused)
            renderer.DrawHorizontalLine(1, y, this->Layout.Width - 1, Cfg->ListView.Focused.Border, true);
        else
            renderer.DrawHorizontalLine(1, y, this->Layout.Width - 1, Cfg->ListView.Normal.Border, true);
        params.Align = TextAlignament::Center;
        params.Width = this->Layout.Width - 2;
        params.X     = 1;
        params.Flags |= WriteTextFlags::LeftMargin | WriteTextFlags::RightMargin;
        if (currentItem)
            params.Color = Cfg->ListView.FocusColor;
        renderer.WriteText(*subitem, params);
        return;
    }

    // first column
    int end_first_column = x + ((int) column->Width);
    x += (int) item->XOffset;
    if ((Flags & ListViewFlags::CheckBoxes) != ListViewFlags::None)
    {
        if (x < end_first_column)
        {
            if (item->Flags & ITEM_FLAG_CHECKED)
                renderer.WriteSpecialCharacter(x, y, SpecialChars::CheckMark, checkCol);
            else
                renderer.WriteCharacter(x, y, 'x', uncheckCol);
        }
        x += 2;
    }
    itemStarts = x;
    if (x < end_first_column)
    {
        params.Width = end_first_column - x;
        params.X     = x;
        params.Align = column->Align;
        renderer.WriteText(*subitem, params);
    }

    // rest of the columns
    x = end_first_column + 1;
    subitem++;
    column++;
    for (uint32 tr = 1; (tr < Columns.Count) && (x < (int) this->Layout.Width); tr++, column++)
    {
        params.Width = column->Width;
        params.X     = x;
        params.Align = column->Align;
        renderer.WriteText(*subitem, params);

        x += column->Width;
        x++;
        subitem++;
    }
    if (Focused)
    {
        if (currentItem)
        {
            if (((Flags & ListViewFlags::AllowMultipleItemsSelection) != ListViewFlags::None) &&
                (item->Flags & ITEM_FLAG_SELECTED))
                renderer.FillHorizontalLine(itemStarts, y, this->Layout.Width, -1, Cfg->ListView.FocusAndSelectedColor);
            else
                renderer.FillHorizontalLine(itemStarts, y, this->Layout.Width, -1, Cfg->ListView.FocusColor);
            if ((Flags & ListViewFlags::CheckBoxes) != ListViewFlags::None)
                renderer.SetCursor(itemStarts - 2, y); // point the cursor to the check/uncheck
        }
        else
        {
            if (((Flags & ListViewFlags::AllowMultipleItemsSelection) != ListViewFlags::None) &&
                (item->Flags & ITEM_FLAG_SELECTED))
                renderer.FillHorizontalLine(itemStarts, y, this->Layout.Width, -1, Cfg->ListView.SelectionColor);
        }
    }
    else
    {
        if (Flags & GATTR_ENABLE)
        {
            if (((Flags & ListViewFlags::HideCurrentItemWhenNotFocused) == ListViewFlags::None) && (currentItem))
                renderer.FillHorizontalLine(itemStarts, y, this->Layout.Width, -1, Cfg->ListView.SelectionColor);
            if (((Flags & ListViewFlags::AllowMultipleItemsSelection) != ListViewFlags::None) &&
                (item->Flags & ITEM_FLAG_SELECTED))
                renderer.FillHorizontalLine(itemStarts, y, this->Layout.Width, -1, Cfg->ListView.SelectionColor);
        }
    }
    if ((Flags & ListViewFlags::ItemSeparators) != ListViewFlags::None)
    {
        y++;
        ColorPair col = this->Cfg->ListView.ColumnNormal.Separator;
        if (!(this->Flags & GATTR_ENABLE))
            col = this->Cfg->ListView.ColumnInactive.Separator;
        renderer.DrawHorizontalLine(1, y, Layout.Width - 2, col);
        // draw crosses
        if ((Flags & ListViewFlags::HideColumnsSeparator) == ListViewFlags::None)
        {
            x                      = 1 - Columns.XOffset;
            ListViewColumn* column = this->Columns.List;
            for (uint32 tr = 0; (tr < Columns.Count) && (x < (int) this->Layout.Width); tr++, column++)
            {
                x += column->Width;
                renderer.WriteSpecialCharacter(x, y, SpecialChars::BoxCrossSingleLine, col);
                x++;
            }
        }
    }
}

void ListViewControlContext::Paint(Graphics::Renderer& renderer)
{
    int y       = 1;
    auto* lvCol = &this->Cfg->ListView.Normal;
    if (!(this->Flags & GATTR_ENABLE))
        lvCol = &this->Cfg->ListView.Inactive;
    else if (this->Focused)
        lvCol = &this->Cfg->ListView.Focused;
    else if (this->MouseIsOver)
        lvCol = &this->Cfg->ListView.Hover;

    renderer.DrawRectSize(0, 0, this->Layout.Width, this->Layout.Height, lvCol->Border, LineType::Single);
    renderer.SetClipMargins(1, 1, 1, 1);
    if ((Flags & ListViewFlags::HideColumns) == ListViewFlags::None)
    {
        DrawColumn(renderer);
        y++;
    }
    uint32 index = this->Items.FirstVisibleIndex;
    uint32 count = this->Items.Indexes.Len();
    while ((y < this->Layout.Height) && (index < count))
    {
        ListViewItem* item = GetFilteredItem(index);
        DrawItem(renderer, item, y, index == static_cast<unsigned>(this->Items.CurentItemIndex));
        y++;
        if ((Flags & ListViewFlags::ItemSeparators) != ListViewFlags::None)
            y++;
        index++;
    }
    // columns separators
    if ((this->Focused) && ((Columns.HoverSeparatorColumnIndex != INVALID_COLUMN_INDEX) || (Columns.ResizeModeEnabled)))
        DrawColumnSeparatorsForResizeMode(renderer);

    // filtering & status
    if (Focused)
    {
        int x_ofs = 2;
        int yPoz  = ((int) this->Layout.Height) - 1;
        renderer.ResetClip();

        // search bar
        if ((this->Layout.Width > 20) && ((Flags & ListViewFlags::HideSearchBar) == ListViewFlags::None))
        {
            renderer.FillHorizontalLine(x_ofs, yPoz, LISTVIEW_SEARCH_BAR_WIDTH + 3, ' ', Cfg->ListView.FilterText);
            const auto search_text = this->Filter.SearchText.ToStringView();
            if (search_text.length() < LISTVIEW_SEARCH_BAR_WIDTH)
            {
                renderer.WriteSingleLineText(3, yPoz, search_text, Cfg->ListView.FilterText);
                if (Filter.FilterModeEnabled)
                    renderer.SetCursor((int) (3 + search_text.length()), yPoz);
            }
            else
            {
                renderer.WriteSingleLineText(
                      3,
                      yPoz,
                      search_text.substr(search_text.length() - LISTVIEW_SEARCH_BAR_WIDTH, LISTVIEW_SEARCH_BAR_WIDTH),
                      Cfg->ListView.FilterText);
                if (Filter.FilterModeEnabled)
                    renderer.SetCursor(3 + LISTVIEW_SEARCH_BAR_WIDTH, yPoz);
            }
            x_ofs = 17;
        }
        // status information
        if ((this->Flags & ListViewFlags::AllowMultipleItemsSelection) != ListViewFlags::None)
        {
            renderer.WriteSingleLineText(
                  x_ofs,
                  yPoz,
                  string_view(this->Selection.Status, this->Selection.StatusLength),
                  Cfg->ListView.StatusColor);
        }
    }
}
// coloane
bool ListViewControlContext::AddColumn(const ConstString& text, TextAlignament Align, uint32 width)
{
    CHECK(Columns.Count < MAX_LISTVIEW_COLUMNS, false, "");
    Columns.List[Columns.Count].Reset();
    CHECK(Columns.List[Columns.Count].SetName(text), false, "Fail to set column name: %s", text);
    CHECK(Columns.List[Columns.Count].SetAlign(Align), false, "Fail to set alignament to: %d", Align);
    Columns.List[Columns.Count].SetWidth(width);
    Columns.Count++;
    UpdateColumnsWidth();
    return true;
}
void ListViewControlContext::UpdateColumnsWidth()
{
    this->Columns.TotalWidth = 0;
    for (uint32 tr = 0; tr < this->Columns.Count; tr++)
        this->Columns.TotalWidth += ((uint32) (this->Columns.List[tr].Width)) + 1;
}
bool ListViewControlContext::DeleteColumn(uint32 index)
{
    CHECK(index < Columns.Count, false, "Invalid column index: %d (should be smaller than %d)", index, Columns.Count);
    for (uint32 tr = index; tr < Columns.Count; tr++)
        Columns.List[tr] = Columns.List[tr + 1];
    Columns.Count--;
    UpdateColumnsWidth();
    return true;
}
void ListViewControlContext::DeleteAllColumns()
{
    Columns.Count = 0;
    UpdateColumnsWidth();
}
int ListViewControlContext::GetNrColumns()
{
    return Columns.Count;
}

ItemHandle ListViewControlContext::AddItem(const ConstString& text)
{
    ItemHandle idx = (uint32) Items.List.size();
    Items.List.push_back(ListViewItem(Cfg->ListView.Item.Regular));
    Items.Indexes.Push(idx);
    SetItemText(idx, 0, text);
    return idx;
}
bool ListViewControlContext::SetItemText(ItemHandle item, uint32 subItem, const ConstString& text)
{
    PREPARE_LISTVIEW_ITEM(item, false);
    CHECK(subItem < Columns.Count,
          false,
          "Invalid column index (%d), should be smaller than %d",
          subItem,
          Columns.Count);
    CHECK(i.SubItem[subItem].Set(text), false, "Fail to set text to a sub-item: %s", text);
    return true;
}
Graphics::CharacterBuffer* ListViewControlContext::GetItemText(ItemHandle item, uint32 subItem)
{
    PREPARE_LISTVIEW_ITEM(item, nullptr);
    CHECK(subItem < Columns.Count,
          nullptr,
          "Invalid column index (%d), should be smaller than %d",
          subItem,
          Columns.Count);
    return &i.SubItem[subItem];
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
    UpdateSelectionInfo();
    return true;
}
bool ListViewControlContext::SetItemColor(ItemHandle item, ColorPair color)
{
    PREPARE_LISTVIEW_ITEM(item, false);
    CHECK(i.Type == ListViewItemType::Normal,
          false,
          "Item color only applies to regular item. Use SetItemType to change item type !");
    i.ItemColor = color;
    return true;
}
bool ListViewControlContext::SetItemType(ItemHandle item, ListViewItemType type)
{
    PREPARE_LISTVIEW_ITEM(item, false);
    i.Type = type;
    return true;
}
bool ListViewControlContext::SetItemDataAsValue(ItemHandle item, uint64 value)
{
    PREPARE_LISTVIEW_ITEM(item, false);
    i.Data = value;
    return true;
}
bool ListViewControlContext::GetItemDataAsValue(const ItemHandle item, uint64& value) const
{
    CONST_PREPARE_LISTVIEW_ITEM(item, false);
    CHECK(std::holds_alternative<uint64>(i.Data), false, "No value was stored in current item");
    value = std::get<uint64>(i.Data);
    return true;
}
bool ListViewControlContext::SetItemDataAsPointer(ItemHandle item, GenericRef obj)
{
    PREPARE_LISTVIEW_ITEM(item, false);
    i.Data = obj;
    return true;
}
GenericRef ListViewControlContext::GetItemDataAsPointer(const ItemHandle item) const
{
    CONST_PREPARE_LISTVIEW_ITEM(item, GenericRef(nullptr));
    if (std::holds_alternative<GenericRef>(i.Data))
    {
        return std::get<GenericRef>(i.Data);
    }
    return GenericRef(nullptr);
}
bool ListViewControlContext::SetItemXOffset(ItemHandle item, uint32 XOffset)
{
    PREPARE_LISTVIEW_ITEM(item, false);
    i.XOffset = (uint32) XOffset;
    return true;
}
uint32 ListViewControlContext::GetItemXOffset(ItemHandle item)
{
    PREPARE_LISTVIEW_ITEM(item, 0);
    return i.XOffset;
}
bool ListViewControlContext::SetItemHeight(ItemHandle item, uint32 itemHeight)
{
    CHECK(itemHeight > 0, false, "Item height should be bigger than 0");
    PREPARE_LISTVIEW_ITEM(item, false);
    i.Height = itemHeight;
    return true;
}
uint32 ListViewControlContext::GetItemHeight(ItemHandle item)
{
    PREPARE_LISTVIEW_ITEM(item, 0);
    return i.Height;
}
void ListViewControlContext::SetClipboardSeparator(char ch)
{
    clipboardSeparator = ch;
}
bool ListViewControlContext::SetColumnClipboardCopyState(uint32 columnIndex, bool allowCopy)
{
    CHECK(columnIndex < Columns.Count,
          false,
          "Invalid column index: %d (should be smaller than %d)",
          columnIndex,
          Columns.Count);
    if (allowCopy)
        Columns.List[columnIndex].Flags -= Columns.List[columnIndex].Flags & COLUMN_DONT_COPY;
    else
        Columns.List[columnIndex].Flags |= COLUMN_DONT_COPY;
    return true;
}
bool ListViewControlContext::SetColumnFilterMode(uint32 columnIndex, bool allowFilterForThisColumn)
{
    CHECK(columnIndex < Columns.Count,
          false,
          "Invalid column index (%d), should be smaller than %d",
          columnIndex,
          Columns.Count);
    if (allowFilterForThisColumn)
        Columns.List[columnIndex].Flags -= Columns.List[columnIndex].Flags & COLUMN_DONT_FILTER;
    else
        Columns.List[columnIndex].Flags |= COLUMN_DONT_FILTER;
    return true;
}
bool ListViewControlContext::IsItemChecked(ItemHandle item)
{
    PREPARE_LISTVIEW_ITEM(item, false);
    return (bool) ((i.Flags & ITEM_FLAG_CHECKED) != 0);
}
bool ListViewControlContext::IsItemSelected(ItemHandle item)
{
    PREPARE_LISTVIEW_ITEM(item, false);
    return (bool) ((i.Flags & ITEM_FLAG_SELECTED) != 0);
}
void ListViewControlContext::SelectAllItems()
{
    UpdateSelection(0, Items.Indexes.Len(), true);
}
void ListViewControlContext::UnSelectAllItems()
{
    UpdateSelection(0, Items.Indexes.Len(), false);
}
void ListViewControlContext::CheckAllItems()
{
    uint32 sz       = Items.Indexes.Len();
    uint32* indexes = Items.Indexes.GetUInt32Array();
    if (indexes == nullptr)
        return;
    for (uint32 tr = 0; tr < sz; tr++, indexes++)
        SetItemCheck(*indexes, true);
}
void ListViewControlContext::UncheckAllItems()
{
    uint32 sz       = Items.Indexes.Len();
    uint32* indexes = Items.Indexes.GetUInt32Array();
    if (indexes == nullptr)
        return;
    for (uint32 tr = 0; tr < sz; tr++, indexes++)
        SetItemCheck(*indexes, false);
}
uint32 ListViewControlContext::GetCheckedItemsCount()
{
    uint32 count    = 0;
    uint32 sz       = Items.Indexes.Len();
    uint32* indexes = Items.Indexes.GetUInt32Array();
    if (indexes == nullptr)
        return 0;
    for (uint32 tr = 0; tr < sz; tr++, indexes++)
    {
        if ((*indexes) < sz)
        {
            if ((Items.List[*indexes].Flags & ITEM_FLAG_CHECKED) != 0)
                count++;
        }
    }

    return count;
}

bool ListViewControlContext::SetCurrentIndex(ItemHandle item)
{
    CHECK((uint32) item < Items.Indexes.Len(),
          false,
          "Invalid index: %d (should be smaller than %d)",
          item,
          Items.Indexes.Len());
    MoveTo((int) item);
    return true;
}
int ListViewControlContext::GetFirstVisibleLine()
{
    return Items.FirstVisibleIndex;
}
bool ListViewControlContext::SetFirstVisibleLine(ItemHandle item)
{
    if (SetCurrentIndex(item) == false)
        return false;
    Items.FirstVisibleIndex = Items.CurentItemIndex;
    return true;
}
void ListViewControlContext::DeleteAllItems()
{
    Items.List.clear();
    Items.Indexes.Clear();
    Columns.XOffset          = 0;
    Items.FirstVisibleIndex  = 0;
    Items.CurentItemIndex    = 0;
    Filter.FilterModeEnabled = false;
    Filter.SearchText.Clear();
}
// movement
int ListViewControlContext::GetVisibleItemsCount()
{
    int vis = Layout.Height - 3;
    if ((Flags & ListViewFlags::HideColumns) != ListViewFlags::None)
        vis++;
    int dim = 0, poz = Items.FirstVisibleIndex, nrItems = 0;
    int sz = (int) Items.Indexes.Len();
    while ((dim < vis) && (poz < sz))
    {
        ListViewItem* i = GetFilteredItem(poz);
        if (i)
            dim += i->Height;
        if ((Flags & ListViewFlags::ItemSeparators) != ListViewFlags::None)
            dim++;
        nrItems++;
        poz++;
    }
    return nrItems;
}
void ListViewControlContext::UpdateSelectionInfo()
{
    uint32 count = 0;
    uint32 size  = (uint32) this->Items.List.size();
    for (uint32 tr = 0; tr < size; tr++)
    {
        if (this->Items.List[tr].Flags & ITEM_FLAG_SELECTED)
            count++;
    }
    Utils::String tmp;
    while (true)
    {
        CHECKBK(
              tmp.Create(this->Selection.Status, sizeof(this->Selection.Status), true),
              "Fail to create selection string ");
        CHECKBK(tmp.SetFormat("[%u/%u]", count, size), "Fail to format selection status string !");
        this->Selection.StatusLength = tmp.Len();
        return;
    }
    this->Selection.Status[0]    = 0;
    this->Selection.StatusLength = 0;
}
void ListViewControlContext::UpdateSelection(int start, int end, bool select)
{
    ListViewItem* i;
    int totalItems = Items.Indexes.Len();

    while ((start != end) && (start >= 0) && (start < totalItems))
    {
        i = GetFilteredItem(start);
        if (i != nullptr)
        {
            if (select)
                i->Flags |= ITEM_FLAG_SELECTED;
            else
                i->Flags &= (0xFFFFFFFF - ITEM_FLAG_SELECTED);
        }
        if (start < end)
            start++;
        else
            start--;
    }
    UpdateSelectionInfo();
}
void ListViewControlContext::MoveTo(int index)
{
    int count = Items.Indexes.Len();
    if (count <= 0)
        return;
    if (index >= count)
        index = count - 1;
    if (index < 0)
        index = 0;
    if (index == Items.CurentItemIndex)
        return;
    int vis               = GetVisibleItemsCount();
    int rel               = index - Items.FirstVisibleIndex;
    int originalPoz       = Items.CurentItemIndex;
    Items.CurentItemIndex = index;
    if (rel < 0)
        Items.FirstVisibleIndex = index;
    if (rel >= vis)
        Items.FirstVisibleIndex = (index - vis) + 1;
    if (originalPoz != index)
        SendMsg(Event::ListViewCurrentItemChanged);
}
bool ListViewControlContext::OnKeyEvent(Input::Key keyCode, char16 UnicodeChar)
{
    LocalUnicodeStringBuilder<256> temp;
    ListViewItem* lvi;
    bool selected;

    if (Columns.ResizeModeEnabled)
    {
        Filter.FilterModeEnabled = false;
        switch (keyCode)
        {
        case Key::Ctrl | Key::Right:
            Columns.ResizeColumnIndex++;
            if (Columns.ResizeColumnIndex >= Columns.Count)
                Columns.ResizeColumnIndex = 0;
            return true;
        case Key::Ctrl | Key::Left:
            if (Columns.ResizeColumnIndex > 0)
                Columns.ResizeColumnIndex--;
            else
                Columns.ResizeColumnIndex = Columns.Count - 1;
            return true;
        case Key::Left:
            Columns.List[Columns.ResizeColumnIndex].SetWidth(
                  ((uint32) Columns.List[Columns.ResizeColumnIndex].Width) - 1);
            UpdateColumnsWidth();
            return true;
        case Key::Right:
            Columns.List[Columns.ResizeColumnIndex].SetWidth(
                  ((uint32) Columns.List[Columns.ResizeColumnIndex].Width) + 1);
            UpdateColumnsWidth();
            return true;
        };
        if ((UnicodeChar > 0) || (keyCode != Key::None))
        {
            Columns.ResizeModeEnabled = false;
            Columns.ResizeColumnIndex = INVALID_COLUMN_INDEX;
            return true;
        }
    }
    else
    {
        if ((Flags & ListViewFlags::AllowMultipleItemsSelection) != ListViewFlags::None)
        {
            lvi = GetFilteredItem(Items.CurentItemIndex);
            if (lvi != nullptr)
                selected = ((lvi->Flags & ITEM_FLAG_SELECTED) != 0);
            else
                selected = false;
            switch (keyCode)
            {
            case Key::Shift | Key::Up:
                UpdateSelection(Items.CurentItemIndex, Items.CurentItemIndex - 1, !selected);
                MoveTo(Items.CurentItemIndex - 1);
                Filter.FilterModeEnabled = false;
                SendMsg(Event::ListViewSelectionChanged);
                return true;
            case Key::Insert:
            case Key::Down | Key::Shift:
                UpdateSelection(Items.CurentItemIndex, Items.CurentItemIndex + 1, !selected);
                MoveTo(Items.CurentItemIndex + 1);
                Filter.FilterModeEnabled = false;
                SendMsg(Event::ListViewSelectionChanged);
                return true;
            case Key::PageUp | Key::Shift:
                UpdateSelection(Items.CurentItemIndex, Items.CurentItemIndex - GetVisibleItemsCount(), !selected);
                MoveTo(Items.CurentItemIndex - GetVisibleItemsCount());
                Filter.FilterModeEnabled = false;
                SendMsg(Event::ListViewSelectionChanged);
                return true;
            case Key::PageDown | Key::Shift:
                UpdateSelection(Items.CurentItemIndex, Items.CurentItemIndex + GetVisibleItemsCount(), !selected);
                MoveTo(Items.CurentItemIndex + GetVisibleItemsCount());
                Filter.FilterModeEnabled = false;
                SendMsg(Event::ListViewSelectionChanged);
                return true;
            case Key::Home | Key::Shift:
                UpdateSelection(Items.CurentItemIndex, 0, !selected);
                MoveTo(0);
                Filter.FilterModeEnabled = false;
                SendMsg(Event::ListViewSelectionChanged);
                return true;
            case Key::End | Key::Shift:
                UpdateSelection(Items.CurentItemIndex, Items.Indexes.Len(), !selected);
                MoveTo(Items.Indexes.Len());
                Filter.FilterModeEnabled = false;
                SendMsg(Event::ListViewSelectionChanged);
                return true;
            };
        }
        switch (keyCode)
        {
        case Key::Up:
            MoveTo(Items.CurentItemIndex - 1);
            Filter.FilterModeEnabled = false;
            return true;
        case Key::Down:
            MoveTo(Items.CurentItemIndex + 1);
            Filter.FilterModeEnabled = false;
            return true;
        case Key::PageUp:
            MoveTo(Items.CurentItemIndex - GetVisibleItemsCount());
            Filter.FilterModeEnabled = false;
            return true;
        case Key::PageDown:
            MoveTo(Items.CurentItemIndex + GetVisibleItemsCount());
            Filter.FilterModeEnabled = false;
            return true;
        case Key::Left:
            if (Columns.XOffset > 0)
                Columns.XOffset--;
            Filter.FilterModeEnabled = false;
            return true;
        case Key::Right:
            UpdateColumnsWidth();
            Columns.XOffset          = std::min<>(Columns.XOffset + 1, (int) Columns.TotalWidth);
            Filter.FilterModeEnabled = false;
            return true;
        case Key::Home:
            MoveTo(0);
            Filter.FilterModeEnabled = false;
            return true;
        case Key::End:
            MoveTo(((int) Items.Indexes.Len()) - 1);
            Filter.FilterModeEnabled = false;
            return true;
        case Key::Backspace:
            if ((Flags & ListViewFlags::HideSearchBar) == ListViewFlags::None)
            {
                Filter.FilterModeEnabled = true;
                if (Filter.SearchText.Len() > 0)
                {
                    Filter.SearchText.Truncate(Filter.SearchText.Len() - 1);
                    UpdateSearch(0);
                }
            }
            return true;
        case Key::Space:
            if (!Filter.FilterModeEnabled)
            {
                if ((Flags & ListViewFlags::CheckBoxes) == ListViewFlags::None)
                    return false;
                lvi = GetFilteredItem(Items.CurentItemIndex);
                if (lvi != nullptr)
                {
                    if ((lvi->Flags & ITEM_FLAG_CHECKED) != 0)
                        lvi->Flags -= ITEM_FLAG_CHECKED;
                    else
                        lvi->Flags |= ITEM_FLAG_CHECKED;
                }
                SendMsg(Event::ListViewItemChecked);
            }
            else
            {
                if ((Flags & ListViewFlags::HideSearchBar) == ListViewFlags::None)
                {
                    Filter.SearchText.AddChar(' ');
                    UpdateSearch(0);
                }
            }
            return true;
        case Key::Enter | Key::Ctrl:
            if (((Flags & ListViewFlags::SearchMode) != ListViewFlags::None) &&
                ((Flags & ListViewFlags::HideSearchBar) == ListViewFlags::None) && (Filter.SearchText.Len() > 0))
            {
                if (Filter.LastFoundItem >= 0)
                    UpdateSearch(Filter.LastFoundItem + 1);
                else
                    UpdateSearch(0);
                return true; // de vazut daca are send
            }
            return false;
        case Key::Enter:
            SendMsg(Event::ListViewItemClicked);
            return true;
        case Key::Escape:
            if ((Flags & ListViewFlags::HideSearchBar) == ListViewFlags::None)
            {
                if (Filter.FilterModeEnabled)
                {
                    if (Filter.SearchText.Len() > 0)
                    {
                        Filter.SearchText.Clear();
                        UpdateSearch(0);
                    }
                    else
                    {
                        Filter.FilterModeEnabled = false;
                    }
                    return true;
                }
                if ((Flags & ListViewFlags::SearchMode) == ListViewFlags::None)
                {
                    if (Filter.SearchText.Len() > 0)
                    {
                        Filter.SearchText.Clear();
                        UpdateSearch(0);
                        return true;
                    }
                }
            }
            return false;

        case Key::Ctrl | Key::Right:
        case Key::Ctrl | Key::Left:
            Columns.ResizeColumnIndex = 0;
            Columns.ResizeModeEnabled = true;
            return true;
        case Key::Ctrl | Key::C:
        case Key::Ctrl | Key::Insert:
            if (Items.Indexes.Len() > 0)
            {
                for (uint32 tr = 0; tr < Columns.Count; tr++)
                {
                    if ((Columns.List[tr].Flags & COLUMN_DONT_COPY) == 0)
                    {
                        temp.Add(GetFilteredItem(Items.CurentItemIndex)->SubItem[tr]);
                        if (clipboardSeparator != 0)
                            temp.Add(string_view{ &clipboardSeparator, 1 });
                    }
                }
                OS::Clipboard::SetText(temp);
            }
            return true;
        case Key::Ctrl | Key::Alt | Key::Insert:
            for (uint32 gr = 0; gr < Items.Indexes.Len(); gr++)
            {
                for (uint32 tr = 0; tr < Columns.Count; tr++)
                {
                    if ((Columns.List[tr].Flags & COLUMN_DONT_COPY) == 0)
                    {
                        temp.Add(GetFilteredItem(gr)->SubItem[tr]);
                        if (clipboardSeparator != 0)
                            temp.Add(string_view{ &clipboardSeparator, 1 });
                    }
                }
                temp.Add("\n");
            }
            OS::Clipboard::SetText(temp);
            return true;
        };
        // caut sort
        if ((Flags & ListViewFlags::HideSearchBar) == ListViewFlags::None)
        {
            if (Filter.FilterModeEnabled == false)
            {
                for (uint32 tr = 0; tr < Columns.Count; tr++)
                {
                    if (Columns.List[tr].HotKeyCode == keyCode)
                    {
                        ColumnSort(tr);
                        return true;
                    }
                }
            }
            // search mode
            if (UnicodeChar > 0)
            {
                Filter.FilterModeEnabled = true;
                Filter.SearchText.AddChar(UnicodeChar);
                UpdateSearch(0);
                return true;
            }
        }
    }
    return false;
}
bool ListViewControlContext::MouseToHeader(int x, int, uint32& HeaderIndex, uint32& HeaderColumnIndex)
{
    int xx                 = 1 - Columns.XOffset;
    ListViewColumn* column = this->Columns.List;
    for (uint32 tr = 0; tr < Columns.Count; tr++, column++)
    {
        if ((x >= xx) && (x <= xx + column->Width) && (x > 1) && (x < (this->Layout.Width - 2)))
        {
            if (x == (xx + column->Width))
            {
                HeaderColumnIndex = tr;
                HeaderIndex       = INVALID_COLUMN_INDEX;
            }
            else
            {
                HeaderColumnIndex = INVALID_COLUMN_INDEX;
                HeaderIndex       = tr;
            }
            return true;
        }
        xx += column->Width;
        xx++;
    }
    HeaderColumnIndex = INVALID_COLUMN_INDEX;
    HeaderIndex       = INVALID_COLUMN_INDEX;
    return false;
}
void ListViewControlContext::OnMouseReleased(int, int, Input::MouseButton)
{
    Columns.ResizeModeEnabled         = false;
    Columns.ResizeColumnIndex         = INVALID_COLUMN_INDEX;
    Columns.HoverSeparatorColumnIndex = INVALID_COLUMN_INDEX;
    Columns.HoverColumnIndex          = INVALID_COLUMN_INDEX;
}
void ListViewControlContext::OnMousePressed(int x, int y, Input::MouseButton button)
{
    if (((Flags & ListViewFlags::HideColumns) == ListViewFlags::None))
    {
        uint32 hIndex, hColumn;
        if (MouseToHeader(x, y, hIndex, hColumn))
        {
            if ((y == 1) && (hIndex != INVALID_COLUMN_INDEX))
            {
                ColumnSort(hIndex);
                return;
            }
            if (hColumn != INVALID_COLUMN_INDEX)
                return;
        }
    }
    // check is the search bar was pressed
    if ((this->Layout.Width > 20) && ((Flags & ListViewFlags::HideSearchBar) == ListViewFlags::None) &&
        (y == (this->Layout.Height - 1)))
    {
        if ((x >= 2) && (x <= (3 + LISTVIEW_SEARCH_BAR_WIDTH)))
        {
            this->Filter.FilterModeEnabled = true;
            return;
        }
    }

    // check if items are pressed
    if ((Flags & ListViewFlags::HideColumns) != ListViewFlags::None)
        y--;
    else
        y -= 2;
    if ((Flags & ListViewFlags::ItemSeparators) != ListViewFlags::None)
        y = y / 2;

    if (y < GetVisibleItemsCount())
    {
        this->Filter.FilterModeEnabled = false;

        if ((y + Items.FirstVisibleIndex) != this->Items.CurentItemIndex)
            MoveTo(y + Items.FirstVisibleIndex);

        if ((y + Items.FirstVisibleIndex) == this->Items.CurentItemIndex)
        {
            auto i = GetFilteredItem(Items.CurentItemIndex);
            if (x == ((1 - Columns.XOffset) + (int) i->XOffset))
            {
                if ((i->Flags & ITEM_FLAG_CHECKED) != 0)
                    i->Flags -= ITEM_FLAG_CHECKED;
                else
                    i->Flags |= ITEM_FLAG_CHECKED;
                SendMsg(Event::ListViewItemChecked);
            }
            else
            {
                if (((button & Input::MouseButton::DoubleClicked) != Input::MouseButton::None))
                    SendMsg(Event::ListViewItemClicked);
            }
        }
    }
}
bool ListViewControlContext::OnMouseDrag(int x, int, Input::MouseButton)
{
    if (Columns.HoverSeparatorColumnIndex != INVALID_COLUMN_INDEX)
    {
        int xx                 = 1 - Columns.XOffset;
        ListViewColumn* column = this->Columns.List;
        for (uint32 tr = 0; tr < Columns.HoverSeparatorColumnIndex; tr++, column++)
            xx += (((uint32) column->Width) + 1);
        // xx = the start of column
        if (x > xx)
        {
            column->SetWidth((uint32) (x - xx));
            UpdateColumnsWidth();
        }
        return true;
    }
    return false;
}
bool ListViewControlContext::OnMouseOver(int x, int y)
{
    if ((Flags & ListViewFlags::HideColumns) == ListViewFlags::None)
    {
        uint32 hIndex, hColumn;
        MouseToHeader(x, y, hIndex, hColumn);
        if ((hIndex != Columns.HoverColumnIndex) && (y == 1) &&
            ((Flags & ListViewFlags::Sortable) != ListViewFlags::None))
        {
            Columns.HoverColumnIndex          = hIndex;
            Columns.HoverSeparatorColumnIndex = INVALID_COLUMN_INDEX;
            return true;
        }
        if (hColumn != Columns.HoverSeparatorColumnIndex)
        {
            Columns.HoverColumnIndex          = INVALID_COLUMN_INDEX;
            Columns.HoverSeparatorColumnIndex = hColumn;
            return true;
        }
    }
    return false;
}
bool ListViewControlContext::OnMouseWheel(int, int, Input::MouseWheel direction)
{
    switch (direction)
    {
    case Input::MouseWheel::Up:
        if (this->Items.FirstVisibleIndex > 0)
            this->Items.FirstVisibleIndex--;
        return true;
    case Input::MouseWheel::Down:
        if (this->Items.FirstVisibleIndex >= 0)
        {
            if (((size_t) this->Items.FirstVisibleIndex) + 1 < this->Items.Indexes.Len())
                this->Items.FirstVisibleIndex++;
            return true;
        }
        break;
    case Input::MouseWheel::Left:
        return OnKeyEvent(Key::Left, 0);
    case Input::MouseWheel::Right:
        return OnKeyEvent(Key::Right, 0);
    }
    return false;
}
// sort
void ListViewControlContext::SetSortColumn(uint32 colIndex)
{
    if (colIndex >= Columns.Count)
        this->SortParams.ColumnIndex = INVALID_COLUMN_INDEX;
    else
        this->SortParams.ColumnIndex = colIndex;
    if ((Flags & ListViewFlags::Sortable) == ListViewFlags::None)
        this->SortParams.ColumnIndex = INVALID_COLUMN_INDEX;
}

void ListViewControlContext::ColumnSort(uint32 columnIndex)
{
    if ((Flags & ListViewFlags::Sortable) == ListViewFlags::None)
    {
        this->SortParams.ColumnIndex = INVALID_COLUMN_INDEX;
        return;
    }
    if (columnIndex != SortParams.ColumnIndex)
        SetSortColumn(columnIndex);
    else
        SortParams.Ascendent = !SortParams.Ascendent;
    Sort();
}
//----------------------

int SortIndexesCompareFunction(uint32 indx1, uint32 indx2, void* context)
{
    ListViewControlContext* lvcc = (ListViewControlContext*) context;
    if (lvcc->SortParams.CompareCallbak)
    {
        return lvcc->SortParams.CompareCallbak(
              (ListView*) lvcc->Host,
              indx1,
              indx2,
              lvcc->SortParams.ColumnIndex,
              lvcc->SortParams.CompareCallbakContext);
    }
    else
    {
        uint32 itemsCount = (uint32) lvcc->Items.List.size();
        if ((indx1 < itemsCount) && (indx2 < itemsCount) && (lvcc->SortParams.ColumnIndex != INVALID_COLUMN_INDEX))
        {
            return lvcc->Items.List[indx1].SubItem[lvcc->SortParams.ColumnIndex].CompareWith(
                  lvcc->Items.List[indx2].SubItem[lvcc->SortParams.ColumnIndex], true);
        }
        else
        {
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
    if (SortParams.CompareCallbak == nullptr)
    {
        // sanity check
        CHECK(SortParams.ColumnIndex < Columns.Count, false, "No sort column or custom sort function defined !");
    }
    Items.Indexes.Sort(SortIndexesCompareFunction, SortParams.Ascendent, this);
    return true;
}
int ListViewControlContext::SearchItem(uint32 startPoz)
{
    const auto count = static_cast<uint32>(Items.List.size());
    if (startPoz >= count)
        startPoz = 0;
    if (count == 0)
        return -1;

    uint32 originalStartPoz = startPoz;
    int found               = -1;
    do
    {
        if (FilterItem(Items.List[startPoz], true))
        {
            if (found == -1)
                found = startPoz;
        }
        startPoz++;
        if (startPoz >= count)
            startPoz = 0;

    } while (startPoz != originalStartPoz);
    return found;
}
bool ListViewControlContext::FilterItem(ListViewItem& lvi, bool clearColorForAll)
{
    uint32 columnID = 0;
    int index       = -1;

    for (uint32 gr = 0; gr < Columns.Count; gr++)
    {
        if ((Columns.List[gr].Flags & COLUMN_DONT_FILTER) != 0)
            continue;
        index = lvi.SubItem[gr].Find(this->Filter.SearchText.ToStringView(), true);
        if (index >= 0)
        {
            columnID = gr;
            break;
        }
    }
    if ((clearColorForAll) || (index >= 0))
    {
        // clear all colors
        for (uint32 gr = 0; gr < Columns.Count; gr++)
        {
            lvi.SubItem[gr].SetColor(this->Cfg->ListView.Highlight.Normal);
        }
    }
    if (index >= 0)
    {
        // set color for
        lvi.SubItem[columnID].SetColor(
              index, index + this->Filter.SearchText.Len(), this->Cfg->ListView.Highlight.Selected);
        return true;
    }

    return false;
}
void ListViewControlContext::FilterItems()
{
    Items.Indexes.Clear();
    uint32 count = (uint32) Items.List.size();
    if (this->Filter.SearchText.Len() == 0)
    {
        Items.Indexes.Reserve((uint32) Items.List.size());
        for (uint32 tr = 0; tr < count; tr++)
            Items.Indexes.Push(tr);
    }
    else
    {
        for (uint32 tr = 0; tr < count; tr++)
        {
            if (FilterItem(Items.List[tr], false))
                Items.Indexes.Push(tr);
        }
    }
    this->Items.FirstVisibleIndex = 0;
    this->Items.CurentItemIndex   = 0;
    SendMsg(Event::ListViewCurrentItemChanged);
}
void ListViewControlContext::UpdateSearch(int startPoz)
{
    int index;

    // daca fac filtrare
    if ((Flags & ListViewFlags::SearchMode) == ListViewFlags::None)
    {
        FilterItems();
    }
    else
    {
        if ((index = SearchItem(startPoz)) >= 0)
        {
            this->Filter.LastFoundItem = index;
            MoveTo(index);
        }
        else
        {
            if (Filter.SearchText.Len() > 0)
                Filter.SearchText.Truncate(Filter.SearchText.Len() - 1);
            // research and re-highlight
            this->Filter.LastFoundItem = SearchItem(startPoz);
        }
    }
    this->Filter.FilterModeEnabled = Filter.SearchText.Len() > 0;
}
void ListViewControlContext::SendMsg(Event eventType)
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
ListView::ListView(string_view layout, ListViewFlags flags) : Control(new ListViewControlContext(), "", layout, false)
{
    auto Members              = reinterpret_cast<ListViewControlContext*>(this->Context);
    Members->Layout.MinWidth  = 5;
    Members->Layout.MinHeight = 3;
    Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | GATTR_HSCROLL | GATTR_VSCROLL | (uint32) flags;
    Members->ScrollBars.LeftMargin = 25;
    // allocate items
    ASSERT(Members->Items.Indexes.Create(32), "Fail to allocate Listview indexes");
    Members->Items.List.reserve(32);

    // initialize
    Members->Columns.Count                     = 0;
    Members->Columns.XOffset                   = 0;
    Members->Items.FirstVisibleIndex           = 0;
    Members->Items.CurentItemIndex             = 0;
    Members->SortParams.ColumnIndex            = INVALID_COLUMN_INDEX;
    Members->Columns.HoverColumnIndex          = INVALID_COLUMN_INDEX;
    Members->Columns.HoverSeparatorColumnIndex = INVALID_COLUMN_INDEX;
    Members->SortParams.Ascendent              = true;
    Members->SortParams.CompareCallbak         = nullptr;
    Members->SortParams.CompareCallbakContext  = nullptr;
    Members->Columns.ResizeModeEnabled         = false;
    Members->Filter.FilterModeEnabled          = false;
    Members->Filter.LastFoundItem              = -1;
    Members->Columns.ResizeColumnIndex         = INVALID_COLUMN_INDEX;
    Members->clipboardSeparator                = '\t';
    Members->Columns.TotalWidth                = 0;
    Members->Host                              = this;
    Members->Filter.SearchText.Clear();
    Members->Selection.Status[0]    = 0;
    Members->Selection.StatusLength = 0;
}
void ListView::Paint(Graphics::Renderer& renderer)
{
    CREATE_TYPECONTROL_CONTEXT(ListViewControlContext, Members, );
    Members->Paint(renderer);
}
bool ListView::OnKeyEvent(Input::Key keyCode, char16 UnicodeChar)
{
    return WRAPPER->OnKeyEvent(keyCode, UnicodeChar);
}
void ListView::OnUpdateScrollBars()
{
    CREATE_TYPECONTROL_CONTEXT(ListViewControlContext, Members, );
    // compute XOfsset
    uint32 left_margin = 2;
    if ((Members->Flags & ListViewFlags::HideSearchBar) == ListViewFlags::None)
        left_margin = 17;
    if ((Members->Flags & ListViewFlags::AllowMultipleItemsSelection) != ListViewFlags::None)
        left_margin += (Members->Selection.StatusLength + 1);

    Members->ScrollBars.LeftMargin = left_margin;
    UpdateHScrollBar(Members->Columns.XOffset, Members->Columns.TotalWidth);
    uint32 count = Members->Items.Indexes.Len();
    if (count > 0)
        count--;
    UpdateVScrollBar(Members->Items.CurentItemIndex, count);
}

bool ListView::AddColumn(const ConstString& text, TextAlignament Align, uint32 Size)
{
    return WRAPPER->AddColumn(text, Align, Size);
}
bool ListView::SetColumnText(uint32 columnIndex, const ConstString& text)
{
    CHECK(columnIndex < WRAPPER->Columns.Count,
          false,
          "Invalid column index:%d (should be smaller than %d)",
          columnIndex,
          WRAPPER->Columns.Count);
    return WRAPPER->Columns.List[columnIndex].SetName(text);
}
bool ListView::SetColumnAlignament(uint32 columnIndex, TextAlignament Align)
{
    CHECK(columnIndex < WRAPPER->Columns.Count,
          false,
          "Invalid column index:%d (should be smaller than %d)",
          columnIndex,
          WRAPPER->Columns.Count);
    return WRAPPER->Columns.List[columnIndex].SetAlign(Align);
}
bool ListView::SetColumnWidth(uint32 columnIndex, uint32 width)
{
    CHECK(columnIndex < WRAPPER->Columns.Count,
          false,
          "Invalid column index:%d (should be smaller than %d)",
          columnIndex,
          WRAPPER->Columns.Count);
    WRAPPER->Columns.List[columnIndex].SetWidth(width);
    WRAPPER->UpdateColumnsWidth();
    return true;
}
bool ListView::SetColumnClipboardCopyState(uint32 columnIndex, bool allowCopy)
{
    return WRAPPER->SetColumnClipboardCopyState(columnIndex, allowCopy);
}
bool ListView::SetColumnFilterMode(uint32 columnIndex, bool allowFilterForThisColumn)
{
    return WRAPPER->SetColumnFilterMode(columnIndex, allowFilterForThisColumn);
}
bool ListView::DeleteColumn(uint32 columnIndex)
{
    return WRAPPER->DeleteColumn(columnIndex);
}
void ListView::DeleteAllColumns()
{
    if (Context != nullptr)
    {
        WRAPPER->DeleteAllColumns();
    }
}
uint32 ListView::GetColumnsCount()
{
    return WRAPPER->GetNrColumns();
}

ItemHandle ListView::AddItem(const ConstString& text)
{
    return WRAPPER->AddItem(text);
}
ItemHandle ListView::AddItem(const ConstString& text, const ConstString& subItem1)
{
    ItemHandle handle = WRAPPER->AddItem(text);
    CHECK(handle != InvalidItemHandle, InvalidItemHandle, "Fail to allocate item for ListView");
    CHECK(WRAPPER->SetItemText(handle, 1, subItem1), InvalidItemHandle, "");
    return handle;
}
ItemHandle ListView::AddItem(const ConstString& text, const ConstString& subItem1, const ConstString& subItem2)
{
    ItemHandle handle = WRAPPER->AddItem(text);
    CHECK(handle != InvalidItemHandle, InvalidItemHandle, "Fail to allocate item for ListView");
    CHECK(WRAPPER->SetItemText(handle, 1, subItem1), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 2, subItem2), InvalidItemHandle, "");
    return handle;
}
ItemHandle ListView::AddItem(
      const ConstString& text, const ConstString& subItem1, const ConstString& subItem2, const ConstString& subItem3)
{
    ItemHandle handle = WRAPPER->AddItem(text);
    CHECK(handle != InvalidItemHandle, InvalidItemHandle, "Fail to allocate item for ListView");
    CHECK(WRAPPER->SetItemText(handle, 1, subItem1), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 2, subItem2), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 3, subItem3), InvalidItemHandle, "");
    return handle;
}
ItemHandle ListView::AddItem(
      const ConstString& text,
      const ConstString& subItem1,
      const ConstString& subItem2,
      const ConstString& subItem3,
      const ConstString& subItem4)
{
    ItemHandle handle = WRAPPER->AddItem(text);
    CHECK(handle != InvalidItemHandle, InvalidItemHandle, "Fail to allocate item for ListView");
    CHECK(WRAPPER->SetItemText(handle, 1, subItem1), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 2, subItem2), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 3, subItem3), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 4, subItem4), InvalidItemHandle, "");
    return handle;
}
ItemHandle ListView::AddItem(
      const ConstString& text,
      const ConstString& subItem1,
      const ConstString& subItem2,
      const ConstString& subItem3,
      const ConstString& subItem4,
      const ConstString& subItem5)
{
    ItemHandle handle = WRAPPER->AddItem(text);
    CHECK(handle != InvalidItemHandle, InvalidItemHandle, "Fail to allocate item for ListView");
    CHECK(WRAPPER->SetItemText(handle, 1, subItem1), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 2, subItem2), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 3, subItem3), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 4, subItem4), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 5, subItem5), InvalidItemHandle, "");
    return handle;
}
ItemHandle ListView::AddItem(
      const ConstString& text,
      const ConstString& subItem1,
      const ConstString& subItem2,
      const ConstString& subItem3,
      const ConstString& subItem4,
      const ConstString& subItem5,
      const ConstString& subItem6)
{
    ItemHandle handle = WRAPPER->AddItem(text);
    CHECK(handle != InvalidItemHandle, InvalidItemHandle, "Fail to allocate item for ListView");
    CHECK(WRAPPER->SetItemText(handle, 1, subItem1), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 2, subItem2), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 3, subItem3), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 4, subItem4), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 5, subItem5), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 6, subItem6), InvalidItemHandle, "");
    return handle;
}
ItemHandle ListView::AddItem(
      const ConstString& text,
      const ConstString& subItem1,
      const ConstString& subItem2,
      const ConstString& subItem3,
      const ConstString& subItem4,
      const ConstString& subItem5,
      const ConstString& subItem6,
      const ConstString& subItem7)
{
    ItemHandle handle = WRAPPER->AddItem(text);
    CHECK(handle != InvalidItemHandle, InvalidItemHandle, "Fail to allocate item for ListView");
    CHECK(WRAPPER->SetItemText(handle, 1, subItem1), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 2, subItem2), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 3, subItem3), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 4, subItem4), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 5, subItem5), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 6, subItem6), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 7, subItem7), InvalidItemHandle, "");
    return handle;
}
ItemHandle ListView::AddItem(
      const ConstString& text,
      const ConstString& subItem1,
      const ConstString& subItem2,
      const ConstString& subItem3,
      const ConstString& subItem4,
      const ConstString& subItem5,
      const ConstString& subItem6,
      const ConstString& subItem7,
      const ConstString& subItem8)
{
    ItemHandle handle = WRAPPER->AddItem(text);
    CHECK(handle != InvalidItemHandle, InvalidItemHandle, "Fail to allocate item for ListView");
    CHECK(WRAPPER->SetItemText(handle, 1, subItem1), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 2, subItem2), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 3, subItem3), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 4, subItem4), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 5, subItem5), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 6, subItem6), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 7, subItem7), InvalidItemHandle, "");
    CHECK(WRAPPER->SetItemText(handle, 8, subItem8), InvalidItemHandle, "");
    return handle;
}

bool ListView::SetItemText(ItemHandle item, uint32 subItem, const ConstString& text)
{
    return WRAPPER->SetItemText(item, subItem, text);
}
const Graphics::CharacterBuffer& ListView::GetItemText(ItemHandle item, uint32 subItemIndex)
{
    auto obj = WRAPPER->GetItemText(item, subItemIndex);
    if (obj)
        return *obj;
    else
    {
        __temp_listviewitem_reference_object__.Destroy();
        return __temp_listviewitem_reference_object__;
    }
}
bool ListView::SetItemCheck(ItemHandle item, bool check)
{
    return WRAPPER->SetItemCheck(item, check);
}
bool ListView::SetItemSelect(ItemHandle item, bool select)
{
    return WRAPPER->SetItemSelect(item, select);
}
bool ListView::SetItemColor(ItemHandle item, ColorPair col)
{
    return WRAPPER->SetItemColor(item, col);
}
bool ListView::SetItemType(ItemHandle item, ListViewItemType type)
{
    return WRAPPER->SetItemType(item, type);
}
bool ListView::IsItemChecked(ItemHandle item)
{
    return WRAPPER->IsItemChecked(item);
}
bool ListView::IsItemSelected(ItemHandle item)
{
    return WRAPPER->IsItemSelected(item);
}
bool ListView::SetItemDataAsPointer(ItemHandle item, GenericRef Data)
{
    return WRAPPER->SetItemDataAsPointer(item, Data);
}
GenericRef ListView::GetItemDataAsPointer(ItemHandle item) const
{
    return WRAPPER->GetItemDataAsPointer(item);
}
bool ListView::SetItemData(ItemHandle item, uint64 value)
{
    return WRAPPER->SetItemDataAsValue(item, value);
}
uint64 ListView::GetItemData(ItemHandle item, uint64 errorValue)
{
    uint64 value;
    CHECK(WRAPPER->GetItemDataAsValue(item, value), errorValue, "");
    return value;
}
bool ListView::SetItemXOffset(ItemHandle item, uint32 XOffset)
{
    return WRAPPER->SetItemXOffset(item, XOffset);
}
uint32 ListView::GetItemXOffset(ItemHandle item)
{
    return WRAPPER->GetItemXOffset(item);
}
bool ListView::SetItemHeight(ItemHandle item, uint32 Height)
{
    return WRAPPER->SetItemHeight(item, Height);
}
uint32 ListView::GetItemHeight(ItemHandle item)
{
    return WRAPPER->GetItemHeight(item);
}
void ListView::DeleteAllItems()
{
    if (Context != nullptr)
    {
        WRAPPER->DeleteAllItems();
    }
}
uint32 ListView::GetItemsCount()
{
    if (Context != nullptr)
    {
        return (uint32) WRAPPER->Items.List.size();
    }
    return 0;
}
ItemHandle ListView::GetCurrentItem()
{
    ListViewControlContext* lvcc = ((ListViewControlContext*) this->Context);
    if ((lvcc->Items.CurentItemIndex < 0) || (lvcc->Items.CurentItemIndex >= (int) lvcc->Items.Indexes.Len()))
        return InvalidItemHandle;
    uint32* indexes = lvcc->Items.Indexes.GetUInt32Array();
    return indexes[lvcc->Items.CurentItemIndex];
}
bool ListView::SetCurrentItem(ItemHandle item)
{
    ListViewControlContext* lvcc = ((ListViewControlContext*) this->Context);
    uint32* indexes              = lvcc->Items.Indexes.GetUInt32Array();
    uint32 count                 = lvcc->Items.Indexes.Len();
    if (count <= 0)
        return false;
    // caut indexul
    for (uint32 tr = 0; tr < count; tr++, indexes++)
    {
        if ((*indexes) == item)
            return WRAPPER->SetCurrentIndex(tr);
    }
    return false;
}
void ListView::SelectAllItems()
{
    WRAPPER->SelectAllItems();
}
void ListView::UnSelectAllItems()
{
    WRAPPER->UnSelectAllItems();
}
void ListView::CheckAllItems()
{
    WRAPPER->CheckAllItems();
}
void ListView::UncheckAllItems()
{
    WRAPPER->UncheckAllItems();
}
uint32 ListView::GetCheckedItemsCount()
{
    return WRAPPER->GetCheckedItemsCount();
}

void ListView::SetClipboardSeparator(char ch)
{
    WRAPPER->SetClipboardSeparator(ch);
}
void ListView::OnMouseReleased(int x, int y, Input::MouseButton button)
{
    WRAPPER->OnMouseReleased(x, y, button);
}
void ListView::OnMousePressed(int x, int y, Input::MouseButton button)
{
    WRAPPER->OnMousePressed(x, y, button);
}
bool ListView::OnMouseDrag(int x, int y, Input::MouseButton button)
{
    return WRAPPER->OnMouseDrag(x, y, button);
}
bool ListView::OnMouseWheel(int x, int y, Input::MouseWheel direction)
{
    return WRAPPER->OnMouseWheel(x, y, direction);
}
bool ListView::OnMouseOver(int x, int y)
{
    return WRAPPER->OnMouseOver(x, y);
}
bool ListView::OnMouseLeave()
{
    WRAPPER->Columns.HoverSeparatorColumnIndex = INVALID_COLUMN_INDEX;
    WRAPPER->Columns.HoverColumnIndex          = INVALID_COLUMN_INDEX;
    return true;
}
void ListView::OnFocus()
{
    WRAPPER->Columns.HoverSeparatorColumnIndex = INVALID_COLUMN_INDEX;
    WRAPPER->Columns.HoverColumnIndex          = INVALID_COLUMN_INDEX;
    WRAPPER->Filter.FilterModeEnabled          = false;
    if ((WRAPPER->Flags & ListViewFlags::AllowMultipleItemsSelection) != ListViewFlags::None)
        WRAPPER->UpdateSelectionInfo();
}
void ListView::SetItemCompareFunction(Handlers::ListViewItemComparer fnc, void* Context)
{
    WRAPPER->SortParams.CompareCallbak        = fnc;
    WRAPPER->SortParams.CompareCallbakContext = Context;
}
bool ListView::Sort()
{
    return WRAPPER->Sort();
}
bool ListView::Sort(uint32 columnIndex, bool ascendent)
{
    CHECK(columnIndex < WRAPPER->Columns.Count,
          false,
          "Invalid column index (%d). Should be smaller than %d",
          columnIndex,
          WRAPPER->Columns.Count);
    CHECK((WRAPPER->Flags & ListViewFlags::Sortable) != ListViewFlags::None,
          false,
          "Can not sort items (have you set 'ListViewFlags::Sortable' when creating ListView ?)");
    WRAPPER->SortParams.ColumnIndex = columnIndex;
    WRAPPER->SortParams.Ascendent   = ascendent;
    return WRAPPER->Sort();
}
bool ListView::Reserve(uint32 itemsCount)
{
    WRAPPER->Items.List.reserve(itemsCount);
    return WRAPPER->Items.Indexes.Reserve(itemsCount);
}
} // namespace AppCUI
