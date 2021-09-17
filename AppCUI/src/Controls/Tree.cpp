#include "ControlContext.hpp"

namespace AppCUI::Controls
{
bool Tree::Create(Control* parent, const std::string_view& layout)
{
    Context = new TreeControlContext();
    CHECK(Context != nullptr, false, "");

    const auto cc        = reinterpret_cast<TreeControlContext*>(Context);
    cc->Layout.MinHeight = 1;
    cc->Layout.MaxHeight = 1;
    cc->Layout.MinWidth  = 10;
    CHECK(Init(parent, "", layout, true), false, "Failed to create tree!");
    cc->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;

    return true;
}

void Tree::Paint(Graphics::Renderer& renderer)
{
    CHECKRET(Context != nullptr, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    // TODO: move these (context, cfg, etc)
    char plus[]             = " + ";
    char minus[]            = " - ";
    char nothing[]          = " * ";
    const auto colorPlus    = ColorPair{ Color::Green, Color::White };
    const auto colorMinus   = ColorPair{ Color::Red, Color::White };
    const auto colorNothing = ColorPair{ Color::Black, Color::White };
    const auto colorText    = ColorPair{ Color::White, Color::Black };

    for (const auto& item : cc->items)
    {
        if (item.second.parent == InvalidItemHandle)
        {
            if (IsExpandable(item.second.handle))
            {
                if (cc->view[item.second.handle].expanded)
                {
                    renderer.WriteSingleLineText(0, 0, minus, colorMinus);
                    // TODO:
                }
                else
                {
                    renderer.WriteSingleLineText(0, 0, plus, colorPlus);
                }
            }
            else
            {
                renderer.WriteSingleLineText(0, 0, nothing, colorNothing);
            }

            renderer.WriteSingleLineText(4, 0, item.second.value, colorText);
        }
        else
        {
            // TODO: here or if (cc->view[item.second.handle].expanded)
        }
    }
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
