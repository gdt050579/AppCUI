#include "AppCUI.hpp"

namespace AppCUI::Dialogs
{
using namespace Controls;

enum class MessageBoxButtons
{
    Ok,
    OkCancel,
    YesNoCancel
};

bool MessageBoxWindowEventHandler(Reference<Control> control, Controls::Event eventType, int controlID)
{
    switch (eventType)
    {
    case Event::WindowClose:
        control.DownCast<Window>()->Exit(Result::Cancel);
        return true;
    case Event::ButtonClicked:
        control.DownCast<Window>()->Exit(controlID);
        return true;
    default:
        return false;
    }
    return false;
}

Result CreateMessageBoxWindow(
      const ConstString& title, const ConstString& content, WindowFlags flags, MessageBoxButtons buttonsType)
{
    auto wnd = Factory::Window::Create(title, "d:c,w:60,h:10", flags);

    switch (buttonsType)
    {
    case MessageBoxButtons::Ok:
        Factory::Button::Create(wnd, "&Ok", "x:23,y:6,w:15", (int) Result::Ok)->SetFocus();
        break;
    case MessageBoxButtons::OkCancel:
        Factory::Button::Create(wnd, "&Cancel", "x:31,y:6,w:15", (int) Result::Cancel);
        Factory::Button::Create(wnd, "&Ok", "x:15,y:6,w:15", (int) Result::Ok)->SetFocus();
        break;
    case MessageBoxButtons::YesNoCancel:
        Factory::Button::Create(wnd, "&No", "x:23,y:6,w:15", (int) Result::No);
        Factory::Button::Create(wnd, "&Cancel", "x:39,y:6,w:15", (int) Result::Cancel);
        Factory::Button::Create(wnd, "&Yes", "x:7,y:6,w:15", (int) Result::Yes)->SetFocus();
        break;
    }
    Factory::Label::Create(wnd, content, "x:1,y:1,w:56,h:3");
    wnd->Handlers()->OnEvent = MessageBoxWindowEventHandler;
    return static_cast<Result>(wnd->Show());
}
void MessageBox::ShowError(const ConstString& title, const ConstString& message)
{
    CreateMessageBoxWindow(title, message, WindowFlags::ErrorWindow, MessageBoxButtons::Ok);
}
void MessageBox::ShowNotification(const ConstString& title, const ConstString& message)
{
    CreateMessageBoxWindow(title, message, WindowFlags::NotifyWindow, MessageBoxButtons::Ok);
}
void MessageBox::ShowWarning(const ConstString& title, const ConstString& message)
{
    CreateMessageBoxWindow(title, message, WindowFlags::WarningWindow, MessageBoxButtons::Ok);
}
Result MessageBox::ShowYesNoCancel(const ConstString& title, const ConstString& message)
{
    return CreateMessageBoxWindow(title, message, WindowFlags::NotifyWindow, MessageBoxButtons::YesNoCancel);
}
Result MessageBox::ShowOkCancel(const ConstString& title, const ConstString& message)
{
    return CreateMessageBoxWindow(title, message, WindowFlags::NotifyWindow, MessageBoxButtons::OkCancel);
}
} // namespace AppCUI::Dialogs