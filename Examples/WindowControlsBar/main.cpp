#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;
using namespace AppCUI::Graphics;

int value = 1;

class MyControl : public AppCUI::Controls::UserControl
{
  public:
    Color back;
    void Paint(AppCUI::Graphics::Renderer& renderer) override
    {
        renderer.Clear(' ', ColorPair{ Color::Black, back });
    }
};
class WindowControlsBarExample : public AppCUI::Controls::Window
{
    MyControl m;
    ItemHandle itText;

  public:
    WindowControlsBarExample(char id)
    {
        this->Create("Test", "l:2,t:1,r:2,b:1", WindowFlags::Sizeable);
        m.Create(this, "l:2,t:1,r:2,b:1");
        m.back = Color::Black;
        // associated a hot key
        this->SetHotKey(id);
        // add a TAG
        this->SetTag("TAG", "A tag is a small string\nthat explains what is the purpose\nof this window");
        // add buttons;
        auto cb = this->GetControlBar(WindowControlsBarLayout::TopBarFromLeft);
        cb.AddCommandItem("Close", 12345, "When you press this button the Window will close");
        cb.AddCommandItem("Center", 12346, "When you press this button the Window will center to the screen");
        cb.AddCommandItem("+", 12347, "Increase the number from text item");
        cb = this->GetControlBar(WindowControlsBarLayout::BottomBarFromRight);
        cb.AddSingleChoiceItem("&Red", 100, false);
        cb.AddSingleChoiceItem("&Green", 101, false);
        cb.AddSingleChoiceItem("&Blue", 102, id == '2');
        cb = this->GetControlBar(WindowControlsBarLayout::BottomBarFromLeft);
        cb.AddCheckItem("&Option 1", 200, false, "Check this to enable option 1");
        cb.AddCheckItem("O&ption 2", 201, false, "Check this to enable option 2");
        itText = cb.AddTextItem("-1-");
    }
    bool OnEvent(Control*, Event eventType, int ID) override
    {
        Utils::LocalString<128> s;
        if (eventType == Event::WindowClose)
        {
            Application::Close();
            return true;
        }
        if (eventType == Event::Command)
        {
            switch (ID)
            {
            case 12345:
                Application::Close();
                return true;
            case 12346:
                this->CenterScreen();
                return true;
            case 12347:
                value += 1;
                s.Format("%d", value);
                this->GetControlBar(WindowControlsBarLayout::BottomBarFromLeft).SetItemText(itText, s.GetText());
                s.Format("Dec:%d\nHex:%08X\nOct:%o", value, value, value);
                this->GetControlBar(WindowControlsBarLayout::BottomBarFromLeft).SetItemToolTip(itText, s.GetText());
                return true;
            case 100:
                m.back = Color::Red;
                return true;
            case 101:
                m.back = Color::Green;
                return true;
            case 102:
                m.back = Color::Blue;
                return true;
            }
        }
        return false;
    }
};
int main()
{
    if (!Application::Init())
        return 1;
    Application::AddWindow(std::make_unique<WindowControlsBarExample>('1'));
    Application::AddWindow(std::make_unique<WindowControlsBarExample>('2'));
    Application::ArrangeWindows(ArangeWindowsMethod::Grid);
    Application::Run();
    return 0;
}
