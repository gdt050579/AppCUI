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
    char nothing[]          = "   ";
    const auto colorPlus    = ColorPair{ Color::Green, Color::White };
    const auto colorMinus   = ColorPair{ Color::Red, Color::White };
    const auto colorNothing = ColorPair{ Color::White, Color::White };
    const auto colorText    = ColorPair{ Color::White, Color::Black };

    if (IsExpandable())
    {
        if (cc->expanded)
        {
            renderer.WriteSingleLineText(0, 0, minus, colorMinus);
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

    renderer.WriteSingleLineText(4, 0, cc->value, colorText);
}

bool Tree::OnKeyEvent(AppCUI::Input::Key keyCode, char16_t)
{
    return false;
}

void Tree::SetValue(const std::string_view& value)
{
    CHECKRET(Context != nullptr, "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    cc->value = value;
}

const std::string_view Tree::GetValue() const
{
    CHECK(Context != nullptr, "", "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    return cc->value;
}

const bool Controls::Tree::IsExpandable() const
{
    CHECK(Context != nullptr, "", "");
    const auto cc = reinterpret_cast<TreeControlContext*>(Context);

    return cc->children.size() > 0;
}

} // namespace AppCUI::Controls
