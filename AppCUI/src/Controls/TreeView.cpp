#include "ControlContext.hpp"

#include <queue>

namespace AppCUI::Controls
{
constexpr auto TreeSearchBarWidth      = 23U;
constexpr auto TreeScrollbarLeftOffset = 25U;
constexpr auto ItemSymbolOffset        = 2U;
constexpr auto MinColumnWidth          = 10U;
constexpr auto BorderOffset            = 1U;
constexpr auto InvalidIndex            = 0xFFFFFFFFU;

const static Utils::UnicodeStringBuilder cb{};

ColumnsHeaderViewFlags TreeViewFlagsToColumnsHeaderViewFlags(TreeViewFlags flags)
{
    ColumnsHeaderViewFlags f = ColumnsHeaderViewFlags::None;
    if ((flags & TreeViewFlags::HideColumnsSeparator) != TreeViewFlags::None)
        f |= ColumnsHeaderViewFlags::HideSeparators;
    if ((flags & TreeViewFlags::Sortable) != TreeViewFlags::None)
        f |= ColumnsHeaderViewFlags::Sortable;
    if ((flags & TreeViewFlags::HideColumns) != TreeViewFlags::None)
        f |= ColumnsHeaderViewFlags::HideHeader;
    return f;
}

TreeView::TreeView(string_view layout, std::initializer_list<ConstString> columns, TreeViewFlags flags)
    : ColumnsHeaderView(new TreeControlContext(this, columns, TreeViewFlagsToColumnsHeaderViewFlags(flags)), layout)
{
    const auto cc        = reinterpret_cast<TreeControlContext*>(Context);
    cc->Layout.MinHeight = 1;
    cc->Layout.MaxHeight = 200000;
    cc->Layout.MinWidth  = 20;
    cc->host             = this;

    cc->treeFlags = static_cast<uint32>(flags);

    cc->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;

    if ((cc->treeFlags & TreeViewFlags::HideScrollBar) == TreeViewFlags::None)
    {
        cc->Flags |= GATTR_VSCROLL;
        cc->Flags |= GATTR_HSCROLL;
        cc->ScrollBars.LeftMargin     = TreeScrollbarLeftOffset; // search field
        cc->ScrollBars.TopMargin      = BorderOffset + 1;        // border + column header
        cc->ScrollBars.OutsideControl = false;
    }

    if ((cc->treeFlags & TreeViewFlags::HideSearchBar) == TreeViewFlags::None)
    {
        if ((cc->treeFlags & TreeViewFlags::FilterSearch) != TreeViewFlags::None)
        {
            cc->filter.mode = TreeControlContext::FilterMode::Filter;
        }
        else if ((cc->treeFlags & TreeViewFlags::Searchable) != TreeViewFlags::None)
        {
            cc->filter.mode = TreeControlContext::FilterMode::Search;
        }
        else
        {
            cc->treeFlags |= static_cast<uint32>(TreeViewFlags::HideSearchBar);
        }
    }

    cc->itemsToDrew.reserve(100);
    cc->orderedItems.reserve(100);

    if (cc->treeFlags && TreeViewFlags::Sortable)
    {
        cc->Header.SetSortColumn(InvalidIndex);
        cc->Sort();
    }

    cc->AdjustItemsBoundsOnResize();

    cc->SetColorForItems(cc->Cfg->Text.Normal);
}

void TreeView::Paint(Graphics::Renderer& renderer)
{
    CHECKRET(Context != nullptr, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    ColumnsHeaderView::Paint(renderer);

    if (cc->notProcessed)
    {
        cc->ProcessItemsToBeDrawn(InvalidItemHandle, true);
        cc->notProcessed = false;
    }

    cc->PaintItems(renderer);

    if ((cc->treeFlags & TreeViewFlags::HideBorder) == TreeViewFlags::None)
    {
        renderer.DrawRectSize(0, 0, cc->Layout.Width, cc->Layout.Height, cc->Cfg->Border.Normal, LineType::Single);
    }

    if (cc->Focused)
    {
        if (cc->Layout.Width > TreeSearchBarWidth && cc->filter.mode != TreeControlContext::FilterMode::None)
        {
            renderer.FillHorizontalLine(1, cc->Layout.Height - 1, TreeSearchBarWidth, ' ', cc->Cfg->SearchBar.Normal);

            if (const auto searchTextLen = cc->filter.searchText.Len(); searchTextLen > 0)
            {
                if (const auto searchText = cc->filter.searchText.ToStringView();
                    searchText.length() < TreeSearchBarWidth - 2ULL)
                {
                    renderer.WriteSingleLineText(2, cc->Layout.Height - 1, searchText, cc->Cfg->Text.Normal);
                    renderer.SetCursor((int) (2 + searchText.length()), cc->Layout.Height - 1);
                }
                else
                {
                    renderer.WriteSingleLineText(
                          2,
                          cc->Layout.Height - 1,
                          searchText.substr(searchText.length() - TreeSearchBarWidth + 2, TreeSearchBarWidth - 2),
                          cc->Cfg->Text.Normal);
                    renderer.SetCursor(TreeSearchBarWidth, cc->Layout.Height - 1);
                }
            }
            else if (cc->isMouseOn == TreeControlContext::IsMouseOn::SearchField)
            {
                renderer.SetCursor(2, cc->Layout.Height - 1);
            }
        }
    }
}

bool TreeView::OnKeyEvent(Input::Key keyCode, char16 character)
{
    if (ColumnsHeaderView::OnKeyEvent(keyCode, character))
    {
        return true;
    }

    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    switch (keyCode)
    {
    case Key::Up:
        return cc->MoveUp();
    case Key::Down:
        return cc->MoveDown();
    case Key::Ctrl | Key::Up:
        if (cc->itemsToDrew.size() > 0)
        {
            if (cc->offsetTopToDraw > 0)
            {
                cc->offsetTopToDraw--;
                cc->offsetBotToDraw--;
                return true;
            }
        }
        break;
    case Key::Ctrl | Key::Down:
        if (cc->itemsToDrew.size() > 0)
        {
            if (cc->offsetBotToDraw < cc->itemsToDrew.size())
            {
                cc->offsetTopToDraw++;
                cc->offsetBotToDraw++;
                return true;
            }
        }
        break;
    case Key::PageUp:
        if (cc->itemsToDrew.size() == 0)
        {
            break;
        }

        if (static_cast<size_t>(cc->offsetTopToDraw) > cc->maxItemsToDraw)
        {
            const auto it    = find(cc->itemsToDrew.begin(), cc->itemsToDrew.end(), cc->GetCurrentItemHandle());
            const auto index = static_cast<uint32>(it - cc->itemsToDrew.begin()) - cc->maxItemsToDraw;

            cc->SetCurrentItemHandle(cc->itemsToDrew[index]);

            cc->offsetTopToDraw -= cc->maxItemsToDraw;
            cc->offsetBotToDraw -= cc->maxItemsToDraw;
        }
        else if (cc->offsetTopToDraw > 0)
        {
            const auto difference = cc->offsetTopToDraw;

            const auto it    = find(cc->itemsToDrew.begin(), cc->itemsToDrew.end(), cc->GetCurrentItemHandle());
            const auto index = static_cast<uint32>(it - cc->itemsToDrew.begin()) - difference;

            cc->SetCurrentItemHandle(cc->itemsToDrew[index]);

            cc->offsetTopToDraw -= difference;
            cc->offsetBotToDraw -= difference;
        }
        else
        {
            cc->SetCurrentItemHandle(cc->itemsToDrew[0]);
        }

        return true;
    case Key::PageDown:
        if (cc->itemsToDrew.size() == 0)
        {
            break;
        }

        if (static_cast<size_t>(cc->offsetBotToDraw) + cc->maxItemsToDraw < cc->itemsToDrew.size())
        {
            const auto it    = find(cc->itemsToDrew.begin(), cc->itemsToDrew.end(), cc->GetCurrentItemHandle());
            const auto index = static_cast<uint32>(it - cc->itemsToDrew.begin()) + cc->maxItemsToDraw;

            cc->SetCurrentItemHandle(cc->itemsToDrew[index]);

            cc->offsetTopToDraw += cc->maxItemsToDraw;
            cc->offsetBotToDraw += cc->maxItemsToDraw;
        }
        else if (static_cast<size_t>(cc->offsetBotToDraw) < cc->itemsToDrew.size())
        {
            const auto difference = cc->itemsToDrew.size() - cc->offsetBotToDraw;

            const auto it    = find(cc->itemsToDrew.begin(), cc->itemsToDrew.end(), cc->GetCurrentItemHandle());
            const auto index = static_cast<uint32>(it - cc->itemsToDrew.begin()) + difference;

            cc->SetCurrentItemHandle(cc->itemsToDrew[index]);

            cc->offsetTopToDraw += static_cast<uint32>(difference);
            cc->offsetBotToDraw += static_cast<uint32>(difference);
        }
        else
        {
            cc->SetCurrentItemHandle(cc->itemsToDrew[cc->itemsToDrew.size() - 1]);
        }

        return true;

    case Key::Home:
        if (cc->itemsToDrew.size() == 0)
        {
            break;
        }
        cc->offsetTopToDraw = 0;
        cc->offsetBotToDraw = cc->maxItemsToDraw;

        cc->SetCurrentItemHandle(cc->itemsToDrew[0]);

        return true;

    case Key::End:
        if (cc->itemsToDrew.size() == 0)
        {
            break;
        }
        {
            const auto itemsFitInList = cc->maxItemsToDraw >= static_cast<uint32>(cc->itemsToDrew.size());
            cc->offsetTopToDraw = itemsFitInList ? 0 : static_cast<uint32>(cc->itemsToDrew.size()) - cc->maxItemsToDraw;
            cc->offsetBotToDraw = cc->offsetTopToDraw + cc->maxItemsToDraw;

            cc->SetCurrentItemHandle(cc->itemsToDrew[cc->itemsToDrew.size() - 1]);
        }

        return true;

    case Key::Ctrl | Key::Space:
    {
        GetCurrentItem().ToggleRecursively();

        if (cc->filter.searchText.Len() > 0 && cc->filter.mode != TreeControlContext::FilterMode::None)
        {
            cc->SearchItems();
        }

        return true;
    }
    case Key::Shift | Key::Space:
    case Key::Shift | Key::Enter:
        if (cc->Focused)
        {
            cc->TriggerOnItemPressed();
        }
        return true;

    case Key::Space:
    case Key::Enter:
        if (cc->GetCurrentItemHandle() != InvalidItemHandle)
        {
            if (GetCurrentItem().IsExpandable())
            {
                GetCurrentItem().Toggle();

                cc->ProcessItemsToBeDrawn(InvalidItemHandle);
                if (cc->filter.searchText.Len() > 0 && cc->filter.mode != TreeControlContext::FilterMode::None)
                {
                    cc->SearchItems();
                }
            }
            else
            {
                if (cc->Focused)
                {
                    cc->TriggerOnItemPressed();
                }
            }
        }
        return true;
    case Key::Escape:
        if (cc->filter.mode == TreeControlContext::FilterMode::Search)
        {
            if (cc->filter.searchText.Len() > 0)
            {
                cc->filter.searchText.Clear();
                cc->SetColorForItems(cc->Cfg->Text.Normal);
                return true;
            }
        }
        else if (cc->filter.mode == TreeControlContext::FilterMode::Filter)
        {
            if (cc->filter.searchText.Len() > 0)
            {
                cc->filter.searchText.Clear();
                cc->SetColorForItems(cc->Cfg->Text.Normal);
                cc->ProcessItemsToBeDrawn(InvalidItemHandle);
                return true;
            }
        }
        break;

    case Key::Ctrl | Key::Insert:
        if (const auto it = cc->items.find(cc->GetCurrentItemHandle()); it != cc->items.end())
        {
            LocalUnicodeStringBuilder<1024> lusb;
            for (const auto& value : it->second.values)
            {
                if (lusb.Len() > 0)
                {
                    lusb.Add(" ");
                }
                lusb.Add(value);
            }
            if (OS::Clipboard::SetText(lusb) == false)
            {
                const std::string input{ lusb };
                LOG_WARNING("Fail to copy string [%s] to the clipboard!", input.c_str());
            }
        }
        break;

    case Key::Backspace:
        if (cc->filter.mode != TreeControlContext::FilterMode::None)
        {
            if (cc->filter.searchText.Len() > 0)
            {
                cc->filter.searchText.Truncate(cc->filter.searchText.Len() - 1);
                cc->SearchItems();
                return true;
            }
        }
        break;

    case Key::Ctrl | Key::Enter:
        if (cc->filter.mode == TreeControlContext::FilterMode::Search)
        {
            if (cc->filter.searchText.Len() > 0)
            {
                bool foundCurrent = false;

                for (const auto& handle : cc->orderedItems)
                {
                    if (foundCurrent == false)
                    {
                        foundCurrent = cc->GetCurrentItemHandle() == handle;
                        continue;
                    }

                    const auto& item = cc->items[handle];
                    if (item.markedAsFound == true)
                    {
                        cc->SetCurrentItemHandle(handle);
                        return true;
                    }
                }

                // there's no next so go back to the first
                for (const auto& handle : cc->orderedItems)
                {
                    const auto& item = cc->items[handle];
                    if (item.markedAsFound == true)
                    {
                        cc->SetCurrentItemHandle(handle);
                        return true;
                    }
                }
            }
        }
        break;

    case Key::Ctrl | Key::Shift | Key::Enter:
        if (cc->filter.mode == TreeControlContext::FilterMode::Search)
        {
            if (cc->filter.searchText.Len() > 0)
            {
                bool foundCurrent = false;

                for (auto it = cc->orderedItems.rbegin(); it != cc->orderedItems.rend(); ++it)
                {
                    const auto handle = *it;
                    if (foundCurrent == false)
                    {
                        foundCurrent = cc->GetCurrentItemHandle() == handle;
                        continue;
                    }

                    const auto& item = cc->items[handle];
                    if (item.markedAsFound == true)
                    {
                        cc->SetCurrentItemHandle(handle);
                        return true;
                    }
                }

                // there's no previous so go back to the last
                for (auto it = cc->orderedItems.rbegin(); it != cc->orderedItems.rend(); ++it)
                {
                    const auto handle = *it;
                    const auto& item  = cc->items[handle];
                    if (item.markedAsFound == true)
                    {
                        cc->SetCurrentItemHandle(handle);
                        return true;
                    }
                }
            }
        }
        break;

    default:
        break;
    }

    if (cc->filter.mode != TreeControlContext::FilterMode::None)
    {
        if (character > 0)
        {
            cc->filter.searchText.AddChar(character);
            cc->SetColorForItems(cc->Cfg->Text.Normal);
            if (cc->SearchItems() == false)
            {
                cc->filter.searchText.Truncate(cc->filter.searchText.Len() - 1);
                if (cc->filter.searchText.Len() > 0)
                {
                    cc->SearchItems();
                }
                else
                {
                    cc->notProcessed = true;
                    cc->SetColorForItems(cc->Cfg->Text.Normal);
                }
            }
            return true;
        }
    }

    return false;
}

void TreeView::OnFocus()
{
    CHECKRET(Context != nullptr, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    if (cc->itemsToDrew.size() > 0)
    {
        if (cc->GetCurrentItemHandle() == InvalidItemHandle)
        {
            cc->SetCurrentItemHandle(cc->itemsToDrew[cc->offsetTopToDraw]);
        }
    }

    cc->Header.RecomputeColumnsSizes();
}

void TreeView::OnMousePressed(int x, int y, Input::MouseButton button)
{
    CHECKRET(Context != nullptr, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    cc->isMouseOn = TreeControlContext::IsMouseOn::None;

    if (cc->IsMouseOnBorder(x, y))
    {
        cc->isMouseOn = TreeControlContext::IsMouseOn::Border;
    }
    else if (cc->IsMouseOnColumnSeparator(x, y))
    {
        cc->isMouseOn = TreeControlContext::IsMouseOn::ColumnSeparator;
    }
    else if (cc->IsMouseOnToggleSymbol(x, y))
    {
        cc->isMouseOn = TreeControlContext::IsMouseOn::ToggleSymbol;
    }
    else if (cc->IsMouseOnColumnHeader(x, y))
    {
        cc->isMouseOn = TreeControlContext::IsMouseOn::ColumnHeader;
    }
    else if (cc->IsMouseOnItem(x, y))
    {
        cc->isMouseOn = TreeControlContext::IsMouseOn::Item;
    }
    else if (cc->IsMouseOnSearchField(x, y))
    {
        cc->isMouseOn = TreeControlContext::IsMouseOn::SearchField;
    }

    switch (button)
    {
    case Input::MouseButton::None:
        return ColumnsHeaderView::OnMousePressed(x, y, button);
    case Input::MouseButton::Left:
        switch (cc->isMouseOn)
        {
        case TreeControlContext::IsMouseOn::Border:
            return ColumnsHeaderView::OnMousePressed(x, y, button);
        case TreeControlContext::IsMouseOn::ColumnHeader:
            return ColumnsHeaderView::OnMousePressed(x, y, button);
        case TreeControlContext::IsMouseOn::ColumnSeparator:
            return ColumnsHeaderView::OnMousePressed(x, y, button);
        case TreeControlContext::IsMouseOn::ToggleSymbol:
        {
            const uint32 index    = y - 2;
            const auto itemHandle = cc->itemsToDrew.at(static_cast<size_t>(cc->offsetTopToDraw) + index);
            const auto it         = cc->items.find(itemHandle);
            auto item             = TreeViewItem{ this->Context, it->second.handle };
            item.Toggle();
            if (it->second.expanded == false)
            {
                if (cc->IsAncestorOfChild(it->second.handle, cc->GetCurrentItemHandle()))
                {
                    cc->SetCurrentItemHandle(it->second.handle);
                }
            }
            cc->ProcessItemsToBeDrawn(InvalidItemHandle);
            cc->SearchItems();
        }
        break;
        case TreeControlContext::IsMouseOn::Item:
        {
            const uint32 index = y - 2;
            if (index >= cc->offsetBotToDraw || index >= cc->itemsToDrew.size())
            {
                break;
            }

            const auto itemHandle = cc->itemsToDrew[static_cast<size_t>(cc->offsetTopToDraw) + index];
            const auto it         = cc->items.find(itemHandle);

            if (x > static_cast<int>(it->second.depth * ItemSymbolOffset + ItemSymbolOffset) &&
                x < static_cast<int>(cc->Layout.Width))
            {
                cc->SetCurrentItemHandle(itemHandle);
            }
        }
        break;
        default:
            return ColumnsHeaderView::OnMousePressed(x, y, button);
        }
        break;
    case Input::MouseButton::Center:
        return ColumnsHeaderView::OnMousePressed(x, y, button);
    case Input::MouseButton::Right:
        return ColumnsHeaderView::OnMousePressed(x, y, button);
    case Input::MouseButton::DoubleClicked:
        return ColumnsHeaderView::OnMousePressed(x, y, button);
    default:
        return ColumnsHeaderView::OnMousePressed(x, y, button);
    }
}

bool TreeView::OnMouseWheel(int /*x*/, int /*y*/, Input::MouseWheel direction)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    switch (direction)
    {
    case Input::MouseWheel::None:
        break;
    case Input::MouseWheel::Up:
        return cc->MoveUp();
    case Input::MouseWheel::Down:
        return cc->MoveDown();
    case Input::MouseWheel::Left:
        return OnKeyEvent(Key::Left, 0);
    case Input::MouseWheel::Right:
        return OnKeyEvent(Key::Right, 0);
    default:
        break;
    }

    return false;
}

void TreeView::OnUpdateScrollBars()
{
    CHECKRET(Context != nullptr, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    const auto it     = find(cc->itemsToDrew.begin(), cc->itemsToDrew.end(), cc->GetCurrentItemHandle());
    const int64 index = it - cc->itemsToDrew.begin();
    UpdateVScrollBar(index, std::max<size_t>(cc->itemsToDrew.size() - 1, 0));

    uint32 leftMargin = 2;
    if ((cc->Flags & TreeViewFlags::HideSearchBar) == TreeViewFlags::None)
        leftMargin = TreeScrollbarLeftOffset;

    cc->ScrollBars.LeftMargin = leftMargin;
    const auto columnsWidth   = (uint32) cc->Header.GetColumnsWidth();
    const auto headerWidth    = (uint32) (cc->Header.GetHeaderWidth() + 1);
    if (columnsWidth > headerWidth)
    {
        UpdateHScrollBar(cc->Header.GetScrollX(), (uint64) columnsWidth - headerWidth);
    }
    else
    {
        UpdateHScrollBar(cc->Header.GetScrollX(), 0);
    }
}

void TreeView::OnAfterResize(int newWidth, int newHeight)
{
    CHECKRET(Context != nullptr, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);
    CHECKRET(cc->AdjustElementsOnResize(newWidth, newHeight), "");
    ColumnsHeaderView::OnAfterResize(newWidth, newHeight);
}

void TreeView::OnColumnClicked(uint32 columnIndex)
{
    this->Sort();
}

Graphics::Rect TreeView::GetHeaderLayout()
{
    CHECK(Context != nullptr, Graphics::Rect(), "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    auto sz         = GetSize();
    auto hasMargins = !(cc->Flags && TreeViewFlags::HideBorder);
    if (hasMargins)
    {
        if ((sz.Width > 2) && (sz.Height > 2))
        {
            return Graphics::Rect({ 1, 1 }, { sz.Width - 2, sz.Height - 2 });
        }
        return Graphics::Rect();
    }
    return Graphics::Rect({ 0, 0 }, sz);
}

uint32 TreeView::ComputeColumnsPreferedWidth(uint32 columnIndex)
{
    CHECK(Context != nullptr, 0, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);
    // TODO: implement this
    throw std::runtime_error("Not implemented!");
}

Handlers::TreeView* TreeView::Handlers()
{
    GET_CONTROL_HANDLERS(Handlers::TreeView);
}

TreeViewItem TreeView::GetCurrentItem()
{
    CHECK(Context != nullptr, (TreeViewItem{ nullptr, InvalidItemHandle }), "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    return { this->Context, cc->GetCurrentItemHandle() };
}

// --------------------------------------
#define CREATE_TREE_VIEW_ITEM_CONTEXT(result)                                                                          \
    auto cc = reinterpret_cast<TreeControlContext*>(this->obj);                                                        \
    CHECK(cc != nullptr, (result), "");

bool TreeViewItem::SetType(TreeViewItem::Type type)
{
    CREATE_TREE_VIEW_ITEM_CONTEXT(false);
    cc->items.at(handle).type = type;
    return true;
}

bool TreeViewItem::SetColor(const Graphics::ColorPair& color)
{
    CREATE_TREE_VIEW_ITEM_CONTEXT(false);
    cc->items.at(handle).color = color;
    return true;
}

bool TreeViewItem::SetCurrent()
{
    CREATE_TREE_VIEW_ITEM_CONTEXT(false);
    cc->SetCurrentItemHandle(handle);
    return true;
}

bool TreeViewItem::IsCurrent() const
{
    CREATE_TREE_VIEW_ITEM_CONTEXT(false);
    return cc->GetCurrentItemHandle() == handle;
}

bool TreeViewItem::SetFolding(bool expand)
{
    CREATE_TREE_VIEW_ITEM_CONTEXT(false);
    cc->items.at(handle).expanded = expand;
    return true;
}

bool TreeViewItem::IsFolded()
{
    CREATE_TREE_VIEW_ITEM_CONTEXT(false);
    return !cc->items.at(handle).expanded;
}

bool TreeViewItem::SetExpandable(bool expandable)
{
    CREATE_TREE_VIEW_ITEM_CONTEXT(false);
    cc->items.at(handle).isExpandable = expandable;
    return true;
}

bool TreeViewItem::IsExpandable() const
{
    CREATE_TREE_VIEW_ITEM_CONTEXT(false);
    return cc->items.at(handle).isExpandable;
}

uint32 TreeViewItem::GetChildrenCount() const
{
    CREATE_TREE_VIEW_ITEM_CONTEXT(0);
    return static_cast<uint32>(cc->items.at(handle).children.size());
}

TreeViewItem TreeViewItem::GetChild(uint32 index)
{
    CREATE_TREE_VIEW_ITEM_CONTEXT(TreeViewItem());
    CHECK(index < cc->items.at(handle).children.size(), (TreeViewItem{ nullptr, InvalidItemHandle }), "");

    return { this->obj, cc->items.at(handle).children.at(index) };
}

bool TreeViewItem::DeleteChildren()
{
    CREATE_TREE_VIEW_ITEM_CONTEXT(false);

    const auto noChildren = GetChildrenCount();
    for (auto i = 0U; i < noChildren; i++)
    {
        auto child = GetChild(i);
        CHECK(cc->host->RemoveItem(child), false, "");
    }
    cc->items.at(handle).children.clear();

    return true;
}

ItemHandle TreeViewItem::GetHandle() const
{
    return handle;
}

bool TreeViewItem::Toggle(bool recursiveCall)
{
    CREATE_TREE_VIEW_ITEM_CONTEXT(false);
    CHECK(IsExpandable(), true, "");

    if (cc->treeFlags && TreeViewFlags::DynamicallyPopulateNodeChildren)
    {
        CHECK(DeleteChildren(), false, "");
    }

    SetFolding(IsFolded());

    if (!IsFolded())
    {
        if (cc->treeFlags && TreeViewFlags::DynamicallyPopulateNodeChildren)
        {
            return cc->TriggerOnItemToggled(*this, recursiveCall);
        }
    }

    return true;
}

bool TreeViewItem::ToggleRecursively()
{
    CREATE_TREE_VIEW_ITEM_CONTEXT(false);
    CHECK(IsExpandable(), true, ""); // nothing to expand

    std::queue<ItemHandle> ancestorRelated;
    ancestorRelated.push(handle);

    while (ancestorRelated.empty() == false)
    {
        ItemHandle handle = ancestorRelated.front();
        ancestorRelated.pop();

        auto treeItem = cc->host->GetItemByHandle(handle);
        if (treeItem.Toggle(true) == false)
        {
            break;
        }

        const auto& item = cc->items[handle];
        for (const auto& handle : item.children)
        {
            ancestorRelated.push(handle);
        }
    }

    if (cc->treeFlags && TreeViewFlags::Sortable)
    {
        cc->Sort();
    }

    cc->notProcessed = true;

    return true;
}

bool TreeViewItem::Fold()
{
    CHECK(IsValid(), false, "");
    CHECK(IsExpandable(), false, "");
    CHECK(IsFolded() == false, false, "");

    return Toggle();
}

bool TreeViewItem::Unfold()
{
    CHECK(IsValid(), false, "");
    CHECK(IsExpandable(), false, "");
    CHECK(IsFolded(), false, "");

    return Toggle();
}

bool TreeViewItem::FoldAll()
{
    CHECK(IsValid(), false, "");
    CHECK(IsExpandable(), false, "");
    CHECK(IsFolded() == false, false, "");

    return ToggleRecursively();
}

bool TreeViewItem::UnfoldAll()
{
    CHECK(IsValid(), false, "");
    CHECK(IsExpandable(), false, "");
    CHECK(IsFolded(), false, "");

    return ToggleRecursively();
}

TreeViewItem TreeViewItem::GetParent() const
{
    CREATE_TREE_VIEW_ITEM_CONTEXT(TreeViewItem());

    auto& parent = cc->items.at(handle).parent;
    if (parent == InvalidItemHandle)
    {
        return { nullptr, InvalidItemHandle };
    }

    return { this->obj, parent };
}

uint32 TreeViewItem::GetPriority() const
{
    CREATE_TREE_VIEW_ITEM_CONTEXT(-1);
    return cc->items.at(handle).priority;
}

bool TreeViewItem::SetPriority(uint32 priority) const
{
    CREATE_TREE_VIEW_ITEM_CONTEXT(false);
    cc->items.at(handle).priority = priority;
    return true;
}

TreeViewItem TreeViewItem::AddChild(ConstString name, bool isExpandable)
{
    CREATE_TREE_VIEW_ITEM_CONTEXT(TreeViewItem());
    return { obj, cc->AddItem(handle, { name }, isExpandable) };
}

bool TreeViewItem::SetText(ConstString name)
{
    CREATE_TREE_VIEW_ITEM_CONTEXT(false)
    return cc->items.at(handle).values.at(0).Set(name);
}

const CharacterBuffer& TreeViewItem::GetText() const
{
    static const CharacterBuffer cb{};
    CREATE_TREE_VIEW_ITEM_CONTEXT(cb);
    const auto it = cc->items.find(handle);
    if (it != cc->items.end())
    {
        return it->second.values.at(0);
    }

    return cb;
}

bool TreeViewItem::SetValues(const std::initializer_list<ConstString> values)
{
    CREATE_TREE_VIEW_ITEM_CONTEXT(false);

    auto& vals = cc->items.at(handle).values;
    auto it    = vals.begin();
    std::advance(it, 1); // past name

    bool end = (it == vals.end());

    for (const auto& value : values)
    {
        if (end)
        {
            auto& val = vals.emplace_back();
            val.Set(value);
        }
        else
        {
            it->Set(value);
            std::advance(it, 1);
            end = (it == vals.end());
        }
    }

    return true;
}

bool TreeViewItem::SetText(uint32 subItemIndex, const ConstString& text)
{
    CREATE_TREE_VIEW_ITEM_CONTEXT(false);
    CHECK(subItemIndex < cc->Header.GetColumnsCount(), false, "");

    auto& item = cc->items.at(handle);
    if (item.values.size() <= subItemIndex)
    {
        item.values.resize(subItemIndex + 1ULL);
    }

    return item.values.at(subItemIndex).Set(text);
}

const Graphics::CharacterBuffer& TreeViewItem::GetText(uint32 subItemIndex) const
{
    static const CharacterBuffer cb{};
    CREATE_TREE_VIEW_ITEM_CONTEXT(cb);
    CHECK(subItemIndex < cc->items.at(handle).values.size(), cb, "");

    return cc->items.at(handle).values.at(subItemIndex);
}

bool TreeViewItem::SetData(uint64 value)
{
    CREATE_TREE_VIEW_ITEM_CONTEXT(false);
    cc->items.at(handle).data = value;
    return true;
}

uint64 TreeViewItem::GetData(uint64 errorValue) const
{
    CREATE_TREE_VIEW_ITEM_CONTEXT(errorValue);
    return std::get<uint64>(cc->items.at(handle).data);
}

GenericRef TreeViewItem::GetItemDataAsPointer() const
{
    CREATE_TREE_VIEW_ITEM_CONTEXT(nullptr);
    return cc->GetItemDataAsPointer(handle);
}

bool TreeViewItem::SetItemDataAsPointer(GenericRef ref)
{
    CREATE_TREE_VIEW_ITEM_CONTEXT(false);
    return cc->SetItemDataAsPointer(handle, ref);
}

bool TreeView::RemoveItem(TreeViewItem& item)
{
    CHECK(item.IsValid(), false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(item.obj);
    if (cc->RemoveItem(item.handle))
    {
        item.obj    = nullptr;
        item.handle = InvalidItemHandle;
        return true;
    }

    return false;
}

bool TreeView::ClearItems()
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    cc->items.clear();

    cc->nextItemHandle = 1ULL;

    cc->SetCurrentItemHandle(InvalidItemHandle);

    cc->roots.clear();

    cc->ProcessItemsToBeDrawn(InvalidItemHandle);

    return true;
}

TreeViewItem TreeView::GetItemByIndex(const uint32 index)
{
    CHECK(Context != nullptr, (TreeViewItem{ nullptr, InvalidItemHandle }), "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);
    CHECK(index < cc->items.size(), (TreeViewItem{ nullptr, InvalidItemHandle }), "");

    auto it = cc->items.begin();
    std::advance(it, index);
    CHECK(it != cc->items.end(), (TreeViewItem{ nullptr, InvalidItemHandle }), "");

    return { this->Context, it->second.handle };
}

uint32 TreeView::GetItemsCount() const
{
    CHECK(Context != nullptr, 0, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);
    return static_cast<uint32>(cc->items.size());
}

TreeViewItem TreeView::GetItemByHandle(ItemHandle handle)
{
    CHECK(Context != nullptr, TreeViewItem{}, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    CHECK(cc->items.find(handle) != cc->items.end(), TreeViewItem{}, "");

    return { Context, handle };
}

TreeViewItem TreeView::AddItem(ConstString name, bool isExpandable)
{
    auto cc = reinterpret_cast<TreeControlContext*>(this->Context);
    CHECK(cc != nullptr, (TreeViewItem{ nullptr, InvalidItemHandle }), "");

    return { this->Context, cc->AddItem(InvalidItemHandle, { name }, isExpandable) };
}

bool TreeView::Sort()
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);
    return cc->Sort();
}

bool TreeView::Sort(uint32 columnIndex, SortDirection direction)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);
    CHECK(columnIndex < cc->Header.GetColumnsCount(), false, "");

    CHECK(cc->Header.SetSortColumn(columnIndex, direction), false, "");

    return cc->Sort();
}
} // namespace AppCUI::Controls

namespace AppCUI
{
void TreeControlContext::SetCurrentItemHandle(ItemHandle handle)
{
    currentItemHandle = handle;
    TriggerOnCurrentItemChanged();
}

ItemHandle TreeControlContext::GetCurrentItemHandle() const
{
    return currentItemHandle;
}

void TreeControlContext::ColumnSort(uint32 columnIndex)
{
    if (treeFlags && TreeViewFlags::Sortable)
    {
        this->Header.SetSortColumn(columnIndex);
        Sort();
    }
}

bool TreeControlContext::Sort()
{
    const auto result = SortByColumn(InvalidItemHandle);
    notProcessed      = true;
    return result;
}

struct ItemComparator
{
    Reference<TreeView> tv;

    inline bool operator()(const ItemHandle i1, const ItemHandle i2)
    {
        auto tcc = reinterpret_cast<TreeControlContext*>(tv->Context);
        CHECK(tcc != nullptr, true, "");

        auto& a = tcc->items.at(i1);
        auto& b = tcc->items.at(i2);

        auto result                 = a.priority > b.priority;
        const auto sortedByPriority = a.priority != b.priority;

        if (sortedByPriority)
        {
            return result;
        }

        const auto& index = tcc->Header.GetSortColumnIndex();
        CHECK(index.has_value(), true, "");

        const auto aInvalidColumnIndex = *index >= a.values.size();
        const auto bInvalidColumnIndex = *index >= b.values.size();

        if (aInvalidColumnIndex && bInvalidColumnIndex)
        {
            return a.handle < b.handle;
        }
        else if (aInvalidColumnIndex)
        {
            return true;
        }
        else if (bInvalidColumnIndex)
        {
            return false;
        }

        if (tcc->handlers != nullptr)
        {
            auto handler = reinterpret_cast<Controls::Handlers::TreeView*>(tcc->handlers.get());
            if (handler->CompareItems.obj)
            {
                result = handler->CompareItems.obj->CompareItems(tv, tv->GetItemByHandle(i1), tv->GetItemByHandle(i2));

                if (tcc->Header.GetSortDirection() == SortDirection::Ascendent)
                {
                    result = !result;
                }

                return result;
            }
        }

        const auto& aVal = a.values.at(*tcc->Header.GetSortColumnIndex());
        const auto& bVal = b.values.at(*tcc->Header.GetSortColumnIndex());

        const auto cResult = aVal.CompareWith(bVal, true);

        if (cResult == 0)
        {
            result = false;
        }
        else
        {
            result = cResult < 0;
            if (tcc->Header.GetSortDirection() != SortDirection::Ascendent)
            {
                result = !result;
            }
        }

        return result;
    }

    ItemComparator(Reference<TreeView> _tv) : tv(_tv)
    {
    }
};

bool TreeControlContext::SortByColumn(const ItemHandle handle)
{
    CHECK(this->Header.GetSortColumnIndex().has_value(), false, "");
    CHECK(items.size() > 0, false, "");

    if (handle == InvalidItemHandle)
    {
        std::sort(roots.begin(), roots.end(), ItemComparator(host));

        for (const auto& rootHandle : roots)
        {
            auto& root = items[rootHandle];
            std::sort(root.children.begin(), root.children.end(), ItemComparator(host));
            for (auto& childHandle : root.children)
            {
                SortByColumn(childHandle);
            }
        }
    }
    else
    {
        auto& item = items[handle];
        std::sort(item.children.begin(), item.children.end(), ItemComparator(host));
        for (auto& childHandle : item.children)
        {
            SortByColumn(childHandle);
        }
    }

    return true;
}

bool TreeControlContext::ProcessOrderedItems(const ItemHandle handle, const bool clear)
{
    if (clear)
    {
        orderedItems.clear();
        orderedItems.reserve(items.size());
    }

    CHECK(items.size() > 0, true, "");

    if (handle == InvalidItemHandle)
    {
        for (const auto& rootHandle : roots)
        {
            orderedItems.emplace_back(rootHandle);

            const auto& root = items[rootHandle];
            for (auto& childHandle : root.children)
            {
                ProcessOrderedItems(childHandle, false);
            }
        }
    }
    else
    {
        const auto& item = items[handle];
        orderedItems.emplace_back(item.handle);
        for (auto& childHandle : item.children)
        {
            ProcessOrderedItems(childHandle, false);
        }
    }

    return true;
}

bool TreeControlContext::IsMouseOnToggleSymbol(int x, int y) const
{
    const uint32 index = y - 2;
    if (index >= offsetBotToDraw || index >= itemsToDrew.size())
    {
        return false;
    }

    const auto itemHandle = itemsToDrew.at(static_cast<size_t>(offsetTopToDraw) + index);
    const auto it         = items.find(itemHandle);

    if (x > static_cast<int>(it->second.depth * ItemSymbolOffset + ItemSymbolOffset) &&
        x < static_cast<int>(Layout.Width))
    {
        return false; // on item
    }

    if (x >= static_cast<int>(it->second.depth * ItemSymbolOffset) &&
        x < static_cast<int>(it->second.depth * ItemSymbolOffset + ItemSymbolOffset - 1U))
    {
        return true;
    }

    return false;
}

bool TreeControlContext::IsMouseOnItem(int x, int y) const
{
    const uint32 index = y - 2;
    if (index >= offsetBotToDraw || index >= itemsToDrew.size())
    {
        return false;
    }

    const auto itemHandle = itemsToDrew[static_cast<size_t>(offsetTopToDraw) + index];
    const auto it         = items.find(itemHandle);

    return (
          x > static_cast<int>(it->second.depth * ItemSymbolOffset + ItemSymbolOffset) &&
          x < static_cast<int>(Layout.Width));
}

bool TreeControlContext::IsMouseOnBorder(int x, int y) const
{
    return (x == 0 || x == Layout.Width - BorderOffset) || (y == 0 || y == Layout.Width - BorderOffset);
}

bool TreeControlContext::IsMouseOnColumnHeader(int x, int y)
{
    mouseOverColumnIndex = InvalidIndex;
    CHECK(x >= 0, false, "");
    CHECK(y == 1, false, "");

    CHECK((treeFlags && TreeViewFlags::HideColumns) == false, false, "");

    for (auto i = 0U; i < this->Header.GetColumnsCount(); i++)
    {
        const auto& col = this->Header[i];
        if (x >= col.x && x <= col.x + (int32) col.width)
        {
            mouseOverColumnIndex = i;
            return true;
        }
        i++;
    }

    return false;
}

bool TreeControlContext::IsMouseOnColumnSeparator(int x, int y)
{
    mouseOverColumnSeparatorIndex = InvalidIndex;

    const auto count = this->Header.GetColumnsCount();
    for (auto i = 0U; i < count; i++)
    {
        const auto& col = this->Header[i];
        const auto xs   = col.x + col.width;
        if (xs == x)
        {
            mouseOverColumnSeparatorIndex = i;
            return true;
        }
    }

    return false;
}

bool TreeControlContext::IsMouseOnSearchField(int x, int y) const
{
    if (Focused)
    {
        if (y == Layout.Height - 1)
        {
            if (x > 0 && x < TreeSearchBarWidth)
            {
                return true;
            }
        }
    }

    return false;
}

bool TreeControlContext::AdjustElementsOnResize(const int /*newWidth*/, const int /*newHeight*/)
{
    CHECK(AdjustItemsBoundsOnResize(), false, "");
    const auto columnsSize = this->Header.GetColumnsCount();
    const uint32 width = (static_cast<uint32>(Layout.Width)) / static_cast<uint32>(std::max<uint32>(columnsSize, 1U));

    uint32 xPreviousColumn       = 0;
    uint32 widthOfPreviousColumn = 0;
    for (auto i = 0U; i < columnsSize; i++)
    {
        auto& col             = this->Header[i];
        col.x                 = static_cast<uint32>(xPreviousColumn + widthOfPreviousColumn + BorderOffset);
        xPreviousColumn       = col.x;
        widthOfPreviousColumn = col.width;
    }

    if (Layout.Width <= TreeScrollbarLeftOffset)
    {
        if (hidSearchBarOnResize == false)
        {
            if ((treeFlags & TreeViewFlags::HideSearchBar) != TreeViewFlags::None)
            {
                filter.mode = TreeControlContext::FilterMode::None;
                filter.searchText.Clear();
            }
            hidSearchBarOnResize = true;
        }
    }
    else
    {
        if (hidSearchBarOnResize)
        {
            treeFlags ^= static_cast<uint32>(TreeViewFlags::HideSearchBar);
            hidSearchBarOnResize = false;
        }

        if ((treeFlags & TreeViewFlags::FilterSearch) != TreeViewFlags::None)
        {
            filter.mode = TreeControlContext::FilterMode::Filter;
        }
        else if ((treeFlags & TreeViewFlags::Searchable) != TreeViewFlags::None)
        {
            filter.mode = TreeControlContext::FilterMode::Search;
        }
    }

    return true;
}

bool TreeControlContext::AdjustItemsBoundsOnResize()
{
    maxItemsToDraw  = Layout.Height - 1 - 1 - 1; // 0 - border top | 1 - column header | 2 - border bottom
    offsetBotToDraw = offsetTopToDraw + maxItemsToDraw;

    return true;
}

bool TreeControlContext::SetColorForItems(const ColorPair& color)
{
    for (auto& item : items)
    {
        for (auto& value : item.second.values)
        {
            value.SetColor(color);
        }
    }

    return true;
}

bool TreeControlContext::SearchItems()
{
    bool found = false;

    MarkAllItemsAsNotFound();
    SetColorForItems(Cfg->Text.Normal);
    ItemComparator ic(host);

    std::set<ItemHandle> toBeExpanded;
    if (filter.searchText.Len() > 0)
    {
        for (auto& item : items)
        {
            for (auto& value : item.second.values)
            {
                if (const auto index = value.Find(filter.searchText.ToStringView(), true); index >= 0)
                {
                    item.second.markedAsFound = true;
                    if (filter.mode == TreeControlContext::FilterMode::Filter)
                    {
                        MarkAllAncestorsWithChildFoundInFilterSearch(item.second.handle);
                    }

                    if (items[currentItemHandle].markedAsFound == false)
                    {
                        SetCurrentItemHandle(item.second.handle);
                    }
                    else
                    {
                        if (currentItemHandle != item.second.handle &&
                            items[currentItemHandle].depth == item.second.depth)
                        {
                            if (ic.operator()(currentItemHandle, item.second.handle) == false)
                            {
                                SetCurrentItemHandle(item.second.handle);
                            }
                        }
                        else if (items[currentItemHandle].depth > item.second.depth)
                        {
                            SetCurrentItemHandle(item.second.handle);
                        }
                    }

                    found = true;
                    value.SetColor(index, index + filter.searchText.Len(), Cfg->Selection.SearchMarker);

                    ItemHandle ancestorHandle = item.second.parent;
                    do
                    {
                        if (const auto& it = items.find(ancestorHandle); it != items.end())
                        {
                            const auto& ancestor = it->second;
                            if (ancestor.isExpandable && ancestor.expanded == false &&
                                (treeFlags & TreeViewFlags::DynamicallyPopulateNodeChildren) == TreeViewFlags::None)
                            {
                                toBeExpanded.insert(ancestorHandle);
                            }
                            ancestorHandle = ancestor.parent;
                        }
                        else
                        {
                            break;
                        }
                    } while (ancestorHandle != InvalidItemHandle);
                }
            }
        }
    }

    for (const auto itemHandle : toBeExpanded)
    {
        auto item = host->GetItemByHandle(itemHandle);
        item.Toggle();
    }

    JumpToCurrent();

    if (toBeExpanded.size() > 0 || filter.mode == TreeControlContext::FilterMode::Filter)
    {
        ProcessItemsToBeDrawn(InvalidItemHandle);
    }

    ProcessOrderedItems(InvalidItemHandle, true);

    return found;
}

bool TreeControlContext::MarkAllItemsAsNotFound()
{
    for (auto& [handle, item] : items)
    {
        item.markedAsFound = false;

        if (filter.mode == TreeControlContext::FilterMode::Filter)
        {
            item.hasAChildThatIsMarkedAsFound = false;
        }
    }

    return true;
}

bool TreeControlContext::MarkAllAncestorsWithChildFoundInFilterSearch(const ItemHandle handle)
{
    CHECK(handle != InvalidItemHandle, false, "");

    const auto& item          = items[handle];
    ItemHandle ancestorHandle = item.parent;
    do
    {
        if (const auto& it = items.find(ancestorHandle); it != items.end())
        {
            auto& ancestor                        = it->second;
            ancestor.hasAChildThatIsMarkedAsFound = true;
            ancestorHandle                        = ancestor.parent;
        }
        else
        {
            break;
        }
    } while (ancestorHandle != InvalidItemHandle);

    return true;
}

bool TreeControlContext::PaintItems(Graphics::Renderer& renderer)
{
    WriteTextParams wtp;
    wtp.Y = ((treeFlags & TreeViewFlags::HideColumns) == TreeViewFlags::None) +
            ((treeFlags & TreeViewFlags::HideBorder) == TreeViewFlags::None); // 0  is for border | 1 is for header

    for (auto i = offsetTopToDraw; i < std::min<size_t>(offsetBotToDraw, itemsToDrew.size()); i++)
    {
        auto& item = items[itemsToDrew[i]];

        uint32 j = 0; // column index
        for (auto i = 0U; i < this->Header.GetColumnsCount(); i++)
        {
            const auto& col = this->Header[i];
            wtp.Flags       = WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth;
            wtp.Align       = col.align;
            if (j == 0)
            {
                wtp.X     = col.x + item.depth * ItemSymbolOffset - 1;
                wtp.Width = col.width - item.depth * ItemSymbolOffset -
                            ((treeFlags & TreeViewFlags::HideColumnsSeparator) == TreeViewFlags::None);

                if (wtp.X < static_cast<int>(col.x + col.width))
                {
                    if (item.isExpandable)
                    {
                        if (item.expanded)
                        {
                            renderer.WriteSpecialCharacter(
                                  wtp.X, wtp.Y, SpecialChars::TriangleDown, Cfg->Symbol.Arrows);
                        }
                        else
                        {
                            renderer.WriteSpecialCharacter(
                                  wtp.X, wtp.Y, SpecialChars::TriangleRight, Cfg->Symbol.Arrows);
                        }
                    }
                    else
                    {
                        renderer.WriteSpecialCharacter(wtp.X, wtp.Y, SpecialChars::CircleFilled, Cfg->Symbol.Inactive);
                    }
                }

                wtp.X += ItemSymbolOffset;
            }
            else
            {
                wtp.X     = col.x;
                wtp.Width = col.width;
            }

            if (j < item.values.size())
            {
                if (item.handle == currentItemHandle)
                {
                    if (Focused)
                    {
                        wtp.Color = Cfg->Cursor.Normal;
                    }
                    else
                    {
                        wtp.Color = Cfg->Text.Focused;
                    }
                }
                else if (item.markedAsFound)
                {
                    // nothing - color already set
                }
                else
                {
                    switch (item.type)
                    {
                    case TreeViewItem::Type::Normal:
                        wtp.Color = Cfg->Text.Normal;
                        break;
                    case TreeViewItem::Type::Highlighted:
                        wtp.Color = Cfg->Text.Focused;
                        break;
                    case TreeViewItem::Type::GrayedOut:
                        wtp.Color = Cfg->Text.Inactive;
                        break;
                    case TreeViewItem::Type::ErrorInformation:
                        wtp.Color = Cfg->Text.Error;
                        break;
                    case TreeViewItem::Type::WarningInformation:
                        wtp.Color = Cfg->Text.Warning;
                        break;
                    case TreeViewItem::Type::Emphasized_1:
                        wtp.Color = Cfg->Text.Emphasized1;
                        break;
                    case TreeViewItem::Type::Emphasized_2:
                        wtp.Color = Cfg->Text.Emphasized2;
                        break;
                    case TreeViewItem::Type::Category:
                        wtp.Color = Cfg->Text.Highlighted;
                        break;
                    case TreeViewItem::Type::Colored:
                        wtp.Color = item.color;
                        break;
                    default:
                        break;
                    }
                }

                if (item.markedAsFound == false)
                {
                    item.values[j].SetColor(wtp.Color);
                }

                if (wtp.X < static_cast<int>(col.x + col.width))
                {
                    renderer.WriteText(item.values[j], wtp);
                }
            }

            j++;
        }

        if (item.handle == currentItemHandle && Focused)
        {
            const uint32 addX = treeFlags && TreeViewFlags::HideBorder ? 0 : 1;
            renderer.FillHorizontalLine(addX, wtp.Y, this->Layout.Width - 1 - addX, -1, Cfg->Cursor.Normal);
        }

        wtp.Y++;
    }

    return true;
}

bool TreeControlContext::MoveUp()
{
    if (itemsToDrew.size() > 0)
    {
        const auto it       = std::find(itemsToDrew.begin(), itemsToDrew.end(), currentItemHandle);
        const auto index    = static_cast<uint32>(it - itemsToDrew.begin());
        const auto newIndex = std::min<uint32>(index - 1, static_cast<uint32>(itemsToDrew.size() - 1U));

        if (newIndex == itemsToDrew.size() - 1)
        {
            return true;
        }

        SetCurrentItemHandle(itemsToDrew[newIndex]);

        if (newIndex < offsetTopToDraw && offsetTopToDraw > 0)
        {
            offsetBotToDraw--;
            offsetTopToDraw--;
        }

        return true;
    }

    return false;
}

bool TreeControlContext::MoveDown()
{
    if (itemsToDrew.size() > 0)
    {
        const auto it       = std::find(itemsToDrew.begin(), itemsToDrew.end(), currentItemHandle);
        const auto index    = static_cast<uint32>(it - itemsToDrew.begin());
        const auto newIndex = std::min<uint32>(index + 1, (index + 1ULL > itemsToDrew.size() - 1 ? 0 : index + 1));

        if (newIndex == 0)
        {
            return true;
        }

        SetCurrentItemHandle(itemsToDrew[newIndex]);

        if (newIndex >= offsetBotToDraw)
        {
            offsetBotToDraw++;
            offsetTopToDraw++;
        }

        return true;
    }

    return false;
}

bool TreeControlContext::JumpToCurrent()
{
    if (itemsToDrew.size() > 0)
    {
        const auto it    = std::find(itemsToDrew.begin(), itemsToDrew.end(), currentItemHandle);
        const auto index = static_cast<uint32>(it - itemsToDrew.begin());

        if (index < offsetTopToDraw || offsetBotToDraw < index)
        {
            offsetTopToDraw = std::max<>(0, (int32) ((int32) index - (maxItemsToDraw / 2)));
            offsetBotToDraw = offsetTopToDraw + maxItemsToDraw;
        }

        return true;
    }

    return false;
}

bool TreeControlContext::ProcessItemsToBeDrawn(const ItemHandle handle, bool clear)
{
    if (clear)
    {
        itemsToDrew.clear();
        itemsToDrew.reserve(items.size());
    }

    CHECK(items.size() > 0, true, "");

    if (handle == InvalidItemHandle)
    {
        for (const auto& handle : roots)
        {
            const auto& item = items[handle];
            if (filter.mode == TreeControlContext::FilterMode::Filter && filter.searchText.Len() > 0)
            {
                if (item.hasAChildThatIsMarkedAsFound == false && item.markedAsFound == false)
                {
                    continue;
                }
            }

            itemsToDrew.emplace_back(handle);

            if (item.isExpandable == false || item.expanded == false)
            {
                continue;
            }

            for (auto& it : item.children)
            {
                const auto& child = items[it];

                if (filter.mode == TreeControlContext::FilterMode::Filter && filter.searchText.Len() > 0)
                {
                    if (child.hasAChildThatIsMarkedAsFound == false && child.markedAsFound == false)
                    {
                        continue;
                    }
                }

                if (child.isExpandable)
                {
                    if (child.expanded)
                    {
                        ProcessItemsToBeDrawn(it, false);
                    }
                    else
                    {
                        itemsToDrew.emplace_back(child.handle);
                    }
                }
                else
                {
                    itemsToDrew.emplace_back(child.handle);
                }
            }
        }
    }
    else
    {
        const auto& item = items[handle];

        if (filter.mode == TreeControlContext::FilterMode::Filter && filter.searchText.Len() > 0)
        {
            if (item.hasAChildThatIsMarkedAsFound == false && item.markedAsFound == false)
            {
                return true;
            }
        }

        itemsToDrew.emplace_back(item.handle);
        CHECK(item.isExpandable, true, "");

        for (auto& it : item.children)
        {
            const auto& child = items[it];

            if (filter.mode == TreeControlContext::FilterMode::Filter && filter.searchText.Len() > 0)
            {
                if (child.hasAChildThatIsMarkedAsFound == false && child.markedAsFound == false)
                {
                    continue;
                }
            }

            if (child.isExpandable)
            {
                if (child.expanded)
                {
                    ProcessItemsToBeDrawn(it, false);
                }
                else
                {
                    itemsToDrew.emplace_back(child.handle);
                }
            }
            else
            {
                itemsToDrew.emplace_back(child.handle);
            }
        }
    }

    return true;
}

bool TreeControlContext::IsAncestorOfChild(const ItemHandle ancestor, const ItemHandle child)
{
    std::queue<ItemHandle> ancestorRelated;
    ancestorRelated.push(ancestor);

    while (ancestorRelated.empty() == false)
    {
        const ItemHandle current = ancestorRelated.front();
        ancestorRelated.pop();

        for (const auto& handle : items[current].children)
        {
            if (handle == child)
            {
                return true;
            }
            else
            {
                ancestorRelated.push(handle);
            }
        }
    }

    return false;
}

bool TreeControlContext::RemoveItem(const ItemHandle handle)
{
    std::queue<ItemHandle> ancestorRelated;
    ancestorRelated.push(handle);

    while (ancestorRelated.empty() == false)
    {
        ItemHandle current = ancestorRelated.front();
        ancestorRelated.pop();

        if (const auto it = items.find(handle); it != items.end())
        {
            for (const auto& handle : items.at(current).children)
            {
                ancestorRelated.push(handle);
            }
            items.erase(it);

            if (const auto rootIt = std::find(roots.begin(), roots.end(), handle); rootIt != roots.end())
            {
                roots.erase(rootIt);
            }
        }
    }

    notProcessed = true;

    return true;
}

GenericRef TreeControlContext::GetItemDataAsPointer(ItemHandle handle) const
{
    const auto it = items.find(handle);
    if (it != items.end())
    {
        if (std::holds_alternative<GenericRef>(it->second.data))
            return std::get<GenericRef>(it->second.data);
    }

    return nullptr;
}

bool TreeControlContext::SetItemDataAsPointer(ItemHandle item, GenericRef value)
{
    auto it = items.find(item);
    if (it == items.end())
    {
        return false;
    }
    it->second.data = value;

    return true;
}

ItemHandle TreeControlContext::AddItem(ItemHandle parent, std::initializer_list<ConstString> values, bool isExpandable)
{
    CHECK(values.size() > 0, InvalidItemHandle, "");

    std::vector<CharacterBuffer> cbvs;
    cbvs.reserve(values.size());
    for (const auto& value : values)
    {
        auto& cb = cbvs.emplace_back();
        cb.Set(value);
    }

    items[nextItemHandle]              = { parent, nextItemHandle, std::move(cbvs) };
    items[nextItemHandle].isExpandable = isExpandable;

    if (parent == InvalidItemHandle)
    {
        roots.emplace_back(items[nextItemHandle].handle);
    }
    else
    {
        auto& parentItem            = items[parent];
        items[nextItemHandle].depth = parentItem.depth + 1;
        parentItem.children.emplace_back(items[nextItemHandle].handle);
        parentItem.isExpandable = true;
    }

    if (items.size() == 1)
    {
        SetCurrentItemHandle(items[nextItemHandle].handle);
    }

    notProcessed = true;

    return items[nextItemHandle++].handle;
}

void TreeControlContext::TriggerOnCurrentItemChanged()
{
    if (handlers != nullptr)
    {
        auto handler = reinterpret_cast<Controls::Handlers::TreeView*>(handlers.get());
        if (handler->OnCurrentItemChanged.obj)
        {
            auto item = host->GetCurrentItem();
            handler->OnCurrentItemChanged.obj->OnTreeViewCurrentItemChanged(host, item);
        }
    }
}

void TreeControlContext::TriggerOnItemPressed()
{
    if (handlers != nullptr)
    {
        auto handler = reinterpret_cast<Controls::Handlers::TreeView*>(handlers.get());
        if (handler->OnItemPressed.obj)
        {
            TreeViewItem item = host->GetCurrentItem();
            handler->OnItemPressed.obj->OnTreeViewItemPressed(host, item);
        }
    }
}

bool TreeControlContext::TriggerOnItemToggled(TreeViewItem& item, bool recursiveCall)
{
    if (handlers != nullptr)
    {
        auto handler = reinterpret_cast<Controls::Handlers::TreeView*>(handlers.get());
        if (handler->OnItemToggle.obj)
        {
            return handler->OnItemToggle.obj->OnTreeViewItemToggle(host, item, recursiveCall);
        }
    }

    return true;
}
} // namespace AppCUI
