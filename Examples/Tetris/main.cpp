#include "Game.hpp"

using namespace AppCUI::Application;

int main()
{
    if (Init(InitializationFlags::SingleWindowApp | InitializationFlags::Maximized |
             InitializationFlags::EnableFPSMode) == false)
    {
        return 1;
    }

    RunSingleApp(std::make_unique<Tetris::Game>());

    return 0;
}
