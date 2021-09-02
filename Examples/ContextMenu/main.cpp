#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;
using namespace AppCUI::Input;

#define MENU_CMD_SAVE           1000
#define MENU_CMD_OPEN           1001
#define MENU_CMD_CLEAR          1002
#define MENU_CMD_RED            1003
#define MENU_CMD_GREEN          1004
#define MENU_CMD_BLUE           1005

std::string_view colors[] = {
    "red",        "yellow",       "blue",           "brown",         "orange",     "green",
    "violet",     "black",        "carnation pink", "yellow orange", "blue green", "red violet",
    "red orange", "yellow green", "blue violet",    "white",         "violet red", "dandelion",
    "cerulean",   "apricot",      "scarlet",        "green yellow",  "indigo",     "gray"
};

class MyUserControl: public AppCUI::Controls::UserControl
{
    Menu ctxMenu;

  public:
    Color squareColor;
    bool smallMenu;
  public:
    void Create(Control* parent);
    void Paint(AppCUI::Graphics::Renderer& renderer) override;
    void OnMousePressed(int x, int y, MouseButton button) override;
};
void MyUserControl::Create(Control* parent)
{
    UserControl::Create(parent, "x:50%,y:50%,w:2,h:1");
    squareColor = Color::Red;
    smallMenu   = false;
    // build a menu
    ctxMenu.AddCommandItem("&Save content", MENU_CMD_SAVE, Key::F2);
    ctxMenu.AddCommandItem("&Open content", MENU_CMD_OPEN, Key::F3);
    auto cmd1 = ctxMenu.AddCommandItem("&Reset content", MENU_CMD_CLEAR, Key::R|Key::Ctrl|Key::Alt);
    ctxMenu.SetEnable(cmd1, false);
    ctxMenu.AddSeparator();
    auto flg1 = ctxMenu.AddCheckItem("Flag &1",100);
    ctxMenu.SetChecked(flg1, true);
    ctxMenu.AddCheckItem("Flag &2",200);
    ctxMenu.AddCheckItem("Flag &3",300);
    ctxMenu.AddSeparator();
    ctxMenu.AddRadioItem("Select option &A",350,true);
    ctxMenu.AddRadioItem("Select option &B",351);
    ctxMenu.AddRadioItem("Select option &C",352);
    ctxMenu.AddSeparator();
    auto smHandle = ctxMenu.AddSubMenu("Co&lor");
    ctxMenu.GetSubMenu(smHandle)->AddRadioItem("Red", MENU_CMD_RED, true);
    ctxMenu.GetSubMenu(smHandle)->AddRadioItem("Green", MENU_CMD_GREEN);
    ctxMenu.GetSubMenu(smHandle)->AddRadioItem("Blue", MENU_CMD_BLUE);
    ctxMenu.GetSubMenu(smHandle)->AddSeparator();
    auto smOther = ctxMenu.GetSubMenu(smHandle)->AddSubMenu("&Other");
    for (unsigned int tr = 0; tr < sizeof(colors) / sizeof(colors[0]);tr++)
    {
        ctxMenu.GetSubMenu(smHandle)->GetSubMenu(smOther)->AddRadioItem(colors[tr], 2000 + tr, tr == 0);
    }
    ctxMenu.AddSeparator();
    ctxMenu.AddCommandItem("An item with a large caption with multiple characters", 1234, Key::Q | Key::Ctrl | Key::Alt);
}
void MyUserControl::OnMousePressed(int x, int y, MouseButton button)
{
    if ((button & MouseButton::Right) != MouseButton::None)
    {
        if (smallMenu)
            ctxMenu.Show(this, x, y, { 20, 5 });
        else
            ctxMenu.Show(this, x, y);
    }
}
void MyUserControl::Paint(AppCUI::Graphics::Renderer& renderer)
{
    renderer.Clear(' ', ColorPair{ Color::White, squareColor });
}
class ContextMenuExample : public AppCUI::Controls::Window
{
    Label l1;
    MyUserControl m;
    CheckBox cb;
  public:
    ContextMenuExample()
    {
        this->Create("Context menu", "a:c,w:64,h:10");
        l1.Create(this, "Right click on the red-square below to view a context menu\nTIP: Use mouse wheel to scroll menus", "x:1,y:1,w:62,h:2");
        cb.Create(this, "Small contextual menu with scroll", "x:1,y:6,w:62");
        m.Create(this);
    }
    bool OnEvent(Control*, Event eventType, int controlID) override
    {
        if (eventType == Event::EVENT_WINDOW_CLOSE)
        {
            Application::Close();
            return true;
        }
        if (eventType == Event::EVENT_CHECKED_STATUS_CHANGED)
        {
            m.smallMenu = cb.IsChecked();
            return true;
        }
        if (eventType == Event::EVENT_COMMAND)
        {
            switch (controlID)
            {
            case MENU_CMD_RED:
                m.squareColor = Color::Red;
                break;
            case MENU_CMD_GREEN:
                m.squareColor = Color::Green;
                break;
            case MENU_CMD_BLUE:
                m.squareColor = Color::Blue;
                break;
            }
        }
        return false;
    }
};
int main()
{
    if (!Application::Init())
        return 1;
    Application::AddWindow(new ContextMenuExample());
    Application::Run();
    return 0;
}
