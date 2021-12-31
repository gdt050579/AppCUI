#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;
using namespace AppCUI::Input;

class MyWin : public Window,
              public Handlers::OnButtonPressedInterface,
              public Handlers::PaintControlInterface,
              public Handlers::OnCheckInterface,
              public Handlers::OnFocusInterface,
              public Handlers::OnLoseFocusInterface
{
    Reference<Button> b;

  public:
    MyWin() : Window("Test", "d:c,w:40,h:10", WindowFlags::None)
    {
        b = this->CreateChildControl<Button>("Push Me", "x:1,y:1,w:10", 123, ButtonFlags::None);
        b->Handlers()->OnButtonPressed = this;
        b->Handlers()->OnFocus         = this;
        b->Handlers()->OnLoseFocus     = this;
        auto p                         = this->CreateChildControl<Panel>("", "x:12,y:1,w:5,h:2");
        p->Handlers()->PaintControl    = this;
        auto cb                        = this->CreateChildControl<CheckBox>("I am not checked", "x:1,y:3,w:30", 123);
        cb->Handlers()->OnCheck        = this;
    }
    void OnButtonPressed(Reference<Button> /*r*/) override
    {
        Dialogs::MessageBox::ShowNotification("Button", "This button was pressed !");
    };
    void PaintControl(Reference<Control> /*c*/, Renderer& r) override
    {
        r.Clear('X', ColorPair{ Color::Red, Color::Black });
    };
    void OnCheck(Reference<Control> c, bool value) override
    {
        if (value)
            c->SetText("I am checked !");
        else
            c->SetText("I am not checked !");
    }
    void OnFocus(Reference<Control> c) override
    {
        if (b.ToBase<Control>() == c)
        {
            b->SetText("Push Me");
        }
    }
    void OnLoseFocus(Reference<Control> c) override
    {
        if (b.ToBase<Control>() == c)
        {
            b->SetText("Push");
        }
    }
};
int main()
{
    if (!Application::Init())
        return 1;

    Application::AddWindow(std::make_unique<MyWin>());
    Application::Run();
    return 0;
}
