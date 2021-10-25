#include "Game.hpp"
#include "SplashState.hpp"

Game::Game() noexcept
{
    data->tab = AppCUI::Controls::Factory::Tab::Create(this, "d:c", AppCUI::Controls::TabFlags::HideTabs);
    data->machine->PushState<SplashState>(data);
    data->machine->ProcessStackAction();
}

bool Game::OnEvent(AppCUI::Controls::Control* ctrl, AppCUI::Controls::Event eventType, int controlID)
{
    if (AppCUI::Controls::SingleApp::OnEvent(ctrl, eventType, controlID))
    {
        return true;
    }

    const bool handled = data->machine->PeekState()->HandleEvent(ctrl, eventType, controlID);
    data->machine->PeekState()->Update();
    const bool actionTaken = data->machine->ProcessStackAction();

    return handled || actionTaken;
}

void Game::Paint(AppCUI::Graphics::Renderer& r)
{
    data->machine->PeekState()->Draw(r);
}

bool Game::OnFrameUpdate()
{
    const bool updated     = data->machine->PeekState()->Update();
    const bool actionTaken = data->machine->ProcessStackAction();

    return updated || actionTaken;
}
