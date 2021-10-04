#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

#define A_CHECKBOX_ID 12345

class MyWin : public Window
{
    Label * lb;
    CheckBox* c3;
  public:
    MyWin() : Window("Checkbox example", "d:c,w:60,h:12", WindowFlags::None)
    {
        Factory::CheckBox::Create(this, "A &regular checkbox with hot key", "x:1,y:1,w:56");
        Factory::CheckBox::Create(this, "A inactive(disabled) checkbox", "x:1,y:2,w:56")->SetEnabled(false);
        c3 = Factory::CheckBox::Create(
              this,
              "A &multi line checkbox that has an ID (defined as\n\rA_CHECKBOX_ID with value 12345) that can be used\nto "
              "see if the checkbox has been clicked",
              "x:1,y:4,w:56,h:4",
              A_CHECKBOX_ID);
        lb = Factory::Label::Create(this, "3rd checkbox status: NOT CHECKED", "x:1,y:9,w:58");
    }
    bool OnEvent(Control*, Event eventType, int controlID) override
    {
        if (eventType == Event::WindowClose)
        {
            Application::Close();
            return true;
        }
        if ((eventType == Event::CheckedStatusChanged) && (controlID == A_CHECKBOX_ID))
        {
            if (c3->IsChecked())
                lb->SetText("3rd checkbox status: CHECKED");
            else
                lb->SetText("3rd checkbox status: NOT CHECKED");
        }
        return false;
    }
};
int main()
{
    if (!Application::Init())
        return 1;
    Application::AddWindow(std::make_unique<MyWin>());
    Application::Run();
    return 0;
}
