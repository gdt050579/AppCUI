#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;
using namespace AppCUI::Graphics;
using namespace AppCUI::Utils;

int value = 1;

class MyControl : public AppCUI::Controls::UserControl
{
  public:
    MyControl(std::string_view layout) : UserControl(layout)
    {
    }
    Color back;
    void Paint(AppCUI::Graphics::Renderer& renderer) override
    {
        renderer.Clear(' ', ColorPair{ Color::Black, back });
    }
};
class WindowControlsBarExample : public Window
{
    Reference<MyControl> m;
    ItemHandle itText, itRed, itGreen, itBlue, itmShowHide;

  public:
    WindowControlsBarExample(char id) : Window("Test", "l:2,t:1,r:2,b:1", WindowFlags::Sizeable)
    {
        m       = this->AddControl<MyControl>(std::make_unique<MyControl>("l:2,t:1,r:2,b:1"));
        m->back = Color::Black;

        // associated a hot key
        this->SetHotKey(id);
        // add a TAG
        this->SetTag("TAG", "A tag is a small string\nthat explains what is the purpose\nof this window");
        // add buttons;
        auto cb     = this->GetControlBar(WindowControlsBarLayout::TopBarFromLeft);
        itmShowHide = cb.AddCommandItem("Hide Colors", 12345, "Show/Hide colors group");
        cb.AddCommandItem("Center", 12346, "When you press this button the Window will center to the screen");
        cb.AddCommandItem("+", 12347, "Increase the number from text item");
        cb      = this->GetControlBar(WindowControlsBarLayout::BottomBarFromRight);
        itRed   = cb.AddSingleChoiceItem("&Red", 100, false);
        itGreen = cb.AddSingleChoiceItem("&Green", 101, false);
        itBlue  = cb.AddSingleChoiceItem("&Blue", 102, id == '2');
        cb      = this->GetControlBar(WindowControlsBarLayout::BottomBarFromLeft);
        cb.AddCheckItem("&Option 1", 200, false, "Check this to enable option 1");
        cb.AddCheckItem("O&ption 2", 201, false, "Check this to enable option 2");
        itText = cb.AddTextItem("-1-");
    }
    bool OnEvent(Reference<Control> sender, Event eventType, int ID) override
    {
        Utils::LocalString<128> s;
        bool b_value;
        if (Window::OnEvent(sender, eventType, ID))
            return true;
        if (eventType == Event::Command)
        {
            switch (ID)
            {
            case 12345:
                b_value = !this->GetControlBar(WindowControlsBarLayout::BottomBarFromRight).IsItemVisible(itRed);
                this->GetControlBar(WindowControlsBarLayout::BottomBarFromRight).SetItemVisible(itRed, b_value);
                this->GetControlBar(WindowControlsBarLayout::BottomBarFromRight).SetItemVisible(itGreen, b_value);
                this->GetControlBar(WindowControlsBarLayout::BottomBarFromRight).SetItemVisible(itBlue, b_value);
                if (b_value)
                    this->GetControlBar(WindowControlsBarLayout::TopBarFromLeft)
                          .SetItemText(itmShowHide, "Hide colors");
                else
                    this->GetControlBar(WindowControlsBarLayout::TopBarFromLeft)
                          .SetItemText(itmShowHide, "Show colors");
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
                m->back = Color::Red;
                return true;
            case 101:
                m->back = Color::Green;
                return true;
            case 102:
                m->back = Color::Blue;
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
    Application::ArrangeWindows(ArrangeWindowsMethod::Grid);
    Application::Run();
    return 0;
}
