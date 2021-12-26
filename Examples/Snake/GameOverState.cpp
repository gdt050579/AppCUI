#include "GameOverState.hpp"
#include "MainMenuState.hpp"

namespace Snake
{
GameOverState::GameOverState(const std::shared_ptr<GameData>& data)
    : data(data), score(data->score), timeElapsed(data->timeElapsed)
{
    page             = Factory::TabPage::Create(data->tab, "");
    gameOver         = Factory::Panel::Create(page, "Game over", "d:c,h:9,w:60");
    scoreLabel       = Factory::Label::Create(gameOver, "0", "t:1,b:0,w:100%,x:2%,a:l");
    timeElapsedLabel = Factory::Label::Create(gameOver, "0", "t:2,b:0,w:100%,x:2%,a:l");
    messageLabel     = Factory::Label::Create(gameOver, "0", "t:4,b:0,w:100%,x:2%,a:l");

    gameOver->Handlers()->OnKeyEvent = this;

    messageLabel->SetText("Thank you for playing! Press ENTER to continue...");

    LocalString<128> ls;
    ls.Format("Score: %u", score);
    scoreLabel->SetText(ls.GetText());

    ls.Format("Time elapsed: %lus", timeElapsed);
    timeElapsedLabel->SetText(ls.GetText());
}

GameOverState::~GameOverState()
{
    data->tab->RemoveControl(page);
}

void GameOverState::Init()
{
    data->tab->SetCurrentTabPage(page);
}

bool GameOverState::HandleEvent(Reference<Control> ctrl, Event eventType, int controlID)
{
    return false;
}

bool GameOverState::Update()
{
    return true;
}

void GameOverState::Draw(Renderer& renderer)
{
    renderer.HideCursor();
    renderer.Clear(' ', ColorPair{ Color::White, Color::DarkRed });
}

void GameOverState::Pause()
{
}

void GameOverState::Resume()
{
    data->tab->SetCurrentTabPage(page);
}

bool GameOverState::OnKeyEvent(Reference<Control> control, Key keyCode, char16_t unicodeChar)
{
    if (keyCode == Key::Enter)
    {
        data->machine->PushState<MainMenuState>(data, true);
    }

    return true;
}
} // namespace Tetris
