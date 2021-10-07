#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Graphics;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

class MyDeskop : public Desktop
{
  public:
    void Paint(Renderer& r) override
    {
        int h          = this->GetHeight();
        int w          = this->GetWidth();
        ColorPair c[3] = { ColorPair{ Color::Black, Color::Blue },
                           ColorPair{ Color::Black, Color::Gray },
                           ColorPair{ Color::Black, Color::DarkRed } };
        for (auto y = 0; y < h; y++)
        {
            for (auto x = 0; x < w; x += 2)
            {
                r.WriteSingleLineText(x, y, "  ", c[(x + y) % 3]);
            }
        }
        Utils::LocalString<64> tmp;
        tmp.Format(" Desktop size: %d x %d ", w, h);
        r.WriteSingleLineText(w - tmp.Len() - 2, h - 2, tmp.GetText(), ColorPair{ Color::Green, Color::Black });
    }
};

int main()
{
    InitializationData initData;
    initData.CustomDesktopConstructor = []()-> Desktop* { return new MyDeskop(); };
    initData.Flags                    = InitializationFlags::CommandBar | InitializationFlags::Menu;
    if (!Application::Init(initData))
        return 1;
    auto w = Factory::Window::Create("Test", "d:c,w:20,h:5", WindowFlags::Sizeable);
    Application::AddWindow(std::move(w));
    Application::Run();
    return 0;
}
