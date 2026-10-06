// Sample of docs/source/getting_started.rst - compiled with the examples (docs/snippets/CMakeLists.txt).
// [window]
#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

class GreetingWindow : public Window
{
    static constexpr int BUTTON_GREET = 1;
    Reference<TextField> name;

  public:
    GreetingWindow() : Window("Hello", "d:c,w:50,h:8", WindowFlags::None)
    {
        Factory::Label::Create(this, "Name", "x:1,y:1,w:6");
        name = Factory::TextField::Create(this, "", "x:8,y:1,w:39");
        Factory::Button::Create(this, "&Greet", "x:50%,y:4,w:20,a:t", BUTTON_GREET);
        name->SetFocus();
    }
    bool OnEvent(Reference<Control> sender, Event eventType, int controlID) override
    {
        if ((eventType == Event::ButtonClicked) && (controlID == BUTTON_GREET))
        {
            std::string text;
            name->GetText().ToString(text);
            text = "Hello, " + (text.empty() ? std::string("stranger") : text) + "!";
            Dialogs::MessageBox::ShowNotification("Greeting", std::string_view(text));
            return true;
        }
        if (eventType == Event::WindowClose)
        {
            Application::Close(); // closing the window ends the application
            return true;
        }
        return Window::OnEvent(sender, eventType, controlID);
    }
};

int main()
{
    if (!Application::Init())
        return 1;
    Application::AddWindow(std::make_unique<GreetingWindow>());
    Application::Run();
    return 0;
}
// [/window]
