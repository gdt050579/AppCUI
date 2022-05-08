#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;
using namespace AppCUI::Graphics;
using namespace AppCUI::Utils;

#define CLOSE_BUTTON_ID    123456
#define SHOW_TAB_BUTTON_ID 654321
#define TAB_MODE_GROUP     101

class TabNoFocusExample : public Window
{
    Reference<Tab> tb;

  public:
    TabNoFocusExample(TabFlags flags, uint32 /*tabSize*/, int tabsCount)
        : Window("Tab Control Example", "d:c,w:60,h:20", WindowFlags::Sizeable)
    {
        tb       = Factory::Tab::Create(this, "l:2,t:4,r:2,b:2", flags);
        auto tx  = Factory::TextField::Create(this, "Some text ....", "l:2,t:2,r:2,h:1");
        auto w_b = this->GetControlBar(WindowControlsBarLayout::BottomBarFromLeft);

        LocalString<128> tmp;

        for (int tr = 0; tr < tabsCount; tr++)
        {
            tmp.SetFormat("Tab: &%d", tr + 1);
            auto pg = Factory::TabPage::Create(tb, tmp.GetText());
            auto lv = Factory::ListView::Create(pg, "d:c", { "n:Values,a:l,w:20" });

            lv->AddItem(tmp.Format("Field: %d -> Dragos", tr + 1));
            lv->AddItem(tmp.Format("Field: %d -> Ghiorghita", tr + 1));
            lv->AddItem(tmp.Format("Field: %d -> Denis", tr + 1));
            lv->AddItem(tmp.Format("Field: %d -> Andrei", tr + 1));

            w_b.AddSingleChoiceItem(tmp.Format("T:%d", tr + 1), 1000 + tr, tr == 0);
        }
        tx->SetFocus();
    }
    bool OnEvent(Reference<Control> ctrl, Event eventType, int controlID) override
    {
        if (Window::OnEvent(ctrl, eventType, controlID))
            return true;
        if (eventType == Event::Command)
        {
            if ((controlID >= 1000) && (controlID < 1100))
            {
                tb->SetCurrentTabPageByIndex(controlID - 1000);
                return true;
            }
        }
        return false;
    }
};

class TabExampleWin2 : public Window
{
    Reference<Tab> tb_h, tb_v;

  public:
    TabExampleWin2(TabFlags flags, uint32 /*tabSize*/, int tabsCount)
        : Window("Tab Control Example", "d:c,w:60,h:20", WindowFlags::Sizeable)
    {
        auto spv = Factory::Splitter::Create(this, "d:c", SplitterFlags::Vertical);
        auto sph = Factory::Splitter::Create(spv, "d:c");

        Factory::Button::Create(sph, "Add", "d:c", 1234);

        tb_h = Factory::Tab::Create(sph, "d:c", flags);
        tb_v = Factory::Tab::Create(spv, "d:c", flags);

        LocalString<128> tmp;

        for (int tr = 0; tr < tabsCount; tr++)
        {
            tmp.SetFormat("VTab: &%d", tr + 1);
            auto pg = Factory::TabPage::Create(tb_v, tmp.GetText());
            auto lv = Factory::ListView::Create(pg, "d:c", { "n:Name,a:l,w:20" });

            tmp[0] = 'H';
            pg     = Factory::TabPage::Create(tb_h, tmp.GetText());
            lv     = Factory::ListView::Create(pg, "d:c", { "n:Name,a:l,w:20" });

        }
    }
    bool OnEvent(Reference<Control> ctrl, Event eventType, int controlID) override
    {
        if (Window::OnEvent(ctrl, eventType, controlID))
            return true;
        if ((eventType == Event::ButtonClicked) && (controlID == 1234))
        {
            auto pg = Factory::TabPage::Create(tb_v, "New page");
            auto lv = Factory::ListView::Create(pg, "d:c", { "n:Name,a:l,w:20" });

            pg = Factory::TabPage::Create(tb_h, "New page");
            lv = Factory::ListView::Create(pg, "d:c", { "n:Name,a:l,w:20" });

            return true;
        }
        return false;
    }
};

class TabExampleWin : public Window
{
  public:
    TabExampleWin(TabFlags flags, uint32 tabSize, int tabsCount)
        : Window("Tab Control Example", "d:c,w:60,h:20", WindowFlags::NoCloseButton)
    {
        Factory::Button::Create(this, "Close", "d:b,w:12", CLOSE_BUTTON_ID);
        auto spl = Factory::Splitter::Create(this, "l:0,t:0,r:0,b:3");
        spl->SetSecondPanelSize(8);
        // information panel
        auto p_inf = Factory::Panel::Create(spl, "Informations", "x:1,y:1,w:56,h:5");
        Factory::Label::Create(
              p_inf, "To navigate through Tabs use Ctrl+TAB / Ctrl+Shift+Tab\n", "x:0,y:0,w:100%,h:100%");

        auto tb = Factory::Tab::Create(spl, "x:1,y:7,w:56,h:8", flags);
        tb->SetTabPageTitleSize(tabSize);
        // first page
        if (tabsCount >= 1)
        {
            auto pg1 = Factory::TabPage::Create(tb, "&RadioBox");
            auto lv  = Factory::ListView::Create(pg1, "l:20,t:0,r:0,b:0", { "n:Names,a:l,w:50" });

            lv->AddItem("Andrei");
            lv->AddItem("Denis");
            lv->AddItem("Dragos");
            lv->AddItem("Gheorghita");
            lv->AddItem("Raul");

            Factory::RadioBox::Create(pg1, "Option &1", "x:1,y:1,w:20", 100);
            Factory::RadioBox::Create(pg1, "Option &2", "x:1,y:2,w:20", 100);
            Factory::RadioBox::Create(pg1, "Option &3", "x:1,y:3,w:20", 100);
        }

        // second page
        if (tabsCount >= 2)
        {
            auto pg2 = Factory::TabPage::Create(tb, "&CheckBox");
            Factory::CheckBox::Create(pg2, "Enable flag &1", "x:1,y:1,w:20");
            Factory::CheckBox::Create(pg2, "Enable flag &2", "x:1,y:2,w:20");
            Factory::CheckBox::Create(pg2, "Enable flag &3", "x:1,y:3,w:20");
        }

        // third page
        if (tabsCount >= 3)
        {
            auto pg3 = Factory::TabPage::Create(tb, "&TextField");
            Factory::Label::Create(pg3, "Enter a text", "x:1,y:1,w:15");
            Factory::TextField::Create(pg3, "some text ...", "x:17,y:1,w:20,h:5");
        }

        tb->SetCurrentTabPageByIndex(0);
    }
    bool OnEvent(Reference<Control>, Event eventType, int controlID) override
    {
        if ((eventType == Event::ButtonClicked) && (controlID == CLOSE_BUTTON_ID))
        {
            this->Exit(0);
            return true;
        }
        return false;
    }
};

class MyWin : public Window
{
    Reference<RadioBox> tabTop, tabBottom, tabLeft, tabList, tabNoTabs;
    Reference<CheckBox> cbTransparent, cbTabBar;
    Reference<NumericSelector> selector, tabsCount;
    Reference<ComboBox> example;

  public:
    MyWin() : Window("Tab example config", "d:c,w:50,h:21", WindowFlags::None)
    {
        auto p    = Factory::Panel::Create(this, "Tab mode", "x:1,y:1,w:46,h:7");
        tabTop    = Factory::RadioBox::Create(p, "Tab pages on &top", "x:1,y:0,w:40", TAB_MODE_GROUP);
        tabBottom = Factory::RadioBox::Create(p, "Tab pages on &bottom", "x:1,y:1,w:40", TAB_MODE_GROUP);
        tabLeft   = Factory::RadioBox::Create(p, "Tab pages on &left", "x:1,y:2,w:40", TAB_MODE_GROUP);
        tabList   = Factory::RadioBox::Create(p, "Lis&ts", "x:1,y:3,w:40", TAB_MODE_GROUP);
        tabNoTabs = Factory::RadioBox::Create(p, "&Hide tabs (no visible tabs)", "x:1,y:4,w:40", TAB_MODE_GROUP);
        tabTop->SetChecked(true);

        cbTransparent = Factory::CheckBox::Create(this, "Transparent background for tab pages", "x:1,y:8,w:46");
        cbTabBar      = Factory::CheckBox::Create(this, "Show tab bar with pages", "x:1,y:9,w:46");

        Factory::Label::Create(this, "Tabs &width", "x:1,y:11,w:10");
        selector = Factory::NumericSelector::Create(this, 5, 18, 6, "x:14,y:11,w:32");
        selector->SetHotKey('W');

        Factory::Label::Create(this, "Tabs count", "x:1,y:13,w:10");
        tabsCount = Factory::NumericSelector::Create(this, 1, 3, 3, "x:14,y:13,w:32");

        Factory::Label::Create(this, "Example", "x:1,y:15,w:10");
        example = Factory::ComboBox::Create(this, "x:14,y:15,w:32", "Normal,Sliders,NoFocus");
        example->SetCurentItemIndex(2);

        Factory::Button::Create(this, "&Show tab control", "l:16,b:0,w:21", SHOW_TAB_BUTTON_ID);
    }
    void ShowTabControl()
    {
        TabFlags flags = TabFlags::TopTabs;
        if (tabTop->IsChecked())
            flags = TabFlags::TopTabs;
        if (tabBottom->IsChecked())
            flags = TabFlags::BottomTabs;
        if (tabLeft->IsChecked())
            flags = TabFlags::LeftTabs;
        if (tabList->IsChecked())
            flags = TabFlags::ListView;
        if (tabNoTabs->IsChecked())
            flags = TabFlags::HideTabs;
        if (cbTransparent->IsChecked())
            flags = flags | TabFlags::TransparentBackground;
        if (cbTabBar->IsChecked())
            flags = flags | TabFlags::TabsBar;

        auto v_tabSize   = static_cast<uint32>(selector->GetValue());
        auto v_tabsCount = static_cast<int>(tabsCount->GetValue());

        TabExampleWin tw(flags, v_tabSize, v_tabsCount);
        TabExampleWin2 tw2(flags, v_tabSize, v_tabsCount);
        TabNoFocusExample tnf(flags, v_tabSize, v_tabsCount);
        switch (example->GetCurrentItemIndex())
        {
        case 0:
            tw.Show();
            break;
        case 1:
            tw2.Show();
            break;
        case 2:
            tnf.Show();
            break;
        }
    }
    bool OnEvent(Reference<Control>, Event eventType, int controlID) override
    {
        if (eventType == Event::WindowClose)
        {
            Application::Close();
            return true;
        }
        if ((eventType == Event::ButtonClicked) && (controlID == SHOW_TAB_BUTTON_ID))
        {
            ShowTabControl();
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
