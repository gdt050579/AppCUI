#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;
using namespace AppCUI::Graphics;

constexpr int MENU_CMD_SET_HELLO_WORLD = 1234;
constexpr int MENU_CMD_CLEAR_TEXT      = 1235;

void HighlightNumberAndCapitalLetters(Reference<Control>, Graphics::Character* chars, uint32 charsCount)
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

bool OnlyNumbers(Reference<Control>, char16 character)
{
    return (character >= '0') && (character <= '9');
}

class MyWin : public AppCUI::Controls::Window,
              public AppCUI::Controls::Handlers::OnAfterSetTextInterface,
              public AppCUI::Controls::Handlers::OnButtonPressedInterface,
              public AppCUI::Controls::Handlers::OnTextRightClickInterface
{
    Reference<TextField> tf1;
    Reference<TextField> tx_custommenu;
    AppCUI::Controls::Menu mnu;

  public:
    MyWin() : Window("Text Field Example", "d:c,w:70,h:22", WindowFlags::Sizeable)
    {
        Factory::Label::Create(this, "&Normal text", "x:1,y:1,w:15");
        Factory::TextField::Create(this, "a normal text", "l:19,t:1,r:1")->SetHotKey('N');

        Factory::Label::Create(this, "&Read only", "x:1,y:3,w:15");
        Factory::TextField::Create(this, "this text cannnot be changed", "l:19,t:3,r:1", TextFieldFlags::Readonly)
              ->SetHotKey('R');

        Factory::Label::Create(this, "Inactive text", "x:1,y:5,w:15");
        Factory::TextField::Create(this, "this text is inactive", "l:19,t:5,r:1")->SetEnabled(false);

        Factory::Label::Create(this, "&Multi-line", "x:1,y:7,w:15");
        Factory::TextField::Create(this, "this is a large text the expends for over the next lines", "l:19,t:7,r:1,h:3")
              ->SetHotKey('M');

        Factory::Label::Create(this, "Syntax &Highlight", "x:1,y:11,w:16");
        auto tf = Factory::TextField::Create(
              this, "Capital Letters and numbers (12345)", "l:19,t:11,r:1", TextFieldFlags::SyntaxHighlighting);
        tf->SetHotKey('H');
        tf->Handlers()->OnTextColor = HighlightNumberAndCapitalLetters;

        Factory::Label::Create(this, "Handler test", "x:1,y:13,w:16");
        tf1 = Factory::TextField::Create(this, "<... a text will be set ...>", "l:19,t:13,r:16");
        tf1->Handlers()->OnAfterSetText = this;
        tf1->SetEnabled(false);

        Factory::Button::Create(this, "Copy", "r:1,t:13,w:10", 123)->Handlers()->OnButtonPressed = this;

        Factory::Label::Create(this, "Custom menu", "x:1,y:15,w:16");
        tx_custommenu = Factory::TextField::Create(this, "Right click to see a custom menu", "l:19,t:15,r:1,h:1");
        tx_custommenu->Handlers()->OnTextRightClick = this;
        mnu.AddCommandItem("Set ... Hello World", MENU_CMD_SET_HELLO_WORLD);
        mnu.AddCommandItem("Empty text", MENU_CMD_CLEAR_TEXT);

        Factory::Label::Create(this, "Just numbers", "x:1,y:17,w:16");
        Factory::TextField::Create(this, "", "l:19,t:17,r:1,h:1")->Handlers()->OnValidateCharacter = OnlyNumbers;
    }
    bool OnEvent(Reference<Control>, Event eventType, int id) override
    {
        if (eventType == Event::WindowClose)
        {
            Application::Close();
            return true;
        }
        if (eventType == Event::Command)
        {
            if (id == MENU_CMD_SET_HELLO_WORLD)
            {
                tx_custommenu->SetText("Hello World");
            }
            if (id == MENU_CMD_CLEAR_TEXT)
            {
                tx_custommenu->SetText("");
            }
            return true;
        }
        return false;
    }
    // Handlers via interface
    void OnButtonPressed(Reference<Button>) override
    {
        tf1->SetText("A text was set");
    }
    void OnAfterSetText(Reference<Control>) override
    {
        Dialogs::MessageBox::ShowNotification("Info", "A new text was set in the text field");
    }
    void OnTextRightClick(Reference<Control> control, int x, int y) override
    {
        mnu.Show(control, x, y + 1);
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
