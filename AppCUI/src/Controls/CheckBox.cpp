#include "ControlContext.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;
using namespace AppCUI::Input;

bool AppCUI::Controls::CheckBox::Create(Control* parent, const AppCUI::Utils::ConstString& caption, const std::string_view& layout, int controlID)
{
    CONTROL_INIT_CONTEXT(ControlContext);
    CREATE_CONTROL_CONTEXT(this, Members, false);
    Members->Layout.MinWidth  = 5;
    Members->Layout.MinHeight = 1;
    CHECK(Init(parent, caption, layout, true), false, "Unable to create check box !");
    Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
    SetControlID(controlID);
    return true;
}
void AppCUI::Controls::CheckBox::Paint(Graphics::Renderer& renderer)
{
    CREATE_CONTROL_CONTEXT(this, Members, );

    auto* cbc = &Members->Cfg->StateControl.Normal;
    if (!this->IsEnabled())
        cbc = &Members->Cfg->StateControl.Inactive;
    if (Members->Focused)
        cbc = &Members->Cfg->StateControl.Focused;
    else if (Members->MouseIsOver)
        cbc = &Members->Cfg->StateControl.Hover;

    renderer.WriteSingleLineText(0, 0, "[ ] ", cbc->TextColor, 4);

    WriteCharacterBufferParams params(
          WriteCharacterBufferFlags::OVERWRITE_COLORS | WriteCharacterBufferFlags::PROCESS_NEW_LINE |
          WriteCharacterBufferFlags::HIGHLIGHT_HOTKEY);
    params.HotKeyPosition = Members->HotKeyOffset;
    if (Members->Layout.Height == 1)
    {
        params.Color       = cbc->TextColor;
        params.HotKeyColor = cbc->HotKeyColor;
        params.Flags |= WriteCharacterBufferFlags::SINGLE_LINE;
    }
    else
    {
        params.Color       = cbc->TextColor;
        params.HotKeyColor = cbc->HotKeyColor;
        params.Flags |= WriteCharacterBufferFlags::MULTIPLE_LINES | WriteCharacterBufferFlags::WRAP_TO_WIDTH;
        params.Width = Members->Layout.Width - 4; // without the '[ ] ' characters
    }
    renderer.WriteCharacterBuffer(4, 0, Members->Text, params);

    if (IsChecked())
    {
        renderer.WriteCharacter(1, 0, 'X', cbc->StateSymbolColor);
    }
    if (Members->Focused)
        renderer.SetCursor(1, 0);
}
void AppCUI::Controls::CheckBox::OnHotKey()
{
    SetChecked(!IsChecked());
    RaiseEvent(Event::EVENT_CHECKED_STATUS_CHANGED);
}
bool AppCUI::Controls::CheckBox::OnKeyEvent(AppCUI::Input::Key KeyCode, char AsciiCode)
{
    if (KeyCode == Key::Space)
    {
        OnHotKey();
        return true;
    }
    return false;
}
void AppCUI::Controls::CheckBox::OnMouseReleased(int x, int y, AppCUI::Input::MouseButton button)
{
    if (IsMouseInControl(x, y))
        OnHotKey();
}
bool AppCUI::Controls::CheckBox::OnMouseEnter()
{
    return true;
}
bool AppCUI::Controls::CheckBox::OnMouseLeave()
{
    return true;
}
