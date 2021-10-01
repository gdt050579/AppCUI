#include "AppCUI.hpp"
#include "ControlContext.hpp"

using namespace AppCUI::Input;
using namespace AppCUI::Graphics;

namespace AppCUI::Controls
{
std::unique_ptr<NumericSelector> NumericSelector::Create(
      const long long minValue,
      const long long maxValue,
      long long value,
      const std::string_view& layout)
{
    INIT_CONTROL(NumericSelector, NumericSelectorControlContext);

    const auto cc        = Members;
    cc->Layout.MinHeight = 1;
    cc->Layout.MaxHeight = 1;
    cc->Layout.MinWidth  = 10;
    CHECK(me->Init("", layout, true), nullptr, "Failed to create numeric selector!");
    cc->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;

    cc->minValue = minValue;
    cc->maxValue = maxValue;
    me->SetValue(value);

    return me;
}
NumericSelector* NumericSelector::Create(
      Control& parent,
      const long long minValue,
      const long long maxValue,
      long long value,
      const std::string_view& layout)
{
    auto me = NumericSelector::Create(minValue, maxValue, value, layout);
    CHECK(me, nullptr, "Fail to create a NumericSelector control !");
    return parent.AddControl<NumericSelector>(std::move(me));
}    
    
long long NumericSelector::GetValue() const
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);
    return cc->value;
}

void NumericSelector::SetValue(const long long value)
{
    CHECKRET(Context != nullptr, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);

    if (value < cc->minValue)
    {
        cc->value = cc->minValue;
    }
    else if (value > cc->maxValue)
    {
        cc->value = cc->maxValue;
    }
    else
    {
        cc->value = value;
    }

    RaiseEvent(Event::NumericSelectorValueChanged);
}

void NumericSelector::SetMinValue(const long long minValue)
{
    CHECKRET(Context != nullptr, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);

    cc->minValue = minValue;
    cc->maxValue = std::max<>(minValue, cc->maxValue);
    SetValue(std::min<>(std::max<>(cc->value, minValue), cc->maxValue));
}

void NumericSelector::SetMaxValue(const long long maxValue)
{
    CHECKRET(Context != nullptr, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);

    cc->maxValue = maxValue;
    cc->minValue = std::min<>(maxValue, cc->minValue);
    SetValue(std::max<>(std::min<>(cc->value, maxValue), cc->minValue));
}

void NumericSelector::Paint(Renderer& renderer)
{
    CHECKRET(Context != nullptr, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);

    ColorPair color{};
    CHECKRET(GetRenderColor(color), "");

    WriteTextParams params(
          WriteTextFlags::SingleLine | WriteTextFlags::OverwriteColors | WriteTextFlags::ClipToWidth |
                WriteTextFlags::FitTextToWidth,
          TextAlignament::Center);
    params.X     = cc->buttonPadding + 1;
    params.Y     = 0;
    params.Width = cc->Layout.Width - (cc->buttonPadding * 2) - 2;
    params.Color = color;

    CHECKRET(FormatTextField(), "");

    renderer.WriteSingleLineText(0, 0, " - ", color);
    renderer.FillHorizontalLine(cc->buttonPadding, 0, cc->Layout.Width - cc->buttonPadding - 1, ' ', color);
    renderer.WriteText(cc->stringValue.GetText(), params);

    if (cc->isMouseLeftClickPressed)
    {
        renderer.WriteCharacter(
              static_cast<int>(cc->sliderPosition + cc->buttonPadding),
              0,
              -1,
              cc->Cfg->NumericSelector.Hover.TextColor);
    }

    renderer.WriteSingleLineText(cc->Layout.Width + 1 - cc->buttonPadding, 0, " + ", color);

    switch (cc->isMouseOn)
    {
    case NumericSelectorControlContext::IsMouseOn::MinusButton:
        renderer.FillHorizontalLine(0, 0, cc->buttonPadding - 2, -1, cc->Cfg->NumericSelector.Hover.TextColor);
        break;
    case NumericSelectorControlContext::IsMouseOn::PlusButton:
        renderer.FillHorizontalLine(
              GetWidth() - cc->buttonPadding + 1, 0, GetWidth(), -1, cc->Cfg->NumericSelector.Hover.TextColor);
        break;
    case NumericSelectorControlContext::IsMouseOn::TextField:
        if (static_cast<int>(cc->stringValue.Len()) > cc->Layout.Width - cc->buttonPadding * 2 - 2)
        {
            ShowToolTip(cc->stringValue.GetText());
        }
        else
        {
            HideToolTip();
        }
        break;
    case NumericSelectorControlContext::IsMouseOn::None:
    default:
        break;
    }
}

bool NumericSelector::OnKeyEvent(Key keyCode, char16_t unicodeChar)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);

    switch (keyCode)
    {
    case Key::Left:
        SetValue(cc->value - 1);
        return true;

    case Key::Right:
        SetValue(cc->value + 1);
        return true;

    case Key::Escape:
        if (cc->intoInsertionMode == false)
        {
            return false;
        }

        cc->intoInsertionMode  = false;
        cc->insertionModevalue = 0LL;
        return true;

    case Key::Enter:
        if (cc->intoInsertionMode)
        {
            cc->intoInsertionMode = false;
            if (IsValueInsertedWrong() == false)
            {
                SetValue(cc->insertionModevalue);
            }
            cc->insertionModevalue = 0LL;
        }
        return true;

    case Key::Backspace:
        if (cc->intoInsertionMode == false)
        {
            cc->insertionModevalue = cc->value;
        }

        cc->intoInsertionMode = true;
        if (-9 <= cc->insertionModevalue && cc->insertionModevalue <= 9)
        {
            cc->insertionModevalue = 0;
        }
        else
        {
            cc->insertionModevalue /= 10;
        }
        return true;

    case Key::N0:
    case Key::N1:
    case Key::N2:
    case Key::N3:
    case Key::N4:
    case Key::N5:
    case Key::N6:
    case Key::N7:
    case Key::N8:
    case Key::N9:
    {
        if (cc->intoInsertionMode == false)
        {
            cc->insertionModevalue = cc->value;
        }

        cc->intoInsertionMode = true;
        int toAdd             = static_cast<int>(keyCode) - static_cast<int>(Key::N0);
        if (cc->insertionModevalue < 0)
        {
            toAdd *= -1;
        }

        cc->insertionModevalue = cc->insertionModevalue * 10LL + toAdd;
        if (cc->wasMinusPressed)
        {
            cc->insertionModevalue = cc->insertionModevalue * -1;
            cc->wasMinusPressed    = false;
        }
    }
        return true;

    case Key::Ctrl | Key::C:
        AppCUI::OS::Clipboard::SetText(cc->stringValue.GetText());
        return true;
    case Key::Ctrl | Key::V:
    {
        LocalUnicodeStringBuilder<256> b{};
        AppCUI::OS::Clipboard::GetText(b);
        const std::string output(b);

        if (output.empty())
        {
            SetValue(0);
        }
        else
        {
            const std::optional<long long> value = Number::ToUInt64(output);
            if (value.has_value())
            {
                cc->insertionModevalue = value.value();
                cc->intoInsertionMode  = true;
            }
            else
            {
                LOG_ERROR("Invalid argument pasted: [%s]!", output.c_str());
            }
        }
    }
        return true;

    case Key::PageUp:
    {
        const auto percentFive =
              static_cast<long long>(std::max<>(std::abs((cc->maxValue - cc->minValue) / 20LL), 1LL));
        SetValue(cc->value + percentFive);
    }
        return true;
    case Key::PageDown:
    {
        const auto percentFive =
              static_cast<long long>(std::max<>(std::abs((cc->maxValue - cc->minValue) / 20LL), 1LL));
        SetValue(cc->value - percentFive);
    }
        return true;

    case Key::Home:
        SetValue(cc->minValue);
        return true;
    case Key::End:
        SetValue(cc->maxValue);
        return true;

    default:
        if (unicodeChar == u'+')
        {
            cc->intoInsertionMode  = true;
            cc->wasMinusPressed    = false;
            cc->insertionModevalue = 0LL;
            return true;
        }
        else if (unicodeChar == u'-')
        {
            cc->intoInsertionMode  = true;
            cc->wasMinusPressed    = true;
            cc->insertionModevalue = 0LL;
            cc->stringValue.Set("-");
            return true;
        }
        break;
    }

    return false;
}

void NumericSelector::OnMousePressed(int x, int y, MouseButton button)
{
    CHECKRET(Context != nullptr, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);

    switch (button)
    {
    case MouseButton::Left:
        if (IsOnMinusButton(x, y) == false && IsOnPlusButton(x, y) == false)
        {
            cc->isMouseLeftClickPressed = true;
            break;
        }
        else
        {
            cc->isMouseLeftClickPressed = false;
        }

        // height is always 1 constrained - y doesn't matter

        if (IsOnMinusButton(x, y))
        {
            SetValue(cc->value - 1);
        }
        else if (IsOnPlusButton(x, y))
        {
            SetValue(cc->value + 1);
        }
        else if (IsOnTextField(x, y))
        {
            const long long& max = cc->maxValue;
            const long long& min = cc->minValue;

            const long long valueIntervalLength = max - min;

            const long long& lowerBound = cc->buttonPadding;
            const long long upperBound  = static_cast<long long>(this->GetWidth()) - cc->buttonPadding - 1LL;

            const long long boundIntervalLength = upperBound - lowerBound;

            cc->sliderPosition = x - lowerBound;
            const double ratio = static_cast<double>(cc->sliderPosition) / boundIntervalLength;

            SetValue(min + static_cast<long long>(valueIntervalLength * ratio));
        }

    default:
        break;
    }
}

void NumericSelector::OnMouseReleased(int, int, MouseButton)
{
    CHECKRET(Context != nullptr, "");
    const auto cc               = reinterpret_cast<NumericSelectorControlContext*>(Context);
    cc->isMouseLeftClickPressed = false;
}

void NumericSelector::OnLoseFocus()
{
    CHECKRET(Context != nullptr, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);

    cc->intoInsertionMode  = false;
    cc->insertionModevalue = 0LL;
    cc->isMouseOn          = NumericSelectorControlContext::IsMouseOn::None;
}

bool NumericSelector::OnMouseWheel(int, int, MouseWheel direction)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);

    switch (direction)
    {
    case MouseWheel::Down:
    case MouseWheel::Left:
        SetValue(cc->value - 1);
        return true;

    case MouseWheel::Up:
    case MouseWheel::Right:
        SetValue(cc->value + 1);
        return true;

    default:
        break;
    }

    return false;
}

bool NumericSelector::OnMouseEnter()
{
    return true;
}

bool NumericSelector::OnMouseLeave()
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);
    cc->isMouseOn = NumericSelectorControlContext::IsMouseOn::None;
    return true;
}

bool NumericSelector::OnMouseDrag(int x, int y, AppCUI::Input::MouseButton button)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);

    switch (button)
    {
    case MouseButton::Left:

        // height is always 1 constrained - y doesn't matter

        if (IsOnTextField(x, y) && cc->isMouseLeftClickPressed)
        {
            const long long& max = cc->maxValue;
            const long long& min = cc->minValue;

            const long long valueIntervalLength = max - min;

            const long long& lowerBound = cc->buttonPadding;
            const long long upperBound  = static_cast<long long>(this->GetWidth()) - cc->buttonPadding - 1LL;

            const long long boundIntervalLength = upperBound - lowerBound;

            cc->sliderPosition = x - lowerBound;
            const double ratio = static_cast<double>(cc->sliderPosition) / boundIntervalLength;

            SetValue(min + static_cast<long long>(valueIntervalLength * ratio));

            return true;
        }
        else
        {
            return true;
        }

    default:
        break;
    }

    return false;
}

bool NumericSelector::OnMouseOver(int x, int y)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);
    if (IsOnTextField(x, y))
    {
        cc->isMouseOn = NumericSelectorControlContext::IsMouseOn::TextField;
    }
    else if (IsOnMinusButton(x, y))
    {
        cc->isMouseOn = NumericSelectorControlContext::IsMouseOn::MinusButton;
    }
    else if (IsOnPlusButton(x, y))
    {
        cc->isMouseOn = NumericSelectorControlContext::IsMouseOn::PlusButton;
    }
    else
    {
        cc->isMouseOn = NumericSelectorControlContext::IsMouseOn::None;
    }
    return true;
}

bool NumericSelector::IsValidValue(const long long) const
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);
    return cc->minValue <= cc->insertionModevalue && cc->insertionModevalue <= cc->maxValue;
}

bool NumericSelector::IsValueInsertedWrong() const
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);
    return IsValidValue(cc->insertionModevalue) == false;
}

bool NumericSelector::GetRenderColor(Graphics::ColorPair& color) const
{
    CHECK(Context != nullptr, false, "");
    const auto cc     = reinterpret_cast<NumericSelectorControlContext*>(Context);
    const auto& nsCfg = cc->Cfg->NumericSelector;

    if (IsEnabled() == false)
    {
        color = nsCfg.Inactive.TextColor;
    }
    else if (cc->intoInsertionMode && IsValueInsertedWrong())
    {
        color = nsCfg.WrongValue.TextColor;
    }
    else if (cc->Focused)
    {
        color = nsCfg.Focused.TextColor;
    }
    else
    {
        color = nsCfg.Normal.TextColor;
    }

    return true;
}

bool NumericSelector::FormatTextField()
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);

    if (cc->intoInsertionMode)
    {
        if (cc->insertionModevalue != 0LL || cc->wasMinusPressed == false)
        {
            cc->stringValue.Format("%lld", cc->insertionModevalue);
        }
    }
    else
    {
        cc->stringValue.Format("%lld", cc->value);
    }

    return true;
}

bool NumericSelector::IsOnMinusButton(const int x, const int) const
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);
    return (x < cc->buttonPadding);
}

bool NumericSelector::IsOnPlusButton(const int x, const int) const
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);
    return (x > GetWidth() - cc->buttonPadding - 1);
}

bool NumericSelector::IsOnTextField(const int x, const int) const
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);
    return (x >= cc->buttonPadding && x < this->GetWidth() - cc->buttonPadding);
}

} // namespace AppCUI::Controls
