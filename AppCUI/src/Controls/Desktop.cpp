﻿#include "ControlContext.hpp"

namespace AppCUI::Controls
{
void GoToNextWindow(ControlContext* Members, int direction)
{
    if (!Members)
        return;
    if ((Members->ControlsCount == 0) || (Members->Controls == nullptr))
        return;

    int start = Members->CurrentControlIndex;
    if (start < 0)
    {
        if (direction > 0)
            start = -1;
        else
            start = Members->ControlsCount;
    }
    start += direction;
    if (start >= (int) Members->ControlsCount)
        start = 0;
    if (start < 0)
        start = ((int) Members->ControlsCount) - 1;
    if (start != (int) Members->CurrentControlIndex)
        Members->Controls[start]->SetFocus();
}
Desktop::Desktop() : Control(new ControlContext(), "", "x:0,y:0,w:1,h:1", false)
{
    auto Members   = reinterpret_cast<ControlContext*>(this->Context);
    Members->Flags = GATTR_ENABLE | GATTR_VISIBLE | GATTR_TABSTOP;
}

void Desktop::Paint(Graphics::Renderer& renderer)
{
    CREATE_TYPECONTROL_CONTEXT(ControlContext, Members, );
    renderer.ClearWithSpecialChar(SpecialChars::Block50, Members->Cfg->Symbol.Desktop);
}
bool Desktop::OnKeyEvent(Input::Key keyCode, char16 /*UnicodeChar*/)
{
    CREATE_TYPECONTROL_CONTEXT(ControlContext, Members, false);
    switch (keyCode)
    {
    case Key::Escape:
        Application::Close();
        return true;
    case Key::Ctrl | Key::Tab:
        GoToNextWindow(Members, 1);
        return true;
    case Key::Ctrl | Key::Shift | Key::Tab:
        GoToNextWindow(Members, -1);
        return true;
    case Key::Alt | Key::N0:
        Dialogs::WindowManager::Show();
        return true;
    }
    // check controls hot keys
    if ((((uint32) keyCode) & (uint32) (Key::Shift | Key::Alt | Key::Ctrl)) == ((uint32) Key::Alt))
    {
        auto* b        = Members->Controls;
        auto* e        = b + Members->ControlsCount;
        uint32 ctrlIdx = 0U;
        if (b)
        {
            while (b < e)
            {
                const auto winMembers = reinterpret_cast<ControlContext*>((*b)->Context);
                if ((winMembers) && (winMembers->HotKey == keyCode))
                {
                    if (ctrlIdx != Members->CurrentControlIndex)
                        (*b)->SetFocus();
                    return true;
                }
                b++;
                ctrlIdx++;
            }
        }
    }
    return false;
}

} // namespace AppCUI::Controls