#include "ControlContext.hpp"

#include <queue>

namespace AppCUI::Controls
{
bool Tree::Create(Control* parent, const std::string_view& layout)
{
    Context = new TreeControlContext();
    CHECK(Context != nullptr, false, "");

    const auto cc        = reinterpret_cast<TreeControlContext*>(Context);
    cc->Layout.MinHeight = 1;
    cc->Layout.MaxHeight = 200000;
    cc->Layout.MinWidth  = 4;
    CHECK(Init(parent, "", layout, true), false, "Failed to create tree!");
    cc->Flags                 = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | GATTR_HSCROLL | GATTR_VSCROLL;
    cc->ScrollBars.LeftMargin = 25;

    cc->maxItemsToDraw  = this->GetHeight() - 2;
    cc->offsetTopToDraw = 0;
    cc->offsetBotToDraw = cc->maxItemsToDraw;

    return true;
}

bool Tree::RecursiveItemPainting(
      Graphics::Renderer& renderer, const ItemHandle ih, WriteTextParams& wtp, const unsigned int offset) const
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    wtp.X += offset;

    for (auto i = 0U; i < cc->itemsToDrew.size(); i++)
    {
        auto& item = cc->items[cc->itemsToDrew[i]];

        if (item.parent != ih)
        {
            continue;
        }

        CHECK(cc->itemsDrew.size() < cc->maxItemsToDraw, false, "Performance improvement - don't draw hidden objects.");

        item.startOffset = wtp.X;
        if (i >= cc->offsetTopToDraw && i <= cc->offsetBotToDraw)
        {
            if (item.children.size() > 0)
            {
                if (item.expanded)
                {
                    wtp.Color = cc->colorTriangleDown;
                    renderer.WriteSpecialCharacter(wtp.X, wtp.Y, SpecialChars::TriangleDown, wtp.Color);
                }
                else
                {
                    wtp.Color = cc->colorTriangleRight;
                    renderer.WriteSpecialCharacter(wtp.X, wtp.Y, SpecialChars::TriangleRight, wtp.Color);
                }
            }
            else
            {
                wtp.Color = cc->colorCircleFiled;
                renderer.WriteSpecialCharacter(wtp.X, wtp.Y, SpecialChars::CircleFilled, wtp.Color);
            }

            wtp.X += cc->offset;

            if (item.handle == cc->currentSelectedItemHandle)
            {
                wtp.Color = cc->colorTextSelected;
            }
            else
            {
                wtp.Color = cc->colorText;
            }

            renderer.WriteText(item.value, wtp);

            wtp.X -= cc->offset;

            wtp.Y++;

            cc->itemsDrew.emplace_back(item.handle);
        }

        if (item.children.size() > 0)
        {
            if (item.expanded)
            {
                CHECK(RecursiveItemPainting(renderer, item.handle, wtp, cc->offset), false, "");
            }
        }
    }

    wtp.X -= offset;

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

    for (auto& it : cc->items)
    {
        auto& item = it.second;
        if (item.parent != handle)
        {
            continue;
        }

        cc->itemsToDrew.emplace_back(item.handle);

        if (item.children.size() > 0)
        {
            if (item.expanded)
            {
                ProcessItemsToBeDrawn(item.handle, false);
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

bool Tree::ToggleExpandRecursive(const ItemHandle handle) const
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    std::queue<ItemHandle> ancestorRelated;
    ancestorRelated.push(handle);

    while (ancestorRelated.empty() == false)
    {
        ItemHandle current = ancestorRelated.front();
        ancestorRelated.pop();

        auto& item    = cc->items[current];
        item.expanded = !item.expanded;

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

    renderer.DrawRectSize(0, 0, cc->Layout.Width, cc->Layout.Height, cc->colorText, false);

    const auto flags = WriteTextFlags::LeftMargin | WriteTextFlags::SingleLine | WriteTextFlags::FitTextToWidth |
                       WriteTextFlags::OverwriteColors;
    WriteTextParams wtp{ flags };
    wtp.X     = 1; // 0  is for border
    wtp.Y     = 1; // 0  is for border
    wtp.Width = cc->Layout.Width;

    cc->itemsDrew.clear();
    if (cc->notProcessed)
    {
        ProcessItemsToBeDrawn(InvalidItemHandle);
        cc->notProcessed = false;
    }
    RecursiveItemPainting(renderer, InvalidItemHandle, wtp, 0);
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

            cc->offsetTopToDraw += difference;
            cc->offsetBotToDraw += difference;
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
        cc->offsetTopToDraw           = cc->itemsToDrew.size() - cc->maxItemsToDraw;
        cc->offsetBotToDraw           = cc->offsetTopToDraw + cc->maxItemsToDraw;
        cc->currentSelectedItemHandle = cc->itemsToDrew[cc->itemsToDrew.size() - 1];
        return true;

    case Key::Ctrl | Key::Space:
        if (cc->itemsDrew.size() > 0)
        {
            ToggleExpandRecursive(cc->currentSelectedItemHandle);
            return true;
        }
    case Key::Space:
        if (cc->itemsDrew.size() > 0)
        {
            auto& item = cc->items[cc->currentSelectedItemHandle];
            if (item.children.size() > 0)
            {
                item.expanded = !item.expanded;
                ProcessItemsToBeDrawn(InvalidItemHandle);
            }
        }
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

    if (cc->itemsDrew.size() > 0)
    {
        if (cc->currentSelectedItemHandle == InvalidItemHandle)
        {
            cc->currentSelectedItemHandle = cc->itemsDrew[0];
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
        if (y > 0 && y < this->GetHeight())
        {
            const unsigned int index = y - 1;
            if (index >= cc->itemsDrew.size())
            {
                break;
            }

            const auto itemHandle = cc->itemsDrew[index];
            const auto it         = cc->items.find(itemHandle);

            if (x > static_cast<int>(it->second.startOffset + cc->offset) && x < this->GetWidth())
            {
                cc->currentSelectedItemHandle = itemHandle;
                break;
            }

            if (it->second.children.size() > 0)
            {
                if (x >= static_cast<int>(it->second.startOffset) &&
                    x < static_cast<int>(it->second.startOffset + cc->offset))
                {
                    it->second.expanded = !it->second.expanded;
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

ItemHandle Tree::AddItem(const ItemHandle parent, const std::u16string_view& value, void* data, bool process)
{
    CHECK(Context != nullptr, InvalidItemHandle, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    TreeItem ti{ parent, GetHandleForNewItem(), value.data() };
    ti.data              = ItemData(data);
    cc->items[ti.handle] = ti;

    if (cc->items.size() == 1)
    {
        cc->currentSelectedItemHandle = ti.handle;
    }

    if (parent != InvalidItemHandle)
    {
        cc->items[parent].children.emplace_back(ti.handle);
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

    // delete element
    const auto it = cc->items.find(handle);
    if (it != cc->items.end())
    {
        // delete children
        for (const auto& it : it->second.children)
        {
            cc->items.erase(it);
        }

        // delete self as child
        auto& children   = cc->items[it->second.parent].children;
        const auto child = std::find(children.begin(), children.end(), handle);
        if (child != children.end())
        {
            children.erase(child);
        }

        // delete self
        cc->items.erase(it);
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

ItemHandle Tree::GetHandleForNewItem() const
{
    CHECK(Context != nullptr, InvalidItemHandle, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    const auto current = cc->nextItemHandle;
    cc->nextItemHandle++;
    return current;
}

} // namespace AppCUI::Controls
