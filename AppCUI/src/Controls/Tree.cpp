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
        const auto& item = it.second;

        if (item.parent != ih)
        {
            continue;
        }

        if (IsExpandable(item.handle))
        {
            if (cc->view[item.handle].expanded)
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

        wtp.X += 2;
        wtp.Color = cc->colorText;

        renderer.WriteText(item.value, wtp);

        wtp.X -= cc->offset;

        wtp.Y++;

        if (IsExpandable(item.handle))
        {
            if (cc->view[item.handle].expanded)
            {
                CHECK(RecursiveItemPainting(renderer, item.handle, wtp, cc->offset), false, "");
            }
        }
    }

    wtp.X -= offset;

    CHECK(static_cast<unsigned int>(wtp.Y) < this->GetHeight(), false, "Performance improvement - don't draw hidden objects.");

    return true;
}

void Tree::Paint(Graphics::Renderer& renderer)
{
    CHECKRET(Context != nullptr, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    const auto flags = WriteTextFlags::LeftMargin | WriteTextFlags::SingleLine | WriteTextFlags::FitTextToWidth |
                       WriteTextFlags::OverwriteColors;
    WriteTextParams wtp{ flags };
    wtp.X     = 0;
    wtp.Y     = 0;
    wtp.Width = cc->Layout.Width;

    RecursiveItemPainting(renderer, InvalidItemHandle, wtp, 0);
}

bool Tree::OnKeyEvent(AppCUI::Input::Key keyCode, char16_t)
{
    return false;
}

const ItemHandle Tree::AddItem(const ItemHandle parent, const std::string_view& value)
{
    CHECK(Context != nullptr, InvalidItemHandle, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    const TreeItem ti{ parent, GetHandleForNewItem(), value.data() };
    cc->items[ti.handle] = ti;

    const TreeItemView tiv{ ti.handle, true };
    cc->view[ti.handle] = tiv;

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
        // delete child in view
        const auto itv = cc->view.find(it->second.parent);
        if (itv != cc->view.end())
        {
            cc->view.erase(itv);
        }

        // delete child in items
        if (it->second.parent == handle)
        {
            it = cc->items.erase(it++);
        }
    }

    // delete element in view
    const auto itv = cc->view.find(handle);
    if (itv != cc->view.end())
    {
        cc->view.erase(itv);
    }

    return true;
}

bool Tree::ClearItems()
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    cc->items.clear();
    cc->view.clear();

    cc->nextItemHandle = 1ULL;

    return true;
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

const ItemHandle Tree::GetHandleForNewItem() const
{
    CHECK(Context != nullptr, InvalidItemHandle, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    const auto current = cc->nextItemHandle;
    cc->nextItemHandle++;
    return current;
}

} // namespace AppCUI::Controls
