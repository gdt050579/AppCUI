#include "../ControlContext.hpp"
#include <AppCUI.hpp>

using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;
using namespace AppCUI::Input;

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

void Desktop::Paint(AppCUI::Graphics::Renderer& renderer)
{
    CREATE_TYPECONTROL_CONTEXT(ControlContext, Members, );
    renderer.ClearWithSpecialChar(SpecialChars::Block50, Members->Cfg->Desktop.Color);
}
bool Desktop::OnKeyEvent(AppCUI::Input::Key keyCode, char16_t /*UnicodeChar*/)
{
    CREATE_TYPECONTROL_CONTEXT(ControlContext, Members, false);
    switch (keyCode)
    {
    case Key::Escape:
        AppCUI::Application::Close();
        return true;
    case Key::Ctrl | Key::Tab:
        GoToNextWindow(Members, 1);
        return true;
    case Key::Ctrl | Key::Shift | Key::Tab:
        GoToNextWindow(Members, -1);
        return true;
    case Key::Alt | Key::N0:
        AppCUI::Dialogs::WindowManager::Show();
        return true;
    }
    // check controls hot keys
    if ((((unsigned int) keyCode) & (unsigned int) (Key::Shift | Key::Alt | Key::Ctrl)) == ((unsigned int) Key::Alt))
    {
        auto* b = Members->Controls;
        auto* e = b + Members->ControlsCount;
        if (b)
        {
            while (b < e)
            {
                const auto winMembers = reinterpret_cast<ControlContext*>((*b)->Context);
                if ((winMembers) && (winMembers->HotKey == keyCode))
                {
                    if ((b - Members->Controls) != (size_t) Members->CurrentControlIndex)
                        (*b)->SetFocus();
                    return true;
                }
                b++;
            }
        }
    }
    return false;
}
