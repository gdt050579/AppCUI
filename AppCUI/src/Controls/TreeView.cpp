#include "ControlContext.hpp"
#include <deque>
#include <queue>
#include <stack>
#include <set>
#include <charconv>

namespace AppCUI::Controls
{
constexpr auto TreeSearchBarWidth      = 23U;
constexpr auto TreeScrollbarLeftOffset = 25U;
constexpr auto ItemSymbolOffset        = 2U;
constexpr auto MinColumnWidth          = 10U;
constexpr auto BorderOffset            = 1U;
constexpr auto InvalidIndex            = 0xFFFFFFFFU;

const static Utils::UnicodeStringBuilder cb{};

TreeView::TreeView(string_view layout, std::initializer_list<ColumnBuilder> columns, TreeViewFlags flags)
    : Control(new TreeControlContext(), "", layout, true)
{
    const auto cc        = reinterpret_cast<TreeControlContext*>(Context);
    cc->Layout.MinHeight = 1;
    cc->Layout.MaxHeight = 200000;
    cc->Layout.MinWidth  = 20;

    cc->treeFlags = static_cast<uint32>(flags);

    cc->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;

    if ((cc->treeFlags & TreeViewFlags::HideScrollBar) == TreeViewFlags::None)
    {
        cc->Flags |= GATTR_VSCROLL;
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

    if ((cc->treeFlags & TreeViewFlags::Sortable) != TreeViewFlags::None)
    {
        cc->columnIndexToSortBy = 0;
        cc->Sort();
    }

    cc->AdjustItemsBoundsOnResize();

    for (const auto& column : columns)
    {
        AddColumn(column.name, column.align, column.width);
    }

    cc->separatorIndexSelected = InvalidIndex;

    cc->SetColorForItems(cc->Cfg->Text.Normal);
}

void TreeView::Paint(Graphics::Renderer& renderer)
{
    CHECKRET(Context != nullptr, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    cc->PaintColumnHeaders(renderer);

    if (cc->notProcessed)
    {
        cc->ProcessItemsToBeDrawn(InvalidItemHandle, true);
        cc->notProcessed = false;
    }

    if ((cc->treeFlags & TreeViewFlags::HideScrollBar) == TreeViewFlags::None)
    {
        if (cc->itemsToDrew.size() > cc->maxItemsToDraw)
        {
            if ((cc->Flags & GATTR_VSCROLL) == 0)
            {
                cc->Flags |= GATTR_VSCROLL;
            }
        }
        else
        {
            if ((cc->Flags & GATTR_VSCROLL) == GATTR_VSCROLL)
            {
                cc->Flags ^= GATTR_VSCROLL;
            }
        }
    }

    cc->ItemsPainting(renderer);
    cc->PaintColumnSeparators(renderer);

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
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    switch (keyCode)
    {
    case Key::Up:
        return cc->MoveUp();
    case Key::Down:
        if (cc->separatorIndexSelected != InvalidIndex)
        {
            cc->separatorIndexSelected = InvalidIndex;
            return true;
        }
        else
        {
            return cc->MoveDown();
        }
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
            const auto it    = find(cc->itemsToDrew.begin(), cc->itemsToDrew.end(), cc->currentSelectedItemHandle);
            const auto index = static_cast<uint32>(it - cc->itemsToDrew.begin()) - cc->maxItemsToDraw;
            cc->currentSelectedItemHandle = cc->itemsToDrew[index];

            cc->offsetTopToDraw -= cc->maxItemsToDraw;
            cc->offsetBotToDraw -= cc->maxItemsToDraw;
        }
        else if (cc->offsetTopToDraw > 0)
        {
            const auto difference = cc->offsetTopToDraw;

            const auto it    = find(cc->itemsToDrew.begin(), cc->itemsToDrew.end(), cc->currentSelectedItemHandle);
            const auto index = static_cast<uint32>(it - cc->itemsToDrew.begin()) - difference;
            cc->currentSelectedItemHandle = cc->itemsToDrew[index];

            cc->offsetTopToDraw -= difference;
            cc->offsetBotToDraw -= difference;
        }
        else
        {
            cc->currentSelectedItemHandle = cc->itemsToDrew[0];
        }

        return true;
    case Key::PageDown:
        if (cc->itemsToDrew.size() == 0)
        {
            break;
        }

        if (static_cast<size_t>(cc->offsetBotToDraw) + cc->maxItemsToDraw < cc->itemsToDrew.size())
        {
            const auto it    = find(cc->itemsToDrew.begin(), cc->itemsToDrew.end(), cc->currentSelectedItemHandle);
            const auto index = static_cast<uint32>(it - cc->itemsToDrew.begin()) + cc->maxItemsToDraw;
            cc->currentSelectedItemHandle = cc->itemsToDrew[index];

            cc->offsetTopToDraw += cc->maxItemsToDraw;
            cc->offsetBotToDraw += cc->maxItemsToDraw;
        }
        else if (static_cast<size_t>(cc->offsetBotToDraw) < cc->itemsToDrew.size())
        {
            const auto difference = cc->itemsToDrew.size() - cc->offsetBotToDraw;

            const auto it    = find(cc->itemsToDrew.begin(), cc->itemsToDrew.end(), cc->currentSelectedItemHandle);
            const auto index = static_cast<uint32>(it - cc->itemsToDrew.begin()) + difference;
            cc->currentSelectedItemHandle = cc->itemsToDrew[index];

            cc->offsetTopToDraw += static_cast<uint32>(difference);
            cc->offsetBotToDraw += static_cast<uint32>(difference);
        }
        else
        {
            cc->currentSelectedItemHandle = cc->itemsToDrew[cc->itemsToDrew.size() - 1];
        }

        return true;

    case Key::Home:
        if (cc->itemsToDrew.size() == 0)
        {
            break;
        }
        cc->offsetTopToDraw           = 0;
        cc->offsetBotToDraw           = cc->maxItemsToDraw;
        cc->currentSelectedItemHandle = cc->itemsToDrew[0];
        return true;

    case Key::End:
        if (cc->itemsToDrew.size() == 0)
        {
            break;
        }
        cc->offsetTopToDraw           = static_cast<uint32>(cc->itemsToDrew.size()) - cc->maxItemsToDraw;
        cc->offsetBotToDraw           = cc->offsetTopToDraw + cc->maxItemsToDraw;
        cc->currentSelectedItemHandle = cc->itemsToDrew[cc->itemsToDrew.size() - 1];
        return true;

    case Key::Ctrl | Key::Space:
    {
        GetCurrentItem().ToggleRecursively();

        if (cc->filter.searchText.Len() > 0 && cc->filter.mode != TreeControlContext::FilterMode::None)
        {
            cc->SearchItems(this);
        }

        return true;
    }
    case Key::Space:
        GetCurrentItem().Toggle();

        cc->ProcessItemsToBeDrawn(InvalidItemHandle);
        if (cc->filter.searchText.Len() > 0 && cc->filter.mode != TreeControlContext::FilterMode::None)
        {
            cc->SearchItems(this);
        }
        return true;

    case Key::Ctrl | Key::Left:
        cc->SelectColumnSeparator(-1);
        return true;
    case Key::Ctrl | Key::Right:
        cc->SelectColumnSeparator(1);
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

        if (cc->separatorIndexSelected != InvalidIndex)
        {
            cc->separatorIndexSelected = InvalidIndex;
            return true;
        }
        break;

    case Key::Left:
    case Key::Right:
        if (cc->separatorIndexSelected != InvalidIndex)
        {
            if (cc->AddToColumnWidth(cc->separatorIndexSelected, keyCode == Key::Left ? -1 : 1))
            {
                return true;
            }
        }
        break;

    case Key::Ctrl | Key::C:
        if (const auto it = cc->items.find(cc->currentSelectedItemHandle); it != cc->items.end())
        {
            LocalUnicodeStringBuilder<1024> lusb;
            for (const auto& value : it->second.values)
            {
                if (lusb.Len() > 0)
                {
                    lusb.Add(" ");
                    lusb.Add(value);
                }
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
                cc->SearchItems(this);
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
                        foundCurrent = cc->currentSelectedItemHandle == handle;
                        continue;
                    }

                    const auto& item = cc->items[handle];
                    if (item.markedAsFound == true)
                    {
                        cc->currentSelectedItemHandle = handle;
                        return true;
                    }
                }

                // there's no next so go back to the first
                for (const auto& handle : cc->orderedItems)
                {
                    const auto& item = cc->items[handle];
                    if (item.markedAsFound == true)
                    {
                        cc->currentSelectedItemHandle = handle;
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
                        foundCurrent = cc->currentSelectedItemHandle == handle;
                        continue;
                    }

                    const auto& item = cc->items[handle];
                    if (item.markedAsFound == true)
                    {
                        cc->currentSelectedItemHandle = handle;
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
                        cc->currentSelectedItemHandle = handle;
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
            if (cc->SearchItems(this) == false)
            {
                cc->filter.searchText.Truncate(cc->filter.searchText.Len() - 1);
                if (cc->filter.searchText.Len() > 0)
                {
                    cc->SearchItems(this);
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

    if ((cc->treeFlags & TreeViewFlags::Sortable) != TreeViewFlags::None)
    {
        if (cc->filter.mode != TreeControlContext::FilterMode::Filter)
        {
            for (uint32 i = 0; i < cc->columns.size(); i++)
            {
                if (cc->columns[i].hotKeyCode == keyCode)
                {
                    cc->ColumnSort(i);
                    return true;
                }
            }
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
        if (cc->currentSelectedItemHandle == InvalidItemHandle)
        {
            cc->currentSelectedItemHandle = cc->itemsToDrew[cc->offsetTopToDraw];
        }
    }
}

void TreeView::OnMousePressed(int x, int y, Input::MouseButton button)
{
    CHECKRET(Context != nullptr, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    switch (button)
    {
    case Input::MouseButton::None:
        break;
    case Input::MouseButton::Left:
        switch (cc->isMouseOn)
        {
        case TreeControlContext::IsMouseOn::Border:
            break;
        case TreeControlContext::IsMouseOn::ColumnHeader:
            if (cc->mouseOverColumnIndex != InvalidIndex)
            {
                cc->sortAscendent = !cc->sortAscendent;
                cc->ColumnSort(cc->mouseOverColumnIndex);
            }
            break;
        case TreeControlContext::IsMouseOn::ColumnSeparator:
            break;
        case TreeControlContext::IsMouseOn::ToggleSymbol:
        {
            const uint32 index    = y - 2;
            const auto itemHandle = cc->itemsToDrew[static_cast<size_t>(cc->offsetTopToDraw) + index];
            const auto it         = cc->items.find(itemHandle);
            auto item             = TreeViewItem{ this, it->second.handle };
            item.Toggle();
            if (it->second.expanded == false)
            {
                if (cc->IsAncestorOfChild(it->second.handle, cc->currentSelectedItemHandle))
                {
                    cc->currentSelectedItemHandle = it->second.handle;
                }
            }
            cc->ProcessItemsToBeDrawn(InvalidItemHandle);
            cc->SearchItems(this);
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
                cc->currentSelectedItemHandle = itemHandle;
            }
        }
        break;
        default:
            break;
        }
        break;
    case Input::MouseButton::Center:
        break;
    case Input::MouseButton::Right:
        break;
    case Input::MouseButton::DoubleClicked:
        break;
    default:
        break;
    }
}

bool TreeView::OnMouseOver(int x, int y)
{
    CHECK(Context != nullptr, false, "");
    const auto cc            = reinterpret_cast<TreeControlContext*>(Context);
    cc->mouseOverColumnIndex = 0xFFFFFFFF;

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
    else if (cc->IsMouseOnItem(x, y))
    {
        cc->isMouseOn = TreeControlContext::IsMouseOn::Item;
    }
    else if (cc->IsMouseOnColumnHeader(x, y))
    {
        cc->isMouseOn = TreeControlContext::IsMouseOn::ColumnHeader;
    }
    else if (cc->IsMouseOnSearchField(x, y))
    {
        cc->isMouseOn = TreeControlContext::IsMouseOn::SearchField;
    }
    else
    {
        cc->isMouseOn = TreeControlContext::IsMouseOn::None;
    }

    return false;
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
        break;
    case Input::MouseWheel::Right:
        break;
    default:
        break;
    }

    return false;
}

bool TreeView::OnMouseDrag(int x, int, Input::MouseButton button)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    switch (button)
    {
    case Input::MouseButton::None:
        break;
    case Input::MouseButton::Left:
        switch (cc->isMouseOn)
        {
        case TreeControlContext::IsMouseOn::Border:
            break;
        case TreeControlContext::IsMouseOn::ColumnHeader:
            break;
        case TreeControlContext::IsMouseOn::ColumnSeparator:
            if (cc->mouseOverColumnSeparatorIndex != InvalidIndex)
            {
                const auto xs    = cc->columns[cc->mouseOverColumnSeparatorIndex].x;
                const auto w     = cc->columns[cc->mouseOverColumnSeparatorIndex].width;
                const auto delta = -(static_cast<int32>((xs + w)) - x);
                if (cc->AddToColumnWidth(cc->mouseOverColumnSeparatorIndex, delta))
                {
                    return true;
                }
            }
            break;
        case TreeControlContext::IsMouseOn::ToggleSymbol:
            break;
        case TreeControlContext::IsMouseOn::Item:
            break;
        default:
            break;
        }
        break;
    case Input::MouseButton::Center:
        break;
    case Input::MouseButton::Right:
        break;
    case Input::MouseButton::DoubleClicked:
        break;
    default:
        break;
    }

    return false;
}

void TreeView::OnUpdateScrollBars()
{
    CHECKRET(Context != nullptr, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    const auto it     = find(cc->itemsToDrew.begin(), cc->itemsToDrew.end(), cc->currentSelectedItemHandle);
    const int64 index = it - cc->itemsToDrew.begin();
    UpdateVScrollBar(index, std::max<size_t>(cc->itemsToDrew.size() - 1, 0));
}

void TreeView::OnAfterResize(int newWidth, int newHeight)
{
    CHECKRET(Context != nullptr, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);
    CHECKRET(cc->AdjustElementsOnResize(newWidth, newHeight), "");
}

Handlers::TreeView* TreeView::Handlers()
{
    GET_CONTROL_HANDLERS(Handlers::TreeView);
}

TreeViewItem TreeView::GetCurrentItem()
{
    CHECK(Context != nullptr, (TreeViewItem{ nullptr, InvalidItemHandle }), "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    return { this, cc->currentSelectedItemHandle };
}

// --------------------------------------

bool TreeViewItem::SetType(TreeViewItem::Type type)
{
    CHECK(IsValid(), false, "");

    auto cc = reinterpret_cast<TreeControlContext*>(obj.ToGenericRef().ToReference<TreeView>()->Context);
    CHECK(cc != nullptr, false, "");

    cc->items.at(handle).type = type;

    return true;
}

bool TreeViewItem::SetColor(const Graphics::ColorPair& color)
{
    CHECK(IsValid(), false, "");

    auto cc = reinterpret_cast<TreeControlContext*>(obj.ToGenericRef().ToReference<TreeView>()->Context);
    CHECK(cc != nullptr, false, "");

    cc->items.at(handle).color = color;

    return true;
}

bool TreeViewItem::SetCurrent()
{
    CHECK(IsValid(), false, "");

    auto cc = reinterpret_cast<TreeControlContext*>(obj.ToGenericRef().ToReference<TreeView>()->Context);
    CHECK(cc != nullptr, false, "");

    cc->currentSelectedItemHandle = handle;

    return true;
}

bool TreeViewItem::IsCurrent() const
{
    CHECK(IsValid(), false, "");

    auto cc = reinterpret_cast<TreeControlContext*>(obj.ToGenericRef().ToReference<TreeView>()->Context);
    CHECK(cc != nullptr, false, "");

    return cc->currentSelectedItemHandle == handle;
}

bool TreeViewItem::SetFolding(bool expand)
{
    CHECK(IsValid(), false, "");

    auto cc = reinterpret_cast<TreeControlContext*>(obj.ToGenericRef().ToReference<TreeView>()->Context);
    CHECK(cc != nullptr, false, "");

    cc->items.at(handle).expanded = expand;

    return true;
}

bool TreeViewItem::IsFolded()
{
    CHECK(IsValid(), false, "");

    auto cc = reinterpret_cast<TreeControlContext*>(obj.ToGenericRef().ToReference<TreeView>()->Context);
    CHECK(cc != nullptr, false, "");

    return cc->items.at(handle).expanded;
}

bool TreeViewItem::SetExpandable(bool expandable)
{
    CHECK(IsValid(), false, "");

    auto cc = reinterpret_cast<TreeControlContext*>(obj.ToGenericRef().ToReference<TreeView>()->Context);
    CHECK(cc != nullptr, false, "");

    cc->items.at(handle).isExpandable = expandable;

    return true;
}

bool TreeViewItem::IsExpandable() const
{
    CHECK(IsValid(), false, "");

    auto cc = reinterpret_cast<TreeControlContext*>(obj.ToGenericRef().ToReference<TreeView>()->Context);
    CHECK(cc != nullptr, false, "");

    return cc->items.at(handle).isExpandable;
}

uint32 TreeViewItem::GetChildrenCount() const
{
    CHECK(IsValid(), false, "");

    auto cc = reinterpret_cast<TreeControlContext*>(obj.ToGenericRef().ToReference<TreeView>()->Context);
    CHECK(cc != nullptr, false, "");

    return static_cast<uint32>(cc->items.at(handle).children.size());
}

TreeViewItem TreeViewItem::GetChild(uint32 index)
{
    CHECK(IsValid(), (TreeViewItem{ nullptr, InvalidItemHandle }), "");

    auto cc = reinterpret_cast<TreeControlContext*>(obj.ToGenericRef().ToReference<TreeView>()->Context);
    CHECK(cc != nullptr, (TreeViewItem{ nullptr, InvalidItemHandle }), "");
    CHECK(index < cc->items.at(handle).children.size(), (TreeViewItem{ nullptr, InvalidItemHandle }), "");

    return { this->obj, cc->items.at(handle).children.at(index) };
}

bool TreeViewItem::DeleteChildren()
{
    CHECK(IsValid(), false, "");

    const auto noChildren = GetChildrenCount();
    for (auto i = 0U; i < noChildren; i++)
    {
        auto child = GetChild(i);
        CHECK(obj->RemoveItem(child), false, "");
    }

    auto cc = reinterpret_cast<TreeControlContext*>(obj.ToGenericRef().ToReference<TreeView>()->Context);
    CHECK(cc != nullptr, false, "");

    cc->items.at(handle).children.clear();

    return true;
}

ItemHandle TreeViewItem::GetHandle() const
{
    return handle;
}

bool TreeViewItem::Toggle()
{
    CHECK(IsValid(), false, "");
    CHECK(IsExpandable(), true, ""); // nothing to expand

    auto cc = reinterpret_cast<TreeControlContext*>(obj.ToGenericRef().ToReference<TreeView>()->Context);
    CHECK(cc != nullptr, false, "");

    if (cc->treeFlags && TreeViewFlags::DynamicallyPopulateNodeChildren)
    {
        CHECK(DeleteChildren(), false, "");
    }

    SetFolding(!IsFolded());

    if (IsFolded())
    {
        if (cc->treeFlags && TreeViewFlags::DynamicallyPopulateNodeChildren)
        {
            if (cc->handlers != nullptr)
            {
                auto handler = reinterpret_cast<Controls::Handlers::TreeView*>(cc->handlers.get());
                if (handler->OnTreeItemToggle.obj)
                {
                    return handler->OnTreeItemToggle.obj->OnTreeItemToggle(*this);
                }
            }
        }
    }

    return true;
}

bool TreeViewItem::ToggleRecursively()
{
    CHECK(IsValid(), false, "");
    CHECK(IsExpandable(), true, ""); // nothing to expand

    auto cc = reinterpret_cast<TreeControlContext*>(obj.ToGenericRef().ToReference<TreeView>()->Context);
    CHECK(cc != nullptr, false, "");

    std::queue<ItemHandle> ancestorRelated;
    ancestorRelated.push(handle);

    while (ancestorRelated.empty() == false)
    {
        ItemHandle handle = ancestorRelated.front();
        ancestorRelated.pop();

        auto treeItem = obj->GetItemByHandle(handle);
        treeItem.Toggle();

        const auto& item = cc->items[handle];
        for (const auto& handle : item.children)
        {
            ancestorRelated.push(handle);
        }
    }

    if ((cc->treeFlags & TreeViewFlags::Sortable) != TreeViewFlags::None)
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
    CHECK(IsFolded(), false, "");

    return Toggle();
}

bool TreeViewItem::Unfold()
{
    CHECK(IsValid(), false, "");
    CHECK(IsExpandable(), false, "");
    CHECK(IsFolded() == false, false, "");

    return Toggle();
}

bool TreeViewItem::FoldAll()
{
    CHECK(IsValid(), false, "");
    CHECK(IsExpandable(), false, "");
    CHECK(IsFolded(), false, "");

    return ToggleRecursively();
}

bool TreeViewItem::UnfoldAll()
{
    CHECK(IsValid(), false, "");
    CHECK(IsExpandable(), false, "");
    CHECK(IsFolded() == false, false, "");

    return ToggleRecursively();
}

TreeViewItem TreeViewItem::GetParent() const
{
    CHECK(IsValid(), (TreeViewItem{ nullptr, InvalidItemHandle }), "");
    auto cc = reinterpret_cast<TreeControlContext*>(obj.ToGenericRef().ToReference<TreeView>()->Context);
    CHECK(cc != nullptr, (TreeViewItem{ nullptr, InvalidItemHandle }), "");

    auto& parent = cc->items.at(handle).parent;
    if (parent == InvalidItemHandle)
    {
        return { nullptr, InvalidItemHandle };
    }

    return { this->obj, parent };
}

TreeViewItem TreeViewItem::AddChild(ConstString name, bool isExpandable)
{
    CHECK(IsValid(), (TreeViewItem{ nullptr, InvalidItemHandle }), "");
    auto cc = reinterpret_cast<TreeControlContext*>(obj->Context);
    CHECK(cc != nullptr, (TreeViewItem{ nullptr, InvalidItemHandle }), "");

    return { obj, cc->AddItem(handle, { name }, isExpandable) };
}

bool TreeViewItem::SetText(ConstString name)
{
    CHECK(IsValid(), false, "");
    auto cc = reinterpret_cast<TreeControlContext*>(obj->Context);
    CHECK(cc != nullptr, false, "");

    return cc->items.at(handle).values.at(0).Set(name);
}

const CharacterBuffer& TreeViewItem::GetText() const
{
    static const CharacterBuffer cb{};
    CHECK(IsValid(), cb, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(obj.ToGenericRef().ToReference<TreeView>()->Context);

    const auto it = cc->items.find(handle);
    if (it != cc->items.end())
    {
        return it->second.values.at(0);
    }

    return cb;
}

bool TreeViewItem::SetValues(const std::initializer_list<ConstString> values)
{
    CHECK(IsValid(), false, "");
    auto cc = reinterpret_cast<TreeControlContext*>(obj->Context);
    CHECK(cc != nullptr, false, "");

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
    CHECK(IsValid(), false, "");
    auto cc = reinterpret_cast<TreeControlContext*>(obj->Context);
    CHECK(cc != nullptr, false, "");
    CHECK(subItemIndex < cc->columns.size(), false, "");

    auto& item = cc->items.at(handle);
    if (item.values.size() < subItemIndex)
    {
        item.values.resize(subItemIndex + 1ULL);
    }

    return item.values.at(subItemIndex).Set(text);
}

const Graphics::CharacterBuffer& TreeViewItem::GetText(uint32 subItemIndex) const
{
    static const CharacterBuffer cb{};
    CHECK(IsValid(), cb, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(obj.ToGenericRef().ToReference<TreeView>()->Context);
    CHECK(cc != nullptr, cb, "");
    CHECK(subItemIndex < cc->items.at(handle).values.size(), cb, "");

    return cc->items.at(handle).values.at(subItemIndex);
}

bool TreeViewItem::SetData(uint64 value)
{
    CHECK(IsValid(), false, "");

    auto cc = reinterpret_cast<TreeControlContext*>(obj->Context);
    CHECK(cc != nullptr, false, "");

    cc->items.at(handle).data = value;

    return true;
}

uint64 TreeViewItem::GetData(uint64 errorValue) const
{
    CHECK(IsValid(), errorValue, "");

    auto cc = reinterpret_cast<TreeControlContext*>(obj.ToGenericRef().ToReference<TreeView>()->Context);
    CHECK(cc != nullptr, errorValue, "");

    return std::get<uint64>(cc->items.at(handle).data);
}

GenericRef TreeViewItem::GetItemDataAsPointer() const
{
    CHECK(IsValid(), nullptr, "");
    auto cc = reinterpret_cast<TreeControlContext*>(obj.ToGenericRef().ToReference<TreeView>()->Context);
    CHECK(cc != nullptr, nullptr, "");

    return cc->GetItemDataAsPointer(handle);
}

bool TreeViewItem::SetItemDataAsPointer(GenericRef ref)
{
    CHECK(IsValid(), false, "");

    auto cc = reinterpret_cast<TreeControlContext*>(obj->Context);
    CHECK(cc != nullptr, false, "");

    return cc->SetItemDataAsPointer(handle, ref);
}

bool TreeView::RemoveItem(TreeViewItem& item)
{
    CHECK(item.IsValid(), false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(item.obj->Context);
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

    cc->currentSelectedItemHandle = InvalidItemHandle;

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

    return { this, it->second.handle };
}

uint32 TreeView::GetItemsCount() const
{
    CHECK(Context != nullptr, 0, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);
    return static_cast<uint32>(cc->items.size());
}

TreeViewItem TreeView::GetItemByHandle(ItemHandle handle)
{
    CHECK(Context != nullptr, (TreeViewItem{ nullptr, InvalidItemHandle }), "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    CHECK(cc->items.find(handle) != cc->items.end(), (TreeViewItem{ nullptr, InvalidItemHandle }), "");

    return { this, handle };
}

TreeViewItem TreeView::AddItem(ConstString name, bool isExpandable)
{
    auto cc = reinterpret_cast<TreeControlContext*>(this->Context);
    CHECK(cc != nullptr, (TreeViewItem{ nullptr, InvalidItemHandle }), "");

    return { this, cc->AddItem(InvalidItemHandle, { name }, isExpandable) };
}

TreeViewColumn TreeView::GetColumn(uint32 index)
{
    CHECK(Context != nullptr, (TreeViewColumn{ nullptr, 0 }), "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);
    CHECK(index < cc->columns.size(), (TreeViewColumn{ nullptr, 0 }), "");

    return { Context, index };
}

TreeViewColumn TreeView::AddColumn(const ConstString& title, Graphics::TextAlignament align, uint32 width)
{
    CHECK(Context != nullptr, (TreeViewColumn{ nullptr, 0 }), "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);
    CHECK(cc->AddColumn(title, align, width), (TreeViewColumn{ nullptr, 0 }), "");

    return { Context, static_cast<uint32>(cc->columns.size() - 1ULL) };
}

bool TreeView::AddColumns(std::initializer_list<ColumnBuilder> columns)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    for (auto& column : columns)
    {
        CHECK(cc->AddColumn(column.name, column.align, column.width), false, "");
    }

    return true;
}

uint32 TreeView::GetColumnsCount()
{
    CHECK(Context != nullptr, 0, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    return static_cast<uint32>(cc->columns.size());
}

uint32 TreeView::GetSortColumnIndex()
{
    CHECK(Context != nullptr, 0xFFFFFFFF, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    return cc->columnIndexToSortBy;
}

bool TreeView::DeleteAllColumns()
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    cc->columns.clear();
    cc->items.clear();

    return true;
}

bool TreeView::DeleteColumn(uint32 index)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);
    CHECK(index < cc->columns.size(), false, "");

    cc->columns.erase(cc->columns.begin() + index);

    return true;
}
} // namespace AppCUI::Controls

namespace AppCUI
{
void TreeControlContext::ColumnSort(uint32 columnIndex)
{
    if ((treeFlags & TreeViewFlags::Sortable) == TreeViewFlags::None)
    {
        columnIndexToSortBy = InvalidIndex;
        return;
    }

    if (columnIndex != columnIndexToSortBy)
    {
        SetSortColumn(columnIndex);
    }

    Sort();
}

void TreeControlContext::SetSortColumn(uint32 columnIndex)
{
    if ((treeFlags & TreeViewFlags::Sortable) == TreeViewFlags::None)
    {
        columnIndexToSortBy = InvalidIndex;
    }
    else
    {
        if (columnIndexToSortBy >= columns.size())
        {
            columnIndexToSortBy = InvalidIndex;
        }
        else
        {
            columnIndexToSortBy = columnIndex;
        }
    }
}

void TreeControlContext::SelectColumnSeparator(int32 offset)
{
    if (separatorIndexSelected == InvalidIndex)
    {
        separatorIndexSelected = 0;
        return;
    }

    separatorIndexSelected += offset;
    if (separatorIndexSelected < 0)
    {
        separatorIndexSelected = static_cast<int32>(columns.size() - 1);
    }
    else if (separatorIndexSelected >= columns.size())
    {
        separatorIndexSelected = 0;
    }
}

void TreeControlContext::Sort()
{
    SortByColumn(InvalidItemHandle);
    notProcessed = true;
}

bool TreeControlContext::SortByColumn(const ItemHandle handle)
{
    CHECK(columnIndexToSortBy != InvalidIndex, false, "");
    CHECK(items.size() > 0, false, "");

    const auto Comparator = [this](ItemHandle i1, ItemHandle i2) -> bool
    {
        const auto& a = items[i1];
        const auto& b = items[i2];

        const auto result = a.values[columnIndexToSortBy].CompareWith(b.values[columnIndexToSortBy], true);

        if (result == 0)
        {
            return false;
        }

        if (sortAscendent)
        {
            return result < 0;
        }
        else
        {
            return result > 0;
        }
    };

    if (handle == InvalidItemHandle)
    {
        std::sort(roots.begin(), roots.end(), Comparator);

        for (const auto& rootHandle : roots)
        {
            auto& root = items[rootHandle];
            std::sort(root.children.begin(), root.children.end(), Comparator);
            for (auto& childHandle : root.children)
            {
                SortByColumn(childHandle);
            }
        }
    }
    else
    {
        auto& item = items[handle];
        std::sort(item.children.begin(), item.children.end(), Comparator);
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

    const auto itemHandle = itemsToDrew[static_cast<size_t>(offsetTopToDraw) + index];
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

    CHECK((treeFlags & TreeViewFlags::HideColumns) == TreeViewFlags::None, false, "");

    auto i = 0U;
    for (auto& col : columns)
    {
        if (static_cast<uint32>(x) >= col.x && static_cast<uint32>(x) <= col.x + col.width)
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
    for (auto i = 0U; i < columns.size(); i++)
    {
        const auto& col = columns[i];
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

    const uint32 width =
          (static_cast<uint32>(Layout.Width)) / static_cast<uint32>(std::max<>(columns.size(), size_t(1U)));

    uint32 xPreviousColumn       = 0;
    uint32 widthOfPreviousColumn = 0;
    for (auto i = 0U; i < columns.size(); i++)
    {
        auto& col  = columns[i];
        col.height = static_cast<uint32>(Layout.Height - 2);
        col.x      = static_cast<uint32>(xPreviousColumn + widthOfPreviousColumn + BorderOffset);
        if (col.customWidth == false)
        {
            col.width = std::max<>(width, MinColumnWidth);
        }
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

bool TreeControlContext::AddToColumnWidth(const uint32 columnIndex, const int32 value)
{
    auto& currentColumn = columns[columnIndex];
    if (value < 0 && currentColumn.width == MinColumnWidth)
    {
        return true;
    }

    const auto newWidth = static_cast<int32>(currentColumn.width) + value;
    currentColumn.width = std::max<>(newWidth, static_cast<int32>(MinColumnWidth));

    auto previousX = currentColumn.x + currentColumn.width +
                     ((treeFlags & TreeViewFlags::HideColumnsSeparator) == TreeViewFlags::None);
    for (auto i = columnIndex + 1; i < columns.size(); i++)
    {
        auto& column = columns[i];
        column.x     = previousX;
        previousX += (column.width + ((treeFlags & TreeViewFlags::HideColumnsSeparator) == TreeViewFlags::None));
    }

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

bool TreeControlContext::SearchItems(Reference<TreeView> tree)
{
    bool found = false;

    MarkAllItemsAsNotFound();

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
                    if (found == false)
                    {
                        currentSelectedItemHandle = item.second.handle;
                        SetColorForItems(Cfg->Text.Normal);
                    }
                    found = true;
                    value.SetColor(index, index + filter.searchText.Len(), Cfg->Text.Highlighted);

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
        auto item = tree->GetItemByHandle(itemHandle);
        item.Toggle();
    }

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

bool TreeControlContext::ItemsPainting(Graphics::Renderer& renderer)
{
    WriteTextParams wtp;
    wtp.Y = ((treeFlags & TreeViewFlags::HideColumns) == TreeViewFlags::None) +
            ((treeFlags & TreeViewFlags::HideBorder) == TreeViewFlags::None); // 0  is for border | 1 is for header

    for (auto i = offsetTopToDraw; i < std::min<size_t>(offsetBotToDraw, itemsToDrew.size()); i++)
    {
        auto& item = items[itemsToDrew[i]];

        uint32 j = 0; // column index
        for (const auto& col : columns)
        {
            wtp.Flags = WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth;
            wtp.Align = col.alignment;
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
                if (item.handle == currentSelectedItemHandle)
                {
                    wtp.Color = Cfg->Cursor.Normal;
                }
                else if (item.markedAsFound == false)
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
                item.values[j].SetColor(wtp.Color);

                if (wtp.X < static_cast<int>(col.x + col.width))
                {
                    renderer.WriteText(item.values[j], wtp);
                }
            }

            j++;
        }

        if (item.handle == currentSelectedItemHandle)
        {
            const uint32 addX = treeFlags && TreeViewFlags::HideBorder ? 0 : 1;
            renderer.FillHorizontalLine(addX, wtp.Y, this->Layout.Width - 1 - addX, -1, Cfg->Cursor.Normal);
        }

        wtp.Y++;
    }

    return true;
}

bool TreeControlContext::PaintColumnHeaders(Graphics::Renderer& renderer)
{
    CHECK(columns.size() > 0, true, "");

    const auto controlWidth = Layout.Width - 2 * ((treeFlags && TreeViewFlags::HideBorder) ? 0 : 1) - 1;

    renderer.FillHorizontalLineSize(Layout.X, 1, controlWidth, ' ', Cfg->Header.Text.Focused);

    const auto enabled = (Flags & GATTR_ENABLE) != 0;

    WriteTextParams wtp{ WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth };
    wtp.Y     = 1; // 0  is for border
    wtp.Color = Cfg->Text.Normal;

    uint32 i = 0;
    for (const auto& col : columns)
    {
        wtp.Align = col.alignment;
        wtp.X     = col.x;
        wtp.Width = col.width - ((treeFlags & TreeViewFlags::HideColumnsSeparator) == TreeViewFlags::None);

        if (wtp.X >= controlWidth)
        {
            continue;
        }

        if (wtp.X + static_cast<int32>(wtp.Width) >= controlWidth)
        {
            wtp.Width = controlWidth - wtp.X;
        }

        if (i != columnIndexToSortBy) // skip triangle / sort sign
        {
            wtp.Width++;
        }

        wtp.Color = enabled
                          ? ((i == columnIndexToSortBy) ? Cfg->Header.Text.PressedOrSelected : Cfg->Header.Text.Focused)
                          : Cfg->Header.Text.Inactive;

        if (enabled && Focused && i == mouseOverColumnIndex && i != columnIndexToSortBy)
        {
            wtp.Color = Cfg->Header.Text.Hovered;
        }

        renderer.FillHorizontalLineSize(col.x, 1, wtp.Width, ' ', wtp.Color);

        renderer.WriteText(*const_cast<CharacterBuffer*>(&col.title), wtp);

        if (i == columnIndexToSortBy)
        {
            renderer.WriteSpecialCharacter(
                  static_cast<int32>(wtp.X + wtp.Width),
                  1,
                  sortAscendent ? SpecialChars::TriangleUp : SpecialChars::TriangleDown,
                  Cfg->Header.HotKey.PressedOrSelected);
        }

        i++;
    }

    return true;
}

bool TreeControlContext::PaintColumnSeparators(Graphics::Renderer& renderer)
{
    CHECK(columns.size() > 0, true, "");

    if (separatorIndexSelected != InvalidIndex)
    {
        CHECK(separatorIndexSelected <= columns.size(), // # columns + 1 separators
              false,
              "%u %u",
              separatorIndexSelected,
              columns.size());
    }

    for (auto i = 0U; i < columns.size(); i++)
    {
        const auto& col = columns[i];
        if (static_cast<int32>(col.x + col.width) <=
            Layout.Width - 2 * ((treeFlags & TreeViewFlags::HideBorder) == TreeViewFlags::None))
        {
            const auto& color = (separatorIndexSelected == i || mouseOverColumnSeparatorIndex == i) ? Cfg->Lines.Hovered
                                                                                                    : Cfg->Lines.Normal;
            renderer.DrawVerticalLine(
                  col.x + col.width,
                  1,
                  Layout.Height - 2 * ((treeFlags & TreeViewFlags::HideBorder) == TreeViewFlags::None),
                  color);
        }
    }

    return true;
}

bool TreeControlContext::MoveUp()
{
    if (itemsToDrew.size() > 0)
    {
        const auto it             = std::find(itemsToDrew.begin(), itemsToDrew.end(), currentSelectedItemHandle);
        const auto index          = static_cast<uint32>(it - itemsToDrew.begin());
        const auto newIndex       = std::min<uint32>(index - 1, static_cast<uint32>(itemsToDrew.size() - 1U));
        currentSelectedItemHandle = itemsToDrew[newIndex];

        if (newIndex == itemsToDrew.size() - 1)
        {
            if (itemsToDrew.size() > maxItemsToDraw)
            {
                offsetBotToDraw = static_cast<uint32>(itemsToDrew.size());
            }

            if (offsetBotToDraw >= maxItemsToDraw)
            {
                offsetTopToDraw = offsetBotToDraw - maxItemsToDraw;
            }
        }
        else if (newIndex < offsetTopToDraw && offsetTopToDraw > 0)
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
        const auto it       = std::find(itemsToDrew.begin(), itemsToDrew.end(), currentSelectedItemHandle);
        const auto index    = static_cast<uint32>(it - itemsToDrew.begin());
        const auto newIndex = std::min<uint32>(index + 1, (index + 1ULL > itemsToDrew.size() - 1 ? 0 : index + 1));
        currentSelectedItemHandle = itemsToDrew[newIndex];

        if (newIndex == 0)
        {
            offsetBotToDraw = maxItemsToDraw;
            offsetTopToDraw = 0;
        }
        else if (newIndex >= offsetBotToDraw)
        {
            offsetBotToDraw++;
            offsetTopToDraw++;
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
            for (const auto& handle : items[current].children)
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

bool TreeControlContext::AddColumn(
      const ConstString title, const Graphics::TextAlignament alignment, const uint32 width)
{
    auto x = 0U + ((treeFlags & TreeViewFlags::HideBorder) == TreeViewFlags::None);

    for (const auto& column : columns)
    {
        x += column.width;
    }

    auto& column     = columns.emplace_back();
    const auto index = columns.size() - 1;

    column.height = Layout.Height - 2 * ((treeFlags & TreeViewFlags::HideBorder) == TreeViewFlags::None);

    CHECK(column.title.SetWithHotKey(title, column.hotKeyOffset, column.hotKeyCode, Key::Ctrl), false, "");
    column.alignment = alignment;
    if (width != 0xFFFFFFFF)
    {
        column.width       = std::max<>(width, MinColumnWidth);
        column.customWidth = true;

        // shifts columns
        uint32 currentX = column.x + column.width;
        for (auto i = index + 1; i < columns.size(); i++)
        {
            auto& col       = columns[i];
            col.customWidth = true;
            if (currentX < col.x)
            {
                const auto diff = col.x - currentX;
                col.x -= diff;
            }
            else
            {
                const auto diff = currentX - col.x;
                col.x += diff;
            }
            currentX = col.x + col.width + 1;
        }
    }

    // shift columns back if needed
    uint32 maxRightX = Layout.Width - ((treeFlags & TreeViewFlags::HideBorder) == TreeViewFlags::None);
    for (auto i = static_cast<int>(columns.size()) - 1; i >= 0; i--)
    {
        auto& col                = columns[i];
        const auto currentRightX = col.x + col.width;
        if (currentRightX > maxRightX)
        {
            const auto diff = currentRightX - maxRightX;
            if (col.width > diff && col.width - diff >= MinColumnWidth)
            {
                col.width -= diff;
            }
            else
            {
                col.x -= diff;
            }
        }
        maxRightX = col.x;
    }

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
        currentSelectedItemHandle = items[nextItemHandle].handle;
    }

    notProcessed = true;

    return items[nextItemHandle++].handle;
}

// -----------------------------------------------------------------------------------------------------------

bool TreeViewColumn::SetText(const ConstString& text)
{
    CHECK(context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(context);

    CHECK(index < cc->columns.size(),
          false,
          "Invalid column index:%d (should be smaller than %d)",
          index,
          cc->columns.size());

    return cc->columns[index].title.Set(text);
}

bool TreeViewColumn::SetAlignament(TextAlignament Align)
{
    CHECK(context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(context);

    CHECK(index < cc->columns.size(),
          false,
          "Invalid column index:%d (should be smaller than %d)",
          index,
          cc->columns.size());

    cc->columns[index].alignment = Align;

    return true;
}

bool TreeViewColumn::SetWidth(uint32 width)
{
    CHECK(context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(context);

    CHECK(index < cc->columns.size(),
          false,
          "Invalid column index:%d (should be smaller than %d)",
          index,
          cc->columns.size());

    cc->columns[index].width       = width;
    cc->columns[index].customWidth = true;

    return true;
}
} // namespace AppCUI
