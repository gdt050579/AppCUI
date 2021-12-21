#include "Piece.hpp"

Piece::Piece(const PieceType type, const Reference<Control> control, const Point& position)
    : control(control), type(type), position(position)
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

bool Piece::Draw(Renderer& renderer, int scale, bool center, int w, int h)
{
    if (center)
    {
        if (w < 2 || h < 2)
        {
            return false;
        }

        const auto size = GetSize(scale);
        const int x     = std::max<>(1, static_cast<int>((w - size.Width) / 2));
        const int y     = std::max<>(1, static_cast<int>(static_cast<int>((h - 2 - size.Height)) / 2));
        SetPosition({ x, y });

        return Draw(renderer, scale);
    }

    SetPosition({ 1, 1 });
    return Draw(renderer, scale);
}

bool Piece::Draw(Renderer& renderer, int scale)
{
    Point tmpPosition = position;

    for (auto i = 0U; i < rows; i++)
    {
        for (auto j = 0U; j < columns; j++)
        {
            if (matrix[i][j] == 1)
            {
                renderer.FillRectSize(
                      tmpPosition.X, tmpPosition.Y, GetBlockWidth(scale), GetBlockHeight(scale), '-', color);
            }
            else
            {
                renderer.DrawRectSize(
                      tmpPosition.X, tmpPosition.Y, GetBlockWidth(scale), GetBlockHeight(scale), color, false);
            }

            tmpPosition.X += size.Width * scale * 2;
        }

        tmpPosition.X = position.X;
        tmpPosition.Y += size.Height * scale;
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

void Piece::UpdatePosition(const Point& delta)
{
    position.X += delta.X;
    position.Y += delta.Y;
}

void Piece::SetPosition(const Point& newPosition)
{
    position = newPosition;
}

int Piece::GetBlockWidth(int scale) const
{
    return size.Width * scale * 2;
}

int Piece::GetBlockHeight(int scale) const
{
    return size.Height * scale;
}

int Piece::GetLeftXPosition() const
{
    return position.X;
}

int Piece::GetRightXPosition(int scale) const
{
    return position.X + GetSize(scale).Width;
}

int Piece::GetTopYPosition() const
{
    return position.Y;
}

int Piece::GetBottomYPosition(int scale) const
{
    return position.Y + GetSize(scale).Height;
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

void Piece::SetPositionOnBoardMatrix(const Point& position)
{
    positionOnBoard = position;
}

void Piece::UpdatePositionOnBoardMatrix(const Point& position)
{
    positionOnBoard.X += position.X;
    positionOnBoard.Y += position.Y;
}
