#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;

constexpr int32 RECT_WIDTH  = 18;
constexpr int32 RECT_HEIGHT = 3;
class ExampleWin : public Window
{
  public:
    ExampleWin() : Window("Rectangles", "d:c,w:80,h:23", WindowFlags::None)
    {
    }
    void Paint(Renderer& renderer) override
    {
        Window::Paint(renderer);
        DR(renderer, 0, 0, LineType::Single, "Single");
        DR(renderer, 1, 0, LineType::Double, "Double");
        DR(renderer, 2, 0, LineType::SingleThick, "Single Thick");
        DR(renderer, 3, 0, LineType::Border, "Border");

        DR(renderer, 0, 1, LineType::SingleRound, "Single Round");

        DR(renderer, 0, 2, LineType::Ascii, "Ascii");
        DR(renderer, 1, 2, LineType::AsciiRound, "Ascii Round");
        
    }
    void DR(Renderer& renderer, int32 x, int32 y, LineType lineType, string_view name)
    {
        x = 2 + x * (RECT_WIDTH + 1);
        y = 2 + y * 3;
        renderer.DrawRectSize(x, y, RECT_WIDTH, RECT_HEIGHT, ColorPair{ Color::White, Color::Transparent }, lineType);
        renderer.WriteSingleLineText(
              x + RECT_WIDTH / 2,
              y + RECT_HEIGHT / 2,
              name,
              ColorPair{ Color::Yellow, Color::Transparent },
              TextAlignament::Center);
    }
};
int main()
{
    if (!Application::Init())
        return 1;
    Application::AddWindow(std::make_unique<ExampleWin>());
    Application::Run();
    return 0;
}
