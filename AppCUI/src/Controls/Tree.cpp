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

Tree::Tree(std::string_view layout, const TreeFlags flags, const unsigned int noOfColumns)
    : Control(new TreeControlContext(), "", layout, true)
{
    const auto cc        = reinterpret_cast<TreeControlContext*>(Context);
    cc->Layout.MinHeight = 1;
    cc->Layout.MaxHeight = 200000;
    cc->Layout.MinWidth  = 20;

    cc->treeFlags = static_cast<unsigned int>(flags);

    cc->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;

    if ((cc->treeFlags & TreeFlags::HideScrollBar) == TreeFlags::None)
    {
        cc->Flags |= GATTR_VSCROLL;
        cc->ScrollBars.LeftMargin     = TreeScrollbarLeftOffset; // search field
        cc->ScrollBars.TopMargin      = BorderOffset + 1;        // border + column header
        cc->ScrollBars.OutsideControl = false;
    }

    if ((cc->treeFlags & TreeFlags::HideSearchBar) == TreeFlags::None)
    {
        if ((cc->treeFlags & TreeFlags::FilterSearch) != TreeFlags::None)
        {
            cc->filter.mode = TreeControlContext::FilterMode::Filter;
        }
        else if ((cc->treeFlags & TreeFlags::Searchable) != TreeFlags::None)
        {
            cc->filter.mode = TreeControlContext::FilterMode::Search;
        }
        else
        {
            cc->treeFlags |= static_cast<unsigned int>(TreeFlags::HideSearchBar);
        }
    }

    AdjustItemsBoundsOnResize();

    const auto columnsCount = std::max<>(noOfColumns, 1U);
    const auto width        = std::max<>((static_cast<unsigned int>(cc->Layout.Width) / columnsCount), MinColumnWidth);
    for (auto i = 0U; i < columnsCount; i++)
    {
        TreeColumnData cd{ static_cast<unsigned int>(cc->columns.size() * width + BorderOffset),
                           width,
                           static_cast<unsigned int>(cc->Layout.Height - 2),
                           {},
                           TextAlignament::Center,
                           TextAlignament::Left };
        cc->columns.emplace_back(cd);
    }

    cc->separatorIndexSelected = InvalidIndex;

    SetColorForItems(cc->Cfg->Tree.Text.Normal);

    cc->itemsToDrew.reserve(100);
    cc->orderedItems.reserve(100);
}

bool Tree::ItemsPainting(Graphics::Renderer& renderer, const ItemHandle ih) const
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    WriteTextParams wtp;
    wtp.Y = 2; // 0  is for border | 1 is for header

    for (auto i = cc->offsetTopToDraw; i < std::min<size_t>(cc->offsetBotToDraw, cc->itemsToDrew.size()); i++)
    {
        auto& item = cc->items[cc->itemsToDrew[i]];

        unsigned int j = 0; // column index
        for (const auto& col : cc->columns)
        {
            wtp.Flags = WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth;
            wtp.Align = col.contentAlignment;
            if (j == 0)
            {
                wtp.X     = col.x + item.depth * ItemSymbolOffset - 1;
                wtp.Width = col.width - item.depth * ItemSymbolOffset + ItemSymbolOffset - 2;

                if (wtp.X < static_cast<int>(col.x + col.width))
                {
                    if (item.isExpandable)
                    {
                        if (item.expanded)
                        {
                            wtp.Color = cc->Cfg->Tree.Symbol.Expanded;
                            renderer.WriteSpecialCharacter(wtp.X, wtp.Y, SpecialChars::TriangleDown, wtp.Color);
                        }
                        else
                        {
                            wtp.Color = cc->Cfg->Tree.Symbol.Collapsed;
                            renderer.WriteSpecialCharacter(wtp.X, wtp.Y, SpecialChars::TriangleRight, wtp.Color);
                        }
                    }
                    else
                    {
                        wtp.Color = cc->Cfg->Tree.Symbol.SingleElement;
                        renderer.WriteSpecialCharacter(wtp.X, wtp.Y, SpecialChars::CircleFilled, wtp.Color);
                    }
                }

                wtp.X += ItemSymbolOffset;
            }
            else
            {
                wtp.X = col.x + 1;
                if (j == cc->columns.size() - 1)
                {
                    wtp.Width = col.width - BorderOffset - 2;
                }
                else
                {
                    wtp.Width = col.width; // column separator
                }
            }

            if (j == item.values.size() - 1 && item.handle == cc->currentSelectedItemHandle)
            {
                wtp.Flags = WriteTextFlags::SingleLine | WriteTextFlags::OverwriteColors | WriteTextFlags::ClipToWidth;
                wtp.Color = cc->Cfg->Tree.Text.Focused;

                renderer.FillHorizontalLine(
                      1, wtp.Y, std::min<>(col.x + col.width, cc->Layout.Width - 2U), -1, wtp.Color);
            }

            if (j < item.values.size())
            {
                if (wtp.X < static_cast<int>(col.x + col.width))
                {
                    renderer.WriteText(item.values[j], wtp);
                }
            }

            j++;
        }

        wtp.Y++;
    }

    return true;
}

bool Tree::PaintColumnHeaders(Graphics::Renderer& renderer)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);
    CHECK(cc->columns.size() > 0, true, "");

    WriteTextParams wtp{ WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth };
    wtp.Y     = 1; // 0  is for border
    wtp.Color = cc->Cfg->Tree.Column.Text;

    {
        const auto& firstColumn = cc->columns[0];
        const auto& lastColumn  = cc->columns[cc->columns.size() - 1];
        const auto rightX       = std::min<>(lastColumn.x + lastColumn.width, cc->Layout.Width - 2U);
        renderer.FillHorizontalLine(firstColumn.x, 1, rightX, ' ', cc->Cfg->Tree.Column.Header);
    }

    unsigned int i = 0;
    for (const auto& col : cc->columns)
    {
        wtp.Align = col.headerAlignment;
        wtp.X     = col.x + 1;
        if (i == cc->columns.size() - 1)
        {
            wtp.Width = col.width - 3;
        }
        else
        {
            wtp.Width = col.width - 1 - 1; // left vertical line | right vertical line
        }

        renderer.WriteText(*const_cast<CharacterBuffer*>(&col.headerValue), wtp);
        i++;
    }

    return true;
}

bool Tree::PaintColumnSeparators(Graphics::Renderer& renderer)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);
    CHECK(cc->columns.size() > 0, true, "");

    if (cc->separatorIndexSelected != InvalidIndex)
    {
        CHECK(cc->separatorIndexSelected <= cc->columns.size(), // # columns + 1 separators
              false,
              "%u %u",
              cc->separatorIndexSelected,
              cc->columns.size());
    }

    {
        const auto& firstColumn = cc->columns[0];
        if (cc->separatorIndexSelected == 0)
        {
            renderer.DrawVerticalLine(firstColumn.x, 1, cc->Layout.Height - 2, cc->Cfg->Tree.Separator.Focused);
        }
        else
        {
            renderer.DrawVerticalLine(firstColumn.x, 1, cc->Layout.Height - 2, cc->Cfg->Tree.Separator.Normal);
        }
    }

    for (auto i = 1U; i < cc->columns.size(); i++)
    {
        const auto& col = cc->columns[i];
        if (cc->separatorIndexSelected == i)
        {
            renderer.DrawVerticalLine(col.x, 1, cc->Layout.Height - 2, cc->Cfg->Tree.Separator.Focused);
        }
        else
        {
            renderer.DrawVerticalLine(col.x, 1, cc->Layout.Height - 2, cc->Cfg->Tree.Separator.Normal);
        }
    }

    {
        const auto& lastColumn = cc->columns[cc->columns.size() - 1];
        const auto rightX      = std::min<>(lastColumn.x + lastColumn.width, cc->Layout.Width - 2U);
        if (cc->separatorIndexSelected == cc->columns.size())
        {
            renderer.DrawVerticalLine(rightX, 1, cc->Layout.Height - 2, cc->Cfg->Tree.Separator.Focused);
        }
        else
        {
            renderer.DrawVerticalLine(rightX, 1, cc->Layout.Height - 2, cc->Cfg->Tree.Separator.Normal);
        }
    }

    return true;
}

bool Tree::MoveUp()
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    if (cc->itemsToDrew.size() > 0)
    {
        const auto it       = find(cc->itemsToDrew.begin(), cc->itemsToDrew.end(), cc->currentSelectedItemHandle);
        const auto index    = static_cast<unsigned int>(it - cc->itemsToDrew.begin());
        const auto newIndex = std::min<unsigned int>(index - 1, static_cast<unsigned int>(cc->itemsToDrew.size() - 1U));
        cc->currentSelectedItemHandle = cc->itemsToDrew[newIndex];

        if (newIndex == cc->itemsToDrew.size() - 1)
        {
            if (cc->itemsToDrew.size() > cc->maxItemsToDraw)
            {
                cc->offsetBotToDraw = static_cast<unsigned int>(cc->itemsToDrew.size());
            }

            if (cc->offsetBotToDraw >= cc->maxItemsToDraw)
            {
                cc->offsetTopToDraw = cc->offsetBotToDraw - cc->maxItemsToDraw;
            }
        }
        else if (newIndex < cc->offsetTopToDraw && cc->offsetTopToDraw > 0)
        {
            cc->offsetBotToDraw--;
            cc->offsetTopToDraw--;
        }

        return true;
    }

    return false;
}

bool Tree::MoveDown()
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    if (cc->itemsToDrew.size() > 0)
    {
        const auto it    = find(cc->itemsToDrew.begin(), cc->itemsToDrew.end(), cc->currentSelectedItemHandle);
        const auto index = static_cast<unsigned int>(it - cc->itemsToDrew.begin());
        const auto newIndex =
              std::min<unsigned int>(index + 1, (index + 1 > cc->itemsToDrew.size() - 1 ? 0 : index + 1));
        cc->currentSelectedItemHandle = cc->itemsToDrew[newIndex];

        if (newIndex == 0)
        {
            cc->offsetBotToDraw = cc->maxItemsToDraw;
            cc->offsetTopToDraw = 0;
        }
        else if (newIndex >= cc->offsetBotToDraw)
        {
            cc->offsetBotToDraw++;
            cc->offsetTopToDraw++;
        }

        return true;
    }

    return false;
}

bool Tree::ProcessItemsToBeDrawn(const ItemHandle handle, bool clear)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    if (clear)
    {
        cc->itemsToDrew.clear();
        cc->itemsToDrew.reserve(cc->items.size());
    }

    CHECK(cc->items.size() > 0, true, "");

    if (handle == InvalidItemHandle)
    {
        for (const auto& handle : cc->roots)
        {
            const auto& item = cc->items[handle];
            if (cc->filter.mode == TreeControlContext::FilterMode::Filter && cc->filter.searchText.Len() > 0)
            {
                if (item.hasAChildThatIsMarkedAsFound == false && item.markedAsFound == false)
                {
                    continue;
                }
            }

            cc->itemsToDrew.emplace_back(handle);

            if (item.isExpandable == false || item.expanded == false)
            {
                continue;
            }

            for (auto& it : item.children)
            {
                const auto& child = cc->items[it];

                if (cc->filter.mode == TreeControlContext::FilterMode::Filter && cc->filter.searchText.Len() > 0)
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
                        cc->itemsToDrew.emplace_back(child.handle);
                    }
                }
                else
                {
                    cc->itemsToDrew.emplace_back(child.handle);
                }
            }
        }
    }
    else
    {
        const auto& item = cc->items[handle];

        if (cc->filter.mode == TreeControlContext::FilterMode::Filter && cc->filter.searchText.Len() > 0)
        {
            if (item.hasAChildThatIsMarkedAsFound == false && item.markedAsFound == false)
            {
                return true;
            }
        }

        cc->itemsToDrew.emplace_back(item.handle);
        CHECK(item.isExpandable, true, "");

        for (auto& it : item.children)
        {
            const auto& child = cc->items[it];

            if (cc->filter.mode == TreeControlContext::FilterMode::Filter && cc->filter.searchText.Len() > 0)
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
                    cc->itemsToDrew.emplace_back(child.handle);
                }
            }
            else
            {
                cc->itemsToDrew.emplace_back(child.handle);
            }
        }
    }

    return true;
}

bool Tree::IsAncestorOfChild(const ItemHandle ancestor, const ItemHandle child) const
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    std::queue<ItemHandle> ancestorRelated;
    ancestorRelated.push(ancestor);

    while (ancestorRelated.empty() == false)
    {
        ItemHandle current = ancestorRelated.front();
        ancestorRelated.pop();

        for (const auto& handle : cc->items[current].children)
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

bool Tree::ToggleExpandRecursive(const ItemHandle handle)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    std::queue<ItemHandle> ancestorRelated;
    ancestorRelated.push(handle);

    while (ancestorRelated.empty() == false)
    {
        ItemHandle current = ancestorRelated.front();
        ancestorRelated.pop();

        ToggleItem(current);

        const auto& item = cc->items[current];
        for (const auto& handle : item.children)
        {
            ancestorRelated.push(handle);
        }
    }

    cc->notProcessed = true;

    return true;
}

void Tree::Paint(Graphics::Renderer& renderer)
{
    CHECKRET(Context != nullptr, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    if ((cc->treeFlags & TreeFlags::HideBorder) == TreeFlags::None)
    {
        renderer.DrawRectSize(0, 0, cc->Layout.Width, cc->Layout.Height, cc->Cfg->Tree.Border, false);
    }

    PaintColumnHeaders(renderer);
    PaintColumnSeparators(renderer);

    if (cc->notProcessed)
    {
        ProcessItemsToBeDrawn(InvalidItemHandle);
        cc->notProcessed = false;
    }

    if ((cc->treeFlags & TreeFlags::HideScrollBar) == TreeFlags::None)
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

    ItemsPainting(renderer, InvalidItemHandle);

    if (cc->Focused)
    {
        if (cc->Layout.Width > TreeSearchBarWidth && cc->filter.mode != TreeControlContext::FilterMode::None)
        {
            renderer.FillHorizontalLine(1, cc->Layout.Height - 1, TreeSearchBarWidth, ' ', cc->Cfg->Tree.Text.Filter);

            if (const auto searchTextLen = cc->filter.searchText.Len(); searchTextLen > 0)
            {
                if (const auto searchText = cc->filter.searchText.ToStringView();
                    searchText.length() < TreeSearchBarWidth - 2)
                {
                    renderer.WriteSingleLineText(2, cc->Layout.Height - 1, searchText, cc->Cfg->ListView.FilterText);
                    renderer.SetCursor((int) (2 + searchText.length()), cc->Layout.Height - 1);
                }
                else
                {
                    renderer.WriteSingleLineText(
                          2,
                          cc->Layout.Height - 1,
                          searchText.substr(searchText.length() - TreeSearchBarWidth + 2, TreeSearchBarWidth - 2),
                          cc->Cfg->ListView.FilterText);
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

bool Tree::OnKeyEvent(Input::Key keyCode, char16_t character)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    switch (keyCode)
    {
    case Key::Up:
        return MoveUp();
    case Key::Down:
        if (cc->separatorIndexSelected != InvalidIndex)
        {
            cc->separatorIndexSelected = InvalidIndex;
            return true;
        }
        else
        {
            return MoveDown();
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
            const auto index = static_cast<unsigned int>(it - cc->itemsToDrew.begin()) - cc->maxItemsToDraw;
            cc->currentSelectedItemHandle = cc->itemsToDrew[index];

            cc->offsetTopToDraw -= cc->maxItemsToDraw;
            cc->offsetBotToDraw -= cc->maxItemsToDraw;
        }
        else if (cc->offsetTopToDraw > 0)
        {
            const auto difference = cc->offsetTopToDraw;

            const auto it    = find(cc->itemsToDrew.begin(), cc->itemsToDrew.end(), cc->currentSelectedItemHandle);
            const auto index = static_cast<unsigned int>(it - cc->itemsToDrew.begin()) - difference;
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
            const auto index = static_cast<unsigned int>(it - cc->itemsToDrew.begin()) + cc->maxItemsToDraw;
            cc->currentSelectedItemHandle = cc->itemsToDrew[index];

            cc->offsetTopToDraw += cc->maxItemsToDraw;
            cc->offsetBotToDraw += cc->maxItemsToDraw;
        }
        else if (static_cast<size_t>(cc->offsetBotToDraw) < cc->itemsToDrew.size())
        {
            const auto difference = cc->itemsToDrew.size() - cc->offsetBotToDraw;

            const auto it    = find(cc->itemsToDrew.begin(), cc->itemsToDrew.end(), cc->currentSelectedItemHandle);
            const auto index = static_cast<unsigned int>(it - cc->itemsToDrew.begin()) + difference;
            cc->currentSelectedItemHandle = cc->itemsToDrew[index];

            cc->offsetTopToDraw += static_cast<unsigned int>(difference);
            cc->offsetBotToDraw += static_cast<unsigned int>(difference);
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
        cc->offsetTopToDraw           = static_cast<unsigned int>(cc->itemsToDrew.size()) - cc->maxItemsToDraw;
        cc->offsetBotToDraw           = cc->offsetTopToDraw + cc->maxItemsToDraw;
        cc->currentSelectedItemHandle = cc->itemsToDrew[cc->itemsToDrew.size() - 1];
        return true;

    case Key::Ctrl | Key::Space:
        ToggleExpandRecursive(cc->currentSelectedItemHandle);
        {
            if (cc->filter.searchText.Len() > 0 && cc->filter.mode != TreeControlContext::FilterMode::None)
            {
                SearchItems();
            }
        }
        return true;
    case Key::Space:
        ToggleItem(cc->currentSelectedItemHandle);
        ProcessItemsToBeDrawn(InvalidItemHandle);
        if (cc->filter.searchText.Len() > 0 && cc->filter.mode != TreeControlContext::FilterMode::None)
        {
            SearchItems();
        }
        return true;

    case Key::Ctrl | Key::Left:
        if (cc->separatorIndexSelected == InvalidIndex)
        {
            cc->separatorIndexSelected = 0;
        }
        else
        {
            if (cc->separatorIndexSelected > 0)
            {
                cc->separatorIndexSelected--;
            }
            else
            {
                cc->separatorIndexSelected = static_cast<unsigned int>(cc->columns.size());
            }
        }
        return true;
    case Key::Ctrl | Key::Right:
        if (cc->separatorIndexSelected == InvalidIndex)
        {
            cc->separatorIndexSelected = 0;
        }
        else
        {
            if (cc->separatorIndexSelected < static_cast<unsigned int>(cc->columns.size()))
            {
                cc->separatorIndexSelected++;
            }
            else
            {
                cc->separatorIndexSelected = 0;
            }
        }
        return true;
    case Key::Escape:
        if (cc->filter.mode == TreeControlContext::FilterMode::Search)
        {
            if (cc->filter.searchText.Len() > 0)
            {
                cc->filter.searchText.Clear();
                SetColorForItems(cc->Cfg->Tree.Text.Normal);
                return true;
            }
        }
        else if (cc->filter.mode == TreeControlContext::FilterMode::Filter)
        {
            if (cc->filter.searchText.Len() > 0)
            {
                cc->filter.searchText.Clear();
                SetColorForItems(cc->Cfg->Tree.Text.Normal);
                ProcessItemsToBeDrawn(InvalidItemHandle);
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
        if (cc->separatorIndexSelected != InvalidIndex && cc->separatorIndexSelected != 0 &&
            cc->separatorIndexSelected != cc->columns.size())
        {
            auto previousIndex = static_cast<unsigned int>(cc->separatorIndexSelected - 1);
            if (AddToColumnWidth(previousIndex, keyCode == Key::Left ? -1 : 1))
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

                if (cc->filter.searchText.Len() > 0)
                {
                    SearchItems();
                }
                else
                {
                    SetColorForItems(cc->Cfg->Tree.Text.Normal);
                }
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
            SetColorForItems(cc->Cfg->Tree.Text.Normal);
            if (SearchItems() == false)
            {
                cc->filter.searchText.Truncate(cc->filter.searchText.Len() - 1);
                if (cc->filter.searchText.Len() > 0)
                {
                    SearchItems();
                }
                else
                {
                    SetColorForItems(cc->Cfg->Tree.Text.Normal);
                }
            }
            return true;
        }
    }

    return false;
}

void Tree::OnFocus()
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

void Tree::OnMousePressed(int x, int y, Input::MouseButton button)
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
        case TreeControlContext::IsMouseOn::ToggleSymbol:
        {
            const unsigned int index = y - 2;
            const auto itemHandle    = cc->itemsToDrew[static_cast<size_t>(cc->offsetTopToDraw) + index];
            const auto it            = cc->items.find(itemHandle);
            ToggleItem(it->second.handle);
            if (it->second.expanded == false)
            {
                if (IsAncestorOfChild(it->second.handle, cc->currentSelectedItemHandle))
                {
                    cc->currentSelectedItemHandle = it->second.handle;
                }
            }
            ProcessItemsToBeDrawn(InvalidItemHandle);
            SearchItems();
        }
        break;
        case TreeControlContext::IsMouseOn::Item:
        {
            const unsigned int index = y - 2;
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

bool Tree::OnMouseOver(int x, int y)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    if (IsMouseOnBorder(x, y))
    {
        cc->isMouseOn = TreeControlContext::IsMouseOn::Border;
    }
    else if (IsMouseOnToggleSymbol(x, y))
    {
        cc->isMouseOn = TreeControlContext::IsMouseOn::ToggleSymbol;
    }
    else if (IsMouseOnItem(x, y))
    {
        cc->isMouseOn = TreeControlContext::IsMouseOn::Item;
    }
    else if (IsMouseOnColumnSeparator(x, y))
    {
        cc->isMouseOn = TreeControlContext::IsMouseOn::ColumnSeparator;
    }
    else if (IsMouseOnColumnHeader(x, y))
    {
        cc->isMouseOn = TreeControlContext::IsMouseOn::ColumnHeader;
    }
    else if (IsMouseOnSearchField(x, y))
    {
        cc->isMouseOn = TreeControlContext::IsMouseOn::SearchField;
    }
    else
    {
        cc->isMouseOn = TreeControlContext::IsMouseOn::None;
    }

    return false;
}

bool Tree::OnMouseWheel(int x, int y, Input::MouseWheel direction)
{
    switch (direction)
    {
    case Input::MouseWheel::None:
        break;
    case Input::MouseWheel::Up:
        return MoveUp();
    case Input::MouseWheel::Down:
        return MoveDown();
    case Input::MouseWheel::Left:
        break;
    case Input::MouseWheel::Right:
        break;
    default:
        break;
    }

    return false;
}

void Tree::OnUpdateScrollBars()
{
    CHECKRET(Context != nullptr, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    const auto it         = find(cc->itemsToDrew.begin(), cc->itemsToDrew.end(), cc->currentSelectedItemHandle);
    const long long index = it - cc->itemsToDrew.begin();
    UpdateVScrollBar(index, std::max<size_t>(cc->itemsToDrew.size() - 1, 0));
}

void Tree::OnAfterResize(int newWidth, int newHeight)
{
    CHECKRET(AdjustElementsOnResize(newWidth, newHeight), "");
}

Handlers::Tree* Tree::Handlers()
{
    GET_CONTROL_HANDLERS(Handlers::Tree);
}

ItemHandle Tree::AddItem(
      const ItemHandle parent,
      const std::vector<CharacterBuffer>& values,
      const ConstString metadata,
      bool process,
      bool isExpandable)
{
    CHECK(values.size() > 0, InvalidItemHandle, "");

    CHECK(Context != nullptr, InvalidItemHandle, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    cc->items[cc->nextItemHandle]              = { parent, cc->nextItemHandle, values };
    cc->items[cc->nextItemHandle].isExpandable = isExpandable;
    CHECK(cc->items[cc->nextItemHandle].metadata.Set(metadata), false, "");

    if (parent == InvalidItemHandle)
    {
        cc->roots.emplace_back(cc->items[cc->nextItemHandle].handle);
    }
    else
    {
        auto& parentItem                    = cc->items[parent];
        cc->items[cc->nextItemHandle].depth = parentItem.depth + 1;
        parentItem.children.emplace_back(cc->items[cc->nextItemHandle].handle);
        parentItem.isExpandable = true;
    }

    if (cc->items.size() == 1)
    {
        cc->currentSelectedItemHandle = cc->items[cc->nextItemHandle].handle;
    }

    if (process)
    {
        ProcessItemsToBeDrawn(InvalidItemHandle);
    }
    else
    {
        cc->notProcessed = true;
    }

    return cc->items[cc->nextItemHandle++].handle;
}

bool Tree::RemoveItem(const ItemHandle handle, bool process)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    std::queue<ItemHandle> ancestorRelated;
    ancestorRelated.push(handle);

    while (ancestorRelated.empty() == false)
    {
        ItemHandle current = ancestorRelated.front();
        ancestorRelated.pop();

        if (const auto it = cc->items.find(handle); it != cc->items.end())
        {
            for (const auto& handle : cc->items[current].children)
            {
                ancestorRelated.push(handle);
            }
            cc->items.erase(it);

            if (const auto rootIt = std::find(cc->roots.begin(), cc->roots.end(), handle); rootIt != cc->roots.end())
            {
                cc->roots.erase(rootIt);
            }
        }
    }

    if (process)
    {
        ProcessItemsToBeDrawn(InvalidItemHandle);
    }
    else
    {
        cc->notProcessed = true;
    }

    return true;
}

bool Tree::ClearItems()
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    cc->items.clear();

    cc->nextItemHandle = 1ULL;

    cc->currentSelectedItemHandle = InvalidItemHandle;

    cc->roots.clear();

    ProcessItemsToBeDrawn(InvalidItemHandle);

    return true;
}

ItemHandle Tree::GetCurrentItem()
{
    CHECK(Context != nullptr, InvalidItemHandle, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    return cc->currentSelectedItemHandle;
}

const ConstString Tree::GetItemText(const ItemHandle handle)
{
    CHECK(Context != nullptr, u"", "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    const auto it = cc->items.find(handle);
    if (it != cc->items.end())
    {
        return it->second.values.at(0);
    }

    static const ConstString cs{ "u" };
    return cs;
}

GenericRef Tree::GetItemDataAsPointer(const ItemHandle handle) const
{
    CHECK(Context != nullptr, nullptr, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    const auto it = cc->items.find(handle);
    if (it != cc->items.end())
    {
        if (std::holds_alternative<GenericRef>(it->second.data))
            return std::get<GenericRef>(it->second.data);
    }

    return nullptr;
}

unsigned long long Tree::GetItemData(const size_t index, unsigned long long errorValue)
{
    CHECK(Context != nullptr, errorValue, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    auto it = cc->items.begin();
    std::advance(it, index);
    if (it != cc->items.end())
    {
        if (std::holds_alternative<unsigned long long>(it->second.data))
            return std::get<unsigned long long>(it->second.data);
    }

    return errorValue;
}

ItemHandle Tree::GetItemHandleByIndex(const unsigned int index) const
{
    CHECK(Context != nullptr, InvalidItemHandle, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);
    CHECK(index < cc->items.size(), InvalidItemHandle, "");

    auto it = cc->items.begin();
    std::advance(it, index);
    if (it != cc->items.end())
    {
        return it->second.handle;
    }

    return InvalidItemHandle;
}

bool Tree::SetItemDataAsPointer(ItemHandle item, GenericRef value)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    auto it = cc->items.find(item);
    if (it == cc->items.end())
        return false;
    it->second.data = value;
    return true;
}
bool Tree::SetItemData(ItemHandle item, unsigned long long value)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    auto it = cc->items.find(item);
    if (it == cc->items.end())
        return false;
    it->second.data = value;
    return true;
}
unsigned int Tree::GetItemsCount() const
{
    CHECK(Context != nullptr, 0, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);
    return static_cast<unsigned int>(cc->items.size());
}

bool Tree::AddColumnData(
      const unsigned int index,
      const ConstString title,
      const TextAlignament headerAlignment,
      const TextAlignament contentAlignment,
      const unsigned int width)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    CHECK(index < cc->columns.size(), false, "");

    auto& column = cc->columns[index];

    CHECK(column.headerValue.Set(title), false, "");
    column.headerAlignment  = headerAlignment;
    column.contentAlignment = contentAlignment;
    if (width != 0xFFFFFFFF)
    {
        column.width       = std::max<>(width, MinColumnWidth);
        column.customWidth = true;

        // shifts columns
        unsigned int currentX = column.x + column.width;
        for (auto i = index + 1; i < cc->columns.size(); i++)
        {
            auto& col       = cc->columns[i];
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
    auto maxRightX = cc->Layout.Width - BorderOffset;
    for (auto i = static_cast<int>(cc->columns.size()) - 1; i >= 0; i--)
    {
        auto& col                = cc->columns[i];
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

bool Tree::ToggleItem(const ItemHandle handle)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    auto& item = cc->items[handle];
    CHECK(item.isExpandable, true, "");

    if (cc->treeFlags && TreeFlags::DynamicallyPopulateNodeChildren)
    {
        for (const auto& child : item.children)
        {
            RemoveItem(child);
        }
        item.children.clear();
    }

    item.expanded = !item.expanded;

    if (item.expanded)
    {
        if (cc->treeFlags && TreeFlags::DynamicallyPopulateNodeChildren)
        {
            if (cc->handlers != nullptr)
            {
                auto handler = reinterpret_cast<Controls::Handlers::Tree*>(cc->handlers.get());
                if (handler->OnTreeItemToggle.obj)
                {
                    handler->OnTreeItemToggle.obj->OnTreeItemToggle(this, handle);
                }
            }
        }
    }

    return true;
}

bool Tree::IsMouseOnToggleSymbol(int x, int y) const
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    const unsigned int index = y - 2;
    if (index >= cc->offsetBotToDraw || index >= cc->itemsToDrew.size())
    {
        return false;
    }

    const auto itemHandle = cc->itemsToDrew[static_cast<size_t>(cc->offsetTopToDraw) + index];
    const auto it         = cc->items.find(itemHandle);

    if (x > static_cast<int>(it->second.depth * ItemSymbolOffset + ItemSymbolOffset) &&
        x < static_cast<int>(cc->Layout.Width))
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

bool Tree::IsMouseOnItem(int x, int y) const
{
    CHECK(Context != nullptr, false, "");
    const auto cc            = reinterpret_cast<TreeControlContext*>(Context);
    const unsigned int index = y - 2;
    if (index >= cc->offsetBotToDraw || index >= cc->itemsToDrew.size())
    {
        return false;
    }

    const auto itemHandle = cc->itemsToDrew[static_cast<size_t>(cc->offsetTopToDraw) + index];
    const auto it         = cc->items.find(itemHandle);

    return (
          x > static_cast<int>(it->second.depth * ItemSymbolOffset + ItemSymbolOffset) &&
          x < static_cast<int>(cc->Layout.Width));
}

bool Tree::IsMouseOnBorder(int x, int y) const
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    return (x == 0 || x == cc->Layout.Width - BorderOffset) || (y == 0 || y == cc->Layout.Width - BorderOffset);
}

bool Tree::IsMouseOnColumnHeader(int x, int y) const
{
    // TODO:
    return false;
}

bool Tree::IsMouseOnColumnSeparator(int x, int y) const
{
    // TODO:
    return false;
}

bool Tree::IsMouseOnSearchField(int x, int y) const
{
    if (this->HasFocus())
    {
        if (y == GetHeight() - 1)
        {
            if (x > 0 && x < TreeSearchBarWidth)
            {
                return true;
            }
        }
    }

    return false;
}

bool Tree::AdjustElementsOnResize(const int newWidth, const int newHeight)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    CHECK(AdjustItemsBoundsOnResize(), false, "");

    const unsigned int width = (static_cast<unsigned int>(cc->Layout.Width)) /
                               static_cast<unsigned int>(std::max<>(cc->columns.size(), size_t(1U)));

    unsigned int xPreviousColumn       = 0;
    unsigned int widthOfPreviousColumn = 0;
    for (auto i = 0U; i < cc->columns.size(); i++)
    {
        auto& col  = cc->columns[i];
        col.height = static_cast<unsigned int>(cc->Layout.Height - 2);
        col.x      = static_cast<unsigned int>(xPreviousColumn + widthOfPreviousColumn + BorderOffset);
        if (col.customWidth == false)
        {
            col.width = std::max<>(width, MinColumnWidth);
        }
        xPreviousColumn       = col.x;
        widthOfPreviousColumn = col.width;
    }

    auto& lastColumn            = cc->columns[cc->columns.size() - 1];
    const auto rightLastColumnX = static_cast<int>(lastColumn.x + lastColumn.width);
    if (rightLastColumnX < cc->Layout.Width)
    {
        lastColumn.width += cc->Layout.Width - rightLastColumnX;
    }
    else if (rightLastColumnX > cc->Layout.Width)
    {
        lastColumn.width -= rightLastColumnX - cc->Layout.Width;
    }

    if (cc->Layout.Width <= TreeScrollbarLeftOffset)
    {
        if (cc->hidSearchBarOnResize == false)
        {
            if ((cc->treeFlags & TreeFlags::HideSearchBar) != TreeFlags::None)
            {
                cc->filter.mode = TreeControlContext::FilterMode::None;
                cc->filter.searchText.Clear();
            }
            cc->hidSearchBarOnResize = true;
        }
    }
    else
    {
        if (cc->hidSearchBarOnResize)
        {
            cc->treeFlags ^= static_cast<unsigned int>(TreeFlags::HideSearchBar);
            cc->hidSearchBarOnResize = false;
        }

        if ((cc->treeFlags & TreeFlags::FilterSearch) != TreeFlags::None)
        {
            cc->filter.mode = TreeControlContext::FilterMode::Filter;
        }
        else if ((cc->treeFlags & TreeFlags::Searchable) != TreeFlags::None)
        {
            cc->filter.mode = TreeControlContext::FilterMode::Search;
        }
    }

    return true;
}

bool Tree::AdjustItemsBoundsOnResize()
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    cc->maxItemsToDraw  = cc->Layout.Height - 1 - 1 - 1; // 0 - border top | 1 - column header | 2 - border bottom
    cc->offsetBotToDraw = cc->offsetTopToDraw + cc->maxItemsToDraw;

    return true;
}

bool Tree::AddToColumnWidth(const unsigned int columnIndex, const int value)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    auto& column          = cc->columns[columnIndex];
    unsigned int currentX = column.x + column.width;
    if (currentX == column.x)
    {
        return true;
    }

    auto diff = std::abs(value);

    auto& rightColumn = cc->columns[columnIndex + 1ULL];

    const bool moveLeft = value < 0;
    if (moveLeft)
    {
        if (static_cast<int>(column.width - diff) < MinColumnWidth)
        {
            diff = column.width - MinColumnWidth;
        }

        column.width -= diff;
        rightColumn.width += diff;
        rightColumn.x -= diff;
    }
    else
    {
        if (static_cast<int>(rightColumn.width - diff) < MinColumnWidth)
        {
            diff = rightColumn.width - MinColumnWidth;
        }

        column.width += diff;
        rightColumn.width -= diff;
        rightColumn.x += diff;
    }

    return true;
}

bool Tree::SetColorForItems(const ColorPair& color)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    for (auto& item : cc->items)
    {
        for (auto& value : item.second.values)
        {
            value.SetColor(color);
        }
    }

    return true;
}

bool Tree::SearchItems()
{
    bool found = false;

    MarkAllItemsAsNotFound();

    CHECK(Context != nullptr, found, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    std::set<ItemHandle> toBeExpanded;
    if (cc->filter.searchText.Len() > 0)
    {
        for (auto& item : cc->items)
        {
            for (auto& value : item.second.values)
            {
                if (const auto index = value.Find(cc->filter.searchText.ToStringView(), true) >= 0)
                {
                    item.second.markedAsFound = true;
                    if (cc->filter.mode == TreeControlContext::FilterMode::Filter)
                    {
                        MarkAllAncestorsWithChildFoundInFilterSearch(item.second.handle);
                    }
                    if (found == false)
                    {
                        cc->currentSelectedItemHandle = item.second.handle;
                        SetColorForItems(cc->Cfg->Tree.Text.SearchActive);
                    }
                    found = true;
                    value.SetColor(index, index + cc->filter.searchText.Len(), cc->Cfg->Tree.Text.Filter);

                    ItemHandle ancestorHandle = item.second.parent;
                    do
                    {
                        if (const auto& it = cc->items.find(ancestorHandle); it != cc->items.end())
                        {
                            const auto& ancestor = it->second;
                            if (ancestor.isExpandable && ancestor.expanded == false &&
                                (cc->treeFlags & TreeFlags::DynamicallyPopulateNodeChildren) == TreeFlags::None)
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

    for (const auto handle : toBeExpanded)
    {
        ToggleItem(handle);
    }

    if (toBeExpanded.size() > 0 || cc->filter.mode == TreeControlContext::FilterMode::Filter)
    {
        ProcessItemsToBeDrawn(InvalidItemHandle);
    }

    ProcessOrderedItems(InvalidItemHandle, true);

    return found;
}

bool Tree::ProcessOrderedItems(const ItemHandle handle, const bool clear)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    if (clear)
    {
        cc->orderedItems.clear();
        cc->orderedItems.reserve(cc->items.size());
    }

    CHECK(cc->items.size() > 0, true, "");

    if (handle == InvalidItemHandle)
    {
        for (const auto& rootHandle : cc->roots)
        {
            cc->orderedItems.emplace_back(rootHandle);

            const auto& root = cc->items[rootHandle];
            for (auto& childHandle : root.children)
            {
                const auto& child = cc->items[childHandle];
                ProcessOrderedItems(childHandle, false);
            }
        }
    }
    else
    {
        const auto& item = cc->items[handle];
        cc->orderedItems.emplace_back(item.handle);
        for (auto& childHandle : item.children)
        {
            ProcessOrderedItems(childHandle, false);
        }
    }

    return true;
}

bool Tree::MarkAllItemsAsNotFound()
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    for (auto& [handle, item] : cc->items)
    {
        item.markedAsFound = false;

        if (cc->filter.mode == TreeControlContext::FilterMode::Filter)
        {
            item.hasAChildThatIsMarkedAsFound = false;
        }
    }

    return true;
}

bool Tree::MarkAllAncestorsWithChildFoundInFilterSearch(const ItemHandle handle)
{
    CHECK(handle != InvalidItemHandle, false, "");
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    const auto& item          = cc->items[handle];
    ItemHandle ancestorHandle = item.parent;
    do
    {
        if (const auto& it = cc->items.find(ancestorHandle); it != cc->items.end())
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

const Utils::UnicodeStringBuilder& Tree::GetItemMetadata(ItemHandle handle)
{
    CHECK(Context != nullptr, cb, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);
    if (cc->items.find(handle) == cc->items.end())
    {
        return cb;
    }

    auto& item = cc->items.at(handle);
    return item.metadata;
}

bool Tree::SetItemMetadata(ItemHandle handle, const Utils::ConstString& metadata)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);
    if (cc->items.find(handle) != cc->items.end())
    {
        return false;
    }

    auto& item = cc->items.at(handle);
    item.metadata.Set(metadata);

    return true;
}

} // namespace Controls
