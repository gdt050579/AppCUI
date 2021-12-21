#include "Board.hpp"

bool Board::CanAdvanceOnYAxis(const Piece& piece, int scale) const
{
    const auto bHeight    = piece.GetBlockHeight(scale);
    const auto nextY      = piece.GetBottomYPosition(scale) + bHeight;
    const auto canAdvance = nextY <= matrixYBottom;
    return canAdvance;
}

bool Board::CanAdvanceOnXAxisLeft(const Piece& piece, int scale) const
{
    const auto bWidth     = piece.GetBlockWidth(scale);
    const auto nextX      = piece.GetLeftXPosition() - bWidth;
    const auto canAdvance = nextX > matrixXLeft;
    return canAdvance;
}

bool Board::CanAdvanceOnXAxisRight(const Piece& piece, int scale) const
{
    const auto bWidth     = piece.GetBlockWidth(scale);
    const auto nextX      = piece.GetRightXPosition(scale) + bWidth;
    const auto canAdvance = nextX <= matrixXRight;
    return canAdvance;
}

void Board::AdvanceOnYAxis(Piece& piece, int scale)
{
    const auto bHeight = piece.GetBlockHeight(scale);
    piece.UpdatePosition({ 0, bHeight });
}

void Board::AdvanceOnXAxisLeft(Piece& piece, int scale)
{
    const auto bWidth = piece.GetBlockWidth(scale);
    piece.UpdatePosition({ -bWidth, 0 });
}

void Board::AdvanceOnXAxisRight(Piece& piece, int scale)
{
    const auto bWidth = piece.GetBlockWidth(scale);
    piece.UpdatePosition({ bWidth, 0 });
}
