#include "Board.hpp"

bool Board::CanAdvanceOnYAxis(int scale) const
{
    if (currentPiece.has_value() == false)
    {
        return false;
    }

    const auto bHeight    = currentPiece->GetBlockHeight(scale);
    const auto nextY      = currentPiece->GetBottomYPosition(scale) + bHeight;
    const auto canAdvance = nextY <= matrixYBottom;
    return canAdvance;
}

bool Board::CanAdvanceOnXAxisLeft(int scale) const
{
    if (currentPiece.has_value() == false)
    {
        return false;
    }

    const auto bWidth     = currentPiece->GetBlockWidth(scale);
    const auto nextX      = currentPiece->GetLeftXPosition() - bWidth;
    const auto canAdvance = nextX > matrixXLeft;
    return canAdvance;
}

bool Board::CanAdvanceOnXAxisRight(int scale) const
{
    if (currentPiece.has_value() == false)
    {
        return false;
    }

    const auto bWidth     = currentPiece->GetBlockWidth(scale);
    const auto nextX      = currentPiece->GetRightXPosition(scale) + bWidth;
    const auto canAdvance = nextX <= matrixXRight;
    return canAdvance;
}

void Board::AdvanceOnYAxis(int scale)
{
    if (currentPiece.has_value() == false)
    {
        return;
    }

    const auto bHeight = currentPiece->GetBlockHeight(scale);
    currentPiece->UpdatePosition({ 0, bHeight });
}

void Board::AdvanceOnXAxisLeft(int scale)
{
    if (currentPiece.has_value() == false)
    {
        return;
    }

    const auto bWidth = currentPiece->GetBlockWidth(scale);
    currentPiece->UpdatePosition({ -bWidth, 0 });
}

void Board::AdvanceOnXAxisRight(int scale)
{
    if (currentPiece.has_value() == false)
    {
        return;
    }

    const auto bWidth = currentPiece->GetBlockWidth(scale);
    currentPiece->UpdatePosition({ bWidth, 0 });
}

void Board::Update(
      int scale, unsigned int maxPiecesInQueue, const Reference<Control> control, const Size& size, unsigned long delta)
{
    if (currentPiece.has_value())
    {
        if (CanAdvanceOnYAxis(scale) == false)
        {
            piecesProcessed.emplace_back(*currentPiece);
            currentPiece.reset();
        }
        else if (currentPieceUpdated != delta)
        {
            currentPieceUpdated = delta;
            AdvanceOnYAxis(scale);
        }
    }
    else
    {
        if (pieces.size() > 0)
        {
            currentPiece.emplace(pieces.front());
            pieces.pop_front();

            SetMatrixData(scale, size);
            const auto x = (maxtrixHSize / 2) * currentPiece->GetBlockWidth(scale);
            currentPiece->SetPosition({ x, matrixYTop });
        }
    }

    while (pieces.size() < maxPiecesInQueue)
    {
        const auto pieceType = static_cast<PieceType>(uniform_dist(e1));
        pieces.emplace_front(Piece{ pieceType, control, { 1, matrixYTop } });
    }
}

void Board::SetMatrixData(int scale, const Size& size)
{
    if (currentPiece.has_value())
    {
        // compute # of squares on horizontal
        const auto bWidth     = currentPiece->GetBlockWidth(scale);
        const auto panelWidth = size.Width;
        maxtrixHSize          = panelWidth / bWidth;
        matrixXLeft           = 1 + (panelWidth % bWidth) / 2;
        matrixXRight          = panelWidth - matrixXLeft;

        // compute # of squares on vertical
        const auto bHeight     = currentPiece->GetBlockHeight(scale);
        const auto panelHeight = size.Height;
        maxtrixVSize           = panelHeight / bHeight;
        matrixYTop             = 1 + (panelHeight % bHeight) / 2;
        matrixYBottom          = panelHeight - matrixYTop;
    }
}
