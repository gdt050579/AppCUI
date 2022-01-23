#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;

class MyUserControl : public UserControl
{
  public:
    MyUserControl() : UserControl("x:1,y:3,w:36,h:10")
    {
        cp = CodePageID::DOS_437;
    }
    CodePage cp;
    void Paint(Renderer& r) override
    {
        r.Clear(' ', DefaultColorPair);
        for (uint32 i = 0; i < 256; i++)
        {
            r.WriteCharacter((i % 32) + 2, (i / 32) + 1, cp[i], DefaultColorPair);
        }
    }
};
class ExampleWin : public Window
{
    Reference<MyUserControl> mc;
    Reference<ComboBox> cb;

  public:
    ExampleWin() : Window("Example", "d:c,w:40,h:16", WindowFlags::None)
    {
        cb = Factory::ComboBox::Create(this, "x:1,y:1,w:36", CodePage::GetListValue());
        mc = this->CreateChildControl<MyUserControl>();
        cb->SetCurentItemIndex(0);
    }
    bool OnEvent(Reference<Control> control, Event eventType, int ID) override
    {
        if (Window::OnEvent(control, eventType, ID))
            return true;
        if (eventType == Event::ComboBoxSelectedItemChanged)
        {
            mc->cp = static_cast<CodePageID>(cb->GetCurrentItemIndex());
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
