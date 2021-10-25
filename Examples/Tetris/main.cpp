#include "Game.hpp"

int main()
{
    if (AppCUI::Application::Init(
              AppCUI::Application::InitializationFlags::SingleWindowApp |
              AppCUI::Application::InitializationFlags::EnableFPSMode) == false)
    {
        return 1;
    }

    AppCUI::Application::RunSingleApp(std::make_unique<Game>());

    return 0;
}
