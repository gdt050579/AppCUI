#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;
using namespace AppCUI::Input;

// No class method
int main()
{
    if (!Application::Init())
        return 1;
    auto wnd                         = Factory::Window::Create("Test", "d:c,w:40,h:10");
    
    // a handler to handle key event for wnd
    wnd->Handlers()->OnKeyEvent = [](Reference<Control> c, Key keyCode, char16_t unicodeChar)
    {
        if (keyCode == Key::Backspace)
        {
            Dialogs::MessageBox::ShowNotification("Info", "'Backspace' was pushed !");
            return true;
        };
        return false;
    };

    auto btn                         = Factory::Button::Create(wnd, "Push Me", "x:1,y:1,w:10", 123);
    // a handler to handle button click
    btn->Handlers()->OnButtonPressed = [](Reference<Button> r) { 
        Dialogs::MessageBox::ShowNotification("Button", "This button was pressed !"); 
    };

    auto p                      = Factory::Panel::Create(wnd,"x:12,y:1,w:5,h:2");
    // a handler to handle paint event
    p->Handlers()->PaintControl = [](Reference<Control> c, Renderer& r) {
        r.Clear('X', ColorPair{ Color::Red, Color::Black });
    };

    auto btn_exit = Factory::Button::Create(wnd, "Exit", "x:1,y:4,w:10", 1000);
    // a handler to handle OnEvent messages
    wnd->Handlers()->OnEvent = [](Reference<Control> c, Event eventType, int ID)
    {
        if (c->OnEvent(c, eventType, ID))
            return true;
        if ((eventType == Event::ButtonClicked) && (ID == 1000))
        {
            Application::Close();
            return true;
        }
        return false;
    };

    auto cb = Factory::CheckBox::Create(wnd, "I am not checked !", "x:1,y:7,w:30", 1234);
    // a handler to handle check state events
    cb->Handlers()->OnCheck = [](Reference<Control> c, bool value) {
        if (value)
            c->SetText("I am checked !");
        else
            c->SetText("I am not checked !");
    };

    Application::AddWindow(std::move(wnd));
    Application::Run();
    return 0;
}
