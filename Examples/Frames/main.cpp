#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;
using namespace AppCUI::Utils;


class Ball: public UserControl
{
    int x, y, addX, addY;
  public:
    Ball() : UserControl("d:c")
    {
        x = y = 5;
        addX = addY = 1;
    }
    void Update()
    {
        x += addX;
        y += addY;
        if (x < 0)
        {
            x = 0;
            addX = 1;
        }
        if (x > this->GetWidth())
        {
            x = this->GetWidth();
            addX = -1;
        }
        if (y < 0)
        {
            y    = 0;
            addY = 1;
        }
        if (y > this->GetHeight())
        {
            y    = this->GetHeight();
            addY = -1;
        }
    }
    void Paint(AppCUI::Graphics::Renderer & r)
    {
        r.Clear(' ', ColorPair{ Color::White, Color::Black });
        r.WriteSpecialCharacter(x, y, SpecialChars::CircleFilled, ColorPair{ Color::Aqua, Color::Black });
    }
};
class FramesExample : public Window
{
    Reference<Ball> ball;
    int counter;
  public:
    FramesExample() : Window("Test", "d:c,w:40,h:10", WindowFlags::None), counter(0)
    {
        ball = this->CreateChildControl<Ball>();
    }
    bool OnFrameUpdate() override
    {
        counter++;
        if (counter == 30) /* 30 FPS */
        {
            ball->Update();
            counter = 0;
        }
        
        return true;
    }
};
int main()
{
    if (!Application::Init(InitializationFlags::EnableFPSMode))
        return 1;
    Application::AddWindow(std::make_unique<FramesExample>());
    Application::Run();
    return 0;
}
