#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

int main()
{
    if (!Application::Init())
        return 1;
    auto wnd = Factory::Window::Create("UNICODE test ", "d:c,w:40,h:15");
    Factory::Label::Create(
          wnd,
          u8"Words in German and French: \n - Déjà vu, Schön, Groß, Fähig\n\r"
          u8"Words in Russian: \n - Любовь, Кошка, Улыбаться\n"
          u8"Math symbols: ∑ ∫ ∏\n\r"
          u8"Cards: ♠ ♣ ♥ ♦\n"
          u8"Fractions: ⅓ ⅔ ⅕ ⅖ ⅗ ⅘ ⅙ ⅚ ⅛ ⅜ ⅝ ⅞\n\r"
          u8"--!--",
          "x:1,y:1,w:36,h:10");
    Application::AddWindow(std::move(wnd));
    Application::Run();
    return 0;
}
