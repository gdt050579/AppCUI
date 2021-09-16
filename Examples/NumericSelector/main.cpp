#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

class SimpleWin : public AppCUI::Controls::Window
{
    NumericSelector n1;
    NumericSelector n2;
    NumericSelector n3;
    NumericSelector n4;

  public:
    SimpleWin()
    {
        this->Create("NumericSelectorSample", "d:c,w:40,h:10");

        n1.Create(this, 1, 5, 2, "x:1,y:1,w:36");
        n2.Create(this, -100, 1234567, 1234567, "x:1,y:3,w:15");
        n3.Create(this, 0, 20, 70, "x:1,y:5,w:36");
        n4.Create(this, 0, 100, 50, "x:1,y:7,w:36");

        n1.SetMinValue(-10);

        n2.SetValue(12345678);

        n3.SetMinValue(21);
        n3.SetMaxValue(300);

        n4.SetEnabled(false);
    }

    bool OnEvent(Control*, Event eventType, int) override
    {
        if (eventType == Event::WindowClose)
        {
            Application::Close();
            return true;
        }

        return false;
    }
};

int main()
{
    if (Application::Init() == false)
    {
        return 1;
    }

    Application::AddWindow(std::make_unique<SimpleWin>());
    Application::Run();

    return 0;
}
