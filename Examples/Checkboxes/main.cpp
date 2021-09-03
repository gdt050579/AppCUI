#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

#define A_CHECKBOX_ID 12345

class MyWin : public AppCUI::Controls::Window
{
    CheckBox c1, c2, c3;
    Label c3_info;

  public:
    MyWin()
    {
        this->Create("Checkbox example", "d:c,w:60,h:12");
        c1.Create(this, "A &regular checkbox with hot key", "x:1,y:1,w:56");
        c2.Create(this, "A inactive(disabled) checkbox", "x:1,y:2,w:56");
        c2.SetEnabled(false);
        c3.Create(
              this,
              "A &multi line checkbox that has an ID (defined as\n\rA_CHECKBOX_ID with value 12345) that can be used\nto "
              "see if the checkbox has been clicked",
              "x:1,y:4,w:56,h:4",
              A_CHECKBOX_ID);
        c3_info.Create(this, "3rd checkbox status: NOT CHECKED", "x:1,y:9,w:58");
    }
    bool OnEvent(Control* sender, Event eventType, int controlID) override
    {
        if (eventType == Event::EVENT_WINDOW_CLOSE)
        {
            Application::Close();
            return true;
        }
        if ((eventType == Event::EVENT_CHECKED_STATUS_CHANGED) && (controlID == A_CHECKBOX_ID))
        {
            if (c3.IsChecked())
                c3_info.SetText("3rd checkbox status: CHECKED");
            else
                c3_info.SetText("3rd checkbox status: NOT CHECKED");
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
