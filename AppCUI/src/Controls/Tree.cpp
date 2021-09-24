#include "ControlContext.hpp"

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
    cc->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;

    cc->maxItemsToDraw = this->GetHeight() - 2;

    return true;
}

bool Tree::RecursiveItemPainting(
      Graphics::Renderer& renderer, const ItemHandle ih, WriteTextParams& wtp, const unsigned int offset) const
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    wtp.X += offset;

    for (const auto& it : cc->items)
    {
        auto& item = it.second;

        if (item.parent != ih)
        {
            continue;
        }

        CHECK(cc->itemsDrew.size() < cc->maxItemsToDraw, false, "Performance improvement - don't draw hidden objects.");

        if (IsExpandable(item.handle))
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

        if (IsExpandable(item.handle))
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
    RecursiveItemPainting(renderer, InvalidItemHandle, wtp, 0);
}

bool Tree::OnKeyEvent(AppCUI::Input::Key keyCode, char16_t)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    switch (keyCode)
    {
    case Key::Up:
        if (cc->itemsDrew.size() > 0)
        {
            if (cc->itemsDrew[0] == cc->currentSelectedItemHandle)
            {
                cc->currentSelectedItemHandle = cc->itemsDrew[cc->itemsDrew.size() - 1];
            }
            else
            {
                const auto it         = find(cc->itemsDrew.begin(), cc->itemsDrew.end(), cc->currentSelectedItemHandle);
                const long long index = it - cc->itemsDrew.begin();
                cc->currentSelectedItemHandle = cc->itemsDrew[index - 1];
            }

            return true;
        }
        break;
    case Key::Down:
        if (cc->itemsDrew.size() > 0)
        {
            if (cc->itemsDrew[cc->itemsDrew.size() - 1] == cc->currentSelectedItemHandle)
            {
                cc->currentSelectedItemHandle = cc->itemsDrew[0];
            }
            else
            {
                const auto it         = find(cc->itemsDrew.begin(), cc->itemsDrew.end(), cc->currentSelectedItemHandle);
                const long long index = it - cc->itemsDrew.begin();
                cc->currentSelectedItemHandle = cc->itemsDrew[index + 1];
            }

            return true;
        }
        break;
    case Key::Space:
        if (cc->itemsDrew.size() > 0)
        {
            cc->items[cc->currentSelectedItemHandle].expanded = !cc->items[cc->currentSelectedItemHandle].expanded;
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

ItemHandle Tree::AddItem(const ItemHandle parent, const std::u16string_view& value, void* data)
{
    CHECK(Context != nullptr, InvalidItemHandle, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    TreeItem ti{ parent, GetHandleForNewItem(), value.data() };
    ti.data = ItemData(data);
    cc->items[ti.handle] = ti;

    if (cc->items.size() == 1)
    {
        cc->currentSelectedItemHandle = ti.handle;
    }

    return ti.handle;
}

bool Tree::RemoveItem(const ItemHandle handle)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    // delete element
    const auto it = cc->items.find(handle);
    if (it != cc->items.end())
    {
        cc->items.erase(it);
    }

    // delete children
    for (auto it = cc->items.cbegin(); it != cc->items.cend();)
    {
        // delete child in items
        if (it->second.parent == handle)
        {
            it = cc->items.erase(it++);
        }
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

bool Tree::IsExpandable(const ItemHandle handle) const
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    for (const auto& item : cc->items)
    {
        if (item.second.parent == handle)
        {
            return true;
        }
    }

    return false;
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
