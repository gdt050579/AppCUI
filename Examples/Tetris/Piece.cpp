#include "Piece.hpp"

namespace Tetris
{
Piece::Piece(const PieceType type, const Reference<Control> control, const ColorPair& bodyColor)
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

    this->colorBody = bodyColor;
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
                renderer.FillRectSize(tmpPosition.X + 1, tmpPosition.Y + 1, w - 1, h - 1, ' ', colorBody);
                // renderer.DrawRectSize(tmpPosition.X, tmpPosition.Y, w, h, colorBorder, LineType::Single);
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

bool Piece::Rotate(const Size& boardSize)
{
    std::array<std::array<bool, rows>, columns> tmp{ { { false } } };

    for (auto y = 0U; y < matrix.size(); y++)
    {
        for (auto x = 0U; x < matrix[y].size(); x++)
        {
            tmp[y][x] = matrix[matrix.size() - x - 1][y];
        }
    }

    matrix = tmp;

    Point lowest  = { 0, 0 };
    Point highest = { 0, 0 };
    for (auto y = 0U; y < matrix.size(); y++)
    {
        for (auto x = 0U; x < matrix[y].size(); x++)
        {
            if (matrix[y][x] == true)
            {
                lowest.X = std::min<>(positionOnBoard.X + static_cast<int>(x), lowest.X);
                lowest.Y = std::min<>(positionOnBoard.Y + static_cast<int>(y), lowest.Y);

                highest.X = std::max<>(positionOnBoard.X + static_cast<int>(x), highest.X);
                highest.Y = std::max<>(positionOnBoard.Y + static_cast<int>(y), highest.Y);
            }
        }
    }

    if (lowest.X < 0)
    {
        UpdatePositionOnBoard({ -lowest.X, 0 });
    }
    else if (highest.X >= static_cast<int>(boardSize.Width))
    {
        UpdatePositionOnBoard({ -(highest.X - static_cast<int>(boardSize.Width)) - 1, 0 });
    }
    if (lowest.Y < 0)
    {
        UpdatePositionOnBoard({ 0, -lowest.Y });
    }
    else if (highest.Y >= static_cast<int>(boardSize.Height))
    {
        UpdatePositionOnBoard({ -(highest.Y - static_cast<int>(boardSize.Height)) - 1, 0 });
    }

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

void Piece::SetBorderColor(const ColorPair& color)
{
    colorBorder = color;
}

void Piece::SetBodyColor(const ColorPair& color)
{
    colorBody = color;
}

const ColorPair& Piece::GetBorderColor() const
{
    return colorBorder;
}

const ColorPair& Piece::GetBodyColor() const
{
    return colorBody;
}
} // namespace Tetris
