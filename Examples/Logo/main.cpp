#include "AppCUI.hpp"
#include <string>
#include <cstring>

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Input;
using namespace AppCUI::Graphics;

const char* AppCuiLogo[15] = {
    "............................", "..XXXXX}..XXXXXX}..XXXXXX}..", ".XX{--XX}.XX{--XX}.XX{--XX}.",
    ".XXXXXXX|.XXXXXX{].XXXXXX{].", ".XX{--XX|.XX{---]..XX{---]..", ".XX|..XX|.XX|......XX|......",
    ".[-]..[-].[-]......[-]......", "............................", "....XXXXX}..XX}...XX}.XX}...",
    "...XX{--XX}.XX|...XX|.XX|...", "...XX|..[-].XX|...XX|.XX|...", "...XX|..XX}.XX|...XX|.XX|...",
    "...[XXXXX{].[XXXXXX{].XX|...", "....[----]...[-----]..[-]...", "............................",
};

class LogoWin : public AppCUI::Controls::Window
{
    CanvasViewer viewLogo, viewInactive, viewSmall;
    Splitter sp;
    Panel pn;

    void CreateImage(Canvas* c)
    {
        if (!c)
            return;
        for (int y = 0; y < 15; y++)
        {
            for (int x = 0; AppCuiLogo[y][x]; x++)
            {
                SpecialChars sc;
                ColorPair col;
                switch (AppCuiLogo[y][x])
                {
                case 'X':
                    sc = SpecialChars::Block100;
                    if (y < 7)
                        col = ColorPair{ Color::Aqua, Color::Black };
                    else if (y < 11)
                        col = ColorPair{ Color::Green, Color::Black };
                    else
                        col = ColorPair{ Color::DarkGreen, Color::Black };
                    break;
                case '.':
                    sc  = SpecialChars::Block75;
                    col = ColorPair{ Color::DarkBlue, Color::Gray };
                    break;
                case '|':
                    sc  = SpecialChars::BoxVerticalSingleLine;
                    col = ColorPair{ Color::DarkRed, Color::Black };
                    break;
                case '-':
                    sc  = SpecialChars::BoxHorizontalSingleLine;
                    col = ColorPair{ Color::DarkRed, Color::Black };
                    break;
                case '{':
                    sc  = SpecialChars::BoxTopLeftCornerSingleLine;
                    col = ColorPair{ Color::DarkRed, Color::Black };
                    break;
                case '}':
                    sc  = SpecialChars::BoxTopRightCornerSingleLine;
                    col = ColorPair{ Color::DarkRed, Color::Black };
                    break;
                case '[':
                    sc  = SpecialChars::BoxBottomLeftCornerSingleLine;
                    col = ColorPair{ Color::DarkRed, Color::Black };
                    break;
                case ']':
                    sc  = SpecialChars::BoxBottomRightCornerSingleLine;
                    col = ColorPair{ Color::DarkRed, Color::Black };
                    break;
                }
                c->WriteSpecialCharacter(x, y, sc, col);
            }
        }
    }

  public:
    LogoWin()
    {
        this->Create("Canvas example", "a:c,w:80,h:22");
        sp.Create(this, "x:0,y:0,w:100%,h:100%", true);
        viewLogo.Create(&sp, "&Logo", "x:0,y:0,w:100%,h:100%", 28, 15);

        pn.Create(&sp, "x:0,y:0,w:100%,h:100%");
        viewInactive.Create(&pn, "Inactive", "x:1,y:1,w:29,h:16", 28, 15, ViewerFlags::Border);
        viewSmall.Create(&pn, "&Small", "x:33,y:1,w:14,h:9", 28, 15, ViewerFlags::Border);
        viewInactive.SetEnabled(false);
        CreateImage(viewLogo.GetCanvas());
        CreateImage(viewInactive.GetCanvas());
        CreateImage(viewSmall.GetCanvas());
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
    Application::AddWindow(new LogoWin());
    Application::Run();
    return 0;
}
