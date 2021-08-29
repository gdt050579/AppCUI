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
    CREATE_TYPECONTROL_CONTEXT(NumericSelectorControlContext, Members, false);
    Members->Layout.MinHeight = 1;
    Members->Layout.MaxHeight = 1;
    Members->Layout.MinWidth  = 10;
    CHECK(Init(parent, "", layout, true), false, "Failed to create numeric selector!");
    Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;

    Members->minValue = minValue;
    Members->maxValue = maxValue;
    Members->value    = value;

    return true;
}

const long long NumericSelector::GetValue() const
{
    CREATE_TYPECONTROL_CONTEXT(NumericSelectorControlContext, Members, 0);
    return Members->value;
}

const void NumericSelector::SetValue(const long long value)
{
    CREATE_TYPECONTROL_CONTEXT(NumericSelectorControlContext, Members, );

    if (value < Members->minValue)
    {
        Members->value = Members->minValue;
    }
    else if (value > Members->maxValue)
    {
        Members->value = Members->maxValue;
    }
    else
    {
        Members->value = value;
    }
}

const void NumericSelector::SetMinValue(const long long minValue)
{
    CREATE_TYPECONTROL_CONTEXT(NumericSelectorControlContext, Members, );

    Members->minValue = minValue;
    Members->maxValue = std::max<>(minValue, Members->maxValue);
    Members->value    = std::min<>(std::max<>(Members->value, minValue), Members->maxValue);
}

const void NumericSelector::SetMaxValue(const long long maxValue)
{
    CREATE_TYPECONTROL_CONTEXT(NumericSelectorControlContext, Members, );

    Members->maxValue = maxValue;
    Members->minValue = std::min<>(maxValue, Members->minValue);
    Members->value    = std::max<>(std::min<>(Members->value, maxValue), Members->minValue);
}

void NumericSelector::Paint(Renderer& renderer)
{
    CREATE_TYPECONTROL_CONTEXT(NumericSelectorControlContext, Members, );

    WriteTextParams params(
          WriteTextFlags::SingleLine | WriteTextFlags::OverwriteColors | WriteTextFlags::ClipToWidth |
                WriteTextFlags::FitTextToWidth,
          TextAlignament::Center);
    params.X     = Members->buttonPadding + 1;
    params.Y     = 0;
    params.Width = Members->Layout.Width - (Members->buttonPadding * 2) - 1;

    const auto& nsCfg = Members->Cfg->NumericSelector;

    if (IsEnabled() == false)
    {
        params.Color = nsCfg.Inactive.TextColor;
    }
    else if (Members->intoInsertionMode && Members->wrongValueInserted)
    {
        params.Color = nsCfg.WrongValue.TextColor;
        Members->wrongValueInserted;
    }
    else if (Members->Focused)
    {
        params.Color = nsCfg.Focused.TextColor;
    }
    else if (Members->MouseIsOver)
    {
        params.Color = nsCfg.Hover.TextColor;
    }
    else
    {
        params.Color = nsCfg.Normal.TextColor;
    }

    renderer.WriteSingleLineText(0, 0, " - ", params.Color);
    renderer.DrawHorizontalLine(4, 0, Members->Layout.Width - Members->buttonPadding - 1, ' ', params.Color);

    if (Members->intoInsertionMode)
    {
        if (Members->insertionModevalue != 0LL || Members->wasMinusPressed == false)
        {
            Members->stringValue.Format("%lld", Members->insertionModevalue);
        }
    }
    else
    {
        Members->stringValue.Format("%lld", Members->value);
    }

    renderer.WriteText(Members->stringValue.GetText(), params);

    renderer.WriteSingleLineText(Members->Layout.Width + 1 - Members->buttonPadding, 0, " + ", params.Color);
}

bool NumericSelector::OnKeyEvent(Key keyCode, char AsciiCode)
{
    CREATE_TYPECONTROL_CONTEXT(NumericSelectorControlContext, Members, false);

    // too many early returns so I'm just doing this
    const struct VerifyValueInserted
    {
        NumericSelectorControlContext* Members;
        ~VerifyValueInserted()
        {
            Members->wrongValueInserted = (Members->minValue <= Members->insertionModevalue &&
                                           Members->insertionModevalue <= Members->maxValue) == false;
        }
    } verifyValueInserted{ Members };

    switch (keyCode)
    {
    case Key::Left:
        if (Members->minValue < Members->value)
        {
            Members->value--;
            this->RaiseEvent(Event::EVENT_NUMERICSELECTOR_VALUE_CHANGED);
        }
        return true;

    case Key::Right:
        if (Members->maxValue > Members->value)
        {
            Members->value++;
            this->RaiseEvent(Event::EVENT_NUMERICSELECTOR_VALUE_CHANGED);
        }
        return true;

    case Key::Escape:
        Members->intoInsertionMode  = false;
        Members->insertionModevalue = 0LL;
        return true;

    case Key::Enter:
        if (Members->intoInsertionMode)
        {
            Members->intoInsertionMode = false;
            if (Members->minValue <= Members->insertionModevalue && Members->insertionModevalue <= Members->maxValue)
            {
                Members->value = Members->insertionModevalue;
            }
            Members->insertionModevalue = 0LL;
            this->RaiseEvent(Event::EVENT_NUMERICSELECTOR_VALUE_CHANGED);
        }
        return true;

    case Key::Minus:
        Members->intoInsertionMode  = true;
        Members->wasMinusPressed    = true;
        Members->insertionModevalue = 0LL;
        Members->stringValue.Set("-");
        return true;

    case Key::Shift | Key::Plus:
    case Key::Plus:
        Members->intoInsertionMode  = true;
        Members->wasMinusPressed    = false;
        Members->insertionModevalue = 0LL;
        Members->stringValue.Set("+");
        return true;

    case Key::Backspace:
        if (Members->intoInsertionMode == false)
        {
            Members->insertionModevalue = Members->value;
        }

        Members->intoInsertionMode = true;
        if (-9 <= Members->insertionModevalue && Members->insertionModevalue <= 9)
        {
            Members->insertionModevalue = 0;
        }
        else
        {
            Members->insertionModevalue /= 10;
        }
        this->RaiseEvent(Event::EVENT_NUMERICSELECTOR_VALUE_CHANGED);
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
        if (Members->intoInsertionMode == false)
        {
            Members->insertionModevalue = Members->value;
        }

        Members->intoInsertionMode = true;
        int toAdd                  = static_cast<int>(keyCode) - static_cast<int>(Key::N0);
        if (Members->insertionModevalue < 0)
        {
            toAdd *= -1;
        }

        Members->insertionModevalue = Members->insertionModevalue * 10LL + toAdd;
        if (Members->wasMinusPressed)
        {
            Members->insertionModevalue = Members->insertionModevalue * -1;
            Members->wasMinusPressed    = false;
        }

        this->RaiseEvent(Event::EVENT_NUMERICSELECTOR_VALUE_CHANGED);
    }
        return true;
    default:
        break;
    }

    return false;
}

void NumericSelector::OnMousePressed(int x, int y, MouseButton button)
{
    CREATE_TYPECONTROL_CONTEXT(NumericSelectorControlContext, Members, );

    switch (button)
    {
    case MouseButton::Left:

        // height is always 1 constrained - y doesn't matter

        if (x < Members->buttonPadding) // "-" button
        {
            if (Members->minValue < Members->value)
            {
                Members->value--;
                this->RaiseEvent(Event::EVENT_NUMERICSELECTOR_VALUE_CHANGED);
            }
        }
        else if (x > this->GetWidth() - Members->buttonPadding) // "+" button
        {
            if (Members->maxValue > Members->value)
            {
                Members->value++;
                this->RaiseEvent(Event::EVENT_NUMERICSELECTOR_VALUE_CHANGED);
            }
        }
        else if (x >= Members->buttonPadding && x <= this->GetWidth() - Members->buttonPadding - 1) // text field
        {
            const long long& max = Members->maxValue;
            const long long& min = Members->minValue;

            const long long valueIntervalLength = max - min;

            const long long& lowerBound = Members->buttonPadding;
            const long long upperBound  = static_cast<long long>(this->GetWidth()) - Members->buttonPadding - 1LL;

            const long long boundIntervalLength = upperBound - lowerBound;

            const long long pointOnInterval = x - lowerBound;
            const double ratio              = static_cast<double>(pointOnInterval) / boundIntervalLength;

            Members->value = std::min<>(min + static_cast<long long>(valueIntervalLength * ratio), Members->maxValue);

            this->RaiseEvent(Event::EVENT_NUMERICSELECTOR_VALUE_CHANGED);
        }

    default:
        break;
    }
}

void NumericSelector::OnLoseFocus()
{
    CREATE_TYPECONTROL_CONTEXT(NumericSelectorControlContext, Members, );

    Members->intoInsertionMode  = false;
    Members->insertionModevalue = 0LL;
}

bool NumericSelector::OnMouseWheel(int x, int y, MouseWheel direction)
{
    CREATE_TYPECONTROL_CONTEXT(NumericSelectorControlContext, Members, false);

    switch (direction)
    {
    case MouseWheel::Down:
    case MouseWheel::Left:
        if (Members->minValue < Members->value)
        {
            Members->value--;
            this->RaiseEvent(Event::EVENT_NUMERICSELECTOR_VALUE_CHANGED);
        }
        return true;

    case MouseWheel::Up:
    case MouseWheel::Right:
        if (Members->maxValue > Members->value)
        {
            Members->value++;
            this->RaiseEvent(Event::EVENT_NUMERICSELECTOR_VALUE_CHANGED);
        }
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
    CREATE_TYPECONTROL_CONTEXT(NumericSelectorControlContext, Members, false);

    switch (button)
    {
    case MouseButton::Left:

        // height is always 1 constrained - y doesn't matter

        if (x < Members->buttonPadding) // "-" button
        {
            Members->value = Members->minValue;
            this->RaiseEvent(Event::EVENT_NUMERICSELECTOR_VALUE_CHANGED);
            return true;
        }
        else if (x > this->GetWidth() - Members->buttonPadding) // "+" button
        {
            Members->value = Members->maxValue;
            this->RaiseEvent(Event::EVENT_NUMERICSELECTOR_VALUE_CHANGED);
            return true;
        }
        else if (x >= Members->buttonPadding && x <= this->GetWidth() - Members->buttonPadding - 1) // text field
        {
            const long long& max = Members->maxValue;
            const long long& min = Members->minValue;

            const long long valueIntervalLength = max - min;

            const long long& lowerBound = Members->buttonPadding;
            const long long upperBound  = static_cast<long long>(this->GetWidth()) - Members->buttonPadding - 1LL;

            const long long boundIntervalLength = upperBound - lowerBound;

            const long long pointOnInterval = x - lowerBound;
            const double ratio              = static_cast<double>(pointOnInterval) / boundIntervalLength;

            Members->value = std::min<>(min + static_cast<long long>(valueIntervalLength * ratio), Members->maxValue);

            this->RaiseEvent(Event::EVENT_NUMERICSELECTOR_VALUE_CHANGED);
            return true;
        }

    default:
        break;
    }

    return false;
}

} // namespace AppCUI::Controls
