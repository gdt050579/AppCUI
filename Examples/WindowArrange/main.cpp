#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;

#define COMMAND_ID_MAXIMIZE_ALL 1000
#define COMMAND_ID_CASCADE      1001
#define COMMAND_ID_VERTICAL     1002
#define COMMAND_ID_HORIZONTAL   1003
#define COMMAND_ID_GRID         1004

class MyWin : public Window
{
  public:
    MyWin(std::string_view name, char16 hotKey) : Window(name, "x:1,y:1,w:10,h:5", WindowFlags::Sizeable)
    {
        this->SetHotKey(hotKey);
    }
    bool OnUpdateCommandBar(CommandBar& cmd) override
    {
        cmd.SetCommand(Key::F1, "Maximize all", COMMAND_ID_MAXIMIZE_ALL);
        cmd.SetCommand(Key::F2, "Cascade", COMMAND_ID_CASCADE);
        cmd.SetCommand(Key::F3, "Vertical", COMMAND_ID_VERTICAL);
        cmd.SetCommand(Key::F4, "Horizontal", COMMAND_ID_HORIZONTAL);
        cmd.SetCommand(Key::F5, "Grid", COMMAND_ID_GRID);
        return true;
    }
    bool OnEvent(Reference<Control>, Event eventType, int controlID) override
    {
        if (eventType == Event::WindowClose)
        {
            Application::Close();
            return true;
        }
        if (eventType == Event::Command)
        {
            switch (controlID)
            {
            case COMMAND_ID_MAXIMIZE_ALL:
                Application::ArrangeWindows(ArangeWindowsMethod::MaximizedAll);
                break;
            case COMMAND_ID_CASCADE:
                Application::ArrangeWindows(ArangeWindowsMethod::Cascade);
                break;
            case COMMAND_ID_VERTICAL:
                Application::ArrangeWindows(ArangeWindowsMethod::Vertical);
                break;
            case COMMAND_ID_HORIZONTAL:
                Application::ArrangeWindows(ArangeWindowsMethod::Horizontal);
                break;
            case COMMAND_ID_GRID:
                Application::ArrangeWindows(ArangeWindowsMethod::Grid);
                break;
            }
        }
        return false;
    }
};
int main()
{
    if (!Application::Init(InitializationFlags::CommandBar))
        return 1;
    uint32 winCount = 10;
    char winName[]        = "Win xx";
    for (uint32 tr = 1; tr <= winCount; tr++)
    {
        winName[4]  = '0' + tr / 10;
        winName[5]  = '0' + tr % 10;
        char16 hk = 0;
        if (tr < 10)
            hk = '0' + tr;
        auto w = std::make_unique<MyWin>(winName, hk);
        if (tr == 1)
            w->SetTag("Data", "A window that contains\na lot of data ...");
        if (tr == 2)
            w->SetTag("Numbers", "Plenty of numbers ....");
        Application::AddWindow(std::move(w));
    }
    Application::Run();
    return 0;
}
