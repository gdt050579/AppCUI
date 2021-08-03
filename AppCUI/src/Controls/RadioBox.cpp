#include "ControlContext.hpp"

using namespace AppCUI::Controls;
using namespace AppCUI::Console;
using namespace AppCUI::Input;

bool AppCUI::Controls::RadioBox::Create(Control* parent, const AppCUI::Utils::ConstString& caption, const char* layout, int groupID, int controlID)
{
    CONTROL_INIT_CONTEXT(ControlContext);
    CREATE_CONTROL_CONTEXT(this, Members, false);
    Members->Layout.MinWidth  = 5;
    Members->Layout.MinHeight = 1;
    CHECK(Init(parent, caption, layout, true), false, "Unable to create radioBox box !");
    Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
    SetControlID(controlID);
    SetGroup(groupID);
    return true;
}
void AppCUI::Controls::RadioBox::Paint(Console::Renderer& renderer)
{
    CREATE_CONTROL_CONTEXT(this, Members, );

    auto* cbc = &Members->Cfg->StateControl.Normal;
    if (!this->IsEnabled())
        cbc = &Members->Cfg->StateControl.Inactive;
    if (Members->Focused)
        cbc = &Members->Cfg->StateControl.Focused;
    else if (Members->MouseIsOver)
        cbc = &Members->Cfg->StateControl.Hover;
    renderer.WriteSingleLineText(0, 0, "( ) ", cbc->TextColor, 4);

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
        params.Flags |= WriteCharacterBufferFlags::MULTIPLE_LINES;
    }
    renderer.WriteCharacterBuffer(4, 0, Members->Text, params);

    if (IsChecked())
        renderer.WriteSpecialCharacter(1, 0, SpecialChars::CircleFilled, cbc->StateSymbolColor);
    if (Members->Focused)
        renderer.SetCursor(1, 0);
}
void AppCUI::Controls::RadioBox::OnHotKey()
{
    CREATE_CONTROL_CONTEXT(this, Members, );
    Control* parent = Members->Parent;
    int id          = this->GetGroup();
    if ((parent) && (id > 0))
    {
        CREATE_CONTROL_CONTEXT(parent, pMembers, );
        for (unsigned int tr = 0; tr < pMembers->ControlsCount; tr++)
        {
            if ((pMembers->Controls[tr]) && (pMembers->Controls[tr]->GetGroup() == id))
                pMembers->Controls[tr]->SetChecked(false);
        }
    }
    if (IsChecked() == false)
    {
        SetChecked(true);
        RaiseEvent(Event::EVENT_CHECKED_STATUS_CHANGED);
    }
}
bool AppCUI::Controls::RadioBox::OnKeyEvent(AppCUI::Input::Key KeyCode, char AsciiCode)
{
    if (KeyCode == Key::Space)
    {
        OnHotKey();
        return true;
    }
    return false;
}
void AppCUI::Controls::RadioBox::OnMouseReleased(int x, int y, int butonState)
{
    if (IsMouseInControl(x, y))
        OnHotKey();
}
bool AppCUI::Controls::RadioBox::OnMouseEnter()
{
    return true;
}
bool AppCUI::Controls::RadioBox::OnMouseLeave()
{
    return true;
}
