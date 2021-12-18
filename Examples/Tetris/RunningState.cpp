#include "RunningState.hpp"

RunningState::RunningState(const std::shared_ptr<GameData>& data) : data(data), initialTime(clock())
{
    page            = Factory::TabPage::Create(data->tab, "");
    leftPanel       = Factory::Panel::Create(page, "Game", "t:0,b:0,w:75%,x:0%,a:l");
    rightPanel      = Factory::Panel::Create(page, "Info", "t:0,b:0,w:25%,x:100%,a:r");
    stats           = Factory::Panel::Create(rightPanel, "Stats", "d:t,h:30%");
    scoreLabel      = Factory::Label::Create(stats, "0", "t:1,b:0,w:50%,x:2%,a:l");
    timePassedLabel = Factory::Label::Create(stats, "0", "t:2,b:0,w:50%,x:2%,a:l");
    nextPiece       = Factory::Panel::Create(rightPanel, "Next Pieces", "d:b,h:70%");
    nextPiece01     = Factory::Panel::Create(nextPiece, "01", "d:t,h:35%");
    nextPiece02     = Factory::Panel::Create(nextPiece, "02", "d:c,h:33%");
    nextPiece03     = Factory::Panel::Create(nextPiece, "03", "d:b,h:33%");

    page->SetText("");
    leftPanel->SetText("");
    rightPanel->SetText("");
    scoreLabel->SetText("");
    timePassedLabel->SetText("");

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
    while (pieces.size() < maxPiecesInQueue)
    {
        const auto piece = static_cast<PieceType>(uniform_dist(e1));
        pieces.emplace_back((Piece{ piece, nextPiece.DownCast<Control>(), 1, 1 }));
    }

    data->tab->SetCurrentTabPage(page);
}

bool RunningState::HandleEvent(Reference<Control> ctrl, Event eventType, int controlID)
{
    return false;
}

bool RunningState::Update()
{
    LocalString<128> ls;
    ls.Format("Score: %u", score);
    scoreLabel->SetText(ls.GetText());

    delta = static_cast<unsigned long>((clock() - initialTime) * 1.0 / CLOCKS_PER_SEC);
    ls.Format("Time passed: %lus", delta);
    timePassedLabel->SetText(ls.GetText());

    if (currentPiece.has_value())
    {
        if (currentPieceUpdated != delta)
        {
            const auto bHeight = currentPiece->GetBlockHeight(pieceScaleInLeftPanel);
            if (currentPiece->GetBottomYPosition(pieceScaleInLeftPanel) + bHeight < leftPanel->GetHeight())
            {
                currentPiece->UpdatePosition(0, bHeight);
            }
            currentPieceUpdated = delta;
        }

        if (currentPiece->TouchedTheBottom(pieceScaleInLeftPanel, leftPanel->GetHeight()))
        {
            piecesProcessed.emplace_back(*currentPiece);
            currentPiece.reset();
        }
    }

    if (currentPiece.has_value() == false)
    {
        if (pieces.size() > 0)
        {
            currentPiece.emplace(pieces.front());
            pieces.pop_front();
        }
    }

    while (pieces.size() < maxPiecesInQueue)
    {
        const auto piece = static_cast<PieceType>(uniform_dist(e1));
        pieces.emplace_back((Piece{ piece, nextPiece.DownCast<Control>(), 1, 1 }));
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

bool RunningState::OnKeyEvent(Reference<Control> control, Key keyCode, char16_t unicodeChar)

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
      RunningState& rs, unsigned int id)
    : rs(rs), id(id)
{
}

void RunningState::PaintControlImplementationRightPiecePanels::PaintControl(
      Reference<Control> control, Renderer& renderer)
{
    control->Paint(renderer);

    if (rs.pieces.size() > id)
    {
        auto& piece = rs.pieces[id];
        piece.Draw(renderer, 3, true, control->GetWidth(), control->GetHeight());
    }
}

RunningState::PaintControlImplementationLeftPanel::PaintControlImplementationLeftPanel(RunningState& rs) : rs(rs)
{
}

void RunningState::PaintControlImplementationLeftPanel::PaintControl(Reference<Control> control, Renderer& renderer)
{
    control->Paint(renderer);

    if (rs.currentPiece.has_value())
    {
        if (rs.currentPiece->IsInitialPositionSet() == false)
        {
            rs.currentPiece->SetInitialPosition(control->GetWidth() / 2, 1);
        }
        rs.currentPiece->Draw(renderer, rs.pieceScaleInLeftPanel);
    }

    for (auto& piece : rs.piecesProcessed)
    {
        piece.Draw(renderer, rs.pieceScaleInLeftPanel);
    }
}

RunningState::OnKeyEventInterfaceImplementationLeftPanel::OnKeyEventInterfaceImplementationLeftPanel(RunningState& rs)
    : rs(rs)
{
}

bool RunningState::OnKeyEventInterfaceImplementationLeftPanel::OnKeyEvent(
      Reference<Control> control, Key keyCode, char16_t unicodeChar)
{
    switch (keyCode)
    {
    case Key::Left:
        if (rs.currentPiece.has_value())
        {
            const auto bWidth = rs.currentPiece->GetBlockWidth(rs.pieceScaleInLeftPanel);
            if (rs.currentPiece->GetLeftXPosition() - bWidth > 0)
            {
                rs.currentPiece->UpdatePosition(-bWidth, 0);
                return true;
            }
        }
        break;
    case Key::Right:
        if (rs.currentPiece.has_value())
        {
            const auto bWidth = rs.currentPiece->GetBlockWidth(rs.pieceScaleInLeftPanel);
            if (rs.currentPiece->GetRightXPosition(rs.pieceScaleInLeftPanel) + bWidth < control->GetWidth())
            {
                rs.currentPiece->UpdatePosition(bWidth, 0);
                return true;
            }
        }
        break;

    case Key::Down:
        if (rs.currentPiece.has_value())
        {
            const auto bHeight = rs.currentPiece->GetBlockHeight(rs.pieceScaleInLeftPanel);
            if (rs.currentPiece->GetBottomYPosition(rs.pieceScaleInLeftPanel) + bHeight < control->GetHeight())
            {
                rs.currentPiece->UpdatePosition(0, bHeight);
                return true;
            }
        }
        break;
    default:
        break;
    }

    return false;
}
