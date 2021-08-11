#include "AppCUI.hpp"
#include <string>
#include <cstring>

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;
using namespace AppCUI::Graphics;
using namespace AppCUI::Dialogs;


struct MyUserControl : public UserControl
{
    void Paint(Graphics::Renderer& renderer) override
    {
        renderer.Clear(' ', ColorPair{ Color::White, Color::Black });

        for (unsigned int tr = 2; tr < 24; tr += 2)
        {
            renderer.DrawHorizontalLine(5, tr, 12, '-', ColorPair{ Color::White, Color::Blue });
            renderer.DrawHorizontalLine(25, tr, 32, '-', ColorPair{ Color::White, Color::DarkGreen });
        }
            
        // first batch
        WriteTextParams params(WriteTextFlags::SingleLine);
        params.Width = 8;
        params.X     = 5;
        params.Y     = 2;
        params.Align = TextAlignament::Left;
        params.Color = NoColorPair;
        renderer.WriteText("1234", params);

        params.Y = 4;
        renderer.WriteText("123456789ABCD", params);

        params.Flags = WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth;
        params.Y     = 6;
        renderer.WriteText("123456789ABCD", params);

        params.Flags = WriteTextFlags::SingleLine;
        params.Align = TextAlignament::Right;
        params.Y     = 8;
        renderer.WriteText("1234", params);

        params.Flags = WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth;
        params.Y     = 10;
        renderer.WriteText("1234", params);

        params.Y = 12;
        renderer.WriteText("123456789ABCD", params);

        params.Y = 14;
        renderer.WriteText("12345678", params);

        params.Flags = WriteTextFlags::SingleLine;
        params.Align = TextAlignament::Center;
        params.Y     = 16;
        renderer.WriteText("123", params);

        params.Flags = WriteTextFlags::SingleLine | WriteTextFlags::ClipToWidth;
        params.Y     = 18;
        renderer.WriteText("1234", params);

        params.Y     = 20;
        renderer.WriteText("12345678", params);

        params.Y     = 22;
        renderer.WriteText("123456789ABCD", params);

        // second batch
        renderer.WriteSingleLineText(25, 2, "1234", NoColorPair);
        renderer.WriteSingleLineText(25, 4, "123456789ABCD", NoColorPair);
        renderer.WriteSingleLineText(25, 6, 8, "123456789ABCD", NoColorPair);
        renderer.WriteSingleLineText(25, 8, "1234", NoColorPair, TextAlignament::Right);
        renderer.WriteSingleLineText(25, 10, 8, "1234", NoColorPair, TextAlignament::Right);
        renderer.WriteSingleLineText(25, 12, 8, "123456789ABCD", NoColorPair, TextAlignament::Right);
        renderer.WriteSingleLineText(25, 14, 8, "12345678", NoColorPair, TextAlignament::Right);
        renderer.WriteSingleLineText(25, 16, "123", NoColorPair, TextAlignament::Center);
        renderer.WriteSingleLineText(25, 18, 8, "1234", NoColorPair, TextAlignament::Center);
        renderer.WriteSingleLineText(25, 20, 8, "12345678", NoColorPair, TextAlignament::Center);
        renderer.WriteSingleLineText(25, 22, 8, "123456789ABCD", NoColorPair, TextAlignament::Center);
    }
};

class TicTacToeWin : public AppCUI::Controls::Window
{
    MyUserControl uc;

  public:
    TicTacToeWin()
    {
        this->Create("Custom renderer", "a:c,w:80,h:26");
        uc.Create(this, "x:0,y:0,w:100%,h:100%");
    }
    bool OnEvent(const void* sender, Event eventType, int controlID) override
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
    if (!Application::Init())
        return 1;
    Application::AddWindow(new TicTacToeWin());
    Application::Run();
    return 0;
}
