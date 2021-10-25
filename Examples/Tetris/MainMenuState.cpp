#include "MainMenuState.hpp"

constexpr unsigned int StartButtonID = 0x0001;

MainMenuState::MainMenuState(const std::shared_ptr<GameData>& data) : data(data)
{
    page = AppCUI::Controls::Factory::TabPage::Create(data->tab, "");
    startButton = AppCUI::Controls::Factory::Button::Create(page, "Start", "d:c,w:20", StartButtonID);
}

MainMenuState::~MainMenuState()
{
    data->tab->RemoveControl(page);
}

void MainMenuState::Init()
{
    data->tab->SetCurrentTabPage(page);
}

bool MainMenuState::HandleEvent(AppCUI::Controls::Control* ctrl, AppCUI::Controls::Event eventType, int controlID)
{
    if (eventType == AppCUI::Controls::Event::ButtonClicked)
    {
        if (controlID == StartButtonID)
        {
            AppCUI::Dialogs::MessageBox::ShowNotification("Info", "You have started the game!");
        }
    }

    return false;
}

bool MainMenuState::Update()
{
    return false;
}

void MainMenuState::Draw(AppCUI::Graphics::Renderer& renderer)
{
    renderer.Clear(
          ' ', AppCUI::Graphics::ColorPair{ AppCUI::Graphics::Color::White, AppCUI::Graphics::Color::DarkBlue });
}

void MainMenuState::Pause()
{
}

void MainMenuState::Resume()
{
    data->tab->SetCurrentTabPage(page);
}
