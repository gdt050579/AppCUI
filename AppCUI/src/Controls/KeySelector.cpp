#include "ControlContext.hpp"

namespace AppCUI::Controls
{
KeySelector::KeySelector(string_view layout, Input::Key keyCode, KeySelectorFlags flags)
    : Control(new KeySelectorContext(), "", layout, false)
{
    auto Members              = reinterpret_cast<KeySelectorContext*>(this->Context);
    Members->Layout.MinWidth  = 5;
    Members->Layout.MinHeight = 1;
    Members->Layout.MaxHeight = 1;
    Members->Layout.Height    = 1;
    Members->key              = keyCode;
    Members->Flags            = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP | (uint32) flags;
}

void KeySelector::Paint(Graphics::Renderer& renderer)
{
    CREATE_TYPECONTROL_CONTEXT(KeySelectorContext, Members, );

    auto col = &Members->Cfg->Text.Normal;

    if (!this->IsEnabled())
        col = &Members->Cfg->Text.Inactive;
    else if (Members->Focused)
        col = &Members->Cfg->Text.Focus;
    else if (Members->MouseIsOver)
        col = &Members->Cfg->Text.Hover;

    int sz = Members->Text.Len();
    if ((sz + 3) > Members->Layout.Width)
        sz = Members->Layout.Width - 3;

    renderer.FillHorizontalLineSize(0, 0, Members->Layout.Width, ' ', col->Text);
    LocalString<64> temp;
    if (KeyUtils::ToString(Members->key, temp))
    {
        renderer.WriteSingleLineText(1, 0, Members->Layout.Width - 1, temp, col->Text);
    }
    if (Members->Focused)
        renderer.SetCursor(1, 0);
}

bool KeySelector::OnKeyEvent(Key KeyCode, char16 characterCode)
{
    CREATE_TYPECONTROL_CONTEXT(KeySelectorContext, Members, false);

    auto noModifiers = KeyCode & KeyUtils::KEY_CODE_MASK;
    switch (noModifiers)
    {
    case Key::Enter:
        if (!(Members->Flags && KeySelectorFlags::ProcessReturn))
            return false;
        break;
    case Key::Tab:
        if (!(Members->Flags && KeySelectorFlags::ProcessTab))
            return false;
        break;
    case Key::Escape:
        if (!(Members->Flags && KeySelectorFlags::ProcessEscape))
            return false;
        break;        
    }
    // set the new value only if not in a read-only mode
    if (!(Members->Flags && KeySelectorFlags::ReadOnly))
        Members->key = KeyCode;

    return true;
}
void KeySelector::SetSelectedKey(Input::Key keyCode)
{
    CREATE_TYPECONTROL_CONTEXT(KeySelectorContext, Members, );
    Members->key = keyCode;
}
Input::Key KeySelector::GetSelectedKey()
{
    CREATE_TYPECONTROL_CONTEXT(KeySelectorContext, Members, Key::None);
    return Members->key;
}
bool KeySelector::OnMouseEnter()
{
    CREATE_TYPECONTROL_CONTEXT(KeySelectorContext, Members, false);
    this->ShowToolTip("Press a key when control has focus");
    return true;
}
bool KeySelector::OnMouseLeave()
{
    CREATE_TYPECONTROL_CONTEXT(KeySelectorContext, Members, false);
    this->HideToolTip();
    return true;
}
} // namespace AppCUI::Controls