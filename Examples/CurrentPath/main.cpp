#include "AppCUI.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <istream>

using namespace AppCUI;
using namespace AppCUI::Application;
using namespace AppCUI::Controls;
using namespace AppCUI::Graphics;
namespace fs = std::filesystem;

int main()
{
    if (!Application::Init())
        return -1;
    auto window = Factory::Window::Create("Current executable path", "x:0,y:0,w:100%,h:2");
    auto lb     = Factory::Label::Create(*window, "Path", "x:0,y:0,w:100%,h:2");
    const auto& my_path = AppCUI::OS::GetCurrentApplicationPath();
    if (!my_path.empty())
        lb->SetText(my_path.string());
    Application::AddWindow(std::move(window));
    Application::Run();
    return 0;
}
