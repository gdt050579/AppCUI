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

void Tree::RecursiveItemPainting(
      Graphics::Renderer& renderer, const ItemHandle ih, WriteTextParams& wtp, const unsigned int offset) const
{
    CHECKRET(Context != nullptr, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    wtp.X += offset;

    for (const auto& it : cc->items)
    {
        const auto& item = it.second;

        if (item.parent == ih)
        {
            // TODO: remove constants and create functions
            if (IsExpandable(item.handle))
            {
                if (cc->view[item.handle].expanded)
                {
                    wtp.Color = cc->colorText;
                    renderer.WriteText(cc->openBracket, wtp);
                    wtp.X += 2;

                    wtp.Color = cc->colorMinus;
                    renderer.WriteText(cc->minus, wtp);
                    wtp.X += 2;

                    wtp.Color = cc->colorText;
                    renderer.WriteText(cc->closeBracket, wtp);
                    wtp.X += 2;
                }
                else
                {
                    wtp.Color = cc->colorText;
                    renderer.WriteText(cc->openBracket, wtp);
                    wtp.X += 2;

                    wtp.Color = cc->colorPlus;
                    renderer.WriteText(cc->plus, wtp);
                    wtp.X += 2;

                    wtp.Color = cc->colorText;
                    renderer.WriteText(cc->closeBracket, wtp);
                    wtp.X += 2;
                }
            }
            else
            {
                wtp.Color = cc->colorText;
                renderer.WriteText(cc->openBracket, wtp);
                wtp.X += 2;

                wtp.Color = cc->colorNothing;
                renderer.WriteText(cc->nothing, wtp);
                wtp.X += 2;

                wtp.Color = cc->colorText;
                renderer.WriteText(cc->closeBracket, wtp);
                wtp.X += 2;
            }

            wtp.Color = cc->colorText;

            renderer.WriteText(item.value, wtp);

            wtp.X -= cc->offset;

            wtp.Y++;

            if (IsExpandable(item.handle))
            {
                if (cc->view[item.handle].expanded)
                {
                    RecursiveItemPainting(renderer, item.handle, wtp, cc->offset);
                }
            }
        }
    }

    wtp.X -= offset;
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

    const TreeItemView tiv{ ti.handle, false };
    cc->view[ti.handle] = tiv;

    return ti.handle;
}

bool Tree::RemoveItem(const ItemHandle handle)
{
    CHECK(Context != nullptr, "", "");
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
        if (it->second.parent == handle)
        {
            it = cc->items.erase(it++);
        }
    }

    return true;
}

bool Tree::IsExpandable(const ItemHandle handle) const
{
    CHECK(Context != nullptr, "", "");
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
