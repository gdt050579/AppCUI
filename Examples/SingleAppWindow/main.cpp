#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Graphics;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

class MySingleApp : public SingleApp
{
  public:
    MySingleApp()
    {
        Factory::Button::Create(this, "Push me", "d:c,w:20");
    }
    bool OnEvent(Control* ctrl, Event eventType, int controlID) override
    {
        if (SingleApp::OnEvent(ctrl, eventType, controlID))
            return true;
        if (eventType == Event::ButtonClicked)
        {
            AppCUI::Dialogs::MessageBox::ShowNotification("Info", "You have pushed the buttom !");
            return true;
        }
        return false;
    }
    void Paint(Renderer& r) override
    {
        r.Clear(' ', ColorPair{ Color::White, Color::DarkBlue });
    }
};

int main()
{
    if (!Application::Init(InitializationFlags::SingleWindowApp))
        return 1;
    // calling AddWindow on a SingleWindowApp AppCUI initialization will fail !
    Application::RunSingleApp(std::make_unique<MySingleApp>());
    return 0;
}
