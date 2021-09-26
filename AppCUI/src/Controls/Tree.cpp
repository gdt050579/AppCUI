#include "ControlContext.hpp"

#include <queue>

namespace AppCUI::Controls
{
bool Tree::Create(
      Control* parent, const std::string_view& layout, const TreeFlags flags, const std::vector<std::u16string> columns)
{
    Context = new TreeControlContext();
    CHECK(Context != nullptr, false, "");

    const auto cc        = reinterpret_cast<TreeControlContext*>(Context);
    cc->Layout.MinHeight = 1;
    cc->Layout.MaxHeight = 200000;
    cc->Layout.MinWidth  = 4;
    CHECK(Init(parent, "", layout, true), false, "Failed to create tree!");
    cc->Flags                 = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | GATTR_HSCROLL | GATTR_VSCROLL;
    cc->ScrollBars.LeftMargin = 25; // search field

    cc->maxItemsToDraw  = this->GetHeight() - 1 - 1 - 1; // 0 - border top | 1 - column header | 2 - border bottom
    cc->offsetTopToDraw = 0;
    cc->offsetBotToDraw = cc->maxItemsToDraw;

    cc->columns.headerValues = columns;

    if (columns.size() == 0)
    {
        cc->columns.itemWidth   = cc->Layout.Width;
        cc->columns.columnWidth = 0;
    }
    else
    {
        cc->columns.itemWidth   = cc->Layout.Width / 2 - 1;
        cc->columns.columnWidth = (static_cast<unsigned int>(cc->Layout.Width) - cc->columns.itemWidth) /
                                  static_cast<unsigned int>(cc->columns.headerValues.size());
    }

    return true;
}

bool Tree::ItemsPainting(Graphics::Renderer& renderer, const ItemHandle ih) const
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    WriteTextParams wtp{ WriteTextFlags::SingleLine | WriteTextFlags::OverwriteColors | WriteTextFlags::ClipToWidth };
    wtp.Y     = 2; // 0  is for border | 1 is for header
    wtp.Align = TextAlignament::Left;

    for (auto i = cc->offsetTopToDraw; i < std::min<size_t>(cc->offsetBotToDraw, cc->itemsToDrew.size()); i++)
    {
        auto& item = cc->items[cc->itemsToDrew[i]];

        wtp.X     = item.depth * cc->offset;
        wtp.Width = cc->columns.itemWidth - item.depth * cc->offset + cc->offset - 3;

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

        renderer.WriteText(item.value, wtp);

        wtp.Y++;
    }

    return true;
}

bool Tree::PaintColumnHeaders(Graphics::Renderer& renderer)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);
    CHECK(cc->columns.headerValues.size() > 0, true, "");

    WriteTextParams wtp{ WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth };
    wtp.Y     = 1; // 0  is for border
    wtp.Align = TextAlignament::Left;
    wtp.Color = cc->Cfg->Tree.Column.Text;

    renderer.FillHorizontalLine(1, 1, cc->Layout.Width - 2, ' ', cc->Cfg->Tree.Column.Header);
    renderer.WriteSingleLineText(1, 1, "content", wtp.Color);

    for (auto i = 0; i < cc->columns.headerValues.size(); i++)
    {
        wtp.Width = cc->columns.columnWidth - 1 - 1; // left vertical line | right vertical line
        wtp.X     = static_cast<int>((cc->columns.itemWidth + 1) + i * cc->columns.columnWidth) + 1;
        renderer.WriteText(cc->columns.headerValues[i], wtp);
    }

    return false;
}

bool Tree::PaintColumnSeparators(Graphics::Renderer& renderer)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);
    CHECK(cc->columns.headerValues.size() > 0, true, "");

    for (auto i = 0; i < cc->columns.headerValues.size(); i++)
    {
        const auto x = static_cast<int>((cc->columns.itemWidth + 1) + i * cc->columns.columnWidth);
        renderer.DrawVerticalLine(x, 1, cc->Layout.Height - 2, cc->Cfg->Tree.Column.Separator);
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

    renderer.DrawRectSize(0, 0, cc->Layout.Width, cc->Layout.Height, cc->Cfg->Tree.Border, false);
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
        if (y > 1 && y < this->GetHeight())
        {
            const unsigned int index = y - 2;
            if (index >= cc->offsetBotToDraw || index >= cc->itemsToDrew.size())
            {
                break;
            }

            const auto itemHandle = cc->itemsToDrew[static_cast<size_t>(cc->offsetTopToDraw) + index];
            const auto it         = cc->items.find(itemHandle);

            if (x > static_cast<int>(it->second.depth * cc->offset + cc->offset) && x < this->GetWidth())
            {
                cc->currentSelectedItemHandle = itemHandle;
                break;
            }

            if (x >= static_cast<int>(it->second.depth * cc->offset) &&
                x < static_cast<int>(it->second.depth * cc->offset + cc->offset))
            {
                ToggleItem(it->second.handle);
                if (it->second.expanded == false)
                {
                    if (IsAncestorOfChild(it->second.handle, cc->currentSelectedItemHandle))
                    {
                        cc->currentSelectedItemHandle = it->second.handle;
                    }
                }
                ProcessItemsToBeDrawn(InvalidItemHandle);
                break;
            }
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

ItemHandle Tree::AddItem(
      const ItemHandle parent,
      const std::u16string_view& value,
      void* data,
      bool process,
      std::u16string metadata,
      bool isExpandable)
{
    CHECK(Context != nullptr, InvalidItemHandle, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    TreeItem ti{ parent, GetHandleForNewItem(), value.data() };
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
        return it->second.value;
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
    cc->callback  = callback;
}

bool Tree::ToggleItem(const ItemHandle handle)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    auto& item = cc->items[handle];
    CHECK(item.isExpandable, true, "");

    for (const auto& child : item.children)
    {
        RemoveItem(child);
    }
    item.children.clear();

    item.expanded = !item.expanded;

    if (item.expanded)
    {
        if (cc->callback)
        {
            CHECK(cc->callback(*this, handle, &item.metadata), false, "");
        }
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
