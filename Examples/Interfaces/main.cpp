#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;
using namespace AppCUI::Input;

class MyWin : public Window, public Handlers::OnButtonPressedInterface, public Handlers::PaintControlInterface
{
  public:
    MyWin() : Window("Test", "d:c,w:40,h:10", WindowFlags::None)
    {
        auto b = this->CreateChildControl<Button>("Push Me", "x:1,y:1,w:10", 123, ButtonFlags::None);
        b->Handlers()->OnButtonPressed = this;
        auto p                         = this->CreateChildControl<Panel>("","x:12,y:1,w:5,h:2");
        p->Handlers()->PaintControl    = this;
    }
    void OnButtonPressed(Reference<Button> r)
    {
        Dialogs::MessageBox::ShowNotification("Button", "This button was pressed !");
    };
    void PaintControl(Reference<Control> c, Renderer& r)
    {
        r.Clear('X', ColorPair{ Color::Red, Color::Black });
    };
};
int main()
{
    if (!Application::Init())
        return 1;

    Application::AddWindow(std::make_unique<MyWin>());
    Application::Run();
    return 0;
}
