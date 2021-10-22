#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Graphics;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

constexpr unsigned int StartButtonID = 0x0001;

class Tetris : public SingleApp
{
  private:
    Reference<Button> startButton = nullptr;

  public:
    Tetris()
    {
        startButton = Factory::Button::Create(this, "Start", "d:c,w:20", StartButtonID);
    }

    bool OnEvent(Control* ctrl, Event eventType, int controlID) override
    {
        if (SingleApp::OnEvent(ctrl, eventType, controlID))
        {
            return true;
        }

        if (eventType == Event::ButtonClicked)
        {
            if (controlID == StartButtonID)
            {
                AppCUI::Dialogs::MessageBox::ShowNotification("Info", "You have started the game!");
                return true;
            }
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
    if (Application::Init(InitializationFlags::SingleWindowApp) == false)
    {
        return 1;
    }

    Application::RunSingleApp(std::make_unique<Tetris>());

    return 0;
}
