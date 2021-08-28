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

const void NumericSelector::SetMinValue(const long long minValue)
{
    CREATE_TYPECONTROL_CONTEXT(NumericSelectorControlContext, Members, );
    Members->minValue = minValue;
}

const void NumericSelector::SetMaxValue(const long long maxValue)
{
    CREATE_TYPECONTROL_CONTEXT(NumericSelectorControlContext, Members, );
    Members->maxValue = maxValue;
}

void NumericSelector::Paint(Renderer& renderer)
{
    CREATE_TYPECONTROL_CONTEXT(NumericSelectorControlContext, Members, );

    [&]()
    {
        const auto& nsCfg = Members->Cfg->NumericSelector;

        if (IsEnabled() == false)
        {
            Members->textColor = nsCfg.Inactive.TextColor;
            return;
        }

        if (Members->MouseIsOver)
        {
            Members->textColor = nsCfg.Hover.TextColor;
            return;
        }

        if (Members->Focused)
        {
            Members->textColor = nsCfg.Focused.TextColor;
            return;
        }

        Members->textColor = nsCfg.Normal.TextColor;
    }();

    renderer.WriteSingleLineText(0, 0, " - ", Members->textColor);
    renderer.DrawHorizontalLine(4, 0, Members->Layout.Width - Members->buttonPadding - 1, ' ', Members->textColor);

    // TODO: find another way!
    LocalString<256> tmp;
    tmp.Format("%lld", Members->value);

    renderer.WriteSingleLineText(
          Members->Layout.Width / 2, 0, tmp.GetText(), Members->textColor, TextAlignament::Center);
    renderer.WriteSingleLineText(Members->Layout.Width + 1 - Members->buttonPadding, 0, " + ", Members->textColor);
}

bool NumericSelector::OnKeyEvent(Key keyCode, char AsciiCode)
{
    CREATE_TYPECONTROL_CONTEXT(NumericSelectorControlContext, Members, false);
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

        if (x < Members->buttonPadding)
        {
            if (Members->minValue < Members->value)
            {
                Members->value--;
                this->RaiseEvent(Event::EVENT_NUMERICSELECTOR_VALUE_CHANGED);
            }
        }
        else if (x > this->GetWidth() - Members->buttonPadding)
        {
            if (Members->maxValue > Members->value)
            {
                Members->value++;
                this->RaiseEvent(Event::EVENT_NUMERICSELECTOR_VALUE_CHANGED);
            }
        }

    default:
        break;
    }
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

} // namespace AppCUI::Controls
