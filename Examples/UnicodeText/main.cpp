#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

class SimpleWin : public AppCUI::Controls::Window
{
    Label l1;

  public:
    SimpleWin()
    {
        this->Create("UNICODE test ", "d:c,w:40,h:15");
        l1.Create(
              this,
              u8"Words in German and French: \n - Déjà vu, Schön, Groß, Fähig\n\r"
              u8"Words in Russian: \n - Любовь, Кошка, Улыбаться\n"
              u8"Math symbols: ∑ ∫ ∏\n\r"
              u8"Cards: ♠ ♣ ♥ ♦\n"
              u8"Fractions: ⅓ ⅔ ⅕ ⅖ ⅗ ⅘ ⅙ ⅚ ⅛ ⅜ ⅝ ⅞\n\r"
              u8"--!--",
              "x:1,y:1,w:36,h:10");
    }
    bool OnEvent(Control*, Event eventType, int) override
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
    if (!Application::Init())
        return 1;
    Application::AddWindow(new SimpleWin());
    Application::Run();
    return 0;
}
