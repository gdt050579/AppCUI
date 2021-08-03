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
        this->Create("UNICODE test ", "a:c,w:40,h:15");
        l1.Create(
              this,
              u8"Words in German and Franch: \n - Déjà vu, Schön, Groß, Fähig\n"
              "Words in Russian: \n - Любовь, Кошка, Улыбаться\n"
              "Math symbols: ∑ ∫ ∏\n"
              "Cards: ♠ ♣ ♥ ♦\n"
              "Fractions: ⅓ ⅔ ⅕ ⅖ ⅗ ⅘ ⅙ ⅚ ⅛ ⅜ ⅝ ⅞\n"
              "--!--",
              "x:1,y:1,w:36,h:10");
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
    Application::AddWindow(new SimpleWin());
    Application::Run();
    return 0;
}
