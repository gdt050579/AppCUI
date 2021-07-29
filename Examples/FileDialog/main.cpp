#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Dialogs;

#define BUTTON_ID_SHOW_SAVE     1
#define BUTTON_ID_SHOW_OPEN     2


class MyWin : public AppCUI::Controls::Window
{
    Button b1, b2, b3, b4, b5;
public:
    MyWin()
    {
        this->Create("File example", "a:c,w:40,h:13");
        b1.Create(this, "&Save", "x:1,y:1,w:9", BUTTON_ID_SHOW_SAVE);
        b2.Create(this, "&Open", "x:1,y:3,w:9", BUTTON_ID_SHOW_OPEN);

    }
    bool OnEvent(const void* sender, Event eventType, int controlID) override
    {
        if (eventType == Event::EVENT_WINDOW_CLOSE)
        {
            Application::Close();
            return true;
        }
        if (eventType == Event::EVENT_BUTTON_CLICKED)
        {
            switch (controlID)
            {
                case BUTTON_ID_SHOW_SAVE: 
                    FileDialog::ShowOpenFileWindow("", "*.*", "E:\\");
                    break;
                case BUTTON_ID_SHOW_OPEN: 
                    break;
            }
            return true;
        }
        return false;
    }
};
int main()
{
    if (!Application::Init())
        return 1;
    Application::AddWindow(new MyWin());
    Application::Run();
    return 0;
}
