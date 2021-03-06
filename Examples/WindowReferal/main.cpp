#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;

int winID                     = 1;
constexpr auto tagsNo         = 8U;
constexpr auto buttonNewWinID = 1234;

std::string_view tags[tagsNo] = { "Random", "General", "Mathematics", "Computer", "Logic", "Fast", "Wind", "Help" };

class WindowExample : public Window
{
    std::string my_name;

  public:
    WindowExample(std::string_view name) : Window(name, "d:c,w:40,h:10", WindowFlags::Sizeable)
    {
        Factory::Button::Create(this, "New win", "l:2,t:2,r:2,b:2", buttonNewWinID);
        my_name = name;
        if (winID % 5 != 0)
            this->SetTag(tags[winID % tagsNo], "bla bla bla");
    }

    bool OnEvent(Reference<Control> c, Event eventType, int id) override
    {
        if (Window::OnEvent(c, eventType, id))
            return true;
        if ((eventType == Event::ButtonClicked) && (id == buttonNewWinID))
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
    bool OnEvent(Reference<Control>, Event eventType, int id) override
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
                Application::ArrangeWindows(ArrangeWindowsMethod::Horizontal);
                return true;
            case 12347:
                Application::ArrangeWindows(ArrangeWindowsMethod::Vertical);
                return true;
            case 12348:
                Application::ArrangeWindows(ArrangeWindowsMethod::Cascade);
                return true;
            case 12349:
                Application::ArrangeWindows(ArrangeWindowsMethod::Grid);
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
    initData.CustomDesktopConstructor = []() { return (Desktop*) (new MyDesktop()); };
    initData.Flags                    = InitializationFlags::Menu | InitializationFlags::AutoHotKeyForWindow |
                     InitializationFlags::DisableAutoCloseDesktop;
    if (Application::Init(initData) == false)
    {
        return 1;
    }

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
