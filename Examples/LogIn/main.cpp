#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

class LoginWin : public Window
{
    Reference<Password> pwd;

  public:
    LoginWin() : Window("Log in", "d:c,w:40,h:9", WindowFlags::None)
    {
        Factory::TextField::Create(this, "", "x:12,y:1,w:25");
        Factory::Label::Create(this, "User name", "x:1,y:1,w:10,h:1");
        pwd = Factory::Password::Create(this, "", "x:12,y:3,w:25");
        Factory::Label::Create(this, "Password", "x:1,y:3,w:10,h:1");
        Factory::Button::Create(this, "Log In", "b:0,l:5,r:5", 1234);
    }
    bool OnEvent(Reference<Control> ctrl, Event eventType, int controlID) override
    {
        if (Window::OnEvent(ctrl, eventType, controlID))
            return true;
        if ((eventType == Event::PasswordValidate) || (eventType == Event::ButtonClicked))
        {
            auto pass = (std::string) pwd->GetText();
            if (pass == "1234")
            {
                AppCUI::Dialogs::MessageBox::ShowNotification("Log in", "Correct password !");
                AppCUI::Application::Close();
            }
            else
            {
                AppCUI::Dialogs::MessageBox::ShowNotification("Log in", "Invalid password - try again !");
                pwd->SetText("");
                pwd->SetFocus();
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
    Application::AddWindow(std::make_unique<LoginWin>());
    Application::Run();
    return 0;
}