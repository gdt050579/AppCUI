#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;


class ExampleWin : public Window
{
    Reference<Button> b;
    Reference<ColorPicker> c;
  public:
    ExampleWin() : Window("Example", "d:c,w:40,h:10", WindowFlags::Sizeable)
    {
        c = Factory::ColorPicker::Create(this, "l:1,t:1,r:1", Graphics::Color::DarkGreen);
        b = Factory::Button::Create(this, "A button", "d:b,w:20", 123);
    }
    bool OnEvent(Reference<Control> sender, Event eventType, int id) override
    {
        if (Window::OnEvent(sender, eventType, id))
            return true;
        if (eventType == Event::ColorPickerSelectedColorChanged)
        {
            b->SetText(ColorUtils::GetColorName(c->GetColor()));
            return true;
        }
        return false;
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
