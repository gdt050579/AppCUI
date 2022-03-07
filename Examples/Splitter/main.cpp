#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;

class Example1 : public Window
{
    Reference<Label> l;
    Reference<Splitter> v;
    Reference<Splitter> h;

  public:
    Example1() : Window("Splitter example", "d:c,w:60,h:10", WindowFlags::Sizeable)
    {
        h = Factory::Splitter::Create(this, "x:0,y:0,w:100%,h:100%");
        v = Factory::Splitter::Create(h, "x:0,y:0,w:100%,h:100%", SplitterFlags::Vertical);
        h->SetSecondPanelSize(2);
        h->SetPanel2Bounderies(1); // minimum 1 line at the bottom
        h->SetPanel1Bounderies(4); // minimum 4 lines at the top
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
            auto sp1 = Factory::Splitter::Create(win, "d:c%");
            sp1->SetSecondPanelSize(10);
            Factory::Splitter::Create(sp1, "d:c", SplitterFlags::Vertical)->SetSecondPanelSize(10);
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

class Example2: public Window
{
  public:
    Example2() : Window("Example 2", "d:c,w:70,h:20", WindowFlags::None)
    {
        Factory::TextField::Create(this, "Some text", "x:1,y:1,w:10");
        auto p = Factory::Panel::Create(this, "Splitter", "l:12,t:1,r:1,b:1");
        auto sp = Factory::Splitter::Create(p, "d:c", SplitterFlags::Vertical);
        sp->SetPanel2Bounderies(0, 15); // maximum 15 chars size on the right
        auto lv = Factory::ListView::Create(sp, "d:c");
        lv->AddColumn("Name", TextAlignament::Left, 15);
        lv->AddColumn("Grade", TextAlignament::Right, 10);
        lv->AddItem("Dragos", "10");
        lv->AddItem("Raul", "9");
        lv->AddItem("Gheorghita", "10");
        lv->AddItem("Andrei", "7");
        auto tb = Factory::Tab::Create(sp, "d:c", TabFlags::ListView);
        auto tp1 = Factory::TabPage::Create(tb, "&Infos");
        auto tp2 = Factory::TabPage::Create(tb, "&Data");
        auto tp22= Factory::TabPage::Create(tb, "&Extra");

    }
};

class MyWin : public Window
{
  public:
    MyWin() : Window("Splitter examples", "d:c,w:30,h:10", WindowFlags::None)
    {
        Factory::Button::Create(this, "Simple example", "x:1,y:1,w:26", 1)->Handlers()->OnButtonPressed =
              [](Reference<Button>)
        {
            Example1 dlg;
            dlg.Show();
        };
        Factory::Button::Create(this, "Collapse example", "x:1,y:3,w:26", 1)->Handlers()->OnButtonPressed =
              [](Reference<Button>)
        {
            Example2 dlg;
            dlg.Show();
        };
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
