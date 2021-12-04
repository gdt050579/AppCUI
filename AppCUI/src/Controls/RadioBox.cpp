#include "ControlContext.hpp"

using namespace Controls;
using namespace Graphics;
using namespace Input;

RadioBox::RadioBox(
      const Utils::ConstString& caption, std::string_view layout, int groupID, int controlID)
    : Control(new ControlContext(), caption, layout, true)
{
    auto Members              = reinterpret_cast<ControlContext*>(this->Context);
    Members->Layout.MinWidth  = 5;
    Members->Layout.MinHeight = 1;    
    Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
    this->SetControlID(controlID);
    this->SetGroup(groupID);
}


void Controls::RadioBox::Paint(Graphics::Renderer& renderer)
{
    CREATE_CONTROL_CONTEXT(this, Members, );

    auto* cbc = &Members->Cfg->StateControl.Normal;
    if (!this->IsEnabled())
        cbc = &Members->Cfg->StateControl.Inactive;
    if (Members->Focused)
        cbc = &Members->Cfg->StateControl.Focused;
    else if (Members->MouseIsOver)
        cbc = &Members->Cfg->StateControl.Hover;
    renderer.WriteSingleLineText(0, 0, "( ) ", cbc->TextColor);

    WriteTextParams params(WriteTextFlags::OverwriteColors | WriteTextFlags::HighlightHotKey);
    params.HotKeyPosition = Members->HotKeyOffset;
    params.X              = 4;
    params.Y              = 0;
    if (Members->Layout.Height == 1)
    {
        params.Color       = cbc->TextColor;
        params.HotKeyColor = cbc->HotKeyColor;
        params.Flags |= WriteTextFlags::SingleLine;
    }
    else
    {
        params.Color       = cbc->TextColor;
        params.HotKeyColor = cbc->HotKeyColor;
        params.Flags |= WriteTextFlags::MultipleLines | WriteTextFlags::WrapToWidth;
        params.Width = Members->Layout.Width - 4; // without the '( ) ' characters
    }
    renderer.WriteText(Members->Text, params);

    if (IsChecked())
        renderer.WriteSpecialCharacter(1, 0, SpecialChars::CircleFilled, cbc->StateSymbolColor);
    if (Members->Focused)
        renderer.SetCursor(1, 0);
}
void Controls::RadioBox::OnHotKey()
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
        CREATE_CONTROL_CONTEXT(this, Members, );
        if (Members->handlers)
        {
            auto ch = this->Handlers();
            if (ch->OnCheck.obj)
            {
                ch->OnCheck.obj->OnCheck(this, true);
                return;
            }
        }
        RaiseEvent(Event::CheckedStatusChanged);
    }
}
bool Controls::RadioBox::OnKeyEvent(Input::Key KeyCode, char16_t)
{
    if (KeyCode == Key::Space)
    {
        OnHotKey();
        return true;
    }
    return false;
}
void Controls::RadioBox::OnMouseReleased(int x, int y, Input::MouseButton)
{
    if (IsMouseInControl(x, y))
        OnHotKey();
}
bool Controls::RadioBox::OnMouseEnter()
{
    CREATE_CONTROL_CONTEXT(this, Members, false);
    if ((int) Members->Text.Len() >= Members->Layout.Width)
        this->ShowToolTip(Members->Text);
    return true;
}
bool Controls::RadioBox::OnMouseLeave()
{
    return true;
}
// handlers covariant
Handlers::CheckState* RadioBox::Handlers()
{
    GET_CONTROL_HANDLERS(Handlers::CheckState);
}