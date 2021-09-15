#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;

class WindowExample : public AppCUI::Controls::Window
{
  public:
    WindowExample()
    {
        this->Create("Text", "d:c,w:40,h:10", WindowFlags::Menu);
    }
};
class MyDesktop: public Desktop
{
  public:
    bool OnEvent(Control*, Event eventType, int  id) override
    {
        if (eventType == Event::Command)
        {
            switch (id)
            {
            case 12345:
                return true;
            case 12346:
                Application::ArrangeWindows(ArangeWindowsMethod::Horizontal);
                return true;
            case 12347:
                Application::ArrangeWindows(ArangeWindowsMethod::Vertical);
                return true;
            case 12348:
                Application::ArrangeWindows(ArangeWindowsMethod::Cascade);
                return true;
            case 12349:
                Application::ArrangeWindows(ArangeWindowsMethod::Grid);
                return true;
            case 12350:
                Application::Close();
                return true;
            }
        }
        return false;
    }
};
int main()
{
    if (!Application::Init(InitializationFlags::Menu))
        return 1;
    auto m = Application::AddMenu("Windows");
    m->AddCommandItem("&New", 12345);
    m->AddSeparator();
    m->AddCommandItem("Arrange &horizontally", 12346);
    m->AddCommandItem("Arrange &vertically", 12347);
    m->AddCommandItem("&Cascade", 12348);
    m->AddCommandItem("&Fit", 12349);
    m->AddSeparator();
    m->AddCommandItem("E&xit", 12350);
    
    Application::Run();
    return 0;
}
