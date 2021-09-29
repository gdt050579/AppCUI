#include "ControlContext.hpp"

#include <queue>

namespace AppCUI::Controls
{
bool Tree::Create(
      Control* parent, const std::string_view& layout, const unsigned int flags, const unsigned int noOfColumns)
{
    CHECK(noOfColumns > 0, false, "");

    Context = new TreeControlContext();
    CHECK(Context != nullptr, false, "");

    const auto cc        = reinterpret_cast<TreeControlContext*>(Context);
    cc->Layout.MinHeight = 1;
    cc->Layout.MaxHeight = 200000;
    cc->Layout.MinWidth  = 4;
    CHECK(Init(parent, "", layout, true), false, "Failed to create tree!");

    cc->treeFlags = flags;

    cc->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;

    cc->width  = cc->Layout.Width % 2 == 0 ? cc->Layout.Width : cc->Layout.Width - 1;
    cc->height = cc->Layout.Height % 2 == 0 ? cc->Layout.Height - 2 : cc->Layout.Height - 1 - 2;

    if (cc->treeFlags & static_cast<unsigned int>(TreeFlags::HideScrollBar))
    {
        // no scrollbar
    }
    else
    {
        cc->Flags |= GATTR_HSCROLL | GATTR_VSCROLL;
        cc->ScrollBars.LeftMargin = 25; // search field
    }

    cc->maxItemsToDraw  = cc->height - 1 - 1 - 1; // 0 - border top | 1 - column header | 2 - border bottom
    cc->offsetTopToDraw = 0;
    cc->offsetBotToDraw = cc->maxItemsToDraw;

    const unsigned int offsetLeft   = 1; // border
    const unsigned int offsetRight  = 1; // border
    const unsigned int offsetTop    = 1; // border
    const unsigned int offsetBottom = 1; // border
    const unsigned int width        = (static_cast<unsigned int>(cc->width) / noOfColumns);
    for (auto i = 0U; i < noOfColumns; i++)
    {
        ColumnData cd{ static_cast<unsigned int>(cc->columns.size() * width + offsetLeft),
                       width,
                       static_cast<unsigned int>(cc->height - 2),
                       u"",
                       TextAlignament::Center,
                       TextAlignament::Left };
        cc->columns.emplace_back(cd);
    }

    return true;
}

bool Tree::ItemsPainting(Graphics::Renderer& renderer, const ItemHandle ih) const
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    WriteTextParams wtp{ WriteTextFlags::SingleLine | WriteTextFlags::OverwriteColors | WriteTextFlags::ClipToWidth };
    wtp.Y = 2; // 0  is for border | 1 is for header

    for (auto i = cc->offsetTopToDraw; i < std::min<size_t>(cc->offsetBotToDraw, cc->itemsToDrew.size()); i++)
    {
        auto& item = cc->items[cc->itemsToDrew[i]];

        unsigned int j = 0; // column index
        for (const auto& col : cc->columns)
        {
            wtp.Align = col.contentAlignment;
            if (j == 0)
            {
                wtp.X     = col.x + item.depth * cc->offset - 1;
                wtp.Width = col.width - item.depth * cc->offset + cc->offset - 3;

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

                wtp.X += cc->offset;

                if (item.handle == cc->currentSelectedItemHandle)
                {
                    wtp.Color = cc->Cfg->Tree.Text.Focused;
                }
                else
                {
                    wtp.Color = cc->Cfg->Tree.Text.Normal;
                }
            }
            else
            {
                wtp.X = col.x + 1;
                if (j == cc->columns.size() - 1)
                {
                    wtp.Width = col.width - 3;
                }
                else
                {
                    wtp.Width = col.width - 1;
                }
                wtp.Color = cc->Cfg->Tree.Text.Normal;
            }

            if (j < item.values.size())
            {
                renderer.WriteText(item.values[j], wtp);
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
        const auto rightX       = std::min<>(lastColumn.x + lastColumn.width, cc->width - 2);
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

        renderer.WriteText(col.headerValue, wtp);
        i++;
    }

    return true;
}

bool Tree::PaintColumnSeparators(Graphics::Renderer& renderer)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);
    CHECK(cc->columns.size() > 0, true, "");

    for (const auto& col : cc->columns)
    {
        renderer.DrawVerticalLine(col.x, 1, cc->height - 2, cc->Cfg->Tree.Column.Separator);
    }

    const auto& lastColumn = cc->columns[cc->columns.size() - 1];
    renderer.DrawVerticalLine(lastColumn.x + lastColumn.width, 1, cc->height - 2, cc->Cfg->Tree.Column.Separator);

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
    }

    CHECK(cc->items.size() > 0, true, "");

    if (handle == InvalidItemHandle)
    {
        for (const auto& handle : cc->roots)
        {
            cc->itemsToDrew.emplace_back(handle);

            const auto& item = cc->items[handle];
            if (item.isExpandable == false || item.expanded == false)
            {
                continue;
            }

            for (auto& it : item.children)
            {
                const auto& child = cc->items[it];
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
        cc->itemsToDrew.emplace_back(item.handle);
        CHECK(item.isExpandable, true, "");

        for (auto& it : item.children)
        {
            const auto& child = cc->items[it];
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

    renderer.DrawRectSize(0, 0, cc->width, cc->height, cc->Cfg->Tree.Border, false);
    PaintColumnHeaders(renderer);
    PaintColumnSeparators(renderer);

    if (cc->notProcessed)
    {
        ProcessItemsToBeDrawn(InvalidItemHandle);
        cc->notProcessed = false;
    }

    ItemsPainting(renderer, InvalidItemHandle);
}

bool Tree::OnKeyEvent(AppCUI::Input::Key keyCode, char16_t)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    switch (keyCode)
    {
    case Key::Up:
        return MoveUp();
    case Key::Down:
        return MoveDown();
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
        return true;
    case Key::Space:
        ToggleItem(cc->currentSelectedItemHandle);
        ProcessItemsToBeDrawn(InvalidItemHandle);
        return true;
    default:
        break;
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

void Tree::OnMousePressed(int x, int y, AppCUI::Input::MouseButton button)
{
    CHECKRET(Context != nullptr, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    switch (button)
    {
    case AppCUI::Input::MouseButton::None:
        break;
    case AppCUI::Input::MouseButton::Left:
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

            if (x > static_cast<int>(it->second.depth * cc->offset + cc->offset) && x < static_cast<int>(cc->width))
            {
                cc->currentSelectedItemHandle = itemHandle;
            }
        }
        break;
        default:
            break;
        }
        break;
    case AppCUI::Input::MouseButton::Center:
        break;
    case AppCUI::Input::MouseButton::Right:
        break;
    case AppCUI::Input::MouseButton::DoubleClicked:
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

bool Tree::OnMouseWheel(int x, int y, AppCUI::Input::MouseWheel direction)
{
    switch (direction)
    {
    case AppCUI::Input::MouseWheel::None:
        break;
    case AppCUI::Input::MouseWheel::Up:
        return MoveUp();
    case AppCUI::Input::MouseWheel::Down:
        return MoveDown();
    case AppCUI::Input::MouseWheel::Left:
        break;
    case AppCUI::Input::MouseWheel::Right:
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

    // cc->ScrollBars.LeftMargin = 1;
    // UpdateHScrollBar(index, static_cast<unsigned long long>(this->GetHeight()) - 2);
    const unsigned long long count = std::max<size_t>(cc->itemsToDrew.size() - 1, 0);
    UpdateVScrollBar(index, count);
}

void Tree::OnAfterResize(int newWidth, int newHeight)
{
    CHECKRET(AdjustElementsOnResize(newWidth, newHeight), "");
}

ItemHandle Tree::AddItem(
      const ItemHandle parent,
      const std::vector<std::u16string_view> values,
      void* data,
      bool process,
      std::u16string metadata,
      bool isExpandable)
{
    CHECK(values.size() > 0, InvalidItemHandle, "");

    CHECK(Context != nullptr, InvalidItemHandle, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    std::vector<std::u16string> itemValues;
    for (const auto& value : values)
    {
        itemValues.push_back(value.data());
    }
    TreeItem ti{ parent, GetHandleForNewItem(), itemValues };
    ti.data         = ItemData(data);
    ti.isExpandable = isExpandable;
    ti.metadata     = metadata;

    if (parent == InvalidItemHandle)
    {
        cc->roots.emplace_back(ti.handle);
    }
    else
    {
        auto& parentItem = cc->items[parent];
        ti.depth         = parentItem.depth + 1;
        parentItem.children.emplace_back(ti.handle);
    }

    cc->items[ti.handle] = ti;

    if (cc->items.size() == 1)
    {
        cc->currentSelectedItemHandle = ti.handle;
    }

    if (process)
    {
        ProcessItemsToBeDrawn(InvalidItemHandle);
    }
    else
    {
        cc->notProcessed = true;
    }

    return ti.handle;
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

        const auto it = cc->items.find(handle);

        if (it != cc->items.end())
        {
            for (const auto& handle : cc->items[current].children)
            {
                ancestorRelated.push(handle);
            }
            cc->items.erase(it);

            const auto rootIt = std::find(cc->roots.begin(), cc->roots.end(), handle);
            if (rootIt != cc->roots.end())
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

const std::u16string_view Tree::GetItemText(const ItemHandle handle)
{
    CHECK(Context != nullptr, u"", "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    const auto it = cc->items.find(handle);
    if (it != cc->items.end())
    {
        return it->second.values[0];
    }

    return u"";
}

ItemData* Tree::GetItemData(const ItemHandle handle)
{
    CHECK(Context != nullptr, nullptr, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    const auto it = cc->items.find(handle);
    if (it != cc->items.end())
    {
        return &it->second.data;
    }

    return nullptr;
}

ItemData* Tree::GetItemData(const size_t index)
{
    CHECK(Context != nullptr, nullptr, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    auto it = cc->items.begin();
    std::advance(it, index);
    return &it->second.data;
}

size_t Tree::GetItemsCount()
{
    CHECK(Context != nullptr, 0, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);
    return cc->items.size();
}

void Tree::SetToggleItemHandle(
      const std::function<bool(Tree& tree, const ItemHandle handle, const void* context)> callback)
{
    CHECKRET(Context != nullptr, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);
    if (cc->treeFlags & static_cast<unsigned int>(TreeFlags::DynamicallyPopulateNodeChildren))
    {
        CHECKRET(callback != nullptr, "");
        cc->callback = callback;
    }
}

bool Tree::AddColumnData(
      const unsigned int index,
      std::u16string_view title,
      const TextAlignament headerAlignment,
      const TextAlignament contentAlignment,
      const unsigned int width)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    CHECK(index < cc->columns.size(), false, "");

    auto& column = cc->columns[index];

    column.headerValue      = title;
    column.headerAlignment  = headerAlignment;
    column.contentAlignment = contentAlignment;
    if (width != 0xFFFFFFFF)
    {
        column.width       = width;
        column.customWidth = true;

        // shifts columns
        unsigned int currentX = column.x + column.width + 1;
        for (auto i = index + 1; i < cc->columns.size(); i++)
        {
            auto& col       = cc->columns[i];
            col.customWidth = true;
            if (currentX < col.x)
            {
                const auto diff = col.x - currentX;
                col.x -= diff;
                col.width -= diff;
            }
            else
            {
                const auto diff = currentX - col.x;
                col.x += diff;
                col.width += diff;
            }
            currentX = col.x + col.width + 1;
        }
    }

    return true;
}

bool Tree::ToggleItem(const ItemHandle handle)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    auto& item = cc->items[handle];
    CHECK(item.isExpandable, true, "");

    if (cc->treeFlags & static_cast<unsigned int>(TreeFlags::DynamicallyPopulateNodeChildren))
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
        if (cc->treeFlags & static_cast<unsigned int>(TreeFlags::DynamicallyPopulateNodeChildren))
        {
            if (cc->callback)
            {
                CHECK(cc->callback(*this, handle, &item.metadata), false, "");
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

    if (x > static_cast<int>(it->second.depth * cc->offset + cc->offset) && x < static_cast<int>(cc->width))
    {
        return false; // on item
    }

    if (x >= static_cast<int>(it->second.depth * cc->offset) &&
        x < static_cast<int>(it->second.depth * cc->offset + cc->offset - 1U))
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

    return (x > static_cast<int>(it->second.depth * cc->offset + cc->offset) && x < static_cast<int>(cc->width));
}

bool Tree::IsMouseOnBorder(int x, int y) const
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    return (x == 0 || x == cc->width - 1) || (y == 0 || y == cc->width - 1);
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
    // TODO:
    return false;
}

bool Tree::AdjustElementsOnResize(const int newWidth, const int newHeight)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    cc->width  = cc->Layout.Width % 2 == 0 ? cc->Layout.Width : cc->Layout.Width - 1;
    cc->height = cc->height = cc->Layout.Height % 2 == 0 ? cc->Layout.Height - 2 : cc->Layout.Height - 1 - 2;

    cc->maxItemsToDraw  = cc->height - 1 - 1 - 1; // 0 - border top | 1 - column header | 2 - border bottom
    cc->offsetTopToDraw = 0;
    cc->offsetBotToDraw = cc->maxItemsToDraw;

    const unsigned int offsetLeft   = 1; // border
    const unsigned int offsetRight  = 1; // border
    const unsigned int offsetTop    = 1; // border
    const unsigned int offsetBottom = 1; // border
    const unsigned int width        = (static_cast<unsigned int>(cc->width)) /
                               static_cast<unsigned int>(std::max<>(cc->columns.size(), size_t(1U)));

    unsigned int i                     = 0;
    unsigned int xPreviousColumn       = 0;
    unsigned int widthOfPreviousColumn = 0;
    for (auto& col : cc->columns)
    {
        col.height = static_cast<unsigned int>(cc->height - 2);
        col.x      = static_cast<unsigned int>(xPreviousColumn + widthOfPreviousColumn + offsetLeft);
        if (col.customWidth == false)
        {
            col.width = width;
        }
        xPreviousColumn       = col.x;
        widthOfPreviousColumn = col.width;
        i++;
    }

    return true;
}

ItemHandle Tree::GetHandleForNewItem() const
{
    CHECK(Context != nullptr, InvalidItemHandle, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    const auto current = cc->nextItemHandle;
    cc->nextItemHandle++;
    return current;
}

} // namespace AppCUI::Controls
