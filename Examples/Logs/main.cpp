#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

/* ==============================================================
   | MAKE SURE THAT YOU ARE TESTING THIS IN DEBUG CONFIGURATION |
   ==============================================================
*/

class SimpleWin : public AppCUI::Controls::Window
{
    Label l1;

  public:
    SimpleWin()
    {
        LOG_INFO("Creating Simple Win");
        this->Create("Log Example", "d:c,w:40,h:10");
        l1.Create(this, "xxx", "xxxxx:1,y:1,w:36,h:5"); // this should produce an error as 'xxxxx:1' is invalid
    }
    bool OnEvent(Control* sender, Event eventType, int controlID) override
    {
        if (eventType == Event::EVENT_WINDOW_CLOSE)
        {
            LOG_INFO("Close button was pressed -> closing app");
            Application::Close();
            return true;
        }
        return false;
    }
};
int main()
{
    Log::ToFile("./appcuilogs.txt");

    // Window users can also use
    //    Log::ToOutputDebugString();

    // If you want to log to stderr, use
    //    Log::ToStdErr();
    // Example: Run 'Logs.exe 2>err_log.txt' after enable logging to stderr. This will write all logs to a file
    // (err_log.txt)
    if (!Application::Init())
        return 1;
    Application::AddWindow(new SimpleWin());
    Application::Run();
    return 0;
}
