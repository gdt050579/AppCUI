#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;

int winID                = 1;
std::string_view tags[8] = { "Random", "General", "Mathematics", "Computer", "Logic", "Fast", "Wind", "Help" };

class WindowExample : public AppCUI::Controls::Window
{
    Button b;
    std::string my_name;

  public:
    WindowExample(std::string_view name)
    {
        this->Create(name, "d:c,w:40,h:10", WindowFlags::Sizeable);
        b.Create(this, "New win", "l:2,t:2,r:2,b:2", 1234);
        my_name = name;
        if (winID % 5 != 0)
            this->SetTag(tags[winID % 8], "bla bla bla");
    }
    bool OnEvent(Control* c, Event eventType, int id) override
    {
        if (Window::OnEvent(c, eventType, id))
            return true;
        if ((eventType == Event::ButtonClicked) && (id == 1234))
        {
            AppCUI::Utils::LocalString<128> tmp;
            tmp.Format("%s_%d", my_name.c_str(), winID++);
            Application::AddWindow(std::make_unique<WindowExample>(tmp.GetText()), this);
            return true;
        }
        return false;
    }
};
class MyDesktop : public Desktop
{
  public:
    bool OnEvent(Control*, Event eventType, int id) override
    {
        AppCUI::Utils::LocalString<128> tmp;
        if (eventType == Event::Command)
        {
            switch (id)
            {
            case 12345:
                tmp.Format("Win_%d", winID++);
                Application::AddWindow(std::make_unique<WindowExample>(tmp.GetText()));
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
    InitializationData initData;
    initData.CustomDesktop = new MyDesktop();
    initData.Flags         = InitializationFlags::Menu | InitializationFlags::AutoHotKeyForWindow;
    if (!Application::Init(initData))
        return 1;
    auto m = Application::AddMenu("&Windows");
    m->AddCommandItem("&New", 12345, Key::F1);
    m->AddSeparator();
    m->AddCommandItem("Arrange &horizontally", 12346);
    m->AddCommandItem("Arrange &vertically", 12347);
    m->AddCommandItem("&Cascade", 12348);
    m->AddCommandItem("&Fit", 12349, Key::F5);
    m->AddSeparator();
    m->AddCommandItem("E&xit", 12350, Key::F10);

    Application::Run();
    return 0;
}
