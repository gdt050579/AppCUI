#include "MainMenuState.hpp"

constexpr unsigned int StartButtonID     = 0x0001;
constexpr unsigned int HighScoreButtonID = 0x0002;
constexpr unsigned int ExitButtonID      = 0x0003;

MainMenuState::MainMenuState(const std::shared_ptr<GameData>& data) : data(data)
{
    page            = AppCUI::Controls::Factory::TabPage::Create(data->tab, "d:c,height:80");
    startButton     = AppCUI::Controls::Factory::Button::Create(page, "Start", "d:t,w:20", StartButtonID);
    highScoreButton = AppCUI::Controls::Factory::Button::Create(page, "HighScore", "d:c,w:20", HighScoreButtonID);
    exitButton      = AppCUI::Controls::Factory::Button::Create(page, "Exit", "d:b,w:20", ExitButtonID);
}

MainMenuState::~MainMenuState()
{
    data->tab->RemoveControl(page);
}

void MainMenuState::Init()
{
    // TODO: temporary fix for index
    data->tab->SetCurrentTabPage(data->machine->GetStatesCount() - 1);
    // data->tab->SetCurrentTabPage(page);
}

bool MainMenuState::HandleEvent(AppCUI::Controls::Control* ctrl, AppCUI::Controls::Event eventType, int controlID)
{
    if (eventType == AppCUI::Controls::Event::ButtonClicked)
    {
        switch (controlID)
        {
        case StartButtonID:
            AppCUI::Dialogs::MessageBox::ShowNotification("Info", "You have started the game!");
            break;
        case HighScoreButtonID:
            AppCUI::Dialogs::MessageBox::ShowNotification("Info", "High Score list!");
            break;
        case ExitButtonID:
            AppCUI::Dialogs::MessageBox::ShowNotification("Info", "Exiting the game! (not really)");
            break;
        default:
            break;
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
