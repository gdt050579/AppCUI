#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

class MyWin : public Window
{
    Reference<Label> l;
    Reference<Splitter> v;
    Reference<Splitter> h;
  public:
    MyWin() : Window("Splitter example", "d:c,w:60,h:10", WindowFlags::Sizeable)
    {        
        h = Factory::Splitter::Create(this, "x:0,y:0,w:100%,h:100%", false);
        v  = Factory::Splitter::Create(h, "x:0,y:0,w:100%,h:100%", true);
        h->SetSecondPanelSize(2);
        v->SetSecondPanelSize(30);
        auto pleft   = Factory::Panel::Create(v, "x:0,y:0,w:100%,h:100%");
        auto pright  = Factory::Panel::Create(v, "x:0,y:0,w:100%,h:100%");
        auto pbottom = Factory::Panel::Create(h, "x:0,y:0,w:100%,h:100%");

        Factory::CheckBox::Create(pleft, "Enable first seeting", "x:1,y:1,w:30");
        Factory::CheckBox::Create(pleft, "Enable second seeting", "x:1,y:2,w:30");
        Factory::CheckBox::Create(pleft, "Enable third seeting", "x:1,y:3,w:30");

        Factory::RadioBox::Create(pright, "Option &1", "x:1,y:1,w:20", 5);
        Factory::RadioBox::Create(pright, "Option &2", "x:1,y:2,w:20", 5);
        Factory::RadioBox::Create(pright, "Option &3", "x:1,y:3,w:20", 5);

        l = Factory::Label::Create(pbottom, "Splitter Status", "l:1,t:0,w:90%,h:2");
        UpdateSplitterStatus();
    }
    void UpdateSplitterStatus()
    {
        LocalString<128> temp;
        if (l.IsValid())
            l->SetText(temp.Format(
                  "VS: [L=%d,R=%d], HS: [T=%d,B=%d]",
                  v->GetFirstPanelSize(),
                  v->GetSecondPanelSize(),
                  h->GetFirstPanelSize(),
                  h->GetSecondPanelSize()));
    }
    bool OnEvent(Reference<Control>, AppCUI::Controls::Event eventType, int) override
    {
        if (eventType == AppCUI::Controls::Event::WindowClose)
        {
            Application::Close();
            return true;
        }
        if (eventType == AppCUI::Controls::Event::ButtonClicked)
        {
            auto win = Factory::Window::Create("Empty example", "d:c,w:60,h:20", WindowFlags::Sizeable);
            auto sp1 = Factory::Splitter::Create(win, "d:c%", false);
            sp1->SetSecondPanelSize(10);
            Factory::Splitter::Create(sp1, "d:c", true)->SetSecondPanelSize(10);
            win->Show();
            return true;
        }
        if (eventType == AppCUI::Controls::Event::SplitterPositionChanged)
        {
            UpdateSplitterStatus();
            return true;
        }
        return false;
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
