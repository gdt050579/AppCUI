﻿#include "AppCUI.hpp"

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
    bool OnEvent(Reference<Control> control, AppCUI::Controls::Event eventType, int ID) override
    {
        if (Window::OnEvent(control, eventType, ID))
            return true;

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

constexpr int BTN_CMD_SHOW_DATATAB = 12345;
class Example2 : public Window
{
    Reference<TabPage> tp1, tp2, tp3;
    Reference<Tab> tb;
    Reference<Label> lb;

  public:
    Example2() : Window("Example 2", "d:c,w:70,h:20", WindowFlags::None)
    {
        Factory::TextField::Create(this, "Some text", "x:1,y:1,w:10");
        Factory::Button::Create(this, "Data Tab", "x:1,y:3,w:10", BTN_CMD_SHOW_DATATAB);
        lb      = Factory::Label::Create(this, "", "x:1,y:5,w:10");
        auto p  = Factory::Panel::Create(this, "Splitter", "l:12,t:1,r:1,b:1");
        auto sp = Factory::Splitter::Create(p, "d:c", SplitterFlags::Vertical | SplitterFlags::AutoCollapsePanel2);
        sp->SetPanel2Bounderies(0, 25); // maximum 15 chars size on the right
        sp->SetDefaultPanelSize(20);
        auto lv = Factory::ListView::Create(sp, "d:c", { "n:Name,a:l,w:15", "n:Grade,a:r,w:10" });

        lv->AddItems({ { "Dragos", "10" }, { "Raul", "9" }, { "Gheorghita", "10" }, { "Andrei", "7" } });

        tb  = Factory::Tab::Create(sp, "d:c", TabFlags::ListView);
        tp1 = Factory::TabPage::Create(tb, "&Infos");
        Factory::CheckBox::Create(tp1, "Setting &1", "l:1,t:1,r:1");
        Factory::CheckBox::Create(tp1, "Setting &2", "l:1,t:2,r:1");
        Factory::CheckBox::Create(tp1, "Setting &3", "l:1,t:3,r:1");
        tp2 = Factory::TabPage::Create(tb, "&Data");
        Factory::RadioBox::Create(tp2, "Option &1", "l:1,t:1,r:1", 1);
        Factory::RadioBox::Create(tp2, "Option &2", "l:1,t:2,r:1", 1);
        Factory::RadioBox::Create(tp2, "Option &3", "l:1,t:3,r:1", 1);
        tp3 = Factory::TabPage::Create(tb, "&Extra");
        Factory::TextField::Create(tp3, "Number", "l:1,t:1,r:1");
        Factory::TextField::Create(tp3, "pass", "l:1,t:3,r:1");
    }
    bool OnEvent(Reference<Control> control, AppCUI::Controls::Event eventType, int ID) override
    {
        if (Window::OnEvent(control, eventType, ID))
            return true;
        switch (eventType)
        {
        case Event::ButtonClicked:
            if (ID == BTN_CMD_SHOW_DATATAB)
                tb->SetCurrentTabPageByIndex(1, true);
            return true;
        case Event::SplitterPositionChanged:
            lb->SetText("ChangePos");
            return true;
        case Event::SplitterPanelAutoExpanded:
            lb->SetText("Expand");
            return true;
        case Event::SplitterPanelAutoCollapsed:
            lb->SetText("Collapsed");
            return true;
        }

        return false;
    }
};

class Example3 : public Window
{
    Reference<TabPage> tp1_1, tp1_2, tp1_3;
    Reference<Tab> tb1, tb2;
    Reference<Splitter> sp_h;

  public:
    Example3() : Window("Example 3", "d:c,w:70,h:20", WindowFlags::None)
    {
        Factory::TextField::Create(this, "Some text", "x:1,y:1,w:10");
        Factory::Button::Create(this, "Data Tab", "x:1,y:3,w:10", BTN_CMD_SHOW_DATATAB);
        auto p    = Factory::Panel::Create(this, "Splitter", "l:12,t:1,r:1,b:1");
        sp_h = Factory::Splitter::Create(p, "d:c", SplitterFlags::Horizontal | SplitterFlags::AutoCollapsePanel2);
        auto sp_v = Factory::Splitter::Create(sp_h, "d:c", SplitterFlags::Vertical | SplitterFlags::AutoCollapsePanel2);
        sp_v->SetPanel2Bounderies(0, 25); // maximum 15 chars size on the right
        sp_v->SetDefaultPanelSize(20);
        sp_h->SetPanel2Bounderies(2);
        sp_h->SetDefaultPanelSize(2);
        auto lv = Factory::ListView::Create(sp_v, "d:c", { "n:Name,a:l,w:15", "n:Grade,a:r,w:10" });

        lv->AddItems({ { "Dragos", "10" }, { "Raul", "9" }, { "Gheorghita", "10" }, { "Andrei", "7" } });

        tb1  = Factory::Tab::Create(sp_v, "d:c", TabFlags::ListView);
        tp1_1 = Factory::TabPage::Create(tb1, "&Infos");
        Factory::CheckBox::Create(tp1_1, "Setting &1", "l:1,t:1,r:1");
        Factory::CheckBox::Create(tp1_1, "Setting &2", "l:1,t:2,r:1");
        Factory::CheckBox::Create(tp1_1, "Setting &3", "l:1,t:3,r:1");
        tp1_2 = Factory::TabPage::Create(tb1, "&Data");
        Factory::RadioBox::Create(tp1_2, "Option &1", "l:1,t:1,r:1", 1);
        Factory::RadioBox::Create(tp1_2, "Option &2", "l:1,t:2,r:1", 1);
        Factory::RadioBox::Create(tp1_2, "Option &3", "l:1,t:3,r:1", 1);
        tp1_3 = Factory::TabPage::Create(tb1, "&Extra");
        Factory::TextField::Create(tp1_3, "Number", "l:1,t:1,r:1");
        Factory::TextField::Create(tp1_3, "pass", "l:1,t:3,r:1");

        tb2 = Factory::Tab::Create(sp_h, "d:c", TabFlags::BottomTabs);
        auto tp2_1 = Factory::TabPage::Create(tb2, "In&fos");
        Factory::CheckBox::Create(tp2_1, "Setting &1", "x:1,y:0,w:13");
        Factory::CheckBox::Create(tp2_1, "Setting &2", "x:15,y:0,w:13");
        Factory::CheckBox::Create(tp2_1, "Setting &3", "x:30,y:0,w:13");
        auto tp2_2 = Factory::TabPage::Create(tb2, "Da&ta");
        Factory::RadioBox::Create(tp2_2, "Option &1", "x:1,y:0,w:13", 1);
        Factory::RadioBox::Create(tp2_2, "Option &2", "x:15,y:0,w:13", 1);
        Factory::RadioBox::Create(tp2_2, "Option &3", "x:30,y:0,w:13", 1);
        auto tp2_3 = Factory::TabPage::Create(tb2, "E&xtra");
        Factory::TextField::Create(tp2_3, "Number", "x:1,y:0,w:15");
        Factory::TextField::Create(tp2_3, "pass", "x:20,y:0,w:15");
    }
    bool OnEvent(Reference<Control> control, AppCUI::Controls::Event eventType, int ID) override
    {
        if (Window::OnEvent(control, eventType, ID))
            return true;
        switch (eventType)
        {
        case Event::ButtonClicked:
            if (ID == BTN_CMD_SHOW_DATATAB)
                tb1->SetCurrentTabPageByIndex(1, true);
            return true;
        case Event::SplitterPanelAutoCollapsed:
            if(control == sp_h)
            {
                tb2->SetCurrentTabPageByIndex(0);
            }
            return true;
        }

        return false;
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
        Factory::Button::Create(this, "GView example", "x:1,y:5,w:26", 1)->Handlers()->OnButtonPressed =
              [](Reference<Button>)
        {
            Example3 dlg;
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
