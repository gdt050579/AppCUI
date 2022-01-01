#include "RunningState.hpp"

namespace Tetris
{
RunningState::RunningState(const std::shared_ptr<GameData>& data) : data(data), initialTime(clock())
{
    page              = Factory::TabPage::Create(data->tab, "");
    leftPanel         = Factory::Panel::Create(page, "Game", "t:0,b:0,w:75%,x:0%,a:l");
    rightPanel        = Factory::Panel::Create(page, "Info", "t:0,b:0,w:25%,x:100%,a:r");
    stats             = Factory::Panel::Create(rightPanel, "Stats", "d:t,h:30%");
    scoreLabel        = Factory::Label::Create(stats, "0", "t:1,b:0,w:50%,x:2%,a:l");
    timePassedLabel   = Factory::Label::Create(stats, "0", "t:2,b:0,w:50%,x:2%,a:l");
    trainingModeLabel = Factory::Label::Create(stats, "False", "t:3,b:0,w:50%,x:2%,a:l");
    nextPiece         = Factory::Panel::Create(rightPanel, "Next Pieces", "d:b,h:70%");
    nextPiece01       = Factory::Panel::Create(nextPiece, "01", "d:t,h:35%");
    nextPiece02       = Factory::Panel::Create(nextPiece, "02", "d:c,h:33%");
    nextPiece03       = Factory::Panel::Create(nextPiece, "03", "d:b,h:33%");

    page->SetText("");
    leftPanel->SetText("");
    rightPanel->SetText("");
    scoreLabel->SetText("");
    timePassedLabel->SetText("");
    trainingModeLabel->SetText("");

    page->Handlers()->OnKeyEvent          = this;
    nextPiece01->Handlers()->PaintControl = &pcirpp01;
    nextPiece02->Handlers()->PaintControl = &pcirpp02;
    nextPiece03->Handlers()->PaintControl = &pcirpp03;

    leftPanel->Handlers()->PaintControl = &pcilp;
    leftPanel->Handlers()->OnKeyEvent   = &okeiilp;
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

    delta = static_cast<unsigned long>((clock() - initialTime) * 1.0 / CLOCKS_PER_SEC);
    ls.Format("Time passed: %lus", delta);
    timePassedLabel->SetText(ls.GetText());

    if (board.GetTrainingMode())
    {
        trainingModeLabel->SetText("Training mode: true");
    }
    else
    {
        trainingModeLabel->SetText("Training mode: false");
    }

    bool gameOver = false;
    board.Update(
          pieceScaleInLeftPanel,
          maxPiecesInQueue,
          nextPiece.ToBase<Control>(),
          { leftPanel->GetWidth() - 2U, leftPanel->GetHeight() - 2U },
          delta,
          gameOver);

    if (gameOver)
    {
        data->score       = board.GetScore();
        data->timeElapsed = delta;
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

RunningState::PaintControlImplementationRightPiecePanels::PaintControlImplementationRightPiecePanels(
      Board& board, unsigned int id)
    : board(board), id(id)
{
}

void RunningState::PaintControlImplementationRightPiecePanels::PaintControl(
      Reference<Control> control, Renderer& renderer)
{
    control->Paint(renderer);
    board.DrawPieceById(
          renderer,
          id,
          { static_cast<unsigned int>(control->GetWidth()), static_cast<unsigned int>(control->GetHeight()) },
          2);
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
        return board.AdvanceOnXAxisLeft();
    case Key::Right:
        return board.AdvanceOnXAxisRight();

    case Key::Down:
        return board.AdvanceOnYAxis();

    case Key::Up:
    case Key::Space:
        return board.Rotate();

    case Key::T:
        board.ToggleTrainingMode();

    default:
        break;
    }

    return false;
}
} // namespace Tetris
