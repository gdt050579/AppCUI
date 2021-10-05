#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

class SimpleWin : public AppCUI::Controls::Window
{
  public:
    SimpleWin() : Window("NumericSelectorSample", "d:c,w:40,h:10", WindowFlags::None)
    {
        auto n1 = Factory::NumericSelector::Create(this, 1, 5, 2, "x:1,y:1,w:36");
        auto n2 = Factory::NumericSelector::Create(this, -100, 1234567, 1234567, "x:1,y:3,w:15");
        auto n3 = Factory::NumericSelector::Create(this, 0, 20, 70, "x:1,y:5,w:36");
        auto n4 = Factory::NumericSelector::Create(this, 0, 100, 50, "x:1,y:7,w:36");

        n1->SetMinValue(-10);

        n2->SetValue(12345678);

        n3->SetMinValue(21);
        n3->SetMaxValue(300);

        n4->SetEnabled(false);
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
