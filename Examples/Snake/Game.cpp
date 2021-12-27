#include "Game.hpp"
#include "MainMenuState.hpp"

namespace Snake
{
Game::Game() noexcept
{
    data->tab = Factory::Tab::Create(this, "d:c", TabFlags::HideTabs | TabFlags::TransparentBackground);
    data->machine->PushState<MainMenuState>(data);
    data->machine->ProcessStackAction();
}

bool Game::OnEvent(Reference<Control> ctrl, Event eventType, int controlID)
{
    if (SingleApp::OnEvent(ctrl, eventType, controlID))
    {
        return true;
    }

    return data->machine->PeekState()->HandleEvent(ctrl, eventType, controlID);
}

void Game::Paint(Renderer& rd)
{
    data->machine->PeekState()->Draw(rd);
}

bool Game::OnFrameUpdate()
{
    const bool updated     = data->machine->PeekState()->Update();
    const bool actionTaken = data->machine->ProcessStackAction();

    return updated || actionTaken;
}
} // namespace Snake
