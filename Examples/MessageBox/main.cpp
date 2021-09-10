#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Dialogs;

#define BUTTON_ID_SHOW_ERROR        1
#define BUTTON_ID_SHOW_NOTIFICATION 2
#define BUTTON_ID_SHOW_QUESTION     3
#define BUTTON_ID_SHOW_YNC_QUESTION 4
#define BUTTON_ID_SHOW_WARNING      5

class MyWin : public AppCUI::Controls::Window
{
    Button b1, b2, b3, b4, b5;

  public:
    MyWin()
    {
        this->Create("Message box example", "d:c,w:40,h:13");
        b1.Create(this, "Show an &error !", "x:1,y:1,w:36", BUTTON_ID_SHOW_ERROR);
        b2.Create(this, "Show an &notification", "x:1,y:3,w:36", BUTTON_ID_SHOW_NOTIFICATION);
        b3.Create(this, "Ask a &question", "x:1,y:5,w:36", BUTTON_ID_SHOW_QUESTION);
        b4.Create(this, "Ask a &Yes/No/Cancel question", "x:1,y:7,w:36", BUTTON_ID_SHOW_YNC_QUESTION);
        b5.Create(this, "Show a &warning !", "x:1,y:9,w:36", BUTTON_ID_SHOW_WARNING);
    }
    bool OnEvent(Control*, Event eventType, int controlID) override
    {
        if (eventType == Event::WindowClose)
        {
            Application::Close();
            return true;
        }
        if (eventType == Event::ButtonClicked)
        {
            switch (controlID)
            {
            case BUTTON_ID_SHOW_ERROR:
                MessageBox::ShowError("Error", "This is an error message example !");
                break;
            case BUTTON_ID_SHOW_NOTIFICATION:
                MessageBox::ShowNotification("Notification", "This is a notification message example !");
                break;
            case BUTTON_ID_SHOW_QUESTION:
                MessageBox::ShowOkCancel("Question", "Are you ok with this ?");
                break;
            case BUTTON_ID_SHOW_YNC_QUESTION:
                MessageBox::ShowYesNoCancel("Question", "Are you ok with this ?");
                break;
            case BUTTON_ID_SHOW_WARNING:
                MessageBox::ShowWarning("Warning", "This is a warning message example !");
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
