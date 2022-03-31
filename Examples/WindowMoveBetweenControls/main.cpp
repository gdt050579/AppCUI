#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

class ExampleWin : public Window
{
  public:
    ExampleWin() : Window("Example", "d:c,w:80,h:24", WindowFlags::None)
    {
        Factory::Label::Create(
              this, "Use Up/Down/Left/Right or combine with CTRL to move between controls", "x:0,y:0,w:78");

        Factory::CheckBox::Create(this, "Checkbox 1", "x:1,y:2,w:14")->SetFocus();
        Factory::CheckBox::Create(this, "Checkbox 2", "x:1,y:3,w:14");
        Factory::CheckBox::Create(this, "Checkbox 3", "x:1,y:4,w:14");
        Factory::CheckBox::Create(this, "Checkbox 4", "x:1,y:5,w:14");
        Factory::CheckBox::Create(this, "Checkbox 5", "x:1,y:6,w:14");

        Factory::CheckBox::Create(this, "Checkbox 6", "x:20,y:2,w:14");
        Factory::CheckBox::Create(this, "Checkbox 7", "x:20,y:3,w:14");
        Factory::CheckBox::Create(this, "Checkbox 8", "x:20,y:4,w:14");
        Factory::Button::Create(this, "Button 1", "x:20,y:6,w:14", 100);

        Factory::Button::Create(this, "B2", "x:1,y:8,w:6", 100,ButtonFlags::Flat);
        Factory::Button::Create(this, "B3", "x:10,y:8,w:6", 100, ButtonFlags::Flat);
        Factory::Button::Create(this, "B4", "x:19,y:8,w:6", 100, ButtonFlags::Flat);
        Factory::Button::Create(this, "B5", "x:1,y:10,w:6", 100, ButtonFlags::Flat);
        Factory::Button::Create(this, "B6", "x:19,y:10,w:6", 100, ButtonFlags::Flat);

        Factory::ComboBox::Create(this, "x:10,y:12,w:22", "Value1,Value2,Value3");

        auto p = Factory::Panel::Create(this, "P1","x:40,y:1,w:36,h:15");
        Factory::CheckBox::Create(p, "CB1", "x:1,y:1,w:8");
        Factory::CheckBox::Create(p, "CB2", "x:1,y:2,w:8");
        Factory::CheckBox::Create(p, "CB3", "x:1,y:3,w:8");
        Factory::CheckBox::Create(p, "CB4", "x:10,y:1,w:8");
        Factory::CheckBox::Create(p, "CB5", "x:10,y:2,w:8");
        Factory::CheckBox::Create(p, "CB6", "x:10,y:3,w:8");
        Factory::Button::Create(p, "Button 2", "x:1,y:5,w:14", 100);

        auto p2 = Factory::Panel::Create(p, "P2", "x:1,y:7,w:20,h:6");
        Factory::Button::Create(p2, "B7", "x:1,y:1,w:8", 100, ButtonFlags::Flat);
        Factory::Button::Create(p2, "B8", "x:1,y:3,w:8", 100, ButtonFlags::Flat);
        Factory::Button::Create(p2, "B9", "x:10,y:1,w:7", 100, ButtonFlags::Flat);
        Factory::Button::Create(p2, "B10", "x:10,y:3,w:7", 100, ButtonFlags::Flat);

        auto p3 = Factory::Panel::Create(p, "P3", "x:22,y:1,w:12,h:6");
        Factory::Button::Create(p3, "B11", "x:1,y:1,w:8", 100, ButtonFlags::Flat);
        Factory::Button::Create(p3, "B12", "x:1,y:3,w:8", 100, ButtonFlags::Flat);

    }
};
int main()
{
    if (!Application::Init())
        return 1;
    Application::AddWindow(std::make_unique<ExampleWin>());
    Application::Run();
    return 0;
}
