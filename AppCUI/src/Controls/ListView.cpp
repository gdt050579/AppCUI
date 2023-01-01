#include "ControlContext.hpp"

namespace AppCUI
{
constexpr uint32 ITEM_FLAG_CHECKED         = 0x0001;
constexpr uint32 ITEM_FLAG_SELECTED        = 0x0002;
constexpr uint32 LISTVIEW_SEARCH_BAR_WIDTH = 12;

#define PREPARE_LISTVIEW_ITEM(index, returnValue)                                                                      \
    CHECK(index < Items.List.size(), returnValue, "Invalid index: %d", index);                                         \
    InternalListViewItem& i = Items.List[index];

#define CONST_PREPARE_LISTVIEW_ITEM(index, returnValue)                                                                \
    CHECK(index < Items.List.size(), returnValue, "Invalid index: %d", index);                                         \
    const InternalListViewItem& i = Items.List[index];

#define WRAPPER ((ListViewControlContext*) this->Context)

Graphics::CharacterBuffer
      __temp_listviewitem_reference_object__; // use this as std::option<const T&> is not available yet

InternalListViewItem::InternalListViewItem() : Data(nullptr)
{
    this->Flags     = 0;
    this->Type      = ListViewItem::Type::Normal;
    this->ItemColor = DefaultColorPair;
    this->Height    = 1;
    this->XOffset   = 0;
}
InternalListViewItem::InternalListViewItem(const InternalListViewItem& obj) : Data(obj.Data)
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
InternalListViewItem::InternalListViewItem(InternalListViewItem&& obj) noexcept : Data(obj.Data)
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

InternalListViewItem* ListViewControlContext::GetFilteredItem(uint32 index)
{
    uint32 idx;
    CHECK(Items.Indexes.Get(index, idx), nullptr, "Fail to get index value for item with ID: %d", index);
    CHECK(idx < Items.List.size(), nullptr, "Invalid index (%d)", idx);
    return &Items.List[idx];
}

void ListViewControlContext::DrawItem(Graphics::Renderer& renderer, InternalListViewItem* item, int y, bool currentItem)
{
    int x = this->Header[0].x;
    int itemStart;
    auto columnsCount        = Header.GetColumnsCount();
    CharacterBuffer* subitem = item->SubItem;
    ColorPair itemCol        = Cfg->Text.Normal;
    ColorPair checkCol, uncheckCol;
    WriteTextParams params(WriteTextFlags::SingleLine | WriteTextFlags::OverwriteColors | WriteTextFlags::ClipToWidth);
    params.Y = y;

    if (Flags & GATTR_ENABLE)
    {
        checkCol   = Cfg->Symbol.Checked;
        uncheckCol = Cfg->Symbol.Unchecked;
    }
    else
    {
        checkCol = uncheckCol = Cfg->Symbol.Inactive;
    }
    // select color based on item type
    switch (item->Type)
    {
    case ListViewItem::Type::Normal:
        itemCol = Cfg->Text.Normal;
        break;
    case ListViewItem::Type::Highlighted:
        itemCol = Cfg->Text.Focused;
        break;
    case ListViewItem::Type::ErrorInformation:
        itemCol = Cfg->Text.Error;
        break;
    case ListViewItem::Type::WarningInformation:
        itemCol = Cfg->Text.Warning;
        break;
    case ListViewItem::Type::GrayedOut:
        itemCol = Cfg->Text.Inactive;
        break;
    case ListViewItem::Type::Emphasized_1:
        itemCol = Cfg->Text.Emphasized1;
        break;
    case ListViewItem::Type::Emphasized_2:
        itemCol = Cfg->Text.Emphasized2;
        break;
    case ListViewItem::Type::Emphasized_3:
        itemCol = Cfg->Text.Emphasized3;
        break;
    case ListViewItem::Type::Category:
        itemCol = Cfg->Text.Highlighted;
        break;
    case ListViewItem::Type::Colored:
        itemCol = item->ItemColor;
        break;
    default:
        break;
    }
    // disable is not active
    if (!(Flags & GATTR_ENABLE))
        itemCol = Cfg->Text.Inactive;
    else
    {
        // if activ and filtered
        if ((this->Filter.filterMode == ListViewFilterMode::CustomHighlight) || (this->Filter.SearchText.Len() > 0))
        {
            params.Flags =
                  static_cast<WriteTextFlags>((uint32) params.Flags - (uint32) WriteTextFlags::OverwriteColors);
        }
    }
    // prepare params
    params.Color = itemCol;

    // for chategory items a special draw is made (only first comlumn is shown)
    if (item->Type == ListViewItem::Type::Category)
    {
        if (Focused)
            renderer.DrawHorizontalLine(x, y, this->Layout.Width - x, Cfg->Border.Focused, true);
        else
            renderer.DrawHorizontalLine(x, y, this->Layout.Width - x, Cfg->Border.Normal, true);
        params.Align = TextAlignament::Center;
        params.Width = this->Layout.Width - 2;
        params.X     = x;
        params.Flags |= WriteTextFlags::LeftMargin | WriteTextFlags::RightMargin;
        if (currentItem)
            params.Color = Cfg->Cursor.Normal;
        renderer.WriteText(*subitem, params);
        return;
    }
    if (item->Height > 1)
    {
        params.Flags = WriteTextFlags::MultipleLines | WriteTextFlags::OverwriteColors |
                       WriteTextFlags::FitTextToWidth | WriteTextFlags::ClipToWidth;
        if ((this->Filter.filterMode == ListViewFilterMode::CustomHighlight) || (this->Filter.SearchText.Len() > 0))
        {
            params.Flags =
                  static_cast<WriteTextFlags>((uint32) params.Flags - (uint32) WriteTextFlags::OverwriteColors);
        }
    }
    // first column
    const auto& firstColumn = this->Header[0];
    int end_first_column    = x + ((int) firstColumn.width);
    x += (int) item->XOffset;
    this->Header.SetColumnClipRect(renderer, 0);
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

    if (x < end_first_column)
    {
        params.Width = end_first_column - x;
        params.X     = x;
        params.Align = firstColumn.align;
        renderer.WriteText(*subitem, params);
    }
    // rest of the columns
    itemStart = x;
    x         = end_first_column + 1;
    subitem++;

    for (uint32 tr = 1; (tr < columnsCount) && (x < (int) this->Layout.Width); tr++)
    {
        if (this->Header.SetColumnClipRect(renderer, tr))
        {
            const auto& column = this->Header[tr];
            params.Width       = column.width;
            params.X           = column.x;
            params.Align       = column.align;
            renderer.WriteText(*subitem, params);
        }
        subitem++;
    }
    // set the viewing clip
    if (((((uint32) Flags) & ((uint32) ListViewFlags::HideBorder)) == 0))
        renderer.SetClipMargins(1, 1, 1, 1);
    else
        renderer.ResetClip();
    if (Focused)
    {
        if (currentItem)
        {
            if (((Flags & ListViewFlags::AllowMultipleItemsSelection) != ListViewFlags::None) &&
                (item->Flags & ITEM_FLAG_SELECTED))
                renderer.FillRectSize(
                      this->Header.GetX(),
                      y,
                      this->Header.GetHeaderWidth(),
                      item->Height,
                      -1,
                      Cfg->Cursor.OverSelection);
            else
                renderer.FillRectSize(
                      this->Header.GetX(), y, this->Header.GetHeaderWidth(), item->Height, -1, Cfg->Cursor.Normal);
            if ((Flags & ListViewFlags::CheckBoxes) != ListViewFlags::None)
                renderer.SetCursor(itemStart - 2, y); // point the cursor to the check/uncheck
        }
        else
        {
            if (((Flags & ListViewFlags::AllowMultipleItemsSelection) != ListViewFlags::None) &&
                (item->Flags & ITEM_FLAG_SELECTED))
                renderer.FillRectSize(
                      this->Header.GetX(), y, this->Header.GetHeaderWidth(), item->Height, -1, Cfg->Selection.Text);
        }
    }
    else
    {
        if (Flags & GATTR_ENABLE)
        {
            if (((Flags & ListViewFlags::HideCurrentItemWhenNotFocused) == ListViewFlags::None) && (currentItem))
                renderer.FillRectSize(
                      this->Header.GetX(), y, this->Layout.Width, item->Height, -1, Cfg->Cursor.Inactive);
            if (((Flags & ListViewFlags::AllowMultipleItemsSelection) != ListViewFlags::None) &&
                (item->Flags & ITEM_FLAG_SELECTED))
                renderer.FillRectSize(
                      this->Header.GetX(), y, this->Layout.Width, item->Height, -1, Cfg->Cursor.Inactive);
        }
    }
    if ((Flags & ListViewFlags::ItemSeparators) != ListViewFlags::None)
    {
        y += item->Height;
        ColorPair col = this->Cfg->Lines.Normal;
        if (!(this->Flags & GATTR_ENABLE))
            col = this->Cfg->Lines.Inactive;
        else if (Focused)
            col = this->Cfg->Lines.Focused;
        renderer.DrawHorizontalLine(0, y, Layout.Width, col);
        // draw crosses
        if ((Flags & ListViewFlags::HideColumnsSeparator) == ListViewFlags::None)
        {
            for (uint32 tr = 0; (tr < columnsCount) && (x < (int) this->Layout.Width); tr++)
            {
                const auto& column = this->Header[tr];
                x                  = column.x + (int32) column.width;
                if (column.leftClip <= column.rightClip)
                    renderer.WriteSpecialCharacter(
                          x,
                          y,
                          (tr + 1) == this->Header.GetFrozenColumnsCount() ? SpecialChars::BoxVerticalDoubleLine
                                                                           : SpecialChars::BoxCrossSingleLine,
                          col);
            }
        }
    }
}
bool ListViewControlContext::DrawSearchBar(Graphics::Renderer& renderer)
{
    if (Flags && ListViewFlags::HideSearchBar)
        return false; // search bar will not be drawn
    if (this->Layout.Width < ((int) LISTVIEW_SEARCH_BAR_WIDTH + 6))
        return false; // width is too small to render the seach bar
    int x, y;
    if (Flags && ListViewFlags::PopupSearchBar)
    {
        if (this->Filter.filterMode != ListViewFilterMode::Search)
            return false; // Popup search bar is only visible when FilterModeEnabled is true
        x = (this->Layout.Width - LISTVIEW_SEARCH_BAR_WIDTH) / 2;
        y = this->Layout.Height >= 4 ? (this->Layout.Height - 3) : this->Layout.Height;
    }
    else
    {
        x = 2;
        y = ((int) this->Layout.Height) - 1;
    }
    renderer.FillHorizontalLine(x, y, LISTVIEW_SEARCH_BAR_WIDTH + x + 1, ' ', Cfg->SearchBar.Focused);
    const auto search_text = this->Filter.SearchText.ToStringView();
    if (search_text.length() < LISTVIEW_SEARCH_BAR_WIDTH)
    {
        renderer.WriteSingleLineText(x + 1, y, search_text, Cfg->SearchBar.Focused);
        if (this->Filter.filterMode == ListViewFilterMode::Search)
            renderer.SetCursor((int) (x + 1 + search_text.length()), y);
    }
    else
    {
        renderer.WriteSingleLineText(
              x + 1,
              y,
              search_text.substr(search_text.length() - LISTVIEW_SEARCH_BAR_WIDTH, LISTVIEW_SEARCH_BAR_WIDTH),
              Cfg->SearchBar.Focused);
        if (this->Filter.filterMode == ListViewFilterMode::Search)
            renderer.SetCursor(x + 1 + LISTVIEW_SEARCH_BAR_WIDTH, y);
    }
    return true;
}
void ListViewControlContext::Paint(Graphics::Renderer& renderer)
{
    int y     = (Flags && ListViewFlags::HideColumns) ? 0 : 1;
    auto colB = this->Cfg->Border.GetColor(this->GetControlState(ControlStateFlags::ProcessHoverStatus));
    const int itemSeparatorHeight = (Flags && ListViewFlags::ItemSeparators) ? 1 : 0;

    if (((((uint32) Flags) & ((uint32) ListViewFlags::HideBorder)) == 0))
    {
        renderer.DrawRectSize(0, 0, this->Layout.Width, this->Layout.Height, colB, LineType::Single);
        renderer.SetClipMargins(1, 1, 1, 1);
        y++;
    }

    uint32 index = this->Items.FirstVisibleIndex;
    uint32 count = this->Items.Indexes.Len();
    while ((y < this->Layout.Height) && (index < count))
    {
        InternalListViewItem* item = GetFilteredItem(index);
        DrawItem(renderer, item, y, index == static_cast<unsigned>(this->Items.CurentItemIndex));
        y += item->Height;
        y += itemSeparatorHeight;
        index++;
    }

    // filtering & status
    if (Focused)
    {
        int x_ofs = 2;
        int yPoz  = ((int) this->Layout.Height) - 1;
        renderer.ResetClip();
        if (DrawSearchBar(renderer))
            x_ofs += 15;

        // status information
        if ((this->Flags & ListViewFlags::AllowMultipleItemsSelection) != ListViewFlags::None)
        {
            renderer.WriteSingleLineText(
                  x_ofs,
                  yPoz,
                  string_view(this->Selection.Status, this->Selection.StatusLength),
                  Cfg->Text.Highlighted);
        }
    }
}

ItemHandle ListViewControlContext::AddItem(const ConstString& text)
{
    ItemHandle idx = (uint32) Items.List.size();
    Items.List.push_back(InternalListViewItem(Cfg->Text.Normal));
    Items.Indexes.Push(idx);
    SetItemText(idx, 0, text);
    return idx;
}
bool ListViewControlContext::SetItemText(ItemHandle item, uint32 subItem, const ConstString& text)
{
    PREPARE_LISTVIEW_ITEM(item, false);
    CHECK(subItem < Header.GetColumnsCount(),
          false,
          "Invalid column index (%d), should be smaller than %d",
          subItem,
          Header.GetColumnsCount());
    CHECK(subItem < MAX_LISTVIEW_COLUMNS, false, "Subitem must be smaller than 64");
    CHECK(i.SubItem[subItem].Set(text), false, "Fail to set text to a sub-item: %s", text);
    if (this->Filter.filterMode != ListViewFilterMode::None)
        i.SubItem[subItem].SetColor(this->Cfg->Text.Inactive);
    return true;
}
Graphics::CharacterBuffer* ListViewControlContext::GetItemText(ItemHandle item, uint32 subItem)
{
    PREPARE_LISTVIEW_ITEM(item, nullptr);
    CHECK(subItem < Header.GetColumnsCount(),
          nullptr,
          "Invalid column index (%d), should be smaller than %d",
          subItem,
          Header.GetColumnsCount());
    CHECK(subItem < MAX_LISTVIEW_COLUMNS, nullptr, "Subitem must be smaller than 64");
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
    i.ItemColor = color;
    i.Type      = ListViewItem::Type::Colored;
    return true;
}
bool ListViewControlContext::SetItemType(ItemHandle item, ListViewItem::Type type)
{
    PREPARE_LISTVIEW_ITEM(item, false);
    i.Type = type;
    if (type == ListViewItem::Type::Colored)
        i.ItemColor = Cfg->Text.Normal;
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
bool ListViewControlContext::HighlightText(ItemHandle item, uint32 subItemIndex, uint32 offset, uint32 charactersCount)
{
    PREPARE_LISTVIEW_ITEM(item, false);
    CHECK(subItemIndex < Header.GetColumnsCount(),
          false,
          "Invalid column index (%d), should be smaller than %d",
          subItemIndex,
          Header.GetColumnsCount());
    CHECK(subItemIndex < MAX_LISTVIEW_COLUMNS, false, "Subitem must be smaller than 64");
    i.SubItem[subItemIndex].SetColor(offset, offset + charactersCount, this->Cfg->Selection.SearchMarker);
    return true;
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

void ListViewControlContext::SetSearchString(const ConstString& text)
{
    Filter.filterMode = ListViewFilterMode::Search;
    Filter.SearchText.Set(text);
    UpdateSearch(0);
}
void ListViewControlContext::EnableCustomHighlightingMode()
{
    SetSearchString("");
    Filter.filterMode = ListViewFilterMode::CustomHighlight;

    // change colors for all items
    auto columnsCount = this->Header.GetColumnsCount();
    for (auto& lvi : this->Items.List)
    {
        // clear all colors
        for (uint32 gr = 0; gr < columnsCount; gr++)
        {
            lvi.SubItem[gr].SetColor(this->Cfg->Text.Inactive);
        }
    }
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
    Items.FirstVisibleIndex = 0;
    Items.CurentItemIndex   = 0;
    Filter.filterMode       = ListViewFilterMode::None;
    Filter.SearchText.Clear();
}
// movement
int ListViewControlContext::GetVisibleItemsCount()
{
    int vis = Layout.Height - 3;
    if (Flags && ListViewFlags::HideColumns)
        vis++;
    if (Flags && ListViewFlags::HideBorder)
        vis += 2;
    int dim = 0, poz = Items.FirstVisibleIndex, nrItems = 0;
    int sz = (int) Items.Indexes.Len();
    while ((dim < vis) && (poz < sz))
    {
        InternalListViewItem* i = GetFilteredItem(poz);
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
        this->Selection.Count        = count;
        return;
    }
    this->Selection.Status[0]    = 0;
    this->Selection.StatusLength = 0;
    this->Selection.Count        = 0;
}
void ListViewControlContext::UpdateSelection(int start, int end, bool select)
{
    InternalListViewItem* i;
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
        TriggerListViewItemChangedEvent();
}
void ListViewControlContext::CopyToClipboard(bool justCurrentItem)
{
    LocalUnicodeStringBuilder<256> temp;

    if (Items.Indexes.Len() == 0)
        return;
    ColumnsHeaderView::TableBuilder tb(this->Host, temp);
    if (!tb.Start())
        return;

    if (justCurrentItem)
    {
        if (!tb.AddNewRow())
            return;
        for (uint32 tr = 0; tr < Header.GetColumnsCount(); tr++)
        {
            if (!tb.AddString(tr, (CharacterView) (GetFilteredItem(Items.CurentItemIndex)->SubItem[tr])))
                return;
        }
    }
    else
    {
        // copy all selected items
        for (uint32 gr = 0; gr < Items.Indexes.Len(); gr++)
        {
            if ((GetFilteredItem(gr)->Flags & ITEM_FLAG_SELECTED) == 0)
                continue;
            if (!tb.AddNewRow())
                return;
            for (uint32 tr = 0; tr < Header.GetColumnsCount(); tr++)
            {
                if (!tb.AddString(tr, (CharacterView) (GetFilteredItem(gr)->SubItem[tr])))
                    return;
            }
        }
    }
    if (!tb.Finalize())
        return;
    OS::Clipboard::SetText(temp);
}
bool ListViewControlContext::OnKeyEvent(Input::Key keyCode, char16 UnicodeChar)
{
    InternalListViewItem* lvi;
    bool selected;

    if ((Flags & ListViewFlags::AllowMultipleItemsSelection) != ListViewFlags::None)
    {
        lvi                   = GetFilteredItem(Items.CurentItemIndex);
        auto currentItemIndex = Items.CurentItemIndex;
        if (lvi != nullptr)
            selected = ((lvi->Flags & ITEM_FLAG_SELECTED) != 0);
        else
            selected = false;
        switch (keyCode)
        {
        case Key::Shift | Key::Up:
            UpdateSelection(Items.CurentItemIndex, Items.CurentItemIndex - 1, !selected);
            MoveTo(Items.CurentItemIndex - 1);
            if (Filter.filterMode == ListViewFilterMode::Search)
                Filter.filterMode = ListViewFilterMode::None;
            TriggerSelectionChangeEvent(currentItemIndex);
            return true;
        case Key::Insert:
        case Key::Down | Key::Shift:
            UpdateSelection(Items.CurentItemIndex, Items.CurentItemIndex + 1, !selected);
            MoveTo(Items.CurentItemIndex + 1);
            if (Filter.filterMode == ListViewFilterMode::Search)
                Filter.filterMode = ListViewFilterMode::None;
            TriggerSelectionChangeEvent(currentItemIndex);
            return true;
        case Key::PageUp | Key::Shift:
            UpdateSelection(Items.CurentItemIndex, Items.CurentItemIndex - GetVisibleItemsCount(), !selected);
            MoveTo(Items.CurentItemIndex - GetVisibleItemsCount());
            if (Filter.filterMode == ListViewFilterMode::Search)
                Filter.filterMode = ListViewFilterMode::None;
            TriggerSelectionChangeEvent(currentItemIndex);
            return true;
        case Key::PageDown | Key::Shift:
            UpdateSelection(Items.CurentItemIndex, Items.CurentItemIndex + GetVisibleItemsCount(), !selected);
            MoveTo(Items.CurentItemIndex + GetVisibleItemsCount());
            if (Filter.filterMode == ListViewFilterMode::Search)
                Filter.filterMode = ListViewFilterMode::None;
            TriggerSelectionChangeEvent(currentItemIndex);
            return true;
        case Key::Home | Key::Shift:
            UpdateSelection(Items.CurentItemIndex, 0, !selected);
            MoveTo(0);
            if (Filter.filterMode == ListViewFilterMode::Search)
                Filter.filterMode = ListViewFilterMode::None;
            TriggerSelectionChangeEvent(currentItemIndex);
            return true;
        case Key::End | Key::Shift:
            UpdateSelection(Items.CurentItemIndex, Items.Indexes.Len(), !selected);
            MoveTo(Items.Indexes.Len());
            if (Filter.filterMode == ListViewFilterMode::Search)
                Filter.filterMode = ListViewFilterMode::None;
            TriggerSelectionChangeEvent(currentItemIndex);
            return true;
        };
    }
    switch (keyCode)
    {
    case Key::Up:
        MoveTo(Items.CurentItemIndex - 1);
        if (Filter.filterMode == ListViewFilterMode::Search)
            Filter.filterMode = ListViewFilterMode::None;
        return true;
    case Key::Down:
        MoveTo(Items.CurentItemIndex + 1);
        if (Filter.filterMode == ListViewFilterMode::Search)
            Filter.filterMode = ListViewFilterMode::None;
        return true;
    case Key::PageUp:
        MoveTo(Items.CurentItemIndex - GetVisibleItemsCount());
        if (Filter.filterMode == ListViewFilterMode::Search)
            Filter.filterMode = ListViewFilterMode::None;
        return true;
    case Key::PageDown:
        MoveTo(Items.CurentItemIndex + GetVisibleItemsCount());
        if (Filter.filterMode == ListViewFilterMode::Search)
            Filter.filterMode = ListViewFilterMode::None;
        return true;
    case Key::Home:
        MoveTo(0);
        if (Filter.filterMode == ListViewFilterMode::Search)
            Filter.filterMode = ListViewFilterMode::None;
        return true;
    case Key::End:
        MoveTo(((int) Items.Indexes.Len()) - 1);
        if (Filter.filterMode == ListViewFilterMode::Search)
            Filter.filterMode = ListViewFilterMode::None;
        return true;
    case Key::Backspace:
        if ((Flags & ListViewFlags::HideSearchBar) == ListViewFlags::None)
        {
            Filter.filterMode = ListViewFilterMode::Search;
            if (Filter.SearchText.Len() > 0)
            {
                Filter.SearchText.Truncate(Filter.SearchText.Len() - 1);
                UpdateSearch(0);
            }
        }
        return true;
    case Key::Space:
        if (Filter.filterMode == ListViewFilterMode::Search)
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
            TriggerListViewItemCheckedEvent();
        }
        else
        {
            if ((Flags & ListViewFlags::HideSearchBar) == ListViewFlags::None)
            {
                Filter.SearchText.AddChar(' ');
                UpdateSearch(0);
            }
            else
            {
                lvi = GetFilteredItem(Items.CurentItemIndex);
                if (lvi != nullptr)
                {
                    if ((lvi->Flags & ITEM_FLAG_CHECKED) != 0)
                        lvi->Flags -= ITEM_FLAG_CHECKED;
                    else
                        lvi->Flags |= ITEM_FLAG_CHECKED;
                }
                TriggerListViewItemCheckedEvent();
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
        TriggerListViewItemPressedEvent();
        return true;
    case Key::Escape:
        if ((Flags & ListViewFlags::HideSearchBar) == ListViewFlags::None)
        {
            if (Filter.filterMode == ListViewFilterMode::Search)
            {
                if (Filter.SearchText.Len() > 0)
                {
                    Filter.SearchText.Clear();
                    UpdateSearch(0);
                }
                else
                {
                    Filter.filterMode = ListViewFilterMode::None;
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

    case Key::Ctrl | Key::C:
    case Key::Ctrl | Key::Insert:
        CopyToClipboard(this->Selection.Count == 0);
        return true;
    case Key::Ctrl | Key::Alt | Key::Insert:
        CopyToClipboard(true);
        return true;
    };
    // search mode
    if ((!(Flags && ListViewFlags::HideSearchBar)) && (UnicodeChar > 0))
    {
        Filter.filterMode = ListViewFilterMode::Search;
        Filter.SearchText.AddChar(UnicodeChar);
        UpdateSearch(0);
        return true;
    }

    return false;
}

void ListViewControlContext::OnMousePressed(int x, int y, Input::MouseButton button)
{
    // check is the search bar was pressed
    if ((Flags && (ListViewFlags::HideBorder | ListViewFlags::HideSearchBar | ListViewFlags::PopupSearchBar)) == false)
    {
        if ((this->Layout.Width > 20) && (y == (this->Layout.Height - 1)))
        {
            if ((x >= 2) && (x <= (3 + (int32) LISTVIEW_SEARCH_BAR_WIDTH)))
            {
                this->Filter.filterMode = ListViewFilterMode::Search;
                return;
            }
        }
    }

    // check if items are pressed
    if (Flags && ListViewFlags::HideBorder)
    {
        if (!(Flags && ListViewFlags::HideColumns))
            y--;
    }
    else
    {
        if (Flags && ListViewFlags::HideColumns)
            y--;
        else
            y -= 2;
    }

    const int itemSeparators = (Flags && ListViewFlags::ItemSeparators) ? 1 : 0;
    auto cnt                 = GetVisibleItemsCount();
    auto idx                 = this->Items.FirstVisibleIndex;
    auto leftPos             = GetLeftPos();
    int pozY                 = 0;
    while (cnt > 0)
    {
        InternalListViewItem* i = GetFilteredItem(idx);
        if (i == nullptr)
            break;
        int next = pozY + i->Height + itemSeparators;
        if ((y >= pozY) && (y < next))
        {
            // found an item
            if (idx != this->Items.CurentItemIndex)
                MoveTo(idx);
            if (idx == this->Items.CurentItemIndex)
            {
                auto i = GetFilteredItem(Items.CurentItemIndex);
                if (x == (leftPos + (int) i->XOffset))
                {
                    if ((i->Flags & ITEM_FLAG_CHECKED) != 0)
                        i->Flags -= ITEM_FLAG_CHECKED;
                    else
                        i->Flags |= ITEM_FLAG_CHECKED;
                    TriggerListViewItemCheckedEvent();
                }
                else
                {
                    if (((button & Input::MouseButton::DoubleClicked) != Input::MouseButton::None))
                        TriggerListViewItemPressedEvent();
                }
            }
        }
        pozY = next;
        idx++;
        cnt--;
    }
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

void ListViewControlContext::Sort(uint32 columnIndex)
{
    if (!Header.SetSortColumn(columnIndex))
        return;
    Sort();
}
//----------------------

int SortIndexesCompareFunction(uint32 index_1, uint32 index_2, void* context)
{
    ListViewControlContext* lvcc = (ListViewControlContext*) context;

    const uint32 itemsCount = (uint32) lvcc->Items.List.size();
    if ((index_1 < itemsCount) && (index_2 < itemsCount))
    {
        // valid indexes
        if ((lvcc->handlers) && ((Handlers::ListView*) (lvcc->handlers.get()))->ComparereItem.obj)
        {
            return ((Handlers::ListView*) (lvcc->handlers.get()))
                  ->ComparereItem.obj->CompareItems(
                        lvcc->Host, lvcc->Host->GetItem(index_1), lvcc->Host->GetItem(index_2));
        }
        else
        {
            const auto sortColumnIndex = lvcc->Header.GetSortColumnIndex();
            if (sortColumnIndex.has_value())
            {
                const auto sortColIndex = lvcc->Header.GetSortColumnIndex();
                return lvcc->Items.List[index_1].SubItem[sortColIndex.value()].CompareWith(
                      lvcc->Items.List[index_2].SubItem[sortColIndex.value()], true);
            }
            else
            {
                if (index_1 < index_2)
                    return -1;
                else if (index_1 > index_2)
                    return 1;
                else
                    return 0;
            }
        }
    }
    else
    {
        return 0; // dont chage the order
    }
}
bool ListViewControlContext::Sort()
{
    // sanity check
    CHECK(Header.GetSortColumnIndex().has_value(), false, "");
    Items.Indexes.Sort(SortIndexesCompareFunction, Header.GetSortDirection(), this);
    return true;
}
bool ListViewControlContext::Sort(uint32 columnIndex, SortDirection direction)
{
    CHECK(Header.SetSortColumn(columnIndex, direction), false, "");
    return Sort();
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
bool ListViewControlContext::FilterItem(InternalListViewItem& lvi, bool clearColorForAll)
{
    uint32 columnID         = 0;
    int index               = -1;
    const auto columnsCount = Header.GetColumnsCount();

    for (uint32 gr = 0; gr < columnsCount; gr++)
    {
        if ((Header[gr].flags & InternalColumnFlags::SearcheableValue) == InternalColumnFlags::None)
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
        for (uint32 gr = 0; gr < columnsCount; gr++)
        {
            lvi.SubItem[gr].SetColor(this->Cfg->Text.Inactive);
        }
    }
    if (index >= 0)
    {
        // set color for
        lvi.SubItem[columnID].SetColor(index, index + this->Filter.SearchText.Len(), this->Cfg->Selection.SearchMarker);
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
    TriggerListViewItemChangedEvent();
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
    if (Filter.SearchText.Len() == 0)
        this->Filter.filterMode = ListViewFilterMode::None;
}
void ListViewControlContext::SendMsg(Event eventType)
{
    Host->RaiseEvent(eventType);
}
void ListViewControlContext::TriggerSelectionChangeEvent(uint32 itemIndex)
{
    if (this->handlers)
    {
        auto lvh = (Handlers::ListView*) (this->handlers.get());
        if (lvh->OnItemSelected.obj)
        {
            lvh->OnItemSelected.obj->OnListViewItemSelected(this->Host, this->Host->GetItem(itemIndex));
        }
    }
    Host->RaiseEvent(Event::ListViewSelectionChanged);
}
void ListViewControlContext::TriggerListViewItemChangedEvent()
{
    if (this->handlers)
    {
        auto lvh = (Handlers::ListView*) (this->handlers.get());
        if (lvh->OnCurrentItemChanged.obj)
        {
            lvh->OnCurrentItemChanged.obj->OnListViewCurrentItemChanged(this->Host, this->Host->GetCurrentItem());
        }
    }
    Host->RaiseEvent(Event::ListViewCurrentItemChanged);
}
void ListViewControlContext::TriggerListViewItemPressedEvent()
{
    if (this->handlers)
    {
        auto lvh = (Handlers::ListView*) (this->handlers.get());
        if (lvh->OnItemPressed.obj)
        {
            lvh->OnItemPressed.obj->OnListViewItemPressed(this->Host, this->Host->GetCurrentItem());
        }
    }
    Host->RaiseEvent(Event::ListViewItemPressed);
}
void ListViewControlContext::TriggerListViewItemCheckedEvent()
{
    if (this->handlers)
    {
        auto lvh = (Handlers::ListView*) (this->handlers.get());
        if (lvh->OnItemChecked.obj)
        {
            lvh->OnItemChecked.obj->OnListViewItemChecked(this->Host, this->Host->GetCurrentItem());
        }
    }
    Host->RaiseEvent(Event::ListViewItemChecked);
}

uint32 ListViewControlContext::ComputeColumnsPreferedWidth(uint32 columnIndex)
{
    if (columnIndex >= this->Header.GetColumnsCount())
        return 0;
    uint32 colSize = 0;
    uint32 extra   = 0;
    if (columnIndex == 0) // first column
    {
        if (this->Flags && ListViewFlags::CheckBoxes)
            extra += 2;
        for (auto& itm : this->Items.List)
        {
            colSize = std::max<>(colSize, itm.SubItem[columnIndex].Len() + extra + itm.XOffset);
        }
    }
    else
    {
        for (auto& itm : this->Items.List)
        {
            colSize = std::max<>(colSize, itm.SubItem[columnIndex].Len());
        }
    }
    return colSize;
}
//=====================================================================================================
ColumnsHeaderViewFlags ListViewFlagsToColumnsHeaderViewFlags(ListViewFlags flags)
{
    ColumnsHeaderViewFlags f = ColumnsHeaderViewFlags::None;
    if ((flags & ListViewFlags::HideColumnsSeparator) != ListViewFlags::None)
        f |= ColumnsHeaderViewFlags::HideSeparators;
    if ((flags & ListViewFlags::Sortable) != ListViewFlags::None)
        f |= ColumnsHeaderViewFlags::Sortable;
    if ((flags & ListViewFlags::HideColumns) != ListViewFlags::None)
        f |= ColumnsHeaderViewFlags::HideHeader;
    return f;
}
ListView::~ListView()
{
    DeleteAllItems();
    DELETE_CONTROL_CONTEXT(ListViewControlContext);
}
ListView::ListView(string_view layout, std::initializer_list<ConstString> columns, ListViewFlags flags)
    : ColumnsHeaderView(new ListViewControlContext(this, columns, ListViewFlagsToColumnsHeaderViewFlags(flags)), layout)
{
    auto Members              = reinterpret_cast<ListViewControlContext*>(this->Context);
    Members->Layout.MinWidth  = 5;
    Members->Layout.MinHeight = 3;
    Members->Flags            = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | (uint32) flags;

    // PopupSearchBar is implicitelly set if HideBorder is set
    if (Members->Flags && ListViewFlags::HideBorder)
        Members->Flags = Members->Flags | ListViewFlags::PopupSearchBar;
    if (!(Members->Flags && ListViewFlags::HideScrollBar))
    {
        Members->Flags |= (GATTR_HSCROLL | GATTR_VSCROLL);
        Members->ScrollBars.LeftMargin = 25;
    }
    // allocate items
    ASSERT(Members->Items.Indexes.Create(32), "Fail to allocate Listview indexes");
    Members->Items.List.reserve(32);

    // initialize
    Members->Items.FirstVisibleIndex   = 0;
    Members->Items.CurentItemIndex     = 0;
    Members->Filter.filterMode         = ListViewFilterMode::None;
    Members->Filter.LastFoundItem      = -1;
    Members->Host                      = this;
    Members->ScrollBars.OutsideControl = Members->Flags && ListViewFlags::HideBorder;
    Members->Filter.SearchText.Clear();
    Members->Selection.Status[0]    = 0;
    Members->Selection.StatusLength = 0;
    Members->Selection.Count        = 0;
}
void ListView::Paint(Graphics::Renderer& renderer)
{
    // paint columns
    ColumnsHeaderView::Paint(renderer);
    // paint items
    CREATE_TYPECONTROL_CONTEXT(ListViewControlContext, Members, );
    Members->Paint(renderer);
}
bool ListView::OnKeyEvent(Input::Key keyCode, char16 UnicodeChar)
{
    if (ColumnsHeaderView::OnKeyEvent(keyCode, UnicodeChar))
        return true;
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
    const auto columnsWidth        = (uint32) Members->Header.GetColumnsWidth();
    const auto headerWidth         = (uint32) (Members->Header.GetHeaderWidth() + 1);
    if (columnsWidth > headerWidth)
        UpdateHScrollBar(Members->Header.GetScrollX(), columnsWidth - headerWidth);
    else
        UpdateHScrollBar(Members->Header.GetScrollX(), 0);
    uint32 count = Members->Items.Indexes.Len();
    if (count > 0)
        count--;
    UpdateVScrollBar(Members->Items.CurentItemIndex, count);
}

ListViewItem ListView::AddItem(const ConstString& text)
{
    return { this->Context, WRAPPER->AddItem(text) };
}
ListViewItem ListView::AddItem(std::initializer_list<ConstString> values)
{
    ItemHandle handle = WRAPPER->AddItem("");
    CHECK(handle != InvalidItemHandle, ListViewItem(nullptr, InvalidItemHandle), "Fail to allocate item for ListView");
    auto index = 0U;
    for (auto& value : values)
    {
        WRAPPER->SetItemText(handle, index++, value);
    }
    return { this->Context, handle };
}
void ListView::AddItems(std::initializer_list<std::initializer_list<ConstString>> items)
{
    for (auto& item : items)
    {
        AddItem(item);
    }
}
ListViewItem ListView::GetItem(uint32 index)
{
    if (this->Context == nullptr)
        return { nullptr, 0 };
    if (index >= WRAPPER->Items.List.size())
        return { nullptr, 0 };
    return { this->Context, index };
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
ListViewItem ListView::GetCurrentItem()
{
    ListViewControlContext* lvcc = ((ListViewControlContext*) this->Context);
    if ((lvcc->Items.CurentItemIndex < 0) || (lvcc->Items.CurentItemIndex >= (int) lvcc->Items.Indexes.Len()))
        return { nullptr, InvalidItemHandle };
    uint32* indexes = lvcc->Items.Indexes.GetUInt32Array();
    return { this->Context, indexes[lvcc->Items.CurentItemIndex] };
}
bool ListView::SetCurrentItem(ListViewItem item)
{
    ListViewControlContext* lvcc = ((ListViewControlContext*) this->Context);
    uint32* indexes              = lvcc->Items.Indexes.GetUInt32Array();
    uint32 count                 = lvcc->Items.Indexes.Len();
    if (count <= 0)
        return false;
    // caut indexul
    for (uint32 tr = 0; tr < count; tr++, indexes++)
    {
        if ((*indexes) == item.item)
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

void ListView::OnMousePressed(int x, int y, Input::MouseButton button)
{
    ColumnsHeaderView::OnMousePressed(x, y, button);
    if (!this->HeaderHasMouseCaption())
        WRAPPER->OnMousePressed(x, y, button);
}

bool ListView::OnMouseWheel(int x, int y, Input::MouseWheel direction)
{
    return WRAPPER->OnMouseWheel(x, y, direction);
}

void ListView::OnFocus()
{
    WRAPPER->Header.RecomputeColumnsSizes();
    if (WRAPPER->Filter.filterMode == ListViewFilterMode::Search)
        WRAPPER->Filter.filterMode = ListViewFilterMode::None;
    if ((WRAPPER->Flags & ListViewFlags::AllowMultipleItemsSelection) != ListViewFlags::None)
        WRAPPER->UpdateSelectionInfo();
}

bool ListView::Sort()
{
    return WRAPPER->Sort();
}
bool ListView::Sort(uint32 columnIndex, SortDirection direction)
{
    return WRAPPER->Sort(columnIndex, direction);
}
void ListView::OnColumnClicked(uint32 /*columnIndex*/)
{
    this->Sort();
}
Rect ListView::GetHeaderLayout()
{
    auto sz          = GetSize();
    auto has_margins = !(WRAPPER->Flags && ListViewFlags::HideBorder);
    if (has_margins)
    {
        if ((sz.Width > 2) && (sz.Height > 2))
        {
            return Graphics::Rect({ 1, 1 }, { sz.Width - 2, sz.Height - 2 });
        }
        else
        {
            return Graphics::Rect();
        }
    }
    else
    {
        return Graphics::Rect({ 0, 0 }, sz);
    }
}
uint32 ListView::ComputeColumnsPreferedWidth(uint32 columnIndex)
{
    return WRAPPER->ComputeColumnsPreferedWidth(columnIndex);
}
bool ListView::Reserve(uint32 itemsCount)
{
    WRAPPER->Items.List.reserve(itemsCount);
    return WRAPPER->Items.Indexes.Reserve(itemsCount);
}
void ListView::SetSearchString(const ConstString& text)
{
    WRAPPER->SetSearchString(text);
}

Handlers::ListView* ListView::Handlers()
{
    GET_CONTROL_HANDLERS(Handlers::ListView);
}

// ================================================================== [ListViewItem] ==========================
#define LVIC ((ListViewControlContext*) this->context)
#define LVICHECK(result)                                                                                               \
    if (this->context == nullptr)                                                                                      \
        return result;
bool ListViewItem::SetData(uint64 value)
{
    LVICHECK(false);
    return LVIC->SetItemDataAsValue(item, value);
}
uint64 ListViewItem::GetData(uint64 errorValue) const
{
    LVICHECK(errorValue);
    uint64 value;
    CHECK(LVIC->GetItemDataAsValue(item, value), errorValue, "");
    return value;
}
bool ListViewItem::SetCheck(bool check)
{
    LVICHECK(false);
    return LVIC->SetItemCheck(item, check);
}
bool ListViewItem::IsChecked() const
{
    LVICHECK(false);
    return LVIC->IsItemChecked(item);
}
bool ListViewItem::SetType(ListViewItem::Type type)
{
    LVICHECK(false);
    return LVIC->SetItemType(item, type);
}
bool ListViewItem::SetText(uint32 subItem, const ConstString& text)
{
    LVICHECK(false);
    return LVIC->SetItemText(item, subItem, text);
}
bool ListViewItem::SetValues(std::initializer_list<ConstString> values)
{
    LVICHECK(false);
    auto idx              = 0U;
    const auto maxColumns = LVIC->Header.GetColumnsCount();
    for (auto& value : values)
    {
        if (idx >= maxColumns)
            break;
        CHECK(LVIC->SetItemText(item, idx, value), false, "Fail to set value for item: %d", idx);
        idx++;
    }
    return true;
}
const Graphics::CharacterBuffer& ListViewItem::GetText(uint32 subItemIndex) const
{
    if (this->context)
    {
        auto obj = LVIC->GetItemText(item, subItemIndex);
        if (obj)
            return *obj;
    }
    // error fallback
    __temp_listviewitem_reference_object__.Destroy();
    return __temp_listviewitem_reference_object__;
}
bool ListViewItem::SetXOffset(uint32 XOffset)
{
    LVICHECK(false);
    return LVIC->SetItemXOffset(item, XOffset);
}
uint32 ListViewItem::GetXOffset() const
{
    LVICHECK(0);
    return LVIC->GetItemXOffset(item);
}
bool ListViewItem::SetColor(ColorPair col)
{
    LVICHECK(false);
    return LVIC->SetItemColor(item, col);
}
bool ListViewItem::SetSelected(bool select)
{
    LVICHECK(false);
    return LVIC->SetItemSelect(item, select);
}
bool ListViewItem::IsSelected() const
{
    LVICHECK(false);
    return LVIC->IsItemSelected(item);
}
bool ListViewItem::IsCurrent() const
{
    LVICHECK(false);
    ListViewControlContext* lvcc = ((ListViewControlContext*) this->context);
    if ((lvcc->Items.CurentItemIndex < 0) || (lvcc->Items.CurentItemIndex >= (int) lvcc->Items.Indexes.Len()))
        return false;
    uint32* indexes = lvcc->Items.Indexes.GetUInt32Array();
    return ((uint32) this->item) == indexes[lvcc->Items.CurentItemIndex];
}

bool ListViewItem::SetHeight(uint32 Height)
{
    LVICHECK(false);
    return LVIC->SetItemHeight(item, Height);
}
uint32 ListViewItem::GetHeight() const
{
    LVICHECK(0);
    return LVIC->GetItemHeight(item);
}
bool ListViewItem::SetItemDataAsPointer(GenericRef Data)
{
    LVICHECK(false);
    return LVIC->SetItemDataAsPointer(item, Data);
}
GenericRef ListViewItem::GetItemDataAsPointer() const
{
    LVICHECK(nullptr);
    return LVIC->GetItemDataAsPointer(item);
}
bool ListViewItem::HighlightText(uint32 subItemIndex, uint32 offset, uint32 charactersCount)
{
    LVICHECK(false);
    // make sure that we enable search mode
    if (LVIC->Filter.filterMode != ListViewFilterMode::CustomHighlight)
        LVIC->EnableCustomHighlightingMode();
    return LVIC->HighlightText(item, subItemIndex, offset, charactersCount);
}

#undef LVICHECK
#undef LVIC
} // namespace AppCUI
