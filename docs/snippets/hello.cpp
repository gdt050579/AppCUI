// Sample of docs/source/index.rst - compiled with the examples (docs/snippets/CMakeLists.txt).
// [hello]
#include "AppCUI.hpp"
using namespace AppCUI;

int main()
{
    if (!Application::Init())
        return 1;
    auto window = Controls::Factory::Window::Create("Hello", "d:c,w:30,h:6");
    Controls::Factory::Label::Create(*window, "Hello, AppCUI!", "x:1,y:1,w:20");
    Application::AddWindow(std::move(window));
    Application::Run();
    return 0;
}
// [/hello]
