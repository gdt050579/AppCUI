#include "RunningState.hpp"

namespace Snake
{
RunningState::RunningState(const std::shared_ptr<GameData>& data) : data(data), initialTime(clock())
{
    page            = Factory::TabPage::Create(data->tab, "");
    leftPanel       = Factory::Panel::Create(page, "Game", "t:0,b:0,w:75%,x:0%,a:l");
    rightPanel      = Factory::Panel::Create(page, "Info", "t:0,b:0,w:25%,x:100%,a:r");
    scoreLabel      = Factory::Label::Create(rightPanel, "0", "t:1,b:0,w:50%,x:2%,a:l");
    timePassedLabel = Factory::Label::Create(rightPanel, "0", "t:2,b:0,w:50%,x:2%,a:l");
    levelLabel      = Factory::Label::Create(rightPanel, "0", "t:3,b:0,w:50%,x:2%,a:l");

    page->SetText("");
    leftPanel->SetText("");
    rightPanel->SetText("");
    scoreLabel->SetText("");
    timePassedLabel->SetText("");
    levelLabel->SetText("");

    page->Handlers()->OnKeyEvent = this;

    leftPanel->Handlers()->PaintControl = &pcilp;
    leftPanel->Handlers()->OnKeyEvent   = &okeiilp;

    data->level       = 0;
    data->score       = 0;
    data->timeElapsed = 0;
}

RunningState::~RunningState()
{
    data->tab->RemoveControl(page);
}

void RunningState::Init()
{
    data->tab->SetCurrentTabPage(page);
}

bool RunningState::HandleEvent(Reference<Control> /*ctrl*/, Event /*eventType*/, int /*controlID*/)
{
    return false;
}

bool RunningState::Update()
{
    LocalString<128> ls;
    ls.Format("Score: %u", board.GetScore());
    scoreLabel->SetText(ls.GetText());

    delta = static_cast<unsigned long>((clock() - initialTime) * 1.0);
    ls.Format("Time passed: %lus", delta / CLOCKS_PER_SEC);
    timePassedLabel->SetText(ls.GetText());

    ls.Format("Level: %u", board.GetLevel());
    levelLabel->SetText(ls.GetText());

    bool gameOver = false;
    board.Update(
          pieceScaleInLeftPanel,
          rightPanel.ToBase<Control>(),
          { leftPanel->GetWidth() - 2U, leftPanel->GetHeight() - 2U },
          delta,
          gameOver);

    if (gameOver)
    {
        data->score       = board.GetScore();
        data->timeElapsed = delta / CLOCKS_PER_SEC;
        data->level       = board.GetLevel();
        data->machine->PushState<GameOverState>(data, true);
    }

    return true;
}

void RunningState::Draw(Renderer& renderer)
{
    renderer.HideCursor();
    renderer.Clear(' ', ColorPair{ Color::White, Color::DarkBlue });
}

void RunningState::Pause()
{
}

void RunningState::Resume()
{
    data->tab->SetCurrentTabPage(page);
}

bool RunningState::OnKeyEvent(Reference<Control> /*control*/, Key keyCode, char16_t /*unicodeChar*/)

{
    switch (keyCode)
    {
    case Key::Escape:
        this->data->machine->PushState<PauseState>(this->data, false);
        return true;
    default:
        break;
    }

    return false;
}

RunningState::PaintControlImplementationLeftPanel::PaintControlImplementationLeftPanel(Board& board) : board(board)
{
}

void RunningState::PaintControlImplementationLeftPanel::PaintControl(Reference<Control> control, Renderer& renderer)
{
    control->Paint(renderer);
    board.Draw(
          renderer,
          { static_cast<unsigned int>(control->GetWidth()), static_cast<unsigned int>(control->GetHeight()) });
}

RunningState::OnKeyEventInterfaceImplementationLeftPanel::OnKeyEventInterfaceImplementationLeftPanel(Board& board)
    : board(board)
{
}

bool RunningState::OnKeyEventInterfaceImplementationLeftPanel::OnKeyEvent(
      Reference<Control> /*control*/, Key keyCode, char16_t /*unicodeChar*/)
{
    switch (keyCode)
    {
    case Key::Left:
        return board.SetDirection(Board::HeadingTo::Left);
    case Key::Right:
        return board.SetDirection(Board::HeadingTo::Right);
    case Key::Down:
        return board.SetDirection(Board::HeadingTo::Down);
    case Key::Up:
        return board.SetDirection(Board::HeadingTo::Up);

    default:
        break;
    }

    return false;
}
} // namespace Snake
