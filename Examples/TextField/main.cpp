#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;
using namespace AppCUI::Graphics;

void HighlightNumberAndCapitalLetters(Reference<Control>, Graphics::Character* chars, unsigned int charsCount)
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
  public:
    MyWin() : Window("Text Field Example", "d:c,w:70,h:20",WindowFlags::Sizeable)
    {
        Factory::Label::Create(this, "&Normal text", "x:1,y:1,w:15");
        Factory::TextField::Create(this, "a normal text", "l:19,t:1,r:1")->SetHotKey('N');

        Factory::Label::Create(this, "&Read only", "x:1,y:3,w:15");
        Factory::TextField::Create(this, "this text cannnot be changed", "l:19,t:3,r:1", TextFieldFlags::Readonly)->SetHotKey('R');

        Factory::Label::Create(this, "Inactive text", "x:1,y:5,w:15");
        Factory::TextField::Create(this, "this text is inactive", "l:19,t:5,r:1")->SetEnabled(false);

        Factory::Label::Create(this, "&Multi-line", "x:1,y:7,w:15");
        Factory::TextField::Create(
              this, "this is a large text the expends for over the next lines", "l:19,t:7,r:1,h:3")->SetHotKey('M');

        Factory::Label::Create(this, "Syntax &Highlight", "x:1,y:11,w:16");
        auto tf = Factory::TextField::Create(
              this, "Capital Letters and numbers (12345)", "l:19,t:11,r:1", TextFieldFlags::SyntaxHighlighting);
        tf->SetHotKey('H');
        tf->Handlers()->OnTextColor = HighlightNumberAndCapitalLetters;
    }
    bool OnEvent(Reference<Control>, Event eventType, int) override
    {
        if (eventType == Event::WindowClose)
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
    Application::AddWindow(std::make_unique<MyWin>());
    Application::Run();
    return 0;
}
