#include "AppCUI.hpp"

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;

#define USING_A_CLASS_IMPLEMENTATION

#ifdef USING_A_CLASS_IMPLEMENTATION
class SimpleWin : public AppCUI::Controls::Window
{
  public:
    SimpleWin()
    {
        Window::Init("Test", "d:c,w:40,h:10", WindowFlags::None);
        Label::Create(
              *this,
              "This is a simple Window example that creates a 40x20 fixed window that  will close if you click on the "
              "[X]  button from the bottom-top corner.",
              "x:1,y:1,w:36,h:5");
    }
};
int main()
{
    if (!Application::Init())
        return 1;
    Application::AddWindow(std::make_unique<SimpleWin>());
    Application::Run();
    return 0;
}
#else
// No class method
int main()
{
    if (!Application::Init())
        return 1;
    auto wnd = Window::Create("Test", "d:c,w:40,h:10");
    Label::Create(
          *wnd,
          "This is a simple Window example that creates a 40x20 fixed window that  will close if you click on the "
          "[X]  button from the bottom-top corner.",
          "x:1,y:1,w:36,h:5");
    Application::AddWindow(std::move(wnd));
    Application::Run();
    return 0;
}
#endif