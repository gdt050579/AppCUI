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
            renderer.FillHorizontalLine(5, tr, 12, '-', ColorPair{ Color::White, Color::Blue });
            renderer.FillHorizontalLine(25, tr, 32, '-', ColorPair{ Color::White, Color::DarkGreen });
            renderer.FillHorizontalLine(45, tr, 52, '-', ColorPair{ Color::White, Color::DarkRed });
        }
            
        // first batch (direct write through WriteText function)
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

        // 3rd batch (with 3rd letter (offset 2 in a zero-index array) is a hotkey highlighted)
        ColorPair hkCP = ColorPair{ Color::Black, Color::White };
        renderer.WriteSingleLineText(45, 2, "1234", NoColorPair, hkCP, 2);
        renderer.WriteSingleLineText(45, 4, "123456789ABCD", NoColorPair, hkCP, 2);
        renderer.WriteSingleLineText(45, 6, 8, "123456789ABCD", NoColorPair,hkCP, 2);
        renderer.WriteSingleLineText(45, 8, "1234", NoColorPair, hkCP, 2, TextAlignament::Right);
        renderer.WriteSingleLineText(45, 10, 8, "1234", NoColorPair, hkCP, 2, TextAlignament::Right);
        renderer.WriteSingleLineText(45, 12, 8, "123456789ABCD", NoColorPair, hkCP, 2, TextAlignament::Right);
        renderer.WriteSingleLineText(45, 14, 8, "12345678", NoColorPair, hkCP, 2, TextAlignament::Right);
        renderer.WriteSingleLineText(45, 16, "123", NoColorPair, hkCP, 2, TextAlignament::Center);
        renderer.WriteSingleLineText(45, 18, 8, "1234", NoColorPair, hkCP, 2, TextAlignament::Center);
        renderer.WriteSingleLineText(45, 20, 8, "12345678", NoColorPair, hkCP, 2, TextAlignament::Center);
        renderer.WriteSingleLineText(45, 22, 8, "123456789ABCD", NoColorPair, hkCP, 2, TextAlignament::Center);

        renderer.FillVerticalLineWithSpecialChar(
              60, 0, 26, SpecialChars::BoxVerticalDoubleLine, ColorPair{ Color::White, Color::DarkBlue });

        // multi line example
        renderer.FillRectSize(62, 2, 15, 3, ' ', ColorPair{ Color::White, Color::Blue });
        params.Flags = WriteTextFlags::MultipleLines;
        params.Align = TextAlignament::Left;
        params.X     = 62;
        params.Y     = 2;
        renderer.WriteText("This is a multi-line string\n- it expends over 2 lines\n\r- it has no clipping wo width", params);
        
        renderer.FillRectSize(62, 7, 20, 3, ' ', ColorPair{ Color::White, Color::Blue });
        params.Flags |= WriteTextFlags::ClipToWidth;
        params.Y     = 7;
        params.Width = 20;
        renderer.WriteText("This is a multi-line string\n- Clipped to width\n\r- Long line being clipped by width", params);

        renderer.FillRectSize(62, 11, 20, 3, ' ', ColorPair{ Color::White, Color::Blue });
        params.Y     = 11;
        params.Align = TextAlignament::Right;
        renderer.WriteText("This is a multi-line string\n- Clipped to width\n\r- Long line being clipped by width", params);

        renderer.FillRectSize(62, 16, 20, 4, ' ', ColorPair{ Color::White, Color::Blue });
        params.Flags = WriteTextFlags::MultipleLines | WriteTextFlags::WrapToWidth;
        params.Y     = 16;
        params.Width = 20;
        params.Align = TextAlignament::Left;
        renderer.WriteText("This is a string that will be wrapped to the current width into multiple lines.", params);

    }
};

class TicTacToeWin : public AppCUI::Controls::Window
{
    MyUserControl uc;

  public:
    TicTacToeWin()
    {
        this->Create("Single line strings", "d:c,w:100,h:26");
        uc.Create(this, "x:0,y:0,w:100%,h:100%");
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
    if (!Application::Init())
        return 1;
    Application::AddWindow(new TicTacToeWin());
    Application::Run();
    return 0;
}
