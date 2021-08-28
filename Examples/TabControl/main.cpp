#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;
using namespace AppCUI::Graphics;

#define CLOSE_BUTTON_ID    123456
#define SHOW_TAB_BUTTON_ID 654321
#define TAB_MODE_GROUP     101

class TabExampleWin : public AppCUI::Controls::Window
{
    Tab tb;
    TabPage pg1, pg2, pg3;
    RadioBox r1, r2, r3;
    CheckBox c1, c2, c3;
    Label lb1;
    TextField tx1;
    Panel p_inf;
    Label inf;
    Button btnClose;


  public:
    TabExampleWin(TabFlags flags, unsigned int tabSize)
    {
        Utils::LocalString<256> temp;
        temp.Set("To navigate through Tabs use Ctrl+TAB / Ctrl+Shift+Tab\n");
        this->Create("Tab Control Example", "a:c,w:60,h:20", WindowFlags::NoCloseButton);
        // information panel
        p_inf.Create(this, "Informations", "x:1,y:1,w:56,h:5");
        inf.Create(&p_inf, temp.GetText(), "x:0,y:0,w:100%,h:100%");

        btnClose.Create(this, "Close", "r:0,b:0,w:12", CLOSE_BUTTON_ID);

        tb.Create(this, "x:1,y:7,w:56,h:8", flags);
        tb.SetTabPageTitleSize(tabSize);
        // first page
        pg1.Create(&tb, "&RadioBox");
        r1.Create(&pg1, "Option &1", "x:1,y:1,w:20", 100);
        r2.Create(&pg1, "Option &2", "x:1,y:2,w:20", 100);
        r3.Create(&pg1, "Option &3", "x:1,y:3,w:20", 100);

        // second page
        pg2.Create(&tb, "&CheckBox");
        c1.Create(&pg2, "Enable flag &1", "x:1,y:1,w:20");
        c2.Create(&pg2, "Enable flag &2", "x:1,y:2,w:20");
        c3.Create(&pg2, "Enable flag &3", "x:1,y:3,w:20");

        // third page
        pg3.Create(&tb, "&TextField");
        lb1.Create(&pg3, "Enter a text", "x:1,y:1,w:15");
        tx1.Create(&pg3, "some text ...", "x:17,y:1,w:20,h:5");


        tb.SetCurrentTabPage(0);
    }
    bool OnEvent(Control* sender, Event eventType, int controlID) override
    {
        if ((eventType == Event::EVENT_BUTTON_CLICKED) && (controlID == CLOSE_BUTTON_ID))
        {
            this->Exit(0);
            return true;
        }
        return false;
    }
};

class MyWin : public AppCUI::Controls::Window
{
    Panel p;
    RadioBox tabTop, tabBottom, tabLeft, tabList;
    CheckBox cbTransparent, cbTabBar;
    Button btnShow;
    ComboBox cbTabSize;
    Label lbTabSize;

  public:
    MyWin()
    {
        this->Create("Tab example config", "a:c,w:50,h:17");
        p.Create(this, "Tab mode", "x:1,y:1,w:46,h:6");
        tabTop.Create(&p, "Tab pages on &top", "x:1,y:0,w:40", TAB_MODE_GROUP);
        tabBottom.Create(&p, "Tab pages on &bottom", "x:1,y:1,w:40", TAB_MODE_GROUP);
        tabLeft.Create(&p, "Tab pages on &left", "x:1,y:2,w:40", TAB_MODE_GROUP);
        tabList.Create(&p, "Lis&ts", "x:1,y:3,w:40", TAB_MODE_GROUP);
        tabTop.SetChecked(true);

        cbTransparent.Create(this, "Transparent background for tab pages", "x:1,y:8,w:46");
        cbTabBar.Create(this, "Show tab bar with pages", "x:1,y:9,w:46");

        lbTabSize.Create(this, "Tabs &width", "x:1,y:11,w:10");
        cbTabSize.Create(
              this,
              "x:14,y:11,w:32",
              "Tiny (5 characters),Small (7 characters),Medium (10 characters),Normal (14 characters),Large (18 characters)");
        cbTabSize.SetHotKey('W');
        cbTabSize.SetCurentItemIndex(3);

        btnShow.Create(this, "&Show tab control", "l:14,b:0,w:21", SHOW_TAB_BUTTON_ID);
    }
    void ShowTabControl()
    {
        TabFlags flags;
        if (tabTop.IsChecked())
            flags = TabFlags::TopTabs;
        if (tabBottom.IsChecked())
            flags = TabFlags::BottomTabs;
        if (tabLeft.IsChecked())
            flags = TabFlags::LeftTabs;
        if (tabList.IsChecked())
            flags = TabFlags::ListView;
        if (cbTransparent.IsChecked())
            flags = flags | TabFlags::TransparentBackground;
        if (cbTabBar.IsChecked())
            flags = flags | TabFlags::TabsBar;

        unsigned int tabSizes[] = { 5, 7, 10, 14, 18 };

        TabExampleWin tw(flags, tabSizes[cbTabSize.GetCurrentItemIndex()]);
        tw.Show();
    }
    bool OnEvent(Control* sender, Event eventType, int controlID) override
    {
        if (eventType == Event::EVENT_WINDOW_CLOSE)
        {
            Application::Close();
            return true;
        }
        if ((eventType == Event::EVENT_BUTTON_CLICKED) && (controlID == SHOW_TAB_BUTTON_ID))
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
    Application::AddWindow(new MyWin());
    Application::Run();
    return 0;
}
