#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;
using namespace AppCUI::Graphics;

#define CLOSE_BUTTON_ID    123456
#define SHOW_TAB_BUTTON_ID 654321
#define TAB_MODE_GROUP     101

class TabExampleWin : public Window
{
  public:
    TabExampleWin(TabFlags flags, unsigned int tabSize, int tabsCount)
        : Window("Tab Control Example", "d:c,w:60,h:20", WindowFlags::NoCloseButton)
    {
        // information panel
        auto p_inf = Factory::Panel::Create(this, "Informations", "x:1,y:1,w:56,h:5");
        Factory::Label::Create(
              p_inf, "To navigate through Tabs use Ctrl+TAB / Ctrl+Shift+Tab\n", "x:0,y:0,w:100%,h:100%");

        Factory::Button::Create(this, "Close", "d:b,w:12", CLOSE_BUTTON_ID);

        auto tb = Factory::Tab::Create(this, "x:1,y:7,w:56,h:8", flags);
        tb->SetTabPageTitleSize(tabSize);
        // first page
        if (tabsCount >= 1)
        {
            auto pg1 = Factory::TabPage::Create(tb, "&RadioBox");
            Factory::RadioBox::Create(pg1, "Option &1", "x:1,y:1,w:20", 100);
            Factory::RadioBox::Create(pg1, "Option &2", "x:1,y:2,w:20", 100);
            Factory::RadioBox::Create(pg1, "Option &3", "x:1,y:3,w:20", 100);
            auto lv = Factory::ListView::Create(pg1, "l:20,t:0,r:0,b:0");
            lv->AddColumn("Names", TextAlignament::Left, 50);
            lv->AddItem("Andrei");
            lv->AddItem("Denis");
            lv->AddItem("Dragos");
            lv->AddItem("Ghiorghita");
            lv->AddItem("Raul");
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

        tb->SetCurrentTabPage(0);
    }
    bool OnEvent(Control*, Event eventType, int controlID) override
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
    RadioBox *tabTop, *tabBottom, *tabLeft, *tabList, *tabNoTabs;
    CheckBox *cbTransparent, *cbTabBar;
    NumericSelector *selector, *tabsCount;

  public:
    MyWin() : Window("Tab example config", "d:c,w:50,h:19",WindowFlags::None)
    {        
        auto p = Factory::Panel::Create(this, "Tab mode", "x:1,y:1,w:46,h:7");
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

        Factory::Button::Create(this, "&Show tab control", "l:14,b:0,w:21", SHOW_TAB_BUTTON_ID);
    }
    void ShowTabControl()
    {
        TabFlags flags;
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

        TabExampleWin tw(flags, static_cast<unsigned int>(selector->GetValue()), static_cast<int>(tabsCount->GetValue()));
        tw.Show();
    }
    bool OnEvent(Control*, Event eventType, int controlID) override
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
