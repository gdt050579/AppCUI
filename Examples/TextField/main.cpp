#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;
using namespace AppCUI::Graphics;

void HighlightNumberAndCapitalLetters(Control*, Graphics::Character* chars, unsigned int charsCount, void*)
{
    Graphics::Character* end = chars + charsCount;
    while (chars < end)
    {
        if ((chars->Code >= '0') && (chars->Code <= '9'))
            chars->Color = ColorPair{ Color::Aqua, Color::Black };
        else if ((chars->Code >= 'A') && (chars->Code <= 'Z'))
            chars->Color = ColorPair{ Color::Yellow, Color::Black };
        else
            chars->Color = ColorPair{ Color::Silver, Color::Black };
        chars++;
    }
}

class MyWin : public AppCUI::Controls::Window
{
    Label l1, l2, l3, l4, l5;
    TextField t1, t2, t3, t4, t5;

  public:
    MyWin()
    {
        this->Create("Text Field Example", "d:c,w:70,h:20");
        l1.Create(this, "&Normal text", "x:1,y:1,w:15");
        t1.Create(this, "a normal text", "x:19,y:1,w:48");
        t1.SetHotKey('N');

        l2.Create(this, "&Read only", "x:1,y:3,w:15");
        t2.Create(this, "this text cannnot be changed", "x:19,y:3,w:48", TextFieldFlags::Readonly);
        t2.SetHotKey('R');

        l3.Create(this, "Inactive text", "x:1,y:5,w:15");
        t3.Create(this, "this text is inactive", "x:19,y:5,w:48");
        t3.SetEnabled(false);

        l4.Create(this, "&Multi-line", "x:1,y:7,w:15");
        t4.Create(this, "this is a large text the expends for over the next lines", "x:19,y:7,w:48,h:3");
        t4.SetHotKey('M');

        l5.Create(this, "Syntax &Highlight", "x:1,y:11,w:16");
        t5.Create(
              this,
              "Capital Letters and numbers (12345)",
              "x:19,y:11,w:48",
              TextFieldFlags::SyntaxHighlighting,
              HighlightNumberAndCapitalLetters);
        t5.SetHotKey('H');
    }
    bool OnEvent(Control*, Event eventType, int) override
    {
        if (eventType == Event::EVENT_WINDOW_CLOSE)
        {
            Application::Close();
            return true;
        }
        return false;
    }
};
int main()
{
    if (!Application::Init(Application::InitializationFlags::CommandBar))
        return 1;
    Application::AddWindow(new MyWin());
    Application::Run();
    return 0;
}
