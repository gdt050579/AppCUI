#include "AppCUI.h"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

#define BUTTON_ID_SHOW_ERROR        1
#define BUTTON_ID_SHOW_NOTIFICATION 2
#define BUTTON_ID_SHOW_QUESTION     3
#define BUTTON_ID_SHOW_YNC_QUESTION 4
#define BUTTON_ID_SHOW_WARNING      5

class MyWin : public AppCUI::Controls::Window
{
    Button b1,b2,b3,b4,b5;
public:
    MyWin()
    {
        this->Create("Message box example", "a:c,w:40,h:13");
        b1.Create(this, "Show an &error !", "x:1,y:1,w:36", BUTTON_ID_SHOW_ERROR);
        b2.Create(this, "Show an &notification", "x:1,y:3,w:36", BUTTON_ID_SHOW_NOTIFICATION);
        b3.Create(this, "Ask a &question", "x:1,y:5,w:36", BUTTON_ID_SHOW_QUESTION);
        b4.Create(this, "Ask a &Yes/No/Cancel question", "x:1,y:7,w:36", BUTTON_ID_SHOW_YNC_QUESTION);
        b5.Create(this, "Show a &warning !", "x:1,y:9,w:36", BUTTON_ID_SHOW_WARNING);
    }
    bool OnEvent(const void* sender, Event::Type eventType, int controlID) override
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
                case BUTTON_ID_SHOW_ERROR: MessageBox::ShowError("Error", "This is an error message example !"); break;
                case BUTTON_ID_SHOW_NOTIFICATION: MessageBox::ShowNotification("Notification", "This is a notification message example !"); break;
                case BUTTON_ID_SHOW_QUESTION: MessageBox::ShowOkCancel("Question", "Are you ok with this ?"); break;
                case BUTTON_ID_SHOW_YNC_QUESTION: MessageBox::ShowYesNoCancel("Question", "Are you ok with this ?"); break;
                case BUTTON_ID_SHOW_WARNING: MessageBox::ShowWarning("Warning", "This is a warning message example !"); break;
            }
            return true;
        }
        return false;
    }
};
int main()
{
    Application::Init();
    Application::AddWindow(new MyWin());
    Application::Run();
    return 0;
}
