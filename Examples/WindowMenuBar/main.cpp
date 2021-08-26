#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;

class WindowMenuBarExample : public AppCUI::Controls::Window
{
  public:
    WindowMenuBarExample()
    {
        this->Create("Text", "a:c,w:40,h:10", WindowFlags::Menu);
        Menu* files = this->AddMenu("&File");
        files->AddCommandItem("&Save", 100, Key::Ctrl | Key::S);
        files->AddCommandItem("Save All", 101);
        files->AddCommandItem("&Open", 102, Key::Ctrl | Key::O);
        files->AddSeparator();
        files->AddCommandItem("E&xit", 103, Key::Ctrl | Key::Q);
        Menu* edit = this->AddMenu("&Edit");
        edit->AddCommandItem("&Copy", 200, Key::Ctrl | Key::Insert);
        edit->AddCommandItem("&paste", 201, Key::Shift | Key::Insert);
        edit->AddCommandItem("&Cut", 202, Key::Shift | Key::Delete);
        edit->AddCommandItem("Special copy", 203, Key::Ctrl | Key::Alt | Key::Shift | Key::Insert);
        Menu* help = this->AddMenu("&Help");
        help->AddCommandItem("&About", 300);
        help->AddCommandItem("Search &online", 301);
        help->AddSeparator();
        help->AddCommandItem("Check for &updates", 302);
    }
    bool OnEvent(const void* sender, Event eventType, int controlID) override
    {
        if (eventType == Event::EVENT_WINDOW_CLOSE)
        {
            Application::Close();
            return true;
        }
        return false;
    }
};
int main()
{
    if (!Application::Init())
        return 1;
    Application::AddWindow(new WindowMenuBarExample());
    Application::Run();
    return 0;
}
