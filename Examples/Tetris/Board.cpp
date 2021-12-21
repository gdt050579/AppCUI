#include "Board.hpp"

bool Board::CanAdvanceOnYAxis() const
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

bool Board::CanAdvanceOnXAxisLeft() const
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

bool Board::CanAdvanceOnXAxisRight() const
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

bool Board::AdvanceOnYAxis()
{
    if (CanAdvanceOnYAxis() == false)
    {
        return false;
    }

    const auto bHeight = currentPiece->GetBlockHeight(scale);
    currentPiece->UpdatePosition({ 0, bHeight });
    currentPiece->UpdatePositionOnBoardMatrix({ 0, 1 });

    const auto& piecePositionOnBoard = currentPiece->GetPositionOnBoard();
    const auto& pieceMatrix          = currentPiece->GetMatrix();
    // TODO: update on board

    return true;
}

bool Board::AdvanceOnXAxisLeft()
{
    if (CanAdvanceOnXAxisLeft() == false)
    {
        return false;
    }

    const auto bWidth = currentPiece->GetBlockWidth(scale);
    currentPiece->UpdatePosition({ -bWidth, 0 });
    currentPiece->UpdatePositionOnBoardMatrix({ -1, 0 });

    const auto& piecePositionOnBoard = currentPiece->GetPositionOnBoard();
    const auto& pieceMatrix          = currentPiece->GetMatrix();
    // TODO: update on board

    return true;
}

bool Board::AdvanceOnXAxisRight()
{
    if (CanAdvanceOnXAxisRight() == false)
    {
        return false;
    }

    const auto bWidth = currentPiece->GetBlockWidth(scale);
    currentPiece->UpdatePosition({ bWidth, 0 });
    currentPiece->UpdatePositionOnBoardMatrix({ 1, 0 });

    const auto& piecePositionOnBoard = currentPiece->GetPositionOnBoard();
    const auto& pieceMatrix          = currentPiece->GetMatrix();
    // TODO: update on board

    return true;
}

void Board::Update(
      int scale, unsigned int maxPiecesInQueue, const Reference<Control> control, const Size& size, unsigned long delta)
{
    this->scale = scale;

    if (currentPiece.has_value())
    {
        if (CanAdvanceOnYAxis() == false)
        {
            piecesProcessed.emplace_back(*currentPiece);
            currentPiece.reset();
        }
        else if (currentPieceUpdated != delta)
        {
            currentPieceUpdated = delta;
            AdvanceOnYAxis();
        }
    }
    else
    {
        if (pieces.size() > 0)
        {
            currentPiece.emplace(pieces.front());
            pieces.pop_front();

            SetMatrixBounds(size);
            const auto x = (maxtrixHSize / 2) * currentPiece->GetBlockWidth(scale);
            currentPiece->SetPosition({ x, matrixYTop });
            currentPiece->SetPositionOnBoardMatrix({ maxtrixHSize / 2 - 1, 0 });
        }
    }

    while (pieces.size() < maxPiecesInQueue)
    {
        const auto pieceType = static_cast<PieceType>(uniform_dist(e1));
        pieces.emplace_front(Piece{ pieceType, control, { 1, matrixYTop } });
    }
}

void Board::SetMatrixBounds(const Size& size)
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
