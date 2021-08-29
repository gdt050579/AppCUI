#pragma once

#include "AppCUI.hpp"
#include "ControlContext.hpp"

using namespace AppCUI::Input;
using namespace AppCUI::Graphics;

namespace AppCUI::Controls
{
bool NumericSelector::Create(
      Control* parent,
      const long long minValue,
      const long long maxValue,
      long long value,
      const std::string_view& layout)
{
    CONTROL_INIT_CONTEXT(NumericSelectorControlContext);
    CHECK(Context != nullptr, false, "");

    const auto cc        = reinterpret_cast<NumericSelectorControlContext*>(Context);
    cc->Layout.MinHeight = 1;
    cc->Layout.MaxHeight = 1;
    cc->Layout.MinWidth  = 10;
    CHECK(Init(parent, "", layout, true), false, "Failed to create numeric selector!");
    cc->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;

    cc->minValue = minValue;
    cc->maxValue = maxValue;
    SetValue(value);

    return true;
}

const long long NumericSelector::GetValue() const
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);
    return cc->value;
}

const void NumericSelector::SetValue(const long long value)
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

    RaiseEvent(Event::EVENT_NUMERICSELECTOR_VALUE_CHANGED);
}

const void NumericSelector::SetMinValue(const long long minValue)
{
    CHECKRET(Context != nullptr, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);

    cc->minValue = minValue;
    cc->maxValue = std::max<>(minValue, cc->maxValue);
    SetValue(std::min<>(std::max<>(cc->value, minValue), cc->maxValue));
}

const void NumericSelector::SetMaxValue(const long long maxValue)
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
    renderer.DrawHorizontalLine(cc->buttonPadding, 0, cc->Layout.Width - cc->buttonPadding - 1, ' ', color);
    renderer.WriteText(cc->stringValue.GetText(), params);

    if (cc->isMouseLeftClickPressed)
    {
        int charCode = 0;
        ColorPair usedColor{};
        renderer.GetCharacter(static_cast<int>(cc->sliderPosition + cc->buttonPadding), 0, charCode, usedColor);
        usedColor.Background = cc->Cfg->NumericSelector.Hover.TextColor.Background;
        renderer.WriteCharacter(static_cast<int>(cc->sliderPosition + cc->buttonPadding), 0, charCode, usedColor);
    }

    renderer.WriteSingleLineText(cc->Layout.Width + 1 - cc->buttonPadding, 0, " + ", color);
}

bool NumericSelector::OnKeyEvent(Key keyCode, char16_t UnicodeChar)
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

    case Key::Minus:
        cc->intoInsertionMode  = true;
        cc->wasMinusPressed    = true;
        cc->insertionModevalue = 0LL;
        cc->stringValue.Set("-");
        return true;

    case Key::Shift | Key::Plus:
    case Key::Plus:
        cc->intoInsertionMode  = true;
        cc->wasMinusPressed    = false;
        cc->insertionModevalue = 0LL;
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
    default:
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

void NumericSelector::OnMouseReleased(int x, int y, MouseButton button)
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
}

bool NumericSelector::OnMouseWheel(int x, int y, MouseWheel direction)
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

const bool NumericSelector::IsValidValue(const long long value) const
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);
    return cc->minValue <= cc->insertionModevalue && cc->insertionModevalue <= cc->maxValue;
}

const bool NumericSelector::IsValueInsertedWrong() const
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);
    return IsValidValue(cc->insertionModevalue) == false;
}

const bool NumericSelector::GetRenderColor(Graphics::ColorPair& color) const
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
    else if (cc->MouseIsOver)
    {
        color = nsCfg.Hover.TextColor;
    }
    else
    {
        color = nsCfg.Normal.TextColor;
    }

    return true;
}

const bool NumericSelector::FormatTextField()
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

const bool NumericSelector::IsOnMinusButton(const int x, const int y) const
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);
    return (x < cc->buttonPadding);
}

const bool NumericSelector::IsOnPlusButton(const int x, const int y) const
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);
    return (x > GetWidth() - cc->buttonPadding - 1);
}

const bool NumericSelector::IsOnTextField(const int x, const int y) const
{
    CHECK(Context != nullptr, false, "");
    const auto cc = reinterpret_cast<NumericSelectorControlContext*>(Context);
    return (x >= cc->buttonPadding && x < this->GetWidth() - cc->buttonPadding);
}

} // namespace AppCUI::Controls
