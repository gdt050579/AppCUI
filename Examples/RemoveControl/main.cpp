#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

class RemoveControlExample : public Window
{
    Reference<CheckBox> cb;
    Reference<Label> lb;
    Reference<Button> bt;
  public:
    RemoveControlExample() : Window("Remove Example", "d:c,w:60,h:10", WindowFlags::None)
    {
        bt = this->CreateChildControl<Button>("Remove controls", "l:1,t:1,r:1", 123, ButtonFlags::None);
        lb = this->CreateChildControl<Label>("This is a label !", "l:1,t:3,r:1");
        cb = this->CreateChildControl<CheckBox>("This is a checkbos", "l:1,t:5,r:1", 100);
    }
    bool OnEvent(Reference<Control> ctrl, Event eventType, int controlID) override
    {
        if (Window::OnEvent(ctrl, eventType, controlID))
            return true;
        if (eventType == Event::ButtonClicked)
        {
            if (lb && cb)
            {
                this->RemoveControl<Label>(lb);
                this->RemoveControl<CheckBox>(cb);
            }
            else
            {
                Dialogs::MessageBox::ShowNotification("Info", "Controls have already been removed !");
            }
            return true;
        }
        return false;
    }
};
int main()
{
    if (!Application::Init())
        return 1;
    Application::AddWindow(std::make_unique<RemoveControlExample>());
    Application::Run();
    return 0;
}
