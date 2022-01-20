#include "ControlContext.hpp"

namespace AppCUI::Controls
{
using namespace Controls;
using namespace Graphics;
using namespace Input;

RadioBox::RadioBox(const ConstString& caption, string_view layout, int groupID, int controlID)
    : Control(new ControlContext(), caption, layout, true)
{
    auto Members              = reinterpret_cast<ControlContext*>(this->Context);
    Members->Layout.MinWidth  = 5;
    Members->Layout.MinHeight = 1;
    Members->Flags            = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
    this->SetControlID(controlID);
    this->SetGroup(groupID);
}

void RadioBox::Paint(Graphics::Renderer& renderer)
{
    CREATE_CONTROL_CONTEXT(this, Members, );

    const ColorPair colHK = Members->Flags & GATTR_ENABLE ? Members->Cfg->Text.HotKey : Members->Cfg->Text.Inactive;
    ColorPair colTxt;
    if (!this->IsEnabled())
        colTxt = Members->Cfg->Text.Inactive;
    else if (Members->Focused)
        colTxt = Members->Cfg->Text.Focused;
    else if (Members->MouseIsOver)
        colTxt = Members->Cfg->Text.Hovered;
    else
        colTxt = Members->Cfg->Text.Normal;
    renderer.WriteSingleLineText(0, 0, "( ) ", colTxt);

    WriteTextParams params(WriteTextFlags::OverwriteColors | WriteTextFlags::HighlightHotKey);
    params.HotKeyPosition = Members->HotKeyOffset;
    params.X              = 4;
    params.Y              = 0;
    if (Members->Layout.Height == 1)
    {
        params.Color       = colTxt;
        params.HotKeyColor = colHK;
        params.Flags |= WriteTextFlags::SingleLine;
    }
    else
    {
        params.Color       = colTxt;
        params.HotKeyColor = colHK;
        params.Flags |= WriteTextFlags::MultipleLines | WriteTextFlags::WrapToWidth;
        params.Width = Members->Layout.Width - 4; // without the '( ) ' characters
    }
    renderer.WriteText(Members->Text, params);

    if (IsChecked())
    {
        const auto col = (Members->Flags & GATTR_ENABLE) ? Members->Cfg->Symbol.Checked : Members->Cfg->Symbol.Inactive;
        renderer.WriteSpecialCharacter(1, 0, SpecialChars::CircleFilled, col);
    }
    if (Members->Focused)
        renderer.SetCursor(1, 0);
}
void RadioBox::OnHotKey()
{
    CREATE_CONTROL_CONTEXT(this, Members, );
    Control* parent = Members->Parent;
    int id          = this->GetGroup();
    if ((parent) && (id > 0))
    {
        CREATE_CONTROL_CONTEXT(parent, pMembers, );
        for (uint32 tr = 0; tr < pMembers->ControlsCount; tr++)
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
bool RadioBox::OnKeyEvent(Input::Key KeyCode, char16)
{
    if (KeyCode == Key::Space)
    {
        OnHotKey();
        return true;
    }
    return false;
}
void RadioBox::OnMouseReleased(int x, int y, Input::MouseButton)
{
    if (IsMouseInControl(x, y))
        OnHotKey();
}
bool RadioBox::OnMouseEnter()
{
    CREATE_CONTROL_CONTEXT(this, Members, false);
    if ((int) Members->Text.Len() >= Members->Layout.Width)
        this->ShowToolTip(Members->Text);
    return true;
}
bool RadioBox::OnMouseLeave()
{
    return true;
}
// handlers covariant
Handlers::CheckState* RadioBox::Handlers()
{
    GET_CONTROL_HANDLERS(Handlers::CheckState);
}
} // namespace AppCUI::Controls