#include "AppCUI.hpp"
#include "ControlContext.hpp"

namespace AppCUI
{
NumericSelector::NumericSelector(int64 minValue, int64 maxValue, int64 value, string_view layout)
    : Control(new NumericSelectorControlContext(), "", layout, true)
{
    auto cc              = reinterpret_cast<NumericSelectorControlContext*>(this->Context);
    cc->Layout.MinHeight = 1;
    cc->Layout.MaxHeight = 1;
    cc->Layout.MinWidth  = 10;
    cc->Flags            = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
    cc->minValue         = minValue;
    cc->maxValue         = maxValue;
    cc->instance         = this;

    SetValue(value);
}

int64 NumericSelector::GetValue() const
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);
    return cc->value;
}

void NumericSelector::SetValue(int64 value)
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

void NumericSelector::SetMinValue(int64 minValue)
{
    CHECKRET(Context != nullptr, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);

    cc->minValue = minValue;
    cc->maxValue = std::max<>(minValue, cc->maxValue);
    SetValue(std::min<>(std::max<>(cc->value, minValue), cc->maxValue));
}

void NumericSelector::SetMaxValue(int64 maxValue)
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
    CHECKRET(cc->GetRenderColor(color), "");

    WriteTextParams params(
          WriteTextFlags::SingleLine | WriteTextFlags::OverwriteColors | WriteTextFlags::ClipToWidth |
                WriteTextFlags::FitTextToWidth,
          TextAlignament::Center);
    params.X     = cc->buttonPadding + 1;
    params.Y     = 0;
    params.Width = cc->Layout.Width - (cc->buttonPadding * 2) - 2;
    params.Color = color;

    CHECKRET(cc->FormatTextField(), "");

    if (cc->MinValueReached() && cc->intoInsertionMode == false)
    {
        renderer.WriteSingleLineText(0, 0, " - ", cc->Cfg->NumericSelector.Text.Inactive);
    }
    else
    {
        renderer.WriteSingleLineText(0, 0, " - ", color);
    }

    renderer.FillHorizontalLine(cc->buttonPadding, 0, cc->Layout.Width - cc->buttonPadding - 1, ' ', color);
    renderer.WriteText(cc->stringValue.GetText(), params);

    if (cc->isMouseLeftClickPressed)
    {
        renderer.WriteCharacter(
              static_cast<int32>(cc->sliderPosition + cc->buttonPadding), 0, -1, cc->Cfg->NumericSelector.Text.Hover);
    }

    if (cc->MaxValueReached() && cc->intoInsertionMode == false)
    {
        renderer.WriteSingleLineText(
              cc->Layout.Width + 1 - cc->buttonPadding, 0, " + ", cc->Cfg->NumericSelector.Text.Inactive);
    }
    else
    {
        renderer.WriteSingleLineText(cc->Layout.Width + 1 - cc->buttonPadding, 0, " + ", color);
    }

    switch (cc->isMouseOn)
    {
    case NumericSelectorControlContext::IsMouseOn::MinusButton:
        if (cc->MinValueReached() == false)
        {
            renderer.FillHorizontalLine(0, 0, cc->buttonPadding - 2, -1, cc->Cfg->NumericSelector.Text.Hover);
        }
        break;
    case NumericSelectorControlContext::IsMouseOn::PlusButton:
        if (cc->MaxValueReached() == false)
        {
            renderer.FillHorizontalLine(
                  GetWidth() - cc->buttonPadding + 1, 0, GetWidth(), -1, cc->Cfg->NumericSelector.Text.Hover);
        }
        break;
    case NumericSelectorControlContext::IsMouseOn::TextField:
        if (static_cast<int32>(cc->stringValue.Len()) > cc->Layout.Width - cc->buttonPadding * 2 - 2)
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

bool NumericSelector::OnKeyEvent(Key keyCode, char16 unicodeChar)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);

    switch (keyCode)
    {
    case Key::Left:
        if (cc->intoInsertionMode)
        {
            cc->insertionModevalue--;
        }
        else
        {
            SetValue(cc->value - 1);
        }
        return true;

    case Key::Right:
        if (cc->intoInsertionMode)
        {
            cc->insertionModevalue++;
        }
        else
        {
            SetValue(cc->value + 1);
        }
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
            if (cc->IsValueInsertedWrong() == false)
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
        auto toAdd            = static_cast<int32>(keyCode) - static_cast<int32>(Key::N0);
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
        OS::Clipboard::SetText(cc->stringValue.GetText());
        return true;
    case Key::Ctrl | Key::V:
    {
        LocalUnicodeStringBuilder<256> b{};
        OS::Clipboard::GetText(b);
        const std::string output(b);

        if (output.empty())
        {
            SetValue(0);
        }
        else
        {
            const optional<int64> value = Number::ToUInt64(output);
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
        const auto percentFive = static_cast<int64>(std::max<>(std::abs((cc->maxValue - cc->minValue) / 20LL), 1LL));
        SetValue(cc->value + percentFive);
    }
        return true;
    case Key::PageDown:
    {
        const auto percentFive = static_cast<int64>(std::max<>(std::abs((cc->maxValue - cc->minValue) / 20LL), 1LL));
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
    case MouseButton::Left | MouseButton::DoubleClicked:
        if (cc->IsOnMinusButton(x, y) == false && cc->IsOnPlusButton(x, y) == false)
        {
            cc->isMouseLeftClickPressed = true;
            break;
        }
        else
        {
            cc->isMouseLeftClickPressed = false;
        }

        // height is always 1 constrained - y doesn't matter
        if (cc->IsOnMinusButton(x, y))
        {
            if (cc->intoInsertionMode)
            {
                cc->insertionModevalue--;
            }
            else
            {
                SetValue(cc->value - 1);
            }
        }
        else if (cc->IsOnPlusButton(x, y))
        {
            if (cc->intoInsertionMode)
            {
                cc->insertionModevalue++;
            }
            else
            {
                SetValue(cc->value + 1);
            }
        }
        else if (cc->IsOnTextField(x, y))
        {
            const int64& max = cc->maxValue;
            const int64& min = cc->minValue;

            const int64 valueIntervalLength = max - min;

            const int64& lowerBound = cc->buttonPadding;
            const int64 upperBound  = static_cast<int64>(this->GetWidth()) - cc->buttonPadding - 1LL;

            const int64 boundIntervalLength = upperBound - lowerBound;

            cc->sliderPosition = x - lowerBound;
            const double ratio = static_cast<double>(cc->sliderPosition) / boundIntervalLength;

            SetValue(min + static_cast<int64>(valueIntervalLength * ratio));
        }
        break;
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

bool NumericSelector::OnMouseDrag(int x, int y, Input::MouseButton button)
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);

    switch (button)
    {
    case MouseButton::Left:

        // height is always 1 constrained - y doesn't matter

        if (cc->IsOnTextField(x, y) && cc->isMouseLeftClickPressed)
        {
            const int64& max = cc->maxValue;
            const int64& min = cc->minValue;

            const int64 valueIntervalLength = max - min;

            const int64& lowerBound = cc->buttonPadding;
            const int64 upperBound  = static_cast<int64>(this->GetWidth()) - cc->buttonPadding - 1LL;

            const int64 boundIntervalLength = upperBound - lowerBound;

            cc->sliderPosition = x - lowerBound;
            const double ratio = static_cast<double>(cc->sliderPosition) / boundIntervalLength;

            SetValue(min + static_cast<int64>(valueIntervalLength * ratio));

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
    if (cc->IsOnTextField(x, y))
    {
        cc->isMouseOn = NumericSelectorControlContext::IsMouseOn::TextField;
    }
    else if (cc->IsOnMinusButton(x, y))
    {
        cc->isMouseOn = NumericSelectorControlContext::IsMouseOn::MinusButton;
    }
    else if (cc->IsOnPlusButton(x, y))
    {
        cc->isMouseOn = NumericSelectorControlContext::IsMouseOn::PlusButton;
    }
    else
    {
        cc->isMouseOn = NumericSelectorControlContext::IsMouseOn::None;
    }
    return true;
}

bool NumericSelectorControlContext::IsValidValue(int64) const
{
    return minValue <= insertionModevalue && insertionModevalue <= maxValue;
}

bool NumericSelectorControlContext::IsValueInsertedWrong() const
{
    return IsValidValue(insertionModevalue) == false;
}

bool NumericSelectorControlContext::GetRenderColor(Graphics::ColorPair& color) const
{
    const auto& nsCfg = Cfg->NumericSelector;

    if (instance->IsEnabled() == false)
    {
        color = nsCfg.Text.Inactive;
    }
    else if (intoInsertionMode && IsValueInsertedWrong())
    {
        color = nsCfg.Text.WrongValue;
    }
    else if (Focused)
    {
        color = nsCfg.Text.Focused;
    }
    else
    {
        color = nsCfg.Text.Normal;
    }

    return true;
}

bool NumericSelectorControlContext::FormatTextField()
{
    if (intoInsertionMode)
    {
        if (insertionModevalue != 0LL || wasMinusPressed == false)
        {
            stringValue.Format("%lld", insertionModevalue);
        }
    }
    else
    {
        stringValue.Format("%lld", value);
    }

    return true;
}

bool NumericSelectorControlContext::IsOnMinusButton(int32 x, int32) const
{
    return x < buttonPadding;
}

bool NumericSelectorControlContext::IsOnPlusButton(int32 x, int32) const
{
    return (x > instance->GetWidth() - buttonPadding - 1);
}

bool NumericSelectorControlContext::IsOnTextField(int32 x, int32) const
{
    return (x >= buttonPadding && x < instance->GetWidth() - buttonPadding);
}

bool NumericSelectorControlContext::MinValueReached() const
{
    return value == minValue;
}

bool NumericSelectorControlContext::MaxValueReached() const
{
    return value == maxValue;
}
} // namespace AppCUI