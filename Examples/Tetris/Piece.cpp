#include "Piece.hpp"

namespace Tetris
{
Piece::Piece(const PieceType type, const Reference<Control> control, const Point& position)
    : control(control), type(type)
{
    switch (type)
    {
    case PieceType::I:
        matrix[0][0] = 1;
        matrix[1][0] = 1;
        matrix[2][0] = 1;
        matrix[3][0] = 1;
        break;
    case PieceType::O:
        matrix[0][0] = 1;
        matrix[0][1] = 1;
        matrix[1][0] = 1;
        matrix[1][1] = 1;
        break;
    case PieceType::T:
        matrix[0][0] = 1;
        matrix[1][0] = 1;
        matrix[2][0] = 1;
        matrix[1][1] = 1;
        break;
    case PieceType::S:
        matrix[0][1] = 1;
        matrix[0][2] = 1;
        matrix[1][0] = 1;
        matrix[1][1] = 1;
        break;
    case PieceType::L:
        matrix[0][0] = 1;
        matrix[1][0] = 1;
        matrix[1][1] = 1;
        matrix[1][2] = 1;
        break;
    case PieceType::Z:
        matrix[0][0] = 1;
        matrix[0][1] = 1;
        matrix[1][1] = 1;
        matrix[1][2] = 1;
        break;
    case PieceType::J:
        matrix[0][2] = 1;
        matrix[1][0] = 1;
        matrix[1][1] = 1;
        matrix[1][2] = 1;
        break;
    default:
        break;
    }
}

bool Piece::Draw(Renderer& renderer, int scale, const Point& position)
{
    Point tmpPosition = position;
    const auto w      = GetBlockWidth(scale);
    const auto h      = GetBlockHeight(scale);

    for (auto y = 0U; y < rows; y++)
    {
        for (auto x = 0U; x < columns; x++)
        {
            if (matrix[y][x] == true)
            {
                renderer.DrawRectSize(tmpPosition.X, tmpPosition.Y, w, h, color, false);
            }
            tmpPosition.X += w;
        }

        tmpPosition.X = position.X;
        tmpPosition.Y += h;
    }

    return true;
}

Size Piece::GetSize(int scale) const
{
    const auto w = size.Width * scale * 2;
    const auto h = size.Height * scale;

    switch (type)
    {
    case PieceType::I:
        return { w, h * 3 };
    case PieceType::O:
        return { w * 2, h };
    case PieceType::T:
        return { w * 2, h * 2 };
    case PieceType::S:
        return { w * 3, h };
    case PieceType::L:
        return { w * 3, h };
    case PieceType::Z:
        return { w * 3, h };
    case PieceType::J:
        return { w * 3, h };
    default:
        break;
    }

    return { 0, 0 };
}

int Piece::GetBlockWidth(int scale) const
{
    return size.Width * scale * 2;
}

int Piece::GetBlockHeight(int scale) const
{
    return size.Height * scale;
}

bool Piece::Rotate()
{
    std::array<std::array<bool, rows>, columns> tmp{ 0 };

    for (auto i = 0U; i < matrix.size(); i++)
    {
        for (auto j = 0U; j < matrix[i].size(); j++)
        {
            tmp[j][matrix.size() - i - 1] = matrix[i][j];
        }
    }

    matrix = tmp;

    return true;
}

const std::array<std::array<bool, Piece::rows>, Piece::columns>& Piece::GetMatrix() const
{
    return matrix;
}

const Point& Piece::GetPositionOnBoard() const
{
    return positionOnBoard;
}

void Piece::SetPositionOnBoard(const Point& position)
{
    positionOnBoard = position;
}

void Piece::UpdatePositionOnBoard(const Point& position)
{
    positionOnBoard.X += position.X;
    positionOnBoard.Y += position.Y;
}
} // namespace Tetris
