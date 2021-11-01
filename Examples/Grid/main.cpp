#include "AppCUI.hpp"

class SimpleWin : public AppCUI::Controls::Window
{
  public:
    SimpleWin()
        : Window(
                "Grid Sample",
                "d:c,w:100%,h:100%",
                AppCUI::Controls::WindowFlags::NoCloseButton | AppCUI::Controls::WindowFlags::FixedPosition |
                      AppCUI::Controls::WindowFlags::Maximized)
    {
        auto grid = AppCUI::Controls::Factory::Grid::Create(this, "d:c,w:100%,h:100%");
    }
};

int main()
{
    if (AppCUI::Application::Init(
              AppCUI::Application::InitializationFlags::Maximized |
              AppCUI::Application::InitializationFlags::FixedSize) == false)
    {
        return 1;
    }

    AppCUI::Application::AddWindow(std::make_unique<SimpleWin>());
    AppCUI::Application::Run();

    return 0;
}
