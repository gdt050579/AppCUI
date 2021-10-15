#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;


class LoginWin : public Window
{
  public:
    LoginWin() : Window("Log in", "d:c,w:40,h:10", WindowFlags::None)
    {
        Factory::Label::Create(this, "User name", "x:1,y:1,w:10,h:1");
        Factory::TextField::Create(this, "", "x:12,y:1,w:36");
        Factory::Label::Create(this, "Password", "x:1,y:3,w:10,h:1");
        Factory::Password::Create(this, "", "x:12,y:3,w:36");
        Factory::Button::Create("Log In", "b:0,l:5,r:5", 1234);
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