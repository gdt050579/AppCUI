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

class CurrentPath : public AppCUI::Controls::Window
{
  private:
    Label lblMyPath;

  public:
    CurrentPath()
    {
        this->Create("Current executable path", "x:0,y:0,w:100%,h:2");
        lblMyPath.Create(this, "Path", "x:0,y:0,w:100%,h:2");
        const auto& my_path = AppCUI::OS::GetCurrentApplicationPath();
        if (!my_path.empty())
        {
            lblMyPath.SetText(my_path.string());
        }
    }
};

int main()
{
    if (!Application::Init())
        return -1;
    auto window = std::make_unique<CurrentPath>();
    Application::AddWindow(std::move(window));
    Application::Run();
    return 0;
}
